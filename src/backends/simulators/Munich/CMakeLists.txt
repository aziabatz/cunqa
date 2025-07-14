add_subdirectory(munich_adapters)

add_library(munich_executor "${CMAKE_CURRENT_SOURCE_DIR}/munich_executor.cpp")
target_include_directories(munich_executor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                    "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_executor PUBLIC json logger_qpu quantum_task classical_channel munich_adapters logger_executor)


add_library(munich_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_simple_simulator.cpp")
target_include_directories(munich_simple_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_simple_simulator PRIVATE json logger_qpu MQT::DDSim quantum_task)


add_library(munich_cc_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_cc_simulator.cpp")
target_include_directories(munich_cc_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_cc_simulator PUBLIC json logger_qpu MQT::DDSim quantum_task classical_channel)


add_library(munich_qc_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_qc_simulator.cpp")
target_include_directories(munich_qc_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                        "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_qc_simulator PUBLIC json quantum_task classical_channel munich_executor logger_executor)


