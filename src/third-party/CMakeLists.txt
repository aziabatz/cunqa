add_library(aer-cpp INTERFACE)

target_link_libraries(aer-cpp INTERFACE LAPACK::LAPACK)
target_include_directories(aer-cpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/aer-cpp/src")

add_library(argparse INTERFACE)

target_include_directories(argparse INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/argparse/include/argparse")

set(SPDLOG_BUILD_SHARED OFF)
set(SPDLOG_BUILD_PIC ON)
set(SPDLOG_SYSTEM_INCLUDES ON)
add_subdirectory(spdlog)
target_compile_options(spdlog PRIVATE -Wno-covered-switch-default)

# TODO: Generalize the build type for all the project 
add_subdirectory(mqt-ddsim)
target_compile_features(mqt-ddsim PUBLIC cxx_std_20)
