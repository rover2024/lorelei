# One-click test image for lorelei v2. It builds the patched QEMU and its dlcall plugin, qmsetup,
# this repository, and the lorelei-thunks zlib thunk, then runs the end-to-end test (see
# docker/scripts/run-tests.sh). It targets an x86_64 host, where the guest and host ISA are the same
# and a single native toolchain builds everything.
FROM ubuntu:24.04

ARG http_proxy
ARG https_proxy
ARG HTTP_PROXY
ARG HTTPS_PROXY
ARG no_proxy
ARG NO_PROXY
ARG USE_USTC_MIRROR=0

SHELL ["/bin/bash", "-lc"]
ENV USE_USTC_MIRROR=${USE_USTC_MIRROR}

# Install the toolchain and create the build user. Only the bootstrap + external-dependency scripts
# are copied here, so this layer (the slow apt install) and the QEMU build below stay cached even
# when the lorelei/thunks build scripts change; those are run from the repo copy further down.
COPY docker/scripts/bootstrap-ubuntu.sh docker/scripts/build-qmsetup.sh docker/scripts/build-qemu.sh /tmp/scripts/
# Strip any CR line endings (a Windows checkout may carry CRLF), otherwise the shebang breaks.
RUN sed -i 's/\r$//' /tmp/scripts/*.sh && chmod +x /tmp/scripts/*.sh && /tmp/scripts/bootstrap-ubuntu.sh

ENV REPOS_DIR=/home/user
ENV INSTALL_DIR=/home/user/install
ENV LORELEI_SRC=/home/user/lorelei
ENV QEMU_BUILD_DIR=${REPOS_DIR}/qemu/build/release

RUN mkdir -p ${INSTALL_DIR} && chown -R user:user /home/user

USER user
WORKDIR /home/user

# Build the external dependencies first, so they stay cached across edits to this repository.
RUN /tmp/scripts/build-qmsetup.sh && /tmp/scripts/build-qemu.sh

# Copy this repository (lorelei v2) and build it from local source, then the thunks against it. These
# scripts are run from the repo copy, so editing them only rebuilds from here on.
COPY --chown=user:user . ${LORELEI_SRC}
# Same CRLF guard for the in-repo scripts (build-* here, and run-tests.sh at CMD time).
RUN sed -i 's/\r$//' ${LORELEI_SRC}/docker/scripts/*.sh \
    && bash ${LORELEI_SRC}/docker/scripts/build-lorelei.sh \
    && bash ${LORELEI_SRC}/docker/scripts/build-thunks.sh

WORKDIR ${LORELEI_SRC}
CMD ["/bin/bash"]
