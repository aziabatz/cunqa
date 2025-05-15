add_library(cunqa_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/cunqa_simple_simulator.cpp")
target_include_directories(cunqa_simple_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                         "${CMAKE_SOURCE_DIR}/src/third-party/cunqasimulator/")



add_library(cunqa_classical_comm_sim "${CMAKE_CURRENT_SOURCE_DIR}/cunqa_classical_comm_simulator.cpp")
target_include_directories(cunqa_classical_comm_sim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/third-party/cunqasimulator/" 
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")

target_link_libraries(cunqa_classical_comm_sim PRIVATE classical_channel)



