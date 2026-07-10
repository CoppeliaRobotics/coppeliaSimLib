#include <propertiesAndMethods.h>
#include <vector>
#include <simLib/simConst.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_obj{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    OBJECT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropObject p)
{
    return allProps_obj[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_app{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    APP_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropApp p)
{
    return allProps_app[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_customObjectClass{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    CUSTOMOBJECTCLASS_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCustomObjectClass p)
{
    return allProps_customObjectClass[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_customObject{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    CUSTOMOBJECT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCustomObject p)
{
    return allProps_customObject[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_detachedScript{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    DETACHEDSCRIPT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropDetachedScript p)
{
    return allProps_detachedScript[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_stack{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    STACK_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropStack p)
{
    return allProps_stack[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_collCont{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    COLLECTIONCONT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCollectionCont p)
{
    return allProps_collCont[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_collection{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    COLLECTION_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCollection p)
{
    return allProps_collection[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_drawCont{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    DRAWINGOBJECTCONT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropDrawingObjectCont p)
{
    return allProps_drawCont[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_drawingObj{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    DRAWINGOBJECT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropDrawingObject p)
{
    return allProps_drawingObj[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_col{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    COLOR_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropColor p)
{
    return allProps_col[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_volume{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    CONVEXVOLUME_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropConvexVolume p)
{
    return allProps_volume[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_dyn{
#define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) SProperty{str, v1, v2, t1, {w0, w1, w2, w3, w4}},
    DYNCONT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropDynCont p)
{
    return allProps_dyn[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_scene{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    SCENE_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropScene p)
{
    return allProps_scene[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_sim{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    SIMULATION_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropSimulation p)
{
    return allProps_sim[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_meshWrap{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    MESHWRAPPER_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropMeshWrapper p)
{
    return allProps_meshWrap[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_mesh{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    MESH_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropMesh p)
{
    return allProps_mesh[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_viewable{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    VIEWABLEBASE_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropViewableBase p)
{
    return allProps_viewable[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_objCont{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    SCENEOBJECTCONT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropSceneObjectCont p)
{
    return allProps_objCont[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_material{
#define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) SProperty{str, v1, v2, t1, {w0, w1, w2, w3, w4}},
    DYNMATERIAL_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropMaterial p)
{
    return allProps_material[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_customSceneObjectClass{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    CUSTOMSCENEOBJECTCLASS_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCustomSceneObjectClass p)
{
    return allProps_customSceneObjectClass[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_sceneObject{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    SCENEOBJECT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropSceneObject p)
{
    return allProps_sceneObject[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_shape{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    SHAPE_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropShape p)
{
    return allProps_shape[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_camera{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    CAMERA_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCamera p)
{
    return allProps_camera[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_dummy{
#define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) SProperty{str, v1, v2, t1, {w0, w1, w2, w3, w4}},
    DUMMY_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropDummy p)
{
    return allProps_dummy[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_forceSensor{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    FORCESENSOR_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropForceSensor p)
{
    return allProps_forceSensor[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_graph{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    GRAPH_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropGraph p)
{
    return allProps_graph[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_joint{
#define FUNCX(name, str, v1, v2, t1, w0, w1, w2, w3, w4) SProperty{str, v1, v2, t1, {w0, w1, w2, w3, w4}},
    JOINT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropJoint p)
{
    return allProps_joint[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_light{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    LIGHT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropLight p)
{
    return allProps_light[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_ocTree{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    OCTREE_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropOctree p)
{
    return allProps_ocTree[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_pointCloud{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    POINTCLOUD_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropPointCloud p)
{
    return allProps_pointCloud[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_proximitySensor{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    PROXIMITYSENSOR_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropProximitySensor p)
{
    return allProps_proximitySensor[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_visionSensor{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    VISIONSENSOR_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropVisionSensor p)
{
    return allProps_visionSensor[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_script{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    SCRIPT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropScript p)
{
    return allProps_script[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_marker{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    MARKER_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropMarker p)
{
    return allProps_marker[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
const std::vector<SProperty> allProps_customSceneObject{
#define FUNCX(name, str, v1, v2, t1) SProperty{str, v1, v2, t1},
    CUSTOMSCENEOBJECT_PROPERTIES
#undef FUNCX
};
const SProperty& prop(PropCustomSceneObject p)
{
    return allProps_customSceneObject[static_cast<size_t>(p)];
}
// ----------------------------------------------------------------------------------------------

const std::vector<std::pair<std::string, SProperty*>> allProps = []{
    std::vector<std::pair<std::string, SProperty*>> result;
    for (const auto& prop : allProps_obj) result.push_back(std::make_pair("object", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_app) result.push_back(std::make_pair("app", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_detachedScript) result.push_back(std::make_pair("detachedScript", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_stack) result.push_back(std::make_pair("stack", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_collCont) result.push_back(std::make_pair("collection", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_collection) result.push_back(std::make_pair("collection", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_drawCont) result.push_back(std::make_pair("drawingObject", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_drawingObj) result.push_back(std::make_pair("drawingObject", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_volume) result.push_back(std::make_pair("proximitySensor", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_dyn) result.push_back(std::make_pair("scene", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_scene) result.push_back(std::make_pair("scene", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_sim) result.push_back(std::make_pair("scene", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_meshWrap) result.push_back(std::make_pair("shape", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_mesh) result.push_back(std::make_pair("mesh", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_objCont) result.push_back(std::make_pair("scene", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_material) result.push_back(std::make_pair("shape", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_sceneObject) result.push_back(std::make_pair("sceneObject", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_shape) result.push_back(std::make_pair("shape", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_camera) result.push_back(std::make_pair("camera", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_dummy) result.push_back(std::make_pair("dummy", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_forceSensor) result.push_back(std::make_pair("forceSensor", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_graph) result.push_back(std::make_pair("graph", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_joint) result.push_back(std::make_pair("joint", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_light) result.push_back(std::make_pair("light", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_ocTree) result.push_back(std::make_pair("ocTree", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_pointCloud) result.push_back(std::make_pair("pointCloud", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_proximitySensor) result.push_back(std::make_pair("proximitySensor", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_visionSensor) result.push_back(std::make_pair("visionSensor", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_script) result.push_back(std::make_pair("script", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_marker) result.push_back(std::make_pair("marker", const_cast<SProperty*>(&prop)));

    // Following special:
    for (const auto& prop : allProps_viewable) result.push_back(std::make_pair("camera", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_viewable) result.push_back(std::make_pair("visionSensor", const_cast<SProperty*>(&prop)));

    for (const auto& prop : allProps_col) result.push_back(std::make_pair("color", const_cast<SProperty*>(&prop)));

    for (const auto& prop : allProps_customObject) result.push_back(std::make_pair("customObject", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_customObjectClass) result.push_back(std::make_pair("customObject", const_cast<SProperty*>(&prop)));

    for (const auto& prop : allProps_customSceneObject) result.push_back(std::make_pair("customSceneObject", const_cast<SProperty*>(&prop)));
    for (const auto& prop : allProps_customSceneObjectClass) result.push_back(std::make_pair("customSceneObject", const_cast<SProperty*>(&prop)));

    return result;
}();

std::map<std::string, std::vector<SDeprecatedProp>> createDeprecationMapping(int callingApiVer)
{
    std::map<std::string, std::vector<SDeprecatedProp>> result;
    for (size_t i = 0; i < allProps.size(); i++)
    {
        int startSupport = allProps[i].second->info.map[META_STARTSUPPORT].toInt();
        int startDeprecated = allProps[i].second->info.map[META_STARTDEPRECATED].toInt();
        int endSupport = allProps[i].second->info.map[META_ENDSUPPORT].toInt();
        if ((startSupport <= callingApiVer) && ((endSupport == 0) || (endSupport >= callingApiVer)))
        {
            std::string key = allProps[i].second->name;
            SDeprecatedProp p;
            p.type = allProps[i].first;
            if (allProps[i].second->info.map.contains(META_REPLACEDBY))
                p.replacement = allProps[i].second->info.map[META_REPLACEDBY].toString().toStdString();
            else if (allProps[i].second->info.map.contains(META_MIGRATETO))
                p.replacement = allProps[i].second->info.map[META_MIGRATETO].toString().toStdString() + DEPRECATION_NO_REPLACE;
            else
                p.replacement = DEPRECATION_NO_REPLACE;
            p.deprecated = (startDeprecated != 0) && (startDeprecated <= callingApiVer);
            auto it = result.find(key);
            if (it == result.end())
            {
                result[key] = {};
                it = result.find(key);
            }
            it->second.push_back(p);
        }
    }
    return result;
}

const std::map<std::string, std::vector<SDeprecatedProp>> propDeprecationMappings[2] = {createDeprecationMapping(1), createDeprecationMapping(2)};
