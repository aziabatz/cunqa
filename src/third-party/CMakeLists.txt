add_library(aer-cpp INTERFACE)

target_link_libraries(aer-cpp INTERFACE compiler_flags LAPACK::LAPACK)
target_include_directories(aer-cpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/aer-cpp/src")

add_library(argparse INTERFACE)

target_link_libraries(argparse INTERFACE compiler_flags)
target_include_directories(argparse INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/argparse/include/argparse")

set(SPDLOG_BUILD_SHARED OFF)
set(SPDLOG_BUILD_PIC ON)
set(SPDLOG_SYSTEM_INCLUDES ON)
add_subdirectory(spdlog)
target_compile_options(spdlog PRIVATE -Wno-covered-switch-default)

# TODO: Generalize the build type for all the project 
add_subdirectory(mqt-ddsim)