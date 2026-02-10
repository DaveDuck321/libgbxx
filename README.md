# libgb++

A C++26 standard library for the gameboy/ gameboy color.

## Demo games

This source-tree also includes 2 games to demonstrate library features.
- [Tetris](https://github.com/DaveDuck321/libgbxx/tree/main/tetris) ([submission](https://github.com/DaveDuck321/libgbxx/releases/tag/gbcompo2025) to gbcompo2025)
- [Dr Mario](https://github.com/DaveDuck321/libgbxx/tree/main/tetris) (proof-of-concept)

## Building

### Prerequisites:
1) You will need a modern [c++ toolchain](https://github.com/DaveDuck321/gb-llvm/releases) with gameboy support.
2) Linux or windows subsystem for linux.


### Building the standard library and the demos

```
> git clone https://github.com/DaveDuck321/libgbxx
> cd libgbcxx
> export $GB_TOOLCHAIN=<path-to-gb-toolchain>/bin/
> make
```
