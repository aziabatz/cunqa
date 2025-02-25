
pybind11_add_module(qclient bindings.cpp)

target_link_libraries(qclient PUBLIC logger_client
                              PRIVATE ${libzmq_LIBRARIES})

target_include_directories(qclient PRIVATE "${CMAKE_SOURCE_DIR}/src"
                                          "${PROJECT_BINARY_DIR}/src/comm"
                                          "${CMAKE_SOURCE_DIR}/src/utils"
                                          "${CMAKE_SOURCE_DIR}/src/third-party/zmq"
                                          "${libzmq_INCLUDE_DIRS}")

install(TARGETS qclient DESTINATION cunqa)

file(GLOB PYTHON_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(FILES ${PYTHON_SCRIPTS} DESTINATION cunqa)