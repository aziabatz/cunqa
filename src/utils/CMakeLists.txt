add_subdirectory(logger)

add_library(custom-json STATIC custom_json.cpp)
target_include_directories(custom-json PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(custom-json PRIVATE nlohmann_json::nlohmann_json Threads::Threads logger_qpu)
