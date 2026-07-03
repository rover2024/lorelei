# Builds a self-contained lorelei devkit (SDK) prefix and tars it. The devkit bundles clang/LLVM,
# host qmsetup + lorelei, an x86_64 guest sysroot, and the guest cross cmake file, so a thunk author's
# config names no machine path (see docker/devkit/scripts/build-devkit.sh and MakeThunk.py).
#
# It is per-host-arch. Build the arch you want to ship for:
#   docker buildx build --platform linux/amd64 -f docker/devkit/Dockerfile -o type=local,dest=out .
#   docker buildx build --platform linux/arm64 -f docker/devkit/Dockerfile -o type=local,dest=out .
# The final stage exports only the tarball, so `-o type=local,dest=out` drops it on the host.
FROM ubuntu:24.04 AS build

ARG http_proxy
ARG https_proxy
ARG HTTP_PROXY
ARG HTTPS_PROXY
ARG no_proxy
ARG NO_PROXY
ARG USE_USTC_MIRROR=0

SHELL ["/bin/bash", "-lc"]
ENV USE_USTC_MIRROR=${USE_USTC_MIRROR}

ENV REPOS_DIR=/home/build/repos
ENV LORELEI_SRC=/home/build/lorelei
ENV DEVKIT_PREFIX=/home/build/lorelei-devkit
ENV OUT_DIR=/home/build/out

# Toolchain build dependencies (clang/LLVM, cmake/ninja, amd64 apt source). Cached across source edits.
COPY docker/devkit/scripts/bootstrap-devkit.sh /tmp/scripts/
RUN sed -i 's/\r$//' /tmp/scripts/*.sh && chmod +x /tmp/scripts/*.sh && /tmp/scripts/bootstrap-devkit.sh

RUN mkdir -p ${REPOS_DIR} ${OUT_DIR}

# The lorelei v2 source, then assemble the devkit from it.
COPY . ${LORELEI_SRC}
RUN sed -i 's/\r$//' ${LORELEI_SRC}/docker/devkit/scripts/*.sh ${LORELEI_SRC}/docker/common/*.sh \
    && bash ${LORELEI_SRC}/docker/devkit/scripts/build-devkit.sh

# Export stage: only the tarball, so `-o type=local` writes just that to the host.
FROM scratch AS export
COPY --from=build /home/build/out/ /
