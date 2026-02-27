// Gauge.qml
// Custom Circular Gauge for Qt 6 (replaces Qt5 QtQuick.Extras CircularGauge)
// Loaded via QQuickWidget; bound to 'backend' context property (MainWindow).

import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root

    // ── Public properties (bound from C++ via backend context property) ────
    property real value:        backend ? backend.currentTemperature : 0
    property real threshold:    backend ? backend.threshold          : 50
    property real minimumValue: 0
    property real maximumValue: 100
    property string unit:       "°C"
    property string label:      "TEMPERATURE"

    // Derived
    property real normalizedValue: Math.max(0, Math.min(1,
        (value - minimumValue) / (maximumValue - minimumValue)))

    // Needle sweep: –220° → +40°  (260° total arc)
    property real minAngle: -220
    property real maxAngle:   40
    property real needleAngle: minAngle + normalizedValue * (maxAngle - minAngle)

    property color arcColor: value >= threshold ? "#e74c3c" : "#2ecc71"

    implicitWidth:  320
    implicitHeight: 320

    // ── Dark background circle ────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "#1a1a2e"
        border.color: "#0f3460"
        border.width: 4
    }

    // ── Gradient arc (Canvas) ─────────────────────────────────────────────
    Canvas {
        id: arcCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var cx   = width  / 2;
            var cy   = height / 2;
            var r    = Math.min(width, height) / 2 - 20;
            var lw   = 16;

            // Angle convention: canvas 0 rad = 3 o'clock; we offset by –90°
            var startRad = (root.minAngle - 90) * Math.PI / 180;
            var endRad   = (root.maxAngle  - 90) * Math.PI / 180;

            // Gray track
            ctx.beginPath();
            ctx.arc(cx, cy, r, startRad, endRad, false);
            ctx.strokeStyle = "#2d2d44";
            ctx.lineWidth   = lw;
            ctx.lineCap     = "round";
            ctx.stroke();

            // Colored value arc
            if (root.normalizedValue > 0.005) {
                var valueRad = startRad + root.normalizedValue * (endRad - startRad);
                var grad = ctx.createLinearGradient(cx - r, cy, cx + r, cy);
                grad.addColorStop(0.0, "#2ecc71");
                grad.addColorStop(0.6, "#f39c12");
                grad.addColorStop(1.0, "#e74c3c");
                ctx.beginPath();
                ctx.arc(cx, cy, r, startRad, valueRad, false);
                ctx.strokeStyle = grad;
                ctx.lineWidth   = lw;
                ctx.lineCap     = "round";
                ctx.stroke();
            }

            // White threshold tick
            var tn = Math.max(0, Math.min(1,
                (root.threshold - root.minimumValue) /
                (root.maximumValue - root.minimumValue)));
            var tr  = startRad + tn * (endRad - startRad);
            var ri  = r - lw / 2 - 6;
            var ro  = r + lw / 2 + 6;
            ctx.beginPath();
            ctx.moveTo(cx + ri * Math.cos(tr), cy + ri * Math.sin(tr));
            ctx.lineTo(cx + ro * Math.cos(tr), cy + ro * Math.sin(tr));
            ctx.strokeStyle = "#ffffff";
            ctx.lineWidth   = 3;
            ctx.stroke();
        }

        Connections {
            target: root
            function onNormalizedValueChanged() { arcCanvas.requestPaint(); }
            function onThresholdChanged()       { arcCanvas.requestPaint(); }
        }
    }

    // ── Tick marks + labels ───────────────────────────────────────────────
    Repeater {
        id: ticks
        model: 11   // 0 … 100 in steps of 10

        delegate: Item {
            property real frac:  index / (ticks.model - 1)
            property real ang:   root.minAngle + frac * (root.maxAngle - root.minAngle)
            property real angR:  (ang - 90) * Math.PI / 180
            property real trackR: Math.min(root.width, root.height) / 2 - 20

            // Tick
            x: root.width  / 2 + Math.cos(angR) * (trackR - 28) - 1
            y: root.height / 2 + Math.sin(angR) * (trackR - 28) - 5

            Rectangle {
                width: 2; height: 10
                color: "#888888"
                anchors.centerIn: parent
                rotation: ang
            }

            // Label
            Text {
                text: Math.round(root.minimumValue +
                      frac * (root.maximumValue - root.minimumValue))
                color: "#aaaaaa"
                font.pixelSize: 9
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 14
            }
        }
    }

    // ── Needle ────────────────────────────────────────────────────────────
    Item {
        anchors.fill: parent

        // Needle body
        Rectangle {
            id: needle
            width: 4
            height: root.height / 2 - 44
            radius: 2
            color: "#e74c3c"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.verticalCenter
            transformOrigin: Item.Bottom
        }

        // Pivot cap
        Rectangle {
            width: 14; height: 14; radius: 7
            color: "#ffffff"
            anchors.centerIn: parent
        }

        rotation: root.needleAngle
        Behavior on rotation {
            NumberAnimation { duration: 450; easing.type: Easing.OutCubic }
        }
    }

    // ── Centre value text ─────────────────────────────────────────────────
    Column {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 56
        spacing: 4

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.value.toFixed(1) + " " + root.unit
            color: root.arcColor
            font.pixelSize: 28
            font.bold: true
            Behavior on color { ColorAnimation { duration: 300 } }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            color: "#666688"
            font.pixelSize: 10
            font.letterSpacing: 2
        }
    }

    // ── LED badge ─────────────────────────────────────────────────────────
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 18
        width: 96; height: 26; radius: 13
        color: root.value >= root.threshold ? "#c0392b" : "#27ae60"
        Behavior on color { ColorAnimation { duration: 300 } }

        Text {
            anchors.centerIn: parent
            text: root.value >= root.threshold ? "LED: ON" : "LED: OFF"
            color: "white"
            font.pixelSize: 12
            font.bold: true
        }
    }
}
