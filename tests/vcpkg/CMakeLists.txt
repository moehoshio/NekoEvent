cmake_minimum_required(VERSION 3.15)
project(VcpkgTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(NekoEvent REQUIRED)

add_executable(vcpkg_test test.cpp)
target_link_libraries(vcpkg_test PRIVATE Neko::Event)
