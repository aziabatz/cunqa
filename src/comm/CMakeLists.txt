add_library(comm INTERFACE)

configure_file(comm_strat_def.h.in comm_strat_def.h)

add_subdirectory(comm-strats)

target_link_libraries(comm INTERFACE comm_strat)
target_include_directories(comm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")