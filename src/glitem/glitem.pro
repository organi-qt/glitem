TARGET = glitemqmlplugin
TARGETPATH = GLItem
QT += qml quick

SOURCES += \
    glshader.cpp \
    glnode.cpp \
    glrender.cpp \
    gltransform.cpp \
    glitem.cpp \
    glitemqmlplugin.cpp \
    glanimatenode.cpp \
    gllight.cpp \
    ailoaderiostream.cpp \
    ailoaderiosystem.cpp \
    glenvironment.cpp \
    glmaterial.cpp \
    glassimploadmodel.cpp \
    glmodel.cpp \
    gljsonloadmodel.cpp \
    material.cpp \
    gldatamodel.cpp

HEADERS += \
    glshader.h \
    glnode.h \
    glrender.h \
    gltransform.h \
    glitem.h \
    glitemqmlplugin.h \
    gllight.h \
    glanimatenode.h \
    ailoaderiostream.h \
    ailoaderiosystem.h \
    glmaterial.h \
    glenvironment.h \
    material.h \
    glmodel.h \
    gljsonloadmodel.h \
    glassimploadmodel.h \
    mesh.h \
    light.h \
    renderstate.h \
    gldatamodel.h

CONFIG += link_pkgconfig
PKGCONFIG += assimp

load(qml_plugin)
