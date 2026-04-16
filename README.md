# LORELEI

Lorelei is a cross-ISA compatibility layer for user-level binary translators to leverage host libraries and achieve acceleration.

## Build From Source

Lorelei CMake project requires `qmsetup` for configuration, you need to build it first.

```bash
export REPOS_DIR=/home/user/repos
export INSTALL_DIR=/home/user/install

cd $REPOS_DIR
git clone --recursive https://github.com/stdware/qmsetup.git
cd qmsetup

cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/qmsetup
cmake --build build --target all
cmake --build build --target install
```

If you want to build Vulkan thunks, you need to pull Vulkan headers first.

```bash
cd $REPOS_DIR
git clone https://github.com/KhronosGroup/Vulkan-Headers.git
```

If you want to build SDL2 thunks, you need to build SDL2 (HLR-patched) first.

```bash
cd $REPOS_DIR
git clone https://github.com/rover2024/SDL.git
cd SDL
git checkout lore-hlr-patched

cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/SDL \
    -DSDL_STATIC=FALSE \
    -DSDL_SHARED=TRUE
cmake --build build --target all
cmake --build build --target install
```

### Build on x86_64

If you are building this project on an x86_64 Linux system, you only need to build once.

The thunk library generator (TLC) will be built first, and the build system will call it to generate the source files of thunks. Finally, the thunks will be built. On x86_64, TLC is a native executable and can run directly.

```bash
sudo apt install libffcall-dev libllvm-20-dev libclang-20-dev

cd $REPOS_DIR
git clone https://github.com/rover2024/lorelei.git
cd lorelei

cmake -B build -G ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/lorelei \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \
    -DLORE_BUILD_THUNKS=TRUE \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_VULKAN_INCLUDE_DIR=$REPOS_DIR/Vulkan-Headers/include
    "-DLORE_SDL2_INCLUDE_DIR=$INSTALL_DIR/SDL/include;$INSTALL_DIR/SDL/include/SDL2"
cmake --build build --target all
cmake --build build --target install
```

### Build on ARM64

<!-- If you are building this project on an ARM64 Linux system, you need to build twice - once for the host and once for the guest.

First, build for host. The thunk library source files will be generated and installed, the TLC metadata files will be installed to `/lorelei/share/lorelei/thunks` and the thunk source files will be installed to `/lorelei/src/thunks`.

```bash
sudo apt install libffcall-dev libllvm-20-dev libclang-20-dev

git clone https://github.com/rover2024/lorelei.git
cd lorelei

cmake -B build -G ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/lorelei \
    -Dqmsetup_DIR=/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_GUEST_TARGETS=FALSE \
    -DLORE_BUILD_THUNKS=TRUE \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_VULKAN_INCLUDE_DIR=/vulkan/include
cmake --build build --target TLC_generate_all
cmake --build build --target all
cmake --build build --target install
```

Second, build for guest. Specify `LORE_TLC_NO_RUN=FALSE` to disable TLC generation, and specify `LORE_THUNK_DATA_DIR` and `LORE_THUNK_SOURCE_DIR` to make the build system reuse the generated files.

```bash
cmake -B build-x86_64 -G ninja \
    -DCMAKE_C_COMPILER=/usr/bin/gcc-x86_64-linux-gnu \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++-x86_64-linux-gnu \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/lorelei-x86_64 \
    -Dqmsetup_DIR=/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \
    -DLORE_BUILD_THUNKS=TRUE \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_VULKAN_INCLUDE_DIR=/vulkan/include \
    -DLORE_TLC_NO_RUN=TRUE \
    -DLORE_THUNK_DATA_DIR=/lorelei/share/lorelei/thunks \
    -DLORE_THUNK_SOURCE_DIR=/lorelei/src/thunks
cmake --build build --target TLC_generate_all
cmake --build build --target all
cmake --build build --target install
``` -->

If you are building this project on an ARM64 Linux system, then you need to first copy the Lorelei artifacts that have been installed on x86_64, as well as the x86_64 rootfs, to your ARM system.

```bash
# For example, you copy the artifacts by SCP
cd $INSTALL_DIR
scp -r x64-system:$X64_INSTALL_DIR/lorelei .
rename lorelei lorelei-x86_64
```

Specify `LORE_TLC_NO_RUN=FALSE` to disable TLC generation, and specify `LORE_THUNK_DATA_DIR` and `LORE_THUNK_SOURCE_DIR` to make the build system reuse the generated files.

```bash
sudo apt install libffcall-dev

git clone https://github.com/rover2024/lorelei.git
cd lorelei

cmake -B build -G ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/lorelei \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_GUEST_TARGETS=FALSE \
    -DLORE_BUILD_THUNKS=TRUE \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_TLC_NO_RUN=TRUE \
    -DLORE_THUNK_DATA_DIR=$INSTALL_DIR/lorelei-x86_64/share/lorelei/thunks \
    -DLORE_THUNK_SOURCE_DIR=$INSTALL_DIR/lorelei-x86_64/src/thunks
    -DLORE_VULKAN_INCLUDE_DIR=$REPOS_DIR/Vulkan-Headers/include
    "-DLORE_SDL2_INCLUDE_DIR=$INSTALL_DIR/SDL/include;$INSTALL_DIR/SDL/include/SDL2"
cmake --build build --target all
cmake --build build --target install
```

## QEMU Integration

Lorelei is able to integrate with QEMU 11.0 or later, by using `-plugin` option.

The `QEMUThreadHook` library will intercept all thread creation requested by host libraries to make sure that all threads start from QEMU's thread entry, where the virtual `CPUState` is initialized.

The `QEMUPlugin` library will filter Lorelei magic system calls and forward the host-calls from guest thunk libraries to the host libraries.

```bash
LORELEI_GUEST_ROOT=$INSTALL_DIR/lorelei-x86_64 \
LD_PRELOAD=$INSTALL_DIR/lorelei/lib/libLoreQEMUThreadHook.so \
LD_LIBRARY_PATH=$INSTALL_DIR/lorelei/lib:/lorelei/lib/aarch64-LoreHTL \
    qemu-x86_64 -U LD_PRELOAD \
    -E LD_LIBRARY_PATH=$INSTALL_DIR/lorelei-x86_64/lib/x86_64-LoreGTL \
    -plugin $INSTALL_DIR/lorelei/lib/libLoreQEMUPlugin.so \
    <program> <args...>
```

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)
