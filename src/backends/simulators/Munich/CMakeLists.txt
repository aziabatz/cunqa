add_library(munich_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_simple_simulator.cpp")
target_link_libraries(munich_simple_simulator PRIVATE json MQT::DDSim)

add_library(munich_classical_comm_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_classical_comm_simulator.cpp")
target_link_libraries(munich_classical_comm_simulator PUBLIC json MQT::DDSim)
