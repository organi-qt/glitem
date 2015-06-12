import QtQuick 2.2
import QtQuick.Window 2.1
import GLItem 1.0

Window {
    width : 800
    height : 480
    color: "black"
    visible: true

    GLItem {
        anchors.fill: parent

        environment: GLEnvironment {
            top: "model/textures/garage/positiveY.jpg"
            bottom: "model/textures/garage/negativeY.jpg"
            left: "model/textures/garage/negativeX.jpg"
            right: "model/textures/garage/positiveX.jpg"
            front: "model/textures/garage/negativeZ.jpg"
            back: "model/textures/garage/positiveZ.jpg"
        }

        glmodel: [
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_body.json"
                material: body
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_glass.json"
                material: glass
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_bumper.json"
                material: bumper
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.rim.json"
                material: bumper
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.wheel.json"
                material: bumper
            }
        ]

        glmaterial: [
            GLPhongMaterial {
                id: body
                color: "#990000"
                reflectivity: 0.5
                envMap: true
                specular: "#CCCCCC"
            },
            GLPhongMaterial {
                id: glass
                color: "#FFFFFF"
                reflectivity: 1.0
                envMap: true
                specular: "#CCCCCC"
            },
            GLPhongMaterial {
                id: bumper
                color: "#EEEEEE"
                reflectivity: 0.5
                envMap: true
                specular: "#777777"
            }
        ]

        GLAnimateNode {
            //name: "view"
            name: "model"
            transform: [
                GLTranslation{translate: Qt.vector3d(0, -200, -1000)},
                //GLScale{scale: 1.0 / 100},
                GLRotation {
                    id: rot
                    axis: Qt.vector3d(0, 1, 0)
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
}
