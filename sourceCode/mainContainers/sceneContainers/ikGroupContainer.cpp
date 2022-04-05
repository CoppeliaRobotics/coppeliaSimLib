#include "ikGroupContainer.h"
#include "app.h"
#include "tt.h"
#include "simInternal.h"
#include "vDateTime.h"

CIkGroupContainer::CIkGroupContainer()
{
}

CIkGroupContainer::~CIkGroupContainer()
{ // beware, the current world could be nullptr
}

void CIkGroupContainer::simulationAboutToStart()
{
     for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationAboutToStart();
     resetCalculationResults();
}

void CIkGroupContainer::simulationEnded()
{
     for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationEnded();
}

void CIkGroupContainer::addIkGroup(CIkGroup_old* anIkGroup,bool objectIsACopy)
{
    addIkGroupWithSuffixOffset(anIkGroup,objectIsACopy,1);
}

void CIkGroupContainer::addIkGroupWithSuffixOffset(CIkGroup_old* anIkGroup,bool objectIsACopy,int suffixOffset)
{
    std::string theName=anIkGroup->getObjectName();
    if (theName.length()==0)
        theName="IkGroup";
    if (objectIsACopy)
        theName=tt::generateNewName_hash(theName.c_str(),suffixOffset);
    while (getObjectFromName(theName.c_str())!=nullptr)
        theName=tt::generateNewName_hashOrNoHash(theName.c_str(),objectIsACopy);
    anIkGroup->setObjectName(theName.c_str(),false);

    int handle=SIM_IDSTART_IKGROUP;
    while (getObjectFromHandle(handle)!=nullptr)
        handle++;
    anIkGroup->setObjectHandle(handle);

    _addIkGroup(anIkGroup);
}

void CIkGroupContainer::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const
{
    minSuffix=-1;
    maxSuffix=-1;
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true);
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

bool CIkGroupContainer::canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true));
            for (size_t j=0;j<getObjectCount();j++)
            {
                int s2=tt::getNameSuffixNumber(getObjectFromIndex(j)->getObjectName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(getObjectFromIndex(j)->getObjectName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CIkGroupContainer::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true));
            getObjectFromIndex(i)->setObjectName(tt::generateNewName_hash(name1.c_str(),suffix2+1).c_str(),false);
        }
    }
}

void CIkGroupContainer::removeIkGroup(int ikGroupHandle)
{
    App::currentWorld->announceIkGroupWillBeErased(ikGroupHandle);
    _removeIkGroup(ikGroupHandle);
    App::setFullDialogRefreshFlag();
}

void CIkGroupContainer::removeAllIkGroups()
{
    while (getObjectCount()!=0)
        removeIkGroup(getObjectFromIndex(0)->getObjectHandle());
}

bool CIkGroupContainer::shiftIkGroup(int ikGroupHandle,bool shiftUp)
{ // Overridden
    bool retVal=_CIkGroupContainer_::shiftIkGroup(ikGroupHandle,shiftUp);
    // Now we have to check if the getDoOnFailOrSuccessOf is still consistent for all IKGroups:
    std::vector<int> seenIDs;
    for (size_t i=0;i<getObjectCount();i++)
    {
        CIkGroup_old* theGroup=getObjectFromIndex(i);
        int dep=theGroup->getDoOnFailOrSuccessOf();
        if (dep!=-1)
        {
            bool valid=false;
            for (size_t j=0;j<seenIDs.size();j++)
            {
                if (seenIDs[j]==dep)
                {
                    valid=true;
                    break;
                }
            }
            if (!valid)
                theGroup->setDoOnFailOrSuccessOf(-1,false);
        }
        seenIDs.push_back(theGroup->getObjectHandle());
    }
    return(retVal);
}

void CIkGroupContainer::announceObjectWillBeErased(int objHandle)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<getObjectCount())
    {
        if (getObjectFromIndex(i)->announceObjectWillBeErased(objHandle,false))
        { // This ik group has to be erased:
            removeIkGroup(getObjectFromIndex(i)->getObjectHandle()); // This will call announceIkGroupWillBeErased!
            i=0; // order may have changed!
        }
        else
            i++;
    }
}

void CIkGroupContainer::resetCalculationResults()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->resetCalculationResult();
}

void CIkGroupContainer::announceIkGroupWillBeErased(int ikGroupHandle)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<getObjectCount())
    {
        if (getObjectFromIndex(i)->announceIkGroupWillBeErased(ikGroupHandle,false))
        { // This ik group has to be erased (normally never happens)
            removeIkGroup(getObjectFromIndex(i)->getObjectHandle()); // This will call announceIkGroupWillBeErased!
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}

int CIkGroupContainer::computeAllIkGroups(bool exceptExplicitHandling)
{
    int performedCount=0;
    if (App::currentWorld->mainSettings->ikCalculationEnabled)
    {
        for (size_t i=0;i<getObjectCount();i++)
        {
            if ( (!exceptExplicitHandling)||(!getObjectFromIndex(i)->getExplicitHandling()) )
            {
                int stTime=(int)VDateTime::getTimeInMs();
                int res=0;
                res=getObjectFromIndex(i)->computeGroupIk(false);
                getObjectFromIndex(i)->setCalculationResult(res,VDateTime::getTimeDiffInMs(stTime));
                if (res!=sim_ikresult_not_performed)
                    performedCount++;
            }
        }
    }
    return(performedCount);
}

void CIkGroupContainer::_addIkGroup(CIkGroup_old* anIkGroup)
{ // Overridden from _CIkGroupContainer_
    _CIkGroupContainer_::_addIkGroup(anIkGroup);

    if (anIkGroup->setObjectCanSync(true))
        anIkGroup->buildUpdateAndPopulateSynchronizationObject(nullptr);
}

void CIkGroupContainer::_removeIkGroup(int ikGroupHandle)
{ // Overridden from _CIkGroupContainer_
    CIkGroup_old* ig=getObjectFromHandle(ikGroupHandle);
    if (ig!=nullptr)
        ig->removeSynchronizationObject(false);

    _CIkGroupContainer_::_removeIkGroup(ikGroupHandle);
}

void CIkGroupContainer::buildUpdateAndPopulateSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CIkGroup_old* it=getObjectFromIndex(i);
        it->buildUpdateAndPopulateSynchronizationObject(nullptr);
    }
}

void CIkGroupContainer::connectSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CIkGroup_old* it=getObjectFromIndex(i);
        it->connectSynchronizationObject();
    }
}

void CIkGroupContainer::removeSynchronizationObjects(bool localReferencesToItOnly)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CIkGroup_old* it=getObjectFromIndex(i);
        it->removeSynchronizationObject(localReferencesToItOnly);
    }
}
