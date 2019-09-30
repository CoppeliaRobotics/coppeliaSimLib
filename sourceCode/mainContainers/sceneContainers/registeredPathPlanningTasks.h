
#pragma once

#include "pathPlanningTask_old.h"
#include "mainCont.h"

class CRegisteredPathPlanningTasks : public CMainCont 
{
public:
    CRegisteredPathPlanningTasks();
    virtual ~CRegisteredPathPlanningTasks();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    void announceObjectWillBeErased(int objID);
    void announceCollectionWillBeErased(int groupID);
    void setUpDefaultValues();
    void removeAllTasks();
    CPathPlanningTask* getObject(int objID);
    CPathPlanningTask* getObject(std::string objName);
    void addObject(CPathPlanningTask* aTask,bool objectIsACopy);
    void addObjectWithSuffixOffset(CPathPlanningTask* aTask,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    bool removeObject(int objID);

    int addTemporaryPathSearchObject(CPathPlanningTask* obj);
    CPathPlanningTask* getTemporaryPathSearchObject(int id);
    void removeTemporaryPathSearchObjectButDontDestroyIt(CPathPlanningTask* obj);
    int getTemporaryPathSearchObjectCount();

    // Variable that need to be serialized on an individual basis:
    std::vector<CPathPlanningTask*> allObjects;

private:
    std::vector<CPathPlanningTask*> _temporaryPathSearchObjects;
    std::vector<int> _temporaryPathSearchObjectIDs;
};
