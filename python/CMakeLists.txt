
pybind11_add_module(qclient bindings.cpp)

target_link_libraries(qclient PUBLIC compiler_flags)

target_include_directories(qclient PRIVATE "${CMAKE_SOURCE_DIR}/src"
                                          "${PROJECT_BINARY_DIR}/src/comm")

install(TARGETS qclient DESTINATION python)