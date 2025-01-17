add_library(API-Simulator INTERFACE)

set(COMM_LIB "ASIO")

find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads REQUIRED)
find_package(MPI REQUIRED)
find_package(spdlog REQUIRED)

find_package(OpenMP REQUIRED)
find_package(Boost REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

add_subdirectory(comm)
add_subdirectory(config)
add_subdirectory(utils)
add_subdirectory(simulators)
add_subdirectory(third-party)

include_directories(${Python_INCLUDE_DIRS} ${MPI_INCLUDE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/utils")
link_libraries(spdlog)

target_link_libraries(API-Simulator INTERFACE compiler_flags config-json 
                                              nlohmann_json::nlohmann_json 
                                              aer-cpp comm Threads::Threads m
                                              ${Python_LIBRARIES})

add_subdirectory(cli)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/qpu.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/backend.hpp"
        DESTINATION include
       )