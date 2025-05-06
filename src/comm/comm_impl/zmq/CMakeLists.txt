# Server
add_library(server "${CMAKE_CURRENT_SOURCE_DIR}/zmq_server.cpp")
target_include_directories(server PRIVATE "${ZMQ_BINARY_DIR}"
                                          "${ZMQ_INCLUDE_DIR}"
                                          "${CMAKE_SOURCE_DIR}/src/third-party/zmq")
target_link_directories(server PUBLIC "${ZMQ_BINARY_DIR}/lib")                       
target_link_libraries(server PUBLIC zmq 
                             PRIVATE logger_qpu)

# Client
add_library(client SHARED "${CMAKE_CURRENT_SOURCE_DIR}/zmq_client.cpp")
target_include_directories(client PRIVATE "${ZMQ_BINARY_DIR}"
                                          "${ZMQ_INCLUDE_DIR}"
                                          "${CMAKE_SOURCE_DIR}/src/third-party/zmq")
target_link_directories(client PUBLIC "${ZMQ_BINARY_DIR}/lib")  
target_link_libraries(client PUBLIC zmq
                             PRIVATE logger_client)