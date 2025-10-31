add_subdirectory(logger)

# search for nlhomann system package if not found in target 
if(NOT TARGET nlohmann_json::nlohmann_json)
  find_package(nlohmann_json REQUIRED)
endif()

# Main json static library (kept for compatibility)
add_library(json json.cpp)
target_link_libraries(json PRIVATE nlohmann_json::nlohmann_json Threads::Threads)

# Re-link library object 
# FIXME: this is a workaround, must be safe-included!
add_library(json_obj OBJECT json.cpp)
target_link_libraries(json_obj PRIVATE nlohmann_json::nlohmann_json Threads::Threads)
