#include <simInternal.h>
#include <collisionObject_old.h>
#include <sceneObject.h>
#include <global.h>
#include <collisionRoutines.h>
#include <tt.h>
#include <ttUtil.h>
#include <app.h>
#include <simStrings.h>
#include <vDateTime.h>
#include <pluginContainer.h>
#include <collisionContourRendering.h>
#include <base64.h>

CCollisionObject_old::CCollisionObject_old()
{
    _commonInit();
}

CCollisionObject_old::CCollisionObject_old(int entity1Handle,int entity2Handle)
{
    _commonInit();
    _entity1Handle=entity1Handle;
    _entity2Handle=entity2Handle;
}

CCollisionObject_old::~CCollisionObject_old()
{
}

void CCollisionObject_old::_commonInit()
{
    _entity1Handle=-1;
    _entity2Handle=-1;
    _objectHandle=-1;
    _countourWidth=1;
    _colliderChangesColor=true;
    _collideeChangesColor=false;
    _detectAllCollisions=false;
    _explicitHandling=false;

    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    _collisionResult=false;
    _collisionResultValid=false;
    _collObjectHandles[0]=-1;
    _collObjectHandles[1]=-1;
    _initialValuesInitialized=false;
    _calcTimeInMs=0;
    _contourColor.setColorsAllBlack();
    _contourColor.setColor(1.0,1.0,1.0,sim_colorcomponent_emission);
}

void CCollisionObject_old::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized=true;
    _initialExplicitHandling=_explicitHandling;
}

void CCollisionObject_old::simulationAboutToStart()
{
    initializeInitialValues(false);
}

void CCollisionObject_old::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
        setExplicitHandling(_initialExplicitHandling);
    _initialValuesInitialized=false;
}

bool CCollisionObject_old::isSame(int entity1Handle,int entity2Handle) const
{
    if ( (entity1Handle==_entity1Handle)&&(entity2Handle==_entity2Handle) )
        return(true);
    if ( (entity2Handle==_entity1Handle)&&(entity1Handle==_entity2Handle) )
        return(true);
    return(false);
}

std::string CCollisionObject_old::getObjectDescriptiveName() const
{
    std::string theName=getObjectName();
    theName=theName.append(" (");
    if (_entity1Handle<SIM_IDSTART_COLLECTION)
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_entity1Handle);
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
        theName+=it->getObjectName_old();
    }
    else
    {
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(_entity1Handle);
        if (it!=nullptr)
        {
            theName+=IDSN_COLLECTION;
            theName+=":";
            theName+=it->getCollectionName();
        }
    }
    theName=theName.append(" - ");
    if (_entity2Handle>=SIM_IDSTART_COLLECTION)
    {
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(_entity2Handle);
        if (it!=nullptr)
        {
            theName+=IDSN_COLLECTION;
            theName+=":";
            theName+=it->getCollectionName();
        }
    }
    else
    {
        if (_entity2Handle!=-1)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_entity2Handle);
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
            theName+=it->getObjectName_old();
        }
        else
            theName+=IDS_ALL_OTHER_ENTITIES;
    }
    theName=theName.append(")");    
    return(theName);
}

int CCollisionObject_old::getCollisionColor(int entityHandle) const
{
    if (!_collisionResult)
        return(0);
    int retVal=0;
    if ( (entityHandle==_entity1Handle)&&_colliderChangesColor)
        retVal|=1;
    if ( (entityHandle==_entity2Handle)&&_collideeChangesColor)
        retVal|=2;
    // Here we need to check for the special case where object2ID==-1 (which means all other objects)
    if ((_entity2Handle==-1)&&_collideeChangesColor&&(entityHandle<SIM_IDSTART_COLLECTION))
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(entityHandle);
        if ( (it!=nullptr)&&(it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable) )
        {
            if (_entity1Handle<SIM_IDSTART_COLLECTION)
            {
                if (entityHandle!=_entity1Handle)
                    retVal|=2;
            }
            else
            {
                std::vector<CSceneObject*> group1;
                App::currentWorld->collections->getCollidableObjectsFromCollection(_entity1Handle,group1);
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

bool CCollisionObject_old::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{ // Return value true means that this collision object should be destroyed
    if (_entity1Handle==objectHandle)
        return(true);
    if (_entity2Handle==objectHandle)
        return(true);
    return(false);
}

bool CCollisionObject_old::announceCollectionWillBeErased(int collectionHandle,bool copyBuffer)
{ // Return value true means that this collision object should be destroyed
    if (_entity1Handle==collectionHandle)
        return(true);
    if (_entity2Handle==collectionHandle)
        return(true);
    return(false);
}

bool CCollisionObject_old::canComputeCollisionContour() const
{
    if (_entity1Handle>=SIM_IDSTART_COLLECTION)
    {
        if (_entity2Handle>=SIM_IDSTART_COLLECTION)
            return(true); // collection/collection
        else
            return(App::currentWorld->sceneObjects->getShapeFromHandle(_entity2Handle)!=nullptr); // collection/shape
    }
    else
    {
        if (App::currentWorld->sceneObjects->getShapeFromHandle(_entity1Handle)==nullptr)
            return(false); // non-shape/something
        if (_entity2Handle==-1)
            return(true); // shape/allOtherObjects
        if (_entity2Handle>=SIM_IDSTART_COLLECTION)
            return(true); // shape/collection
        else
            return(App::currentWorld->sceneObjects->getShapeFromHandle(_entity2Handle)!=nullptr); // shape/shape
    }
}


void CCollisionObject_old::clearCollisionResult()
{
    _clearCollisionResult();
}

bool CCollisionObject_old::setObjectName(const char* newName,bool check)
{ // Overridden from _CCollisionObject_
    std::string nnn;
    CCollisionObject_old* it=nullptr;
    if (check)
        it=App::currentWorld->collisions->getObjectFromHandle(_objectHandle);
    if (it!=this)
        nnn=newName;
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if (nm.size()>0)
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->collisions->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm.c_str(),!tt::isHashFree(nm.c_str()));
                nnn=nm.c_str();
            }
        }
    }
    bool diff=false;
    if (nnn.size()>0)
    {
        diff=(_objectName!=nnn);
        if (diff)
            _objectName=nnn;
    }
    return(diff);
}

bool CCollisionObject_old::handleCollision()
{   // Return value true means there was a collision
    clearCollisionResult();
    if (!App::currentWorld->mainSettings->collisionDetectionEnabled)
        return(false);
    if (!CPluginContainer::isGeomPluginAvailable())
        return(false);
    int stT=(int)VDateTime::getTimeInMs();
    int collObjs[2];
    bool res=false;
    std::vector<double> collCont;
    if (_detectAllCollisions)
        res=CCollisionRoutine::doEntitiesCollide(_entity1Handle,_entity2Handle,&collCont,false,false,collObjs);
    else
        res=CCollisionRoutine::doEntitiesCollide(_entity1Handle,_entity2Handle,nullptr,false,false,collObjs);
    _setCollisionResult(res,VDateTime::getTimeDiffInMs(stT),collObjs[0],collObjs[1],collCont);
    return(_collisionResult);
}

int CCollisionObject_old::readCollision(int collObjHandles[2]) const
{
    if (_collisionResultValid)
    {
        if (_collisionResult)
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

void CCollisionObject_old::performObjectLoadingMapping(const std::map<int,int>* map)
{
    if (_entity1Handle<SIM_IDSTART_COLLECTION)
        _entity1Handle=CWorld::getLoadingMapping(map,_entity1Handle);
    if ( (_entity2Handle<SIM_IDSTART_COLLECTION)&&(_entity2Handle!=-1) )
        _entity2Handle=CWorld::getLoadingMapping(map,_entity2Handle);
}

void CCollisionObject_old::performCollectionLoadingMapping(const std::map<int,int>* map)
{
    if (_entity1Handle>=SIM_IDSTART_COLLECTION)
        _entity1Handle=CWorld::getLoadingMapping(map,_entity1Handle);
    if (_entity2Handle>=SIM_IDSTART_COLLECTION)
        _entity2Handle=CWorld::getLoadingMapping(map,_entity2Handle);
}

CCollisionObject_old* CCollisionObject_old::copyYourself()
{
    CCollisionObject_old* newCollObj=new CCollisionObject_old();
    newCollObj->_objectHandle=_objectHandle; // important for copy operations connections
    newCollObj->_entity1Handle=_entity1Handle;
    newCollObj->_entity2Handle=_entity2Handle;
    newCollObj->_objectName=_objectName;
    newCollObj->_colliderChangesColor=_colliderChangesColor;
    newCollObj->_collideeChangesColor=_collideeChangesColor;
    newCollObj->_detectAllCollisions=_detectAllCollisions;
    newCollObj->_explicitHandling=_explicitHandling;
    newCollObj->_countourWidth=_countourWidth;
    _contourColor.copyYourselfInto(&newCollObj->_contourColor);

    newCollObj->_initialValuesInitialized=_initialValuesInitialized;
    newCollObj->_initialExplicitHandling=_initialExplicitHandling;

    return(newCollObj);
}

void CCollisionObject_old::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Inx");
            ar << _entity1Handle << _entity2Handle << _objectHandle;
            ar.flush();

            ar.storeDataName("Nme");
            ar << _objectName;
            ar.flush();

            ar.storeDataName("Cwt");
            ar << _countourWidth;
            ar.flush();

            ar.storeDataName("Col");
            ar.setCountingMode();
            _contourColor.serialize(ar,1);
            if (ar.setWritingMode())
                _contourColor.serialize(ar,1);

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_colliderChangesColor);
            SIM_SET_CLEAR_BIT(nothing,1,_collideeChangesColor);
            SIM_SET_CLEAR_BIT(nothing,2,_detectAllCollisions);
            SIM_SET_CLEAR_BIT(nothing,3,_explicitHandling);
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
                        ar >> byteQuantity; 
                        _contourColor.serialize(ar,1);
                    }
                    if (theName.compare("Inx")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _entity1Handle >> _entity2Handle >> _objectHandle;
                    }
                    if (theName.compare("Nme")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectName;
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
                        _colliderChangesColor=SIM_IS_BIT_SET(nothing,0);
                        _collideeChangesColor=SIM_IS_BIT_SET(nothing,1);
                        _detectAllCollisions=SIM_IS_BIT_SET(nothing,2);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,3);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_int("handle",_objectHandle);
            ar.xmlAddNode_string("name",_objectName.c_str());
            ar.xmlAddNode_2int("pairHandles",_entity1Handle,_entity2Handle);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("colliderChangesColor",_colliderChangesColor);
            ar.xmlAddNode_bool("collideeChangesColor",_collideeChangesColor);
            ar.xmlAddNode_bool("explicitHandling",_explicitHandling);
            ar.xmlPopNode();

            ar.xmlPushNewNode("collisionContour");
            ar.xmlAddNode_bool("enabled",_detectAllCollisions);
            ar.xmlAddNode_int("size",_countourWidth);
            ar.xmlPushNewNode("color");
            _contourColor.serialize(ar,1);
            ar.xmlPopNode();
            ar.xmlPopNode();

            std::string str(base64_encode((unsigned char*)_uniquePersistentIdString.c_str(),_uniquePersistentIdString.size()));
            ar.xmlAddNode_string("uniquePersistentIdString_base64Coded",str.c_str());
        }
        else
        {
            ar.xmlGetNode_int("handle",_objectHandle);
            ar.xmlGetNode_string("name",_objectName);
            ar.xmlGetNode_2int("pairHandles",_entity1Handle,_entity2Handle);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("colliderChangesColor",_colliderChangesColor);
                ar.xmlGetNode_bool("collideeChangesColor",_collideeChangesColor);
                ar.xmlGetNode_bool("explicitHandling",_explicitHandling);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("collisionContour"))
            {
                ar.xmlGetNode_bool("enabled",_detectAllCollisions);
                ar.xmlGetNode_int("size",_countourWidth);
                if (ar.xmlPushChildNode("color"))
                {
                    _contourColor.serialize(ar,1);
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }

            if (ar.xmlGetNode_string("uniquePersistentIdString_base64Coded",_uniquePersistentIdString))
                _uniquePersistentIdString=base64_decode(_uniquePersistentIdString);
        }
    }
}

void CCollisionObject_old::displayCollisionContour()
{
    displayContour(this,_countourWidth);
}

std::string CCollisionObject_old::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

void CCollisionObject_old::_clearCollisionResult()
{
    _collisionResult=false;
    _collisionResultValid=false;
    _collObjectHandles[0]=-1;
    _collObjectHandles[1]=-1;
    _calcTimeInMs=0;
    setIntersections(nullptr);
}

void CCollisionObject_old::_setCollisionResult(bool result,int calcTime,int obj1Handle,int obj2Handle,const std::vector<double>& intersect)
{
    _collisionResult=result;
    _collisionResultValid=true;
    _collObjectHandles[0]=obj1Handle;
    _collObjectHandles[1]=obj2Handle;
    _calcTimeInMs=calcTime;
    setIntersections(&intersect);
}

bool CCollisionObject_old::isCollisionResultValid() const
{
    return(_collisionResultValid);
}

bool CCollisionObject_old::getCollisionResult() const
{
    return(_collisionResult);
}

double CCollisionObject_old::getCalculationTime() const
{
    return(double(_calcTimeInMs)*0.001);
}

int CCollisionObject_old::getObjectHandle() const
{
    return(_objectHandle);
}

int CCollisionObject_old::getEntity1Handle() const
{
    return(_entity1Handle);
}

int CCollisionObject_old::getEntity2Handle() const
{
    return(_entity2Handle);
}

std::string CCollisionObject_old::getObjectName() const
{
    return(_objectName);
}

bool CCollisionObject_old::getColliderChangesColor() const
{
    return(_colliderChangesColor);
}

bool CCollisionObject_old::getCollideeChangesColor() const
{
    return(_collideeChangesColor);
}

bool CCollisionObject_old::getExhaustiveDetection() const
{
    return(_detectAllCollisions);
}

bool CCollisionObject_old::getExplicitHandling() const
{
    return(_explicitHandling);
}

int CCollisionObject_old::getContourWidth() const
{
    return(_countourWidth);
}

const std::vector<double>* CCollisionObject_old::getIntersections() const
{
    return(&_intersections);
}

CColorObject* CCollisionObject_old::getContourColor()
{
    return(&_contourColor);
}

bool CCollisionObject_old::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    if (diff)
        _objectHandle=newHandle;
    return(diff);
}

bool CCollisionObject_old::setColliderChangesColor(bool changes)
{
    bool diff=(_colliderChangesColor!=changes);
    if (diff)
        _colliderChangesColor=changes;
    return(diff);
}

bool CCollisionObject_old::setCollideeChangesColor(bool changes)
{
    bool diff=(_collideeChangesColor!=changes);
    if (diff)
        _collideeChangesColor=changes;
    return(diff);
}

bool CCollisionObject_old::setExhaustiveDetection(bool exhaustive)
{
    bool diff=(_detectAllCollisions!=exhaustive);
    if (diff)
        _detectAllCollisions=exhaustive;
    return(diff);
}

bool CCollisionObject_old::setExplicitHandling(bool explicitHandl)
{
    bool diff=(_explicitHandling!=explicitHandl);
    if (diff)
        _explicitHandling=explicitHandl;
    return(diff);
}

bool CCollisionObject_old::setContourWidth(int w)
{
    w=tt::getLimitedInt(1,4,w);
    bool diff=(_countourWidth!=w);
    if (diff)
        _countourWidth=w;
    return(diff);
}

bool CCollisionObject_old::setIntersections(const std::vector<double>* intersections)
{
    bool diff=false;
    if (intersections==nullptr)
    {
        diff=(_intersections.size()>0);
        if (diff)
            _intersections.clear();
    }
    else
    {
        diff=(intersections->size()!=_intersections.size());
        if (!diff)
        {
            for (size_t i=0;i<intersections->size();i++)
            {
                if (intersections->at(i)!=_intersections[i])
                {
                    diff=true;
                    break;
                }
            }
        }
        if (diff)
            _intersections.assign(intersections->begin(),intersections->end());
    }
    return(diff);
}

