#!/usr/bin/env python3
"""
LoreMakeThunk.py: generate a Lorelei thunk (guest GTL + host HTL) for one library, using only
LoreTLC and a C++ compiler. No cmake, no make, no git, no manifest file.

Where those live is named by a MakeThunkConfig.json rather than assumed, so the same script drives
an unpacked devkit (which ships its own config at share/lorelei/MakeThunkConfig.json) or any other
tree that can point at a LoreTLC, the lorelei headers and the two runtimes.

Given a real shared library and the headers that declare its API, it:
  1. dumps the library's exported functions (nm) into a Symbols list,
  2. runs the devkit's LoreTLC to stat + generate the host and guest thunk sources,
  3. compiles each into a shared library with the devkit's clang,
laying the two out in the standard thunk-pack layout, so the output directory is itself a ready
thunk pack (the host runtime finds it from the guest thunk's own location at run time):

  <out>/lib<name>_HTL.so                     (host thunk, this host's arch)
  <out>/x86_64/lib<name>.so   (+ soname symlink)   (guest thunk, x86_64)

The guest thunk carries the relative path to its host thunk (LORE_THUNK_NEXT_LIBRARY), so the
guest runtime loads the host thunk directly and the run needs only -E LD_LIBRARY_PATH=<out>/x86_64.

Usage (the config comes from --config, else from the devkit given by --devkit, $LORELEI_DEVKIT or
this script's own install location. Header flags follow --, clang-tooling style):
  LoreMakeThunk.py [--devkit <dir> | --config <file>] --name <name> --lib <lib.so> --header <hdr> -o <out> [-- <compile arguments>]

The four intermediates it normally generates (Desc.h, Symbols.conf, Manifest_host.cpp,
Manifest_guest.cpp) can each be supplied instead, with --desc / --symbols / --manifest-host /
--manifest-guest. Anything not supplied is still generated. A supplied --desc brings its own #includes,
so --header is then unused, and the symbol list comes from either --symbols or --lib.

Example:
  LoreMakeThunk.py --name z --lib /usr/lib/x86_64-linux-gnu/libz.so.1 --header zlib.h \
               -o ./out -- -I/usr/include
"""
import argparse
import json
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


def varexp(s, find, _depth=0):
    """Expand ${name} references in s, resolving each through find(name). Nesting is allowed and
    $$ is a literal $, matching lore::str::varexp (Support/StringExtras.h) so this config uses the
    same syntax as the ThunkDB.json sitting next to it. Unlike the C++ one, an undefined name is an
    error rather than an empty string: silently emptying a path turns a typo into a puzzling
    "not found" much later."""
    if _depth > 16:
        die(f"variable expansion too deep (a cycle?): {s}")
    out = []
    i = 0
    while i < len(s):
        if s[i] == "$" and i + 1 < len(s):
            if s[i + 1] == "{":
                depth, j = 1, i + 2
                while j < len(s) and depth:
                    if s[j] == "$" and j + 1 < len(s) and s[j + 1] == "{":
                        depth += 1
                        j += 2
                        continue
                    if s[j] == "}":
                        depth -= 1
                    j += 1
                if depth:
                    die(f"unterminated ${{ in {s!r}")
                name = s[i + 2:j - 1]
                if "${" in name:  # nested: resolve the inner reference first
                    name = varexp(name, find, _depth + 1)
                out.append(find(name))
                i = j
                continue
            if s[i + 1] == "$":  # $$ escapes a literal dollar
                out.append("$")
                i += 2
                continue
        out.append(s[i])
        i += 1
    return "".join(out)


class Config:
    """A MakeThunkConfig.json, with its $vars expanded.

    The reserved "$vars" object names values every other string may reference as ${name}; the vars
    may reference each other and the built-in ${configDir} (the directory holding the config), which
    is what keeps a devkit relocatable: it writes ../.. once in $vars rather than in every path.
    """

    def __init__(self, path):
        self.path = Path(path).resolve()
        try:
            doc = json.loads(self.path.read_text())
        except FileNotFoundError:
            die(f"config not found: {self.path}")
        except json.JSONDecodeError as e:
            die(f"config is not valid JSON ({self.path}): {e}")
        if not isinstance(doc, dict):
            die(f"config must be a JSON object: {self.path}")

        raw_vars = doc.pop("$vars", {})
        if not isinstance(raw_vars, dict):
            die(f'"$vars" must be an object: {self.path}')
        self._raw_vars = raw_vars
        self._vars = {"configDir": str(self.path.parent)}
        self._doc = doc

    def _lookup(self, name, _seen=()):
        if name in self._vars:
            return self._vars[name]
        if name not in self._raw_vars:
            die(f"undefined variable ${{{name}}} in {self.path}")
        if name in _seen:
            die(f"variable cycle: ${{{name}}} in {self.path}")
        value = self._raw_vars[name]
        if not isinstance(value, str):
            die(f'"$vars.{name}" must be a string: {self.path}')
        resolved = varexp(value, lambda n: self._lookup(n, _seen + (name,)))
        # A $vars entry names a location, so collapse the . and .. a ${configDir}-relative one picks
        # up; every flag built from it then reads as a plain path. Harmless on a non-path value,
        # which normpath returns unchanged.
        if resolved:
            resolved = os.path.normpath(resolved)
        self._vars[name] = resolved
        return resolved

    def _expand(self, value):
        return varexp(value, self._lookup)

    def str_(self, section, key, default=None):
        """A plain string entry (expanded). Returns default when absent or null."""
        obj = self._doc.get(section) or {}
        if not isinstance(obj, dict):
            die(f'"{section}" must be an object: {self.path}')
        value = obj.get(key)
        if value is None:
            return default
        if not isinstance(value, str):
            die(f'"{section}.{key}" must be a string or null: {self.path}')
        return self._expand(value)

    def path_(self, section, key, what, required=True):
        """A path entry (expanded and normalised). Missing files are reported against the config."""
        value = self.str_(section, key)
        if value is None:
            if required:
                die(f'"{section}.{key}" ({what}) is missing from {self.path}')
            return None
        resolved = Path(os.path.normpath(os.path.join(str(self.path.parent), value)))
        if not resolved.exists():
            die(f"{what} not found: {resolved}\n  (from \"{section}.{key}\" in {self.path})")
        return resolved

    def flags(self, section, key):
        """A list-of-strings entry (each expanded). Absent means no flags. A flag is opaque, so any
        path inside one must come from ${configDir} or a $vars entry rather than be written relative
        to the config."""
        obj = self._doc.get(section) or {}
        value = obj.get(key)
        if value is None:
            return []
        if not isinstance(value, list) or not all(isinstance(x, str) for x in value):
            die(f'"{section}.{key}" must be a list of strings: {self.path}')
        return [self._expand(x) for x in value]


CONFIG_RELPATH = Path("share") / "lorelei" / "MakeThunkConfig.json"


def resolve_config(config_arg, devkit_arg):
    """--config, else the config of the devkit named by --devkit / $LORELEI_DEVKIT / this script's
    own install location. Everything funnels into one config-driven path."""
    if config_arg:
        return Path(config_arg)
    prefix = devkit_arg or os.environ.get("LORELEI_DEVKIT")
    if not prefix:
        # Installed at <devkit>/bin/LoreMakeThunk.py, so the prefix is two levels up.
        cand = Path(__file__).resolve().parent.parent
        if (cand / CONFIG_RELPATH).exists():
            prefix = cand
    if not prefix:
        die("no config: pass --config, or --devkit / $LORELEI_DEVKIT / install this script in a "
            "devkit's bin/")
    return Path(prefix) / CONFIG_RELPATH


class Toolkit:
    """The tools, directories and flags a thunk build needs, as named by a MakeThunkConfig.json.

    Every field below is exactly one config entry: this class holds no knowledge of any directory
    layout, so a devkit and a plain build tree differ only in the config that names them.
    """

    def __init__(self, cfg):
        self.config = cfg

        self.tlc = cfg.path_("tools", "tlc", "LoreTLC")
        self.host_cxx = cfg.path_("tools", "host_cxx", "host C++ compiler")
        # Guest C++ compiler. A wrapper with the target and sysroot baked in, else the host compiler
        # driven at the guest triplet by guest.cxx_flags.
        self.guest_cxx = cfg.path_("tools", "guest_cxx", "guest C++ compiler", required=False)
        self.nm = cfg.path_("tools", "nm", "nm", required=False) or "nm"
        self.readelf = cfg.path_("tools", "readelf", "readelf", required=False) or "readelf"

        self.host_include = cfg.path_("host", "include", "host include dir")
        self.host_libdir = cfg.path_("host", "libdir", "host lib dir")
        self.guest_include = cfg.path_("guest", "include", "guest include dir")
        self.guest_libdir = cfg.path_("guest", "libdir", "guest lib dir")

        # Flag lists rather than single paths, because what makes the headers of a side reachable is
        # not always a sysroot: a cross build tree reaches the guest ones through --gcc-toolchain /
        # -idirafter. The TLC parse is always clang while the compiler is whatever tools names, so
        # each side may separate the two; parse_flags defaults to cxx_flags, which is all a devkit
        # (clang on both) needs.
        self.host_cxx_flags = cfg.flags("host", "cxx_flags")
        self.host_link_flags = cfg.flags("host", "link_flags")
        self.host_parse_flags = cfg.flags("host", "parse_flags") or self.host_cxx_flags
        self.guest_cxx_flags = cfg.flags("guest", "cxx_flags")
        self.guest_link_flags = cfg.flags("guest", "link_flags")
        self.guest_parse_flags = cfg.flags("guest", "parse_flags") or self.guest_cxx_flags

        self.guest_triplet = cfg.str_("guest", "triplet", GUEST_TRIPLET)

        # Sanity: the manifest fragments the generated sources #include must be reachable.
        if not (self.host_include / "lorelei" / "ThunkInterface" / "ManifestHost.cpp.inc").exists():
            die(f"ThunkInterface headers not found under {self.host_include} "
                f'(is "host.include" in {cfg.path} right?)')

        self.host_arch = self._detect_host_arch()
        self.host_triplet = cfg.str_("host", "triplet") or HOST_TRIPLETS[self.host_arch]

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
        """The base argv for compiling the guest thunk."""
        if self.guest_cxx:
            return [self.guest_cxx, *self.guest_cxx_flags]
        return [self.host_cxx, "-target", self.guest_triplet, *self.guest_cxx_flags]


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
        # TLC parses Desc.h as C++, so a C library's declarations need C linkage or the guest
        # thunk exports mangled names and the drop-in fails to resolve them. Give it here (the
        # default) so the header itself needs no extern "C" guards. Only the library headers are
        # wrapped, never the lore ThunkInterface headers below, which are genuinely C++.
        c_linkage = args.lang == "c"
        if c_linkage:
            desc.append('extern "C" {')
        for h in args.header:
            desc.append(f"#include <{h}>")
        if c_linkage:
            desc.append("}")
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
                       help="output prefix (a self-contained thunk pack)")

    g_api = ap.add_argument_group(
        "library and API",
        "What to thunk. Give the symbol list as --lib or --symbols, and the API as --header or --desc.")
    g_api.add_argument("--lib",
                       help="the real shared library: its exported functions are thunked, its SONAME "
                            "reused, and (with --auto-link) the host thunk links against it")
    g_api.add_argument("--header", action="append", default=[], metavar="HEADER",
                       help="a header that declares the API, as written in an #include. Not needed "
                            "when --desc is given, which supplies its own #includes")
    g_api.add_argument("--lang", choices=["c", "c++"], default="c",
                       help='linkage of the --header files. c (default) wraps them in extern "C" '
                            "in the generated Desc.h, so a plain C header needs no guards; pass "
                            "c++ for a genuinely C++ library. Ignored with --desc")

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
                             "(rarely needed; the runtime derives the HTL from the pack layout)")
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

    g_misc = ap.add_argument_group("toolkit and misc")
    g_misc.add_argument("--config", metavar="FILE",
                        help="a MakeThunkConfig.json naming the tools, directories and flags to "
                             "build with (default: the one in the devkit below). Lets a plain build "
                             "tree stand in for a devkit")
    g_misc.add_argument("--devkit",
                        help="unpacked lorelei devkit prefix, i.e. read its "
                             "share/lorelei/MakeThunkConfig.json (default: $LORELEI_DEVKIT, or the "
                             "devkit this script is installed in)")
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

    dk = Toolkit(Config(resolve_config(args.config, args.devkit)))
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
    htl_dir = out
    gtl_dir = out / GUEST_ARCH
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
         "--", "-xc++", "-std=gnu++20", *dk.host_parse_flags, f"-I{dk.host_include}", *cflags])

    print("[3/5] TLC generate (host + guest)")
    htl_src = gendir / "Thunk_host.cpp"
    gtl_src = gendir / "Thunk_guest.cpp"
    run([dk.tlc, "generate", "-o", htl_src, "-s", stat, "-m", "host", gendir / "Manifest_host.cpp",
         "--", "-xc++", "-std=gnu++20", "-target", dk.host_triplet, *dk.host_parse_flags,
         f"-I{dk.host_include}", f"-I{gendir}", *cflags, *args.htl_arg])
    run([dk.tlc, "generate", "-o", gtl_src, "-s", stat, "-m", "guest", gendir / "Manifest_guest.cpp",
         "--", "-xc++", "-std=gnu++20", "-target", dk.guest_triplet, *dk.guest_parse_flags,
         f"-I{dk.guest_include}", f"-I{gendir}", *cflags, *args.gtl_arg])

    print("[4/5] compile host thunk (HTL)")
    htl_out = htl_dir / f"lib{args.name}_HTL.so"
    htl_cmd = [dk.host_cxx, "-shared", *TU_FLAGS, *dk.host_cxx_flags,
               f"-I{dk.host_include}", f"-I{gendir}", *cflags,
               str(htl_src), "-o", str(htl_out),
               f"-L{dk.host_libdir}", "-lLoreHostRT"]
    # e.g. the devkit's build-only libstdc++/libgcc, for clang++'s implicit -lstdc++/-lgcc_s
    htl_cmd += dk.host_link_flags
    if args.auto_link:
        # Link the real library so its symbols resolve. Reference it by name (-l:), so the NEEDED entry
        # is its SONAME (or bare filename), found via LD_LIBRARY_PATH at run time, rather than the path
        # we were given (lld would otherwise bake that path in for a library that carries no SONAME).
        htl_cmd += [f"-L{lib.parent}", f"-l:{lib.name}"]
    else:
        # Not linked in: bake the real library's name so the host thunk dlopens it at run time. A bare
        # name is resolved on the loader's search path, where the real library normally lives.
        htl_cmd.append(f'-DLORE_THUNK_NEXT_LIBRARY="{soname}"')
    htl_cmd += args.htl_arg
    run(htl_cmd)

    print("[5/5] compile guest thunk (GTL)")
    gtl_out = gtl_dir / f"lib{args.name}.so"
    # Bake the host thunk's path, relative to the guest thunk, so the guest runtime loads it directly
    # without the host deriving it from a fixed layout.
    htl_rel = os.path.relpath(htl_out, gtl_dir)
    gtl_cmd = [*dk.guest_compile_cmd(), "-shared", *TU_FLAGS,
               f"-I{dk.guest_include}", f"-I{gendir}", *cflags,
               f'-DLORE_THUNK_NEXT_LIBRARY="{htl_rel}"',
               str(gtl_src), "-o", str(gtl_out),
               f"-L{dk.guest_libdir}", "-lLoreGuestRT",
               *dk.guest_link_flags,
               f"-Wl,-soname,{soname}"]
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
    print(f"\nrun a guest over it by putting the GTL dir on the guest -E LD_LIBRARY_PATH:")
    print(f"  {gtl_out}")
    print("the host runtime finds the rest of the pack from there.")


if __name__ == "__main__":
    main()
