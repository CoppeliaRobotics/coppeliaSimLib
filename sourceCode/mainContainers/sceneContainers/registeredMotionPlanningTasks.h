
#pragma once

#include "vrepMainHeader.h"
#include "motionPlanningTask_old.h"
#include "mainCont.h"

class CRegisteredMotionPlanningTasks : public CMainCont 
{
public:
    CRegisteredMotionPlanningTasks();
    virtual ~CRegisteredMotionPlanningTasks();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    void announceObjectWillBeErased(int objID);
    void announceCollectionWillBeErased(int groupID);
    void announceIkGroupWillBeErased(int ikGroupID);
    void setUpDefaultValues();
    void removeAllTasks();
    CMotionPlanningTask* getObject(int objID);
    CMotionPlanningTask* getObject(std::string objName);
    void addObject(CMotionPlanningTask* aTask,bool objectIsACopy);
    void addObjectWithSuffixOffset(CMotionPlanningTask* aTask,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    bool removeObject(int objID);


    // Variable that need to be serialized on an individual basis:
    std::vector<CMotionPlanningTask*> allObjects;
};
