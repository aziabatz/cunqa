set(SPDLOG_BUILD_SHARED OFF)
set(SPDLOG_BUILD_PIC ON)
set(SPDLOG_SYSTEM_INCLUDES ON)
add_subdirectory(spdlog)
target_compile_options(spdlog PRIVATE -Wno-covered-switch-default)

# TODO: Generalize the build type for all the project 
add_subdirectory(mqt-ddsim)
target_compile_features(mqt-ddsim PUBLIC cxx_std_20)

set(cmake_libzmq_args
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DENABLE_CPACK=OFF
    -DWITH_PERF_TOOL=OFF
    -DCMAKE_BUILD_TYPE=Release
    -DZMQ_BUILD_TESTS=OFF
    -DENABLE_DRAFTS=ON
    -DBUILD_SHARED=ON
    -DBUILD_STATIC=OFF
)

include(ExternalProject)

ExternalProject_Add(libzmq
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libzmq
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libzmq
    DOWNLOAD_COMMAND ""
    CMAKE_ARGS ${cmake_libzmq_args}
)

set(libzmq_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libzmq/src/libzmq-build ${CMAKE_CURRENT_SOURCE_DIR}/libzmq/include CACHE INTERNAL "")
set(libzmq_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libzmq/src/libzmq-build/lib/libzmq.so CACHE INTERNAL "")
