
#pragma once

#include "luaCustomFuncAndVarContainer.h"
#include "registeredCollections.h"
#include "registeredDistances.h"
#include "registeredCollisions.h"
#include "registeredPathPlanningTasks.h"
#include "registeredMotionPlanningTasks.h"
#include "copyBuffer.h"
#include "environment.h"
#include "pageContainer.h"
#include "mainSettings.h"
#include "memorizedConfContainer.h"
#include "simulation.h"
#include "buttonBlockContainer.h"
#include "outsideCommandQueue.h"
#include "luaScriptContainer.h"
#include "addOnScriptContainer.h"
#include "customData.h"
#include "cacheCont.h"
#include "constraintSolverContainer.h"
#include "textureContainer.h"
#include "drawingContainer.h"
#include "pointCloudContainer_old.h"
#include "ghostObjectContainer.h"
#include "bannerContainer.h"
#include "dynamicsContainer.h"
#include "signalContainer.h"
#include "commTubeContainer.h"
#include "persistentDataContainer.h"
#include "interfaceStackContainer.h"
#include "simulatorMessageQueue.h"
#include "registerediks.h"
#include "objCont.h"
#include "calculationInfo.h"
#include "undoBufferCont.h"

#ifdef SIM_WITH_GUI
    #include "globalGuiTextureContainer.h"
#endif

#ifdef SIM_WITH_SERIAL
    #include "serialPortContainer.h"
#endif

class CMainContainer
{
public:
    CMainContainer();
    virtual ~CMainContainer();

    void initialize();
    void deinitialize();
    void renderYourGeneralObject3DStuff_beforeRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void renderYourGeneralObject3DStuff_afterRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void renderYourGeneralObject3DStuff_onTopOfRegularObjects(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);


    void simulationAboutToStart();
    void simulationPaused();
    void simulationAboutToResume();
    void simulationAboutToStep();
    void simulationAboutToEnd();
    void simulationEnded(bool removeNewObjects);

    void setModificationFlag(int bitMask);
    int getModificationFlags(bool clearTheFlagsAfter);

    void emptyScene(bool notCalledFromUndoFunction);
    int getInstanceCount();

    int createNewInstance();
    bool makeInstanceCurrentFromIndex(int instanceIndex,bool previousInstanceWasDestroyed);
    int destroyCurrentInstance();
    int getCurrentInstanceIndex();

    bool isInstanceSwitchingLocked();
    void getAllSceneNames(std::vector<std::string>& l);
    bool setInstanceIndex(int index);


    CCopyBuffer* copyBuffer; // We have only one copy buffer!!
    CPersistentDataContainer* persistentDataContainer; // We have only one such object!!
    CSimulatorMessageQueue* simulatorMessageQueue; // We have only one such object!!
    CCalculationInfo* calcInfo; // We have only one such object!!
    CInterfaceStackContainer* interfaceStackContainer; // We have only one such object!!
    CLuaCustomFuncAndVarContainer* luaCustomFuncAndVarContainer; // We have only one such object!!
    CCustomData* customAppData; // We have only one such object!!
    CAddOnScriptContainer* addOnScriptContainer; // We have only one such object!!
    CLuaScriptObject* sandboxScript; // We have only one such object


    CUndoBufferCont* undoBufferContainer;
    CCommTubeContainer* commTubeContainer;
    CSignalContainer* signalContainer;
    CDynamicsContainer* dynamicsContainer;
    COutsideCommandQueue* outsideCommandQueue;
    CButtonBlockContainer* buttonBlockContainer;
    CRegisteredCollections* collections;
    CRegisteredDistances* distances;
    CRegisteredCollisions* collisions;
    CEnvironment* environment;
    CPageContainer* pageContainer;
    CMainSettings* mainSettings;
    CRegisteredPathPlanningTasks* pathPlanning;
    CRegisteredMotionPlanningTasks* motionPlanning;
    CLuaScriptContainer* luaScriptContainer;
    CTextureContainer* textureCont;
    CMemorizedConfContainer* confContainer;
    CSimulation* simulation;
    CCustomData* customSceneData;
    CCustomData* customSceneData_tempData; // same as above, but not serialized!
    CCacheCont* cacheData;
    CConstraintSolverContainer* constraintSolver;
    CDrawingContainer* drawingCont;
    CPointCloudContainer_old* pointCloudCont;
    CGhostObjectContainer* ghostObjectCont;
    CBannerContainer* bannerCont;
    CRegisterediks* ikGroups;
    CObjCont* objCont;

private:

    int currentInstanceIndex;
    std::vector<int> _initialObjectUniqueIdentifiersForRemovingNewObjects;
    int _savedMouseMode;

    std::vector<int> _uniqueIdsOfSelectionSinceLastTimeGetAndClearModificationFlagsWasCalled;
    int _modificationFlags;
    // +bit 0: object(s) erased
    // +bit 1: object(s) created
    // +bit 2: model loaded
    // +bit 3: scene loaded
    // +bit 4: undo called
    // +bit 5: redo called
    // +bit 6: instance switched
    // +bit 7: edit mode active         --> is set when getAndClearModificationFlags is called
    // +bit 8: object(s) scaled
    // +bit 9: selection state changed  --> is set when getAndClearModificationFlags is called
    // +bit 10: key pressed
    // +bit 11: simulation started
    // +bit 12: simulation ended

    std::vector<CUndoBufferCont*> _undoBufferContainerList;
    std::vector<CCommTubeContainer*> _commTubeContainerList;
    std::vector<CSignalContainer*> _signalContainerList;
    std::vector<CDynamicsContainer*> _dynamicsContainerList;
    std::vector<COutsideCommandQueue*> _outsideCommandQueueList;
    std::vector<CButtonBlockContainer*> _buttonBlockContainerList;
    std::vector<CRegisteredCollections*> _collectionList;
    std::vector<CRegisteredDistances*> _distanceList;
    std::vector<CRegisteredCollisions*> _collisionList;
    std::vector<CEnvironment*> _environmentList;
    std::vector<CPageContainer*> _pageContainerList;
    std::vector<CMainSettings*> _mainSettingsList;
    std::vector<CRegisteredPathPlanningTasks*> _pathPlanningList;
    std::vector<CRegisteredMotionPlanningTasks*> _motionPlanningList;
    std::vector<CLuaScriptContainer*> _luaScriptContainerList;
    std::vector<CTextureContainer*> _textureContList;
    std::vector<CMemorizedConfContainer*> _confContainerList;
    std::vector<CSimulation*> _simulationList;
    std::vector<CCustomData*> _customSceneDataList;
    std::vector<CCustomData*> _customSceneData_tempDataList;
    std::vector<CCacheCont*> _cacheDataList;
    std::vector<CConstraintSolverContainer*> _constraintSolverList;
    std::vector<CDrawingContainer*> _drawingContainerList;
    std::vector<CPointCloudContainer_old*> _pointCloudContainerList;
    std::vector<CGhostObjectContainer*> _ghostObjectContainerList;
    std::vector<CBannerContainer*> _bannerContainerList;
    std::vector<CRegisterediks*> _ikGroupList;
    std::vector<CObjCont*> _objContList;


#ifdef SIM_WITH_GUI
public:
    void keyPress(int key);
    void addMenu(VMenu* menu);
    int getInstanceIndexOfASceneNotYetSaved(bool doNotIncludeCurrentScene);
    void setInstanceIndexWithThumbnails(int index);
    bool processGuiCommand(int commandID);

    CGlobalGuiTextureContainer* globalGuiTextureCont; // We have only one such object!!

private:
#endif

#ifdef SIM_WITH_SERIAL
public:
    CSerialPortContainer* serialPortContainer; // We have only one such object!!
#endif
};
