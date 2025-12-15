#include <propertiesAndMethods.h>
#include <vector>
#include <simLib/simConst.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#define STR(x) #x

static std::string jsonStr(const QJsonObject& obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact).toStdString();
}

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    COLLECTION_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_collection = {COLLECTION_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    COLOR_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_col = {COLOR_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    DRAWINGOBJECT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_drawingObj = {DRAWINGOBJECT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    COLLECTIONCONT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_collCont = {COLLECTIONCONT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    DRAWINGOBJECTCONT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_drawCont = {DRAWINGOBJECTCONT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) extern const SDynProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
    DYNCONT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
    extern const std::vector<SDynProperty> allProps_dyn = {DYNCONT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    ENVIRONMENT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_scene = {ENVIRONMENT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    SCENEOBJECTCONT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_objCont = {SCENEOBJECTCONT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    SIMULATION_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_sim = {SIMULATION_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    CAMERA_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_camera = {CAMERA_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) extern const SDummyProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
    DUMMY_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
    extern const std::vector<SDummyProperty> allProps_dummy = {DUMMY_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    FORCESENSOR_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_forceSensor = {FORCESENSOR_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    GRAPH_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_graph = {GRAPH_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) extern const SJointProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
    JOINT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
    extern const std::vector<SJointProperty> allProps_joint = {JOINT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    LIGHT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_light = {LIGHT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    MIRROR_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_mirror = {MIRROR_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    OCTREE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_ocTree = {OCTREE_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    POINTCLOUD_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_pointCloud = {POINTCLOUD_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    PROXIMITYSENSOR_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_proximitySensor = {PROXIMITYSENSOR_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    CONVEXVOLUME_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_volume = {CONVEXVOLUME_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    SCENEOBJECT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_sceneObject = {SCENEOBJECT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    VIEWABLEBASE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_viewable = {VIEWABLEBASE_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    SCRIPT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_script = {SCRIPT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    SHAPE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_shape = {SHAPE_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    MESH_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_mesh = {MESH_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    MESHWRAPPER_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_meshWrap = {MESHWRAPPER_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    VISIONSENSOR_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_visionSensor = {VISIONSENSOR_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    DETACHEDSCRIPT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_scriptObject = {DETACHEDSCRIPT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    APP_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_app = {APP_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) extern const SEngMaterialProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
    DYNMATERIAL_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
    extern const std::vector<SEngMaterialProperty> allProps_material = {DYNMATERIAL_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------
