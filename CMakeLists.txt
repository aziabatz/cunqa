cmake_minimum_required(VERSION 3.16)
project(API-Simulator VERSION 0.0.1 LANGUAGES CXX)

# Adding C++11 standard as required
add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_17)
message(STATUS "C++ version ${CXX_STANDARD} configured.")

set(CMAKE_CXX_COMPILER "/mnt/netapp1/Optcesga_FT2_RHEL7/2022/gentoo/31032022/usr/bin/g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/mnt/netapp1/Optcesga_FT2_RHEL7/2022/gentoo/31032022/usr/lib/gcc/x86_64-pc-linux-gnu/11.2.1/include/g++-v11/")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

set(CMAKE_INSTALL_PREFIX "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation")
#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#  set(CMAKE_INSTALL_PREFIX "$ENV{STORE}/api-simulator/installation" CACHE PATH "default install path" FORCE)
#  message(STATUS "Installation directory: ${CMAKE_INSTALL_PREFIX}")
#endif()

set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_INCLDIR "${CMAKE_INSTALL_PREFIX}/include")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")

find_package(pybind11 REQUIRED)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)