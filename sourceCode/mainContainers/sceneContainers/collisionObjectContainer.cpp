#include "simInternal.h"
#include "collisionObjectContainer.h"
#include "tt.h"
#include "collisionRoutines.h"
#include "app.h"

CCollisionObjectContainer::CCollisionObjectContainer()
{
}

CCollisionObjectContainer::~CCollisionObjectContainer()
{ // beware, the current world could be nullptr
}

void CCollisionObjectContainer::simulationAboutToStart()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationAboutToStart();
}

void CCollisionObjectContainer::simulationEnded()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationEnded();
}

void CCollisionObjectContainer::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const
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

bool CCollisionObjectContainer::canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
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

void CCollisionObjectContainer::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName().c_str(),true));
            getObjectFromIndex(i)->setObjectName(tt::generateNewName_hash(name1,suffix2+1).c_str(),false);
        }
    }
}

void CCollisionObjectContainer::addObject(CCollisionObject* newCollObj,bool objectIsACopy)
{
    addObjectWithSuffixOffset(newCollObj,objectIsACopy,1);
}

void CCollisionObjectContainer::addObjectWithSuffixOffset(CCollisionObject* newCollObj,bool objectIsACopy,int suffixOffset)
{   // Here we don't check whether such an object already exists or is valid.
    // This routine is mainly used for loading and copying operations
    // We check if that name already exists:
    std::string oName=newCollObj->getObjectName();
    if (oName.length()==0)
        oName="Collision";
    if (objectIsACopy)
        oName=tt::generateNewName_hash(oName,suffixOffset);
    while (getObjectFromName(oName.c_str())!=nullptr)
        oName=tt::generateNewName_hashOrNoHash(oName,objectIsACopy);
    newCollObj->setObjectName(oName.c_str(),false);

    int handle=SIM_IDSTART_COLLISION;
    while (getObjectFromHandle(handle)!=nullptr)
        handle++;
    newCollObj->setObjectHandle(handle);

    _addObject(newCollObj);

    App::setFullDialogRefreshFlag();
}

int CCollisionObjectContainer::addNewObject(int entity1Handle,int entity2Handle,const char* objName)
{
    // We check if the objects are valid:
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
    // We check if we try to check an object against itself (forbidden, except for collections):
    if ( (entity1Handle<SIM_IDSTART_COLLECTION)&&(entity2Handle<SIM_IDSTART_COLLECTION) )
    {
        if (entity1Handle==entity2Handle)
            return(-1);
    }
    // We check if such an object already exists:
    for (size_t i=0;i<getObjectCount();i++)
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
        if (t1==sim_object_shape_type)
        {
            if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type) )
                return(-1);
        }
        if (t1==sim_object_octree_type)
        {
            if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type)&&(t2!=sim_object_pointcloud_type)&&(t2!=sim_object_dummy_type) )
                return(-1);
        }
        if ( (t1==sim_object_pointcloud_type)||(t1==sim_object_dummy_type) )
        {
            if (t2!=sim_object_octree_type)
                return(-1);
        }
    }

    // We create and insert the object
    CCollisionObject* newCollObject=new CCollisionObject(entity1Handle,entity2Handle);
    newCollObject->setObjectName(objName,false);
    addObject(newCollObject,false);
    return(newCollObject->getObjectHandle());
}

void CCollisionObjectContainer::removeObject(int objectHandle)
{
    App::currentWorld->announceCollisionWillBeErased(objectHandle);
    _removeObject(objectHandle);
}


void CCollisionObjectContainer::removeAllCollisionObjects()
{
    while (getObjectCount()!=0)
        removeObject(getObjectFromIndex(0)->getObjectHandle());
}

void CCollisionObjectContainer::announceCollectionWillBeErased(int collectionHandle)
{ // Never called from the copy buffer!
    size_t i=0;
    while (i<getObjectCount())
    {
        if (getObjectFromIndex(i)->announceCollectionWillBeErased(collectionHandle,false))
        { // We have to remove this collision object
            removeObject(getObjectFromIndex(i)->getObjectHandle()); // This will call announceCollisionWillBeErased!
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}


void CCollisionObjectContainer::announceObjectWillBeErased(int objectHandle)
{ // Never called from copy buffer!
    size_t i=0;
    while (i<getObjectCount())
    {
        if (getObjectFromIndex(i)->announceObjectWillBeErased(objectHandle,false))
        { // We have to remove this collision object
            removeObject(getObjectFromIndex(i)->getObjectHandle()); // This will call announceCollisionWillBeErased
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CCollisionObjectContainer::setUpDefaultValues()
{
    removeAllCollisionObjects();
}

int CCollisionObjectContainer::getCollisionColor(int entityID)
{
    int retVal=0;
    for (size_t i=0;i<getObjectCount();i++)
        retVal|=getObjectFromIndex(i)->getCollisionColor(entityID);
    return(retVal);
}

void CCollisionObjectContainer::resetAllCollisions(bool exceptExplicitHandling)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        if ( (!getObjectFromIndex(i)->getExplicitHandling())||(!exceptExplicitHandling) )
            getObjectFromIndex(i)->clearCollisionResult();
    }
}

int CCollisionObjectContainer::handleAllCollisions(bool exceptExplicitHandling)
{
    int retVal=0;
    for (size_t i=0;i<getObjectCount();i++)
    {
        if ( (!getObjectFromIndex(i)->getExplicitHandling())||(!exceptExplicitHandling) )
        {
            if (getObjectFromIndex(i)->handleCollision())
                retVal++;
        }
    }
    return(retVal);
}

void CCollisionObjectContainer::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
    if (displayAttrib&sim_displayattribute_renderpass)
        displayCollisionContours();
}

void CCollisionObjectContainer::displayCollisionContours()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->displayCollisionContour();
}

void CCollisionObjectContainer::_addObject(CCollisionObject* newCollObj)
{ // Overridden from _CCollisionObjectContainer_
    _CCollisionObjectContainer_::_addObject(newCollObj);

    if (newCollObj->setObjectCanSync(true))
        newCollObj->buildUpdateAndPopulateSynchronizationObject(nullptr);
}

void CCollisionObjectContainer::_removeObject(int objectHandle)
{ // Overridden from _CCollisionObjectContainer_
    CCollisionObject* ig=getObjectFromHandle(objectHandle);
    if (ig!=nullptr)
        ig->removeSynchronizationObject(false);

    _CCollisionObjectContainer_::_removeObject(objectHandle);
}

void CCollisionObjectContainer::buildUpdateAndPopulateSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CCollisionObject* it=getObjectFromIndex(i);
        it->buildUpdateAndPopulateSynchronizationObject(nullptr);
    }
}

void CCollisionObjectContainer::connectSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CCollisionObject* it=getObjectFromIndex(i);
        it->connectSynchronizationObject();
    }
}

void CCollisionObjectContainer::removeSynchronizationObjects(bool localReferencesToItOnly)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CCollisionObject* it=getObjectFromIndex(i);
        it->removeSynchronizationObject(localReferencesToItOnly);
    }
}
