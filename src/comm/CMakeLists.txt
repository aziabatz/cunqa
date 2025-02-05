add_library(comm INTERFACE)

configure_file(comm-strat_def.h.in comm-strat_def.h)

add_subdirectory(comm-strats)

target_link_libraries(comm INTERFACE comm_strat)
target_include_directories(comm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")