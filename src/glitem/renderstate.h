#ifndef RENDERSTATE
#define RENDERSTATE

#include <QMatrix4x4>
#include "light.h"

class QOpenGLTexture;

struct RenderState {
    QMatrix4x4 projection_matrix;
    float opacity;
    bool visible;

    QOpenGLTexture *envmap;
    QVector3D light_amb;

    struct RSLight {
        Light *light;
        QVector3D final_pos;
        bool final_pos_dirty;
    };
    QVector<RSLight> lights;

    bool projection_matrix_dirty;
    bool opacity_dirty;
    bool light_amb_dirty;

    void setProjectionMatrix(const QMatrix4x4 &value) {
        if (projection_matrix != value) {
            projection_matrix = value;
            projection_matrix_dirty = true;
        }
    }

    void setOpacity(float value) {
        if (opacity != value) {
            opacity = value;
            opacity_dirty = true;
        }
    }

    void setLightFinalPos(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].final_pos != value) {
            lights[i].final_pos = value;
            lights[i].final_pos_dirty = true;
        }
    }

    void setLightAmb(const QVector3D &value) {
        if (light_amb != value) {
            light_amb = value;
            light_amb_dirty = true;
        }
    }

    void setDirty() {
        projection_matrix_dirty = true;
        opacity_dirty = true;
        light_amb_dirty = true;
        for (int i = 0; i < lights.size(); i++) {
            lights[i].final_pos_dirty = true;
            lights[i].light->dif_dirty = true;
            lights[i].light->spec_dirty = true;
        }
    }

    void resetDirty() {
        projection_matrix_dirty = false;
        opacity_dirty = false;
        light_amb_dirty = false;
        for (int i = 0; i < lights.size(); i++) {
            lights[i].final_pos_dirty = false;
            lights[i].light->dif_dirty = false;
            lights[i].light->spec_dirty = false;
        }
    }
};

#endif // RENDERSTATE

