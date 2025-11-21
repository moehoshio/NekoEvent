vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO moehoshio/NekoEvent
    REF v1.0.0
    SHA512 24867bf3376bf2cdfec7af5d5474615d0a6a103e7139c171de0bdbb54921e1e128ac66a2e7f3877e15e68a84791174beeaa5d6e6ad85c63df0a8cca87a587b37
    HEAD_REF main
)

set(VCPKG_BUILD_TYPE release)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DNEKO_EVENT_BUILD_TESTS=OFF
        -DNEKO_EVENT_AUTO_FETCH_DEPS=OFF
        -DNEKO_EVENT_ENABLE_MODULE=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/NekoEvent PACKAGE_NAME nekoevent)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
