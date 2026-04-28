import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.15
import QtCharts
import QtLocation
import QtPositioning

Window {
    width: 900
    height: 600
    visible: true
    title: "Project Garuda - UAV Ground Control"
    color: "#0f172a" // Deep Space Blue/Black

    // --- Background Styling ---
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#1e293b"
            }
            GradientStop {
                position: 1.0
                color: "#0f172a"
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // --- LEFT PANEL: STATUS & CONTROLS ---
        ColumnLayout {
            Layout.preferredWidth: 300
            spacing: 25

            // Mission Status Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: "#1e293b"
                radius: 8
                border.color: "#334155"

                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    Text {
                        text: "SYSTEM STATUS"
                        color: "#94a3b8"
                        font.pixelSize: 12
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: telemetry.altitude > 0.5 ? "IN FLIGHT" : "STANDBY"
                        color: telemetry.altitude > 0.5 ? "#22c55e" : "#f59e0b"
                        font.pixelSize: 24
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            // Battery Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: "#1e293b"
                radius: 8
                border.color: "#334155"

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 10

                    Text {
                        text: "BATTERY"
                        color: "#94a3b8"
                        font.pixelSize: 12
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: telemetry.battery + "%"
                        color: telemetry.battery < 20 ? "#ef4444" : "#22c55e"
                        font.pixelSize: 24
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Rectangle {
                        width: parent.width
                        height: 8
                        radius: 4
                        color: "#334155"
                        Rectangle {
                            width: parent.width * (telemetry.battery / 100)
                            height: parent.height
                            radius: 4
                            color: telemetry.battery < 20 ? "#ef4444" : "#22c55e"
                        }
                    }
                }
            }

            // Velocity Card
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: "#1e293b"
                radius: 8
                border.color: "#334155"

                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    Text {
                        text: "VELOCITY"
                        color: "#94a3b8"
                        font.pixelSize: 12
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Text {
                        text: telemetry.velocity.toFixed(1) + " m/s"
                        color: "#38bdf8"
                        font.pixelSize: 24
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            } // Spacer
        }

        Rectangle {
            width: 200
            Layout.fillHeight: true
            color: "#2c3e50"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 20 // Space between buttons

                Text {
                    text: "DRONE CONTROLS"
                    color: "white"
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Button {
                    text: "ARM SYSTEM"
                    Layout.fillWidth: true
                    enabled: telemetry.flightState === 0
                    background: Rectangle {
                        color: "#22c55e"
                        radius: 4
                    }
                    onClicked: telemetry.sendCommand(1)
                }

                Button {
                    text: "TAKEOFF"
                    Layout.fillWidth: true
                    enabled: telemetry.flightState === 1
                    onClicked: telemetry.sendCommand(3, 10.0)
                }

                Button {
                    text: "LAND"
                    Layout.fillWidth: true
                    enabled: telemetry.flightState === 3
                    onClicked: telemetry.sendCommand(4)
                }

                Button {
                    text: "DISARM"
                    Layout.fillWidth: true
                    enabled: telemetry.flightState === 1
                    onClicked: telemetry.sendCommand(2)
                }

                Button {
                    text: "EMERGENCY STOP"
                    Layout.fillWidth: true
                    background: Rectangle {
                        color: "#ef4444"
                        radius: 4
                    }
                    enabled: telemetry.flightState > 0
                    onClicked: telemetry.sendCommand(99)
                }

                // Pushes everything to the top
                Item {
                    Layout.fillHeight: true
                }
            }
        }

        // --- RIGHT PANEL: MAP + CHART TABS ---
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 500
            spacing: 0

            TabBar {
                id: tabBar
                Layout.fillWidth: true
                background: Rectangle {
                    color: "#0f172a"
                }
                TabButton {
                    text: "MAP"
                }
                TabButton {
                    text: "CHART"
                }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: tabBar.currentIndex

                Map {
                    id: droneMap
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    plugin: Plugin {
                        name: "osm"
                        PluginParameter { name: "osm.mapping.custom.host"; value: "https://tile.openstreetmap.org/" }
                        PluginParameter { name: "osm.mapping.custom.datacopyright"; value: "© OpenStreetMap contributors" }
                        PluginParameter { name: "osm.useragent"; value: "ProjectGaruda/1.0" }
                    }
                    center: QtPositioning.coordinate(48.1351, 11.5820)
                    zoomLevel: 14

                    onMapReadyChanged: {
                        if (mapReady) {
                            for (var i = 0; i < supportedMapTypes.length; i++) {
                                if (supportedMapTypes[i].name === "Custom URL Map") {
                                    activeMapType = supportedMapTypes[i]
                                    break
                                }
                            }
                        }
                    }

                    MapQuickItem {
                        coordinate: QtPositioning.coordinate(telemetry.latitude, telemetry.longitude)
                        anchorPoint.x: marker.width / 2
                        anchorPoint.y: marker.height / 2
                        sourceItem: Rectangle {
                            id: marker
                            width: 28; height: 28; radius: 14
                            color: "#38bdf8"
                            border.color: "white"; border.width: 3
                        }
                    }

                    MapPolyline {
                        id: flightPath
                        line.width: 5
                        line.color: "#38bdf8"
                    }

                    Connections {
                        target: telemetry
                        function onLatitudeChanged() {
                            var coord = QtPositioning.coordinate(telemetry.latitude, telemetry.longitude)
                            flightPath.addCoordinate(coord)
                            droneMap.center = coord
                        }
                    }
                }
                Item {
                    ChartView {
                        id: altChart
                        anchors.fill: parent
                        anchors.margins: -10 // Pulls the chart to the edges of the rectangle
                        theme: ChartView.ChartThemeDark
                        backgroundColor: "transparent"
                        antialiasing: true
                        legend.visible: false

                        ValueAxis {
                            id: axisY
                            min: 0
                            max: 15 // We'll update this dynamically
                            titleText: "Altitude (m)"
                        }

                        ValueAxis {
                            id: axisX
                            min: 0
                            max: 15 // Show last 50 data points
                            labelFormat: " " // Hide X labels for a cleaner look
                        }

                        LineSeries {
                            id: altSeries
                            name: "Altitude"
                            axisX: axisX
                            axisY: axisY
                            color: "#38bdf8" // Cyber Blue
                            width: 3
                        }

                        // Locate your ChartView and find the Connections block inside it
                        Connections {
                            target: telemetry

                            // Qt 6 style: use the 'on<SignalName>' syntax directly
                            function onAltitudeChanged() {
                                // Log to console once to verify the signal is actually arriving
                                // console.log("New altitude received: " + telemetry.altitude)

                                // 1. Append the data point (X = count, Y = value)
                                altSeries.append(altSeries.count, telemetry.altitude);

                                // 2. Scroll the X-Axis if we have more than 50 points
                                if (altSeries.count > 50) {
                                    axisX.min = altSeries.count - 50;
                                    axisX.max = altSeries.count;
                                }

                                // 3. Auto-scale the Y-Axis if the drone goes higher than the current view
                                if (telemetry.altitude > axisY.max - 10) {
                                    axisY.max = telemetry.altitude + 20;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
