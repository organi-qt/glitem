import QtQuick 2.2
import QtQuick.Window 2.1
import GLItem 1.1

Window {
    width: 1280
    height: 480
    color: "black"
    visible: true

    GLItem {
        id: root
        anchors.fill: parent
        property int car: 0

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
                position: Qt.vector3d(0, 0, 0)
                diffuse: Qt.vector3d(0.8, 0.8, 0.8)
                specular: Qt.vector3d(0.8, 0.8, 0.8)
                view: true
            },
            GLDirectionalLight {
                name: "Sun"
                direction: Qt.vector3d(0, -1, 0)
                diffuse: Qt.vector3d(0.8, 0.8, 0.8)
                specular: Qt.vector3d(0.8, 0.8, 0.8)
            }
        ]

        glmodel: [
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_body.json"
                material: body
                visible: root.car == 0
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_glass.json"
                material: glass
                visible: root.car == 0
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_bumper.json"
                material: bumper
                visible: root.car == 0
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.ferrari_interior.json"
                material: california_interior
                visible: root.car == 0
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.mercedes_body.js"
                material: body
                visible: root.car == 1
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.mercedes_glass.js"
                material: glass
                visible: root.car == 1
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.mercedes_bumper.js"
                material: bumper
                visible: root.car == 1
            },
            GLJSONLoadModel {
                file: "model/models/carvisualizer.mercedes_interior.js"
                material: mercedes_interior
                visible: root.car == 1
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
                material: {
                    switch (root.car) {
                    case 0:
                    default:
                        california_shadow
                        break
                    case 1:
                        mercedes_shadow
                        break
                    }
                }
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
                id: california_interior
                map: "model/textures/california/i01.jpg"
            },
            GLBasicMaterial {
                id: california_shadow
                map: "model/textures/california/s01.png"
                transparent: true
            },
            GLBasicMaterial {
                id: mercedes_interior
                map: "model/textures/mercedes/i01.jpg"
            },
            GLBasicMaterial {
                id: mercedes_shadow
                map: "model/textures/mercedes/s01.png"
                transparent: true
            }
        ]

        GLAnimateNode {
            //name: "view"
            name: "model"
            transform: [
                GLTranslation { translate: Qt.vector3d(0, -250, -800) },
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
                GLTranslation { id: wheel0; translate: Qt.vector3d(148.5, 0, 265.4) },
                GLRotation {
                    axis: Qt.vector3d(0, 1, 0)
                    angle: 180
                }
            ]
        }

        GLAnimateNode {
            name: "wheel1"
            transform: [
                GLTranslation { id: wheel1; translate: Qt.vector3d(-151.1, 0, 265.4) }
            ]
        }

        GLAnimateNode {
            name: "wheel2"
            transform: [
                GLTranslation { id: wheel2; translate: Qt.vector3d(148.5, 0, -243.9) },
                GLRotation {
                    axis: Qt.vector3d(0, 1, 0)
                    angle: 180
                }
            ]
        }

        GLAnimateNode {
            name: "wheel3"
            transform: [
                GLTranslation { id: wheel3; translate: Qt.vector3d(-151.1, 0, -243.9) }
            ]
        }

        state: "california"
        states: [
            State {
                name: "california"
                PropertyChanges { target: root; car: 0 }
                PropertyChanges { target: wheel0; translate: Qt.vector3d(148.5, 0, 265.4) }
                PropertyChanges { target: wheel1; translate: Qt.vector3d(-151.1, 0, 265.4) }
                PropertyChanges { target: wheel2; translate: Qt.vector3d(148.5, 0, -243.9) }
                PropertyChanges { target: wheel3; translate: Qt.vector3d(-151.1, 0, -243.9) }
            },
            State {
                name: "mercedes"
                PropertyChanges { target: root; car: 1 }
                PropertyChanges { target: wheel0; translate: Qt.vector3d(154.5, 0, 268.6) }
                PropertyChanges { target: wheel1; translate: Qt.vector3d(-155.5, 0, 268.6) }
                PropertyChanges { target: wheel2; translate: Qt.vector3d(154.5, 0, -261.8) }
                PropertyChanges { target: wheel3; translate: Qt.vector3d(-155.5, 0, -261.8) }
            }
        ]
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

    Timer {
        interval: 15000; running: false; repeat: true
        onTriggered: {
            switch (root.state) {
            case "california":
                root.state = "mercedes"
                break
            case "mercedes":
            default:
                root.state = "california"
                break
            }
        }
    }
}
