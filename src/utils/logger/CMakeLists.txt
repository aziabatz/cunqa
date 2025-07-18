add_library(logger_qpu STATIC logger_qpu.cpp)
target_link_libraries(logger_qpu PRIVATE spdlog::spdlog)
target_include_directories(logger_qpu PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/spdlog/include")

add_library(logger_client STATIC logger_client.cpp)
target_link_libraries(logger_client PRIVATE spdlog::spdlog)
target_include_directories(logger_client PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/spdlog/include")
target_compile_options(logger_client PRIVATE -fPIC)

add_library(logger_executor STATIC logger_executor.cpp)
target_link_libraries(logger_executor PRIVATE spdlog::spdlog)
target_include_directories(logger_executor PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/spdlog/include")
target_compile_options(logger_executor PRIVATE -fPIC)