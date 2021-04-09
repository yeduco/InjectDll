cmake_minimum_required(VERSION 3.17)
project(InjectDll)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)

include_directories(tools)

add_executable(InjectDll
        src/main.cpp
        tools/tools.cpp
        tools/tools.h src/inject_dll/inject_dll.cpp src/inject_dll/inject_dll.h)

target_link_libraries(InjectDll psapi Version dxgi)
