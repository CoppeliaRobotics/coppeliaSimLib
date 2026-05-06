#pragma once

#include <registeredPathPlanningTasks.h>
#include <environment.h>
#include <pageContainer.h>
#include <mainSettings_old.h>
#include <simulation.h>
#include <buttonBlockContainer.h>
#include <outsideCommandQueue.h>
#include <customData.h>
#include <customData_old.h>
#include <cacheCont.h>
#include <textureContainer.h>
#include <drawingContainer.h>
#include <pointCloudContainer_old.h>
#include <ghostObjectContainer.h>
#include <bannerContainer.h>
#include <dynamicsContainer.h>
#include <commTubeContainer.h>
#include <undoBufferCont.h>
#include <collectionContainer.h>
#include <distanceObjectContainer_old.h>
#include <collisionObjectContainer_old.h>
#include <ikGroupContainer.h>
#include <sceneObjectContainer.h>
#include <customObjectContainer.h>
#include <obj.h>

struct SLoadOperationIssue
{
    int verbosity;
    std::string message;
    int objectHandle;
};

class CScene : public Obj
{
  public:
    CScene();
    virtual ~CScene();

    void rebuildScene_oldIk();
    void removeScene_oldIk();

    void initializeScene();
    void clearScene(bool notCalledFromUndoFunction);
    void deleteScene();

    bool loadScene(CSer& ar, bool forUndoRedoOperation);
    void saveScene(CSer& ar, bool regularSave = true);
    bool loadModel(CSer& ar, bool justLoadThumbnail, bool forceModelAsCopy, C7Vector* optionalModelTr,
                   C3Vector* optionalModelBoundingBoxSize, double* optionalModelNonDefaultTranslationStepSize);

    void instancePass();
    void simulationAboutToStart();
    void simulationPaused();
    void simulationAboutToResume();
    void simulationAboutToStep();
    void simulationAboutToEnd();
    void simulationEnded(bool removeNewObjects);

    int addGeneralObjectsToSceneAndPerformMappings(std::vector<CSceneObject*>* loadedObjectList,
                                                    std::vector<CCollection*>* loadedCollectionList,
                                                    std::vector<CCollisionObject_old*>* loadedCollisionList,
                                                    std::vector<CDistanceObject_old*>* loadedDistanceList,
                                                    std::vector<CIkGroup_old*>* loadedIkGroupList,
                                                    std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
                                                    std::vector<CButtonBlock*>* loadedButtonBlockList,
                                                    std::vector<CDetachedScript*>* loadedLuaScriptList,
                                                    std::vector<CTextureObject*>& loadedTextureObjectList,
                                                    std::vector<CDynMaterialObject*>& loadedDynMaterialObjectList,
                                                    bool model, int fileSimVersion, bool forceModelAsCopy);

    void cleanupHashNames_allObjects(int suffix);

    void announceObjectWillBeErased(const CSceneObject* object);
    void announceScriptWillBeErased(int scriptOrDetachedScriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int detachedScriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);

    CDetachedScript* getDetachedScriptFromHandle(int scriptHandle) const;
    CDetachedScript* getDetachedScriptFromUid(int uid) const;
    void getActiveScripts(std::vector<CDetachedScript*>& scripts, bool reverse = false, bool alsoLegacyScripts = false) const;
    void callScripts(int callType, CInterfaceStack* inStack, CInterfaceStack* outStack, CSceneObject* objectBranch = nullptr, int scriptToExclude = -1);

    void pushGenesisEvents();

    int setBoolProperty_t(long long int target, const char* pName, bool pState);
    int getBoolProperty_t(long long int target, const char* pName, bool& pState) const;
    int setIntProperty_t(long long int target, const char* pName, int pState);
    int getIntProperty_t(long long int target, const char* pName, int& pState) const;
    int setLongProperty_t(long long int target, const char* pName, long long int pState);
    int getLongProperty_t(long long int target, const char* pName, long long int& pState) const;
    int setFloatProperty_t(long long int target, const char* pName, double pState);
    int getFloatProperty_t(long long int target, const char* pName, double& pState) const;
    int setHandleProperty_t(long long int target, const char* pName, long long int pState);
    int getHandleProperty_t(long long int target, const char* pName, long long int& pState) const;
    int setStringProperty_t(long long int target, const char* pName, const std::string& pState);
    int getStringProperty_t(long long int target, const char* pName, std::string& pState) const;
    int setTableProperty_t(long long int target, const char* pName, const std::string& pState);
    int getTableProperty_t(long long int target, const char* pName, std::string& pState) const;
    int setBufferProperty_t(long long int target, const char* pName, const std::string& pState);
    int getBufferProperty_t(long long int target, const char* pName, std::string& pState) const;
    int setIntArray2Property_t(long long int target, const char* pName, const int* pState);
    int getIntArray2Property_t(long long int target, const char* pName, int* pState) const;
    int setVector3Property_t(long long int target, const char* pName, const C3Vector& pState);
    int getVector3Property_t(long long int target, const char* pName, C3Vector& pState) const;
    int setMatrixProperty_t(long long int target, const char* pName, const CMatrix& pState);
    int getMatrixProperty_t(long long int target, const char* pName, CMatrix& pState) const;
    int setQuaternionProperty_t(long long int target, const char* pName, const C4Vector& pState);
    int getQuaternionProperty_t(long long int target, const char* pName, C4Vector& pState) const;
    int setPoseProperty_t(long long int target, const char* pName, const C7Vector& pState);
    int getPoseProperty_t(long long int target, const char* pName, C7Vector& pState) const;
    int setColorProperty_t(long long int target, const char* pName, const float* pState);
    int getColorProperty_t(long long int target, const char* pName, float* pState) const;
    int setFloatArrayProperty_t(long long int target, const char* pName, const std::vector<double>& pState);
    int getFloatArrayProperty_t(long long int target, const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty_t(long long int target, const char* pName, const std::vector<int>& pState);
    int getIntArrayProperty_t(long long int target, const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty_t(long long int target, const char* pName, const std::vector<long long int>& pState); // ALL handle items have to be of the same type
    int getHandleArrayProperty_t(long long int target, const char* pName, std::vector<long long int>& pState) const; // ALL handle items have to be of the same type
    int setStringArrayProperty_t(long long int target, const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty_t(long long int target, const char* pName, std::vector<std::string>& pState) const;
    int setMethodProperty_t(long long int target, const char* pName, const void* pState);
    int getMethodProperty_t(long long int target, const char* pName, void*& pState) const;
    int setMethodProperty_t(long long int target, const char* pName, const std::string& pState);
    int getMethodProperty_t(long long int target, const char* pName, std::string& pState) const;
    int removeProperty_t(long long int target, const char* pName);
    int getPropertyName_t(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_t(long long int target, const char* pName, int& info, std::string& infoTxt) const;
    int setPropertyInfo_t(long long int target, const char* pName, int info, const char* infoTxt);

    // Old:
    void announceIkGroupWillBeErased(int ikGroupHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void announceCollisionWillBeErased(int collisionHandle);
    void announceDistanceWillBeErased(int distanceHandle);
    void announce2DElementWillBeErased(int elementID);
    void announce2DElementButtonWillBeErased(int elementID, int buttonID);

    static void appendLoadOperationIssue(int verbosity, const char* text, int objectId);
    static int getLoadingMapping(const std::map<int, int>* map, int oldVal);
    static void registerNewHandle(UID newHandle, int objectType);
    static void releaseNewHandle(UID newHandle, int objectType = -1);
    static UID getNewHandleFromOldHandle(int oldHandle);
    static int getOldHandleFromNewHandle(UID newHandle);
    static int getOldSerializationHandleFromNewHandle(UID newHandle, int objectType);
    static void resetOldSerializationHandles();

    CUndoBufferCont* undoBufferContainer;
    CDynamicsContainer* dynamicsContainer;
    CEnvironment* environment;
    CPageContainer* pageContainer;
    CTextureContainer* textureContainer;
    CSimulation* simulation;
    CCustomData customSceneData;
    CCustomData customSceneData_volatile; // same as above, but not serialized! (scene-level signals)
    CCacheCont* cacheData;
    CDrawingContainer* drawingCont;
    CCollectionContainer* collections;
    CSceneObjectContainer* sceneObjects;
    CustomObjectContainer* customObjects;

    // Old:
    CMainSettings* mainSettings_old;
    CCustomData_old* customSceneData_old;
    CDistanceObjectContainer_old* distances_old;
    CCollisionObjectContainer_old* collisions_old;
    CIkGroupContainer* ikGroups_old;
    CRegisteredPathPlanningTasks* pathPlanning_old;
    CPointCloudContainer_old* pointCloudCont_old;
    CGhostObjectContainer* ghostObjectCont_old;
    CCommTubeContainer* commTubeContainer_old;
    CBannerContainer* bannerCont_old;
    COutsideCommandQueue* outsideCommandQueue_old;
    CButtonBlockContainer* buttonBlockContainer_old;

  private:
    bool _loadModelOrScene(CSer& ar, bool selectLoaded, bool isScene, bool justLoadThumbnail, bool forceModelAsCopy,
                           C7Vector* optionalModelTr, C3Vector* optionalModelBoundingBoxSize,
                           double* optionalModelNonDefaultTranslationStepSize);
    bool _loadSimpleXmlSceneOrModel(CSer& ar);
    bool _saveSimpleXmlScene(CSer& ar);

    void _getMinAndMaxNameSuffixes(int& smallestSuffix, int& biggestSuffix) const;
    int _getSuffixOffsetForGeneralObjectToAdd(bool tempNames, std::vector<CSceneObject*>* loadedObjectList,
                                              std::vector<CCollection*>* loadedCollectionList,
                                              std::vector<CCollisionObject_old*>* loadedCollisionList,
                                              std::vector<CDistanceObject_old*>* loadedDistanceList,
                                              std::vector<CIkGroup_old*>* loadedIkGroupList,
                                              std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
                                              std::vector<CButtonBlock*>* loadedButtonBlockList,
                                              std::vector<CDetachedScript*>* loadedLuaScriptList) const;
    bool _canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void _setSuffix1ToSuffix2(int suffix1, int suffix2);

    int _savedMouseMode;
    std::vector<long long int> _initialObjectUniqueIdentifiersForRemovingNewObjects;

    static std::vector<SLoadOperationIssue> _loadOperationIssues;
    static std::vector<bool> oldHandles;
    static std::map<int, S_UID> uidFromOldHandle;
    static std::map<UID, S_OID> oldHandleFromUid;
    static int _nextOldSerializationHandle_object;
    static int _nextOldSerializationHandle_collection;
    static int _nextOldSerializationHandle_script;
    static int _nextOldSerializationHandle_texture;
    static int _nextOldSerializationHandle_mesh;
    static std::map<UID, int> oldSerializationHandleFromUid;

#ifdef SIM_WITH_GUI
  public:
    void renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase* renderingObject, int displayAttrib, int windowSize[2], double verticalViewSizeOrAngle, bool perspective);
    void renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase* renderingObject, int displayAttrib, int windowSize[2], double verticalViewSizeOrAngle, bool perspective);
    void renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase* renderingObject, int displayAttrib, int windowSize[2], double verticalViewSizeOrAngle, bool perspective);
#endif
};
