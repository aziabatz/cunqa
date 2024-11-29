# Building custom-json 
#project(Prueba)
#cmake_minimum_required(VERSION 3.16)

find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads REQUIRED)

add_library(custom-json SHARED custom_json.cpp)
target_include_directories(custom-json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(custom-json PRIVATE nlohmann_json::nlohmann_json Threads::Threads)

#For the constants.hpp file
target_include_directories(API-Simulator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS custom-json DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/constants.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/custom_json.hpp"
              "${CMAKE_CURRENT_SOURCE_DIR}/helpers.hpp"
        DESTINATION include/utils
       )

#find_package(nlohmann_json 3.11.3 REQUIRED)
#if(nlohmann_json_FOUND)
#    add_library(${LIBRARY1_NAME} SHARED custom_json.cpp)
#    target_include_directories(${LIBRARY1_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#    target_link_libraries(${LIBRARY1_NAME} PRIVATE nlohmann_json::nlohmann_json)
#else()
#    message(FATAL_ERROR "nlohman/json library not found!")
#endif()

# Building ip-config
#set(LIBRARY2_NAME)

#find_package(Boost 1.85.0 REQUIRED)
#if(Boost_FOUND)
#    include_directories(${Boost_INCLUDE_DIRS})
#    target_include_directories(${LIBRARY2_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#    target_link_libraries(${LIBRARY2_NAME} PRIVATE ${Boost_LIBRARIES})
#else()
#    message(FATAL_ERROR "Boost library not found!")
#endif()