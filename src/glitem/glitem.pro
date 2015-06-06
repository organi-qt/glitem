TARGET = glitemqmlplugin
TARGETPATH = GLItem
QT += qml quick

SOURCES += \
    glshader.cpp \
    glnode.cpp \
    glrender.cpp \
    glloader.cpp \
    gltransform.cpp \
    glitem.cpp \
    glitemqmlplugin.cpp \
    glanimatenode.cpp \
    gllight.cpp \
    ailoaderiostream.cpp \
    ailoaderiosystem.cpp \
    glenvironment.cpp \
    glgeometry.cpp \
    glmaterial.cpp

HEADERS += \
    glshader.h \
    glnode.h \
    glrender.h \
    glloader.h \
    gltransform.h \
    glitem.h \
    glitemqmlplugin.h \
    gllight.h \
    glanimatenode.h \
    ailoaderiostream.h \
    ailoaderiosystem.h \
    glmaterial.h \
    glenvironment.h \
    glgeometry.h \
    material.h

CONFIG += link_pkgconfig
PKGCONFIG += assimp

load(qml_plugin)
