cmake_minimum_required(VERSION 3.14)
project(NekoEvent VERSION 1.0 LANGUAGES CXX)

# ================
# === Config ====
# ================

# NekoEvent project specific options
option(NEKO_EVENT_BUILD_TESTS "Neko Event Build tests" ON)
option(NEKO_EVENT_AUTO_FETCH_DEPS "Neko Event Automatically fetch dependencies" ON)
option(NEKO_EVENT_ENABLE_MODULE "Neko Event Enable C++20 module" OFF)

find_package(NekoSchema QUIET)
find_package(GTest QUIET)

# Print configuration summary
message(STATUS "Start configuration Neko Event...")
message(STATUS "")
message(STATUS "NekoEvent configuration summary:")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "")
message(STATUS "  - Neko Event Auto fetch deps: ${NEKO_EVENT_AUTO_FETCH_DEPS}")
message(STATUS "  - Neko Event Build tests: ${NEKO_EVENT_BUILD_TESTS}")
message(STATUS "  - Neko Event Enable module: ${NEKO_EVENT_ENABLE_MODULE}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - NekoSchema : ${NekoSchema_FOUND} version : ${NekoSchema_VERSION}")
message(STATUS "  - GTest : ${GTest_FOUND} version : ${GTest_VERSION}")
message(STATUS "")


if(NEKO_EVENT_AUTO_FETCH_DEPS)
    include(FetchContent)

    if (NOT NekoSchema_FOUND)
        message(STATUS "NekoSchema not found; Neko Event Fetching NekoSchema...")

        FetchContent_Declare(
            NekoSchema
            GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
            GIT_TAG        main
        )
        set(NEKO_SCHEMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        if (NEKO_EVENT_ENABLE_MODULE)
            set(NEKO_SCHEMA_ENABLE_MODULE ON CACHE BOOL "" FORCE)
        endif()
        FetchContent_MakeAvailable(NekoSchema)
    endif()

    if(NOT GTest_FOUND AND NEKO_EVENT_BUILD_TESTS)
        message(STATUS "GTest not found; Neko Event Fetching GoogleTest...")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.17.0
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

endif(NEKO_EVENT_AUTO_FETCH_DEPS)


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

# ================
# = C++20 Module =
# ================

if(NEKO_EVENT_ENABLE_MODULE)
    message(STATUS "NekoEvent C++20 module enabled (NEKO_EVENT_ENABLE_MODULE=ON)")
    
    # Check CMake version for module support
    if(CMAKE_VERSION VERSION_LESS 3.28)
        message(WARNING "CMake 3.28+ is recommended for full C++20 module support")
    endif()
    
    # Create module library
    add_library(NekoEvent_module)
    add_library(Neko::Event::Module ALIAS NekoEvent_module)
    
    target_sources(NekoEvent_module
        PUBLIC
            FILE_SET CXX_MODULES FILES
                ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/event/neko.event.cppm
    )
    
    target_compile_features(NekoEvent_module PUBLIC cxx_std_20)
    
    # Link dependencies (needed for module compilation)
    target_link_libraries(NekoEvent_module PUBLIC NekoSchema_module)
    
    # Compiler-specific module flags
    if(MSVC)
        target_compile_options(NekoEvent_module PUBLIC /experimental:module)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(NekoEvent_module PUBLIC -fmodules-ts)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(NekoEvent_module PUBLIC -fmodules)
    endif()
    
    # Module still needs access to the include directory for the headers
    target_include_directories(NekoEvent_module PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    
else()
    message(STATUS "NekoEvent C++20 module disabled (NEKO_EVENT_ENABLE_MODULE=OFF)")
endif()

# ================
# ==== Tests =====
# ================

if(NEKO_EVENT_BUILD_TESTS)
    enable_testing()
    message(STATUS "NekoEvent tests enabled (NEKO_EVENT_BUILD_TESTS=ON)")

    if (NOT GTest_FOUND AND NOT NEKO_EVENT_AUTO_FETCH_DEPS)
        message(WARNING "GTest is required for building tests but was not found.")
        message(FATAL_ERROR "Please enable -DNEKO_EVENT_AUTO_FETCH_DEPS=ON or install GTest and make it discoverable by CMake. e.g -DCMAKE_PREFIX_PATH=</path/to/googletest>")
    endif()
    
    add_executable(event_tests tests/event_test.cpp)
    target_include_directories(event_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(event_tests PRIVATE NekoEvent GTest::gtest GTest::gtest_main)
    target_compile_features(event_tests PRIVATE cxx_std_20)

    include(GoogleTest)
    gtest_discover_tests(event_tests)

    # Module tests
    if (NEKO_EVENT_ENABLE_MODULE)
        add_executable(event_module_tests tests/event_module_test.cpp)
        target_link_libraries(event_module_tests PRIVATE Neko::Schema::Module Neko::Event::Module GTest::gtest GTest::gtest_main)
        target_compile_features(event_module_tests PRIVATE cxx_std_20)
        gtest_discover_tests(event_module_tests)
    endif(NEKO_EVENT_ENABLE_MODULE)

else()
    message(STATUS "NekoEvent tests disabled (NEKO_EVENT_BUILD_TESTS=OFF)")
endif()

message(STATUS "Neko Event End of configuration")