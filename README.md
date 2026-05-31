# LORELEI

Lorelei is a cross-ISA compatibility layer for user-level binary translators to leverage host libraries and achieve acceleration.

## Build From Source

Lorelei CMake project requires `qmsetup` for configuration, you need to build it first.

```bash
export INSTALL_DIR=/home/user/install

git clone --recursive https://github.com/stdware/qmsetup.git
cd qmsetup

cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/qmsetup
cmake --build build --target all
cmake --build build --target install
```

Build this project.

```bash
sudo apt install libffcall-dev libllvm-20-dev libclang-20-dev

git clone https://github.com/rover2024/lorelei.git
cd lorelei

cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/lorelei \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \   # Disable if building for host ISA
    -DLORE_BUILD_TOOLS=TRUE             # Disable if building for guest ISA
cmake --build build --target all
cmake --build build --target install
```

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)
