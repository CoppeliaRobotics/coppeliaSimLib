
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "registeredPathPlanningTasks.h"
#include "tt.h"
#include "objCont.h"
#include "app.h"

CRegisteredPathPlanningTasks::CRegisteredPathPlanningTasks()
{
    setUpDefaultValues();
}

CRegisteredPathPlanningTasks::~CRegisteredPathPlanningTasks()
{
    removeAllTasks();

    // Following should not be needed!
    for (int i=0;i<int(_temporaryPathSearchObjects.size());i++)
        delete _temporaryPathSearchObjects[i];
    _temporaryPathSearchObjects.clear();
    _temporaryPathSearchObjectIDs.clear();
}

void CRegisteredPathPlanningTasks::simulationAboutToStart()
{
    for (int i=0;i<int(allObjects.size());i++)
        allObjects[i]->simulationAboutToStart();
}

void CRegisteredPathPlanningTasks::simulationEnded()
{
    for (int i=0;i<int(allObjects.size());i++)
        allObjects[i]->simulationEnded();
    // Remove any temporary path search object left over:
    for (int i=0;i<int(_temporaryPathSearchObjects.size());i++)
        delete _temporaryPathSearchObjects[i];
    _temporaryPathSearchObjects.clear();
    _temporaryPathSearchObjectIDs.clear();
}

int CRegisteredPathPlanningTasks::getTemporaryPathSearchObjectCount()
{
    return((int)_temporaryPathSearchObjects.size());
}

int CRegisteredPathPlanningTasks::addTemporaryPathSearchObject(CPathPlanningTask* obj)
{
    int id=0;
    while (getTemporaryPathSearchObject(id)!=nullptr)
        id++;
    _temporaryPathSearchObjects.push_back(obj);
    _temporaryPathSearchObjectIDs.push_back(id);
    return(id);
}

CPathPlanningTask* CRegisteredPathPlanningTasks::getTemporaryPathSearchObject(int id)
{
    for (int i=0;i<int(_temporaryPathSearchObjectIDs.size());i++)
    {
        if (_temporaryPathSearchObjectIDs[i]==id)
            return(_temporaryPathSearchObjects[i]);
    }
    return(nullptr);
}

void CRegisteredPathPlanningTasks::removeTemporaryPathSearchObjectButDontDestroyIt(CPathPlanningTask* obj)
{
    for (int i=0;i<int(_temporaryPathSearchObjects.size());i++)
    {
        if (_temporaryPathSearchObjects[i]==obj)
        {
            _temporaryPathSearchObjects.erase(_temporaryPathSearchObjects.begin()+i);
            _temporaryPathSearchObjectIDs.erase(_temporaryPathSearchObjectIDs.begin()+i);
            break;
        }
    }
}

void CRegisteredPathPlanningTasks::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(allObjects.size()))
    {
        if (allObjects[i]->announceObjectWillBeErased(objID,false))
        {
            removeObject(allObjects[i]->getObjectID());
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}
void CRegisteredPathPlanningTasks::announceCollectionWillBeErased(int groupID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(allObjects.size()))
    {
        if (allObjects[i]->announceCollectionWillBeErased(groupID,false))
        {
            removeObject(allObjects[i]->getObjectID()); // This will call announcePathPlanningTaskWillBeErased!
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}

void CRegisteredPathPlanningTasks::setUpDefaultValues()
{
    removeAllTasks();
}
void CRegisteredPathPlanningTasks::removeAllTasks()
{
    while (allObjects.size()!=0)
        removeObject(allObjects[0]->getObjectID());
}
CPathPlanningTask* CRegisteredPathPlanningTasks::getObject(int objID)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->getObjectID()==objID)
            return(allObjects[i]);
    }
    return(nullptr);
}
CPathPlanningTask* CRegisteredPathPlanningTasks::getObject(std::string objName)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->getObjectName().compare(objName)==0)
            return(allObjects[i]);
    }
    return(nullptr);
}

void CRegisteredPathPlanningTasks::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(allObjects.size());i++)
    {
        int s=tt::getNameSuffixNumber(allObjects[i]->getObjectName().c_str(),true);
        if (i==0)
        {
            minSuffix=s;
            maxSuffix=s;
        }
        else
        {
            if (s<minSuffix)
                minSuffix=s;
            if (s>maxSuffix)
                maxSuffix=s;
        }
    }
}

bool CRegisteredPathPlanningTasks::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(allObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(allObjects[i]->getObjectName().c_str(),true));
            for (int j=0;j<int(allObjects.size());j++)
            {
                int s2=tt::getNameSuffixNumber(allObjects[j]->getObjectName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(allObjects[j]->getObjectName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CRegisteredPathPlanningTasks::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(allObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(allObjects[i]->getObjectName().c_str(),true));
            allObjects[i]->setObjectName(tt::generateNewName_dash(name1,suffix2+1));
        }
    }
}

void CRegisteredPathPlanningTasks::addObject(CPathPlanningTask* aTask,bool objectIsACopy)
{
    addObjectWithSuffixOffset(aTask,objectIsACopy,1);
}

void CRegisteredPathPlanningTasks::addObjectWithSuffixOffset(CPathPlanningTask* aTask,bool objectIsACopy,int suffixOffset)
{
    // Does that name already exist?
    std::string theName=aTask->getObjectName();
    if (objectIsACopy)
        theName=tt::generateNewName_dash(theName,suffixOffset);
    else
    {
        while (getObject(theName)!=nullptr)
            theName=tt::generateNewName_noDash(theName);
    }
    aTask->setObjectName(theName);

    int objID=SIM_IDSTART_PATHPLANNINGTASK;
    while (getObject(objID)!=nullptr)
        objID++;
    aTask->setObjectID(objID);
    allObjects.push_back(aTask);
    App::setFullDialogRefreshFlag();
}
bool CRegisteredPathPlanningTasks::removeObject(int objID)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->getObjectID()==objID)
        {
            delete allObjects[i];
            allObjects.erase(allObjects.begin()+i);
            App::setFullDialogRefreshFlag();
            return(true);
        }
    }
    return(false);
}

void CRegisteredPathPlanningTasks::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
    if (displayAttrib&sim_displayattribute_renderpass)
    {
        for (size_t i=0;i<allObjects.size();i++)
            allObjects[i]->renderYour3DStuff();
    }
}
