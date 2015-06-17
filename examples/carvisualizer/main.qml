import QtQuick 2.2
import QtQuick.Window 2.1
import GLItem 1.1

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

        gllight: [
            GLPointLight {
                name: "Lamp"
                position: Qt.vector3d(1000, 1000, 1000)
                diffuse: Qt.vector3d(1, 1, 1)
                specular: Qt.vector3d(1, 1, 1)
            }
        ]

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
                file: "model/models/carvisualizer.ferrari_interior.json"
                material: interior
            },
            GLJSONLoadModel {
                name: "wheel"
                file: "model/models/carvisualizer.rim.json"
                material: rim
                node: 4
            },
            GLJSONLoadModel {
                name: "wheel"
                file: "model/models/carvisualizer.wheel.json"
                material: tyre
                node: 4
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.garage.json"
                material: garage
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.floor.json"
                material: floor
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.floor_shadow.json"
                material: floor_shadow
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.car_shadow.json"
                material: car_shadow
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
                transparent: true
                opacity: 0.4
            },
            GLPhongMaterial {
                id: bumper
                color: "#333333"
                reflectivity: 0.5
                envMap: true
                specular: "#777777"
            },
            GLPhongMaterial {
                id: rim
                color: "#EEEEEE"
                reflectivity: 0.5
                envMap: true
                specular: "#777777"
            },
            GLBasicMaterial {
                id: tyre
                map: "model/textures/autoparts/wheel.png"
                transparent: true
            },
            GLBasicMaterial {
                id: interior
                map: "model/textures/california/i01.jpg"
            },
            GLBasicMaterial {
                id: garage
                map: "model/textures/garage/garage.jpg"
            },
            GLBasicMaterial {
                id: floor
                map: "model/textures/garage/floor.jpg"
            },
            GLBasicMaterial {
                id: floor_shadow
                map: "model/textures/garage/floorShadow.png"
                transparent: true
            },
            GLBasicMaterial {
                id: car_shadow
                map: "model/textures/california/s01.png"
                transparent: true
            }
        ]

        GLAnimateNode {
            //name: "view"
            name: "model"
            transform: [
                GLTranslation { translate: Qt.vector3d(0, -300, -1000) },
                //GLScale { scale: 1.0 / 100 },
                GLRotation {
                    id: rot
                    axis: Qt.vector3d(0, 1, 0)
                }
            ]
        }

        GLAnimateNode {
            name: "wheel0"
            transform: [
                GLTranslation { translate: Qt.vector3d(148.5, 0, 265.4) },
                GLRotation {
                    axis: Qt.vector3d(0, 1, 0)
                    angle: 180
                }
            ]
        }

        GLAnimateNode {
            name: "wheel1"
            transform: [
                GLTranslation { translate: Qt.vector3d(-151.1, 0, 265.4) }
            ]
        }

        GLAnimateNode {
            name: "wheel2"
            transform: [
                GLTranslation { translate: Qt.vector3d(148.5, 0, -243.9) },
                GLRotation {
                    axis: Qt.vector3d(0, 1, 0)
                    angle: 180
                }
            ]
        }

        GLAnimateNode {
            name: "wheel3"
            transform: [
                GLTranslation { translate: Qt.vector3d(-151.1, 0, -243.9) }
            ]
        }
    }

    NumberAnimation {
        target: rot
        property: "angle"
        from: 0
        to: 360
        duration: 15000
        running: true
        loops: Animation.Infinite
    }
}
