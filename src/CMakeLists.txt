add_library(API-Simulator INTERFACE)

set(COMM_LIB "ASIO")

add_subdirectory(comm)
add_subdirectory(config)
add_subdirectory(utils)
add_subdirectory(simulators)
add_subdirectory(third-party)

find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(API-Simulator INTERFACE compiler_flags config-json nlohmann_json::nlohmann_json aer-cpp comm Threads::Threads m)

message(STATUS "PREFIJO DE INSTALACION: ${CMAKE_INSTALL_PREFIX}")
#install(TARGETS API-Simulator DESTINATION "${CMAKE_INSTALL_LIBDIR}")

message(STATUS "Librería con los includes: ${CMAKE_INSTALL_INCLDIR}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/qpu.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/backend.hpp"
        DESTINATION include
       )