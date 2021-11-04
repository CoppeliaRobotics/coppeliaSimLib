#include "simInternal.h"
#include "distanceObjectContainer_old.h"
#include "tt.h"
#include "distanceRoutines.h"
#include "app.h"
#include "gV.h"

CDistanceObjectContainer_old::CDistanceObjectContainer_old()
{
}

CDistanceObjectContainer_old::~CDistanceObjectContainer_old()
{ // beware, the current world could be nullptr
    while (_distanceObjects.size()!=0)
        _removeObject(_distanceObjects[0]->getObjectHandle());
}

void CDistanceObjectContainer_old::simulationAboutToStart()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationAboutToStart();
}

void CDistanceObjectContainer_old::simulationEnded()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationEnded();
}

void CDistanceObjectContainer_old::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(getObjectCount());i++)
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

bool CDistanceObjectContainer_old::canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
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

void CDistanceObjectContainer_old::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(getObjectCount());i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true));
            getObjectFromIndex(i)->setObjectName(tt::generateNewName_hash(name1.c_str(),suffix2+1).c_str(),false);
        }
    }
}

void CDistanceObjectContainer_old::addObject(CDistanceObject_old* newDistObj,bool objectIsACopy)
{
    addObjectWithSuffixOffset(newDistObj,objectIsACopy,1);
}

void CDistanceObjectContainer_old::addObjectWithSuffixOffset(CDistanceObject_old* newDistObj,bool objectIsACopy,int suffixOffset)
{
    std::string oName=newDistObj->getObjectName();
    if (oName.length()==0)
        oName="Distance";
    if (objectIsACopy)
        oName=tt::generateNewName_hash(oName.c_str(),suffixOffset);
    while (getObjectFromName(oName.c_str())!=nullptr)
        oName=tt::generateNewName_hashOrNoHash(oName.c_str(),objectIsACopy);
    newDistObj->setObjectName(oName.c_str(),false);

    int handle=SIM_IDSTART_DISTANCE;
    while (getObjectFromHandle(handle)!=nullptr)
        handle++;
    newDistObj->setObjectHandle(handle);

    _addObject(newDistObj);

    App::setFullDialogRefreshFlag();
}

int CDistanceObjectContainer_old::addNewObject(int entity1Handle,int entity2Handle,const char* objName)
{   // We check if the objects are valid:
    if (entity1Handle<SIM_IDSTART_COLLECTION)
    {
        if (App::currentWorld->sceneObjects->getObjectFromHandle(entity1Handle)==nullptr)
            return(-1);
    }
    else
    {
        if (App::currentWorld->collections->getObjectFromHandle(entity1Handle)==nullptr)
            return(-1);
    }
    if (entity2Handle>=SIM_IDSTART_COLLECTION)
    {
        if (App::currentWorld->collections->getObjectFromHandle(entity2Handle)==nullptr)
            return(-1);
    }
    else
    {
        if ( (App::currentWorld->sceneObjects->getObjectFromHandle(entity2Handle)==nullptr)&&(entity2Handle!=-1) )
            return(-1);
    }
    // We check if we try to measure an object against itself (forbidden, except for collections):
    if ( (entity1Handle<SIM_IDSTART_COLLECTION)&&(entity2Handle<SIM_IDSTART_COLLECTION) )
    {
        if (entity1Handle==entity2Handle)
            return(-1);
    }

    // We check if such an object already exists:
    for (int i=0;i<int(getObjectCount());i++)
    {
        if (getObjectFromIndex(i)->isSame(entity1Handle,entity2Handle))
            return(-1);
    }

    // Now check if the combination is valid:
    if ( (entity1Handle<SIM_IDSTART_COLLECTION)&&(entity2Handle<SIM_IDSTART_COLLECTION) )
    {
        int t1=App::currentWorld->sceneObjects->getObjectFromHandle(entity1Handle)->getObjectType();
        int t2=sim_object_octree_type;
        if (entity2Handle!=-1)
            t2=App::currentWorld->sceneObjects->getObjectFromHandle(entity2Handle)->getObjectType();
        if ( (t1==sim_object_shape_type)||(t1==sim_object_octree_type)||(t1==sim_object_pointcloud_type)||(t1==sim_object_dummy_type) )
        {
            if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type)&&(t2!=sim_object_pointcloud_type)&&(t2!=sim_object_dummy_type) )
                return(-1);
        }
        else
            return(-1);
    }

    CDistanceObject_old* newDistObject=new CDistanceObject_old(entity1Handle,entity2Handle);
    newDistObject->setObjectName(objName,false);
    addObject(newDistObject,false);

    return(newDistObject->getObjectHandle());
}

void CDistanceObjectContainer_old::removeObject(int objectHandle)
{
    App::currentWorld->announceDistanceWillBeErased(objectHandle);
    _removeObject(objectHandle);
}

void CDistanceObjectContainer_old::removeAllDistanceObjects()
{
    while (getObjectCount()!=0)
        removeObject(getObjectFromIndex(0)->getObjectHandle());
}

void CDistanceObjectContainer_old::resetAllDistances(bool exceptExplicitHandling)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        if ( (!getObjectFromIndex(i)->getExplicitHandling())||(!exceptExplicitHandling) )
            getObjectFromIndex(i)->clearDistanceResult();
    }
}

void CDistanceObjectContainer_old::announceCollectionWillBeErased(int collectionHandle)
{ // never called from the copy buffer
    size_t i=0;
    while (i<getObjectCount())
    {
        if (getObjectFromIndex(i)->announceCollectionWillBeErased(collectionHandle,false))
        { // We have to remove this distance object
            removeObject(getObjectFromIndex(i)->getObjectHandle()); // This will call announceDistanceWillBeErased!
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CDistanceObjectContainer_old::announceObjectWillBeErased(int objectHandle)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<getObjectCount())
    {
        if (getObjectFromIndex(i)->announceObjectWillBeErased(objectHandle,false))
        { // We have to remove this collision object
            removeObject(getObjectFromIndex(i)->getObjectHandle()); // This will call announceDistanceWillBeErased!
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CDistanceObjectContainer_old::setUpDefaultValues()
{
    removeAllDistanceObjects();
}

float CDistanceObjectContainer_old::handleAllDistances(bool exceptExplicitHandling)
{ // Return value is the smallest distance measured or negative if nothing was measured
    resetAllDistances(exceptExplicitHandling);
    if (!App::currentWorld->mainSettings->distanceCalculationEnabled)
        return(-1.0f);

    float retVal=SIM_MAX_FLOAT;
    bool retPos=false;
    for (size_t i=0;i<getObjectCount();i++)
    {
        if ( (!getObjectFromIndex(i)->getExplicitHandling())||(!exceptExplicitHandling) )
        {
            float d=getObjectFromIndex(i)->handleDistance();
            if (d>=0.0f)
            {
                retPos=true;
                if (d<retVal)
                    retVal=d;
            }
        }
    }
    if (!retPos)
        return(-1.0f);
    return(retVal);
}

void CDistanceObjectContainer_old::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
    if (displayAttrib&sim_displayattribute_renderpass)
        displayDistanceSegments();
}

void CDistanceObjectContainer_old::displayDistanceSegments()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->displayDistanceSegment();
}

void CDistanceObjectContainer_old::_addObject(CDistanceObject_old* newDistObj)
{
    _distanceObjects.push_back(newDistObj);
}

void CDistanceObjectContainer_old::_removeObject(int objectHandle)
{
    for (size_t i=0;i<_distanceObjects.size();i++)
    {
        if (_distanceObjects[i]->getObjectHandle()==objectHandle)
        {
            delete _distanceObjects[i];
            _distanceObjects.erase(_distanceObjects.begin()+i);
            break;
        }
    }
}

size_t CDistanceObjectContainer_old::getObjectCount() const
{
    return(_distanceObjects.size());
}

CDistanceObject_old* CDistanceObjectContainer_old::getObjectFromIndex(size_t index) const
{
    CDistanceObject_old* retVal=nullptr;
    if (index<_distanceObjects.size())
        retVal=_distanceObjects[index];
    return(retVal);
}

CDistanceObject_old* CDistanceObjectContainer_old::getObjectFromHandle(int objectHandle) const
{
    for (size_t i=0;i<_distanceObjects.size();i++)
    {
        if (_distanceObjects[i]->getObjectHandle()==objectHandle)
            return(_distanceObjects[i]);
    }
    return(nullptr);
}

CDistanceObject_old* CDistanceObjectContainer_old::getObjectFromName(const char* objName) const
{
    for (size_t i=0;i<_distanceObjects.size();i++)
    {
        if (_distanceObjects[i]->getObjectName().compare(objName)==0)
            return(_distanceObjects[i]);
    }
    return(nullptr);
}

