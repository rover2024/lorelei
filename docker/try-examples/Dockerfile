# A self-contained "try Lorelei" image on a bare ubuntu:24.04. It downloads and builds the patched
# qemu-x86_64 and the dlcall plugin, downloads the examples, and runs the hello and demo examples.
# The devkit itself is large and is provided at runtime by mounting an unpacked devkit prefix at
# /opt/devkit (docker run -v /path/to/devkit:/opt/devkit:ro ...); see docker/try-examples/README.md.
FROM ubuntu:24.04

ARG QEMU_TARBALL=https://codeload.github.com/rover2024/qemu/tar.gz/refs/heads/minimal-passthrough-plugin
ARG EXAMPLES_TARBALL=https://codeload.github.com/rover2024/lorelei/tar.gz/refs/heads/main

# Optional network knobs, all off/absent by default. http_proxy/https_proxy are forwarded from the host
# environment by try.sh. USE_USTC_MIRROR=1 switches apt to the USTC mirror (faster from within CN).
ARG http_proxy
ARG https_proxy
ARG USE_USTC_MIRROR=0

ENV DEBIAN_FRONTEND=noninteractive

# Everything the image needs on top of a bare ubuntu:24.04:
#  - build-essential/make/python3: build the example host libraries and run the devkit's LoreMakeThunk.py,
#    which drives the devkit's own clang directly (no cmake). The guest side is compiled with the
#    devkit's own bundled clang, so no cross toolchain is installed here.
#  - ninja-build/meson/pkg-config/libglib2.0-dev: build qemu-user and the dlcall plugin.
#  - libedit2/libffi8/libzstd1/libxml2/libtinfo6: runtime dependencies of the devkit's bundled
#    clang/LLVM, which the devkit does not ship itself.
#  - curl/ca-certificates/xz-utils: download and unpack the qemu and examples tarballs.
#  - git: not used to clone anything here, but qemu's meson build fetches one test subproject wrap
#    (berkeley-softfloat) via git, which the source tarball does not carry.
RUN if [ "$USE_USTC_MIRROR" = "1" ]; then \
        sed -i -E 's@https?://(archive|security)\.ubuntu\.com/ubuntu@http://mirrors.ustc.edu.cn/ubuntu@g; s@https?://ports\.ubuntu\.com/ubuntu-ports@http://mirrors.ustc.edu.cn/ubuntu-ports@g' \
            /etc/apt/sources.list.d/ubuntu.sources; \
    fi \
    && apt-get update && apt-get install -y --no-install-recommends \
        build-essential make python3 python3-venv \
        ninja-build meson pkg-config libglib2.0-dev \
        libedit2 libffi8 libzstd1 libxml2 libtinfo6 \
        curl ca-certificates xz-utils git \
    && rm -rf /var/lib/apt/lists/*

# Download and build the reserved_va-patched qemu-x86_64 (linux-user only) and the dlcall plugin, then
# keep just the two artifacts and drop the source tree to keep the image small.
RUN mkdir -p /opt/qemu-src \
    && curl -fsSL "${QEMU_TARBALL}" | tar -xz -C /opt/qemu-src --strip-components=1 \
    && cd /opt/qemu-src \
    && ./configure --target-list=x86_64-linux-user --enable-plugins --python=python3 \
    && make -j"$(nproc)" \
    && install -Dm755 build/qemu-x86_64 /usr/local/bin/qemu-x86_64 \
    && install -Dm644 build/contrib/plugins/libdlcall.so /opt/libdlcall.so \
    && rm -rf /opt/qemu-src

# The examples (hello, demo). LoreMakeThunk.py itself comes from the mounted devkit's bin/, so only the
# example sources are fetched here.
RUN mkdir -p /opt/lorelei \
    && curl -fsSL "${EXAMPLES_TARBALL}" \
       | tar -xz -C /opt/lorelei --strip-components=1 --wildcards '*/examples/*'

COPY run-examples.sh /usr/local/bin/run-examples
RUN chmod +x /usr/local/bin/run-examples

# The devkit is mounted at runtime at /opt/devkit; QEMU and the plugin are baked in above.
ENV LORELEI_DEVKIT=/opt/devkit \
    QEMU=/usr/local/bin/qemu-x86_64 \
    PLUGIN=/opt/libdlcall.so
CMD ["run-examples"]
