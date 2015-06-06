#include "glitemqmlplugin.h"
#include "glitem.h"


void GLItemQmlPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("GLItem"));
    qmlRegisterType<GLItem>("GLItem", 1, 0, "GLItem");

    qmlRegisterType<GLScale>("GLItem", 1, 0, "GLScale");
    qmlRegisterType<GLTranslation>("GLItem", 1, 0, "GLTranslation");
    qmlRegisterType<GLRotation>("GLItem", 1, 0, "GLRotation");
    qmlRegisterType<GLTransform>();

    qmlRegisterType<GLAnimateNode>("GLItem", 1, 0, "GLAnimateNode");
    qmlRegisterType<GLLight>("GLItem", 1, 0, "GLLight");
    qmlRegisterType<GLEnvironment>("GLItem", 1, 0, "GLEnvironment");

    qmlRegisterType<GLLoadGeometry>("GLItem", 1, 0, "GLLoadGeometry");
    qmlRegisterType<GLGeometry>();
    qmlRegisterType<GLPhongMaterial>("GLItem", 1, 0, "GLPhongMaterial");
    qmlRegisterType<GLMaterial>();
}
