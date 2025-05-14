add_subdirectory(logger)

add_library(json STATIC json.cpp)
target_include_directories(json PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(json PRIVATE nlohmann_json::nlohmann_json Threads::Threads logger_qpu MPI::MPI_CXX)
