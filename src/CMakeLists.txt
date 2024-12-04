add_library(API-Simulator INTERFACE)

set(COMM_LIB "ASIO")

add_subdirectory(comm)
add_subdirectory(config)
add_subdirectory(utils)
add_subdirectory(simulators)
add_subdirectory(third-party)

message(STATUS "CMake library path: ${CMAKE_LIBRARY_PATH}")

find_package(Python COMPONENTS Interpreter Development)
find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Threads REQUIRED)

find_package(MPI REQUIRED)
#if (NOT MPI_FOUND)
#        /opt/cesga/2022/software/Compiler/gcc/system/openmpi/4.1.4/lib

FIND_PACKAGE(OpenMP REQUIRED)
find_package(Boost REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

include_directories(${Python_INCLUDE_DIRS} ${MPI_INCLUDE_PATH})

target_link_libraries(API-Simulator INTERFACE compiler_flags config-json 
                                              nlohmann_json::nlohmann_json 
                                              aer-cpp comm Threads::Threads m
                                              ${PYTHON_LIBRARIES})

add_subdirectory(cli)

message(STATUS "PREFIJO DE INSTALACION: ${CMAKE_INSTALL_PREFIX}")
#install(TARGETS API-Simulator DESTINATION "${CMAKE_INSTALL_LIBDIR}")

message(STATUS "Librería con los includes: ${CMAKE_INSTALL_INCLDIR}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/qpu.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/backend.hpp"
        DESTINATION include
       )