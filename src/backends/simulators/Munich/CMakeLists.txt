add_library(munich_simulator "${CMAKE_CURRENT_SOURCE_DIR}/munich_simple_simulator.cpp")
target_link_libraries(munich_simulator PRIVATE json MQT::DDSim)