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

Minimal usage:
  LoreMakeThunk.py --devkit <prefix> --name z --library /usr/lib/x86_64-linux-gnu/libz.so.1 \
               --header zlib.h --include-dir /usr/include -o ./out
"""
import argparse
import re
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


def run(cmd, **kw):
    print("  $ " + " ".join(str(c) for c in cmd))
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


def write_intermediates(gendir, args, funcs):
    gendir.mkdir(parents=True, exist_ok=True)

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
    (gendir / "Desc.h").write_text("\n".join(desc))

    (gendir / "Symbols.conf").write_text("[Function]\n" + "\n".join(funcs) + "\n")

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
    (gendir / "Manifest_host.cpp").write_text("\n".join(host))

    guest = []
    if args.callback_replace:
        guest.append("#define LORE_THUNK_CALLBACK_REPLACE")
    guest += ["",
              '#include "Desc.h"',
              "#include <lorelei/ThunkInterface/ManifestGuest.cpp.inc>",
              "",
              "namespace lore::thunk {}",
              ""]
    (gendir / "Manifest_guest.cpp").write_text("\n".join(guest))


def main():
    ap = argparse.ArgumentParser(
        prog="LoreMakeThunk.py",
        description="Generate a Lorelei guest+host thunk for one library from a devkit, using only "
                    "the devkit's LoreTLC and clang (no cmake/make/git).")
    ap.add_argument("--devkit", required=True, help="unpacked lorelei devkit prefix for this host")
    ap.add_argument("--name", required=True,
                    help="thunk base name; the guest thunk becomes lib<name>.so (e.g. 'z' for zlib)")
    ap.add_argument("--library", required=True,
                    help="the real shared library: its exported functions are thunked, its SONAME "
                         "reused, and (with --auto-link) the host thunk links against it")
    ap.add_argument("--header", action="append", required=True, metavar="HEADER",
                    help="a header that declares the API, as written in an #include (repeatable)")
    ap.add_argument("--include-dir", action="append", default=[], metavar="DIR",
                    help="an include search dir for the headers above (repeatable)")
    ap.add_argument("-o", "--out", required=True, help="output prefix (a LORELEI_THUNK_PATH prefix)")
    ap.add_argument("--soname", help="override the guest thunk SONAME (default: read from --library)")
    ap.add_argument("--no-callback-replace", dest="callback_replace", action="store_false",
                    help="do not thunk function-pointer callbacks (default: do)")
    ap.add_argument("--no-auto-link", dest="auto_link", action="store_false",
                    help="do not link the host thunk against the real library (default: do)")
    ap.add_argument("--keep-intermediates", action="store_true",
                    help="keep the generated Desc.h/Symbols.conf/Manifest/ThunkStat.json/*.cpp")
    args = ap.parse_args()

    dk = Devkit(args.devkit)
    lib = Path(args.library)
    if not lib.exists():
        die(f"--library not found: {lib}")

    soname = args.soname or read_soname(dk, lib) or f"lib{args.name}.so"
    incflags = [f"-I{d}" for d in args.include_dir]

    out = Path(args.out).resolve()
    gendir = out / ".gen" / args.name
    htl_dir = out / "lib" / f"{dk.host_arch}-LoreHTL"
    gtl_dir = out / GUEST_ARCH / "lib" / f"{GUEST_ARCH}-LoreGTL"
    htl_dir.mkdir(parents=True, exist_ok=True)
    gtl_dir.mkdir(parents=True, exist_ok=True)

    print(f"[1/5] host arch={dk.host_arch}  soname={soname}  functions:", end=" ")
    funcs = dump_functions(dk, lib)
    print(len(funcs))
    if not funcs:
        die(f"no exported functions found in {lib}")

    print("[2/5] writing intermediates + running TLC stat")
    write_intermediates(gendir, args, funcs)
    stat = gendir / "ThunkStat.json"
    run([dk.tlc, "stat", "-o", stat, "-c", "Symbols.conf", "Desc.h",
         "--", "-xc++", "-std=gnu++20", f"-I{dk.host_include}", *incflags],
        cwd=gendir)

    print("[3/5] TLC generate (host + guest)")
    htl_src = gendir / "Thunk_host.cpp"
    gtl_src = gendir / "Thunk_guest.cpp"
    run([dk.tlc, "generate", "-o", htl_src, "-s", stat, "-m", "host", "Manifest_host.cpp",
         "--", "-xc++", "-std=gnu++20", "-target", dk.host_triplet,
         f"-I{dk.host_include}", f"-I{gendir}", *incflags],
        cwd=gendir)
    run([dk.tlc, "generate", "-o", gtl_src, "-s", stat, "-m", "guest", "Manifest_guest.cpp",
         "--", "-xc++", "-std=gnu++20", "-target", GUEST_TRIPLET,
         f"--sysroot={dk.guest_sysroot}", f"-I{dk.guest_include}", f"-I{gendir}", *incflags],
        cwd=gendir)

    print("[4/5] compile host thunk (HTL)")
    htl_out = htl_dir / f"lib{args.name}_HTL.so"
    htl_cmd = [dk.host_cxx, "-shared", *TU_FLAGS, f"-I{dk.host_include}", f"-I{gendir}", *incflags,
               str(htl_src), "-o", str(htl_out),
               f"-L{dk.host_libdir}", "-lLoreHostRT", f"-Wl,-rpath,{RPATH}"]
    if args.auto_link:
        htl_cmd.append(str(lib))          # link the real library so its symbols resolve
    run(htl_cmd)

    print("[5/5] compile guest thunk (GTL)")
    gtl_out = gtl_dir / f"lib{args.name}.so"
    gtl_cmd = [*dk.guest_compile_cmd(), "-shared", *TU_FLAGS,
               f"-I{dk.guest_include}", f"-I{gendir}", *incflags,
               str(gtl_src), "-o", str(gtl_out),
               f"-L{dk.guest_libdir}", "-lLoreGuestRT",
               f"-Wl,-soname,{soname}", f"-Wl,-rpath,{RPATH}"]
    run(gtl_cmd)
    if soname != f"lib{args.name}.so":
        link = gtl_dir / soname
        if link.exists() or link.is_symlink():
            link.unlink()
        link.symlink_to(f"lib{args.name}.so")

    if not args.keep_intermediates:
        import shutil
        shutil.rmtree(out / ".gen", ignore_errors=True)

    print("\ndone. thunk-pack prefix:", out)
    print("  HTL:", htl_out)
    print("  GTL:", gtl_out, f"(+ {soname})" if soname != f"lib{args.name}.so" else "")
    print(f"\nrun a guest over it with:  LORELEI_THUNK_PATH={out}")


if __name__ == "__main__":
    main()
