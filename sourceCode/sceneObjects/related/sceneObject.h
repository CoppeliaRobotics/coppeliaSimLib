#pragma once

#include "colorObject.h"
#include "extIkSer.h"
#include "3Vector.h"
#include "4Vector.h"
#include "7Vector.h"
#include "vMutex.h"
#include <map>
#include "userParameters.h"
#include "_sceneObject_.h"

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
class CCustomData;
class CViewableBase;
class CLuaScriptObject;
class CInterfaceStack;

class CSceneObject : public _CSceneObject_
{
public:

    CSceneObject();
    virtual ~CSceneObject();

    // Overridden from CSyncObject:
    virtual void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    virtual void connectSynchronizationObject();
    virtual void removeSynchronizationObject(bool localReferencesToItOnly);

    virtual void display(CViewableBase* renderingObject,int displayAttrib);
    virtual CSceneObject* copyYourself();
    virtual void removeSceneDependencies();
    virtual void scaleObject(float scalingFactor);
    virtual void scaleObjectNonIsometrically(float x,float y,float z);
    virtual void serialize(CSer& ar);
    virtual void serializeWExtIk(CExtIkSer& ar);

    virtual bool announceObjectWillBeErased(int objHandle,bool copyBuffer);
    virtual void announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript,bool copyBuffer);

    virtual void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    virtual void performScriptLoadingMapping(const std::vector<int>* map);
    virtual void performTextureObjectLoadingMapping(const std::vector<int>* map);

    // Old:
    // -----------
    virtual void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    virtual void announceCollectionWillBeErased(int collectionID,bool copyBuffer);
    virtual void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    virtual void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    virtual void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    virtual void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    virtual void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    virtual void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    virtual void performGcsLoadingMapping(const std::vector<int>* map);
    virtual void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    // -----------

    virtual void simulationAboutToStart();
    virtual void simulationEnded();
    virtual void initializeInitialValues(bool simulationAlreadyRunning);
    virtual bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    virtual bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    virtual bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;

    virtual std::string getObjectTypeInfo() const;
    virtual std::string getObjectTypeInfoExtended() const;
    virtual bool isPotentiallyCollidable() const;
    virtual bool isPotentiallyMeasurable() const;
    virtual bool isPotentiallyDetectable() const;
    virtual bool isPotentiallyRenderable() const;


    // Overridden from _CSceneObject_:
    bool setObjectName(const char* newName,bool check);
    bool setObjectAltName(const char* newAltName,bool check);


    void setRestoreToDefaultLights(bool s);
    bool getRestoreToDefaultLights() const;
    
    int getScriptExecutionOrder(int scriptType) const;
    int getScriptsToExecute(int scriptType,int parentTraversalDirection,std::vector<CLuaScriptObject*>& scripts,std::vector<int>& uniqueIds);

    void scalePosition(float scalingFactor);
    void getAllObjectsRecursive(std::vector<CSceneObject*>* objectList,bool baseIncluded=true,bool start=true) const;
    void getChain(std::vector<CSceneObject*>& objectList,bool tipIncluded=true,bool start=true) const;
    int getUniqueID() const;

    void setReferencedHandles(size_t cnt,const int* handles);
    size_t getReferencedHandlesCount() const;
    size_t getReferencedHandles(int* handles) const;
    void setReferencedOriginalHandles(int cnt,const int* handles);
    size_t getReferencedOriginalHandlesCount() const;
    size_t getReferencedOriginalHandles(int* handles) const;
    void checkReferencesToOriginal(const std::map<std::string,int>& allUniquePersistentIdStrings);

    CSceneObject* getFirstParentInSelection(const std::vector<CSceneObject*>* sel) const;
    CSceneObject* getLastParentInSelection(const std::vector<CSceneObject*>* sel) const;
    void setObjectType(int theType);
    void setDynamicObjectFlag_forVisualization(int isDynamicallySimulatedFlag);
    int getDynamicObjectFlag_forVisualization() const;

    void setSpecificLight(int h);
    int getSpecificLight() const;
    bool setBeforeDeleteCallbackSent();

    bool getGlobalMarkingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool& minMaxNotYetDefined,bool first,bool guiIsRendering) const;
    void getBoundingBoxEncompassingBoundingBox(const C7Vector& baseCoordInv,C3Vector& min,C3Vector& max,bool guiIsRendering) const;

    int getModelSelectionHandle(bool firstObject=true);

    void setLocalObjectProperty(int p);
    int getLocalObjectProperty() const;
    int getCumulativeObjectProperty();
    void setModelBase(bool m);

    void setObjectManipulationModePermissions(int p);
    int getObjectManipulationModePermissions() const;
    void setObjectTranslationDisabledDuringSimulation(bool d);
    bool getObjectTranslationDisabledDuringSimulation() const;
    void setObjectTranslationDisabledDuringNonSimulation(bool d);
    bool getObjectTranslationDisabledDuringNonSimulation() const;
    void setObjectTranslationSettingsLocked(bool l);
    bool getObjectTranslationSettingsLocked() const;
    void setObjectRotationDisabledDuringSimulation(bool d);
    bool getObjectRotationDisabledDuringSimulation() const;
    void setObjectRotationDisabledDuringNonSimulation(bool d);
    bool getObjectRotationDisabledDuringNonSimulation() const;
    void setObjectRotationSettingsLocked(bool l);
    bool getObjectRotationSettingsLocked() const;


    void setObjectManipulationTranslationRelativeTo(int p);
    int getObjectManipulationTranslationRelativeTo() const;
    void setObjectManipulationRotationRelativeTo(int p);
    int getObjectManipulationRotationRelativeTo() const;
    void setNonDefaultTranslationStepSize(float s);
    float getNonDefaultTranslationStepSize() const;
    void setNonDefaultRotationStepSize(float s);
    float getNonDefaultRotationStepSize() const;

    void setObjectCustomData(int header,const char* data,int dataLength);
    int getObjectCustomDataLength(int header) const;
    void getObjectCustomData(int header,char* data) const;
    bool getObjectCustomDataHeader(int index,int& header) const;

    // Following same as above, but not serialized:
    void setObjectCustomData_tempData(int header,const char* data,int dataLength);
    int getObjectCustomDataLength_tempData(int header) const;
    void getObjectCustomData_tempData(int header,char* data) const;
    bool getObjectCustomDataHeader_tempData(int index,int& header) const;

    int getParentCount() const;

    void setLocalObjectSpecialProperty(int prop);
    int getLocalObjectSpecialProperty() const;
    void setLocalModelProperty(int prop);
    int getLocalModelProperty() const;
    int getCumulativeModelProperty();
    int getCumulativeObjectSpecialProperty();

    int getTreeDynamicProperty(); // combination of sim_objdynprop_dynamic and sim_objdynprop_respondable

    void disableDynamicTreeForManipulation(bool d);

    void setTransparentObjectDistanceOffset(float d);
    float getTransparentObjectDistanceOffset() const;

    void setAuthorizedViewableObjects(int objOrCollHandle);
    int getAuthorizedViewableObjects() const;

    void setMechanismID(int id);
    int getMechanismID() const;

    // Following two needed when a shape is edited during simulation
    void incrementMemorizedConfigurationValidCounter();
    int getMemorizedConfigurationValidCounter();

    void setSizeFactor(float f);
    float getSizeFactor() const;
    void setSizeValues(const float s[3]);
    void getSizeValues(float s[3]) const;


    void setDynamicsFullRefreshFlag(bool refresh);
    bool getDynamicsFullRefreshFlag() const;

    void setDynamicSimulationIconCode(int c);
    int getDynamicSimulationIconCode() const;

    void clearManipulationModeOverlayGridFlag();

    bool isObjectPartOfInvisibleModel();

    void setModelAcknowledgement(const char* a);
    std::string getModelAcknowledgement() const;

    void setIgnoredByViewFitting(bool ignored);
    bool getIgnoredByViewFitting() const;

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

    size_t getChildCount() const;
    CSceneObject* getChildFromIndex(size_t index) const;
    const std::vector<CSceneObject*>* getChildren() const;
    void addChild(CSceneObject* child);
    bool removeChild(const CSceneObject* child);

    void setHierarchyColorIndex(int c);
    int getHierarchyColorIndex() const;

    void setCollectionSelfCollisionIndicator(int c);
    int getCollectionSelfCollisionIndicator() const;

    void measureVelocity(float dt); // should be called just after the main script was called!!!
    float getMeasuredAngularVelocity() const;
    C3Vector getMeasuredLinearVelocity() const;
    C3Vector getMeasuredAngularVelocity3() const;
    C3Vector getMeasuredAngularVelocityAxis() const;

    void setForceAlwaysVisible_tmp(bool force);

    static void incrementModelPropertyValidityNumber();



    void setAbsoluteTransformation(const C7Vector& v);
    void setAbsoluteTransformation(const C4Vector& q);
    void setAbsoluteTransformation(const C3Vector& x);

    void getCumulativeTransformationMatrix(float m[4][4]) const;

    // Old IK functionality:
    C7Vector getFullParentCumulativeTransformation_ikOld() const;
    C7Vector getFullCumulativeTransformation_ikOld() const;
    C7Vector getFullLocalTransformation_ikOld() const;
    C7Vector getCumulativeTransformation_ikOld() const;

    int getIkPluginCounterpartHandle() const;

    std::string getDisplayName() const;
    bool isObjectParentedWith(const CSceneObject* thePotentialParent) const;

    void setParentHandle_forSerializationOnly(int pHandle);

    // Overridden from _CSceneObject_:
    virtual bool setParent(CSceneObject* newParent,bool keepObjectInPlace);

    int getObjectType() const;
    void getFirstModelRelatives(std::vector<CSceneObject*>& firstModelRelatives,bool visibleModelsOnly) const;
    int countFirstModelRelatives(bool visibleModelsOnly) const;
    std::string getObjectTempName() const;
    std::string getObjectTempAltName() const;
    void acquireCommonPropertiesFromObject_simpleXMLLoading(const CSceneObject* obj);
    CUserParameters* getUserScriptParameterObject();
    void setUserScriptParameterObject(CUserParameters* obj);

    void setIgnorePosAndCameraOrthoviewSize_forUndoRedo(bool s);


protected:
    bool _ignorePosAndCameraOrthoviewSize_forUndoRedo;

    int _getAllowedObjectSpecialProperties() const;

    int _dynamicSimulationIconCode;
    static int _modelPropertyValidityNumber;

    bool _restoreToDefaultLights;
    bool _forceAlwaysVisible_tmp;
    int _specificLight; // -1=default (i.e. all), -2=none, otherwise a light ID or collection ID. Is not serialized!

    int _uniqueID; // valid for a given scene instance (non-persistent)

    std::vector<CSceneObject*> _childList;
    int _parentObjectHandle_forSerializationOnly;

    std::string _dnaString;
    std::string _uniquePersistentIdString;


    int _ikPluginCounterpartHandle;
    std::string _objectTempName;
    std::string _objectTempAltName;

    int _cumulativeModelProperty;
    int _cumulativeModelPropertyValidityNumber;

    int _authorizedViewableObjects; // -1 --> all, -2 --> none, otherwise object or collection handle

    int _objectManipulationModePermissions;
    bool _objectTranslationDisabledDuringSimulation;
    bool _objectTranslationDisabledDuringNonSimulation;
    bool _objectTranslationSettingsLocked;
    bool _objectRotationDisabledDuringSimulation;
    bool _objectRotationDisabledDuringNonSimulation;
    bool _objectRotationSettingsLocked;

    bool _objectManipulationModePermissionsPreviousCtrlKeyDown;
    float _objectTranslationNonDefaultStepSize;
    float _objectRotationNonDefaultStepSize;
    int _objectManipulationTranslationRelativeTo; //0=world, 1=parent, 2=own frame
    int _objectManipulationRotationRelativeTo; //0=world, 1=parent, 2=own frame
    int _memorizedConfigurationValidCounter;
    float _sizeFactor; // just used so that scripts can also adjust for scaling
    float _sizeValues[3];
    int _dynamicProperty; // modified in the dynamics routines. Not saved nor copied!
    bool _dynamicsTemporarilyDisabled;
    bool _dynamicsFullRefreshFlag;

    float _transparentObjectDistanceOffset;

    VMutex _objectMutex;
    C3Vector _objectManipulationModeSubTranslation;
    float _objectManipulationModeSubRotation;
    int _objectManipulationModeEventId;
    int _objectManipulationModeAxisIndex;
    C3Vector _objectManipulationModeRelativePositionOfClickedPoint;
    C3Vector _objectManipulationModeTotalTranslation;
    float _objectManipulationModeTotalRotation;
    unsigned char _objectManipulationMode_flaggedForGridOverlay; // is the rotation or translation axis index + 8 if it is a rotation, or +16 if it is a translation
    CCustomData* _customObjectData;
    CCustomData* _customObjectData_tempData; // this one is not serialized (but copied)!
    std::vector<SCustomRefs> _customReferencedHandles;
    std::vector<SCustomOriginalRefs> _customReferencedOriginalHandles;
    CUserParameters* _userScriptParameters;


    // Other variables
    int _mechanismID; // don't serialize!
    bool _beforeDeleteCallbackSent;
    bool _initialValuesInitialized;
    bool _initialConfigurationMemorized;
    int _initialParentUniqueID;
    int _initialMainPropertyOverride;
    int _initialMemorizedConfigurationValidCounter;
    C7Vector _initialLocalTransformationPart1;
    static int _uniqueIDCounter;

    int _dynamicObjectFlag_forVisualization;

    float _measuredAngularVelocity_velocityMeasurement;
    C3Vector _measuredAngularVelocity3_velocityMeasurement;
    C3Vector _measuredAngularVelocityAxis_velocityMeasurement;
    C3Vector _measuredLinearVelocity_velocityMeasurement;
    C7Vector _previousAbsTransf_velocityMeasurement;
    bool _previousPositionOrientationIsValid;


#ifdef SIM_WITH_GUI
public:
    void displayManipulationModeOverlayGrid(bool transparentAndOverlay);
    bool setLocalTransformationFromObjectRotationMode(const C4X4Matrix& cameraAbsConf,float rotationAmount,bool perspective,int eventID);
    bool setLocalTransformationFromObjectTranslationMode(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID);
#endif

private:
    // Overridden from _CSceneObject_:
    void _setLocalTransformation_send(const C7Vector& tr) const;
    void _setParent_send(int parentHandle,bool keepObjectInPlace) const;
};
