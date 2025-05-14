add_subdirectory(third-party)
add_subdirectory(backends)
add_subdirectory(utils)
add_subdirectory(comm)
add_subdirectory(classical_channel)

add_library(quantum_task quantum_task.cpp)
target_link_libraries(quantum_task PUBLIC json)

add_library(qpu qpu.cpp)
target_link_libraries(qpu PUBLIC server 
                          PRIVATE json quantum_task)

add_subdirectory(cli)