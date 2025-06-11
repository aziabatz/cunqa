add_library(munich_adapters "${CMAKE_CURRENT_SOURCE_DIR}/circuit_simulator_adapter.cpp")
target_include_directories(munich_adapters PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel")
target_link_libraries(munich_adapters PUBLIC json MQT::DDSim)