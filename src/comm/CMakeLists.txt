add_library(comm INTERFACE)

configure_file(strategy_def.h.in strategy_def.h)

add_subdirectory(strategies)

target_link_libraries(comm INTERFACE strategy)
target_include_directories(comm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")