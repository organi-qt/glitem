#include "glitemqmlplugin.h"
#include "glitem.h"
#include "gljsonloadmodel.h"
#include "glassimploadmodel.h"
#include "gltransform.h"
#include "glmaterial.h"
#include "glanimatenode.h"
#include "gllight.h"
#include "glenvironment.h"


void GLItemQmlPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("GLItem"));
    qmlRegisterType<GLItem>("GLItem", 1, 1, "GLItem");

    qmlRegisterType<GLScale>("GLItem", 1, 0, "GLScale");
    qmlRegisterType<GLTranslation>("GLItem", 1, 0, "GLTranslation");
    qmlRegisterType<GLRotation>("GLItem", 1, 0, "GLRotation");
    qmlRegisterType<GLTransform>();

    qmlRegisterType<GLAnimateNode>("GLItem", 1, 0, "GLAnimateNode");
    qmlRegisterType<GLLight>("GLItem", 1, 1, "GLLight");
    qmlRegisterType<GLEnvironment>("GLItem", 1, 1, "GLEnvironment");

    qmlRegisterType<GLJSONLoadModel>("GLItem", 1, 1, "GLJSONLoadModel");
    qmlRegisterType<GLAssimpLoadModel>("GLItem", 1, 1, "GLAssimpLoadModel");
    qmlRegisterType<GLModel>();
    qmlRegisterType<GLPhongMaterial>("GLItem", 1, 1, "GLPhongMaterial");
    qmlRegisterType<GLMaterial>();
}
