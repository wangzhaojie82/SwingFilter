cmake_minimum_required(VERSION 3.26)
project(BounceFilter_Cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(BounceFilter_Cpp main.cpp
        header/MurmurHash3.h
        MurmurHash3.cpp
        header/Sketch.h
        CountMin.cpp
        header/CountMin.h
        BounceFilter.cpp
        header/BounceFilter.h
        LogLogFilter.cpp
        header/LogLogFilter.h
)
