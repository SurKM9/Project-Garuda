cmake_minimum_required(VERSION 3.16)
project(UAV_System_Suite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define where our sub-projects are
add_subdirectory(Common)
add_subdirectory(Simulator)
add_subdirectory(Backend)
add_subdirectory(Dashboard)

find_package(Doxygen)

if (DOXYGEN_FOUND)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)

    # This command creates a "docs" target
    doxygen_add_docs(docs
        ${PROJECT_SOURCE_DIR}/Dashboard
        ${PROJECT_SOURCE_DIR}/Simulator
        ${PROJECT_SOURCE_DIR}/Common
        ALL
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif()
