#pragma once

#include <registeredPathPlanningTasks.h>
#include <environment.h>
#include <pageContainer.h>
#include <mainSettings.h>
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
#include <signalContainer.h>
#include <commTubeContainer.h>
#include <undoBufferCont.h>
#include <collectionContainer.h>
#include <distanceObjectContainer_old.h>
#include <collisionObjectContainer_old.h>
#include <ikGroupContainer.h>
#include <sceneObjectContainer.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propScene_sceneUid,                "sceneUid",                     sim_propertytype_int,       1) \
    FUNCX(propScene_visibilityLayers,        "visibilityLayers",             sim_propertytype_int,       0) \

#define FUNCX(name, str, v1, v2) const CProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<CProperty> allProps_scene = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

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

    bool loadScene(CSer &ar, bool forUndoRedoOperation);
    void saveScene(CSer &ar);
    bool loadModel(CSer &ar, bool justLoadThumbnail, bool forceModelAsCopy, C7Vector *optionalModelTr,
                   C3Vector *optionalModelBoundingBoxSize, double *optionalModelNonDefaultTranslationStepSize);

    void simulationAboutToStart();
    void simulationPaused();
    void simulationAboutToResume();
    void simulationAboutToStep();
    void simulationAboutToEnd();
    void simulationEnded(bool removeNewObjects);

    void addGeneralObjectsToWorldAndPerformMappings(std::vector<CSceneObject *> *loadedObjectList,
                                                    std::vector<CCollection *> *loadedCollectionList,
                                                    std::vector<CCollisionObject_old *> *loadedCollisionList,
                                                    std::vector<CDistanceObject_old *> *loadedDistanceList,
                                                    std::vector<CIkGroup_old *> *loadedIkGroupList,
                                                    std::vector<CPathPlanningTask *> *loadedPathPlanningTaskList,
                                                    std::vector<CButtonBlock *> *loadedButtonBlockList,
                                                    std::vector<CScriptObject *> *loadedLuaScriptList,
                                                    std::vector<CTextureObject *> &loadedTextureObjectList,
                                                    std::vector<CDynMaterialObject *> &loadedDynMaterialObjectList,
                                                    bool model, int fileSimVersion, bool forceModelAsCopy);

    void cleanupHashNames_allObjects(int suffix);

    void announceObjectWillBeErased(const CSceneObject *object);
    void announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);

    CScriptObject *getScriptObjectFromHandle(int scriptHandle) const;
    CScriptObject *getScriptObjectFromUid(int uid) const;
    void callScripts(int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch = nullptr, int scriptToExclude = -1);

    void pushGenesisEvents();

    int setBoolProperty(int target, const char* pName, bool pState);
    int getBoolProperty(int target, const char* pName, bool& pState);
    int setIntProperty(int target, const char* pName, int pState);
    int getIntProperty(int target, const char* pName, int& pState);
    int setFloatProperty(int target, const char* pName, double pState);
    int getFloatProperty(int target, const char* pName, double& pState);
    int setStringProperty(int target, const char* pName, const char* pState);
    int getStringProperty(int target, const char* pName, std::string& pState);
    int setBufferProperty(int target, const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(int target, const char* pName, std::string& pState);
    int setVector3Property(int target, const char* pName, const C3Vector& pState);
    int getVector3Property(int target, const char* pName, C3Vector& pState);
    int setQuaternionProperty(int target, const char* pName, const C4Vector& pState);
    int getQuaternionProperty(int target, const char* pName, C4Vector& pState);
    int setPoseProperty(int target, const char* pName, const C7Vector& pState);
    int getPoseProperty(int target, const char* pName, C7Vector& pState);
    int setMatrix3x3Property(int target, const char* pName, const C3X3Matrix& pState);
    int getMatrix3x3Property(int target, const char* pName, C3X3Matrix& pState);
    int setMatrix4x4Property(int target, const char* pName, const C4X4Matrix& pState);
    int getMatrix4x4Property(int target, const char* pName, C4X4Matrix& pState);
    int setColorProperty(int target, const char* pName, const float* pState);
    int getColorProperty(int target, const char* pName, float* pState);
    int setVectorProperty(int target, const char* pName, const double* v, int vL);
    int getVectorProperty(int target, const char* pName, std::vector<double>& pState);
    int setIntVectorProperty(int target, const char* pName, const int* v, int vL);
    int getIntVectorProperty(int target, const char* pName, std::vector<int>& pState);
    int removeProperty(int target, const char* pName);
    int getPropertyName(int target, int& index, std::string& pName);
    int getPropertyInfo(int target, const char* pName, int& info, int& size);

    // Old:
    void announceIkGroupWillBeErased(int ikGroupHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void announceCollisionWillBeErased(int collisionHandle);
    void announceDistanceWillBeErased(int distanceHandle);
    void announce2DElementWillBeErased(int elementID);
    void announce2DElementButtonWillBeErased(int elementID, int buttonID);

    static void appendLoadOperationIssue(int verbosity, const char *text, int objectId);
    static int getLoadingMapping(const std::map<int, int> *map, int oldVal);

    CUndoBufferCont *undoBufferContainer;
    CSignalContainer *signalContainer;
    CDynamicsContainer *dynamicsContainer;
    CEnvironment *environment;
    CPageContainer *pageContainer;
    CMainSettings *mainSettings;
    CTextureContainer *textureContainer;
    CSimulation *simulation;
    CCustomData customSceneData;
    CCustomData customSceneData_tempData; // same as above, but not serialized!
    CCustomData_old *customSceneData_old;
    CCacheCont *cacheData;
    CDrawingContainer *drawingCont;
    CCollectionContainer *collections;
    CSceneObjectContainer *sceneObjects;

    // Old:
    CDistanceObjectContainer_old *distances;
    CCollisionObjectContainer_old *collisions;
    CIkGroupContainer *ikGroups;
    CRegisteredPathPlanningTasks *pathPlanning;
    CPointCloudContainer_old *pointCloudCont;
    CGhostObjectContainer *ghostObjectCont;
    CCommTubeContainer *commTubeContainer;
    CBannerContainer *bannerCont;
    COutsideCommandQueue *outsideCommandQueue;
    CButtonBlockContainer *buttonBlockContainer;

  private:
    bool _loadModelOrScene(CSer &ar, bool selectLoaded, bool isScene, bool justLoadThumbnail, bool forceModelAsCopy,
                           C7Vector *optionalModelTr, C3Vector *optionalModelBoundingBoxSize,
                           double *optionalModelNonDefaultTranslationStepSize);
    bool _loadSimpleXmlSceneOrModel(CSer &ar);
    bool _saveSimpleXmlScene(CSer &ar);

    void _getMinAndMaxNameSuffixes(int &smallestSuffix, int &biggestSuffix) const;
    int _getSuffixOffsetForGeneralObjectToAdd(bool tempNames, std::vector<CSceneObject *> *loadedObjectList,
                                              std::vector<CCollection *> *loadedCollectionList,
                                              std::vector<CCollisionObject_old *> *loadedCollisionList,
                                              std::vector<CDistanceObject_old *> *loadedDistanceList,
                                              std::vector<CIkGroup_old *> *loadedIkGroupList,
                                              std::vector<CPathPlanningTask *> *loadedPathPlanningTaskList,
                                              std::vector<CButtonBlock *> *loadedButtonBlockList,
                                              std::vector<CScriptObject *> *loadedLuaScriptList) const;
    bool _canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void _setSuffix1ToSuffix2(int suffix1, int suffix2);

    int _worldHandle;
    int _savedMouseMode;
    std::vector<long long int> _initialObjectUniqueIdentifiersForRemovingNewObjects;

    static std::vector<SLoadOperationIssue> _loadOperationIssues;

#ifdef SIM_WITH_GUI
  public:
    void renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase *renderingObject, int displayAttrib,
                                                             int windowSize[2], double verticalViewSizeOrAngle,
                                                             bool perspective);
    void renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase *renderingObject, int displayAttrib,
                                                            int windowSize[2], double verticalViewSizeOrAngle,
                                                            bool perspective);
    void renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase *renderingObject, int displayAttrib,
                                                              int windowSize[2], double verticalViewSizeOrAngle,
                                                              bool perspective);
#endif
};
