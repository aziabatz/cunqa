add_library(munich_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_simple_simulator.cpp")
target_include_directories(munich_simple_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_simple_simulator PRIVATE json logger_qpu MQT::DDSim)


add_library(munich_classical_comm_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_classical_comm_simulator.cpp")
target_include_directories(munich_classical_comm_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_classical_comm_simulator PUBLIC json logger_qpu MQT::DDSim)
