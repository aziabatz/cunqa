#add_library(aer_includes "${CMAKE_CURRENT_SOURCE_DIR}/aer_includes.cpp")
#target_include_directories(aer_includes PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/aer-cpp/src"
#                                                "${PYBIND_PATH}/include")
#target_link_libraries(aer_includes PUBLIC json LAPACK::LAPACK ${Python_LIBRARIES}) 

#add_library(aer_simple_simulator "${CMAKE_CURRENT_SOURCE_DIR}/aer_simple_simulator.cpp")
#target_include_directories(aer_simple_simulator PRIVATE
#                                                 "${PYBIND_PATH}/include")
#target_link_libraries(aer_simple_simulator PUBLIC aer_includes json LAPACK::LAPACK ${Python_LIBRARIES})


#add_library(aer_classical_comm_simulator "${CMAKE_CURRENT_SOURCE_DIR}/aer_classical_comm_simulator.cpp")
#target_include_directories(aer_classical_comm_simulator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
#                                                            "${CMAKE_SOURCE_DIR}/src/classical_channel"
#                                                            "${PYBIND_PATH}/include")
#target_link_libraries(aer_classical_comm_simulator PUBLIC aer_includes json LAPACK::LAPACK ${Python_LIBRARIES}) 


add_library(aer_simulator "${CMAKE_CURRENT_SOURCE_DIR}/aer_simple_simulator.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/aer_classical_comm_simulator.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/aer_includes.cpp")
target_include_directories(aer_simulator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
                                                 "${CMAKE_SOURCE_DIR}/src/classical_channel"
                                                 "${CMAKE_SOURCE_DIR}/src/third-party/aer-cpp/src"
                                                 "${PYBIND_PATH}/include")
target_link_libraries(aer_simulator PUBLIC json LAPACK::LAPACK ${Python_LIBRARIES}) 