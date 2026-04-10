add_library(UAV_Common INTERFACE)

# We use CMAKE_CURRENT_SOURCE_DIR to be explicit
target_include_directories(UAV_Common INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources(UAV_Common INTERFACE
    include/TelemetryData.hpp
)
