
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "registeredCollisions.h"
#include "tt.h"
#include "collisionRoutine.h"
#include "app.h"

CRegisteredCollisions::CRegisteredCollisions()
{
    setUpDefaultValues();
}

CRegisteredCollisions::~CRegisteredCollisions()
{
    removeAllCollisionObjects();
}

void CRegisteredCollisions::simulationAboutToStart()
{
    for (int i=0;i<int(collisionObjects.size());i++)
        collisionObjects[i]->simulationAboutToStart();
}

void CRegisteredCollisions::simulationEnded()
{
    for (int i=0;i<int(collisionObjects.size());i++)
        collisionObjects[i]->simulationEnded();
}

void CRegisteredCollisions::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        int s=tt::getNameSuffixNumber(collisionObjects[i]->getObjectName().c_str(),true);
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

bool CRegisteredCollisions::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(collisionObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(collisionObjects[i]->getObjectName().c_str(),true));
            for (int j=0;j<int(collisionObjects.size());j++)
            {
                int s2=tt::getNameSuffixNumber(collisionObjects[j]->getObjectName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(collisionObjects[j]->getObjectName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CRegisteredCollisions::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(collisionObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(collisionObjects[i]->getObjectName().c_str(),true));
            collisionObjects[i]->setObjectName(tt::generateNewName_dash(name1,suffix2+1));
        }
    }
}

void CRegisteredCollisions::addObject(CRegCollision* newCollObj,bool objectIsACopy)
{
    addObjectWithSuffixOffset(newCollObj,objectIsACopy,1);
}

void CRegisteredCollisions::addObjectWithSuffixOffset(CRegCollision* newCollObj,bool objectIsACopy,int suffixOffset)
{   // Here we don't check whether such an object already exists or is valid.
    // This routine is mainly used for loading and copying operations
    // We check if that name already exists:
    std::string oName=newCollObj->getObjectName();
    if (oName.length()==0)
        oName="Collision";
    if (objectIsACopy)
        oName=tt::generateNewName_dash(oName,suffixOffset);
    else
    {
        while (getObject(oName)!=nullptr)
            oName=tt::generateNewName_noDash(oName);
    }
    newCollObj->setObjectName(oName);
    // We find a free ID:
    int id=SIM_IDSTART_COLLISION;
    while (getObject(id)!=nullptr)
        id++;
    newCollObj->setObjectID(id);
    collisionObjects.push_back(newCollObj);
    App::setFullDialogRefreshFlag();
}

int CRegisteredCollisions::addNewObject(int obj1ID,int obj2ID,std::string objName)
{
    // We check if the objects are valid:
    if (obj1ID<SIM_IDSTART_COLLECTION)
    {
        if (App::ct->objCont->getObjectFromHandle(obj1ID)==nullptr)
            return(-1);
    }
    else
    {
        if (App::ct->collections->getCollection(obj1ID)==nullptr)
            return(-1);
    }
    if (obj2ID>=SIM_IDSTART_COLLECTION)
    {
        if (App::ct->collections->getCollection(obj2ID)==nullptr)
            return(-1);
    }
    else
    {
        if ( (App::ct->objCont->getObjectFromHandle(obj2ID)==nullptr)&&(obj2ID!=-1) )
            return(-1);
    }
    // We check if we try to check an object against itself (forbidden, except for collections):
    if ( (obj1ID<SIM_IDSTART_COLLECTION)&&(obj2ID<SIM_IDSTART_COLLECTION) )
    {
        if (obj1ID==obj2ID)
            return(-1);
    }
    // We check if such an object already exists:
    for (size_t i=0;i<collisionObjects.size();i++)
    {
        if (collisionObjects[i]->isSame(obj1ID,obj2ID))
            return(-1);
    }
    // Now check if the combination is valid:
    if ( (obj1ID<SIM_IDSTART_COLLECTION)&&(obj2ID<SIM_IDSTART_COLLECTION) )
    {
        int t1=App::ct->objCont->getObjectFromHandle(obj1ID)->getObjectType();
        int t2=sim_object_octree_type;
        if (obj2ID!=-1)
            t2=App::ct->objCont->getObjectFromHandle(obj2ID)->getObjectType();
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
    CRegCollision* newCollObject=new CRegCollision(obj1ID,obj2ID,objName,SIM_IDSTART_COLLISION);
    addObject(newCollObject,false);
    return(newCollObject->getObjectID());
}
bool CRegisteredCollisions::removeObject(int objID)
{
    App::ct->objCont->announceCollisionWillBeErased(objID);
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        if (collisionObjects[i]->getObjectID()==objID)
        {
            delete collisionObjects[i];
            collisionObjects.erase(collisionObjects.begin()+i);
            App::setFullDialogRefreshFlag();
            return(true);   // We could remove the object
        }
    }
    return(false);  // The object don't exist
}
CRegCollision* CRegisteredCollisions::getObject(int objID)
{
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        if (collisionObjects[i]->getObjectID()==objID)
            return(collisionObjects[i]);
    }
    return(nullptr);
}
CRegCollision* CRegisteredCollisions::getObject(std::string objName)
{
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        if (collisionObjects[i]->getObjectName().compare(objName)==0)
            return(collisionObjects[i]);
    }
    return(nullptr);
}
void CRegisteredCollisions::removeAllCollisionObjects()
{
    while (collisionObjects.size()!=0)
        removeObject(collisionObjects[0]->getObjectID());
}

void CRegisteredCollisions::announceCollectionWillBeErased(int groupID)
{ // Never called from the copy buffer!
    int i=0;
    while (i<int(collisionObjects.size()))
    {
        if (collisionObjects[i]->announceCollectionWillBeErased(groupID,false))
        { // We have to remove this collision object
            removeObject(collisionObjects[i]->getObjectID()); // This will call announceCollisionWillBeErased!
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}


void CRegisteredCollisions::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(collisionObjects.size()))
    {
        if (collisionObjects[i]->announceObjectWillBeErased(objID,false))
        { // We have to remove this collision object
            removeObject(collisionObjects[i]->getObjectID()); // This will call announceCollisionWillBeErased
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CRegisteredCollisions::setUpDefaultValues()
{
    removeAllCollisionObjects();
}

bool CRegisteredCollisions::setObjectName(int objID,std::string newName)
{
    CRegCollision* it=getObject(newName);
    if (it==nullptr)
    {
        it=getObject(objID);
        if (it==nullptr)
            return(false); // Failure
        it->setObjectName(newName);
        return(true); // Success
    }
    if (it->getObjectID()==objID)
        return(true); // Success
    return(false); // Failure
}

int CRegisteredCollisions::getCollisionColor(int entityID)
{
    int retVal=0;
    for (int i=0;i<int(collisionObjects.size());i++)
        retVal|=collisionObjects[i]->getCollisionColor(entityID);
    return(retVal);
}

void CRegisteredCollisions::resetAllCollisions(bool exceptExplicitHandling)
{
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        if ( (!collisionObjects[i]->getExplicitHandling())||(!exceptExplicitHandling) )
            collisionObjects[i]->clearCollisionResult();
    }
}

int CRegisteredCollisions::handleAllCollisions(bool exceptExplicitHandling)
{
    int retVal=0;
    for (int i=0;i<int(collisionObjects.size());i++)
    {
        if ( (!collisionObjects[i]->getExplicitHandling())||(!exceptExplicitHandling) )
        {
            if (collisionObjects[i]->handleCollision())
                retVal++;
        }
    }
    return(retVal);
}

void CRegisteredCollisions::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
    if (displayAttrib&sim_displayattribute_renderpass)
        displayCollisionContours();
}

void CRegisteredCollisions::displayCollisionContours()
{
    for (size_t i=0;i<collisionObjects.size();i++)
        collisionObjects[i]->displayCollisionContour();
}
