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

struct SDepr
{
    std::string type;
    SProperty oldProp;
    std::string newPropName;
};

const std::vector<SDepr>& getDeprecatedList()
{
    static const std::vector<SDepr> tmpDeprecated =
    {
        {"app", prop(PropApp::DEPRECATED_appArg1),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg2),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg3),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg4),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg5),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg6),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg7),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg8),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_appArg9),              std::string(prop(PropApp::appArgs).name) + DEPRECATION_NO_REPLACE},
        {"app", prop(PropApp::DEPRECATED_dongleID),             prop(PropApp::dongleID).name},
        {"app", prop(PropApp::DEPRECATED_machineIDX),           prop(PropApp::machineIDX).name},
        {"app", prop(PropApp::DEPRECATED_machineID0),           prop(PropApp::machineID0).name},
        {"app", prop(PropApp::DEPRECATED_machineID1),           prop(PropApp::machineID1).name},
        {"app", prop(PropApp::DEPRECATED_machineID2),           prop(PropApp::machineID2).name},
        {"app", prop(PropApp::DEPRECATED_machineID3),           prop(PropApp::machineID3).name},
        {"app", prop(PropApp::DEPRECATED_appDir),               prop(PropApp::appDir).name},
        {"app", prop(PropApp::DEPRECATED_tempDir),              prop(PropApp::tempDir).name},
        {"app", prop(PropApp::DEPRECATED_sceneTempDir),         prop(PropApp::sceneTempDir).name},
        {"app", prop(PropApp::DEPRECATED_settingsDir),          prop(PropApp::settingsDir).name},
        {"app", prop(PropApp::DEPRECATED_luaDir),               prop(PropApp::luaDir).name},
        {"app", prop(PropApp::DEPRECATED_pythonDir),            prop(PropApp::pythonDir).name},
        {"app", prop(PropApp::DEPRECATED_mujocoDir),            prop(PropApp::mujocoDir).name},
        {"app", prop(PropApp::DEPRECATED_systemDir),            prop(PropApp::systemDir).name},
        {"app", prop(PropApp::DEPRECATED_resourceDir),          prop(PropApp::resourceDir).name},
        {"app", prop(PropApp::DEPRECATED_addOnDir),             prop(PropApp::addOnDir).name},
        {"app", prop(PropApp::DEPRECATED_sceneDir),             prop(PropApp::sceneDir).name},
        {"app", prop(PropApp::DEPRECATED_modelDir),             prop(PropApp::modelDir).name},
        {"app", prop(PropApp::DEPRECATED_importExportDir),      prop(PropApp::importExportDir).name},

        {"scene", prop(PropScene::DEPRECATED_sceneIsLocked),                prop(PropScene::sceneIsLocked).name},
        {"scene", prop(PropScene::DEPRECATED_sceneUid),                     prop(PropScene::sceneUid).name},
        {"scene", prop(PropScene::DEPRECATED_sceneUidString),               prop(PropScene::sceneUidString).name},
        {"scene", prop(PropScene::DEPRECATED_scenePath),                    prop(PropScene::scenePath).name},
        {"scene", prop(PropSceneObjectCont::DEPRECATED_objectHandles),      std::string(prop(PropSceneObjectCont::objects).name) + DEPRECATION_NO_REPLACE},
        {"scene", prop(PropSceneObjectCont::DEPRECATED_orphanHandles),      std::string(prop(PropSceneObjectCont::orphans).name) + DEPRECATION_NO_REPLACE},
        {"scene", prop(PropSceneObjectCont::DEPRECATED_selectionHandles),   std::string(prop(PropSceneObjectCont::selection).name) + DEPRECATION_NO_REPLACE},
        {"scene", prop(PropSceneObjectCont::DEPRECATED_objectHandles),      std::string(prop(PropSceneObjectCont::objects).name) + DEPRECATION_NO_REPLACE},
        {"scene", prop(PropSceneObjectCont::DEPRECATED_orphanHandles),      std::string(prop(PropSceneObjectCont::orphans).name) + DEPRECATION_NO_REPLACE},
        {"scene", prop(PropSceneObjectCont::DEPRECATED_selectionHandles),   std::string(prop(PropSceneObjectCont::selection).name) + DEPRECATION_NO_REPLACE},
        {"scene", prop(PropDynCont::DEPRECATED_dynamicsEnabled),            prop(PropDynCont::dynamicsEnabled).name},
        {"scene", prop(PropDynCont::DEPRECATED_showContactPoints),          prop(PropDynCont::showContactPoints).name},
        {"scene", prop(PropDynCont::DEPRECATED_dynamicsEngine),             prop(PropDynCont::dynamicsEngine).name},
        {"scene", prop(PropDynCont::DEPRECATED_dynamicsStepSize),           prop(PropDynCont::dynamicsStepSize).name},
        {"scene", prop(PropDynCont::DEPRECATED_gravity),                    prop(PropDynCont::gravity).name},
        {"scene", prop(PropDynCont::DEPRECATED_engineProperties),           prop(PropDynCont::engineProperties).name},
        {"scene", prop(PropDynCont::DEPRECATED_bulletSolver),               prop(PropDynCont::bulletSolver).name},
        {"scene", prop(PropDynCont::DEPRECATED_bulletIterations),           prop(PropDynCont::bulletIterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_bulletComputeInertias),      prop(PropDynCont::bulletComputeInertias).name},
        {"scene", prop(PropDynCont::DEPRECATED_bulletInternalScalingFull),  prop(PropDynCont::bulletInternalScalingFull).name},
        {"scene", prop(PropDynCont::DEPRECATED_bulletInternalScalingScaling), prop(PropDynCont::bulletInternalScalingScaling).name},
        {"scene", prop(PropDynCont::DEPRECATED_bulletCollMarginScaling),    prop(PropDynCont::bulletCollMarginScaling).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeQuickStepEnabled),        prop(PropDynCont::odeQuickStepEnabled).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeQuickStepIterations),     prop(PropDynCont::odeQuickStepIterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeComputeInertias),         prop(PropDynCont::odeComputeInertias).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeInternalScalingFull),     prop(PropDynCont::odeInternalScalingFull).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeInternalScalingScaling),  prop(PropDynCont::odeInternalScalingScaling).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeGlobalErp),               prop(PropDynCont::odeGlobalErp).name},
        {"scene", prop(PropDynCont::DEPRECATED_odeGlobalCfm),               prop(PropDynCont::odeGlobalCfm).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexComputeInertias),      prop(PropDynCont::vortexComputeInertias).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexContactTolerance),     prop(PropDynCont::vortexContactTolerance).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexAutoSleep),            prop(PropDynCont::vortexAutoSleep).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexMultithreading),       prop(PropDynCont::vortexMultithreading).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexConstraintsLinearCompliance),  prop(PropDynCont::vortexConstraintsLinearCompliance).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexConstraintsLinearDamping),     prop(PropDynCont::vortexConstraintsLinearDamping).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexConstraintsLinearKineticLoss), prop(PropDynCont::vortexConstraintsLinearKineticLoss).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexConstraintsAngularCompliance), prop(PropDynCont::vortexConstraintsAngularCompliance).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexConstraintsAngularDamping),    prop(PropDynCont::vortexConstraintsAngularDamping).name},
        {"scene", prop(PropDynCont::DEPRECATED_vortexConstraintsAngularKineticLoss), prop(PropDynCont::vortexConstraintsAngularKineticLoss).name},
        {"scene", prop(PropDynCont::DEPRECATED_newtonIterations),           prop(PropDynCont::newtonIterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_newtonComputeInertias),      prop(PropDynCont::newtonComputeInertias).name},
        {"scene", prop(PropDynCont::DEPRECATED_newtonMultithreading),       prop(PropDynCont::newtonMultithreading).name},
        {"scene", prop(PropDynCont::DEPRECATED_newtonExactSolver),          prop(PropDynCont::newtonExactSolver).name},
        {"scene", prop(PropDynCont::DEPRECATED_newtonHighJointAccuracy),    prop(PropDynCont::newtonHighJointAccuracy).name},
        {"scene", prop(PropDynCont::DEPRECATED_newtonContactMergeTolerance), prop(PropDynCont::newtonContactMergeTolerance).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoIntegrator),           prop(PropDynCont::mujocoIntegrator).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoSolver),               prop(PropDynCont::mujocoSolver).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoIterations),           prop(PropDynCont::mujocoIterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoRebuildTrigger),       prop(PropDynCont::mujocoRebuildTrigger).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoComputeInertias),      prop(PropDynCont::mujocoComputeInertias).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoMbMemory),             prop(PropDynCont::mujocoMbMemory).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoNjMax),                prop(PropDynCont::mujocoNjMax).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoNconMax),              prop(PropDynCont::mujocoNconMax).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoNstack),               prop(PropDynCont::mujocoNstack).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoCone),                 prop(PropDynCont::mujocoCone).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoKinematicBodiesOverrideFlags), prop(PropDynCont::mujocoKinematicBodiesOverrideFlags).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoKinematicBodiesMass),  prop(PropDynCont::mujocoKinematicBodiesMass).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoKinematicBodiesInertia), prop(PropDynCont::mujocoKinematicBodiesInertia).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoBoundMass),            prop(PropDynCont::mujocoBoundMass).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoBoundInertia),         prop(PropDynCont::mujocoBoundInertia).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoBalanceInertias),      prop(PropDynCont::mujocoBalanceInertias).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoMultithreaded),        prop(PropDynCont::mujocoMultithreaded).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoMulticcd),             prop(PropDynCont::mujocoMulticcd).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoContactParamsOverride), prop(PropDynCont::mujocoContactParamsOverride).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoContactParamsMargin),  prop(PropDynCont::mujocoContactParamsMargin).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoContactParamsSolref),  prop(PropDynCont::mujocoContactParamsSolref).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoContactParamsSolimp),  prop(PropDynCont::mujocoContactParamsSolimp).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoImpRatio),             prop(PropDynCont::mujocoImpRatio).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoWind),                 prop(PropDynCont::mujocoWind).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoDensity),              prop(PropDynCont::mujocoDensity).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoViscosity),            prop(PropDynCont::mujocoViscosity).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoJacobian),             prop(PropDynCont::mujocoJacobian).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoTolerance),            prop(PropDynCont::mujocoTolerance).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoLs_iterations),        prop(PropDynCont::mujocoLs_iterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoLs_tolerance),         prop(PropDynCont::mujocoLs_tolerance).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoNoslip_iterations),    prop(PropDynCont::mujocoNoslip_iterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoNoslip_tolerance),     prop(PropDynCont::mujocoNoslip_tolerance).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoCcd_iterations),       prop(PropDynCont::mujocoCcd_iterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoCcd_tolerance),        prop(PropDynCont::mujocoCcd_tolerance).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoSdf_iterations),       prop(PropDynCont::mujocoSdf_iterations).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoSdf_initpoints),       prop(PropDynCont::mujocoSdf_initpoints).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoEqualityEnable),       prop(PropDynCont::mujocoEqualityEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoFrictionlossEnable),   prop(PropDynCont::mujocoFrictionlossEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoLimitEnable),          prop(PropDynCont::mujocoLimitEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoContactEnable),        prop(PropDynCont::mujocoContactEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoPassiveEnable),        prop(PropDynCont::mujocoPassiveEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoGravityEnable),        prop(PropDynCont::mujocoGravityEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoWarmstartEnable),      prop(PropDynCont::mujocoWarmstartEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoActuationEnable),      prop(PropDynCont::mujocoActuationEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoRefsafeEnable),        prop(PropDynCont::mujocoRefsafeEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoSensorEnable),         prop(PropDynCont::mujocoSensorEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoMidphaseEnable),       prop(PropDynCont::mujocoMidphaseEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoEulerdampEnable),      prop(PropDynCont::mujocoEulerdampEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoAutoresetEnable),      prop(PropDynCont::mujocoAutoresetEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoEnergyEnable),         prop(PropDynCont::mujocoEnergyEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoInvdiscreteEnable),    prop(PropDynCont::mujocoInvdiscreteEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoNativeccdEnable),      prop(PropDynCont::mujocoNativeccdEnable).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoAlignfree),            prop(PropDynCont::mujocoAlignfree).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoKinematicWeldSolref),  prop(PropDynCont::mujocoKinematicWeldSolref).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoKinematicWeldSolimp),  prop(PropDynCont::mujocoKinematicWeldSolimp).name},
        {"scene", prop(PropDynCont::DEPRECATED_mujocoKinematicWeldTorqueScale), prop(PropDynCont::mujocoKinematicWeldTorqueScale).name},
        {"scene", prop(PropSimulation::DEPRECATED_removeNewObjectsAtEnd),   prop(PropSimulation::removeNewObjectsAtEnd).name},
        {"scene", prop(PropSimulation::DEPRECATED_realtimeSimulation),      prop(PropSimulation::realtimeSimulation).name},
        {"scene", prop(PropSimulation::DEPRECATED_pauseSimulationAtTime),   prop(PropSimulation::pauseSimulationAtTime).name},
        {"scene", prop(PropSimulation::DEPRECATED_pauseSimulationAtError),  prop(PropSimulation::pauseSimulationAtError).name},
        {"scene", prop(PropSimulation::DEPRECATED_simulationTime),          prop(PropSimulation::simulationTime).name},
        {"scene", prop(PropSimulation::DEPRECATED_timeStep),                prop(PropSimulation::timeStep).name},
        {"scene", prop(PropSimulation::DEPRECATED_timeToPause),             prop(PropSimulation::timeToPause).name},
        {"scene", prop(PropSimulation::DEPRECATED_realtimeModifier),        prop(PropSimulation::realtimeModifier).name},
        {"scene", prop(PropSimulation::DEPRECATED_stepCount),               prop(PropSimulation::stepCount).name},
        {"scene", prop(PropSimulation::DEPRECATED_simulationState),         prop(PropSimulation::simulationState).name},
        {"scene", prop(PropSimulation::DEPRECATED_stepsPerRendering),       prop(PropSimulation::stepsPerRendering).name},
        {"scene", prop(PropSimulation::DEPRECATED_speedModifier),           prop(PropSimulation::speedModifier).name},

        {"detachedScript", prop(PropDetachedScript::DEPRECATED_scriptType),         prop(PropDetachedScript::scriptType).name},
        {"detachedScript", prop(PropDetachedScript::DEPRECATED_scriptDisabled),     prop(PropDetachedScript::scriptDisabled).name},
        {"detachedScript", prop(PropDetachedScript::DEPRECATED_scriptState),        prop(PropDetachedScript::scriptState).name},

        {"mesh", prop(PropMesh::DEPRECATED_textureResolution),          prop(PropMesh::textureResolution).name},
        {"mesh", prop(PropMesh::DEPRECATED_textureCoordinates),         prop(PropMesh::textureCoordinates).name},
        {"mesh", prop(PropMesh::DEPRECATED_textureApplyMode),           prop(PropMesh::textureApplyMode).name},
        {"mesh", prop(PropMesh::DEPRECATED_textureRepeatU),             prop(PropMesh::textureRepeatU).name},
        {"mesh", prop(PropMesh::DEPRECATED_textureRepeatV),             prop(PropMesh::textureRepeatV).name},
        {"mesh", prop(PropMesh::DEPRECATED_textureInterpolate),         prop(PropMesh::textureInterpolate).name},
        {"mesh", prop(PropMesh::DEPRECATED_texture),                    prop(PropMesh::texture).name},
        {"mesh", prop(PropMesh::DEPRECATED_textureID),                  prop(PropMesh::textureID).name},

        {"sceneObject", prop(PropSceneObject::DEPRECATED_pose),                 prop(PropSceneObject::pose).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_eulerAngles),          prop(PropSceneObject::eulerAngles).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_parentHandle),         std::string(prop(PropSceneObject::parent).name) + DEPRECATION_NO_REPLACE},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_objectUid),            prop(PropSceneObject::uid).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelProperty),        prop(PropSceneObject::modelProperty).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotCollidable),   prop(PropSceneObject::modelNotCollidable).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotMeasurable),   prop(PropSceneObject::modelNotMeasurable).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotDetectable),   prop(PropSceneObject::modelNotDetectable).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotDynamic),      prop(PropSceneObject::modelNotDynamic).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotRespondable),  prop(PropSceneObject::modelNotRespondable).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotVisible),      prop(PropSceneObject::modelNotVisible).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelScriptsNotActive), prop(PropSceneObject::modelScriptsNotActive).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_modelNotInParentBB),   prop(PropSceneObject::modelNotInParentBB).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_bbHsize),              prop(PropSceneObject::modelBBSize).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_alias),                prop(PropSceneObject::name).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movementOptions),      prop(PropSceneObject::movementOptions).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movementStepSize),     prop(PropSceneObject::movementStepSize).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movementRelativity),   prop(PropSceneObject::movementRelativity).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movementPreferredAxes), prop(PropSceneObject::movementPreferredAxes).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movTranslNoSim),       prop(PropSceneObject::movTranslNoSim).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movTranslInSim),       prop(PropSceneObject::movTranslInSim).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movRotNoSim),          prop(PropSceneObject::movRotNoSim).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movRotInSim),          prop(PropSceneObject::movRotInSim).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movAltTransl),         prop(PropSceneObject::movAltTransl).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movAltRot),            prop(PropSceneObject::movAltRot).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movPrefTranslX),       prop(PropSceneObject::movPrefTranslX).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movPrefTranslY),       prop(PropSceneObject::movPrefTranslY).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movPrefTranslZ),       prop(PropSceneObject::movPrefTranslZ).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movPrefRotX),          prop(PropSceneObject::movPrefRotX).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movPrefRotY),          prop(PropSceneObject::movPrefRotY).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_movPrefRotZ),          prop(PropSceneObject::movPrefRotZ).name},
        {"sceneObject", prop(PropSceneObject::DEPRECATED_deprecatedName),       DEPRECATION_NO_REPLACE},

        {"forceSensor", prop(PropForceSensor::DEPRECATED_size),                     prop(PropForceSensor::size).name},
        {"forceSensor", prop(PropForceSensor::DEPRECATED_sensorAverageForce),       prop(PropForceSensor::filteredSensorForce).name},
        {"forceSensor", prop(PropForceSensor::DEPRECATED_sensorAverageTorque),      prop(PropForceSensor::filteredSensorTorque).name},
        {"forceSensor", prop(PropForceSensor::DEPRECATED_sensorForce),              prop(PropForceSensor::sensorForce).name},
        {"forceSensor", prop(PropForceSensor::DEPRECATED_sensorTorque),             prop(PropForceSensor::sensorTorque).name},

        {"script", prop(PropScript::DEPRECATED_size),                       prop(PropScript::size).name},
        {"script", prop(PropScript::DEPRECATED_scriptDisabled),             DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_restartOnError),             DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_execPriority),               DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_scriptType),                 DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_executionDepth),             DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_scriptState),                DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_language),                   DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_code),                       DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_scriptName),                 DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_addOnPath),                  DEPRECATION_NO_REPLACE},
        {"script", prop(PropScript::DEPRECATED_addOnMenuPath),              DEPRECATION_NO_REPLACE},

        {"ocTree", prop(PropOctree::DEPRECATED_voxels),                     prop(PropOctree::points).name},

        {"camera", prop(PropCamera::DEPRECATED_size),                       prop(PropCamera::size).name},
        {"camera", prop(PropCamera::DEPRECATED_trackedObject),              std::string(prop(PropCamera::trackedObject).name) + DEPRECATION_NO_REPLACE},

        {"light", prop(PropLight::DEPRECATED_size),                         prop(PropLight::size).name},
        {"light", prop(PropLight::DEPRECATED_lightType),                    prop(PropLight::lightType).name},

        {"joint", prop(PropJoint::DEPRECATED_position),                     prop(PropJoint::position).name},
        {"joint", prop(PropJoint::DEPRECATED_quaternion),                   prop(PropJoint::quaternion).name},
        {"joint", prop(PropJoint::DEPRECATED_dependencyMaster),             std::string(prop(PropJoint::dependencyMaster).name) + DEPRECATION_NO_REPLACE},
        {"joint", prop(PropJoint::DEPRECATED_targetPos),                    prop(PropJoint::targetPos).name},
        {"joint", prop(PropJoint::DEPRECATED_targetVel),                    prop(PropJoint::targetPos).name},
        {"joint", prop(PropJoint::DEPRECATED_length),                       prop(PropJoint::length).name},
        {"joint", prop(PropJoint::DEPRECATED_diameter),                     prop(PropJoint::diameter).name},
        {"joint", prop(PropJoint::DEPRECATED_jointType),                    prop(PropJoint::jointType).name},
        {"joint", prop(PropJoint::DEPRECATED_jointForce),                   prop(PropJoint::jointForce).name},
        {"joint", prop(PropJoint::DEPRECATED_averageJointForce),            prop(PropJoint::averageJointForce).name},
        {"joint", prop(PropJoint::DEPRECATED_jointMode),                    prop(PropJoint::jointMode).name},
        {"joint", prop(PropJoint::DEPRECATED_dynCtrlMode),                  prop(PropJoint::dynCtrlMode).name},
        {"joint", prop(PropJoint::DEPRECATED_springDamperParams),           prop(PropJoint::springDamperParams).name},
        {"joint", prop(PropJoint::DEPRECATED_dynVelMode),                   prop(PropJoint::dynVelMode).name},
        {"joint", prop(PropJoint::DEPRECATED_dynPosMode),                   prop(PropJoint::dynPosMode).name},
        {"joint", prop(PropJoint::DEPRECATED_engineProperties),             prop(PropJoint::engineProperties).name},
        {"joint", prop(PropJoint::DEPRECATED_bulletStopErp),                prop(PropJoint::bulletStopErp).name},
        {"joint", prop(PropJoint::DEPRECATED_bulletStopCfm),                prop(PropJoint::bulletStopCfm).name},
        {"joint", prop(PropJoint::DEPRECATED_bulletNormalCfm),              prop(PropJoint::bulletNormalCfm).name},
        {"joint", prop(PropJoint::DEPRECATED_bulletPosPid),                 prop(PropJoint::bulletPosPid).name},
        {"joint", prop(PropJoint::DEPRECATED_odeStopErp),                   prop(PropJoint::odeStopErp).name},
        {"joint", prop(PropJoint::DEPRECATED_odeStopCfm),                   prop(PropJoint::odeStopCfm).name},
        {"joint", prop(PropJoint::DEPRECATED_odeNormalCfm),                 prop(PropJoint::odeNormalCfm).name},
        {"joint", prop(PropJoint::DEPRECATED_odeBounce),                    prop(PropJoint::odeBounce).name},
        {"joint", prop(PropJoint::DEPRECATED_odeFudgeFactor),               prop(PropJoint::odeFudgeFactor).name},
        {"joint", prop(PropJoint::DEPRECATED_odePosPid),                    prop(PropJoint::odePosPid).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexLowerLimitDamping),      prop(PropJoint::vortexLowerLimitDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexUpperLimitDamping),      prop(PropJoint::vortexUpperLimitDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexLowerLimitStiffness),    prop(PropJoint::vortexLowerLimitStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexUpperLimitStiffness),    prop(PropJoint::vortexUpperLimitStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexLowerLimitRestitution),  prop(PropJoint::vortexLowerLimitRestitution).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexUpperLimitRestitution),  prop(PropJoint::vortexUpperLimitRestitution).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexLowerLimitMaxForce),     prop(PropJoint::vortexLowerLimitMaxForce).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexUpperLimitMaxForce),     prop(PropJoint::vortexUpperLimitMaxForce).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexAxisFrictionEnabled),    prop(PropJoint::vortexAxisFrictionEnabled).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexAxisFrictionProportional),       prop(PropJoint::vortexAxisFrictionProportional).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexAxisFrictionCoeff),              prop(PropJoint::vortexAxisFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexAxisFrictionMaxForce),           prop(PropJoint::vortexAxisFrictionMaxForce).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexAxisFrictionLoss),               prop(PropJoint::vortexAxisFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexRelaxationEnabledBits),          prop(PropJoint::vortexRelaxationEnabledBits).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexFrictionEnabledBits),            prop(PropJoint::vortexFrictionEnabledBits).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexFrictionProportionalBits),       prop(PropJoint::vortexFrictionProportionalBits).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisPosRelaxationStiffness),    prop(PropJoint::vortexXAxisPosRelaxationStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisPosRelaxationDamping),      prop(PropJoint::vortexXAxisPosRelaxationDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisPosRelaxationLoss),         prop(PropJoint::vortexXAxisPosRelaxationLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisPosFrictionCoeff),          prop(PropJoint::vortexXAxisPosFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisPosFrictionMaxForce),       prop(PropJoint::vortexXAxisPosFrictionMaxForce).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisPosFrictionLoss),           prop(PropJoint::vortexXAxisPosFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisPosRelaxationStiffness),    prop(PropJoint::vortexYAxisPosRelaxationStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisPosRelaxationDamping),      prop(PropJoint::vortexYAxisPosRelaxationDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisPosRelaxationLoss),         prop(PropJoint::vortexYAxisPosRelaxationLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisPosFrictionCoeff),          prop(PropJoint::vortexYAxisPosFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisPosFrictionMaxForce),       prop(PropJoint::vortexYAxisPosFrictionMaxForce).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisPosFrictionLoss),           prop(PropJoint::vortexYAxisPosFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisPosRelaxationStiffness),    prop(PropJoint::vortexZAxisPosRelaxationStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisPosRelaxationDamping),      prop(PropJoint::vortexZAxisPosRelaxationDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisPosRelaxationLoss),         prop(PropJoint::vortexZAxisPosRelaxationLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisPosFrictionCoeff),          prop(PropJoint::vortexZAxisPosFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisPosFrictionMaxForce),       prop(PropJoint::vortexZAxisPosFrictionMaxForce).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisPosFrictionLoss),           prop(PropJoint::vortexZAxisPosFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisOrientRelaxStiffness),      prop(PropJoint::vortexXAxisOrientRelaxStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisOrientRelaxDamping),        prop(PropJoint::vortexXAxisOrientRelaxDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisOrientRelaxLoss),           prop(PropJoint::vortexXAxisOrientRelaxLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisOrientFrictionCoeff),       prop(PropJoint::vortexXAxisOrientFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisOrientFrictionMaxTorque),   prop(PropJoint::vortexXAxisOrientFrictionMaxTorque).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexXAxisOrientFrictionLoss),        prop(PropJoint::vortexXAxisOrientFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisOrientRelaxStiffness),      prop(PropJoint::vortexYAxisOrientRelaxStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisOrientRelaxDamping),        prop(PropJoint::vortexYAxisOrientRelaxDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisOrientRelaxLoss),           prop(PropJoint::vortexYAxisOrientRelaxLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisOrientFrictionCoeff),       prop(PropJoint::vortexYAxisOrientFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisOrientFrictionMaxTorque),   prop(PropJoint::vortexYAxisOrientFrictionMaxTorque).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexYAxisOrientFrictionLoss),        prop(PropJoint::vortexYAxisOrientFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisOrientRelaxStiffness),      prop(PropJoint::vortexZAxisOrientRelaxStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisOrientRelaxDamping),        prop(PropJoint::vortexZAxisOrientRelaxDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisOrientRelaxLoss),           prop(PropJoint::vortexZAxisOrientRelaxLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisOrientFrictionCoeff),       prop(PropJoint::vortexZAxisOrientFrictionCoeff).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisOrientFrictionMaxTorque),   prop(PropJoint::vortexZAxisOrientFrictionMaxTorque).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexZAxisOrientFrictionLoss),        prop(PropJoint::vortexZAxisOrientFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_vortexPosPid),                 prop(PropJoint::vortexPosPid).name},
        {"joint", prop(PropJoint::DEPRECATED_newtonPosPid),                 prop(PropJoint::newtonPosPid).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoArmature),               prop(PropJoint::mujocoArmature).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoMargin),                 prop(PropJoint::mujocoMargin).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoFrictionLoss),           prop(PropJoint::mujocoFrictionLoss).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoSpringStiffness),        prop(PropJoint::mujocoSpringStiffness).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoSpringDamping),          prop(PropJoint::mujocoSpringDamping).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoSpringRef),              prop(PropJoint::mujocoSpringRef).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoPosPid),                 prop(PropJoint::mujocoPosPid).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoLimitsSolRef),           prop(PropJoint::mujocoLimitsSolRef).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoLimitsSolImp),           prop(PropJoint::mujocoLimitsSolImp).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoFrictionSolRef),         prop(PropJoint::mujocoFrictionSolRef).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoFrictionSolImp),         prop(PropJoint::mujocoFrictionSolImp).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoSpringDamper),           prop(PropJoint::mujocoSpringDamper).name},
        {"joint", prop(PropJoint::DEPRECATED_mujocoDependencyPolyCoef),     prop(PropJoint::mujocoDependencyPolyCoef).name},

        {"dummy", prop(PropDummy::DEPRECATED_size),                         prop(PropDummy::size).name},
        {"dummy", prop(PropDummy::DEPRECATED_linkedDummyHandle),            std::string(prop(PropDummy::linkedDummy).name) + DEPRECATION_NO_REPLACE},
        {"dummy", prop(PropDummy::DEPRECATED_dummyType),                    prop(PropDummy::dummyType).name},
        {"dummy", prop(PropDummy::DEPRECATED_engineProperties),             prop(PropDummy::engineProperties).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoLimitsEnabled),          prop(PropDummy::mujocoLimitsEnabled).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoLimitsRange),            prop(PropDummy::mujocoLimitsRange).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoLimitsSolref),           prop(PropDummy::mujocoLimitsSolref).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoLimitsSolimp),           prop(PropDummy::mujocoLimitsSolimp).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoMargin),                 prop(PropDummy::mujocoMargin).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoSpringStiffness),        prop(PropDummy::mujocoSpringStiffness).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoSpringDamping),          prop(PropDummy::mujocoSpringDamping).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoSpringLength),           prop(PropDummy::mujocoSpringLength).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoJointProxyHandle),       prop(PropDummy::mujocoJointProxyHandle).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoOverlapConstrSolref),    prop(PropDummy::mujocoOverlapConstrSolref).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoOverlapConstrSolimp),    prop(PropDummy::mujocoOverlapConstrSolimp).name},
        {"dummy", prop(PropDummy::DEPRECATED_mujocoOverlapConstrTorqueScale),   prop(PropDummy::mujocoOverlapConstrTorqueScale).name},

        {"shape", prop(PropMeshWrapper::DEPRECATED_pmi),                        std::string(prop(PropMeshWrapper::pmi).name) + DEPRECATION_NO_REPLACE},
        {"shape", prop(PropMeshWrapper::DEPRECATED_inertia),                    std::string(prop(PropMeshWrapper::inertiaMatrix).name) + DEPRECATION_NO_REPLACE},
        {"shape", prop(PropMaterial::DEPRECATED_engineProperties),              prop(PropMaterial::engineProperties).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletRestitution),             prop(PropMaterial::bulletRestitution).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletFriction0),               prop(PropMaterial::bulletFriction0).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletFriction),                prop(PropMaterial::bulletFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletLinearDamping),           prop(PropMaterial::bulletLinearDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletAngularDamping),          prop(PropMaterial::bulletAngularDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMarginFactor),         prop(PropMaterial::bulletNonDefaultCollisionMarginFactor).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMarginFactorConvex),   prop(PropMaterial::bulletNonDefaultCollisionMarginFactorConvex).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletSticky),                                  prop(PropMaterial::bulletSticky).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMargin),               prop(PropMaterial::bulletNonDefaultCollisionMargin).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMarginConvex),         prop(PropMaterial::bulletNonDefaultCollisionMarginConvex).name},
        {"shape", prop(PropMaterial::DEPRECATED_bulletAutoShrinkConvex),        prop(PropMaterial::bulletAutoShrinkConvex).name},
        {"shape", prop(PropMaterial::DEPRECATED_odeFriction),                   prop(PropMaterial::odeFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_odeSoftErp),                    prop(PropMaterial::odeSoftErp).name},
        {"shape", prop(PropMaterial::DEPRECATED_odeSoftCfm),                    prop(PropMaterial::odeSoftCfm).name},
        {"shape", prop(PropMaterial::DEPRECATED_odeLinearDamping),              prop(PropMaterial::odeLinearDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_odeAngularDamping),             prop(PropMaterial::odeAngularDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_odeMaxContacts),                prop(PropMaterial::odeMaxContacts).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisFriction),               prop(PropMaterial::vortexPrimaryLinearAxisFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisFriction),             prop(PropMaterial::vortexSecondaryLinearAxisFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisFriction),              prop(PropMaterial::vortexPrimaryAngularAxisFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisFriction),            prop(PropMaterial::vortexSecondaryAngularAxisFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisFriction),               prop(PropMaterial::vortexNormalAngularAxisFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisStaticFrictionScale),    prop(PropMaterial::vortexPrimaryLinearAxisStaticFrictionScale).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisStaticFrictionScale),  prop(PropMaterial::vortexSecondaryLinearAxisStaticFrictionScale).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisStaticFrictionScale),   prop(PropMaterial::vortexPrimaryAngularAxisStaticFrictionScale).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisStaticFrictionScale), prop(PropMaterial::vortexSecondaryAngularAxisStaticFrictionScale).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisStaticFrictionScale),    prop(PropMaterial::vortexNormalAngularAxisStaticFrictionScale).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexCompliance),              prop(PropMaterial::vortexCompliance).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexDamping),                 prop(PropMaterial::vortexDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexRestitution),             prop(PropMaterial::vortexRestitution).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexRestitutionThreshold),    prop(PropMaterial::vortexRestitutionThreshold).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAdhesiveForce),           prop(PropMaterial::vortexAdhesiveForce).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexLinearVelocityDamping),   prop(PropMaterial::vortexLinearVelocityDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAngularVelocityDamping),  prop(PropMaterial::vortexAngularVelocityDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisSlide),  prop(PropMaterial::vortexPrimaryLinearAxisSlide).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisSlide),    prop(PropMaterial::vortexSecondaryLinearAxisSlide).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisSlide),     prop(PropMaterial::vortexPrimaryAngularAxisSlide).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisSlide),   prop(PropMaterial::vortexSecondaryAngularAxisSlide).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisSlide),  prop(PropMaterial::vortexNormalAngularAxisSlide).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisSlip),   prop(PropMaterial::vortexPrimaryLinearAxisSlip).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisSlip), prop(PropMaterial::vortexSecondaryLinearAxisSlip).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisSlip),  prop(PropMaterial::vortexPrimaryAngularAxisSlip).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisSlip),                prop(PropMaterial::vortexSecondaryAngularAxisSlip).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisSlip),                   prop(PropMaterial::vortexNormalAngularAxisSlip).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoSleepLinearSpeedThreshold),           prop(PropMaterial::vortexAutoSleepLinearSpeedThreshold).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoSleepLinearAccelerationThreshold),    prop(PropMaterial::vortexAutoSleepLinearAccelerationThreshold).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoSleepAngularSpeedThreshold),          prop(PropMaterial::vortexAutoSleepAngularSpeedThreshold).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoSleepAngularAccelerationThreshold),   prop(PropMaterial::vortexAutoSleepAngularAccelerationThreshold).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSkinThickness),                           prop(PropMaterial::vortexSkinThickness).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoAngularDampingTensionRatio),          prop(PropMaterial::vortexAutoAngularDampingTensionRatio).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryAxisVector),                       prop(PropMaterial::vortexPrimaryAxisVector).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisFrictionModel),          prop(PropMaterial::vortexPrimaryLinearAxisFrictionModel).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisFrictionModel),        prop(PropMaterial::vortexSecondaryLinearAxisFrictionModel).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisFrictionModel),         prop(PropMaterial::vortexPrimaryAngularAxisFrictionModel).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisFrictionModel),       prop(PropMaterial::vortexSecondaryAngularAxisFrictionModel).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisFrictionModel),          prop(PropMaterial::vortexNormalAngularAxisFrictionModel).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoSleepStepLiveThreshold),              prop(PropMaterial::vortexAutoSleepStepLiveThreshold).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexMaterialUniqueId),        prop(PropMaterial::vortexMaterialUniqueId).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexPrimitiveShapesAsConvex), prop(PropMaterial::vortexPrimitiveShapesAsConvex).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexConvexShapesAsRandom),    prop(PropMaterial::vortexConvexShapesAsRandom).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexRandomShapesAsTerrain),   prop(PropMaterial::vortexRandomShapesAsTerrain).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexFastMoving),              prop(PropMaterial::vortexFastMoving).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoSlip),                prop(PropMaterial::vortexAutoSlip).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis),      prop(PropMaterial::vortexSecondaryLinearAxisSameAsPrimaryLinearAxis).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis),    prop(PropMaterial::vortexSecondaryAngularAxisSameAsPrimaryAngularAxis).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisSameAsPrimaryAngularAxis),       prop(PropMaterial::vortexNormalAngularAxisSameAsPrimaryAngularAxis).name},
        {"shape", prop(PropMaterial::DEPRECATED_vortexAutoAngularDamping),      prop(PropMaterial::vortexAutoAngularDamping).name},
        {"shape", prop(PropMaterial::DEPRECATED_newtonStaticFriction),          prop(PropMaterial::newtonStaticFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_newtonKineticFriction),         prop(PropMaterial::newtonKineticFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_newtonRestitution),             prop(PropMaterial::newtonRestitution).name},
        {"shape", prop(PropMaterial::DEPRECATED_newtonLinearDrag),              prop(PropMaterial::newtonLinearDrag).name},
        {"shape", prop(PropMaterial::DEPRECATED_newtonAngularDrag),             prop(PropMaterial::newtonAngularDrag).name},
        {"shape", prop(PropMaterial::DEPRECATED_newtonFastMoving),              prop(PropMaterial::newtonFastMoving).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoFriction),                prop(PropMaterial::mujocoFriction).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoSolref),                  prop(PropMaterial::mujocoSolref).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoSolimp),                  prop(PropMaterial::mujocoSolimp).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoSolmix),                  prop(PropMaterial::mujocoSolmix).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoMargin),                  prop(PropMaterial::mujocoMargin).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoGap),                     prop(PropMaterial::mujocoGap).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoCondim),                  prop(PropMaterial::mujocoCondim).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoPriority),                prop(PropMaterial::mujocoPriority).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoAdhesion),                prop(PropMaterial::mujocoAdhesion).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoAdhesionGain),            prop(PropMaterial::mujocoAdhesionGain).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoAdhesionForcelimited),    prop(PropMaterial::mujocoAdhesionForcelimited).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoAdhesionCtrlrange),       prop(PropMaterial::mujocoAdhesionCtrlrange).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoAdhesionForcerange),      prop(PropMaterial::mujocoAdhesionForcerange).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoAdhesionCtrl),            prop(PropMaterial::mujocoAdhesionCtrl).name},
        {"shape", prop(PropMaterial::DEPRECATED_mujocoGravcomp),                prop(PropMaterial::mujocoGravcomp).name},

        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_offset),          prop(PropConvexVolume::offset).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_range),           prop(PropConvexVolume::range).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_xSize),           prop(PropConvexVolume::xSize).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_ySize),           prop(PropConvexVolume::ySize).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_radius),          prop(PropConvexVolume::radius).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_angle),           prop(PropConvexVolume::angle).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_faces),           prop(PropConvexVolume::faces).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_subdivisions),    prop(PropConvexVolume::subdivisions).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_edges),           prop(PropConvexVolume::edges).name},
        {"proximitySensor", prop(PropConvexVolume::DEPRECATED_closeEdges),      prop(PropConvexVolume::closeEdges).name},
        {"proximitySensor", prop(PropProximitySensor::DEPRECATED_size),         prop(PropProximitySensor::size).name},
        {"proximitySensor", prop(PropProximitySensor::DEPRECATED_sensorType),   prop(PropProximitySensor::sensorType).name},
        {"proximitySensor", prop(PropProximitySensor::DEPRECATED_detectedObjectHandle),     std::string(prop(PropProximitySensor::detectedObject).name) + DEPRECATION_NO_REPLACE},

        {"visionSensor", prop(PropVisionSensor::DEPRECATED_size),               prop(PropVisionSensor::size).name},
        {"visionSensor", prop(PropVisionSensor::DEPRECATED_depthBuffer),        std::string(prop(PropVisionSensor::depthBuffer).name) + DEPRECATION_NO_REPLACE},
    };
    return tmpDeprecated;
}

std::map<std::string, std::vector<SDeprecatedProp>> createDeprecationMapping(int callingApiVer)
{
    std::map<std::string, std::vector<SDeprecatedProp>> result;
    auto tmpDeprecated = getDeprecatedList();
    for (size_t i = 0; i < tmpDeprecated.size(); i++)
    {
        int startSupport = tmpDeprecated[i].oldProp.info.map["startSupport"].toInt();
        int startDeprecated = tmpDeprecated[i].oldProp.info.map["startDeprecated"].toInt();
        int endSupport = tmpDeprecated[i].oldProp.info.map["endSupport"].toInt();
        if ((startSupport <= callingApiVer) && ((endSupport == 0) || (endSupport >= callingApiVer)))
        {
            std::string key = tmpDeprecated[i].oldProp.name;
            SDeprecatedProp p;
            p.type = tmpDeprecated[i].type;
            p.replacement = tmpDeprecated[i].newPropName;
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

    if (callingApiVer == 1)
    { // very special here (can't list "position" and "quaternion" in sceneObject properties, as they would get confused with joint properties):
        SDeprecatedProp p;
        p.type = "sceneObject";
        std::string key = "position";
        p.replacement = std::string(prop(PropSceneObject::position).name);
        p.deprecated = false;
        auto it = result.find(key);
        if (it == result.end())
        {
            result[key] = {};
            it = result.find(key);
        }
        it->second.push_back(p);

        key = "quaternion";
        p.replacement = std::string(prop(PropSceneObject::quaternion).name);
        it = result.find(key);
        if (it == result.end())
        {
            result[key] = {};
            it = result.find(key);
        }
        it->second.push_back(p);
    }

    return result;
}

const std::map<std::string, std::vector<SDeprecatedProp>> propDeprecationMappings[2] = {createDeprecationMapping(1), createDeprecationMapping(2)};
