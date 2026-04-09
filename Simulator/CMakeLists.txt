# Silence the FindBoost deprecation warning
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# The rest remains the same
add_executable(UAV_Simulator src/main.cpp)

find_package(Boost REQUIRED COMPONENTS system)

target_link_libraries(UAV_Simulator
    PRIVATE
        UAV_Common
        Boost::system
)

set_target_properties(UAV_Simulator PROPERTIES OUTPUT_NAME "drone_sim")
