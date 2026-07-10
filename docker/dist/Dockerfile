# Builds the lorelei distribution (runtime + devkit tarballs) for one or more target arches, all on an
# x86_64 host: the native arch natively and aarch64/riscv64 by cross compilation (no qemu). See
# docker/dist/scripts/build-dist.sh. Select arches with --build-arg DIST_TARGETS="x86_64 aarch64 riscv64".
#
# Build + extract (plain docker build, no buildx):  docker/dist/build.sh
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
ENV THUNKS_SRC=/home/build/lorelei-thunks
ENV OUT_DIR=/home/build/out

# Shared build dependencies (native clang/LLVM, cmake/ninja). Cached across source edits.
# fetch-llvm.sh rides along: bootstrap-dist.sh calls it to pull the self-contained native LLVM.
COPY docker/dist/scripts/bootstrap-dist.sh docker/dist/scripts/fetch-llvm.sh /tmp/scripts/
RUN sed -i 's/\r$//' /tmp/scripts/*.sh && chmod +x /tmp/scripts/*.sh && /tmp/scripts/bootstrap-dist.sh

RUN mkdir -p ${REPOS_DIR} ${OUT_DIR}

# The lorelei-thunks source (the distribution ships the stable thunks).
ARG THUNKS_BRANCH=main
RUN curl -fsSL "https://codeload.github.com/rover2024/lorelei-thunks/tar.gz/refs/heads/${THUNKS_BRANCH}" | tar xz -C /home/build \
    && mv /home/build/lorelei-thunks-${THUNKS_BRANCH} ${THUNKS_SRC}

# The lorelei v2 source, then build the distribution from it.
COPY . ${LORELEI_SRC}
ARG DIST_TARGETS="x86_64"
RUN sed -i 's/\r$//' ${LORELEI_SRC}/docker/dist/scripts/*.sh ${LORELEI_SRC}/docker/common/*.sh \
    && for t in ${DIST_TARGETS}; do bash ${LORELEI_SRC}/docker/dist/scripts/build-dist.sh "$t"; done

# Export stage: only the tarballs.
FROM scratch AS export
COPY --from=build /home/build/out/ /
