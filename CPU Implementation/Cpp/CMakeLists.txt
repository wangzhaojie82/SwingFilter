cmake_minimum_required(VERSION 3.26)
project(SwingFilter_Cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(SwingFilter_Cpp main.cpp
        header/MurmurHash3.h
        MurmurHash3.cpp
        header/Sketch.h
        CountMin.cpp
        header/CountMin.h
        SwingFilter.cpp
        header/SwingFilter.h
        LogLogFilter.cpp
        header/LogLogFilter.h
        header/ColdFilter.h
        ColdFilter.cpp
)
