add_library(CUNQA INTERFACE)

set(COMM_LIB "ASIO")

find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads REQUIRED)
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)
find_package(Boost REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

add_subdirectory(third-party)
add_subdirectory(utils)
add_subdirectory(comm)

include_directories(${Python_INCLUDE_DIRS} ${MPI_INCLUDE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/utils")

add_subdirectory(cli)