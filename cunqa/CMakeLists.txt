
pybind11_add_module(qclient bindings.cpp)

#find_library(GNUTLS_LIB gnutls REQUIRED PATHS /mnt/netapp1/Optcesga_FT2_RHEL7/2022/gentoo/31032022/usr/lib64/)
#find_library(SODIUM_LIB sodium REQUIRED PATHS /mnt/netapp1/Optcesga_FT2_RHEL7/2022/gentoo/31032022/usr/lib64/)
target_link_libraries(qclient PUBLIC logger_client
                              PRIVATE ${libzmq_LIBRARIES}) #${GNUTLS_LIB} ${SODIUM_LIB} bsd)

target_include_directories(qclient PRIVATE "${CMAKE_SOURCE_DIR}/src"
                                          "${PROJECT_BINARY_DIR}/src/comm"
                                          "${CMAKE_SOURCE_DIR}/src/utils"
                                          "${CMAKE_SOURCE_DIR}/src/third-party/zmq"
                                          "${libzmq_INCLUDE_DIRS}")
target_compile_options(qclient PRIVATE -fvisibility=hidden)
target_link_options(qclient PRIVATE -Wl,-Bsymbolic)

install(TARGETS qclient DESTINATION cunqa)

file(GLOB PYTHON_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(FILES ${PYTHON_SCRIPTS} DESTINATION cunqa)