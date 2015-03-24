import QtQuick 2.2
import QtQuick.Window 2.1
import GLItem 1.0

Window {
    width : 800
    height : 480
    color: "black"
    visible: true

    Rectangle {
        x: 100
        y: 100
        width: 200
        height: 200
        opacity: 0.5
        color: "green"
    }

    GLItem {
        id: car
        x: 50
        y: 50
        width: 700
        height: 400
        model: "model/car20150317.3DS"
        light_amb: Qt.vector3d(0.5, 0.5, 0.5)
        gltransform: [
            GLTranslation{translate: Qt.vector3d(0, -1.5, -7)},
            GLRotation {
                id: rot
                axis: Qt.vector3d(0, 1, 0)
            }
        ]

        GLAnimateNode {
            name: "mesh3d_003"
            transform: [
                GLTranslation {
                    id: trans
                    translate: Qt.vector3d(5, 0, 0)
                }
            ]
        }

        GLAnimateNode {
            name: "mesh3d_053"
            transform: [
                GLScale {
                    id: scl
                }
            ]
        }
    }

    NumberAnimation {
        target: rot
        property: "angle"
        from: 0
        to: 360
        duration: 10000
        running: true
        loops: Animation.Infinite
    }

    SequentialAnimation {
        NumberAnimation {
            target: scl
            property: "scale"
            from: 1
            to: 10
            duration: 5000
        }
        NumberAnimation {
            target: scl
            property: "scale"
            from: 10
            to: 1
            duration: 5000
        }

        running: true
        loops: Animation.Infinite
    }

    SequentialAnimation {
        NumberAnimation {
            target: trans
            property: "progress"
            from: 0
            to: 1
            duration: 3000
        }
        NumberAnimation {
            target: trans
            property: "progress"
            from: 1
            to: 0
            duration: 3000
        }

        running: true
        loops: Animation.Infinite
    }
/*
    SequentialAnimation {
        NumberAnimation {
            target: car
            property: "glopacity"
            from: 1
            to: 0
            duration: 4000
        }
        NumberAnimation {
            target: car
            property: "glopacity"
            from: 0
            to: 1
            duration: 4000
        }

        running: true
        loops: Animation.Infinite
    }
*/
}
