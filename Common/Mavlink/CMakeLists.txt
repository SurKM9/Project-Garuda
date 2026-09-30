include(FetchContent)

FetchContent_Declare(mavlink
    URL https://github.com/mavlink/c_library_v2/archive/04fffaab116486ffdf7501c37a3f7393eb7beffc.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(mavlink)

add_library(mavlink_headers INTERFACE)
target_include_directories(mavlink_headers SYSTEM INTERFACE ${mavlink_SOURCE_DIR})
