# LORELEI

Lorelei is a cross-ISA compatibility layer for user-level binary translators to leverage host libraries and achieve acceleration.

## Build From Source

Lorelei CMake project requires `qmsetup` for configuration, you need to build it first.

```bash
sudo apt install libffcall-dev libllvm-dev libclang-dev

# Build qmsetup
git clone --recursive https://github.com/stdware/qmsetup.git
cd qmsetup
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/qmsetup
cmake --build build --target all
cmake --build build --target install

# Get Vulkan headers
git clone https://github.com/KhronosGroup/Vulkan-Headers.git

# Build this project
git clone https://github.com/rover2024/lorelei.git
cd lorelei
git checkout refactor-v2
cmake -B build -G ninja \
    -Dqmsetup_DIR=/qmsetup/lib/cmake/qmsetup \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/lorelei \
    -DLORE_BUILD_GUEST_TARGETS=TRUE
cmake --build build --target all
cmake --build build --target install
```

## QEMU Integration

Lorelei is able to integrate with QEMU 11.0 or later, by using `-plugin` option.

```bash
LD_LIBRARY_PATH=/lorelei/lib:/lorelei/lib/<arch>-LoreHTL \
    qemu-x86_64 -E LD_LIBRARY_PATH=lorelei/lib/x86_64-LoreGTL \
    -plugin /lorelei/lib/libLoreQEMUIntegration.so \
    <program> <args...>
```

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall/)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)
