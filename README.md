# LC-3
This is an implementation of lc-3 assembler and emulator.

## Get sources
```
git clone --recurse-submodules https://github.com/Man-in-the-middl-e/lc3.git
```

## Objectives: 

- [x] Implement lc3 assembler.
- [x] Add build instructions.
- [x] Implement lc3 emulator.
- [ ] Fix bugs
- [ ] Add build instructions for Linux

## LC3 build instructions

### Windows
#### Dependencies:
- Visual Studio Community 2019 Release, or any other VS compiler that supports C++20.
- CMake
#### Build from command line:
```
mkdir build
cd build
cmake ..
cmake --build . --config Debug -j 18 --
```

#### Build from VS Code:
1. Press `Ctrl + Shift + P`, search for `CMake: Build` and run it.
2. To run an lc3assembler or lc3emulator, you simiply need to, choose `Run Emulator` or `Run Assembler` tasks, from VS code tasks(`Ctrl + Shift + P, Run Task`)

### Linux
```
TODO
```

## References:
https://en.wikipedia.org/wiki/Little_Computer_3