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
    STACK_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
        extern const std::vector<SProperty> allProps_stack = {STACK_PROPERTIES};
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
    SCENE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_scene = {SCENE_PROPERTIES};
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
    MARKER_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_marker = {MARKER_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    CUSTOMSCENEOBJECT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_customSceneObject = {CUSTOMSCENEOBJECT_PROPERTIES};
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
    CUSTOMOBJECTCLASS_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_customObjectClass = {CUSTOMOBJECTCLASS_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    CUSTOMOBJECT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_customObject = {CUSTOMOBJECT_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    CUSTOMSCENEOBJECTCLASS_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_customSceneObjectClass = {CUSTOMSCENEOBJECTCLASS_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    DETACHEDSCRIPT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    extern const std::vector<SProperty> allProps_detachedScript = {DETACHEDSCRIPT_PROPERTIES};
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
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name = {str, v1, v2, t1, t2};
    OBJECT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
        extern const std::vector<SProperty> allProps_obj = {OBJECT_PROPERTIES};
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

struct SDeprecatedPropertyItems {
    std::string repl;
    int type;
};

const std::map<std::string, SDeprecatedPropertyItems> tmpMap = {
    {propApp_DEPRECATED_appArg1.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg2.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg3.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg4.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg5.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg6.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg7.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg8.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appArg9.name,                                   {std::string(propApp_appArgs.name) + "__noReplace__", sim_objecttype_app}},
    {propApp_DEPRECATED_appDir.name,                                    {propApp_appDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_tempDir.name,                                   {propApp_tempDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_sceneTempDir.name,                              {propApp_sceneTempDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_settingsDir.name,                               {propApp_settingsDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_luaDir.name,                                    {propApp_luaDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_pythonDir.name,                                 {propApp_pythonDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_mujocoDir.name,                                 {propApp_mujocoDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_systemDir.name,                                 {propApp_systemDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_resourceDir.name,                               {propApp_resourceDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_addOnDir.name,                                  {propApp_addOnDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_sceneDir.name,                                  {propApp_sceneDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_modelDir.name,                                  {propApp_modelDir.name, sim_objecttype_app}},
    {propApp_DEPRECATED_importExportDir.name,                           {propApp_importExportDir.name, sim_objecttype_app}},

    {propScene_DEPRECATED_sceneIsLocked.name,                           {propScene_sceneIsLocked.name, sim_objecttype_scene}},
    {propScene_DEPRECATED_sceneUid.name,                                {propScene_sceneUid.name, sim_objecttype_scene}},
    {propScene_DEPRECATED_sceneUidString.name,                          {propScene_sceneUidString.name, sim_objecttype_scene}},
    {propScene_DEPRECATED_scenePath.name,                               {propScene_scenePath.name, sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_objectHandles.name,                 {std::string(propSceneObjectCont_objects.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_orphanHandles.name,                 {std::string(propSceneObjectCont_orphans.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_selectionHandles.name,              {std::string(propSceneObjectCont_selection.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_objectHandles.name,                 {std::string(propSceneObjectCont_objects.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_orphanHandles.name,                 {std::string(propSceneObjectCont_orphans.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_selectionHandles.name,              {std::string(propSceneObjectCont_selection.name) + "__noReplace__", sim_objecttype_scene}},
    {propDynCont_DEPRECATED_dynamicsEnabled.name,                       {propDynCont_dynamicsEnabled.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_showContactPoints.name,                     {propDynCont_showContactPoints.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_dynamicsEngine.name,                        {propDynCont_dynamicsEngine.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_dynamicsStepSize.name,                      {propDynCont_dynamicsStepSize.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_gravity.name,                               {propDynCont_gravity.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_engineProperties.name,                      {propDynCont_engineProperties.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_bulletSolver.name,                          {propDynCont_bulletSolver.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_bulletIterations.name,                      {propDynCont_bulletIterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_bulletComputeInertias.name,                 {propDynCont_bulletComputeInertias.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_bulletInternalScalingFull.name,             {propDynCont_bulletInternalScalingFull.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_bulletInternalScalingScaling.name,          {propDynCont_bulletInternalScalingScaling.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_bulletCollMarginScaling.name,               {propDynCont_bulletCollMarginScaling.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeQuickStepEnabled.name,                   {propDynCont_odeQuickStepEnabled.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeQuickStepIterations.name,                {propDynCont_odeQuickStepIterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeComputeInertias.name,                    {propDynCont_odeComputeInertias.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeInternalScalingFull.name,                {propDynCont_odeInternalScalingFull.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeInternalScalingScaling.name,             {propDynCont_odeInternalScalingScaling.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeGlobalErp.name,                          {propDynCont_odeGlobalErp.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_odeGlobalCfm.name,                          {propDynCont_odeGlobalCfm.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexComputeInertias.name,                 {propDynCont_vortexComputeInertias.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexContactTolerance.name,                {propDynCont_vortexContactTolerance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexAutoSleep.name,                       {propDynCont_vortexAutoSleep.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexMultithreading.name,                  {propDynCont_vortexMultithreading.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexConstraintsLinearCompliance.name,     {propDynCont_vortexConstraintsLinearCompliance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexConstraintsLinearDamping.name,        {propDynCont_vortexConstraintsLinearDamping.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexConstraintsLinearKineticLoss.name,    {propDynCont_vortexConstraintsLinearKineticLoss.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexConstraintsAngularCompliance.name,    {propDynCont_vortexConstraintsAngularCompliance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexConstraintsAngularDamping.name,       {propDynCont_vortexConstraintsAngularDamping.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_vortexConstraintsAngularKineticLoss.name,   {propDynCont_vortexConstraintsAngularKineticLoss.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_newtonIterations.name,                      {propDynCont_newtonIterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_newtonComputeInertias.name,                 {propDynCont_newtonComputeInertias.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_newtonMultithreading.name,                  {propDynCont_newtonMultithreading.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_newtonExactSolver.name,                     {propDynCont_newtonExactSolver.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_newtonHighJointAccuracy.name,               {propDynCont_newtonHighJointAccuracy.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_newtonContactMergeTolerance.name,           {propDynCont_newtonContactMergeTolerance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoIntegrator.name,                      {propDynCont_mujocoIntegrator.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoSolver.name,                          {propDynCont_mujocoSolver.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoIterations.name,                      {propDynCont_mujocoIterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoRebuildTrigger.name,                  {propDynCont_mujocoRebuildTrigger.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoComputeInertias.name,                 {propDynCont_mujocoComputeInertias.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoMbMemory.name,                        {propDynCont_mujocoMbMemory.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoNjMax.name,                           {propDynCont_mujocoNjMax.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoNconMax.name,                         {propDynCont_mujocoNconMax.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoNstack.name,                          {propDynCont_mujocoNstack.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoCone.name,                            {propDynCont_mujocoCone.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoKinematicBodiesOverrideFlags.name,    {propDynCont_mujocoKinematicBodiesOverrideFlags.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoKinematicBodiesMass.name,             {propDynCont_mujocoKinematicBodiesMass.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoKinematicBodiesInertia.name,          {propDynCont_mujocoKinematicBodiesInertia.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoBoundMass.name,                       {propDynCont_mujocoBoundMass.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoBoundInertia.name,                    {propDynCont_mujocoBoundInertia.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoBalanceInertias.name,                 {propDynCont_mujocoBalanceInertias.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoMultithreaded.name,                   {propDynCont_mujocoMultithreaded.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoMulticcd.name,                        {propDynCont_mujocoMulticcd.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoContactParamsOverride.name,           {propDynCont_mujocoContactParamsOverride.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoContactParamsMargin.name,             {propDynCont_mujocoContactParamsMargin.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoContactParamsSolref.name,             {propDynCont_mujocoContactParamsSolref.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoContactParamsSolimp.name,             {propDynCont_mujocoContactParamsSolimp.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoImpRatio.name,                        {propDynCont_mujocoImpRatio.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoWind.name,                            {propDynCont_mujocoWind.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoDensity.name,                         {propDynCont_mujocoDensity.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoViscosity.name,                       {propDynCont_mujocoViscosity.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoJacobian.name,                        {propDynCont_mujocoJacobian.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoTolerance.name,                       {propDynCont_mujocoTolerance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoLs_iterations.name,                   {propDynCont_mujocoLs_iterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoLs_tolerance.name,                    {propDynCont_mujocoLs_tolerance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoNoslip_iterations.name,               {propDynCont_mujocoNoslip_iterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoNoslip_tolerance.name,                {propDynCont_mujocoNoslip_tolerance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoCcd_iterations.name,                  {propDynCont_mujocoCcd_iterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoCcd_tolerance.name,                   {propDynCont_mujocoCcd_tolerance.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoSdf_iterations.name,                  {propDynCont_mujocoSdf_iterations.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoSdf_initpoints.name,                  {propDynCont_mujocoSdf_initpoints.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoEqualityEnable.name,                  {propDynCont_mujocoEqualityEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoFrictionlossEnable.name,              {propDynCont_mujocoFrictionlossEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoLimitEnable.name,                     {propDynCont_mujocoLimitEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoContactEnable.name,                   {propDynCont_mujocoContactEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoPassiveEnable.name,                   {propDynCont_mujocoPassiveEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoGravityEnable.name,                   {propDynCont_mujocoGravityEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoWarmstartEnable.name,                 {propDynCont_mujocoWarmstartEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoActuationEnable.name,                 {propDynCont_mujocoActuationEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoRefsafeEnable.name,                   {propDynCont_mujocoRefsafeEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoSensorEnable.name,                    {propDynCont_mujocoSensorEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoMidphaseEnable.name,                  {propDynCont_mujocoMidphaseEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoEulerdampEnable.name,                 {propDynCont_mujocoEulerdampEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoAutoresetEnable.name,                 {propDynCont_mujocoAutoresetEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoEnergyEnable.name,                    {propDynCont_mujocoEnergyEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoInvdiscreteEnable.name,               {propDynCont_mujocoInvdiscreteEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoNativeccdEnable.name,                 {propDynCont_mujocoNativeccdEnable.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoAlignfree.name,                       {propDynCont_mujocoAlignfree.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoKinematicWeldSolref.name,             {propDynCont_mujocoKinematicWeldSolref.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoKinematicWeldSolimp.name,             {propDynCont_mujocoKinematicWeldSolimp.name, sim_objecttype_scene}},
    {propDynCont_DEPRECATED_mujocoKinematicWeldTorqueScale.name,        {propDynCont_mujocoKinematicWeldTorqueScale.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_removeNewObjectsAtEnd.name,              {propSimulation_removeNewObjectsAtEnd.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_realtimeSimulation.name,                 {propSimulation_realtimeSimulation.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_pauseSimulationAtTime.name,              {propSimulation_pauseSimulationAtTime.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_pauseSimulationAtError.name,             {propSimulation_pauseSimulationAtError.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_simulationTime.name,                     {propSimulation_simulationTime.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_timeStep.name,                           {propSimulation_timeStep.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_timeToPause.name,                        {propSimulation_timeToPause.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_realtimeModifier.name,                   {propSimulation_realtimeModifier.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_stepCount.name,                          {propSimulation_stepCount.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_simulationState.name,                    {propSimulation_simulationState.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_stepsPerRendering.name,                  {propSimulation_stepsPerRendering.name, sim_objecttype_scene}},
    {propSimulation_DEPRECATED_speedModifier.name,                      {propSimulation_speedModifier.name, sim_objecttype_scene}},

    {propDetachedScript_DEPRECATED_scriptType.name,                     {propDetachedScript_scriptType.name, sim_objecttype_detachedscript}},
    {propDetachedScript_DEPRECATED_scriptDisabled.name,                 {propDetachedScript_scriptDisabled.name, sim_objecttype_detachedscript}},
    {propDetachedScript_DEPRECATED_scriptState.name,                    {propDetachedScript_scriptState.name, sim_objecttype_detachedscript}},

    {propMesh_DEPRECATED_textureResolution.name,                        {propMesh_textureResolution.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_textureCoordinates.name,                       {propMesh_textureCoordinates.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_textureApplyMode.name,                         {propMesh_textureApplyMode.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_textureRepeatU.name,                           {propMesh_textureRepeatU.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_textureRepeatV.name,                           {propMesh_textureRepeatV.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_textureInterpolate.name,                       {propMesh_textureInterpolate.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_texture.name,                                  {propMesh_texture.name, sim_objecttype_mesh}},
    {propMesh_DEPRECATED_textureID.name,                                {propMesh_textureID.name, sim_objecttype_mesh}},

    {propSceneObject_DEPRECATED_parentHandle.name,                      {std::string(propSceneObject_parent.name) + "__noReplace__", sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelProperty.name,                     {propSceneObject_modelProperty.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotCollidable.name,                {propSceneObject_modelNotCollidable.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotMeasurable.name,                {propSceneObject_modelNotMeasurable.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotDetectable.name,                {propSceneObject_modelNotDetectable.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotDynamic.name,                   {propSceneObject_modelNotDynamic.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotRespondable.name,               {propSceneObject_modelNotRespondable.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotVisible.name,                   {propSceneObject_modelNotVisible.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelScriptsNotActive.name,             {propSceneObject_modelScriptsNotActive.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_modelNotInParentBB.name,                {propSceneObject_modelNotInParentBB.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_bbHsize.name,                           {propSceneObject_modelBBSize.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_alias.name,                             {propSceneObject_name.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movementOptions.name,                   {propSceneObject_movementOptions.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movementStepSize.name,                  {propSceneObject_movementStepSize.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movementRelativity.name,                {propSceneObject_movementRelativity.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movementPreferredAxes.name,             {propSceneObject_movementPreferredAxes.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movTranslNoSim.name,                    {propSceneObject_movTranslNoSim.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movTranslInSim.name,                    {propSceneObject_movTranslInSim.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movRotNoSim.name,                       {propSceneObject_movRotNoSim.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movRotInSim.name,                       {propSceneObject_movRotInSim.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movAltTransl.name,                      {propSceneObject_movAltTransl.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movAltRot.name,                         {propSceneObject_movAltRot.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movPrefTranslX.name,                    {propSceneObject_movPrefTranslX.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movPrefTranslY.name,                    {propSceneObject_movPrefTranslY.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movPrefTranslZ.name,                    {propSceneObject_movPrefTranslZ.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movPrefRotX.name,                       {propSceneObject_movPrefRotX.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movPrefRotY.name,                       {propSceneObject_movPrefRotY.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_movPrefRotZ.name,                       {propSceneObject_movPrefRotZ.name, sim_objecttype_sceneobject}},
    {propSceneObject_DEPRECATED_deprecatedName.name,                    {"__noReplace__", sim_objecttype_sceneobject}},

    {propForceSensor_DEPRECATED_sensorAverageForce.name,                {propForceSensor_filteredSensorForce.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_sensorAverageTorque.name,               {propForceSensor_filteredSensorTorque.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_sensorForce.name,                       {propForceSensor_sensorForce.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_sensorTorque.name,                      {propForceSensor_sensorTorque.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_size.name,                              {propForceSensor_size.name, sim_sceneobject_forcesensor}},

    {propScript_DEPRECATED_size.name,                                   {propScript_size.name, sim_sceneobject_script}},
    {propScript_DEPRECATED_scriptDisabled.name,                         {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_restartOnError.name,                         {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_execPriority.name,                           {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_scriptType.name,                             {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_executionDepth.name,                         {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_scriptState.name,                            {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_language.name,                               {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_code.name,                                   {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_scriptName.name,                             {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_addOnPath.name,                              {"__noReplace__", sim_sceneobject_script}},
    {propScript_DEPRECATED_addOnMenuPath.name,                          {"__noReplace__", sim_sceneobject_script}},

    {propOctree_DEPRECATED_voxels.name,                                 {propOctree_points.name, sim_sceneobject_octree}},

    {propGraph_DEPRECATED_size.name,                                    {propGraph_size.name, sim_sceneobject_graph}},

    {propCamera_DEPRECATED_size.name,                                   {propCamera_size.name, sim_sceneobject_camera}},
    {propCamera_DEPRECATED_trackedObject.name,                          {std::string(propCamera_trackedObject.name) + "__noReplace__", sim_sceneobject_camera}},

    {propVisionSensor_DEPRECATED_size.name,                             {propVisionSensor_size.name, sim_sceneobject_visionsensor}},

    {propLight_DEPRECATED_size.name,                                    {propLight_size.name, sim_sceneobject_light}},
    {propLight_DEPRECATED_lightType.name,                               {propLight_lightType.name, sim_sceneobject_light}},

    {propJoint_DEPRECATED_dependencyMaster.name,                        {std::string(propJoint_dependencyMaster.name) + "__noReplace__", sim_sceneobject_joint}},
    {propJoint_DEPRECATED_length.name,                                  {propJoint_length.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_diameter.name,                                {propJoint_diameter.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_jointType.name,                               {propJoint_jointType.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_jointForce.name,                              {propJoint_jointForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_averageJointForce.name,                       {propJoint_averageJointForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_jointMode.name,                               {propJoint_jointMode.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_dynCtrlMode.name,                             {propJoint_dynCtrlMode.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_springDamperParams.name,                      {propJoint_springDamperParams.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_dynVelMode.name,                              {propJoint_dynVelMode.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_dynPosMode.name,                              {propJoint_dynPosMode.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_engineProperties.name,                        {propJoint_engineProperties.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_bulletStopErp.name,                           {propJoint_bulletStopErp.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_bulletStopCfm.name,                           {propJoint_bulletStopCfm.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_bulletNormalCfm.name,                         {propJoint_bulletNormalCfm.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_bulletPosPid.name,                            {propJoint_bulletPosPid.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_odeStopErp.name,                              {propJoint_odeStopErp.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_odeStopCfm.name,                              {propJoint_odeStopCfm.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_odeNormalCfm.name,                            {propJoint_odeNormalCfm.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_odeBounce.name,                               {propJoint_odeBounce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_odeFudgeFactor.name,                          {propJoint_odeFudgeFactor.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_odePosPid.name,                               {propJoint_odePosPid.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexLowerLimitDamping.name,                 {propJoint_vortexLowerLimitDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexUpperLimitDamping.name,                 {propJoint_vortexUpperLimitDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexLowerLimitStiffness.name,               {propJoint_vortexLowerLimitStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexUpperLimitStiffness.name,               {propJoint_vortexUpperLimitStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexLowerLimitRestitution.name,             {propJoint_vortexLowerLimitRestitution.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexUpperLimitRestitution.name,             {propJoint_vortexUpperLimitRestitution.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexLowerLimitMaxForce.name,                {propJoint_vortexLowerLimitMaxForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexUpperLimitMaxForce.name,                {propJoint_vortexUpperLimitMaxForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexAxisFrictionEnabled.name,               {propJoint_vortexAxisFrictionEnabled.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexAxisFrictionProportional.name,          {propJoint_vortexAxisFrictionProportional.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexAxisFrictionCoeff.name,                 {propJoint_vortexAxisFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexAxisFrictionMaxForce.name,              {propJoint_vortexAxisFrictionMaxForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexAxisFrictionLoss.name,                  {propJoint_vortexAxisFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexRelaxationEnabledBits.name,             {propJoint_vortexRelaxationEnabledBits.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexFrictionEnabledBits.name,               {propJoint_vortexFrictionEnabledBits.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexFrictionProportionalBits.name,          {propJoint_vortexFrictionProportionalBits.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisPosRelaxationStiffness.name,       {propJoint_vortexXAxisPosRelaxationStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisPosRelaxationDamping.name,         {propJoint_vortexXAxisPosRelaxationDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisPosRelaxationLoss.name,            {propJoint_vortexXAxisPosRelaxationLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisPosFrictionCoeff.name,             {propJoint_vortexXAxisPosFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisPosFrictionMaxForce.name,          {propJoint_vortexXAxisPosFrictionMaxForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisPosFrictionLoss.name,              {propJoint_vortexXAxisPosFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisPosRelaxationStiffness.name,       {propJoint_vortexYAxisPosRelaxationStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisPosRelaxationDamping.name,         {propJoint_vortexYAxisPosRelaxationDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisPosRelaxationLoss.name,            {propJoint_vortexYAxisPosRelaxationLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisPosFrictionCoeff.name,             {propJoint_vortexYAxisPosFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisPosFrictionMaxForce.name,          {propJoint_vortexYAxisPosFrictionMaxForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisPosFrictionLoss.name,              {propJoint_vortexYAxisPosFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisPosRelaxationStiffness.name,       {propJoint_vortexZAxisPosRelaxationStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisPosRelaxationDamping.name,         {propJoint_vortexZAxisPosRelaxationDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisPosRelaxationLoss.name,            {propJoint_vortexZAxisPosRelaxationLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisPosFrictionCoeff.name,             {propJoint_vortexZAxisPosFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisPosFrictionMaxForce.name,          {propJoint_vortexZAxisPosFrictionMaxForce.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisPosFrictionLoss.name,              {propJoint_vortexZAxisPosFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisOrientRelaxStiffness.name,         {propJoint_vortexXAxisOrientRelaxStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisOrientRelaxDamping.name,           {propJoint_vortexXAxisOrientRelaxDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisOrientRelaxLoss.name,              {propJoint_vortexXAxisOrientRelaxLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisOrientFrictionCoeff.name,          {propJoint_vortexXAxisOrientFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisOrientFrictionMaxTorque.name,      {propJoint_vortexXAxisOrientFrictionMaxTorque.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexXAxisOrientFrictionLoss.name,           {propJoint_vortexXAxisOrientFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisOrientRelaxStiffness.name,         {propJoint_vortexYAxisOrientRelaxStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisOrientRelaxDamping.name,           {propJoint_vortexYAxisOrientRelaxDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisOrientRelaxLoss.name,              {propJoint_vortexYAxisOrientRelaxLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisOrientFrictionCoeff.name,          {propJoint_vortexYAxisOrientFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisOrientFrictionMaxTorque.name,      {propJoint_vortexYAxisOrientFrictionMaxTorque.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexYAxisOrientFrictionLoss.name,           {propJoint_vortexYAxisOrientFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisOrientRelaxStiffness.name,         {propJoint_vortexZAxisOrientRelaxStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisOrientRelaxDamping.name,           {propJoint_vortexZAxisOrientRelaxDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisOrientRelaxLoss.name,              {propJoint_vortexZAxisOrientRelaxLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisOrientFrictionCoeff.name,          {propJoint_vortexZAxisOrientFrictionCoeff.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisOrientFrictionMaxTorque.name,      {propJoint_vortexZAxisOrientFrictionMaxTorque.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexZAxisOrientFrictionLoss.name,           {propJoint_vortexZAxisOrientFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_vortexPosPid.name,                            {propJoint_vortexPosPid.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_newtonPosPid.name,                            {propJoint_newtonPosPid.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoArmature.name,                          {propJoint_mujocoArmature.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoMargin.name,                            {propJoint_mujocoMargin.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoFrictionLoss.name,                      {propJoint_mujocoFrictionLoss.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoSpringStiffness.name,                   {propJoint_mujocoSpringStiffness.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoSpringDamping.name,                     {propJoint_mujocoSpringDamping.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoSpringRef.name,                         {propJoint_mujocoSpringRef.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoPosPid.name,                            {propJoint_mujocoPosPid.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoLimitsSolRef.name,                      {propJoint_mujocoLimitsSolRef.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoLimitsSolImp.name,                      {propJoint_mujocoLimitsSolImp.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoFrictionSolRef.name,                    {propJoint_mujocoFrictionSolRef.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoFrictionSolImp.name,                    {propJoint_mujocoFrictionSolImp.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoSpringDamper.name,                      {propJoint_mujocoSpringDamper.name, sim_sceneobject_joint}},
    {propJoint_DEPRECATED_mujocoDependencyPolyCoef.name,                {propJoint_mujocoDependencyPolyCoef.name, sim_sceneobject_joint}},

    {propDummy_DEPRECATED_linkedDummyHandle.name,                       {std::string(propDummy_linkedDummy.name) + "__noReplace__", sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_size.name,                                    {propDummy_size.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_dummyType.name,                               {propDummy_dummyType.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_engineProperties.name,                        {propDummy_engineProperties.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoLimitsEnabled.name,                     {propDummy_mujocoLimitsEnabled.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoLimitsRange.name,                       {propDummy_mujocoLimitsRange.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoLimitsSolref.name,                      {propDummy_mujocoLimitsSolref.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoLimitsSolimp.name,                      {propDummy_mujocoLimitsSolimp.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoMargin.name,                            {propDummy_mujocoMargin.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoSpringStiffness.name,                   {propDummy_mujocoSpringStiffness.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoSpringDamping.name,                     {propDummy_mujocoSpringDamping.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoSpringLength.name,                      {propDummy_mujocoSpringLength.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoJointProxyHandle.name,                  {propDummy_mujocoJointProxyHandle.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoOverlapConstrSolref.name,               {propDummy_mujocoOverlapConstrSolref.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoOverlapConstrSolimp.name,               {propDummy_mujocoOverlapConstrSolimp.name, sim_sceneobject_dummy}},
    {propDummy_DEPRECATED_mujocoOverlapConstrTorqueScale.name,          {propDummy_mujocoOverlapConstrTorqueScale.name, sim_sceneobject_dummy}},

    {propMeshWrapper_DEPRECATED_inertia.name,                                          {std::string(propMeshWrapper_inertiaMatrix.name) + "__noReplace__", sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_engineProperties.name,                                    {propMaterial_engineProperties.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletRestitution.name,                                   {propMaterial_bulletRestitution.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletFriction0.name,                                     {propMaterial_bulletFriction0.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletFriction.name,                                      {propMaterial_bulletFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletLinearDamping.name,                                 {propMaterial_bulletLinearDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletAngularDamping.name,                                {propMaterial_bulletAngularDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletNonDefaultCollisionMarginFactor.name,               {propMaterial_bulletNonDefaultCollisionMarginFactor.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletNonDefaultCollisionMarginFactorConvex.name,         {propMaterial_bulletNonDefaultCollisionMarginFactorConvex.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletSticky.name,                                        {propMaterial_bulletSticky.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletNonDefaultCollisionMargin.name,                     {propMaterial_bulletNonDefaultCollisionMargin.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletNonDefaultCollisionMarginConvex.name,               {propMaterial_bulletNonDefaultCollisionMarginConvex.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_bulletAutoShrinkConvex.name,                              {propMaterial_bulletAutoShrinkConvex.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_odeFriction.name,                                         {propMaterial_odeFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_odeSoftErp.name,                                          {propMaterial_odeSoftErp.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_odeSoftCfm.name,                                          {propMaterial_odeSoftCfm.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_odeLinearDamping.name,                                    {propMaterial_odeLinearDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_odeAngularDamping.name,                                   {propMaterial_odeAngularDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_odeMaxContacts.name,                                      {propMaterial_odeMaxContacts.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryLinearAxisFriction.name,                     {propMaterial_vortexPrimaryLinearAxisFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryLinearAxisFriction.name,                   {propMaterial_vortexSecondaryLinearAxisFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryAngularAxisFriction.name,                    {propMaterial_vortexPrimaryAngularAxisFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryAngularAxisFriction.name,                  {propMaterial_vortexSecondaryAngularAxisFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexNormalAngularAxisFriction.name,                     {propMaterial_vortexNormalAngularAxisFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryLinearAxisStaticFrictionScale.name,          {propMaterial_vortexPrimaryLinearAxisStaticFrictionScale.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryLinearAxisStaticFrictionScale.name,        {propMaterial_vortexSecondaryLinearAxisStaticFrictionScale.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryAngularAxisStaticFrictionScale.name,         {propMaterial_vortexPrimaryAngularAxisStaticFrictionScale.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryAngularAxisStaticFrictionScale.name,       {propMaterial_vortexSecondaryAngularAxisStaticFrictionScale.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexNormalAngularAxisStaticFrictionScale.name,          {propMaterial_vortexNormalAngularAxisStaticFrictionScale.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexCompliance.name,                                    {propMaterial_vortexCompliance.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexDamping.name,                                       {propMaterial_vortexDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexRestitution.name,                                   {propMaterial_vortexRestitution.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexRestitutionThreshold.name,                          {propMaterial_vortexRestitutionThreshold.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAdhesiveForce.name,                                 {propMaterial_vortexAdhesiveForce.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexLinearVelocityDamping.name,                         {propMaterial_vortexLinearVelocityDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAngularVelocityDamping.name,                        {propMaterial_vortexAngularVelocityDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryLinearAxisSlide.name,                        {propMaterial_vortexPrimaryLinearAxisSlide.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryLinearAxisSlide.name,                      {propMaterial_vortexSecondaryLinearAxisSlide.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryAngularAxisSlide.name,                       {propMaterial_vortexPrimaryAngularAxisSlide.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryAngularAxisSlide.name,                     {propMaterial_vortexSecondaryAngularAxisSlide.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexNormalAngularAxisSlide.name,                        {propMaterial_vortexNormalAngularAxisSlide.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryLinearAxisSlip.name,                         {propMaterial_vortexPrimaryLinearAxisSlip.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryLinearAxisSlip.name,                       {propMaterial_vortexSecondaryLinearAxisSlip.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryAngularAxisSlip.name,                        {propMaterial_vortexPrimaryAngularAxisSlip.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryAngularAxisSlip.name,                      {propMaterial_vortexSecondaryAngularAxisSlip.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexNormalAngularAxisSlip.name,                         {propMaterial_vortexNormalAngularAxisSlip.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoSleepLinearSpeedThreshold.name,                 {propMaterial_vortexAutoSleepLinearSpeedThreshold.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoSleepLinearAccelerationThreshold.name,          {propMaterial_vortexAutoSleepLinearAccelerationThreshold.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoSleepAngularSpeedThreshold.name,                {propMaterial_vortexAutoSleepAngularSpeedThreshold.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoSleepAngularAccelerationThreshold.name,         {propMaterial_vortexAutoSleepAngularAccelerationThreshold.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSkinThickness.name,                                 {propMaterial_vortexSkinThickness.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoAngularDampingTensionRatio.name,                {propMaterial_vortexAutoAngularDampingTensionRatio.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryAxisVector.name,                             {propMaterial_vortexPrimaryAxisVector.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryLinearAxisFrictionModel.name,                {propMaterial_vortexPrimaryLinearAxisFrictionModel.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryLinearAxisFrictionModel.name,              {propMaterial_vortexSecondaryLinearAxisFrictionModel.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimaryAngularAxisFrictionModel.name,               {propMaterial_vortexPrimaryAngularAxisFrictionModel.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryAngularAxisFrictionModel.name,             {propMaterial_vortexSecondaryAngularAxisFrictionModel.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexNormalAngularAxisFrictionModel.name,                {propMaterial_vortexNormalAngularAxisFrictionModel.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoSleepStepLiveThreshold.name,                    {propMaterial_vortexAutoSleepStepLiveThreshold.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexMaterialUniqueId.name,                              {propMaterial_vortexMaterialUniqueId.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexPrimitiveShapesAsConvex.name,                       {propMaterial_vortexPrimitiveShapesAsConvex.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexConvexShapesAsRandom.name,                          {propMaterial_vortexConvexShapesAsRandom.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexRandomShapesAsTerrain.name,                         {propMaterial_vortexRandomShapesAsTerrain.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexFastMoving.name,                                    {propMaterial_vortexFastMoving.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoSlip.name,                                      {propMaterial_vortexAutoSlip.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name,    {propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name,  {propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name,     {propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_vortexAutoAngularDamping.name,                            {propMaterial_vortexAutoAngularDamping.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_newtonStaticFriction.name,                                {propMaterial_newtonStaticFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_newtonKineticFriction.name,                               {propMaterial_newtonKineticFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_newtonRestitution.name,                                   {propMaterial_newtonRestitution.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_newtonLinearDrag.name,                                    {propMaterial_newtonLinearDrag.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_newtonAngularDrag.name,                                   {propMaterial_newtonAngularDrag.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_newtonFastMoving.name,                                    {propMaterial_newtonFastMoving.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoFriction.name,                                      {propMaterial_mujocoFriction.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoSolref.name,                                        {propMaterial_mujocoSolref.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoSolimp.name,                                        {propMaterial_mujocoSolimp.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoSolmix.name,                                        {propMaterial_mujocoSolmix.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoMargin.name,                                        {propMaterial_mujocoMargin.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoGap.name,                                           {propMaterial_mujocoGap.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoCondim.name,                                        {propMaterial_mujocoCondim.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoPriority.name,                                      {propMaterial_mujocoPriority.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoAdhesion.name,                                      {propMaterial_mujocoAdhesion.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoAdhesionGain.name,                                  {propMaterial_mujocoAdhesionGain.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoAdhesionForcelimited.name,                          {propMaterial_mujocoAdhesionForcelimited.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoAdhesionCtrlrange.name,                             {propMaterial_mujocoAdhesionCtrlrange.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoAdhesionForcerange.name,                            {propMaterial_mujocoAdhesionForcerange.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoAdhesionCtrl.name,                                  {propMaterial_mujocoAdhesionCtrl.name, sim_sceneobject_shape}},
    {propMaterial_DEPRECATED_mujocoGravcomp.name,                                      {propMaterial_mujocoGravcomp.name, sim_sceneobject_shape}},

    {propConvexVolume_DEPRECATED_offset.name,                           {propConvexVolume_offset.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_range.name,                            {propConvexVolume_range.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_xSize.name,                            {propConvexVolume_xSize.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_ySize.name,                            {propConvexVolume_ySize.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_radius.name,                           {propConvexVolume_radius.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_angle.name,                            {propConvexVolume_angle.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_faces.name,                            {propConvexVolume_faces.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_subdivisions.name,                     {propConvexVolume_subdivisions.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_edges.name,                            {propConvexVolume_edges.name, sim_sceneobject_proximitysensor}},
    {propConvexVolume_DEPRECATED_closeEdges.name,                       {propConvexVolume_closeEdges.name, sim_sceneobject_proximitysensor}},
    {propProximitySensor_DEPRECATED_size.name,                          {propProximitySensor_size.name, sim_sceneobject_proximitysensor}},
    {propProximitySensor_DEPRECATED_sensorType.name,                    {propProximitySensor_sensorType.name, sim_sceneobject_proximitysensor}},
    {propProximitySensor_DEPRECATED_detectedObjectHandle.name,          {std::string(propProximitySensor_detectedObject.name) + "__noReplace__", sim_sceneobject_proximitysensor}},

};

const std::map<std::string, SDeprecatedProperty> propDeprecationMapping = []() {
    std::map<std::string, SDeprecatedProperty> result;
    for (const auto& [key, item] : tmpMap)
    {
        auto it = result.find(key);
        if (it != result.end())
        {
            auto& types = it->second.types;
            if (std::find(types.begin(), types.end(), item.type) == types.end())
                types.push_back(item.type);
        }
        else
            result[key] = {item.repl, {item.type}};
    }
    return result;
}();
