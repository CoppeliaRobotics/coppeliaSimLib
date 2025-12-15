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
#include <propertiesAndMethods.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
SCENEOBJECT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_sceneObject;
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
    virtual int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState);
    virtual int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const;
    virtual int removeProperty(const char* pName);
    virtual int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    static int getPropertyName_bstatic(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
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
