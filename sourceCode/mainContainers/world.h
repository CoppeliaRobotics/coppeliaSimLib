#pragma once

#include "registeredPathPlanningTasks.h"
#include "environment.h"
#include "pageContainer.h"
#include "mainSettings.h"
#include "simulation.h"
#include "buttonBlockContainer.h"
#include "outsideCommandQueue.h"
#include "embeddedScriptContainer.h"
#include "customData.h"
#include "cacheCont.h"
#include "textureContainer.h"
#include "drawingContainer.h"
#include "pointCloudContainer_old.h"
#include "ghostObjectContainer.h"
#include "bannerContainer.h"
#include "dynamicsContainer.h"
#include "signalContainer.h"
#include "commTubeContainer.h"
#include "undoBufferCont.h"
#include "_world_.h"

struct SLoadOperationIssue
{
    int verbosity;
    std::string message;
    int objectHandle;
};

class CWorld : public _CWorld_
{
public:
    CWorld();
    virtual ~CWorld();

    void setEnableRemoteWorldsSync(bool enabled);
    void rebuildRemoteWorlds();
    void removeRemoteWorlds();

    void initializeWorld();
    void clearScene(bool notCalledFromUndoFunction);
    void deleteWorld();

    bool loadScene(CSer& ar,bool forUndoRedoOperation);
    void saveScene(CSer& ar);
    bool loadModel(CSer& ar,bool justLoadThumbnail,bool forceModelAsCopy,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize);

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
                                                    bool model,int fileSimVersion,bool forceModelAsCopy);

    void cleanupHashNames_allObjects(int suffix);

    void renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);

    void announceObjectWillBeErased(int objectHandle);
    void announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);

    void pushAllInitialEvents();


    // Old:
    void announceIkGroupWillBeErased(int ikGroupHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void announceCollisionWillBeErased(int collisionHandle);
    void announceDistanceWillBeErased(int distanceHandle);
    void announce2DElementWillBeErased(int elementID);
    void announce2DElementButtonWillBeErased(int elementID,int buttonID);

    void exportIkContent(CExtIkSer& ar);

    static void appendLoadOperationIssue(int verbosity,const char* text,int objectId);
    static int getLoadingMapping(const std::vector<int>* map,int oldVal);


    CUndoBufferCont* undoBufferContainer;
    CSignalContainer* signalContainer;
    CDynamicsContainer* dynamicsContainer;
    CEnvironment* environment;
    CPageContainer* pageContainer;
    CMainSettings* mainSettings;
    CEmbeddedScriptContainer* embeddedScriptContainer;
    CTextureContainer* textureContainer;
    CSimulation* simulation;
    CCustomData* customSceneData;
    CCustomData* customSceneData_tempData; // same as above, but not serialized!
    CCacheCont* cacheData;
    CDrawingContainer* drawingCont;

    // Old:
    CRegisteredPathPlanningTasks* pathPlanning;
    CPointCloudContainer_old* pointCloudCont;
    CGhostObjectContainer* ghostObjectCont;
    CCommTubeContainer* commTubeContainer;
    CBannerContainer* bannerCont;
    COutsideCommandQueue* outsideCommandQueue;
    CButtonBlockContainer* buttonBlockContainer;

private:
    bool _loadModelOrScene(CSer& ar,bool selectLoaded,bool isScene,bool justLoadThumbnail,bool forceModelAsCopy,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize);
    bool _loadSimpleXmlSceneOrModel(CSer& ar);
    bool _saveSimpleXmlScene(CSer& ar);

    void _simulationAboutToStart();
    void _simulationPaused();
    void _simulationAboutToResume();
    void _simulationAboutToStep();
    void _simulationAboutToEnd();
    void _simulationEnded();

    void _getMinAndMaxNameSuffixes(int& smallestSuffix,int& biggestSuffix) const;
    int _getSuffixOffsetForGeneralObjectToAdd(bool tempNames,std::vector<CSceneObject*>* loadedObjectList,
        std::vector<CCollection*>* loadedCollectionList,
        std::vector<CCollisionObject_old*>* loadedCollisionList,
        std::vector<CDistanceObject_old*>* loadedDistanceList,
        std::vector<CIkGroup_old*>* loadedIkGroupList,
        std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
        std::vector<CButtonBlock*>* loadedButtonBlockList,
        std::vector<CScriptObject*>* loadedLuaScriptList) const;
    bool _canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void _setSuffix1ToSuffix2(int suffix1,int suffix2);

    static void _prepareFastLoadingMapping(std::vector<int>& map);

    int _savedMouseMode;
    std::vector<int> _initialObjectUniqueIdentifiersForRemovingNewObjects;

    static std::vector<SLoadOperationIssue> _loadOperationIssues;
};
