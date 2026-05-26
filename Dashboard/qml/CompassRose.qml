import QtQuick

// Heading compass rose.
// The degree ring rotates under a fixed triangle marker at 12 o'clock,
// so the marker always reads the drone's current heading.
Item {
    id: root

    width: 200
    height: 200

    // Heading in degrees [0, 360). Drives ring rotation.
    property real heading: 0.0

    onHeadingChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            var cx = width / 2
            var cy = height / 2
            // Slight inset so the border stroke is not clipped at the edge.
            var radius = Math.min(cx, cy) - 2

            // Clear the previous frame before redrawing.
            ctx.clearRect(0, 0, width, height)

            // ── Clip everything to a circle ──────────────────────────────────
            ctx.beginPath()
            ctx.arc(cx, cy, radius, 0, 2 * Math.PI)
            ctx.clip()

            // ── Background fill ───────────────────────────────────────────────
            ctx.fillStyle = "#1e293b"
            ctx.fillRect(0, 0, width, height)

            // ── Rotating ring (ticks + cardinal labels) ───────────────────────
            // Translate to center, then rotate by -heading so the ring
            // counter-rotates as the drone turns: when heading = 90° the "E"
            // label moves up to the 12 o'clock position under the marker.
            ctx.save()
            ctx.translate(cx, cy)
            ctx.rotate(-root.heading * Math.PI / 180)

            // Tick marks every 10°; longer ticks every 30° (every 3rd mark).
            // North-up coordinate convention: x = r·sin(a), y = -r·cos(a)
            // so that 0° points up (north) rather than right (canvas default).
            for (var i = 0; i < 36; i++) {
                var angle   = i * 10 * Math.PI / 180
                var tickLen = (i % 3 === 0) ? 18 : 10
                var outerR  = radius - 4
                var innerR  = outerR - tickLen

                ctx.strokeStyle = "white"
                ctx.lineWidth   = (i % 3 === 0) ? 2 : 1
                ctx.beginPath()
                ctx.moveTo(outerR * Math.sin(angle), -outerR * Math.cos(angle))
                ctx.lineTo(innerR * Math.sin(angle), -innerR * Math.cos(angle))
                ctx.stroke()
            }

            // Cardinal labels at N / E / S / W.
            // "N" is red — standard aviation convention for quick identification.
            var cardinals = [
                { label: "N", angle: 0   },
                { label: "E", angle: 90  },
                { label: "S", angle: 180 },
                { label: "W", angle: 270 }
            ]
            var labelR = radius - 30
            ctx.font         = "bold 13px sans-serif"
            ctx.textAlign    = "center"
            ctx.textBaseline = "middle"
            cardinals.forEach(function(c) {
                var a = c.angle * Math.PI / 180
                ctx.fillStyle = (c.label === "N") ? "#ef4444" : "white"
                ctx.fillText(c.label, labelR * Math.sin(a), -labelR * Math.cos(a))
            })

            ctx.restore()   // back to fixed (non-rotating) frame

            // ── Fixed triangle marker at 12 o'clock ──────────────────────────
            // Drawn after restore so it never rotates — it always points up,
            // indicating the heading value on the ring beneath it.
            ctx.fillStyle = "#f59e0b"   // amber — visible against both light and dark ring segments
            ctx.beginPath()
            ctx.moveTo(cx,     cy - radius + 6)   // tip (pointing down into the ring)
            ctx.lineTo(cx - 8, cy - radius + 20)  // bottom-left
            ctx.lineTo(cx + 8, cy - radius + 20)  // bottom-right
            ctx.closePath()
            ctx.fill()

            // ── Heading readout in the center ────────────────────────────────
            // Normalise to [0, 360) and zero-pad to 3 digits ("045°" not "45°").
            var deg = ((Math.round(root.heading) % 360) + 360) % 360
            ctx.fillStyle    = "white"
            ctx.font         = "bold 16px monospace"
            ctx.textAlign    = "center"
            ctx.textBaseline = "middle"
            ctx.fillText(deg.toString().padStart(3, "0") + "°", cx, cy)

            // ── Outer border ring ─────────────────────────────────────────────
            // Drawn last so it sits on top of all other content.
            ctx.strokeStyle = "#334155"
            ctx.lineWidth   = 3
            ctx.beginPath()
            ctx.arc(cx, cy, radius, 0, 2 * Math.PI)
            ctx.stroke()
        }
    }
}
