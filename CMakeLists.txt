cmake_minimum_required(VERSION 3.14)
project(NekoEvent VERSION 1.0 LANGUAGES CXX)

# ================
# === Config ====
# ================

# NekoEvent project specific options
option(NEKO_EVENT_BUILD_TESTS "Neko Event Build tests" ON)
option(NEKO_EVENT_AUTO_FETCH_DEPS "Neko Event Automatically fetch dependencies" ON)

find_package(GTest CONFIG QUIET)

# Print configuration summary
message(STATUS "Start configuration Neko Event...")
message(STATUS "")
message(STATUS "NekoEvent configuration summary:")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "")
message(STATUS "  - Neko Event Auto fetch deps: ${NEKO_EVENT_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Event Build tests: ${NEKO_EVENT_BUILD_TESTS}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - GTest : ${GTest_FOUND} version : ${GTest_VERSION}")
message(STATUS "")


if(NEKO_EVENT_AUTO_FETCH_DEPS)
    include(FetchContent)

    FetchContent_Declare(
        NekoSchema
        GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(NekoSchema)

    if(NOT GTEST_FOUND AND NEKO_EVENT_BUILD_TESTS)
        message(STATUS "GTest not found; Neko Event Fetching GoogleTest...")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.17.0
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# ================
# = Main target =
# ================

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

# ================
# ==== Tests =====
# ================

if(NEKO_EVENT_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoEvent tests enabled (NEKO_EVENT_BUILD_TESTS=ON)")

    if (NOT GTEST_FOUND AND NOT NEKO_EVENT_AUTO_FETCH_DEPS)
        message(STATUS "GTest is required for building tests but was not found.")
        message(FATAL_ERROR "Please enable -DNEKO_EVENT_AUTO_FETCH_DEPS=ON or install GTest and make it discoverable by CMake. e.g -DCMAKE_PREFIX_PATH=</path/to/googletest>")
    endif()
    
    add_executable(event_tests tests/event_test.cpp)
    target_include_directories(event_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(event_tests PRIVATE NekoEvent GTest::gtest GTest::gtest_main)
    target_compile_features(event_tests PRIVATE cxx_std_20)

    include(GoogleTest)
    gtest_discover_tests(event_tests)
else()
    message(STATUS "NekoEvent tests disabled (NEKO_EVENT_BUILD_TESTS=OFF)")
endif()

message(STATUS "Neko Event End of configuration")