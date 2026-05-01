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

const std::map<std::string, SDeprecatedProperty> propDeprecationMapping = {
    {propForceSensor_DEPRECATED_sensorAverageForce.name,                {propForceSensor_filteredSensorForce.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_sensorAverageTorque.name,               {propForceSensor_filteredSensorTorque.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_sensorForce.name,                       {propForceSensor_sensorForce.name, sim_sceneobject_forcesensor}},
    {propForceSensor_DEPRECATED_sensorTorque.name,                      {propForceSensor_sensorTorque.name, sim_sceneobject_forcesensor}},

    {propSceneObjectCont_DEPRECATED_objectHandles.name,                 {std::string(propSceneObjectCont_objects.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_orphanHandles.name,                 {std::string(propSceneObjectCont_orphans.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_selectionHandles.name,              {std::string(propSceneObjectCont_selection.name) + "__noReplace__", sim_objecttype_scene}},

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

    {propJoint_DEPRECATED_dependencyMaster.name,                        {std::string(propJoint_dependencyMaster.name) + "__noReplace__", sim_sceneobject_joint}},

    {propDummy_DEPRECATED_linkedDummyHandle.name,                       {std::string(propDummy_linkedDummy.name) + "__noReplace__", sim_sceneobject_dummy}},

    {propMeshWrapper_DEPRECATED_inertia.name,                           {std::string(propMeshWrapper_inertiaMatrix.name) + "__noReplace__", sim_sceneobject_shape}},

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

    {propSceneObjectCont_DEPRECATED_objectHandles.name,                 {std::string(propSceneObjectCont_objects.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_orphanHandles.name,                 {std::string(propSceneObjectCont_orphans.name) + "__noReplace__", sim_objecttype_scene}},
    {propSceneObjectCont_DEPRECATED_selectionHandles.name,              {std::string(propSceneObjectCont_selection.name) + "__noReplace__", sim_objecttype_scene}},

    {propScene_DEPRECATED_sceneIsLocked.name,                           {propScene_sceneIsLocked.name, sim_objecttype_scene}},
    {propScene_DEPRECATED_sceneUid.name,                                {propScene_sceneUid.name, sim_objecttype_scene}},
    {propScene_DEPRECATED_sceneUidString.name,                          {propScene_sceneUidString.name, sim_objecttype_scene}},
    {propScene_DEPRECATED_scenePath.name,                               {propScene_scenePath.name, sim_objecttype_scene}},


 //       {xxxxxxxxxxxxxxxxxxxxxxxxxxxx.name,                       {xxxxxxxxxxxxxxxxxxxxxxxxxxxx.name, sim_objecttype_sceneobject}},
};
