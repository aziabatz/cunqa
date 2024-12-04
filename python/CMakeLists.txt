
pybind11_add_module(client bindings.cpp)

target_include_directories(client PRIVATE "${CMAKE_SOURCE_DIR}/src"
                                          "${PROJECT_BINARY_DIR}/src/comm")

install(TARGETS client DESTINATION python)