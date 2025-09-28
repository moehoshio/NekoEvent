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

# Testing
if(NEKO_BUILD_TESTS)
    enable_testing()
    
    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Test executable
    add_executable(event_tests tests/event_test.cpp)
    target_link_libraries(event_tests 
        PRIVATE 
        NekoEvent
        gtest_main
        gtest
    )
    target_compile_features(event_tests PRIVATE cxx_std_20)
    
    if(MSVC)
        target_compile_options(event_tests PRIVATE /Zc:__cplusplus)
    endif()
    
    # Add test to CTest
    include(GoogleTest)
    gtest_discover_tests(event_tests)
endif()
