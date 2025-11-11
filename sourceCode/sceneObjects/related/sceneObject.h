#pragma once

#include <colorObject.h>
#include <simMath/3Vector.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>
#include <map>
#include <userParameters.h>
#include <customData.h>
#include <cbor.h>
#include <simLib/simConst.h>

#define SCENEOBJECT_META_METHODS R"("getAlias": "sim-2.getObjectAlias",
        "getPose": "sim-2.getObjectPose",
        "getPosition": "sim-2.getObjectPosition",
        "getQuaternion": "sim-2.getObjectQuaternion",
        "getVelocity": "sim-2.getObjectVelocity",
        "scaleObject": "sim-2.scaleObject",
        "setParent": "sim-2.setObjectParent",
        "setPose": "sim-2.setObjectPose",
        "setPosition": "sim-2.setObjectPosition",
        "setQuaternion": "sim-2.setObjectQuaternion",
        "visitTree": "sim-2.visitTree",
        "dynamicReset": "sim-2.resetDynamicObject",
        )" APP_META_METHODS

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                                                       \
    FUNCX(propObject_modelInvisible, "modelInvisible", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Inherited model invisibility", "")                                             \
    FUNCX(propObject_modelBase, "modelBase", sim_propertytype_bool, 0, "Model base", "Model base flag, indicates the scene object is the base of a model")                                                                      \
    FUNCX(propObject_layer, "layer", sim_propertytype_int, 0, "Visibility layer", "")                                                                                                                                           \
    FUNCX(propObject_childOrder, "childOrder", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Child order", "")                                                                       \
    FUNCX(propObject_parentUid, "parentUid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Parent UID", "Parent scene object unique identifier")                                    \
    FUNCX(propObject_objectUid, "objectUid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object UID", "Scene object unique identifier")                                           \
    FUNCX(propObject_parentHandle, "parentHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, "", "")                                              \
    FUNCX(propObject_parent, "parent", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, "Parent", "Parent scene object handle")                                              \
    FUNCX(propObject_selected, "selected", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, "Selected", "Selection state")                                                                                             \
    FUNCX(propObject_hierarchyColor, "hierarchyColor", sim_propertytype_int, 0, "Hierarchy color", "Hierarchy color index")                                                                                                     \
    FUNCX(propObject_collectionSelfCollInd, "collectionSelfCollisionIndicator", sim_propertytype_int, 0, "Collection self collision indicator", "")                                                                             \
    FUNCX(propObject_collidable, "collidable", sim_propertytype_bool, 0, "Collidable", "")                                                                                                                                      \
    FUNCX(propObject_measurable, "measurable", sim_propertytype_bool, 0, "Measurable", "")                                                                                                                                      \
    FUNCX(propObject_detectable, "detectable", sim_propertytype_bool, 0, "Detectable", "")                                                                                                                                      \
    FUNCX(propObject_modelAcknowledgment, "modelAcknowledgment", sim_propertytype_string, 0, "Acknowledgment", "Model acknowledgment")                                                                                          \
    FUNCX(propObject_dna, "dna", sim_propertytype_buffer, sim_propertyinfo_notwritable, "DNA", "Scene object DNA")                                                                                                              \
    FUNCX(propObject_persistentUid, "persistentUid", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Persistent UID", "Scene object persistent unique identifier")                  \
    FUNCX(propObject_calcLinearVelocity, "calcLinearVelocity", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Linear velocity", "Calculated scene object linear velocity vector") \
    FUNCX(propObject_calcRotationAxis, "calcRotationAxis", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Rotation axis", "Calculated scene object rotation axis")                \
    FUNCX(propObject_calcRotationVelocity, "calcRotationVelocity", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Rotation velocity", "Calculated scene object rotation velocity")  \
    FUNCX(propObject_dynamicIcon, "dynamicIcon", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Dynamic icon", "")                                                                    \
    FUNCX(propObject_dynamicFlag, "dynamicFlag", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Dynamic flag", "")                                                                    \
    FUNCX(propObject_objectProperty, "objectPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude, "Object flags", "Scene object flags, redundant")                                               \
    FUNCX(propObject_ignoreViewFitting, "ignoreViewFitting", sim_propertytype_bool, 0, "Ignore view fitting", "View fitting is ignored for this scene object")                                                                  \
    FUNCX(propObject_collapsed, "collapsed", sim_propertytype_bool, 0, "Collapsed hierarchy", "")                                                                                                                               \
    FUNCX(propObject_selectable, "selectable", sim_propertytype_bool, 0, "Selectable", "Scene object is selectable")                                                                                                            \
    FUNCX(propObject_selectModel, "selectModel", sim_propertytype_bool, 0, "Select model instead", "Selecting the scene object instead selects the parent model")                                                               \
    FUNCX(propObject_hideFromModelBB, "hideFromModelBB", sim_propertytype_bool, 0, "Hide from model bounding box", "Scene object is not part of a parent model's bounding box")                                                 \
    FUNCX(propObject_selectInvisible, "selectInvisible", sim_propertytype_bool, 0, "Select invisible", "Scene object is invisible during a selection operation")                                                                \
    FUNCX(propObject_depthInvisible, "depthInvisible", sim_propertytype_bool, 0, "Depth pass invisible", "Scene object is invisible for the depth buffer during a click operation")                                             \
    FUNCX(propObject_cannotDelete, "cannotDelete", sim_propertytype_bool, 0, "Cannot delete", "Scene object cannot be deleted while simulation is not running")                                                                 \
    FUNCX(propObject_cannotDeleteSim, "cannotDeleteInSim", sim_propertytype_bool, 0, "Cannot delete during simulation", "Scene object cannot be deleted while simulation is running")                                           \
    FUNCX(propObject_modelPropertyDEPRECATED, "modelPropertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude | sim_propertyinfo_deprecated, "Model flags", "Model flags, redundant")                                                         \
    FUNCX(propObject_modelNotCollidableDEPRECATED, "modelNotCollidable", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model not collidable", "Model is not collidable")                                                                                     \
    FUNCX(propObject_modelNotMeasurableDEPRECATED, "modelNotMeasurable", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model not measurable", "Model is not measurable")                                                                                     \
    FUNCX(propObject_modelNotDetectableDEPRECATED, "modelNotDetectable", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model not detectable", "Model is not detectable")                                                                                     \
    FUNCX(propObject_modelNotDynamicDEPRECATED, "modelNotDynamic", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model not dynamic", "Model is not dynamic, i.e. model is static")                                                                           \
    FUNCX(propObject_modelNotRespondableDEPRECATED, "modelNotRespondable", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model not respondable", "Model is not respondable")                                                                                 \
    FUNCX(propObject_modelNotVisibleDEPRECATED, "modelNotVisible", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model not visible", "Model is not visible")                                                                                                 \
    FUNCX(propObject_modelScriptsNotActiveDEPRECATED, "modelScriptsNotActive", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model scripts inactive", "Model scripts are not active")                                                                        \
    FUNCX(propObject_modelNotInParentBBDEPRECATED, "modelNotInParentBB", sim_propertytype_bool, sim_propertyinfo_deprecated, "Model invisible to other model's bounding boxes", "Model is invisible to other model's bounding boxes")                               \
    FUNCX(propObject_modelProperty, "model.propertyFlags", /*redund.*/ sim_propertytype_int, sim_propertyinfo_modelhashexclude, "Model flags", "Model flags, redundant")                                                         \
    FUNCX(propObject_modelNotCollidable, "model.notCollidable", sim_propertytype_bool, 0, "Model not collidable", "Model is not collidable")                                                                                     \
    FUNCX(propObject_modelNotMeasurable, "model.notMeasurable", sim_propertytype_bool, 0, "Model not measurable", "Model is not measurable")                                                                                     \
    FUNCX(propObject_modelNotDetectable, "model.notDetectable", sim_propertytype_bool, 0, "Model not detectable", "Model is not detectable")                                                                                     \
    FUNCX(propObject_modelNotDynamic, "model.notDynamic", sim_propertytype_bool, 0, "Model not dynamic", "Model is not dynamic, i.e. model is static")                                                                           \
    FUNCX(propObject_modelNotRespondable, "model.notRespondable", sim_propertytype_bool, 0, "Model not respondable", "Model is not respondable")                                                                                 \
    FUNCX(propObject_modelNotVisible, "model.notVisible", sim_propertytype_bool, 0, "Model not visible", "Model is not visible")                                                                                                 \
    FUNCX(propObject_modelScriptsNotActive, "model.scriptsNotActive", sim_propertytype_bool, 0, "Model scripts inactive", "Model scripts are not active")                                                                        \
    FUNCX(propObject_modelNotInParentBB, "model.notInParentBB", sim_propertytype_bool, 0, "Model invisible to other model's bounding boxes", "Model is invisible to other model's bounding boxes")                               \
    FUNCX(propObject_modelBBSize, "model.bbSize", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Model bounding boxe size", "")                               \
    FUNCX(propObject_modelBBPos, "model.bbPos", sim_propertytype_vector3, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Model bounding boxe position", "")                               \
    FUNCX(propObject_pose, "pose", sim_propertytype_pose, 0, "Pose", "Scene object local pose")                                                                                                                                 \
    FUNCX(propObject_position, "position", sim_propertytype_vector3, sim_propertyinfo_modelhashexclude, "Position", "Scene object local position")                                                                                                                                 \
    FUNCX(propObject_quaternion, "quaternion", sim_propertytype_quaternion, sim_propertyinfo_modelhashexclude, "Quaternion", "Scene object local quaternion")                                                                                                                                 \
    FUNCX(propObject_eulerAngles, "eulerAngles", sim_propertytype_vector3, sim_propertyinfo_modelhashexclude, "Euler angles", "Scene object local Euler angles")                                                                                                                                 \
    FUNCX(propObject_absPose, "absPose", sim_propertytype_pose, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, "Pose", "Scene object absolute pose")                                                                                                                                 \
    FUNCX(propObject_absPosition, "absPosition", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, "Position", "Scene object absolute position")                                                                                                                                 \
    FUNCX(propObject_absQuaternion, "absQuaternion", sim_propertytype_quaternion, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, "Quaternion", "Scene object absolute quaternion")                                                                                                                                 \
    FUNCX(propObject_absEulerAngles, "absEulerAngles", sim_propertytype_vector3, sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, "Euler angles", "Scene object absolute Euler angles")                                                                                                                                 \
    FUNCX(propObject_alias, "alias", sim_propertytype_string, 0, "Alias", "Scene object alias")                                                                                                                                 \
    FUNCX(propObject_deprecatedName, "deprecatedName", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, "Deprecated name", "")                                                                                                                                 \
    FUNCX(propObject_bbPose, "bbPose", sim_propertytype_pose, sim_propertyinfo_notwritable, "Bounding box pose", "Bounding box local pose")                                                                                     \
    FUNCX(propObject_bbHsize, "bbHSize", sim_propertytype_vector3, sim_propertyinfo_notwritable, "Bounding box half sizes", "")                                                                                                 \
    FUNCX(propObject_movementOptionsDEPRECATED, "movementOptionsFlags", sim_propertytype_int, sim_propertyinfo_deprecated, "Movement flags", "Scene object movement flags")                                                                                         \
    FUNCX(propObject_movementStepSizeDEPRECATED, "movementStepSize", sim_propertytype_floatarray, sim_propertyinfo_deprecated, "Movement step sizes", "Linear and angular step sizes")                                                                              \
    FUNCX(propObject_movementRelativityDEPRECATED, "movementRelativity", sim_propertytype_intarray, sim_propertyinfo_deprecated, "Movement relativity", "Movement relativity, linear and angular")                                                                  \
    FUNCX(propObject_movementPreferredAxesDEPRECATED, "movementPreferredAxesFlags", sim_propertytype_int, sim_propertyinfo_deprecated, "Preferred axes", "Preferred axes flags (redundant)")                                                                        \
    FUNCX(propObject_movementOptions, "mov.optionsFlags", sim_propertytype_int, 0, "Movement flags", "Scene object movement flags")                                                                                         \
    FUNCX(propObject_movementStepSize, "mov.stepSize", sim_propertytype_floatarray, 0, "Movement step sizes", "Linear and angular step sizes")                                                                              \
    FUNCX(propObject_movementRelativity, "mov.relativity", sim_propertytype_intarray, 0, "Movement relativity", "Movement relativity, linear and angular")                                                                  \
    FUNCX(propObject_movementPreferredAxes, "mov.preferredAxesFlags", sim_propertytype_int, 0, "Preferred axes", "Preferred axes flags (redundant)")                                                                        \
    FUNCX(propObject_movTranslNoSimDEPRECATED, "movTranslNoSim", sim_propertytype_bool, sim_propertyinfo_deprecated, "Translation enabled", "Translation enabled when simulation is not running")                                                                   \
    FUNCX(propObject_movTranslInSimDEPRECATED, "movTranslInSim", sim_propertytype_bool, sim_propertyinfo_deprecated, "Translation enabled during simulation", "Translation enabled when simulation is running")                                                     \
    FUNCX(propObject_movRotNoSimDEPRECATED, "movRotNoSim", sim_propertytype_bool, sim_propertyinfo_deprecated, "Rotation enabled", "Rotation enabled when simulation is not running")                                                                               \
    FUNCX(propObject_movRotInSimDEPRECATED, "movRotInSim", sim_propertytype_bool, sim_propertyinfo_deprecated, "Rotation enabled during simulation", "Rotation enabled when simulation is running")                                                                 \
    FUNCX(propObject_movAltTranslDEPRECATED, "movAltTransl", sim_propertytype_bool, sim_propertyinfo_deprecated, "Alternate translation axes enabled", "")                                                                                                          \
    FUNCX(propObject_movAltRotDEPRECATED, "movAltRot", sim_propertytype_bool, sim_propertyinfo_deprecated, "Alternate rotation axes enabled", "")                                                                                                                   \
    FUNCX(propObject_movPrefTranslXDEPRECATED, "movPrefTranslX", sim_propertytype_bool, sim_propertyinfo_deprecated, "Preferred X-translation", "")                                                                                                                 \
    FUNCX(propObject_movPrefTranslYDEPRECATED, "movPrefTranslY", sim_propertytype_bool, sim_propertyinfo_deprecated, "Preferred Y-translation", "")                                                                                                                 \
    FUNCX(propObject_movPrefTranslZDEPRECATED, "movPrefTranslZ", sim_propertytype_bool, sim_propertyinfo_deprecated, "Preferred Z-translation", "")                                                                                                                 \
    FUNCX(propObject_movPrefRotXDEPRECATED, "movPrefRotX", sim_propertytype_bool, sim_propertyinfo_deprecated, "Preferred X-axis rotation", "")                                                                                                                     \
    FUNCX(propObject_movPrefRotYDEPRECATED, "movPrefRotY", sim_propertytype_bool, sim_propertyinfo_deprecated, "Preferred Y-axis rotation", "")                                                                                                                     \
    FUNCX(propObject_movPrefRotZDEPRECATED, "movPrefRotZ", sim_propertytype_bool, sim_propertyinfo_deprecated, "Preferred Z-axis rotation", "")                                                                                                                     \
    FUNCX(propObject_movTranslNoSim, "mov.translNoSim", sim_propertytype_bool, 0, "Translation enabled", "Translation enabled when simulation is not running")                                                                   \
    FUNCX(propObject_movTranslInSim, "mov.translInSim", sim_propertytype_bool, 0, "Translation enabled during simulation", "Translation enabled when simulation is running")                                                     \
    FUNCX(propObject_movRotNoSim, "mov.rotNoSim", sim_propertytype_bool, 0, "Rotation enabled", "Rotation enabled when simulation is not running")                                                                               \
    FUNCX(propObject_movRotInSim, "mov.rotInSim", sim_propertytype_bool, 0, "Rotation enabled during simulation", "Rotation enabled when simulation is running")                                                                 \
    FUNCX(propObject_movAltTransl, "mov.altTransl", sim_propertytype_bool, 0, "Alternate translation axes enabled", "")                                                                                                          \
    FUNCX(propObject_movAltRot, "mov.altRot", sim_propertytype_bool, 0, "Alternate rotation axes enabled", "")                                                                                                                   \
    FUNCX(propObject_movPrefTranslX, "mov.prefTranslX", sim_propertytype_bool, 0, "Preferred X-translation", "")                                                                                                                 \
    FUNCX(propObject_movPrefTranslY, "mov.prefTranslY", sim_propertytype_bool, 0, "Preferred Y-translation", "")                                                                                                                 \
    FUNCX(propObject_movPrefTranslZ, "mov.prefTranslZ", sim_propertytype_bool, 0, "Preferred Z-translation", "")                                                                                                                 \
    FUNCX(propObject_movPrefRotX, "mov.prefRotX", sim_propertytype_bool, 0, "Preferred X-axis rotation", "")                                                                                                                     \
    FUNCX(propObject_movPrefRotY, "mov.prefRotY", sim_propertytype_bool, 0, "Preferred Y-axis rotation", "")                                                                                                                     \
    FUNCX(propObject_movPrefRotZ, "mov.prefRotZ", sim_propertytype_bool, 0, "Preferred Z-axis rotation", "")                                                                                                                     \
    FUNCX(propObject_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object type", "Scene object type")                                                    \
    FUNCX(propObject_visible, "visible", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Visible", "Whether the scene object is currently visible")                                    \
    FUNCX(propObject_children, "children", sim_propertytype_handlearray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Children handles", "")                                                    \
    FUNCX(propObject_modelHash, "modelHash", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Model hash", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_sceneObject = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

struct SCustomRefs
{
    int generalObjectType; // e.g. sim_objecttype_sceneobject, sim_appobj_collision_type
    int generalObjectHandle;
};

inline bool operator==(const SCustomRefs& lhs, const SCustomRefs& rhs)
{
    return lhs.generalObjectType == rhs.generalObjectType && lhs.generalObjectHandle == rhs.generalObjectHandle;
}

struct SCustomOriginalRefs
{
    int generalObjectType; // e.g. sim_objecttype_sceneobject, sim_appobj_collision_type
    int generalObjectHandle;
    std::string uniquePersistentIdString;
};

inline bool operator==(const SCustomOriginalRefs& lhs, const SCustomOriginalRefs& rhs)
{
    return lhs.generalObjectType == rhs.generalObjectType && lhs.generalObjectHandle == rhs.generalObjectHandle && lhs.uniquePersistentIdString == rhs.uniquePersistentIdString;
}

struct SScriptInfo
{
    int scriptHandle;
    int depth;
};

class CShape;
class CCustomData_old;
class CViewableBase;
class CScriptObject;
class CInterfaceStack;

class CSceneObject
{
  public:
    CSceneObject();
    virtual ~CSceneObject();

    virtual void buildOrUpdate_oldIk();
    virtual void connect_oldIk();
    virtual void remove_oldIk();

    virtual void addSpecializedObjectEventData(CCbor* ev);
    virtual CSceneObject* copyYourself();
    virtual void removeSceneDependencies();
    virtual void scaleObject(double scalingFactor);
    virtual bool scaleObjectNonIsometrically(double x, double y, double z);
    virtual void serialize(CSer& ar);

    virtual void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer);
    virtual void announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript,
                                            bool copyBuffer);

    virtual void performObjectLoadingMapping(const std::map<int, int>* map, int opType);
    virtual void performScriptLoadingMapping(const std::map<int, int>* map, int opType);
    virtual void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType);

    // Old:
    // -----------
    virtual void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer);
    virtual void announceCollectionWillBeErased(int collectionID, bool copyBuffer);
    virtual void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    virtual void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    virtual void performCollectionLoadingMapping(const std::map<int, int>* map, int opType);
    virtual void performCollisionLoadingMapping(const std::map<int, int>* map, int opType);
    virtual void performDistanceLoadingMapping(const std::map<int, int>* map, int opType);
    virtual void performIkLoadingMapping(const std::map<int, int>* map, int opType);
    virtual void performGcsLoadingMapping(const std::map<int, int>* map);
    virtual void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map);
    // -----------

    virtual void simulationAboutToStart();
    void simulationEnded_restoreHierarchy();
    virtual void simulationEnded();
    virtual void initializeInitialValues(bool simulationAlreadyRunning);
    virtual void computeBoundingBox();

    virtual std::string getObjectTypeInfo() const;
    virtual std::string getObjectTypeInfoExtended() const;
    virtual bool isPotentiallyCollidable() const;
    virtual bool isPotentiallyMeasurable() const;
    virtual bool isPotentiallyDetectable() const;
    virtual bool isPotentiallyRenderable() const;

    virtual C7Vector getIntrinsicTransformation(bool includeDynErrorComponent, bool* available = nullptr) const;
    virtual C7Vector getFullLocalTransformation() const;
    virtual bool setParent(CSceneObject* parent);
    virtual void setObjectHandle(int newObjectHandle);
    virtual bool canDestroyNow();
    virtual void setIsInScene(bool s);

    virtual int setBoolProperty(const char* pName, bool pState);
    virtual int getBoolProperty(const char* pName, bool& pState) const;
    virtual int setIntProperty(const char* pName, int pState);
    virtual int getIntProperty(const char* pName, int& pState) const;
    virtual int setLongProperty(const char* pName, long long int pState);
    virtual int getLongProperty(const char* pName, long long int& pState) const;
    virtual int setHandleProperty(const char* pName, long long int pState);
    virtual int getHandleProperty(const char* pName, long long int& pState) const;
    virtual int setFloatProperty(const char* pName, double pState);
    virtual int getFloatProperty(const char* pName, double& pState) const;
    virtual int setStringProperty(const char* pName, const char* pState);
    virtual int getStringProperty(const char* pName, std::string& pState) const;
    virtual int setBufferProperty(const char* pName, const char* buffer, int bufferL);
    virtual int getBufferProperty(const char* pName, std::string& pState) const;
    virtual int setIntArray2Property(const char* pName, const int* pState);
    virtual int getIntArray2Property(const char* pName, int* pState) const;
    virtual int setVector2Property(const char* pName, const double* pState);
    virtual int getVector2Property(const char* pName, double* pState) const;
    virtual int setVector3Property(const char* pName, const C3Vector& pState);
    virtual int getVector3Property(const char* pName, C3Vector& pState) const;
    virtual int setQuaternionProperty(const char* pName, const C4Vector& pState);
    virtual int getQuaternionProperty(const char* pName, C4Vector& pState) const;
    virtual int setPoseProperty(const char* pName, const C7Vector& pState);
    virtual int getPoseProperty(const char* pName, C7Vector& pState) const;
    virtual int setColorProperty(const char* pName, const float* pState);
    virtual int getColorProperty(const char* pName, float* pState) const;
    virtual int setFloatArrayProperty(const char* pName, const double* v, int vL);
    virtual int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    virtual int setIntArrayProperty(const char* pName, const int* v, int vL);
    virtual int getIntArrayProperty(const char* pName, std::vector<int>& pState) const;
    virtual int setHandleArrayProperty(const char* pName, const long long int* v, int vL);
    virtual int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const;
    virtual int removeProperty(const char* pName);
    virtual int getPropertyName(int& index, std::string& pName, std::string& appartenance) const;
    static int getPropertyName_bstatic(int& index, std::string& pName, std::string& appartenance);
    virtual int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    static int getPropertyInfo_bstatic(const char* pName, int& info, std::string& infoTxt);

    void setRestoreToDefaultLights(bool s);
    bool getRestoreToDefaultLights() const;

    int getObjectType() const;
    CSceneObject* getParent() const;
    int getObjectHandle() const;
    long long int getObjectUid() const;
    bool getSelected() const;
    bool getIsInScene() const;
    bool getModelBase() const;
    std::string getExtensionString() const;
    int getVisibilityLayer() const;
    int getChildOrder() const;
    int getHierarchyTreeObjects(std::vector<CSceneObject*>& allObjects);
    std::string getObjectAlias() const;
    std::string getObjectAliasAndOrderIfRequired() const;
    std::string getObjectAliasAndHandle() const;
    std::string getObjectPathAndIndex(size_t modelCnt) const;
    std::string getObjectAlias_fullPath() const;
    std::string getObjectAlias_shortPath() const;
    std::string getObjectAlias_printPath() const;
    std::string getObjectName_old() const;
    std::string getObjectAltName_old() const;

    C7Vector getLocalTransformation() const;
    C7Vector getFullParentCumulativeTransformation() const;
    C7Vector getCumulativeTransformation() const;
    C7Vector getFullCumulativeTransformation() const;

    void setChildOrder(int order);
    void setExtensionString(const char* str);
    void setVisibilityLayer(int l);
    void setObjectAlias_direct(const char* newAlias);
    void setObjectName_direct_old(const char* newName);
    void setObjectAltName_direct_old(const char* newAltName);
    void setLocalTransformation(const C7Vector& tr);
    void setLocalTransformation(const C4Vector& q);
    void setLocalTransformation(const C3Vector& x);

    void recomputeModelInfluencedValues(int overrideFlags = -1);
    void setObjectUniqueId();
    void setSelected(bool s); // doesn't generate a sync msg
    void setParentPtr(CSceneObject* parent);

    int getScriptsInTree(std::vector<SScriptInfo>& scripts, int scriptType, bool legacyEmbeddedScripts, int depth = 0);
    void getScriptsInChain(std::vector<int>& scripts, int scriptType, bool legacyEmbeddedScripts);
    size_t getAttachedScripts(std::vector<CScriptObject*>& scripts, int scriptType, bool legacyEmbeddedScripts);

    void scalePosition(double scalingFactor);
    void getAllObjectsRecursive(std::vector<CSceneObject*>* objectList, bool baseIncluded = true,
                                bool start = true) const;
    void getChain(std::vector<CSceneObject*>& objectList, bool tipIncluded = true, bool start = true) const;

    void setReferencedHandles(size_t cnt, const int* handles, const char* tag);
    size_t getReferencedHandlesCount(const char* tag) const;
    size_t getReferencedHandles(int* handles, const char* tag) const;
    void getReferencedHandlesTags(std::vector<std::string>& tags) const;
    void setReferencedOriginalHandles(int cnt, const int* handles, const char* tag);
    size_t getReferencedOriginalHandlesCount(const char* tag) const;
    size_t getReferencedOriginalHandles(int* handles, const char* tag) const;
    void getReferencedOriginalHandlesTags(std::vector<std::string>& tags) const;
    void checkReferencesToOriginal(const std::map<std::string, int>& allUniquePersistentIdStrings);

    CSceneObject* getFirstParentInSelection(const std::vector<CSceneObject*>* sel) const;
    CSceneObject* getLastParentInSelection(const std::vector<CSceneObject*>* sel) const;
    void setDynamicFlag(int flag);
    int getDynamicFlag() const;

    void setSpecificLight(int h);
    int getSpecificLight() const;
    bool setBeforeDeleteCallbackSent();

    bool getModelBB(const C7Vector& baseCoordInv, C3Vector& minV, C3Vector& maxV, bool first) const;

    int getModelSelectionHandle(bool firstObject = true);

    void setObjectProperty(int p);
    int getObjectProperty() const;
    int getCumulativeObjectProperty();
    void setModelBase(bool m);

    void setObjectMovementPreferredAxes(int p);
    int getObjectMovementPreferredAxes() const;

    void setObjectMovementOptions(int p);
    int getObjectMovementOptions() const;
    void setObjectMovementRelativity(int index, int p);
    int getObjectMovementRelativity(int index) const;
    void setObjectMovementStepSize(int index, double s);
    double getObjectMovementStepSize(int index) const;

    void writeCustomDataBlock(bool tmpData, const char* dataName, const char* data, size_t dataLength);
    std::string readCustomDataBlock(bool tmpData, const char* dataName) const;
    std::string getAllCustomDataBlockTags(bool tmpData, size_t* cnt) const;
    bool getCustomDataEvents(std::map<std::string, bool>& customDataEvents, std::map<std::string, bool>& signalEvents);
    void clearCustomDataEvents();

    void clearObjectCustomData_old();
    int getObjectCustomDataLength_old(int header) const;
    void setObjectCustomData_old(int header, const char* data, int dataLength);
    void getObjectCustomData_old(int header, char* data) const;

    int getParentCount() const;

    void setLocalObjectSpecialProperty(int prop);
    int getLocalObjectSpecialProperty() const;
    bool setModelProperty(int prop);
    int getModelProperty() const;
    int getCumulativeModelProperty() const;
    int getCumulativeObjectSpecialProperty();

    int getTreeDynamicProperty(); // combination of sim_objdynprop_dynamic and sim_objdynprop_respondable

    void temporarilyDisableDynamicTree();

    void setTransparentObjectDistanceOffset(double d);
    double getTransparentObjectDistanceOffset() const;

    void setAuthorizedViewableObjects(int objOrCollHandle);
    int getAuthorizedViewableObjects() const;

    void setMechanismID(int id);
    int getMechanismID() const;

    void setSizeFactor(double f);
    double getSizeFactor() const;
    void setSizeValues(const double s[3]);
    void getSizeValues(double s[3]) const;

    void setScriptExecPriority_raw(int p);
    int getScriptExecPriority() const;

    void setDynamicsResetFlag(bool reset, bool fullHierarchyTree);
    bool getDynamicsResetFlag() const;

    void setDynamicSimulationIconCode(int c);
    int getDynamicSimulationIconCode() const;

    void clearManipulationModeOverlayGridFlag();

    bool isObjectVisible() const;
    bool isObjectInVisibleLayer() const;
    bool isObjectPartOfInvisibleModel() const;

    void setModelAcknowledgement(const char* a);
    std::string getModelAcknowledgement() const;

    bool getShouldObjectBeDisplayed(int viewableHandle, int displayAttrib);

    void setAssemblingLocalTransformation(const C7Vector& tr);
    C7Vector getAssemblingLocalTransformation() const;
    void setAssemblingLocalTransformationIsUsed(bool u);
    bool getAssemblingLocalTransformationIsUsed();
    void setAssemblyMatchValues(bool asChild, const char* str);
    std::string getAssemblyMatchValues(bool asChild) const;
    const std::vector<std::string>* getChildAssemblyMatchValuesPointer() const;
    int getAllChildrenThatMayBecomeAssemblyParent(const std::vector<std::string>* assemblingChildMatchValues,
                                                  std::vector<CSceneObject*>& objects) const;
    bool doesParentAssemblingMatchValuesMatchWithChild(const std::vector<std::string>* assemblingChildMatchValues,
                                                       bool ignoreDefaultNames = false) const;

    void generateDnaString();
    std::string getDnaString() const;
    std::string getUniquePersistentIdString() const;
    void setCopyString(const char* str);
    std::string getCopyString() const;

    size_t getChildCount() const;
    CSceneObject* getChildFromIndex(size_t index) const;
    const std::vector<CSceneObject*>* getChildren() const;
    void addChild(CSceneObject* child);
    bool removeChild(const CSceneObject* child);
    void handleOrderIndexOfChildren();
    int getChildSequence(const CSceneObject* child, int* totalSiblings = nullptr) const;
    bool setChildSequence(CSceneObject* child, int order);

    void setHierarchyColorIndex(int c);
    int getHierarchyColorIndex() const;

    void setCollectionSelfCollisionIndicator(int c);
    int getCollectionSelfCollisionIndicator() const;

    void measureVelocity(double dt); // should be called just after the main script was called!!!
    double getMeasuredAngularVelocity() const;
    C3Vector getMeasuredLinearVelocity() const;
    C3Vector getMeasuredAngularVelocity3() const;
    C3Vector getMeasuredAngularVelocityAxis() const;

    void setForceAlwaysVisible_tmp(bool force);

    void setAbsoluteTransformation(const C7Vector& v);
    void setAbsoluteTransformation(const C4Vector& q);
    void setAbsoluteTransformation(const C3Vector& x);

    int getIkPluginCounterpartHandle() const;

    std::string getDisplayName() const;
    bool hasAncestor(const CSceneObject* potentialAncestor) const;

    void setParentHandle_forSerializationOnly(int pHandle);

    void getFirstModelRelatives(std::vector<CSceneObject*>& firstModelRelatives, bool visibleModelsOnly) const;
    int countFirstModelRelatives(bool visibleModelsOnly) const;
    std::string getObjectTempAlias() const;
    std::string getObjectTempName_old() const;
    std::string getObjectTempAltName_old() const;
    void acquireCommonPropertiesFromObject_simpleXMLLoading(const CSceneObject* obj);
    CUserParameters* getUserScriptParameterObject();
    void setUserScriptParameterObject(CUserParameters* obj);

    void setIgnorePosAndCameraOrthoviewSize_forUndoRedo(bool s);

    void pushObjectCreationEvent();
    void pushObjectRefreshEvent();
    C7Vector getBB(C3Vector* bbHalfSize) const;
    C3Vector getBBHSize() const;

    CCustomData customObjectData;
    CCustomData customObjectData_volatile; // this one is not serialized (but copied)! (object-level signals)

  protected:
    bool _setChildren(std::vector<CSceneObject*>* children);
    void _setMeasuredVelocity(const C3Vector& lin, const C3Vector& ang, const C3Vector& rotAxis, double angle);
    void _setModelInvisible(bool inv);
    void _setBB(const C7Vector& bbFrame, const C3Vector& bbHalfSize);
    void _addCommonObjectEventData(CCbor* ev) const;

    int _objectHandle;
    long long int _objectUid; // valid for a given session (non-persistent)
    std::string _extensionString;
    int _visibilityLayer;
    bool _selected;
    bool _isInScene;
    bool _modelInvisible; // derived from parent model's modelProperty
    int _childOrder;
    std::string _objectAlias;
    C7Vector _localTransformation;

    std::vector<CSceneObject*> _childList;
    C7Vector _assemblingLocalTransformation; // When assembling this object
    bool _assemblingLocalTransformationIsUsed;
    std::vector<std::string> _assemblyMatchValuesChild;
    std::vector<std::string> _assemblyMatchValuesParent;
    CSceneObject* _parentObject;
    int _objectType;
    int _objectProperty;
    bool _modelBase;
    int _hierarchyColorIndex;
    int _collectionSelfCollisionIndicator;
    int _localObjectSpecialProperty;
    int _modelProperty;
    int _scriptExecPriority; // sim_scriptexecorder_first, etc.
    int _calculatedModelProperty;
    int _calculatedObjectProperty;
    std::string _modelAcknowledgement;

    // Old:
    std::string _objectName_old;
    std::string _objectAltName_old;

    bool _ignorePosAndCameraOrthoviewSize_forUndoRedo;

    int _getAllowedObjectSpecialProperties() const;

    int _dynamicSimulationIconCode;

    bool _restoreToDefaultLights;
    bool _forceAlwaysVisible_tmp;
    int _specificLight; // -1=default (i.e. all), -2=none, otherwise a light ID or collection ID. Is not serialized!

    int _parentObjectHandle_forSerializationOnly;

    std::string _dnaString;
    std::string _uniquePersistentIdString;
    std::string _copyString;

    int _ikPluginCounterpartHandle;
    std::string _objectTempAlias;
    std::string _objectTempName_old;
    std::string _objectTempAltName_old;

    int _authorizedViewableObjects; // -1 --> all, -2 --> none, otherwise object or collection handle

    bool _objectMovementPreferredAxesPreviousCtrlKeyDown;

    int _objectMovementPreferredAxes;  // bits 0-2: position x,y,z, bits 3-5: Euler e9,e1,e2
    int _objectMovementOptions;        // bit0=transl not ok when sim. stopped, bit1=transl not ok when sim. running,
                                       // bit2&bit3: same but for rotations, bit4: alt dir transl forbidden, bit5: alt dir rot forbidden
    double _objectMovementStepSize[2]; // 0.0= use app default
    int _objectMovementRelativity[2];  // 0=world, 1=parent, 2=own frame
    C7Vector _bbFrame;
    C3Vector _bbHalfSize;

    double _sizeFactor; // just used so that scripts can also adjust for scaling
    double _sizeValues[3];
    bool _dynamicsTemporarilyDisabled;
    bool _dynamicsResetFlag;

    double _transparentObjectDistanceOffset;

    C3Vector _objectManipulationModeSubTranslation;
    double _objectManipulationModeSubRotation;
    int _objectManipulationModeEventId;
    int _objectManipulationModeAxisIndex;
    C3Vector _objectManipulationModeRelativePositionOfClickedPoint;
    C3Vector _objectManipulationModeTotalTranslation;
    C3Vector _objectManipulationModeMouseDownPos;
    double _objectManipulationModeTotalRotation;
    unsigned char _objectManipulationMode_flaggedForGridOverlay; // is the rotation or translation axis index + 8 if it
                                                                 // is a rotation, or +16 if it is a translation

    CCustomData_old* _customObjectData_old;
    std::map<std::string, std::vector<SCustomRefs>> _customReferencedHandles;
    std::map<std::string, std::vector<SCustomOriginalRefs>> _customReferencedOriginalHandles;
    CUserParameters* _userScriptParameters;

    // Other variables
    int _mechanismID; // don't serialize!
    bool _beforeDeleteCallbackSent;
    bool _initialValuesInitialized;
    bool _initialConfigurationMemorized;
    long long int _initialParentUniqueId;
    int _initialMainPropertyOverride;
    C7Vector _initialLocalPose;
    C7Vector _initialAbsPose;
    int _initialVisibilityLayer;

    int _dynamicFlag; // 1=respondableShape, 2=nonStaticShape, 4=dynJoint, 32=dynForceSensor, 64=dynDummy

    double _measuredAngularVelocity_velocityMeasurement;
    C3Vector _measuredAngularVelocity3_velocityMeasurement;
    C3Vector _measuredAngularVelocityAxis_velocityMeasurement;
    C3Vector _measuredLinearVelocity_velocityMeasurement;
    C7Vector _previousAbsTransf_velocityMeasurement;
    bool _previousPositionOrientationIsValid;

#ifdef SIM_WITH_GUI
  public:
    virtual void displayFrames(CViewableBase* renderingObject, double size, bool persp);
    virtual void displayBoundingBox(CViewableBase* renderingObject, bool mainSelection);
    virtual void display(CViewableBase* renderingObject, int displayAttrib);
    void displayManipulationModeOverlayGrid(CViewableBase* renderingObject, double size, bool persp);
    bool setLocalTransformationFromObjectRotationMode(const C4X4Matrix& cameraAbsConf, double rotationAmount,
                                                      bool perspective, int eventID);
    bool setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,
                                                         const C3Vector& clicked3DPoint, double prevPos[2],
                                                         double pos[2], double screenHalfSizes[2], double halfSizes[2],
                                                         bool perspective, int eventID);
#endif

  private:
    void _setLocalTransformation_send(const C7Vector& tr) const;
    void _setParent_send(int parentHandle) const;
};
