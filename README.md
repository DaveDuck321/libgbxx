# libgb++

A C++26 standard library for the gameboy/ gameboy color.

## Demo games

This source-tree also includes 2 games to demonstrate library features.
- [Tetris](https://github.com/DaveDuck321/libgbxx/tree/main/tetris) (submitted to gbcompo2025)
- [Dr Mario](https://github.com/DaveDuck321/libgbxx/tree/main/tetris) (proof-of-concept)

## Building

### Prerequisites:  
1) You will need a modern [c++ toolchain](https://github.com/DaveDuck321/gb-llvm) with gameboy support.
2) You will need `clang++` and `lld` installed to build the toolchain.
3) Linux or windows subsystem for linux

### Building the toolchain

If facing difficulties (or trying to build on MacOS), please refer to https://llvm.org/docs/CMake.html
```
> git clone https://github.com/DaveDuck321/gb-llvm
> cd gb-llvm
> mkdir build
> cmake -S llvm -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DLLVM_ENABLE_PROJECTS="clang;llvm;lld;lldb" \
    -DLLVM_TARGETS_TO_BUILD="" \
    -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=GB \
    -DLLVM_ENABLE_RUNTIMES="compiler-rt" \
    -DLLVM_RUNTIME_TARGETS="gb-unknown-unknown" \
    -DLLVM_BUILTIN_TARGETS="gb-unknown-unknown" \
    -DLLVM_PARALLEL_LINK_JOBS=8 \
    -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DLLVM_ENABLE_LLD=ON \
    -DCLANG_DEFAULT_CXX_STDLIB=libc++ \
    -DCLANG_DEFAULT_RTLIB=compiler-rt \
    -DCLANG_DEFAULT_LINKER=lld \
    -DRUNTIMES_gb-unknown-unknown_COMPILER_RT_BUILD_BUILTINS=ON \
    -DRUNTIMES_gb-unknown-unknown_COMPILER_RT_BAREMETAL_BUILD=ON \
    -DBUILTINS_gb-unknown-unknown_COMPILER_RT_BAREMETAL_BUILD=ON \
    -DBUILTINS_gb-unknown-unknown_CMAKE_BUILD_TYPE=release
> ninja -C build
> mkdir -p $(pwd)/build/lib/clang/21/lib/gb-unknown-unknown/ldscripts
> ln -s $(pwd)/gameboy-tooling/gb.ld $(pwd)/build/lib/clang/21/lib/gb-unknown-unknown/ldscripts/gb.ld
```
### Building the standard library and the demos

```
> git clone https://github.com/DaveDuck321/libgbxx
> cd libgbcxx
> export $GB_TOOLCHAIN=<path-to-gb-toolchain>/build/bin/
> make
```
