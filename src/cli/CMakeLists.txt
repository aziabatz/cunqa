# QRAISE executable
add_executable(qraise qraise.cpp)
target_include_directories(qraise PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/argparse/include/argparse")
target_link_libraries(qraise PRIVATE json logger_client)
install(TARGETS qraise DESTINATION "${CMAKE_INSTALL_BINDIR}")

# QDROP executable
include(${CMAKE_SOURCE_DIR}/cmake/FindSLURM.cmake)

add_executable(qdrop qdrop.cpp)
target_include_directories(qdrop PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/argparse/include/argparse")
target_link_libraries(qdrop PRIVATE logger_client SLURM::SLURM json)
install(TARGETS qdrop DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/epilog.sh" DESTINATION "$ENV{STORE}/.cunqa")

if("$ENV{LMOD_SYSTEM_NAME}" STREQUAL "QMIO")
    install(FILES "slurm_qmio.conf" DESTINATION "${CMAKE_INSTALL_PREFIX}" RENAME "slurm.conf")
else()
    install(FILES "slurm_ft3.conf" DESTINATION "${CMAKE_INSTALL_PREFIX}" RENAME "slurm.conf")
endif()

# SETUP_QPUS executable
add_executable(setup_qpus setup_qpus.cpp)
target_link_libraries(setup_qpus PRIVATE json logger_qpu qpu aer_simulator munich_simple_simulator munich_classical_comm_simulator cunqa_simple_simulator cunqa_classical_comm_sim)
target_include_directories(qdrop PRIVATE "${CMAKE_SOURCE_DIR}/src")
install(TARGETS setup_qpus DESTINATION "${CMAKE_INSTALL_BINDIR}")

# QINFO executable
add_executable(qinfo qinfo.cpp)
target_link_libraries(qinfo PRIVATE json)
target_include_directories(qinfo PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/argparse/include/argparse")
install(TARGETS qinfo DESTINATION "${CMAKE_INSTALL_BINDIR}")


