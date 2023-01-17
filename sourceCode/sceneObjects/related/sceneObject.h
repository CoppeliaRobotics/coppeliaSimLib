#pragma once

#include "colorObject.h"
#include "3Vector.h"
#include "4Vector.h"
#include "7Vector.h"
#include "vMutex.h"
#include <map>
#include "userParameters.h"
#include "customData.h"
#include "syncObject.h"

struct SCustomRefs
{
    int generalObjectType; // e.g. sim_appobj_object_type, sim_appobj_collision_type
    int generalObjectHandle;
};

struct SCustomOriginalRefs
{
    int generalObjectType; // e.g. sim_appobj_object_type, sim_appobj_collision_type
    int generalObjectHandle;
    std::string uniquePersistentIdString;
};

class CShape;
class CCustomData_old;
class CViewableBase;
class CScriptObject;
class CInterfaceStack;

class CSceneObject : public CSyncObject
{
public:

    CSceneObject();
    virtual ~CSceneObject();

    // Overridden from CSyncObject:
    virtual void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    virtual void connectSynchronizationObject();
    virtual void removeSynchronizationObject(bool localReferencesToItOnly);

    virtual void display(CViewableBase* renderingObject,int displayAttrib);
    virtual void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    virtual CSceneObject* copyYourself();
    virtual void removeSceneDependencies();
    virtual void scaleObject(double scalingFactor);
    virtual void scaleObjectNonIsometrically(double x,double y,double z);
    virtual void serialize(CSer& ar);

    virtual void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    virtual void announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);

    virtual void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    virtual void performScriptLoadingMapping(const std::map<int,int>* map);
    virtual void performTextureObjectLoadingMapping(const std::map<int,int>* map);


    // Old:
    // -----------
    virtual void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    virtual void announceCollectionWillBeErased(int collectionID,bool copyBuffer);
    virtual void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    virtual void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    virtual void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    virtual void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    virtual void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    virtual void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    virtual void performGcsLoadingMapping(const std::map<int,int>* map);
    virtual void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
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

    virtual C7Vector getFullLocalTransformation() const;
    virtual bool setParent(CSceneObject* parent);

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
    unsigned short getVisibilityLayer() const;
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

    void setObjectHandle(int newObjectHandle);
    void setChildOrder(int order);
    void setExtensionString(const char* str);
    void setVisibilityLayer(unsigned short l);
    void setObjectAlias_direct(const char* newAlias);
    void setObjectName_direct_old(const char* newName);
    void setObjectAltName_direct_old(const char* newAltName);
    void setLocalTransformation(const C7Vector& tr);
    void setLocalTransformation(const C4Vector& q);
    void setLocalTransformation(const C3Vector& x);

    void recomputeModelInfluencedValues(int overrideFlags=-1);
    void setObjectUniqueId();
    void setSelected(bool s); // doesn't generate a sync msg
    void setIsInScene(bool s);
    void setParentPtr(CSceneObject* parent);

    size_t getScriptsToExecute(std::vector<int>& scriptHandles,int scriptType);
    size_t getScriptsToExecute_branch(std::vector<int>& scriptHandles,int scriptType);
    int getScriptExecutionOrder_old(int scriptType) const;
    int getScriptsToExecute_old(int scriptType,int parentTraversalDirection,std::vector<CScriptObject*>& scripts,std::vector<int>& uniqueIds);

    void scalePosition(double scalingFactor);
    void getAllObjectsRecursive(std::vector<CSceneObject*>* objectList,bool baseIncluded=true,bool start=true) const;
    void getChain(std::vector<CSceneObject*>& objectList,bool tipIncluded=true,bool start=true) const;

    void setReferencedHandles(size_t cnt,const int* handles);
    size_t getReferencedHandlesCount() const;
    size_t getReferencedHandles(int* handles) const;
    void setReferencedOriginalHandles(int cnt,const int* handles);
    size_t getReferencedOriginalHandlesCount() const;
    size_t getReferencedOriginalHandles(int* handles) const;
    void checkReferencesToOriginal(const std::map<std::string,int>& allUniquePersistentIdStrings);

    CSceneObject* getFirstParentInSelection(const std::vector<CSceneObject*>* sel) const;
    CSceneObject* getLastParentInSelection(const std::vector<CSceneObject*>* sel) const;
    void setDynamicFlag(int flag);
    int getDynamicFlag() const;

    void setSpecificLight(int h);
    int getSpecificLight() const;
    bool setBeforeDeleteCallbackSent();

    bool getGlobalMarkingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool& minMaxNotYetDefined,bool first,bool guiIsRendering);
    void getBoundingBoxEncompassingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool guiIsRendering);

    int getModelSelectionHandle(bool firstObject=true);

    void setObjectProperty(int p);
    int getObjectProperty() const;
    int getCumulativeObjectProperty();
    void setModelBase(bool m);

    void setObjectMovementPreferredAxes(int p);
    int getObjectMovementPreferredAxes() const;

    void setObjectMovementOptions(int p);
    int getObjectMovementOptions() const;
    void setObjectMovementRelativity(int index,int p);
    int getObjectMovementRelativity(int index) const;
    void setObjectMovementStepSize(int index,double s);
    double getObjectMovementStepSize(int index) const;

    void writeCustomDataBlock(bool tmpData,const char* dataName,const char* data,size_t dataLength);
    std::string readCustomDataBlock(bool tmpData,const char* dataName) const;
    std::string getAllCustomDataBlockTags(bool tmpData,size_t* cnt) const;

    void clearObjectCustomData_old();
    int getObjectCustomDataLength_old(int header) const;
    void setObjectCustomData_old(int header,const char* data,int dataLength);
    void getObjectCustomData_old(int header,char* data) const;

    int getParentCount() const;

    void setLocalObjectSpecialProperty(int prop);
    int getLocalObjectSpecialProperty() const;
    void setModelProperty(int prop);
    int getModelProperty() const;
    int getCumulativeModelProperty();
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

    void setScriptExecPriority(int p);
    int getScriptExecPriority() const;


    void setDynamicsResetFlag(bool reset,bool fullHierarchyTree);
    bool getDynamicsResetFlag() const;

    void setDynamicSimulationIconCode(int c);
    int getDynamicSimulationIconCode() const;

    void clearManipulationModeOverlayGridFlag();

    bool isObjectVisible();
    bool isObjectInVisibleLayer();
    bool isObjectPartOfInvisibleModel();

    void setModelAcknowledgement(const char* a);
    std::string getModelAcknowledgement() const;

    bool getShouldObjectBeDisplayed(int viewableHandle,int displayAttrib);

    void setAssemblingLocalTransformation(const C7Vector& tr);
    C7Vector getAssemblingLocalTransformation() const;
    void setAssemblingLocalTransformationIsUsed(bool u);
    bool getAssemblingLocalTransformationIsUsed();
    void setAssemblyMatchValues(bool asChild,const char* str);
    std::string getAssemblyMatchValues(bool asChild) const;
    const std::vector<std::string>* getChildAssemblyMatchValuesPointer() const;
    int getAllChildrenThatMayBecomeAssemblyParent(const std::vector<std::string>* assemblingChildMatchValues,std::vector<CSceneObject*>& objects) const;
    bool doesParentAssemblingMatchValuesMatchWithChild(const std::vector<std::string>* assemblingChildMatchValues,bool ignoreDefaultNames=false) const;

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
    int getChildSequence(const CSceneObject* child) const;
    bool setChildSequence(CSceneObject* child,int order);

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
    bool isObjectParentedWith(const CSceneObject* thePotentialParent) const;

    void setParentHandle_forSerializationOnly(int pHandle);

    void getFirstModelRelatives(std::vector<CSceneObject*>& firstModelRelatives,bool visibleModelsOnly) const;
    int countFirstModelRelatives(bool visibleModelsOnly) const;
    std::string getObjectTempAlias() const;
    std::string getObjectTempName_old() const;
    std::string getObjectTempAltName_old() const;
    void acquireCommonPropertiesFromObject_simpleXMLLoading(const CSceneObject* obj);
    CUserParameters* getUserScriptParameterObject();
    void setUserScriptParameterObject(CUserParameters* obj);

    void setIgnorePosAndCameraOrthoviewSize_forUndoRedo(bool s);

    void pushObjectCreationEvent() const;
    void pushObjectRefreshEvent() const;
    void getBoundingBox(C3Vector& vmin,C3Vector& vmax) const;

protected:
    void _setModelInvisible(bool inv);
    void _setBoundingBox(const C3Vector& vmin,const C3Vector& vmax);
    void _addCommonObjectEventData(CInterfaceStackTable* data) const;
    void _appendObjectMovementEventData(CInterfaceStackTable* data) const;

    int _objectHandle;
    long long int _objectUid; // valid for a given session (non-persistent)
    std::string _extensionString;
    unsigned short _visibilityLayer;
    bool _selected;
    bool _isInScene;
    bool _modelInvisible;
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

    int _objectMovementPreferredAxes; //bits 0-2: position x,y,z, bits 3-5: Euler e9,e1,e2
    int _objectMovementOptions; // bit0=transl not ok when sim. stopped, bit1=transl not ok when sim. running, bit2&bit3: same but for rotations, bit4: manualShift forbidden, bit5: manualRot forbidden, bit6-bit8: forbidden local translation axes, bit9-bit11: forbidden local rotation axes
    double _objectMovementStepSize[2]; // 0.0= use app default
    int _objectMovementRelativity[2]; //0=world, 1=parent, 2=own frame
    C3Vector _boundingBoxMin;
    C3Vector _boundingBoxMax;

    double _sizeFactor; // just used so that scripts can also adjust for scaling
    double _sizeValues[3];
    bool _dynamicsTemporarilyDisabled;
    bool _dynamicsResetFlag;

    double _transparentObjectDistanceOffset;

    VMutex _objectMutex;
    C3Vector _objectManipulationModeSubTranslation;
    double _objectManipulationModeSubRotation;
    int _objectManipulationModeEventId;
    int _objectManipulationModeAxisIndex;
    C3Vector _objectManipulationModeRelativePositionOfClickedPoint;
    C3Vector _objectManipulationModeTotalTranslation;
    double _objectManipulationModeTotalRotation;
    unsigned char _objectManipulationMode_flaggedForGridOverlay; // is the rotation or translation axis index + 8 if it is a rotation, or +16 if it is a translation
    CCustomData _customObjectData;
    CCustomData _customObjectData_tempData; // this one is not serialized (but copied)!
    CCustomData_old* _customObjectData_old;
    std::vector<SCustomRefs> _customReferencedHandles;
    std::vector<SCustomOriginalRefs> _customReferencedOriginalHandles;
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

    int _dynamicFlag; // 1=respondableShape, 2=nonStaticShape, 4=dynJoint, 32=dynForceSensor, 64=dynDummy

    double _measuredAngularVelocity_velocityMeasurement;
    C3Vector _measuredAngularVelocity3_velocityMeasurement;
    C3Vector _measuredAngularVelocityAxis_velocityMeasurement;
    C3Vector _measuredLinearVelocity_velocityMeasurement;
    C7Vector _previousAbsTransf_velocityMeasurement;
    bool _previousPositionOrientationIsValid;


#ifdef SIM_WITH_GUI
public:
    void displayManipulationModeOverlayGrid(bool transparentAndOverlay);
    bool setLocalTransformationFromObjectRotationMode(const C4X4Matrix& cameraAbsConf,double rotationAmount,bool perspective,int eventID);
    bool setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,double prevPos[2],double pos[2],double screenHalfSizes[2],double halfSizes[2],bool perspective,int eventID);
#endif

private:
    void _setLocalTransformation_send(const C7Vector& tr) const;
    void _setParent_send(int parentHandle) const;
};
