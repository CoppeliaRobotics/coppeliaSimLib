#include "simInternal.h"
#include "collisionObject.h"
#include "sceneObject.h"
#include "global.h"
#include "collisionRoutines.h"
#include "tt.h"
#include "ttUtil.h"
#include "app.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "pluginContainer.h"
#include "collisionContourRendering.h"
#include "base64.h"

CCollisionObject::CCollisionObject()
{
    _commonInit();
}

CCollisionObject::CCollisionObject(int entity1Handle,int entity2Handle)
{
    _commonInit();
    _entity1Handle=entity1Handle;
    _entity2Handle=entity2Handle;
}

CCollisionObject::~CCollisionObject()
{
}

void CCollisionObject::_commonInit()
{
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    _collisionResult=false;
    _collisionResultValid=false;
    _collObjectHandles[0]=-1;
    _collObjectHandles[1]=-1;
    _initialValuesInitialized=false;
    _calcTimeInMs=0;
    _contourColor.setColorsAllBlack();
    _contourColor.setColor(1.0f,1.0f,1.0f,sim_colorcomponent_emission);
}

void CCollisionObject::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
}

void CCollisionObject::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CCollisionObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    {
        _CCollisionObject_::setExplicitHandling(_initialExplicitHandling);
    }
    _initialValuesInitialized=false;
}

bool CCollisionObject::isSame(int entity1Handle,int entity2Handle) const
{
    if ( (entity1Handle==_entity1Handle)&&(entity2Handle==_entity2Handle) )
        return(true);
    if ( (entity2Handle==_entity1Handle)&&(entity1Handle==_entity2Handle) )
        return(true);
    return(false);
}

std::string CCollisionObject::getObjectDescriptiveName() const
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
        theName+=it->getObjectName();
    }
    else
    {
        CCollection* it=App::currentWorld->collections->getObjectFromHandle(_entity1Handle);
        if (it!=nullptr)
        {
            theName+=strTranslate(IDSN_COLLECTION);
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
            theName+=strTranslate(IDSN_COLLECTION);
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
            theName+=it->getObjectName();
        }
        else
            theName+=strTranslate(IDS_ALL_OTHER_ENTITIES);
    }
    theName=theName.append(")");    
    return(theName);
}

int CCollisionObject::getCollisionColor(int entityHandle) const
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

bool CCollisionObject::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{ // Return value true means that this collision object should be destroyed
    if (_entity1Handle==objectHandle)
        return(true);
    if (_entity2Handle==objectHandle)
        return(true);
    return(false);
}

bool CCollisionObject::announceCollectionWillBeErased(int collectionHandle,bool copyBuffer)
{ // Return value true means that this collision object should be destroyed
    if (_entity1Handle==collectionHandle)
        return(true);
    if (_entity2Handle==collectionHandle)
        return(true);
    return(false);
}

bool CCollisionObject::canComputeCollisionContour() const
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


void CCollisionObject::clearCollisionResult()
{
    _clearCollisionResult();
}

bool CCollisionObject::setObjectName(const char* newName,bool check)
{ // Overridden from _CCollisionObject_
    bool diff=false;
    CCollisionObject* it=nullptr;
    if (check)
        it=App::currentWorld->collisions->getObjectFromHandle(_objectHandle);
    if (it!=this)
        diff=_CCollisionObject_::setObjectName(newName,check); // no checking or object not yet in world
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if (nm.size()>0)
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->collisions->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm,!tt::isHashFree(nm.c_str()));
                diff=_CCollisionObject_::setObjectName(nm.c_str(),check);
            }
        }
    }
    return(diff);
}


bool CCollisionObject::setContourWidth(int w)
{ // Overridden from _CCollisionObject_
    w=tt::getLimitedInt(1,4,w);
    return(_CCollisionObject_::setContourWidth(w));
}

bool CCollisionObject::handleCollision()
{   // Return value true means there was a collision
    clearCollisionResult();
    if (!App::currentWorld->mainSettings->collisionDetectionEnabled)
        return(false);
    if (!CPluginContainer::isGeomPluginAvailable())
        return(false);
    int stT=VDateTime::getTimeInMs();
    int collObjs[2];
    bool res=false;
    std::vector<float> collCont;
    if (_detectAllCollisions)
        res=CCollisionRoutine::doEntitiesCollide(_entity1Handle,_entity2Handle,&collCont,false,false,collObjs);
    else
        res=CCollisionRoutine::doEntitiesCollide(_entity1Handle,_entity2Handle,nullptr,false,false,collObjs);
    _setCollisionResult(res,VDateTime::getTimeDiffInMs(stT),collObjs[0],collObjs[1],collCont);
    return(_collisionResult);
}

int CCollisionObject::readCollision(int collObjHandles[2]) const
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

void CCollisionObject::performObjectLoadingMapping(const std::vector<int>* map)
{
    if (_entity1Handle<SIM_IDSTART_COLLECTION)
        _entity1Handle=CWorld::getLoadingMapping(map,_entity1Handle);
    if ( (_entity2Handle<SIM_IDSTART_COLLECTION)&&(_entity2Handle!=-1) )
        _entity2Handle=CWorld::getLoadingMapping(map,_entity2Handle);
}

void CCollisionObject::performCollectionLoadingMapping(const std::vector<int>* map)
{
    if (_entity1Handle>=SIM_IDSTART_COLLECTION)
        _entity1Handle=CWorld::getLoadingMapping(map,_entity1Handle);
    if (_entity2Handle>=SIM_IDSTART_COLLECTION)
        _entity2Handle=CWorld::getLoadingMapping(map,_entity2Handle);
}

CCollisionObject* CCollisionObject::copyYourself()
{
    CCollisionObject* newCollObj=new CCollisionObject();
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

void CCollisionObject::serialize(CSer& ar)
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
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
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

void CCollisionObject::displayCollisionContour()
{
    displayContour(this,_countourWidth);
}

std::string CCollisionObject::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

void CCollisionObject::_clearCollisionResult()
{
    _collisionResult=false;
    _collisionResultValid=false;
    _collObjectHandles[0]=-1;
    _collObjectHandles[1]=-1;
    _calcTimeInMs=0;
    _CCollisionObject_::setIntersections(nullptr);
}

void CCollisionObject::_setCollisionResult(bool result,int calcTime,int obj1Handle,int obj2Handle,const std::vector<float>& intersect)
{
    _collisionResult=result;
    _collisionResultValid=true;
    _collObjectHandles[0]=obj1Handle;
    _collObjectHandles[1]=obj2Handle;
    _calcTimeInMs=calcTime;
    _CCollisionObject_::setIntersections(&intersect);
}

bool CCollisionObject::isCollisionResultValid() const
{
    return(_collisionResultValid);
}

bool CCollisionObject::getCollisionResult() const
{
    return(_collisionResult);
}

float CCollisionObject::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

void CCollisionObject::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_objectHandle;
        r.objType=sim_syncobj_collision;
        setSyncMsgRouting(parentRouting,r);

        // Build remote collision object:
        void* data[2];
        data[0]=&_entity1Handle;
        data[1]=&_entity2Handle;
        sendRandom(data,2,sim_syncobj_collisionobject_create);

        // Update the remote object:
        _setExplicitHandling_send(_explicitHandling);
        _setObjectName_send(_objectName.c_str());
        _setColliderChangesColor_send(_colliderChangesColor);
        _setCollideeChangesColor_send(_collideeChangesColor);
        _setExhaustiveDetection_send(_detectAllCollisions);
        _setContourWidth_send(_countourWidth);
        _setIntersections_send(&_intersections);

        // Update the color object:
        _contourColor.buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
    }
}

void CCollisionObject::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
    }
}

void CCollisionObject::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Delete remote collision object:
            sendVoid(sim_syncobj_collisionobject_delete);
        }
    }
}
