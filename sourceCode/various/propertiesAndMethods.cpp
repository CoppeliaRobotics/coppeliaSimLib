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

struct SDeprecatedPropertyItems {
    std::string repl;
    int type;
};

const std::vector<std::pair<std::string, SDeprecatedPropertyItems>> tmpPairs = {
    {prop(PropApp::DEPRECATED_appArg1).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg2).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg3).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg4).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg5).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg6).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg7).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg8).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appArg9).name,                                   {std::string(prop(PropApp::appArgs).name) + "__noReplace__", sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_appDir).name,                                    {prop(PropApp::appDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_tempDir).name,                                   {prop(PropApp::tempDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_sceneTempDir).name,                              {prop(PropApp::sceneTempDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_settingsDir).name,                               {prop(PropApp::settingsDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_luaDir).name,                                    {prop(PropApp::luaDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_pythonDir).name,                                 {prop(PropApp::pythonDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_mujocoDir).name,                                 {prop(PropApp::mujocoDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_systemDir).name,                                 {prop(PropApp::systemDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_resourceDir).name,                               {prop(PropApp::resourceDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_addOnDir).name,                                  {prop(PropApp::addOnDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_sceneDir).name,                                  {prop(PropApp::sceneDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_modelDir).name,                                  {prop(PropApp::modelDir).name, sim_objecttype_app}},
    {prop(PropApp::DEPRECATED_importExportDir).name,                           {prop(PropApp::importExportDir).name, sim_objecttype_app}},

    {prop(PropScene::DEPRECATED_sceneIsLocked).name,                           {prop(PropScene::sceneIsLocked).name, sim_objecttype_scene}},
    {prop(PropScene::DEPRECATED_sceneUid).name,                                {prop(PropScene::sceneUid).name, sim_objecttype_scene}},
    {prop(PropScene::DEPRECATED_sceneUidString).name,                          {prop(PropScene::sceneUidString).name, sim_objecttype_scene}},
    {prop(PropScene::DEPRECATED_scenePath).name,                               {prop(PropScene::scenePath).name, sim_objecttype_scene}},
    {prop(PropSceneObjectCont::DEPRECATED_objectHandles).name,                 {std::string(prop(PropSceneObjectCont::objects).name) + "__noReplace__", sim_objecttype_scene}},
    {prop(PropSceneObjectCont::DEPRECATED_orphanHandles).name,                 {std::string(prop(PropSceneObjectCont::orphans).name) + "__noReplace__", sim_objecttype_scene}},
    {prop(PropSceneObjectCont::DEPRECATED_selectionHandles).name,              {std::string(prop(PropSceneObjectCont::selection).name) + "__noReplace__", sim_objecttype_scene}},
    {prop(PropSceneObjectCont::DEPRECATED_objectHandles).name,                 {std::string(prop(PropSceneObjectCont::objects).name) + "__noReplace__", sim_objecttype_scene}},
    {prop(PropSceneObjectCont::DEPRECATED_orphanHandles).name,                 {std::string(prop(PropSceneObjectCont::orphans).name) + "__noReplace__", sim_objecttype_scene}},
    {prop(PropSceneObjectCont::DEPRECATED_selectionHandles).name,              {std::string(prop(PropSceneObjectCont::selection).name) + "__noReplace__", sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_dynamicsEnabled).name,                       {prop(PropDynCont::dynamicsEnabled).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_showContactPoints).name,                     {prop(PropDynCont::showContactPoints).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_dynamicsEngine).name,                        {prop(PropDynCont::dynamicsEngine).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_dynamicsStepSize).name,                      {prop(PropDynCont::dynamicsStepSize).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_gravity).name,                               {prop(PropDynCont::gravity).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_engineProperties).name,                      {prop(PropDynCont::engineProperties).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_bulletSolver).name,                          {prop(PropDynCont::bulletSolver).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_bulletIterations).name,                      {prop(PropDynCont::bulletIterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_bulletComputeInertias).name,                 {prop(PropDynCont::bulletComputeInertias).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_bulletInternalScalingFull).name,             {prop(PropDynCont::bulletInternalScalingFull).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_bulletInternalScalingScaling).name,          {prop(PropDynCont::bulletInternalScalingScaling).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_bulletCollMarginScaling).name,               {prop(PropDynCont::bulletCollMarginScaling).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeQuickStepEnabled).name,                   {prop(PropDynCont::odeQuickStepEnabled).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeQuickStepIterations).name,                {prop(PropDynCont::odeQuickStepIterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeComputeInertias).name,                    {prop(PropDynCont::odeComputeInertias).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeInternalScalingFull).name,                {prop(PropDynCont::odeInternalScalingFull).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeInternalScalingScaling).name,             {prop(PropDynCont::odeInternalScalingScaling).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeGlobalErp).name,                          {prop(PropDynCont::odeGlobalErp).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_odeGlobalCfm).name,                          {prop(PropDynCont::odeGlobalCfm).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexComputeInertias).name,                 {prop(PropDynCont::vortexComputeInertias).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexContactTolerance).name,                {prop(PropDynCont::vortexContactTolerance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexAutoSleep).name,                       {prop(PropDynCont::vortexAutoSleep).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexMultithreading).name,                  {prop(PropDynCont::vortexMultithreading).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexConstraintsLinearCompliance).name,     {prop(PropDynCont::vortexConstraintsLinearCompliance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexConstraintsLinearDamping).name,        {prop(PropDynCont::vortexConstraintsLinearDamping).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexConstraintsLinearKineticLoss).name,    {prop(PropDynCont::vortexConstraintsLinearKineticLoss).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexConstraintsAngularCompliance).name,    {prop(PropDynCont::vortexConstraintsAngularCompliance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexConstraintsAngularDamping).name,       {prop(PropDynCont::vortexConstraintsAngularDamping).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_vortexConstraintsAngularKineticLoss).name,   {prop(PropDynCont::vortexConstraintsAngularKineticLoss).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_newtonIterations).name,                      {prop(PropDynCont::newtonIterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_newtonComputeInertias).name,                 {prop(PropDynCont::newtonComputeInertias).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_newtonMultithreading).name,                  {prop(PropDynCont::newtonMultithreading).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_newtonExactSolver).name,                     {prop(PropDynCont::newtonExactSolver).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_newtonHighJointAccuracy).name,               {prop(PropDynCont::newtonHighJointAccuracy).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_newtonContactMergeTolerance).name,           {prop(PropDynCont::newtonContactMergeTolerance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoIntegrator).name,                      {prop(PropDynCont::mujocoIntegrator).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoSolver).name,                          {prop(PropDynCont::mujocoSolver).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoIterations).name,                      {prop(PropDynCont::mujocoIterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoRebuildTrigger).name,                  {prop(PropDynCont::mujocoRebuildTrigger).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoComputeInertias).name,                 {prop(PropDynCont::mujocoComputeInertias).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoMbMemory).name,                        {prop(PropDynCont::mujocoMbMemory).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoNjMax).name,                           {prop(PropDynCont::mujocoNjMax).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoNconMax).name,                         {prop(PropDynCont::mujocoNconMax).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoNstack).name,                          {prop(PropDynCont::mujocoNstack).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoCone).name,                            {prop(PropDynCont::mujocoCone).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoKinematicBodiesOverrideFlags).name,    {prop(PropDynCont::mujocoKinematicBodiesOverrideFlags).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoKinematicBodiesMass).name,             {prop(PropDynCont::mujocoKinematicBodiesMass).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoKinematicBodiesInertia).name,          {prop(PropDynCont::mujocoKinematicBodiesInertia).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoBoundMass).name,                       {prop(PropDynCont::mujocoBoundMass).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoBoundInertia).name,                    {prop(PropDynCont::mujocoBoundInertia).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoBalanceInertias).name,                 {prop(PropDynCont::mujocoBalanceInertias).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoMultithreaded).name,                   {prop(PropDynCont::mujocoMultithreaded).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoMulticcd).name,                        {prop(PropDynCont::mujocoMulticcd).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoContactParamsOverride).name,           {prop(PropDynCont::mujocoContactParamsOverride).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoContactParamsMargin).name,             {prop(PropDynCont::mujocoContactParamsMargin).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoContactParamsSolref).name,             {prop(PropDynCont::mujocoContactParamsSolref).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoContactParamsSolimp).name,             {prop(PropDynCont::mujocoContactParamsSolimp).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoImpRatio).name,                        {prop(PropDynCont::mujocoImpRatio).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoWind).name,                            {prop(PropDynCont::mujocoWind).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoDensity).name,                         {prop(PropDynCont::mujocoDensity).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoViscosity).name,                       {prop(PropDynCont::mujocoViscosity).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoJacobian).name,                        {prop(PropDynCont::mujocoJacobian).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoTolerance).name,                       {prop(PropDynCont::mujocoTolerance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoLs_iterations).name,                   {prop(PropDynCont::mujocoLs_iterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoLs_tolerance).name,                    {prop(PropDynCont::mujocoLs_tolerance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoNoslip_iterations).name,               {prop(PropDynCont::mujocoNoslip_iterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoNoslip_tolerance).name,                {prop(PropDynCont::mujocoNoslip_tolerance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoCcd_iterations).name,                  {prop(PropDynCont::mujocoCcd_iterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoCcd_tolerance).name,                   {prop(PropDynCont::mujocoCcd_tolerance).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoSdf_iterations).name,                  {prop(PropDynCont::mujocoSdf_iterations).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoSdf_initpoints).name,                  {prop(PropDynCont::mujocoSdf_initpoints).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoEqualityEnable).name,                  {prop(PropDynCont::mujocoEqualityEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoFrictionlossEnable).name,              {prop(PropDynCont::mujocoFrictionlossEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoLimitEnable).name,                     {prop(PropDynCont::mujocoLimitEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoContactEnable).name,                   {prop(PropDynCont::mujocoContactEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoPassiveEnable).name,                   {prop(PropDynCont::mujocoPassiveEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoGravityEnable).name,                   {prop(PropDynCont::mujocoGravityEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoWarmstartEnable).name,                 {prop(PropDynCont::mujocoWarmstartEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoActuationEnable).name,                 {prop(PropDynCont::mujocoActuationEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoRefsafeEnable).name,                   {prop(PropDynCont::mujocoRefsafeEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoSensorEnable).name,                    {prop(PropDynCont::mujocoSensorEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoMidphaseEnable).name,                  {prop(PropDynCont::mujocoMidphaseEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoEulerdampEnable).name,                 {prop(PropDynCont::mujocoEulerdampEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoAutoresetEnable).name,                 {prop(PropDynCont::mujocoAutoresetEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoEnergyEnable).name,                    {prop(PropDynCont::mujocoEnergyEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoInvdiscreteEnable).name,               {prop(PropDynCont::mujocoInvdiscreteEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoNativeccdEnable).name,                 {prop(PropDynCont::mujocoNativeccdEnable).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoAlignfree).name,                       {prop(PropDynCont::mujocoAlignfree).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoKinematicWeldSolref).name,             {prop(PropDynCont::mujocoKinematicWeldSolref).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoKinematicWeldSolimp).name,             {prop(PropDynCont::mujocoKinematicWeldSolimp).name, sim_objecttype_scene}},
    {prop(PropDynCont::DEPRECATED_mujocoKinematicWeldTorqueScale).name,        {prop(PropDynCont::mujocoKinematicWeldTorqueScale).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_removeNewObjectsAtEnd).name,              {prop(PropSimulation::removeNewObjectsAtEnd).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_realtimeSimulation).name,                 {prop(PropSimulation::realtimeSimulation).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_pauseSimulationAtTime).name,              {prop(PropSimulation::pauseSimulationAtTime).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_pauseSimulationAtError).name,             {prop(PropSimulation::pauseSimulationAtError).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_simulationTime).name,                     {prop(PropSimulation::simulationTime).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_timeStep).name,                           {prop(PropSimulation::timeStep).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_timeToPause).name,                        {prop(PropSimulation::timeToPause).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_realtimeModifier).name,                   {prop(PropSimulation::realtimeModifier).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_stepCount).name,                          {prop(PropSimulation::stepCount).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_simulationState).name,                    {prop(PropSimulation::simulationState).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_stepsPerRendering).name,                  {prop(PropSimulation::stepsPerRendering).name, sim_objecttype_scene}},
    {prop(PropSimulation::DEPRECATED_speedModifier).name,                      {prop(PropSimulation::speedModifier).name, sim_objecttype_scene}},

    {prop(PropDetachedScript::DEPRECATED_scriptType).name,                     {prop(PropDetachedScript::scriptType).name, sim_objecttype_detachedscript}},
    {prop(PropDetachedScript::DEPRECATED_scriptDisabled).name,                 {prop(PropDetachedScript::scriptDisabled).name, sim_objecttype_detachedscript}},
    {prop(PropDetachedScript::DEPRECATED_scriptState).name,                    {prop(PropDetachedScript::scriptState).name, sim_objecttype_detachedscript}},

    {prop(PropMesh::DEPRECATED_textureResolution).name,                        {prop(PropMesh::textureResolution).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_textureCoordinates).name,                       {prop(PropMesh::textureCoordinates).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_textureApplyMode).name,                         {prop(PropMesh::textureApplyMode).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_textureRepeatU).name,                           {prop(PropMesh::textureRepeatU).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_textureRepeatV).name,                           {prop(PropMesh::textureRepeatV).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_textureInterpolate).name,                       {prop(PropMesh::textureInterpolate).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_texture).name,                                  {prop(PropMesh::texture).name, sim_objecttype_mesh}},
    {prop(PropMesh::DEPRECATED_textureID).name,                                {prop(PropMesh::textureID).name, sim_objecttype_mesh}},

    {prop(PropSceneObject::DEPRECATED_parentHandle).name,                      {std::string(prop(PropSceneObject::parent).name) + "__noReplace__", sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_objectUid).name,                         {prop(PropSceneObject::uid).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelProperty).name,                     {prop(PropSceneObject::modelProperty).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotCollidable).name,                {prop(PropSceneObject::modelNotCollidable).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotMeasurable).name,                {prop(PropSceneObject::modelNotMeasurable).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotDetectable).name,                {prop(PropSceneObject::modelNotDetectable).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotDynamic).name,                   {prop(PropSceneObject::modelNotDynamic).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotRespondable).name,               {prop(PropSceneObject::modelNotRespondable).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotVisible).name,                   {prop(PropSceneObject::modelNotVisible).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelScriptsNotActive).name,             {prop(PropSceneObject::modelScriptsNotActive).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_modelNotInParentBB).name,                {prop(PropSceneObject::modelNotInParentBB).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_bbHsize).name,                           {prop(PropSceneObject::modelBBSize).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_alias).name,                             {prop(PropSceneObject::name).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movementOptions).name,                   {prop(PropSceneObject::movementOptions).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movementStepSize).name,                  {prop(PropSceneObject::movementStepSize).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movementRelativity).name,                {prop(PropSceneObject::movementRelativity).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movementPreferredAxes).name,             {prop(PropSceneObject::movementPreferredAxes).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movTranslNoSim).name,                    {prop(PropSceneObject::movTranslNoSim).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movTranslInSim).name,                    {prop(PropSceneObject::movTranslInSim).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movRotNoSim).name,                       {prop(PropSceneObject::movRotNoSim).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movRotInSim).name,                       {prop(PropSceneObject::movRotInSim).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movAltTransl).name,                      {prop(PropSceneObject::movAltTransl).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movAltRot).name,                         {prop(PropSceneObject::movAltRot).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movPrefTranslX).name,                    {prop(PropSceneObject::movPrefTranslX).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movPrefTranslY).name,                    {prop(PropSceneObject::movPrefTranslY).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movPrefTranslZ).name,                    {prop(PropSceneObject::movPrefTranslZ).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movPrefRotX).name,                       {prop(PropSceneObject::movPrefRotX).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movPrefRotY).name,                       {prop(PropSceneObject::movPrefRotY).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_movPrefRotZ).name,                       {prop(PropSceneObject::movPrefRotZ).name, sim_objecttype_sceneobject}},
    {prop(PropSceneObject::DEPRECATED_deprecatedName).name,                    {"__noReplace__", sim_objecttype_sceneobject}},

    {prop(PropForceSensor::DEPRECATED_sensorAverageForce).name,                {prop(PropForceSensor::filteredSensorForce).name, sim_sceneobject_forcesensor}},
    {prop(PropForceSensor::DEPRECATED_sensorAverageTorque).name,               {prop(PropForceSensor::filteredSensorTorque).name, sim_sceneobject_forcesensor}},
    {prop(PropForceSensor::DEPRECATED_sensorForce).name,                       {prop(PropForceSensor::sensorForce).name, sim_sceneobject_forcesensor}},
    {prop(PropForceSensor::DEPRECATED_sensorTorque).name,                      {prop(PropForceSensor::sensorTorque).name, sim_sceneobject_forcesensor}},

    {prop(PropScript::DEPRECATED_scriptDisabled).name,                         {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_restartOnError).name,                         {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_execPriority).name,                           {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_scriptType).name,                             {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_executionDepth).name,                         {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_scriptState).name,                            {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_language).name,                               {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_code).name,                                   {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_scriptName).name,                             {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_addOnPath).name,                              {"__noReplace__", sim_sceneobject_script}},
    {prop(PropScript::DEPRECATED_addOnMenuPath).name,                          {"__noReplace__", sim_sceneobject_script}},

    {prop(PropOctree::DEPRECATED_voxels).name,                                 {prop(PropOctree::points).name, sim_sceneobject_octree}},

    {prop(PropCamera::DEPRECATED_trackedObject).name,                          {std::string(prop(PropCamera::trackedObject).name) + "__noReplace__", sim_sceneobject_camera}},

    {prop(PropLight::DEPRECATED_lightType).name,                               {prop(PropLight::lightType).name, sim_sceneobject_light}},

    {prop(PropJoint::DEPRECATED_dependencyMaster).name,                        {std::string(prop(PropJoint::dependencyMaster).name) + "__noReplace__", sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_targetPos).name,                               {prop(PropJoint::targetPos).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_targetVel).name,                               {prop(PropJoint::targetPos).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_length).name,                                  {prop(PropJoint::length).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_diameter).name,                                {prop(PropJoint::diameter).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_jointType).name,                               {prop(PropJoint::jointType).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_jointForce).name,                              {prop(PropJoint::jointForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_averageJointForce).name,                       {prop(PropJoint::averageJointForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_jointMode).name,                               {prop(PropJoint::jointMode).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_dynCtrlMode).name,                             {prop(PropJoint::dynCtrlMode).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_springDamperParams).name,                      {prop(PropJoint::springDamperParams).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_dynVelMode).name,                              {prop(PropJoint::dynVelMode).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_dynPosMode).name,                              {prop(PropJoint::dynPosMode).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_engineProperties).name,                        {prop(PropJoint::engineProperties).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_bulletStopErp).name,                           {prop(PropJoint::bulletStopErp).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_bulletStopCfm).name,                           {prop(PropJoint::bulletStopCfm).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_bulletNormalCfm).name,                         {prop(PropJoint::bulletNormalCfm).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_bulletPosPid).name,                            {prop(PropJoint::bulletPosPid).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_odeStopErp).name,                              {prop(PropJoint::odeStopErp).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_odeStopCfm).name,                              {prop(PropJoint::odeStopCfm).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_odeNormalCfm).name,                            {prop(PropJoint::odeNormalCfm).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_odeBounce).name,                               {prop(PropJoint::odeBounce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_odeFudgeFactor).name,                          {prop(PropJoint::odeFudgeFactor).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_odePosPid).name,                               {prop(PropJoint::odePosPid).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexLowerLimitDamping).name,                 {prop(PropJoint::vortexLowerLimitDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexUpperLimitDamping).name,                 {prop(PropJoint::vortexUpperLimitDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexLowerLimitStiffness).name,               {prop(PropJoint::vortexLowerLimitStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexUpperLimitStiffness).name,               {prop(PropJoint::vortexUpperLimitStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexLowerLimitRestitution).name,             {prop(PropJoint::vortexLowerLimitRestitution).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexUpperLimitRestitution).name,             {prop(PropJoint::vortexUpperLimitRestitution).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexLowerLimitMaxForce).name,                {prop(PropJoint::vortexLowerLimitMaxForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexUpperLimitMaxForce).name,                {prop(PropJoint::vortexUpperLimitMaxForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexAxisFrictionEnabled).name,               {prop(PropJoint::vortexAxisFrictionEnabled).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexAxisFrictionProportional).name,          {prop(PropJoint::vortexAxisFrictionProportional).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexAxisFrictionCoeff).name,                 {prop(PropJoint::vortexAxisFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexAxisFrictionMaxForce).name,              {prop(PropJoint::vortexAxisFrictionMaxForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexAxisFrictionLoss).name,                  {prop(PropJoint::vortexAxisFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexRelaxationEnabledBits).name,             {prop(PropJoint::vortexRelaxationEnabledBits).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexFrictionEnabledBits).name,               {prop(PropJoint::vortexFrictionEnabledBits).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexFrictionProportionalBits).name,          {prop(PropJoint::vortexFrictionProportionalBits).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisPosRelaxationStiffness).name,       {prop(PropJoint::vortexXAxisPosRelaxationStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisPosRelaxationDamping).name,         {prop(PropJoint::vortexXAxisPosRelaxationDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisPosRelaxationLoss).name,            {prop(PropJoint::vortexXAxisPosRelaxationLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisPosFrictionCoeff).name,             {prop(PropJoint::vortexXAxisPosFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisPosFrictionMaxForce).name,          {prop(PropJoint::vortexXAxisPosFrictionMaxForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisPosFrictionLoss).name,              {prop(PropJoint::vortexXAxisPosFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisPosRelaxationStiffness).name,       {prop(PropJoint::vortexYAxisPosRelaxationStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisPosRelaxationDamping).name,         {prop(PropJoint::vortexYAxisPosRelaxationDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisPosRelaxationLoss).name,            {prop(PropJoint::vortexYAxisPosRelaxationLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisPosFrictionCoeff).name,             {prop(PropJoint::vortexYAxisPosFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisPosFrictionMaxForce).name,          {prop(PropJoint::vortexYAxisPosFrictionMaxForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisPosFrictionLoss).name,              {prop(PropJoint::vortexYAxisPosFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisPosRelaxationStiffness).name,       {prop(PropJoint::vortexZAxisPosRelaxationStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisPosRelaxationDamping).name,         {prop(PropJoint::vortexZAxisPosRelaxationDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisPosRelaxationLoss).name,            {prop(PropJoint::vortexZAxisPosRelaxationLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisPosFrictionCoeff).name,             {prop(PropJoint::vortexZAxisPosFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisPosFrictionMaxForce).name,          {prop(PropJoint::vortexZAxisPosFrictionMaxForce).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisPosFrictionLoss).name,              {prop(PropJoint::vortexZAxisPosFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisOrientRelaxStiffness).name,         {prop(PropJoint::vortexXAxisOrientRelaxStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisOrientRelaxDamping).name,           {prop(PropJoint::vortexXAxisOrientRelaxDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisOrientRelaxLoss).name,              {prop(PropJoint::vortexXAxisOrientRelaxLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisOrientFrictionCoeff).name,          {prop(PropJoint::vortexXAxisOrientFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisOrientFrictionMaxTorque).name,      {prop(PropJoint::vortexXAxisOrientFrictionMaxTorque).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexXAxisOrientFrictionLoss).name,           {prop(PropJoint::vortexXAxisOrientFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisOrientRelaxStiffness).name,         {prop(PropJoint::vortexYAxisOrientRelaxStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisOrientRelaxDamping).name,           {prop(PropJoint::vortexYAxisOrientRelaxDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisOrientRelaxLoss).name,              {prop(PropJoint::vortexYAxisOrientRelaxLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisOrientFrictionCoeff).name,          {prop(PropJoint::vortexYAxisOrientFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisOrientFrictionMaxTorque).name,      {prop(PropJoint::vortexYAxisOrientFrictionMaxTorque).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexYAxisOrientFrictionLoss).name,           {prop(PropJoint::vortexYAxisOrientFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisOrientRelaxStiffness).name,         {prop(PropJoint::vortexZAxisOrientRelaxStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisOrientRelaxDamping).name,           {prop(PropJoint::vortexZAxisOrientRelaxDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisOrientRelaxLoss).name,              {prop(PropJoint::vortexZAxisOrientRelaxLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisOrientFrictionCoeff).name,          {prop(PropJoint::vortexZAxisOrientFrictionCoeff).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisOrientFrictionMaxTorque).name,      {prop(PropJoint::vortexZAxisOrientFrictionMaxTorque).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexZAxisOrientFrictionLoss).name,           {prop(PropJoint::vortexZAxisOrientFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_vortexPosPid).name,                            {prop(PropJoint::vortexPosPid).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_newtonPosPid).name,                            {prop(PropJoint::newtonPosPid).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoArmature).name,                          {prop(PropJoint::mujocoArmature).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoMargin).name,                            {prop(PropJoint::mujocoMargin).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoFrictionLoss).name,                      {prop(PropJoint::mujocoFrictionLoss).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoSpringStiffness).name,                   {prop(PropJoint::mujocoSpringStiffness).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoSpringDamping).name,                     {prop(PropJoint::mujocoSpringDamping).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoSpringRef).name,                         {prop(PropJoint::mujocoSpringRef).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoPosPid).name,                            {prop(PropJoint::mujocoPosPid).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoLimitsSolRef).name,                      {prop(PropJoint::mujocoLimitsSolRef).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoLimitsSolImp).name,                      {prop(PropJoint::mujocoLimitsSolImp).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoFrictionSolRef).name,                    {prop(PropJoint::mujocoFrictionSolRef).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoFrictionSolImp).name,                    {prop(PropJoint::mujocoFrictionSolImp).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoSpringDamper).name,                      {prop(PropJoint::mujocoSpringDamper).name, sim_sceneobject_joint}},
    {prop(PropJoint::DEPRECATED_mujocoDependencyPolyCoef).name,                {prop(PropJoint::mujocoDependencyPolyCoef).name, sim_sceneobject_joint}},

    {prop(PropDummy::DEPRECATED_linkedDummyHandle).name,                       {std::string(prop(PropDummy::linkedDummy).name) + "__noReplace__", sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_dummyType).name,                               {prop(PropDummy::dummyType).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_engineProperties).name,                        {prop(PropDummy::engineProperties).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoLimitsEnabled).name,                     {prop(PropDummy::mujocoLimitsEnabled).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoLimitsRange).name,                       {prop(PropDummy::mujocoLimitsRange).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoLimitsSolref).name,                      {prop(PropDummy::mujocoLimitsSolref).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoLimitsSolimp).name,                      {prop(PropDummy::mujocoLimitsSolimp).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoMargin).name,                            {prop(PropDummy::mujocoMargin).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoSpringStiffness).name,                   {prop(PropDummy::mujocoSpringStiffness).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoSpringDamping).name,                     {prop(PropDummy::mujocoSpringDamping).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoSpringLength).name,                      {prop(PropDummy::mujocoSpringLength).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoJointProxyHandle).name,                  {prop(PropDummy::mujocoJointProxyHandle).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoOverlapConstrSolref).name,               {prop(PropDummy::mujocoOverlapConstrSolref).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoOverlapConstrSolimp).name,               {prop(PropDummy::mujocoOverlapConstrSolimp).name, sim_sceneobject_dummy}},
    {prop(PropDummy::DEPRECATED_mujocoOverlapConstrTorqueScale).name,          {prop(PropDummy::mujocoOverlapConstrTorqueScale).name, sim_sceneobject_dummy}},

    {prop(PropMeshWrapper::DEPRECATED_pmi).name,                                              {std::string(prop(PropMeshWrapper::pmi).name) + "__noReplace__", sim_sceneobject_shape}},
    {prop(PropMeshWrapper::DEPRECATED_inertia).name,                                          {std::string(prop(PropMeshWrapper::inertiaMatrix).name) + "__noReplace__", sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_engineProperties).name,                                    {prop(PropMaterial::engineProperties).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletRestitution).name,                                   {prop(PropMaterial::bulletRestitution).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletFriction0).name,                                     {prop(PropMaterial::bulletFriction0).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletFriction).name,                                      {prop(PropMaterial::bulletFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletLinearDamping).name,                                 {prop(PropMaterial::bulletLinearDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletAngularDamping).name,                                {prop(PropMaterial::bulletAngularDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMarginFactor).name,               {prop(PropMaterial::bulletNonDefaultCollisionMarginFactor).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMarginFactorConvex).name,         {prop(PropMaterial::bulletNonDefaultCollisionMarginFactorConvex).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletSticky).name,                                        {prop(PropMaterial::bulletSticky).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMargin).name,                     {prop(PropMaterial::bulletNonDefaultCollisionMargin).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletNonDefaultCollisionMarginConvex).name,               {prop(PropMaterial::bulletNonDefaultCollisionMarginConvex).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_bulletAutoShrinkConvex).name,                              {prop(PropMaterial::bulletAutoShrinkConvex).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_odeFriction).name,                                         {prop(PropMaterial::odeFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_odeSoftErp).name,                                          {prop(PropMaterial::odeSoftErp).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_odeSoftCfm).name,                                          {prop(PropMaterial::odeSoftCfm).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_odeLinearDamping).name,                                    {prop(PropMaterial::odeLinearDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_odeAngularDamping).name,                                   {prop(PropMaterial::odeAngularDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_odeMaxContacts).name,                                      {prop(PropMaterial::odeMaxContacts).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisFriction).name,                     {prop(PropMaterial::vortexPrimaryLinearAxisFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisFriction).name,                   {prop(PropMaterial::vortexSecondaryLinearAxisFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisFriction).name,                    {prop(PropMaterial::vortexPrimaryAngularAxisFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisFriction).name,                  {prop(PropMaterial::vortexSecondaryAngularAxisFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisFriction).name,                     {prop(PropMaterial::vortexNormalAngularAxisFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisStaticFrictionScale).name,          {prop(PropMaterial::vortexPrimaryLinearAxisStaticFrictionScale).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisStaticFrictionScale).name,        {prop(PropMaterial::vortexSecondaryLinearAxisStaticFrictionScale).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisStaticFrictionScale).name,         {prop(PropMaterial::vortexPrimaryAngularAxisStaticFrictionScale).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisStaticFrictionScale).name,       {prop(PropMaterial::vortexSecondaryAngularAxisStaticFrictionScale).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisStaticFrictionScale).name,          {prop(PropMaterial::vortexNormalAngularAxisStaticFrictionScale).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexCompliance).name,                                    {prop(PropMaterial::vortexCompliance).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexDamping).name,                                       {prop(PropMaterial::vortexDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexRestitution).name,                                   {prop(PropMaterial::vortexRestitution).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexRestitutionThreshold).name,                          {prop(PropMaterial::vortexRestitutionThreshold).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAdhesiveForce).name,                                 {prop(PropMaterial::vortexAdhesiveForce).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexLinearVelocityDamping).name,                         {prop(PropMaterial::vortexLinearVelocityDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAngularVelocityDamping).name,                        {prop(PropMaterial::vortexAngularVelocityDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisSlide).name,                        {prop(PropMaterial::vortexPrimaryLinearAxisSlide).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisSlide).name,                      {prop(PropMaterial::vortexSecondaryLinearAxisSlide).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisSlide).name,                       {prop(PropMaterial::vortexPrimaryAngularAxisSlide).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisSlide).name,                     {prop(PropMaterial::vortexSecondaryAngularAxisSlide).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisSlide).name,                        {prop(PropMaterial::vortexNormalAngularAxisSlide).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisSlip).name,                         {prop(PropMaterial::vortexPrimaryLinearAxisSlip).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisSlip).name,                       {prop(PropMaterial::vortexSecondaryLinearAxisSlip).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisSlip).name,                        {prop(PropMaterial::vortexPrimaryAngularAxisSlip).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisSlip).name,                      {prop(PropMaterial::vortexSecondaryAngularAxisSlip).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisSlip).name,                         {prop(PropMaterial::vortexNormalAngularAxisSlip).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoSleepLinearSpeedThreshold).name,                 {prop(PropMaterial::vortexAutoSleepLinearSpeedThreshold).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoSleepLinearAccelerationThreshold).name,          {prop(PropMaterial::vortexAutoSleepLinearAccelerationThreshold).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoSleepAngularSpeedThreshold).name,                {prop(PropMaterial::vortexAutoSleepAngularSpeedThreshold).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoSleepAngularAccelerationThreshold).name,         {prop(PropMaterial::vortexAutoSleepAngularAccelerationThreshold).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSkinThickness).name,                                 {prop(PropMaterial::vortexSkinThickness).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoAngularDampingTensionRatio).name,                {prop(PropMaterial::vortexAutoAngularDampingTensionRatio).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryAxisVector).name,                             {prop(PropMaterial::vortexPrimaryAxisVector).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryLinearAxisFrictionModel).name,                {prop(PropMaterial::vortexPrimaryLinearAxisFrictionModel).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisFrictionModel).name,              {prop(PropMaterial::vortexSecondaryLinearAxisFrictionModel).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimaryAngularAxisFrictionModel).name,               {prop(PropMaterial::vortexPrimaryAngularAxisFrictionModel).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisFrictionModel).name,             {prop(PropMaterial::vortexSecondaryAngularAxisFrictionModel).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisFrictionModel).name,                {prop(PropMaterial::vortexNormalAngularAxisFrictionModel).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoSleepStepLiveThreshold).name,                    {prop(PropMaterial::vortexAutoSleepStepLiveThreshold).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexMaterialUniqueId).name,                              {prop(PropMaterial::vortexMaterialUniqueId).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexPrimitiveShapesAsConvex).name,                       {prop(PropMaterial::vortexPrimitiveShapesAsConvex).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexConvexShapesAsRandom).name,                          {prop(PropMaterial::vortexConvexShapesAsRandom).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexRandomShapesAsTerrain).name,                         {prop(PropMaterial::vortexRandomShapesAsTerrain).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexFastMoving).name,                                    {prop(PropMaterial::vortexFastMoving).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoSlip).name,                                      {prop(PropMaterial::vortexAutoSlip).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis).name,    {prop(PropMaterial::vortexSecondaryLinearAxisSameAsPrimaryLinearAxis).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis).name,  {prop(PropMaterial::vortexSecondaryAngularAxisSameAsPrimaryAngularAxis).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexNormalAngularAxisSameAsPrimaryAngularAxis).name,     {prop(PropMaterial::vortexNormalAngularAxisSameAsPrimaryAngularAxis).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_vortexAutoAngularDamping).name,                            {prop(PropMaterial::vortexAutoAngularDamping).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_newtonStaticFriction).name,                                {prop(PropMaterial::newtonStaticFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_newtonKineticFriction).name,                               {prop(PropMaterial::newtonKineticFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_newtonRestitution).name,                                   {prop(PropMaterial::newtonRestitution).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_newtonLinearDrag).name,                                    {prop(PropMaterial::newtonLinearDrag).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_newtonAngularDrag).name,                                   {prop(PropMaterial::newtonAngularDrag).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_newtonFastMoving).name,                                    {prop(PropMaterial::newtonFastMoving).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoFriction).name,                                      {prop(PropMaterial::mujocoFriction).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoSolref).name,                                        {prop(PropMaterial::mujocoSolref).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoSolimp).name,                                        {prop(PropMaterial::mujocoSolimp).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoSolmix).name,                                        {prop(PropMaterial::mujocoSolmix).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoMargin).name,                                        {prop(PropMaterial::mujocoMargin).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoGap).name,                                           {prop(PropMaterial::mujocoGap).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoCondim).name,                                        {prop(PropMaterial::mujocoCondim).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoPriority).name,                                      {prop(PropMaterial::mujocoPriority).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoAdhesion).name,                                      {prop(PropMaterial::mujocoAdhesion).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoAdhesionGain).name,                                  {prop(PropMaterial::mujocoAdhesionGain).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoAdhesionForcelimited).name,                          {prop(PropMaterial::mujocoAdhesionForcelimited).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoAdhesionCtrlrange).name,                             {prop(PropMaterial::mujocoAdhesionCtrlrange).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoAdhesionForcerange).name,                            {prop(PropMaterial::mujocoAdhesionForcerange).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoAdhesionCtrl).name,                                  {prop(PropMaterial::mujocoAdhesionCtrl).name, sim_sceneobject_shape}},
    {prop(PropMaterial::DEPRECATED_mujocoGravcomp).name,                                      {prop(PropMaterial::mujocoGravcomp).name, sim_sceneobject_shape}},

    {prop(PropConvexVolume::DEPRECATED_offset).name,                           {prop(PropConvexVolume::offset).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_range).name,                            {prop(PropConvexVolume::range).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_xSize).name,                            {prop(PropConvexVolume::xSize).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_ySize).name,                            {prop(PropConvexVolume::ySize).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_radius).name,                           {prop(PropConvexVolume::radius).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_angle).name,                            {prop(PropConvexVolume::angle).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_faces).name,                            {prop(PropConvexVolume::faces).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_subdivisions).name,                     {prop(PropConvexVolume::subdivisions).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_edges).name,                            {prop(PropConvexVolume::edges).name, sim_sceneobject_proximitysensor}},
    {prop(PropConvexVolume::DEPRECATED_closeEdges).name,                       {prop(PropConvexVolume::closeEdges).name, sim_sceneobject_proximitysensor}},
    {prop(PropProximitySensor::DEPRECATED_size).name,                          {prop(PropProximitySensor::size).name, sim_sceneobject_proximitysensor}},
    {prop(PropProximitySensor::DEPRECATED_sensorType).name,                    {prop(PropProximitySensor::sensorType).name, sim_sceneobject_proximitysensor}},
    {prop(PropProximitySensor::DEPRECATED_detectedObjectHandle).name,          {std::string(prop(PropProximitySensor::detectedObject).name) + "__noReplace__", sim_sceneobject_proximitysensor}},

    {prop(PropVisionSensor::DEPRECATED_depthBuffer).name,                      {std::string(prop(PropVisionSensor::depthBuffer).name) + "__noReplace__", sim_sceneobject_visionsensor}},

};

const std::map<std::string, SDeprecatedProperty> propDeprecationMapping = []() {
    std::map<std::string, SDeprecatedProperty> result;
    for (size_t i = 0; i < tmpPairs.size(); i++)
    {
        std::string key = tmpPairs[i].first;
        auto item = tmpPairs[i].second;
        auto it = result.find(key);
        if (it != result.end())
        {
            auto& types = it->second.types;
            auto& repls = it->second.replacements;
            types.push_back(item.type);
            repls.push_back(item.repl);
        }
        else
            result[key] = {{item.repl}, {item.type}};
    }
    return result;
}();
