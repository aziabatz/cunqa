add_library(custom-json STATIC custom_json.cpp)
target_include_directories(custom-json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(custom-json PRIVATE nlohmann_json::nlohmann_json Threads::Threads)

#For the constants.hpp file
target_include_directories(API-Simulator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS custom-json DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/constants.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/custom_json.hpp"
              "${CMAKE_CURRENT_SOURCE_DIR}/helpers.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/logger.hpp"
        DESTINATION include/utils
       )