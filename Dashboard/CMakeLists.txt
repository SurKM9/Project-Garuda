# 1. The 'find_package' MUST come before any 'qt_' commands
# This tells CMake where Qt is and loads the 'qt_add_executable' macro
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Charts Widgets)

# 2. These are the engines that handle Signals/Slots and Resources
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 3. Define your executable
# We use 'DashboardApp' so it doesn't collide with the 'UAV_Dashboard' folder
qt_add_executable(DashboardApp
    src/main.cpp
    src/TelemetryProvider.cpp
)

# 4. Register your QML files
# This bundles Main.qml into your binary
qt_add_qml_module(DashboardApp
    URI UAV_Dashboard
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

# 5. This tells the compiler WHERE to look
target_include_directories(DashboardApp PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# This tells the IDE WHICH files to show you in the sidebar
target_sources(DashboardApp PRIVATE
    include/TelemetryProvider.hpp
    # (Any other headers you create later go here)
)

# 6. Link your Common library and Qt
target_link_libraries(DashboardApp
    PRIVATE
        UAV_Common
        Qt6::Quick
        Qt6::Charts
        Qt6::Widgets
)
