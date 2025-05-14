#Example 1: Simple sending of circuit
add_executable(example1 example1.cpp)

target_link_libraries(example1 PRIVATE client json) 
install(TARGETS example1 DESTINATION example)

#Example 1: Sending consecutive circuits, both correct
add_executable(example2 example2.cpp)

target_link_libraries(example2 PRIVATE client json) 
install(TARGETS example2 DESTINATION example)

add_executable(example3 example3.cpp)

target_link_libraries(example3 PRIVATE client json) 
install(TARGETS example3 DESTINATION example)
