add_library(UAV_Common INTERFACE)

target_include_directories(UAV_Common INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# IDE-only target so Qt Creator shows headers in the project tree.
# Has no effect on the build.
add_custom_target(UAV_Common_Headers SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TelemetryData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/GarudaConfig.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SpscQueue.hpp
)
