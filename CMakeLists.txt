cmake_minimum_required(VERSION 3.16)
project(UAV_System_Suite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Keep a symlink at the project root so clangd always finds the latest database
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        ERROR_QUIET
    )
endif()

# Define where our sub-projects are
# Add a toggle option (default to ON for your local ThinkPad work)
option(BUILD_DASHBOARD "Build the Qt Dashboard" ON)
option(BUILD_SIMULATOR "Build the Flight Controller Simulator" ON)
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_DOCS  "Build API documentation" ON)

add_subdirectory(Common)

if(BUILD_SIMULATOR)
    add_subdirectory(Simulator)
endif()

if(BUILD_DASHBOARD)
    add_subdirectory(Dashboard)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

if(BUILD_DOCS)
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
endif()
