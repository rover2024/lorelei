# Try Lorelei In A Container

A one-command way to run the `hello` and `demo` thunk examples on a bare `ubuntu:24.04`, without installing anything on your host beyond Docker. The image downloads and builds the patched `qemu-x86_64` and the dlcall plugin, and downloads the examples. The one piece it does not build is the devkit (the clang or LLVM toolchain, LoreTLC, qmsetup, guest sysroot and runtimes), which is large. You unpack a devkit yourself and mount it at runtime.

## Quick Start

Grab the `lorelei-devkit-<arch>` package for your host architecture from the latest [Lorelei release](https://github.com/rover2024/lorelei/releases), unpack it somewhere, then point `try.sh` at the resulting directory:

```bash
./try.sh /path/to/lorelei-devkit-<arch>
```

It builds the image if needed and runs both examples. You should see, at the end:

```text
hello from guest: world, lucky 7
hello from host: world, lucky 7
demo_puts: Hello from the guest program
demo_printf: checking 20 + 22 = 42
demo_vprintf: checking 7
demo_qsort: 1 2 3 4 5
```

The `from host` line and the `demo_*` lines come from a native host library, called from an unmodified x86_64 guest under qemu. The `from guest` line is that same program run without the thunk.

## What It Does

1. `docker build` produces the `lorelei-try` image: it installs the build and runtime dependencies, downloads and compiles `qemu-x86_64` (linux-user only) with the dlcall plugin, and downloads the examples.
2. `docker run -v <devkit>:/opt/devkit:ro` runs [run-examples.sh](run-examples.sh), which builds and runs `examples/hello` and `examples/demo` against the mounted devkit, the built qemu, and the built plugin.

The devkit is mounted read-only, so the build writes only inside the container and your unpacked devkit is left untouched.

## Manual Invocation

If you would rather drive it yourself:

```bash
docker build -t lorelei-try docker/try-examples
docker run --rm -v /path/to/lorelei-devkit-x86_64:/opt/devkit:ro lorelei-try
```

Behind a proxy, forward it into the build so the in-container downloads can reach the network (`try.sh` does this for you from your shell environment):

```bash
docker build --build-arg http_proxy="$http_proxy" --build-arg https_proxy="$https_proxy" \
    -t lorelei-try docker/try-examples
```

From within China, `USE_USTC_MIRROR=1` switches the image's apt to the USTC mirror. `try.sh` forwards it (and any proxy) from your environment, so `USE_USTC_MIRROR=1 ./try.sh /path/to/lorelei-devkit-<arch>` is enough. With a raw `docker build`, pass `--build-arg USE_USTC_MIRROR=1`.

## Notes

- This works on any of the three supported host architectures (x86_64, aarch64, riscv64). The `ubuntu:24.04` base is multi-arch, so on each host the image builds that host's own `qemu-x86_64`; you mount the devkit for that same host. The x86_64 that stays constant is the guest, not the host.
- Each example builds into its own `build/` + `install/` tree, and `run-examples.sh` cleans before each run.
