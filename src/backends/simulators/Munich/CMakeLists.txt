add_library(munich_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_simple_simulator.cpp")
target_include_directories(munich_simple_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_simple_simulator PRIVATE json MQT::DDSim)


add_library(munich_cc_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_cc_simulator.cpp")
target_include_directories(munich_cc_simulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_cc_simulator PUBLIC json MQT::DDSim)
