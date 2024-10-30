# ============================================================================ #
# Copyright (c) 2022 - 2024 NVIDIA Corporation & Affiliates.                   #
# All rights reserved.                                                         #
#                                                                              #
# This source code and the accompanying materials are made available under     #
# the terms of the Apache License 2.0 which accompanies this distribution.     #
# ============================================================================ #

# Building custom-json 
set(LIBRARY1_NAME custom-json)

find_package(nlohmann_json 3.11.3 REQUIRED)
if(nlohmann_json_FOUND)
    add_library(${LIBRARY1_NAME} SHARED custom_json.cpp)
    target_include_directories(${LIBRARY1_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${LIBRARY1_NAME} PRIVATE nlohmann_json::nlohmann_json)
else()
    message(FATAL_ERROR "nlohman/json library not found!")
endif()

# Building ip-config
set(LIBRARY2_NAME )

find_package(Boost 1.85.0 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_include_directories(${LIBRARY2_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${LIBRARY2_NAME} PRIVATE ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost library not found!")
endif()