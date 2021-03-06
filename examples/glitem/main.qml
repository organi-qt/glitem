import QtQuick 2.2
import QtQuick.Window 2.1
import GLItem 1.1

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
        antialiasing: true
        NumberAnimation on rotation {
            from: 0
            to: 360
            duration: 20000
            running: true
            loops: Animation.Infinite
        }
    }

    GLItem {
        id: car
        x: 50
        y: 50
        width: 700
        height: 400
        //asynchronous: false

        glmodel: [
            GLAssimpLoadModel {
                file: "qrc:/model/car.dae"
                //file: "model/SWIM/Car Swim N220711.3DS"
                //ignoreLight: true
                //material: body
            }
        ]
//*
        environment: GLEnvironment {
            top: "model/lake/lake1_up.JPG"
            bottom: "model/lake/lake1_dn.JPG"
            left: "model/lake/lake1_lf.JPG"
            right: "model/lake/lake1_rt.JPG"
            front: "model/lake/lake1_ft.JPG"
            back: "model/lake/lake1_bk.JPG"
        }
//*/
//*
        gllight: [
            GLDirectionalLight {
                name: "Sun"
                direction: Qt.vector3d(-1, -1, -1)
                diffuse: Qt.vector3d(0.5, 0.5, 0.5)
                specular: Qt.vector3d(0.5, 0.5, 0.5)
                view: true
            },
            GLPointLight {
                name: "Lamp"
                position: Qt.vector3d(1000, 1000, 1000)
                diffuse: Qt.vector3d(0.5, 0.5, 0.5)
                specular: Qt.vector3d(0.5, 0.5, 0.5)
                view: true
            }
        ]
//*/
        glmaterial: [
            GLPhongMaterial {
                id: body
                name: "_1___Default-material"
                color: "#990000"
                reflectivity: 0.1
                envMap: true
                specular: "#CCCCCC"
            },
            GLPhongMaterial {
                id: glass
                name: "_1-material"
                color: "#FFFFFF"
                reflectivity: 1.0
                envMap: true
                specular: "#CCCCCC"
                transparent: true
                opacity: 0.4
            },
            GLPhongMaterial {
                id: light
                name: "_5___Default2-material"
                color: "#FFFFFF"
                reflectivity: 1.0
                envMap: true
                specular: "#CCCCCC"
                transparent: true
                opacity: 0.4
            }
        ]

        GLAnimateNode {
            //name: "view"
            name: "model"
            transform: [
                GLTranslation{translate: Qt.vector3d(0, -1.5, -7)},
                //GLScale{scale: 1.0 / 1200},
                GLRotation {
                    id: rot
                    axis: Qt.vector3d(0, 1, 0)
                }
            ]
        }

        GLAnimateNode {
            name: "mesh3d_003"
            transform: [
                GLTranslation {
                    id: trans
                    //translate: Qt.vector3d(5, 0, 0)
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
/*
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
//*/
/*
    SequentialAnimation {
        NumberAnimation {
            target: car
            property: "opacity"
            from: 1
            to: 0
            duration: 4000
        }
        NumberAnimation {
            target: car
            property: "opacity"
            from: 0
            to: 1
            duration: 4000
        }

        running: true
        loops: Animation.Infinite
    }
//*/
/*
    SequentialAnimation {

        PauseAnimation {
            duration: 3000
        }

        PropertyAction {
            target: car
            property: "visible"
            value: false
        }

        PauseAnimation {
            duration: 3000
        }

        PropertyAction {
            target: car
            property: "visible"
            value: true
        }

        running: true
        loops: Animation.Infinite
    }
//*/
/*
    SequentialAnimation {
        NumberAnimation {
            target: car
            property: "width"
            from: 700
            to: 50
            duration: 3000
        }
        NumberAnimation {
            target: car
            property: "height"
            from: 400
            to: 0
            duration: 3000
        }
        NumberAnimation {
            target: car
            property: "height"
            from: 0
            to: 400
            duration: 3000
        }
        NumberAnimation {
            target: car
            property: "width"
            from: 50
            to: 700
            duration: 3000
        }

        running: true
        loops: Animation.Infinite
    }
//*/
/*
    SequentialAnimation {
        NumberAnimation {
            target: car
            property: "x"
            from: 50
            to: 700
            duration: 3000
        }
        NumberAnimation {
            target: car
            property: "y"
            from: 50
            to: 480
            duration: 3000
        }
        NumberAnimation {
            target: car
            property: "y"
            from: 480
            to: 50
            duration: 3000
        }
        NumberAnimation {
            target: car
            property: "x"
            from: 700
            to: 50
            duration: 3000
        }

        running: true
        loops: Animation.Infinite
    }
//*/
}
