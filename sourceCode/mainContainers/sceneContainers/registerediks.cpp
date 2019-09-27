
#include "vrepMainHeader.h"
#include "registerediks.h"
#include "ikRoutine.h"
#include "app.h"
#include "tt.h"
#include "v_rep_internal.h"
#include "vDateTime.h"

CRegisterediks::CRegisterediks()
{
}

CRegisterediks::~CRegisterediks()
{
    removeAllIkGroups();
}

void CRegisterediks::simulationAboutToStart()
{
     for (int i=0;i<int(ikGroups.size());i++)
        ikGroups[i]->simulationAboutToStart();
     resetCalculationResults();
}

void CRegisterediks::simulationEnded()
{
     for (int i=0;i<int(ikGroups.size());i++)
        ikGroups[i]->simulationEnded();
}

void CRegisterediks::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

void CRegisterediks::addIkGroup(CikGroup* anIkGroup,bool objectIsACopy)
{
    addIkGroupWithSuffixOffset(anIkGroup,objectIsACopy,1);
}

void CRegisterediks::addIkGroupWithSuffixOffset(CikGroup* anIkGroup,bool objectIsACopy,int suffixOffset)
{ // Be careful! We don't check if the group is valid!!
    // We look for a free id:
    int newID=SIM_IDSTART_IKGROUP;
    while (getIkGroup(newID)!=nullptr)
        newID++;
    anIkGroup->setObjectID(newID);
    // We make a unique name:
    std::string theName=anIkGroup->getObjectName();
    if (objectIsACopy)
        theName=tt::generateNewName_dash(theName,suffixOffset);
    else
    {
        while (getIkGroup(theName)!=nullptr)
            theName=tt::generateNewName_noDash(theName);
    }
    anIkGroup->setObjectName(theName);
    ikGroups.push_back(anIkGroup);
}

void CRegisterediks::announceCollectionWillBeErased(int groupID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(ikGroups.size()))
    {
        if (ikGroups[i]->announceCollectionWillBeErased(groupID,false))
        { // This case should never happen (at least for now!
            removeIkGroup(ikGroups[i]->getObjectID()); // This will call announceIkGroupWillBeErased!
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}

void CRegisterediks::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(ikGroups.size());i++)
    {
        int s=tt::getNameSuffixNumber(ikGroups[i]->getObjectName().c_str(),true);
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

bool CRegisterediks::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(ikGroups.size());i++)
    {
        int s1=tt::getNameSuffixNumber(ikGroups[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(ikGroups[i]->getObjectName().c_str(),true));
            for (int j=0;j<int(ikGroups.size());j++)
            {
                int s2=tt::getNameSuffixNumber(ikGroups[j]->getObjectName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(ikGroups[j]->getObjectName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CRegisterediks::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(ikGroups.size());i++)
    {
        int s1=tt::getNameSuffixNumber(ikGroups[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(ikGroups[i]->getObjectName().c_str(),true));
            ikGroups[i]->setObjectName(tt::generateNewName_dash(name1,suffix2+1));
        }
    }
}

CikGroup* CRegisterediks::getIkGroup(int groupID)
{
     for (int i=0;i<int(ikGroups.size());i++)
     {
        if (ikGroups[i]->getObjectID()==groupID)
            return(ikGroups[i]);
     }
     return(nullptr);
}

CikGroup* CRegisterediks::getIkGroup(std::string groupName)
{
    for (int i=0;i<int(ikGroups.size());i++)
    {
        if (ikGroups[i]->getObjectName().compare(groupName)==0)
            return(ikGroups[i]);
    }
    return(nullptr);
}

void CRegisterediks::removeIkGroup(int ikGroupID)
{
    App::ct->objCont->announceIkGroupWillBeErased(ikGroupID);
    for (size_t i=0;i<ikGroups.size();i++)
    {
        if (ikGroups[i]->getObjectID()==ikGroupID)
        {
            delete ikGroups[i];
            ikGroups.erase(ikGroups.begin()+i);
            App::setFullDialogRefreshFlag();
            return;
        }
    }
}

void CRegisterediks::removeAllIkGroups()
{
    while (ikGroups.size()!=0)
        removeIkGroup(ikGroups[0]->getObjectID());
}

void CRegisterediks::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(ikGroups.size()))
    {
        if (ikGroups[i]->announceObjectWillBeErased(objID,false))
        { // This ik group has to be erased:
            removeIkGroup(ikGroups[i]->getObjectID()); // This will call announceIkGroupWillBeErased!
            i=0; // order may have changed!
        }
        else
            i++;
    }
}

void CRegisterediks::resetCalculationResults()
{
    for (int i=0;i<int(ikGroups.size());i++)
        ikGroups[i]->resetCalculationResult();
}

void CRegisterediks::announceIkGroupWillBeErased(int ikGroupID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(ikGroups.size()))
    {
        if (ikGroups[i]->announceIkGroupWillBeErased(ikGroupID,false))
        { // This ik group has to be erased (normally never happens)
            removeIkGroup(ikGroups[i]->getObjectID()); // This will call announceIkGroupWillBeErased!
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}

int CRegisterediks::computeAllIkGroups(bool exceptExplicitHandling)
{
    int performedCount=0;
    if (App::ct->mainSettings->ikCalculationEnabled)
    {
        for (int i=0;i<int(ikGroups.size());i++)
        {
            if ((!exceptExplicitHandling)||(!ikGroups[i]->getExplicitHandling()))
            {
                int stTime=VDateTime::getTimeInMs();
                int res=0;
                res=ikGroups[i]->computeGroupIk(false);
                ikGroups[i]->setCalculationResult(res,VDateTime::getTimeDiffInMs(stTime));
                if (res!=sim_ikresult_not_performed)
                    performedCount++;
            }
        }
    }
    return(performedCount);
}
