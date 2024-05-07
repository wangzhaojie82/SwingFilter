# C++ implementation

This is a C++ implementation for BounceFilter and related solutions. The source code for LadderFilter is provided by the authors in their repository [LadderFilterCode/LadderFilter (github.com)](https://github.com/LadderFilterCode/LadderFilter).

## Project Structure

```
BounceFilter_Cpp/
├── main.cpp
├── header/
│   ├── MurmurHash3.h
│   ├── Sketch.h
│   ├── CountMin.h
│   ├── BounceFilter.h
│   ├── ColdFilter.h
│   ├── ASketch.h
│   └── LogLogFilter.h
├── MurmurHash3.cpp
├── CountMin.cpp
├── BounceFilter.cpp
├── ColdFilter.cpp
├── ASketch.cpp
└── LogLogFilter.cpp
```

## Usage

### Compilation

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Execution

```bash
$ ./BounceFilter_Cpp
```

## Requirements

- CMake 3.26 or above
- Compiler with support for C++17 standard
