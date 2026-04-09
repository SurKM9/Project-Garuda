import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

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
            GradientStop { position: 0.0; color: "#1e293b" }
            GradientStop { position: 1.0; color: "#0f172a" }
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

            // Battery Gauge
            Rectangle {
                Layout.fillWidth: true
                height: 80
                color: "transparent"

                Column {
                    width: parent.width
                    spacing: 8
                    Text { text: "BATTERY: " + (telemetry ? telemetry.battery : 0) + "%"; color: "white"; font.bold: true }
                    ProgressBar {
                        width: parent.width
                        value: telemetry.battery / 100
                        background: Rectangle { color: "#334155"; radius: 4 }
                        contentItem: Item {
                            Rectangle {
                                width: parent.width * parent.parent.value
                                height: parent.height
                                radius: 4
                                color: telemetry.battery < 20 ? "#ef4444" : "#22c55e"
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true } // Spacer

            // Big Action Button
            Button {
                Layout.fillWidth: true
                height: 60
                text: "INITIATE LANDING"
                onClicked: telemetry.sendLandCommand()

                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.down ? "#991b1b" : "#ef4444"
                    radius: 8
                    layer.enabled: true
                }
            }
        }

        // --- RIGHT PANEL: REAL-TIME CHART ---
        Rectangle {
            // Explicitly tell the Layout how to handle this box
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 500 // <--- Forces the chart to take at least this much space

            color: "#1e293b"
            radius: 8
            border.color: "#334155"
            clip: true

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
                    max: 100 // We'll update this dynamically
                    titleText: "Altitude (m)"
                }

                ValueAxis {
                    id: axisX
                    min: 0
                    max: 50 // Show last 50 data points
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
