#ifndef RENDERSTATE
#define RENDERSTATE

#include <QMatrix>
#include "light.h"

struct RenderState {
    QMatrix4x4 projection_matrix;
    float opacity;
    bool visible;
    float env_alpha;

    struct RSLight {
        Light *light;
        QVector3D final_pos;
        bool pos_dirty;
        bool amb_dirty;
        bool dif_dirty;
        bool spec_dirty;
    };
    QVector<RSLight> lights;

    bool projection_matrix_dirty;
    bool opacity_dirty;
    bool env_alpha_dirty;

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

    void setEnvAlpha(float value) {
        if (env_alpha != value) {
            env_alpha = value;
            env_alpha_dirty = true;
        }
    }

    void setLightPos(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        lights[i].light->pos = value;
    }

    void setLightFinalPos(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].final_pos != value) {
            lights[i].final_pos = value;
            lights[i].pos_dirty = true;
        }
    }

    void setLightAmb(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].light->amb != value) {
            lights[i].light->amb = value;
            lights[i].amb_dirty = true;
        }
    }

    void setLightDif(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].light->dif != value) {
            lights[i].light->dif = value;
            lights[i].dif_dirty = true;
        }
    }

    void setLightSpec(int i, const QVector3D &value) {
        Q_ASSERT(i >= 0 && i < lights.size());

        if (lights[i].light->spec != value) {
            lights[i].light->spec = value;
            lights[i].spec_dirty = true;
        }
    }

    void setDirty() {
        projection_matrix_dirty = true;
        opacity_dirty = true;
        env_alpha_dirty = true;
        for (int i = 0; i < lights.size(); i++) {
            lights[i].pos_dirty = true;
            lights[i].amb_dirty = true;
            lights[i].dif_dirty = true;
            lights[i].spec_dirty = true;
        }
    }

    void resetDirty() {
        projection_matrix_dirty = false;
        opacity_dirty = false;
        env_alpha_dirty = false;
        for (int i = 0; i < lights.size(); i++) {
            lights[i].pos_dirty = false;
            lights[i].amb_dirty = false;
            lights[i].dif_dirty = false;
            lights[i].spec_dirty = false;
        }
    }
};

#endif // RENDERSTATE

