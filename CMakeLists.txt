cmake_minimum_required(VERSION 3.14)
project(NekoEvent VERSION 1.0 LANGUAGES CXX)

option(NEKO_AUTO_FETCH_DEPS "Automatically fetch dependencies" ON)
option(NEKO_BUILD_TESTS "Build tests" ON)

# Print configuration summary
message(STATUS "NekoEvent configuration summary:")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "")
message(STATUS "  - Neko Auto fetch deps: ${NEKO_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Build tests: ${NEKO_BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - GoogleTest support: ${GTest_FOUND}")
message(STATUS "")


if(NEKO_AUTO_FETCH_DEPS)
    include(FetchContent)

    FetchContent_Declare(
        NekoSchema
        GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoSchema)

    if(NEKO_BUILD_TESTS)
        message(STATUS "GTest not found. Fetching GoogleTest...")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.12.1
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
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

# Tests
if(NEKO_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoEvent tests enabled")
    
    if (NOT NEKO_AUTO_FETCH_DEPS)
        message(FATAL_ERROR "GTest is required for building tests. Please enable NEKO_AUTO_FETCH_DEPS")
    endif()
    
    add_executable(event_tests tests/event_test.cpp)
    target_link_libraries(event_tests PRIVATE NekoEvent gtest_main)
    target_compile_features(event_tests PRIVATE cxx_std_20)

    include(GoogleTest)
    gtest_discover_tests(event_tests)
else()
    message(STATUS "NekoEvent tests disabled (NEKO_BUILD_TESTS=OFF)")
endif()
