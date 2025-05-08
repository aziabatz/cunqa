add_library(CUNQA INTERFACE)

# Selecting communication library between the client and the servers (QPUs)
option(USE_ASIO "Using the ASIO library for communication" OFF)
option(USE_ZMQ "Using the ZMQ library for communication" OFF)
option(USE_CROW "Using the CROW library for communication" OFF)

if(USE_ASIO)
    set(COMM_LIB 0)
elseif(USE_ZMQ)
    set(COMM_LIB 1)
elseif(USE_CROW)
    message(FATAL_ERROR "CROW is not implemented yet.")
    set(COMM_LIB 2)
else()
    # We set the default communication library to ZMQ
    set(COMM_LIB 1)
endif()

# Selecting communication library between the QPUs
option(USE_QPU_MPI "Using MPI for QPU communication" OFF)
option(USE_QPU_ZMQ "Using ZMQ library for QPU communication" OFF)

if(USE_QPU_MPI)
    set(QPU_COMM MPI)
    add_definitions(-DQPU_MPI)
elseif(USE_QPU_ZMQ)
    set(QPU_COMM ZMQ)
    add_definitions(-DQPU_ZMQ)
else()
    set(QPU_COMM MPI)
    add_definitions(-DQPU_ZMQ)
endif()



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

include_directories("${Python_INCLUDE_DIRS}" "${MPI_INCLUDE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/utils")

add_subdirectory(cli)