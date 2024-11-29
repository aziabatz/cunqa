add_library(comm INTERFACE)

configure_file(strategy_def.h.in strategy_def.h)

add_subdirectory(strategies)

target_link_libraries(comm INTERFACE strategy compiler_flags)
target_include_directories(comm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/client.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/server.hpp" 
              "${PROJECT_BINARY_DIR}/src/comm/strategy_def.h"
        DESTINATION include/comm
       )
