
add_executable(example1 example1.cpp)

target_link_libraries(example1 PRIVATE client json) 
install(TARGETS example1 DESTINATION example)