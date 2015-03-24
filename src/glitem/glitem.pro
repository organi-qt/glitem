TARGET = glitemqmlplugin
TARGETPATH = GLItem
QT += qml quick

SOURCES += \
    glshader.cpp \
    glnode.cpp \
    glrender.cpp \
    glloader.cpp \
    gltransform.cpp \
    glanimatenode.cpp \
    glitem.cpp \
    glitemqmlplugin.cpp

HEADERS += \
    glshader.h \
    glnode.h \
    glrender.h \
    glloader.h \
    gltransform.h \
    glanimatenode.h \
    glitem.h \
    glitemqmlplugin.h

CONFIG += link_pkgconfig
PKGCONFIG += assimp

load(qml_plugin)
