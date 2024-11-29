#add_library(simulators INTERFACE)

#target_link_libraries(simulators PUBLIC compiler_flags)
target_include_directories(API-Simulator INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/aer_simulator.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/munich_simulator.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/simulator.hpp"
        DESTINATION include/simulators
       )