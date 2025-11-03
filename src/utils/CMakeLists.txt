add_subdirectory(logger)

add_library(json INTERFACE)
target_include_directories(json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(json INTERFACE nlohmann_json::nlohmann_json)