
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "registeredDistances.h"
#include "tt.h"
#include "distanceRoutine.h"
#include "app.h"
#include "gV.h"

CRegisteredDistances::CRegisteredDistances()
{
    setUpDefaultValues();
}

CRegisteredDistances::~CRegisteredDistances()
{
    removeAllDistanceObjects();
}

void CRegisteredDistances::simulationAboutToStart()
{
    for (int i=0;i<int(distanceObjects.size());i++)
        distanceObjects[i]->simulationAboutToStart();
}

void CRegisteredDistances::simulationEnded()
{
    for (int i=0;i<int(distanceObjects.size());i++)
        distanceObjects[i]->simulationEnded();
}

void CRegisteredDistances::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix)
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        int s=tt::getNameSuffixNumber(distanceObjects[i]->getObjectName().c_str(),true);
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

bool CRegisteredDistances::canSuffix1BeSetToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(distanceObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(distanceObjects[i]->getObjectName().c_str(),true));
            for (int j=0;j<int(distanceObjects.size());j++)
            {
                int s2=tt::getNameSuffixNumber(distanceObjects[j]->getObjectName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(distanceObjects[j]->getObjectName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CRegisteredDistances::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(distanceObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(distanceObjects[i]->getObjectName().c_str(),true));
            distanceObjects[i]->setObjectName(tt::generateNewName_dash(name1,suffix2+1));
        }
    }
}

void CRegisteredDistances::addObject(CRegDist* newDistObj,bool objectIsACopy)
{
    addObjectWithSuffixOffset(newDistObj,objectIsACopy,1);
}

void CRegisteredDistances::addObjectWithSuffixOffset(CRegDist* newDistObj,bool objectIsACopy,int suffixOffset)
{   // Here we don't check whether such an object already exists or is valid.
    // This routine is mainly used for loading and copying operations
    // We check if that name already exists:
    std::string oName=newDistObj->getObjectName();
    if (oName.length()==0)
        oName="Distance";
    if (objectIsACopy)
        oName=tt::generateNewName_dash(oName,suffixOffset);
    else
    {
        while (getObject(oName)!=nullptr)
            oName=tt::generateNewName_noDash(oName);
    }
    newDistObj->setObjectName(oName);
    // We find a free ID:
    int id=SIM_IDSTART_DISTANCE;
    while (getObject(id)!=nullptr)
        id++;
    newDistObj->setObjectID(id);
    distanceObjects.push_back(newDistObj);
    App::setFullDialogRefreshFlag();
}

int CRegisteredDistances::addNewObject(int obj1ID,int obj2ID,
                                        std::string objName)
{   // We check if the objects are valid:
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
    // We check if we try to measure an object against itself (forbidden, except for collections):
    if ( (obj1ID<SIM_IDSTART_COLLECTION)&&
        (obj2ID<SIM_IDSTART_COLLECTION) )
    {
        if (obj1ID==obj2ID)
            return(-1);
    }

    // We check if such an object already exists:
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        if (distanceObjects[i]->isSame(obj1ID,obj2ID))
            return(-1);
    }

    // Now check if the combination is valid:
    if ( (obj1ID<SIM_IDSTART_COLLECTION)&&(obj2ID<SIM_IDSTART_COLLECTION) )
    {
        int t1=App::ct->objCont->getObjectFromHandle(obj1ID)->getObjectType();
        int t2=sim_object_octree_type;
        if (obj2ID!=-1)
            t2=App::ct->objCont->getObjectFromHandle(obj2ID)->getObjectType();
        if ( (t1==sim_object_shape_type)||(t1==sim_object_octree_type)||(t1==sim_object_pointcloud_type)||(t1==sim_object_dummy_type) )
        {
            if ( (t2!=sim_object_shape_type)&&(t2!=sim_object_octree_type)&&(t2!=sim_object_pointcloud_type)&&(t2!=sim_object_dummy_type) )
                return(-1);
        }
        else
            return(-1);
    }

    CRegDist* newDistObject=new CRegDist(obj1ID,obj2ID,objName,SIM_IDSTART_DISTANCE);
    addObject(newDistObject,false);

    return(newDistObject->getObjectID());
}

bool CRegisteredDistances::removeObject(int objID)
{
    App::ct->objCont->announceDistanceWillBeErased(objID);
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        if (distanceObjects[i]->getObjectID()==objID)
        {
            delete distanceObjects[i];
            distanceObjects.erase(distanceObjects.begin()+i);
            App::setFullDialogRefreshFlag();
            return(true);   // We could remove the object
        }
    }
    return(false);  // The object doesn't exist
}

CRegDist* CRegisteredDistances::getObject(int objID)
{
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        if (distanceObjects[i]->getObjectID()==objID)
            return(distanceObjects[i]);
    }
    return(nullptr);
}

CRegDist* CRegisteredDistances::getObject(std::string objName)
{
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        if (distanceObjects[i]->getObjectName().compare(objName)==0)
            return(distanceObjects[i]);
    }
    return(nullptr);
}

void CRegisteredDistances::removeAllDistanceObjects()
{
    while (distanceObjects.size()!=0)
        removeObject(distanceObjects[0]->getObjectID());
}

void CRegisteredDistances::resetAllDistances(bool exceptExplicitHandling)
{
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        if ( (!distanceObjects[i]->getExplicitHandling())||(!exceptExplicitHandling) )
            distanceObjects[i]->clearDistanceResult();
    }
}

void CRegisteredDistances::announceCollectionWillBeErased(int groupID)
{ // never called from the copy buffer
    int i=0;
    while (i<int(distanceObjects.size()))
    {
        if (distanceObjects[i]->announceCollectionWillBeErased(groupID,false))
        { // We have to remove this distance object
            removeObject(distanceObjects[i]->getObjectID()); // This will call announceDistanceWillBeErased!
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CRegisteredDistances::announceObjectWillBeErased(int objID)
{ // Never called from copy buffer!
    int i=0;
    while (i<int(distanceObjects.size()))
    {
        if (distanceObjects[i]->announceObjectWillBeErased(objID,false))
        { // We have to remove this collision object
            removeObject(distanceObjects[i]->getObjectID()); // This will call announceDistanceWillBeErased!
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CRegisteredDistances::setUpDefaultValues()
{
    removeAllDistanceObjects();
}

bool CRegisteredDistances::setObjectName(int objID,std::string newName)
{
    CRegDist* it=getObject(newName);
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

float CRegisteredDistances::handleAllDistances(bool exceptExplicitHandling)
{ // Return value is the smallest distance measured or negative if nothing was measured
    resetAllDistances(exceptExplicitHandling);
    if (!App::ct->mainSettings->distanceCalculationEnabled)
        return(-1.0f);

    float retVal=SIM_MAX_FLOAT;
    bool retPos=false;
    for (int i=0;i<int(distanceObjects.size());i++)
    {
        if ( (!distanceObjects[i]->getExplicitHandling())||(!exceptExplicitHandling) )
        {
            float d=distanceObjects[i]->handleDistance();
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

void CRegisteredDistances::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{
    if (displayAttrib&sim_displayattribute_renderpass)
        displayDistanceSegments();
}

void CRegisteredDistances::displayDistanceSegments()
{
    for (size_t i=0;i<distanceObjects.size();i++)
        distanceObjects[i]->displayDistanceSegment();
}
