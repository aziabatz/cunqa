# Server
add_library(server "${CMAKE_CURRENT_SOURCE_DIR}/asio_server.cpp")
target_include_directories(server PRIVATE "${Boost_INCLUDE_DIRS}")
target_link_libraries(server PRIVATE "${Boost_LIBRARIES}" logger_qpu)           

# Client
add_library(client "${CMAKE_CURRENT_SOURCE_DIR}/asio_client.cpp")
target_include_directories(client PRIVATE "${Boost_INCLUDE_DIRS}")
target_link_libraries(client PRIVATE "${Boost_LIBRARIES}" logger_client)


