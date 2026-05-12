import QtQuick

Item {
    id: root

    property real roll: 0.0
    property real pitch: 0.0

    readonly property real pixelsPerDegree: height / 60.0

    width: 200
    height: 200

    // repaint whenever roll or pitch changes
    onRollChanged: canvas.requestPaint()
    onPitchChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            var cx = width / 2
            var cy = height / 2
            var radius = Math.min(cx, cy) - 2

            ctx.clearRect(0, 0, width, height)

            ctx.save()
            ctx.beginPath()
            ctx.arc(cx, cy, radius, 0, 2 * Math.PI)
            ctx.clip()

            ctx.translate(cx,cy)
            ctx.rotate(root.roll * Math.PI / 180)
            ctx.translate(0, root.pitch * root.pixelsPerDegree)

            ctx.fillStyle = "#1a6ca8"
            ctx.fillRect(-width, -height, width * 2, height)

            ctx.fillStyle = "#7a4e2d"
            ctx.fillRect(-width, 0, width * 2, height)

            ctx.strokeStyle = "white"
            ctx.lineWidth = 2
            ctx.beginPath()
            ctx.moveTo(-width, 0)
            ctx.lineTo(width, 0)
            ctx.stroke()

            ctx.restore()

            // --- Fixed aircraft symbol (drawn after restore — never rotates) ---
           // Left wing
            ctx.strokeStyle = "white"
            ctx.lineWidth=3
            ctx.beginPath()
            ctx.moveTo(cx - 40, cy)
            ctx.lineTo(cx-12,cy)
            ctx.stroke()

            // Right wing
            ctx.beginPath()
            ctx.moveTo(cx + 12, cy)
            ctx.lineTo(cx+40,cy)
            ctx.stroke()

            // Centre dot
            ctx.fillStyle="white"
            ctx.beginPath()
            ctx.arc(cx, cy,3,0,2*Math.PI)
            ctx.fill()

            // --- Border circle ---
            ctx.strokeStyle="#334155"
            ctx.lineWidth = 3
            ctx.beginPath()
            ctx.arc(cx,cy,radius,0,2*Math.PI)
            ctx.stroke()
        }
    }
}
