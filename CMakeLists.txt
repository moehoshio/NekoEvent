cmake_minimum_required(VERSION 3.14)
project(NekoEvent VERSION 1.0 LANGUAGES CXX)

option(NEKO_AUTO_FETCH_DEPS "Automatically fetch dependencies" ON)
option(NEKO_BUILD_TESTS "Build tests" ON)


if(NEKO_AUTO_FETCH_DEPS)
    include(FetchContent)
    FetchContent_Declare(
        NekoSchema
        GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoSchema)
endif()

# Main library target
add_library(NekoEvent INTERFACE)
add_library(Neko::Event ALIAS NekoEvent)

target_include_directories(NekoEvent INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(NekoEvent INTERFACE NekoSchema)
target_compile_features(NekoEvent INTERFACE cxx_std_20)

if(MSVC)
    target_compile_options(NekoEvent INTERFACE /Zc:__cplusplus)
endif()
