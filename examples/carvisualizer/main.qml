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
            top: "model/t_01/g_01/py.jpg"
            bottom: "model/t_01/g_01/ny.jpg"
            left: "model/t_01/g_01/nx.jpg"
            right: "model/t_01/g_01/px.jpg"
            front: "model/t_01/g_01/nz.jpg"
            back: "model/t_01/g_01/pz.jpg"
        }

        gllight: [
            GLLight {
                name: "Sun"
                pos: Qt.vector3d(-1, 1, -1)
                amb: Qt.vector3d(0, 0, 0)
                dif: Qt.vector3d(0.5, 0.5, 0.5)
                spec: Qt.vector3d(0.5, 0.5, 0.5)
            }
        ]

        glgeometry: [
            GLLoadGeometry {
                file: "model/m_01/a_01/c_01/b01.json"
                material: body
            }
        ]

        glmaterial: [
            GLPhongMaterial {
                id: body
                color: "#990000"
                reflectivity: 0.5
                envMap: true
                specular: "#CCCCCC"
            }
        ]
    }
}
