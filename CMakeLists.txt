cmake_minimum_required(VERSION 3.16)
project(API-Simulator VERSION 0.1.0 LANGUAGES CXX)

# Adding C++11 standard as required
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
message(STATUS "C++ version ${CXX_STANDARD} configured.")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "default install path" FORCE)
endif()

set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(cxxFlags)

add_compile_flags_if_supported(-Wno-attributes)
add_compile_flags_if_supported(-Wno-deprecated-declarations)
add_compile_flags_if_supported(-Wno-maybe-uninitialized)

add_subdirectory(src)

# Installation rules
install(TARGETS api-simulator
    RUNTIME DESTINATION "bin"    # For executables
)