#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "regCollision.h"
#include "3DObject.h"
#include "global.h"
#include "collisionRoutine.h"
#include "tt.h"
#include "ttUtil.h"
#include "app.h"
#include "v_repStrings.h"
#include "vDateTime.h"
#include "pluginContainer.h"
#include "collisionContourRendering.h"

CRegCollision::CRegCollision(int obj1ID,int obj2ID,std::string objName,int objID)
{ // obj2ID can be -1, in which case obj1ID will be checked against all other collidable objects in the scene
    object1ID=obj1ID;
    object2ID=obj2ID;
    objectName=objName;
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    objectID=objID;
    collisionResult=false;
    _collisionResultValid=false;
    _collObjectHandles[0]=-1;
    _collObjectHandles[1]=-1;
    _countourWidth=1;
    contourColor.setColorsAllBlack();
    contourColor.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_emission);
    _initialValuesInitialized=false;
    _calcTimeInMs=0;

    colliderChangesColor=true;
    collideeChangesColor=false;
    detectAllCollisions=false;
    explicitHandling=false;
}

std::string CRegCollision::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

void CRegCollision::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=explicitHandling;
    }
}

void CRegCollision::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CRegCollision::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        explicitHandling=_initialExplicitHandling;
    }
    _initialValuesInitialized=false;
}

CRegCollision::~CRegCollision()
{
}

bool CRegCollision::isSame(int obj1ID,int obj2ID) const
{
    if ( (obj1ID==object1ID)&&(obj2ID==object2ID) )
        return(true);
    if ( (obj2ID==object1ID)&&(obj1ID==object2ID) )
        return(true);
    return(false);
}

int CRegCollision::getObjectID() const
{
    return(objectID);
}

std::string CRegCollision::getObjectName() const
{
    return(objectName);
}

std::string CRegCollision::getObjectPartnersName() const
{
    std::string theName=getObjectName();
    theName=theName.append(" (");
    if (object1ID<SIM_IDSTART_COLLECTION)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(object1ID);
        int t=it->getObjectType();
        if (t==sim_object_shape_type)
            theName+=IDSN_SHAPE;
        if (t==sim_object_octree_type)
            theName+=IDSN_OCTREE;
        if (t==sim_object_pointcloud_type)
            theName+=IDSN_POINTCLOUD;
        if (t==sim_object_dummy_type)
            theName+=IDSN_DUMMY;
        theName+=":";
        theName+=it->getObjectName();
    }
    else
    {
        CRegCollection* it=App::ct->collections->getCollection(object1ID);
        if (it!=nullptr)
        {
            theName+=strTranslate(IDSN_COLLECTION);
            theName+=":";
            theName+=it->getCollectionName();
        }
    }
    theName=theName.append(" - ");
    if (object2ID>=SIM_IDSTART_COLLECTION)
    {
        CRegCollection* it=App::ct->collections->getCollection(object2ID);
        if (it!=nullptr)
        {
            theName+=strTranslate(IDSN_COLLECTION);
            theName+=":";
            theName+=it->getCollectionName();
        }
    }
    else
    {
        if (object2ID!=-1)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(object2ID);
            int t=it->getObjectType();
            if (t==sim_object_shape_type)
                theName+=IDSN_SHAPE;
            if (t==sim_object_octree_type)
                theName+=IDSN_OCTREE;
            if (t==sim_object_pointcloud_type)
                theName+=IDSN_POINTCLOUD;
            if (t==sim_object_dummy_type)
                theName+=IDSN_DUMMY;
            theName+=":";
            theName+=it->getObjectName();
        }
        else
            theName+=strTranslate(IDS_ALL_OTHER_ENTITIES);
    }
    theName=theName.append(")");    
    return(theName);
}

int CRegCollision::getCollisionColor(int entityID) const
{
    if (!collisionResult)
        return(0);
    int retVal=0;
    if ( (entityID==object1ID)&&colliderChangesColor)
        retVal|=1;
    if ( (entityID==object2ID)&&collideeChangesColor)
        retVal|=2;
    // Here we need to check for the special case where object2ID==-1 (which means all other objects)
    if ((object2ID==-1)&&collideeChangesColor&&(entityID<SIM_IDSTART_COLLECTION))
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(entityID);
        if ( (it!=nullptr)&&(it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable) )
        {
            if (object1ID<SIM_IDSTART_COLLECTION)
            {
                if (entityID!=object1ID)
                    retVal|=2;
            }
            else
            {
                std::vector<C3DObject*> group1;
                App::ct->collections->getCollidableObjectsFromCollection(object1ID,group1);
                bool isContained=false;
                for (size_t i=0;i<group1.size();i++)
                {
                    if (group1[i]==it)
                    {
                        isContained=true;
                        break;
                    }
                }
                if (!isContained)
                    retVal|=2;
            }
        }
    }
    return(retVal);
}

bool CRegCollision::announceObjectWillBeErased(int objID,bool copyBuffer)
{ // Return value true means that this collision object should be destroyed
    if (object1ID==objID)
        return(true);
    if (object2ID==objID)
        return(true);
    return(false);
}

bool CRegCollision::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{ // Return value true means that this collision object should be destroyed
    if (object1ID==groupID)
        return(true);
    if (object2ID==groupID)
        return(true);
    return(false);
}

void CRegCollision::setObjectName(std::string newName)
{
    objectName=newName;
}
void CRegCollision::setObjectID(int newID)
{
    objectID=newID;
}
int CRegCollision::getObject1ID() const
{
    return(object1ID);
}
int CRegCollision::getObject2ID() const
{
    return(object2ID);
}

void CRegCollision::setColliderChangesColor(bool changes)
{
    colliderChangesColor=changes;
}

bool CRegCollision::getColliderChangesColor() const
{
    return(colliderChangesColor);
}

void CRegCollision::setCollideeChangesColor(bool changes)
{
    collideeChangesColor=changes;
}

bool CRegCollision::getCollideeChangesColor() const
{
    return(collideeChangesColor);
}

void CRegCollision::setExhaustiveDetection(bool exhaustive)
{
    detectAllCollisions=exhaustive;
}

bool CRegCollision::getExhaustiveDetection() const
{
    return(detectAllCollisions);
}

bool CRegCollision::canComputeCollisionContour() const
{
    if (object1ID>=SIM_IDSTART_COLLECTION)
    {
        if (object2ID>=SIM_IDSTART_COLLECTION)
            return(true); // collection/collection
        else
            return(App::ct->objCont->getShape(object2ID)!=nullptr); // collection/shape
    }
    else
    {
        if (App::ct->objCont->getShape(object1ID)==nullptr)
            return(false); // non-shape/something
        if (object2ID==-1)
            return(true); // shape/allOtherObjects
        if (object2ID>=SIM_IDSTART_COLLECTION)
            return(true); // shape/collection
        else
            return(App::ct->objCont->getShape(object2ID)!=nullptr); // shape/shape
    }
}


void CRegCollision::setExplicitHandling(bool explicitHandl)
{
    explicitHandling=explicitHandl;
}

bool CRegCollision::getExplicitHandling() const
{
    return(explicitHandling);
}

void CRegCollision::clearCollisionResult()
{
    collisionResult=false;
    _collisionResultValid=false;
    _collObjectHandles[0]=-1;
    _collObjectHandles[1]=-1;
    _calcTimeInMs=0;
    intersections.clear();
}

bool CRegCollision::isCollisionResultValid() const
{
    return(_collisionResultValid);
}

bool CRegCollision::getCollisionResult() const
{
    return(collisionResult);
}

float CRegCollision::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

void CRegCollision::setContourWidth(int w)
{
    w=tt::getLimitedInt(1,4,w);
    _countourWidth=w;
}

int CRegCollision::getContourWidth() const
{
    return(_countourWidth);
}

std::vector<float>* CRegCollision::getIntersectionsPtr()
{
    return(&intersections);
}

bool CRegCollision::handleCollision()
{   // Return value true means there was a collision
    clearCollisionResult();
    if (!App::ct->mainSettings->collisionDetectionEnabled)
        return(false);
    if (!CPluginContainer::isMeshPluginAvailable())
        return(false);
    int stT=VDateTime::getTimeInMs();
    if (detectAllCollisions)
    {
        std::vector<float> collCont;
        collisionResult=CCollisionRoutine::doEntitiesCollide(object1ID,object2ID,&collCont,false,false,_collObjectHandles);
        for (int i=0;i<int(collCont.size());i++)
            intersections.push_back(collCont[i]);
    }
    else
        collisionResult=CCollisionRoutine::doEntitiesCollide(object1ID,object2ID,nullptr,false,false,_collObjectHandles);
    _calcTimeInMs=VDateTime::getTimeDiffInMs(stT);
    _collisionResultValid=true;
    return(collisionResult);
}

int CRegCollision::readCollision(int collObjHandles[2]) const
{
    if (_collisionResultValid)
    {
        if (collisionResult)
        {
            if (collObjHandles!=nullptr)
            {
                collObjHandles[0]=_collObjectHandles[0];
                collObjHandles[1]=_collObjectHandles[1];
            }
            return(1);
        }
        return(0);
    }
    return(0); // was -1 until 16/6/2015
}

void CRegCollision::performObjectLoadingMapping(std::vector<int>* map)
{
    if (object1ID<SIM_IDSTART_COLLECTION)
        object1ID=App::ct->objCont->getLoadingMapping(map,object1ID);
    if ( (object2ID<SIM_IDSTART_COLLECTION)&&(object2ID!=-1) )
        object2ID=App::ct->objCont->getLoadingMapping(map,object2ID);
}

void CRegCollision::performCollectionLoadingMapping(std::vector<int>* map)
{
    if (object1ID>=SIM_IDSTART_COLLECTION)
        object1ID=App::ct->objCont->getLoadingMapping(map,object1ID);
    if (object2ID>=SIM_IDSTART_COLLECTION)
        object2ID=App::ct->objCont->getLoadingMapping(map,object2ID);
}

CRegCollision* CRegCollision::copyYourself()
{
    CRegCollision* newCollObj=new CRegCollision(object1ID,object2ID,objectName,objectID);
    newCollObj->colliderChangesColor=colliderChangesColor;
    newCollObj->collideeChangesColor=collideeChangesColor;
    newCollObj->detectAllCollisions=detectAllCollisions;
    newCollObj->explicitHandling=explicitHandling;

    newCollObj->_countourWidth=_countourWidth;
    contourColor.copyYourselfInto(&newCollObj->contourColor);

    newCollObj->_initialValuesInitialized=_initialValuesInitialized;
    newCollObj->_initialExplicitHandling=_initialExplicitHandling;

    return(newCollObj);
}

void CRegCollision::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Inx");
            ar << object1ID << object2ID << objectID;
            ar.flush();

            ar.storeDataName("Nme");
            ar << objectName;
            ar.flush();

            ar.storeDataName("Cwt");
            ar << _countourWidth;
            ar.flush();

            ar.storeDataName("Col");
            ar.setCountingMode();
            contourColor.serialize(ar,1);
            if (ar.setWritingMode())
                contourColor.serialize(ar,1);

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,colliderChangesColor);
            SIM_SET_CLEAR_BIT(nothing,1,collideeChangesColor);
            SIM_SET_CLEAR_BIT(nothing,2,detectAllCollisions);
            SIM_SET_CLEAR_BIT(nothing,3,explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Cwt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _countourWidth;
                    }
                    if (theName.compare("Col")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        contourColor.serialize(ar,1);
                    }
                    if (theName.compare("Inx")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> object1ID >> object2ID >> objectID;
                    }
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> objectName;
                    }
                    if (theName.compare("Uis")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _uniquePersistentIdString;
                    }
                    if (theName=="Par")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        colliderChangesColor=SIM_IS_BIT_SET(nothing,0);
                        collideeChangesColor=SIM_IS_BIT_SET(nothing,1);
                        detectAllCollisions=SIM_IS_BIT_SET(nothing,2);
                        explicitHandling=SIM_IS_BIT_SET(nothing,3);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CRegCollision::displayCollisionContour()
{
    displayContour(this,_countourWidth);
}
