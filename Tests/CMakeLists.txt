# Tests/CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Force GTest to use the same runtime as our project
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Find the threading library (required for GTest on Linux)
find_package(Threads REQUIRED)

add_executable(run_tests
    test_main.cpp
    flight_logic_tests.cpp
    ../Simulator/FlightController.cpp
)

# Ensure the tests can find the Simulator headers
target_include_directories(run_tests PRIVATE ../Simulator)

# MENTOR TIP: Order matters!
# We link the 'main' wrapper, then the 'core' engine, then the system threads.
target_link_libraries(run_tests
    PRIVATE
    GTest::gtest_main
    GTest::gtest
    Threads::Threads
    UAV_Common
)

# This tells CMake to 'discover' the tests inside the executable
include(GoogleTest)
gtest_discover_tests(run_tests)
