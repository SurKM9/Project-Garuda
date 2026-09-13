include(FetchContent)

FetchContent_Declare(mavlink
    GIT_REPOSITORY https://github.com/mavlink/c_library_v2.git
    GIT_TAG 04fffaab116486ffdf7501c37a3f7393eb7beffc
)
FetchContent_MakeAvailable(mavlink)

add_library(mavlink_headers INTERFACE)
target_include_directories(mavlink_headers INTERFACE ${mavlink_SOURCE_DIR})

add_executable(UAV_Backend src/main.cpp)
target_link_libraries(UAV_Backend PRIVATE UAV_Common mavlink_headers)
