add_library(logger_qpu STATIC logger_qpu.cpp)
target_link_libraries(logger_qpu PRIVATE spdlog::spdlog)
target_include_directories(logger_qpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_SOURCE_DIR}/src/third-party/spdlog/include)

add_library(logger_client STATIC logger_client.cpp)
target_link_libraries(logger_client PRIVATE spdlog::spdlog)
target_include_directories(logger_client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} 
                                                ${CMAKE_SOURCE_DIR}/src/third-party/spdlog/include)
target_compile_options(logger_client PRIVATE -fPIC)