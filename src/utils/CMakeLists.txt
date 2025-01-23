add_subdirectory(logger)

add_library(custom-json STATIC custom_json.cpp )
target_include_directories(custom-json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(custom-json PRIVATE nlohmann_json::nlohmann_json Threads::Threads logger_qpu)

#For the constants.hpp file
target_include_directories(API-Simulator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
install(FILES "backend_utils.hpp" DESTINATION include/utils)
install(FILES "basis_gates.json"  DESTINATION include/utils)

