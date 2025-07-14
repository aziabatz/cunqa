add_library(aer_simulator "${CMAKE_CURRENT_SOURCE_DIR}/aer_simulator.cpp")
target_include_directories(aer_simulator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
                                                 "${CMAKE_SOURCE_DIR}/src/classical_channel"
                                                 "${CMAKE_SOURCE_DIR}/src/third-party/aer-cpp/src"
                                                 "${PYBIND_PATH}/include")
target_link_libraries(aer_simulator PUBLIC json logger_qpu LAPACK::LAPACK ${Python_LIBRARIES}) 