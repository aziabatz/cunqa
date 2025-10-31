# QRAISE executable
add_executable(qraise qraise.cpp)
target_include_directories(qraise PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/argparse/include/argparse")
target_link_libraries(qraise PRIVATE json logger_client)

if(USE_MPI_BTW_QPU)
    target_compile_definitions(qraise PUBLIC USE_MPI_BTW_QPU)
elseif(USE_ZMQ_BTW_QPU)
    target_compile_definitions(qraise PUBLIC USE_ZMQ_BTW_QPU)
endif()

install(TARGETS qraise DESTINATION "${CMAKE_INSTALL_BINDIR}")

# QDROP executable
if(NOT CUNQA_SLURMLESS)
  include(${CMAKE_SOURCE_DIR}/cmake/FindSLURM.cmake)
  if(SLURM_FOUND)
    add_executable(qdrop qdrop.cpp)
    target_include_directories(qdrop PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/argparse/include/argparse")
    target_link_libraries(qdrop PRIVATE logger_client SLURM::SLURM json)
    install(TARGETS qdrop DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/epilog.sh" DESTINATION "$ENV{STORE}/.cunqa")
  else()
    message(STATUS "SLURM not found: skipping qdrop build")
  endif()
else()
  message(STATUS "CUNQA_SLURMLESS enabled: skipping qdrop build")
endif()

file(GLOB NOISE_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/noise_model/*.py")
install(FILES ${NOISE_SCRIPTS} DESTINATION "$ENV{STORE}/.cunqa/noise_model")

# SETUP_QPUS executable
add_executable(setup_qpus setup_qpus.cpp)
target_link_libraries(setup_qpus PRIVATE 
    $<$<CXX_COMPILER_ID:GNU>:-Wl,--start-group>
    logger_qpu qpu 
    aer_simple_simulator aer_cc_simulator aer_qc_simulator
    munich_simple_simulator munich_cc_simulator munich_qc_simulator 
    cunqa_simple_simulator cunqa_cc_simulator cunqa_qc_simulator
    $<$<CXX_COMPILER_ID:GNU>:-Wl,--end-group>
)
# FIXME: workaround for write_on_file not referenced on link time
target_sources(setup_qpus PRIVATE ${CMAKE_SOURCE_DIR}/src/utils/json.cpp)
target_include_directories(setup_qpus PRIVATE "${CMAKE_SOURCE_DIR}/src")
install(TARGETS setup_qpus DESTINATION "${CMAKE_INSTALL_BINDIR}")

# EXECUTOR executable
add_executable(setup_executor setup_executor.cpp)
target_link_libraries(setup_executor PRIVATE 
    $<$<CXX_COMPILER_ID:GNU>:-Wl,--start-group>
    aer_executor munich_executor cunqa_executor logger_executor
    $<$<CXX_COMPILER_ID:GNU>:-Wl,--end-group>
)
target_sources(setup_executor PRIVATE ${CMAKE_SOURCE_DIR}/src/utils/json.cpp)
target_include_directories(setup_executor PRIVATE "${CMAKE_SOURCE_DIR}/src")
install(TARGETS setup_executor DESTINATION "${CMAKE_INSTALL_BINDIR}")

# QINFO executable
add_executable(qinfo qinfo.cpp)
target_link_libraries(qinfo PRIVATE json logger_client)
target_include_directories(qinfo PRIVATE "${CMAKE_SOURCE_DIR}/src/third-party/argparse/include/argparse")
install(TARGETS qinfo DESTINATION "${CMAKE_INSTALL_BINDIR}")
