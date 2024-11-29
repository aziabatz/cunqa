add_library(aer-cpp INTERFACE)

find_package(LAPACK REQUIRED)

target_link_libraries(aer-cpp INTERFACE compiler_flags LAPACK::LAPACK)
target_include_directories(aer-cpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/aer-cpp/src")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/aer-cpp/src"
        DESTINATION include/aer-cpp
       )