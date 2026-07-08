#!/usr/bin/env python3
"""
LoreMakeThunk.py: generate a Lorelei thunk (guest GTL + host HTL) for one library from a devkit,
using only the devkit's own LoreTLC and clang. No cmake, no make, no git, no manifest file.

Given a real shared library and the headers that declare its API, it:
  1. dumps the library's exported functions (nm) into a Symbols list,
  2. runs the devkit's LoreTLC to stat + generate the host and guest thunk sources,
  3. compiles each into a shared library with the devkit's clang,
laying the two out in the standard thunk-pack layout, so the output directory is itself a ready
LORELEI_THUNK_PATH prefix:

  <out>/lib/<host-arch>-LoreHTL/lib<name>_HTL.so
  <out>/x86_64/lib/x86_64-LoreGTL/lib<name>.so   (+ soname symlink)

Usage (--devkit defaults to $LORELEI_DEVKIT or the devkit this script is installed in. Header flags
follow --, clang-tooling style):
  LoreMakeThunk.py [--devkit <dir>] --name <name> --lib <lib.so> --header <hdr> -o <out> [-- <compile arguments>]

The four intermediates it normally generates (Desc.h, Symbols.conf, Manifest_host.cpp,
Manifest_guest.cpp) can each be supplied instead, with --desc / --symbols / --manifest-host /
--manifest-guest. Anything not supplied is still generated. A supplied --desc brings its own #includes,
so --header is then unused, and the symbol list comes from either --symbols or --lib.

Example:
  LoreMakeThunk.py --name z --lib /usr/lib/x86_64-linux-gnu/libz.so.1 --header zlib.h \
               -o ./out -- -I/usr/include
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

GUEST_ARCH = "x86_64"
GUEST_TRIPLET = "x86_64-pc-linux-gnu"
HOST_TRIPLETS = {
    "x86_64": "x86_64-pc-linux-gnu",
    "aarch64": "aarch64-unknown-linux-gnu",
    "riscv64": "riscv64-unknown-linux-gnu",
}
# Flags every thunk TU is compiled with (mirrors thunk_configure_target in LoreThunkBuildApi.cmake).
TU_FLAGS = ["-std=gnu++20", "-fPIC", "-fvisibility=hidden",
            "-fvisibility-inlines-hidden", "-fno-exceptions", "-fno-rtti"]
# The thunk sits at lib/<arch>-Lore?TL (two components under the prefix), so one ../.. reaches the
# prefix and ../../lib the runtime lib dir (mirrors thunk_default_install_rpath).
RPATH = "$ORIGIN:$ORIGIN/../../lib"


DRY_RUN = False


def run(cmd, **kw):
    print("  $ " + " ".join(str(c) for c in cmd))
    if DRY_RUN:
        return
    subprocess.run([str(c) for c in cmd], check=True, **kw)


def capture(cmd):
    return subprocess.run([str(c) for c in cmd], check=True,
                          capture_output=True, text=True).stdout


def die(msg):
    sys.exit("LoreMakeThunk.py: error: " + msg)


def first_existing(*paths):
    for p in paths:
        if p and Path(p).exists():
            return Path(p)
    return None


def resolve_devkit(arg):
    """--devkit, else $LORELEI_DEVKIT, else the devkit this script is installed in (bin/..)."""
    if arg:
        return arg
    env = os.environ.get("LORELEI_DEVKIT")
    if env:
        return env
    # Installed at <devkit>/bin/LoreMakeThunk.py, so the prefix is two levels up.
    cand = Path(__file__).resolve().parent.parent
    if (cand / "bin" / "LoreTLC").exists():
        return cand
    die("no devkit: pass --devkit, set LORELEI_DEVKIT, or install this script in a devkit's bin/")


class Devkit:
    """Resolve the fixed devkit layout and validate the pieces we need are present."""

    def __init__(self, prefix):
        self.prefix = Path(prefix).resolve()
        if not self.prefix.is_dir():
            die(f"--devkit is not a directory: {self.prefix}")

        self.tlc = self._need(self.prefix / "bin" / "LoreTLC", "LoreTLC")
        self.host_cxx = self._need(first_existing(self.prefix / "bin" / "clang++"), "host clang++")
        # Guest C++ compiler: prefer the devkit's x86_64 wrapper (target + sysroot baked in); else
        # drive the host clang++ at x86_64 with the devkit's guest sysroot.
        self.guest_cxx_wrapper = first_existing(self.prefix / "bin" / "x86_64-linux-gnu-clang++")

        self.host_include = self._need(self.prefix / "include", "host include dir")
        self.guest_include = self._need(self.prefix / "x86_64" / "include", "guest include dir")
        self.host_libdir = self._need(self.prefix / "lib", "host lib dir")
        self.guest_libdir = self._need(self.prefix / "x86_64" / "lib", "guest lib dir")
        self.guest_sysroot = self._need(self.prefix / "x86_64" / "sysroot", "guest x86_64 sysroot")

        self.nm = first_existing(self.prefix / "bin" / "llvm-nm") or "nm"
        self.readelf = first_existing(self.prefix / "bin" / "llvm-readelf") or "readelf"

        # Sanity: the manifest fragments the generated sources #include must be reachable.
        self._need(self.host_include / "lorelei" / "ThunkInterface" / "ManifestHost.cpp.inc",
                   "ThunkInterface headers (is this a lorelei devkit?)")

        self.host_arch = self._detect_host_arch()
        self.host_triplet = HOST_TRIPLETS[self.host_arch]

    def _need(self, path, what):
        if path is None or not Path(path).exists():
            die(f"{what} not found in devkit ({self.prefix})")
        return Path(path)

    def _detect_host_arch(self):
        arch = capture([self.host_cxx, "-dumpmachine"]).strip().split("-", 1)[0]
        if arch in ("x86_64", "amd64"):
            return "x86_64"
        if arch in ("aarch64", "arm64"):
            return "aarch64"
        if arch == "riscv64":
            return "riscv64"
        die(f"unsupported host architecture '{arch}'")

    def guest_compile_cmd(self):
        """The base argv for compiling the guest (x86_64) thunk."""
        if self.guest_cxx_wrapper:
            return [self.guest_cxx_wrapper]
        return [self.host_cxx, "-target", GUEST_TRIPLET, f"--sysroot={self.guest_sysroot}"]


def read_soname(dk, lib):
    try:
        out = capture([dk.readelf, "-d", lib])
    except subprocess.CalledProcessError:
        return None
    m = re.search(r"SONAME.*\[(.+?)\]", out)
    return m.group(1) if m else None


def make_aliases(dirpath, target, names):
    """Create each name in `names` as a symlink to `target` in `dirpath`, skipping the target itself
    and duplicates. Returns the names actually created."""
    made = []
    for name in dict.fromkeys(names):        # dedup, preserve order
        if name == target:
            continue
        link = dirpath / name
        if link.exists() or link.is_symlink():
            link.unlink()
        link.symlink_to(target)
        made.append(name)
    return made


def dump_functions(dk, lib):
    """Exported text symbols of the real library, like DumpSyms.py's [Function] section."""
    out = capture([dk.nm, "-D", "--defined-only", str(lib)])
    funcs, seen = [], set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        typ = parts[-2] if len(parts) >= 3 else parts[0]
        name = parts[-1].split("@", 1)[0]   # drop nm's @@VERSION / @VERSION tag
        if typ in ("T", "W") and name and name not in seen:
            seen.add(name)
            funcs.append(name)
    return funcs


def functions_from_symbols(path):
    """The [Function] names from a Symbols.conf (for the #undef list when we generate Desc.h)."""
    funcs, section = [], None
    for line in Path(path).read_text().splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        if s.startswith("[") and s.endswith("]"):
            section = s[1:-1].strip().lower()
            continue
        if section == "function":
            funcs.append(s)
    return funcs


def write_intermediates(gendir, args, funcs):
    """Lay Desc.h / Symbols.conf / Manifest_{host,guest}.cpp into gendir. Each is copied verbatim from
    the matching --desc/--symbols/--manifest-* override if given, otherwise generated from funcs."""
    gendir.mkdir(parents=True, exist_ok=True)

    def place(name, override, generate):
        dst = gendir / name
        if override:
            src = Path(override)
            if not src.exists():
                die(f"{name} source not found: {src}")
            shutil.copyfile(src, dst)
        else:
            dst.write_text(generate())

    def gen_desc():
        desc = ["#pragma once", ""]
        for h in args.header:
            desc.append(f"#include <{h}>")
        desc.append("")
        desc.append("// Undo any function-like macros the headers define over the names we thunk (zlib's")
        desc.append("// gzgetc is one). #undef of a non-macro is a harmless no-op, so this is always safe,")
        desc.append("// and it lets the generated thunk redeclare each name as a real function.")
        for f in funcs:
            desc.append(f"#undef {f}")
        desc += ["",
                 "#include <lorelei/ThunkInterface/Proc.h>",
                 "#include <lorelei/ThunkInterface/PassTags.h>",
                 "",
                 "namespace lore::thunk {}",
                 ""]
        return "\n".join(desc)

    def gen_symbols():
        return "[Function]\n" + "\n".join(funcs) + "\n"

    def gen_manifest_host():
        host = []
        if args.callback_replace:
            host.append("#define LORE_THUNK_CALLBACK_REPLACE")
        if args.auto_link:
            host.append("#define LORE_THUNK_AUTO_LINK")
        host += ["",
                 '#include "Desc.h"',
                 "#include <lorelei/ThunkInterface/ManifestHost.cpp.inc>",
                 "",
                 "namespace lore::thunk {}",
                 ""]
        return "\n".join(host)

    def gen_manifest_guest():
        guest = []
        if args.callback_replace:
            guest.append("#define LORE_THUNK_CALLBACK_REPLACE")
        guest += ["",
                  '#include "Desc.h"',
                  "#include <lorelei/ThunkInterface/ManifestGuest.cpp.inc>",
                  "",
                  "namespace lore::thunk {}",
                  ""]
        return "\n".join(guest)

    place("Desc.h", args.desc, gen_desc)
    place("Symbols.conf", args.symbols, gen_symbols)
    place("Manifest_host.cpp", args.manifest_host, gen_manifest_host)
    place("Manifest_guest.cpp", args.manifest_guest, gen_manifest_guest)


class HelpFormatter(argparse.RawDescriptionHelpFormatter):
    """Keep the epilog's hand-wrapped layout, and mark a repeatable option by appending ` ...` to its
    argument (so both the usage line and the option list show, e.g., `--header HEADER ...`)."""

    def _format_args(self, action, default_metavar):
        text = super()._format_args(action, default_metavar)
        if isinstance(action, argparse._AppendAction):
            text += " ..."
        return text


def main():
    ap = argparse.ArgumentParser(
        prog="LoreMakeThunk.py",
        formatter_class=HelpFormatter,
        fromfile_prefix_chars="@",
        description="Generate a Lorelei guest+host thunk for one library from a devkit.",
        epilog="Notes:\n"
               "  - An option shown with a trailing ` ...` may be given more than once.\n"
               "  - Flags for parsing the headers go after `--`, clang-tooling style, for example\n"
               "    `-- -I/usr/include -DFOO -fno-strict-aliasing`. They are forwarded to the TLC\n"
               "    parse and to the thunk compile.\n"
               "  - @FILE reads arguments, one per line, from FILE.")

    g_req = ap.add_argument_group("required")
    g_req.add_argument("--name", required=True,
                       help="thunk base name; the guest thunk becomes lib<name>.so (e.g. 'z' for zlib)")
    g_req.add_argument("-o", "--out", required=True,
                       help="output prefix (a LORELEI_THUNK_PATH prefix)")

    g_api = ap.add_argument_group(
        "library and API",
        "What to thunk. Give the symbol list as --lib or --symbols, and the API as --header or --desc.")
    g_api.add_argument("--lib",
                       help="the real shared library: its exported functions are thunked, its SONAME "
                            "reused, and (with --auto-link) the host thunk links against it")
    g_api.add_argument("--header", action="append", default=[], metavar="HEADER",
                       help="a header that declares the API, as written in an #include. Not needed "
                            "when --desc is given, which supplies its own #includes")

    g_ovr = ap.add_argument_group(
        "intermediate overrides",
        "Supply one of the files LoreMakeThunk otherwise generates. Anything omitted is generated.")
    g_ovr.add_argument("--desc", metavar="FILE",
                       help="a Desc.h to use verbatim (its own #includes and any pass:: descriptors "
                            "for printf-style functions). When given, --header is ignored")
    g_ovr.add_argument("--symbols", metavar="FILE",
                       help="a Symbols.conf listing the functions to thunk, instead of dumping them "
                            "from --lib with nm")
    g_ovr.add_argument("--manifest-host", dest="manifest_host", metavar="FILE",
                       help="a Manifest_host.cpp to use instead of the generated one")
    g_ovr.add_argument("--manifest-guest", dest="manifest_guest", metavar="FILE",
                       help="a Manifest_guest.cpp to use instead of the generated one")

    g_tune = ap.add_argument_group("build tuning")
    g_tune.add_argument("--soname",
                        help="override the guest thunk SONAME (default: the SONAME of --lib, else "
                             "lib<name>.so)")
    g_tune.add_argument("--gtl-alias", dest="gtl_alias", action="append", default=[], metavar="NAME",
                        help="extra symlink in the guest thunk dir pointing at lib<name>.so, e.g. "
                             "--gtl-alias libz.so.1 (on top of the SONAME alias from --lib)")
    g_tune.add_argument("--htl-alias", dest="htl_alias", action="append", default=[], metavar="NAME",
                        help="extra symlink in the host thunk dir pointing at lib<name>_HTL.so "
                             "(rarely needed; the runtime finds the HTL via LORELEI_THUNK_PATH)")
    g_tune.add_argument("--nm", help="nm command for dumping the library's symbols "
                                     "(default: the devkit's llvm-nm, else nm on PATH)")
    g_tune.add_argument("--htl-arg", action="append", default=[], metavar="FLAG",
                        help="extra flag for the host thunk only (its parse + compile), e.g. "
                             "--htl-arg=-lfoo (use = for flags starting with -)")
    g_tune.add_argument("--gtl-arg", action="append", default=[], metavar="FLAG",
                        help="extra flag for the guest thunk only (its parse + compile)")
    g_tune.add_argument("--no-callback-replace", dest="callback_replace", action="store_false",
                        help="do not thunk function-pointer callbacks (default: do)")
    g_tune.add_argument("--no-auto-link", dest="auto_link", action="store_false",
                        help="do not link the host thunk against the real library (default: do)")

    g_misc = ap.add_argument_group("devkit and misc")
    g_misc.add_argument("--devkit",
                        help="unpacked lorelei devkit prefix (default: $LORELEI_DEVKIT, or the devkit "
                             "this script is installed in)")
    g_misc.add_argument("--keep-intermediates", action="store_true",
                        help="keep the generated Desc.h/Symbols.conf/Manifest/ThunkStat.json/*.cpp")
    g_misc.add_argument("-n", "--dry-run", action="store_true",
                        help="print the LoreTLC and compiler commands without running them")

    ap.add_argument("compile_args", nargs="*", metavar="-- COMPILE_ARGS",
                    help="compile arguments after --, forwarded to the header parse and the compile")
    args = ap.parse_args()

    global DRY_RUN
    DRY_RUN = args.dry_run

    if not args.lib and not args.symbols:
        die("need the function list: pass --lib (dumped with nm) or --symbols (a Symbols.conf)")
    if not args.desc and not args.header:
        die("need the API: pass --header (repeatable) or --desc (a Desc.h that #includes it)")
    if args.desc and args.header:
        print("note: --desc given, --header is ignored")

    dk = Devkit(resolve_devkit(args.devkit))
    if args.nm:
        dk.nm = args.nm
    lib = Path(args.lib) if args.lib else None
    if lib and not lib.exists():
        die(f"--lib not found: {lib}")
    if args.auto_link and not lib:
        print("note: no --lib to link against, building the host thunk without --auto-link")
        args.auto_link = False

    soname = args.soname or (read_soname(dk, lib) if lib else None) or f"lib{args.name}.so"
    cflags = args.compile_args

    out = Path(args.out).resolve()
    gendir = out / ".gen" / args.name
    htl_dir = out / "lib" / f"{dk.host_arch}-LoreHTL"
    gtl_dir = out / GUEST_ARCH / "lib" / f"{GUEST_ARCH}-LoreGTL"
    if not DRY_RUN:
        htl_dir.mkdir(parents=True, exist_ok=True)
        gtl_dir.mkdir(parents=True, exist_ok=True)

    print(f"[1/5] host arch={dk.host_arch}  soname={soname}  functions:", end=" ")
    funcs = functions_from_symbols(args.symbols) if args.symbols else dump_functions(dk, lib)
    print(len(funcs))
    if not funcs:
        die("no functions to thunk (empty symbol list)")

    print("[2/5] writing intermediates + running TLC stat")
    if not DRY_RUN:
        write_intermediates(gendir, args, funcs)
    stat = gendir / "ThunkStat.json"
    # Reference the generated intermediates by absolute path and do not chdir into gendir, so any
    # relative path in the user's compile args (e.g. `-- -I.`) still resolves against the directory
    # LoreMakeThunk was invoked from, not gendir.
    run([dk.tlc, "stat", "-o", stat, "-c", gendir / "Symbols.conf", gendir / "Desc.h",
         "--", "-xc++", "-std=gnu++20", f"-I{dk.host_include}", *cflags])

    print("[3/5] TLC generate (host + guest)")
    htl_src = gendir / "Thunk_host.cpp"
    gtl_src = gendir / "Thunk_guest.cpp"
    run([dk.tlc, "generate", "-o", htl_src, "-s", stat, "-m", "host", gendir / "Manifest_host.cpp",
         "--", "-xc++", "-std=gnu++20", "-target", dk.host_triplet,
         f"-I{dk.host_include}", f"-I{gendir}", *cflags, *args.htl_arg])
    run([dk.tlc, "generate", "-o", gtl_src, "-s", stat, "-m", "guest", gendir / "Manifest_guest.cpp",
         "--", "-xc++", "-std=gnu++20", "-target", GUEST_TRIPLET,
         f"--sysroot={dk.guest_sysroot}", f"-I{dk.guest_include}", f"-I{gendir}", *cflags, *args.gtl_arg])

    print("[4/5] compile host thunk (HTL)")
    htl_out = htl_dir / f"lib{args.name}_HTL.so"
    htl_cmd = [dk.host_cxx, "-shared", *TU_FLAGS, f"-I{dk.host_include}", f"-I{gendir}", *cflags,
               str(htl_src), "-o", str(htl_out),
               f"-L{dk.host_libdir}", "-lLoreHostRT", f"-Wl,-rpath,{RPATH}"]
    if args.auto_link:
        htl_cmd.append(str(lib))          # link the real library so its symbols resolve
    htl_cmd += args.htl_arg
    run(htl_cmd)

    print("[5/5] compile guest thunk (GTL)")
    gtl_out = gtl_dir / f"lib{args.name}.so"
    gtl_cmd = [*dk.guest_compile_cmd(), "-shared", *TU_FLAGS,
               f"-I{dk.guest_include}", f"-I{gendir}", *cflags,
               str(gtl_src), "-o", str(gtl_out),
               f"-L{dk.guest_libdir}", "-lLoreGuestRT",
               f"-Wl,-soname,{soname}", f"-Wl,-rpath,{RPATH}"]
    gtl_cmd += args.gtl_arg
    run(gtl_cmd)

    if DRY_RUN:
        print("\n(dry run: the commands above were not executed, nothing was written)")
        return

    # Guest thunk symlinks: the SONAME alias derived from --lib (so a guest linking -l<name> resolves
    # its NEEDED to the file), plus any explicit --gtl-alias. Host thunk symlinks: only --htl-alias.
    made_gtl = make_aliases(gtl_dir, f"lib{args.name}.so", [soname, *args.gtl_alias])
    made_htl = make_aliases(htl_dir, f"lib{args.name}_HTL.so", args.htl_alias)

    if not args.keep_intermediates:
        shutil.rmtree(out / ".gen", ignore_errors=True)

    print("\ndone. thunk-pack prefix:", out)
    print("  HTL:", htl_out, f"(+ {', '.join(made_htl)})" if made_htl else "")
    print("  GTL:", gtl_out, f"(+ {', '.join(made_gtl)})" if made_gtl else "")
    print(f"\nrun a guest over it with:  LORELEI_THUNK_PATH={out}")


if __name__ == "__main__":
    main()
