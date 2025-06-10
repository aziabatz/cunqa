
add_library(cunqa_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/cunqa_simple_simulator.cpp")
target_include_directories(cunqa_simple_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
                                                            "${CMAKE_SOURCE_DIR}/src/third-party/cunqasimulator/"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(cunqa_simple_simulator PUBLIC json classical_channel)


add_library(cunqa_cc_sim "${CMAKE_CURRENT_SOURCE_DIR}/cunqa_cc_simulator.cpp")
target_include_directories(cunqa_cc_sim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/third-party/cunqasimulator/" 
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")

target_link_libraries(cunqa_cc_sim PRIVATE classical_channel)



