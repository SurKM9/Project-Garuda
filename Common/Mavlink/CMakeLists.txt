include(FetchContent)

# MAVLink C library, pinned to a commit. Fetched as a tarball because CI has no git.
FetchContent_Declare(mavlink
    URL https://github.com/mavlink/c_library_v2/archive/04fffaab116486ffdf7501c37a3f7393eb7beffc.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(mavlink)

# Header-only library. SYSTEM hides warnings coming from inside the generated headers.
add_library(mavlink_headers INTERFACE)
target_include_directories(mavlink_headers SYSTEM INTERFACE ${mavlink_SOURCE_DIR})

# Ground station MAVLink code. Must stay Qt-free so CI (built without the Dashboard) can test it.
# mavlink_headers is PUBLIC because MavlinkParser.hpp exposes MAVLink types.
add_library(garuda_mavlink STATIC MavlinkParser.cpp)
target_include_directories(garuda_mavlink PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(garuda_mavlink PUBLIC mavlink_headers)
