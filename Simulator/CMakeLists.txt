# Silence the FindBoost deprecation warning
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

option(GARUDA_VERBOSE_LOGGING "Enable verbose debug logging in the simulator" OFF)

set(SIM_SOURCES
    main.cpp
    FlightController.cpp
)

set(SIM_HEADERS
    FlightController.hpp
)

# create an executable
add_executable(UAV_Simulator ${SIM_SOURCES} ${SIM_HEADERS})

if(GARUDA_VERBOSE_LOGGING)
    target_compile_definitions(UAV_Simulator PRIVATE GARUDA_VERBOSE_LOGGING)
endif()

find_package(Boost REQUIRED COMPONENTS system)

target_link_libraries(UAV_Simulator
    PRIVATE
        UAV_Common
        Boost::system
)

set_target_properties(UAV_Simulator PROPERTIES OUTPUT_NAME "drone_sim")
