cmake_minimum_required(VERSION 3.21)

project(CUNQA VERSION 0.0.1 LANGUAGES CXX)

# Adding C++20 standard as required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "C++ version ${CXX_STANDARD} configured.")
message(STATUS "${CMAKE_VERSION}")
if(${CMAKE_VERSION} VERSION_EQUAL "3.27.6")
    cmake_policy(SET CMP0144 OLD)
endif()
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

message(STATUS "Install location will be: $ENV{HOME}")
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64")
set(CMAKE_INSTALL_BINDIR "bin")

find_package(Python 3.9.9 EXACT COMPONENTS Interpreter Development)

#TODO: Load pybind11 depending on the environment
if(DEFINED PYBIND_PATH)
    set(pybind11_DIR "${PYBIND_PATH}/share/cmake/pybind11")
else(NOT DEFINED PYBIND_PATH)
    string(FIND "$ENV{LOADEDMODULES}" "pybind11" module_pos)
    if(module_pos GREATER -1)
        #list(APPEND CMAKE_PREFIX_PATH "/opt/cesga/2022/software/Compiler/gcccore/system/pybind11/2.12.0/lib/python3.9/site-packages/pybind11/share/cmake/pybind11")
        set(PYBIND_PATH "/opt/cesga/qmio/hpc/software/Compiler/gcccore/12.3.0/pybind11/2.12.0-python-3.9.9/lib64/python3.9/site-packages/pybind11")
        set(pybind11_DIR "/opt/cesga/qmio/hpc/software/Compiler/gcccore/12.3.0/pybind11/2.12.0-python-3.9.9/lib64/python3.9/site-packages/pybind11/share/cmake/pybind11")
        message(STATUS "Loading the default path for pybind11...")
    else()
        message(WARNING
        "Yo do not have any module of pybind11 loaded. This will fail " 
        "unless you have a conda environment with the correct pybind11 "
        "versions. \n"
        "This is NOT recomended." 
        )
    endif()
endif()

set(PYBIND11_PYTHON_VERSION 3.9 CACHE STRING "")

find_package(pybind11 2.12 REQUIRED)
find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads REQUIRED)
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Boost REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/utils/logger" "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${Python_INCLUDE_DIRS}" "${MPI_INCLUDE_PATH}")

add_subdirectory(src)
add_subdirectory(cunqa)
add_subdirectory(examples)