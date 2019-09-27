
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "registeredMotionPlanningTasks.h"
#include "tt.h"
#include "objCont.h"
#include "app.h"

CRegisteredMotionPlanningTasks::CRegisteredMotionPlanningTasks()
{
    setUpDefaultValues();
}

CRegisteredMotionPlanningTasks::~CRegisteredMotionPlanningTasks()
{
    removeAllTasks();
}

void CRegisteredMotionPlanningTasks::simulationAboutToStart()
{
    for (int i=0;i<int(allObjects.size());i++)
        allObjects[i]->simulationAboutToStart();
}

void CRegisteredMotionPlanningTasks::simulationEnded()
{
    for (int i=0;i<int(allObjects.size());i++)
        allObjects[i]->simulationEnded();
}

void CRegisteredMotionPlanningTasks::announceObjectWillBeErased(int objID)
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
void CRegisteredMotionPlanningTasks::announceCollectionWillBeErased(int groupID)
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
void CRegisteredMotionPlanningTasks::announceIkGroupWillBeErased(int ikGroupID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(allObjects.size()))
    {
        if (allObjects[i]->announceIkGroupWillBeErased(ikGroupID,false))
        {
            removeObject(allObjects[i]->getObjectID()); // This will call announceMotionPlanningTaskWillBeErased!
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}

void CRegisteredMotionPlanningTasks::setUpDefaultValues()
{
    removeAllTasks();
}
void CRegisteredMotionPlanningTasks::removeAllTasks()
{
    while (allObjects.size()!=0)
        removeObject(allObjects[0]->getObjectID());
}
CMotionPlanningTask* CRegisteredMotionPlanningTasks::getObject(int objID)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->getObjectID()==objID)
            return(allObjects[i]);
    }
    return(nullptr);
}
CMotionPlanningTask* CRegisteredMotionPlanningTasks::getObject(std::string objName)
{
    for (int i=0;i<int(allObjects.size());i++)
    {
        if (allObjects[i]->getObjectName().compare(objName)==0)
            return(allObjects[i]);
    }
    return(nullptr);
}

void CRegisteredMotionPlanningTasks::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
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

bool CRegisteredMotionPlanningTasks::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
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

void CRegisteredMotionPlanningTasks::setSuffix1ToSuffix2(int suffix1,int suffix2)
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

void CRegisteredMotionPlanningTasks::addObject(CMotionPlanningTask* aTask,bool objectIsACopy)
{
    addObjectWithSuffixOffset(aTask,objectIsACopy,1);
}

void CRegisteredMotionPlanningTasks::addObjectWithSuffixOffset(CMotionPlanningTask* aTask,bool objectIsACopy,int suffixOffset)
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

    int objID=SIM_IDSTART_MOTIONPLANNINGTASK;
    while (getObject(objID)!=nullptr)
        objID++;
    aTask->setObjectID(objID);
    allObjects.push_back(aTask);
    App::setFullDialogRefreshFlag();
}

bool CRegisteredMotionPlanningTasks::removeObject(int objID)
{
    for (size_t i=0;i<allObjects.size();i++)
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

void CRegisteredMotionPlanningTasks::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
    if (displayAttrib&sim_displayattribute_renderpass)
    {
        for (size_t i=0;i<allObjects.size();i++)
            allObjects[i]->renderYour3DStuff();
    }
}
