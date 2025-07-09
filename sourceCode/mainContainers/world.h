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

struct SLoadOperationIssue
{
    int verbosity;
    std::string message;
    int objectHandle;
};

class CWorld
{
  public:
    CWorld();
    virtual ~CWorld();

    void rebuildWorld_oldIk();
    void removeWorld_oldIk();

    void initializeWorld();
    void clearScene(bool notCalledFromUndoFunction);
    void deleteWorld();
    int getWorldHandle() const;
    void setWorldHandle(int handle);

    bool loadScene(CSer& ar, bool forUndoRedoOperation);
    void saveScene(CSer& ar, bool regularSave = true);
    bool loadModel(CSer& ar, bool justLoadThumbnail, bool forceModelAsCopy, C7Vector* optionalModelTr,
                   C3Vector* optionalModelBoundingBoxSize, double* optionalModelNonDefaultTranslationStepSize);

    void simulationAboutToStart();
    void simulationPaused();
    void simulationAboutToResume();
    void simulationAboutToStep();
    void simulationAboutToEnd();
    void simulationEnded(bool removeNewObjects);

    void addGeneralObjectsToWorldAndPerformMappings(std::vector<CSceneObject*>* loadedObjectList,
                                                    std::vector<CCollection*>* loadedCollectionList,
                                                    std::vector<CCollisionObject_old*>* loadedCollisionList,
                                                    std::vector<CDistanceObject_old*>* loadedDistanceList,
                                                    std::vector<CIkGroup_old*>* loadedIkGroupList,
                                                    std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
                                                    std::vector<CButtonBlock*>* loadedButtonBlockList,
                                                    std::vector<CScriptObject*>* loadedLuaScriptList,
                                                    std::vector<CTextureObject*>& loadedTextureObjectList,
                                                    std::vector<CDynMaterialObject*>& loadedDynMaterialObjectList,
                                                    bool model, int fileSimVersion, bool forceModelAsCopy);

    void cleanupHashNames_allObjects(int suffix);

    void announceObjectWillBeErased(const CSceneObject* object);
    void announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);

    CScriptObject* getScriptObjectFromHandle(int scriptHandle) const;
    CScriptObject* getScriptObjectFromUid(int uid) const;
    void getActiveScripts(std::vector<CScriptObject*>& scripts, bool reverse = false, bool alsoLegacyScripts = false) const;
    void callScripts(int callType, CInterfaceStack* inStack, CInterfaceStack* outStack, CSceneObject* objectBranch = nullptr, int scriptToExclude = -1);

    void pushGenesisEvents();

    int setBoolProperty(long long int target, const char* pName, bool pState);
    int getBoolProperty(long long int target, const char* pName, bool& pState) const;
    int setIntProperty(long long int target, const char* pName, int pState);
    int getIntProperty(long long int target, const char* pName, int& pState) const;
    int setLongProperty(long long int target, const char* pName, long long int pState);
    int getLongProperty(long long int target, const char* pName, long long int& pState) const;
    int setFloatProperty(long long int target, const char* pName, double pState);
    int getFloatProperty(long long int target, const char* pName, double& pState) const;
    int setStringProperty(long long int target, const char* pName, const char* pState);
    int getStringProperty(long long int target, const char* pName, std::string& pState) const;
    int setBufferProperty(long long int target, const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(long long int target, const char* pName, std::string& pState) const;
    int setIntArray2Property(long long int target, const char* pName, const int* pState);
    int getIntArray2Property(long long int target, const char* pName, int* pState) const;
    int setVector2Property(long long int target, const char* pName, const double* pState);
    int getVector2Property(long long int target, const char* pName, double* pState) const;
    int setVector3Property(long long int target, const char* pName, const C3Vector& pState);
    int getVector3Property(long long int target, const char* pName, C3Vector& pState) const;
    int setQuaternionProperty(long long int target, const char* pName, const C4Vector& pState);
    int getQuaternionProperty(long long int target, const char* pName, C4Vector& pState) const;
    int setPoseProperty(long long int target, const char* pName, const C7Vector& pState);
    int getPoseProperty(long long int target, const char* pName, C7Vector& pState) const;
    int setColorProperty(long long int target, const char* pName, const float* pState);
    int getColorProperty(long long int target, const char* pName, float* pState) const;
    int setFloatArrayProperty(long long int target, const char* pName, const double* v, int vL);
    int getFloatArrayProperty(long long int target, const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty(long long int target, const char* pName, const int* v, int vL);
    int getIntArrayProperty(long long int target, const char* pName, std::vector<int>& pState) const;
    int removeProperty(long long int target, const char* pName);
    static int getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, CWorld* targetObject);
    static int getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt, CWorld* targetObject);

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
                                              std::vector<CScriptObject*>* loadedLuaScriptList) const;
    bool _canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void _setSuffix1ToSuffix2(int suffix1, int suffix2);
    static bool _getStackLocation_write(const char* ppName, int& ind, std::string& key, CInterfaceStack* stack);
    static bool _getStackLocation_read(const char* ppName, int& ind, std::string& key, int& arrIndex, CInterfaceStack* stack);
    static int _getPropertyTypeForStackItem(const CInterfaceStackObject* obj, std::string& str, bool firstCall = true);

    int _worldHandle;
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
    void renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase* renderingObject, int displayAttrib,
                                                             int windowSize[2], double verticalViewSizeOrAngle,
                                                             bool perspective);
    void renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase* renderingObject, int displayAttrib,
                                                            int windowSize[2], double verticalViewSizeOrAngle,
                                                            bool perspective);
    void renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase* renderingObject, int displayAttrib,
                                                              int windowSize[2], double verticalViewSizeOrAngle,
                                                              bool perspective);
#endif
};
