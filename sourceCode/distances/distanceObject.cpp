#include "simInternal.h"
#include "distanceObject.h"
#include "sceneObject.h"
#include "global.h"
#include "distanceRoutines.h"
#include "tt.h"
#include "ttUtil.h"
#include "app.h"
#include "gV.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "pluginContainer.h"
#include "distanceRendering.h"
#include "base64.h"

CDistanceObject::CDistanceObject()
{
    _commonInit();
}

CDistanceObject::CDistanceObject(int entity1Handle,int entity2Handle)
{
    _commonInit();
    _entity1Handle=entity1Handle;
    _entity2Handle=entity2Handle;
}

CDistanceObject::~CDistanceObject()
{
}

void CDistanceObject::_commonInit()
{
    _uniquePersistentIdString=CTTUtil::generateUniqueReadableString(); // persistent
    for (size_t i=0;i<7;i++)
        _distanceResult[i]=0.0f;
    _distanceBuffer[0]=-1;
    _distanceBuffer[1]=-1;
    _distanceBuffer[2]=-1;
    _distanceBuffer[3]=-1;
    _distanceIsValid=false;
    _segmentColor.setColorsAllBlack();
    _initialValuesInitialized=false;
    _calcTimeInMs=0;
}

void CDistanceObject::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
}

void CDistanceObject::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CDistanceObject::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::currentWorld->simulation->getResetSceneAtSimulationEnd())
    {
        _CDistanceObject_::setExplicitHandling(_initialExplicitHandling);
    }
    _initialValuesInitialized=false;
}

bool CDistanceObject::isSame(int entity1Handle,int entity2Handle) const
{
    if ( (entity1Handle==_entity1Handle)&&(entity2Handle==_entity2Handle) )
        return(true);
    if ( (entity2Handle==_entity1Handle)&&(entity1Handle==_entity2Handle) )
        return(true);
    return(false);
}

bool CDistanceObject::getDistanceResult(float dist[7]) const
{ // Return value false means the distance was not measured or above the specified treshhold.
    if (_distanceIsValid)
    {
        dist[0]=_distanceResult[0];
        dist[1]=_distanceResult[1];
        dist[2]=_distanceResult[2];
        dist[3]=_distanceResult[3];
        dist[4]=_distanceResult[4];
        dist[5]=_distanceResult[5];
        dist[6]=_distanceResult[6];
    }
    return(_distanceIsValid);
}

float CDistanceObject::getCalculationTime() const
{
    return(float(_calcTimeInMs)*0.001f);
}

float CDistanceObject::readDistance() const
{
    if (_distanceIsValid)
        return(_distance);
    return(-1.0f); // keep this negative. Indicates an invalid distance
}

std::string CDistanceObject::getUniquePersistentIdString() const
{
    return(_uniquePersistentIdString);
}

std::string CDistanceObject::getObjectDescriptiveName() const
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

bool CDistanceObject::setObjectName(const char* newName,bool check)
{ // Overridden from _CDistanceObject_
    bool diff=false;
    CDistanceObject* it=nullptr;
    if (check)
        it=App::currentWorld->distances->getObjectFromHandle(_objectHandle);
    if (it!=this)
        diff=_CDistanceObject_::setObjectName(newName,check); // no checking or object not yet in world
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm,true);
        if (nm.size()>0)
        {
            if (getObjectName()!=nm)
            {
                while (App::currentWorld->distances->getObjectFromName(nm.c_str())!=nullptr)
                    nm=tt::generateNewName_hashOrNoHash(nm,!tt::isHashFree(nm.c_str()));
                diff=_CDistanceObject_::setObjectName(nm.c_str(),check);
            }
        }
    }
    return(diff);
}

bool CDistanceObject::setThreshold(float tr)
{ // Overridden from _CDistanceObject_
    tt::limitValue(0.0001f,10000.0f,tr);
    return(_CDistanceObject_::setThreshold(tr));
}

void CDistanceObject::clearDistanceResult()
{
    _distanceIsValid=false;
    _calcTimeInMs=0;
}

float CDistanceObject::handleDistance()
{
    clearDistanceResult();
    if (!App::currentWorld->mainSettings->distanceCalculationEnabled)
        return(-1.0);
    if (!CPluginContainer::isGeomPluginAvailable())
        return(-1.0);
    int stTime=VDateTime::getTimeInMs();
    _distance=SIM_MAX_FLOAT;
    if (_thresholdEnabled)
        _distance=_threshold;
    if (CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(_entity1Handle,_entity2Handle,_distance,_distanceResult,_distanceBuffer,_distanceBuffer+2,false,false))
    {
        _distanceResult[6]=_distance;
        _distanceIsValid=true;
        _calcTimeInMs=VDateTime::getTimeDiffInMs(stTime);
    }
    return(readDistance());
}

bool CDistanceObject::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{ // Return value true means that this distance object should be destroyed
    if (_entity1Handle==objectHandle)
        return(true);
    if (_entity2Handle==objectHandle)
        return(true);
    return(false);
}

bool CDistanceObject::announceCollectionWillBeErased(int collectionHandle,bool copyBuffer)
{ // Return value true means that this distance object should be destroyed
    if (_entity1Handle==collectionHandle)
        return(true);
    if (_entity2Handle==collectionHandle)
        return(true);
    return(false);
}

void CDistanceObject::performObjectLoadingMapping(const std::vector<int>* map)
{
    if (_entity1Handle<SIM_IDSTART_COLLECTION)
        _entity1Handle=CWorld::getLoadingMapping(map,_entity1Handle);
    if ( (_entity2Handle<SIM_IDSTART_COLLECTION)&&(_entity2Handle!=-1) )
        _entity2Handle=CWorld::getLoadingMapping(map,_entity2Handle);
}

void CDistanceObject::performCollectionLoadingMapping(const std::vector<int>* map)
{
    if (_entity1Handle>=SIM_IDSTART_COLLECTION)
        _entity1Handle=CWorld::getLoadingMapping(map,_entity1Handle);
    if (_entity2Handle>=SIM_IDSTART_COLLECTION)
        _entity2Handle=CWorld::getLoadingMapping(map,_entity2Handle);
}

CDistanceObject* CDistanceObject::copyYourself()
{
    CDistanceObject* newDistObj=new CDistanceObject();
    newDistObj->_objectHandle=_objectHandle; // important for copy operations connections
    newDistObj->_entity1Handle=_entity1Handle;
    newDistObj->_entity2Handle=_entity2Handle;
    newDistObj->_objectName=_objectName;
    newDistObj->_threshold=_threshold;
    newDistObj->_thresholdEnabled=_thresholdEnabled;
    newDistObj->_displaySegment=_displaySegment;
    newDistObj->_explicitHandling=_explicitHandling;

    newDistObj->_segmentWidth=_segmentWidth;
    _segmentColor.copyYourselfInto(&newDistObj->_segmentColor);

    newDistObj->_initialValuesInitialized=_initialValuesInitialized;
    newDistObj->_initialExplicitHandling=_initialExplicitHandling;

    return(newDistObj);
}

bool CDistanceObject::setSegmentWidth(int w)
{ // Overridden from _CDistanceObject_
    w=tt::getLimitedInt(1,4,w);
    return(_CDistanceObject_::setSegmentWidth(w));
}

void CDistanceObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Inx");
            ar << _entity1Handle << _entity2Handle << _objectHandle;
            ar.flush();

            ar.storeDataName("Trh");
            ar << _threshold;
            ar.flush();

            ar.storeDataName("Swt");
            ar << _segmentWidth;
            ar.flush();

            ar.storeDataName("Col");
            ar.setCountingMode();
            _segmentColor.serialize(ar,1);
            if (ar.setWritingMode())
                _segmentColor.serialize(ar,1);

            ar.storeDataName("Var");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_thresholdEnabled);
            SIM_SET_CLEAR_BIT(nothing,1,!_displaySegment);
            SIM_SET_CLEAR_BIT(nothing,2,_explicitHandling);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Nme");
            ar << _objectName;
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
                    if (theName.compare("Inx")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _entity1Handle >> _entity2Handle >> _objectHandle;
                    }
                    if (theName.compare("Trh")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _threshold;
                    }
                    if (theName.compare("Swt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _segmentWidth;
                    }
                    if (theName.compare("Col")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        _segmentColor.serialize(ar,1);
                    }
                    if (theName.compare("Var")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _thresholdEnabled=SIM_IS_BIT_SET(nothing,0);
                        _displaySegment=!SIM_IS_BIT_SET(nothing,1);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,2);
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
            ar.xmlAddNode_bool("explicitHandling",_explicitHandling);
            ar.xmlPopNode();

            ar.xmlPushNewNode("threshold");
            ar.xmlAddNode_bool("enabled",_thresholdEnabled);
            ar.xmlAddNode_float("treshhold",_threshold);
            ar.xmlPopNode();

            ar.xmlPushNewNode("distanceSegment");
            ar.xmlAddNode_bool("show",_displaySegment);
            ar.xmlAddNode_int("size",_segmentWidth);
            ar.xmlPushNewNode("color");
            _segmentColor.serialize(ar,1);
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
                ar.xmlGetNode_bool("explicitHandling",_explicitHandling);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("threshold"))
            {
                ar.xmlGetNode_bool("enabled",_thresholdEnabled);
                ar.xmlGetNode_float("treshhold",_threshold);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("distanceSegment"))
            {
                ar.xmlGetNode_bool("show",_displaySegment);
                ar.xmlGetNode_int("size",_segmentWidth);
                if (ar.xmlPushChildNode("color"))
                {
                    _segmentColor.serialize(ar,1);
                    ar.xmlPopNode();
                }
                ar.xmlPopNode();
            }

            if (ar.xmlGetNode_string("uniquePersistentIdString_base64Coded",_uniquePersistentIdString))
                _uniquePersistentIdString=base64_decode(_uniquePersistentIdString);
        }
    }
}

void CDistanceObject::displayDistanceSegment()
{
    if ( (!_distanceIsValid)||(!_displaySegment) )
        return;
    displayDistance(this,_segmentWidth,_distanceResult);
}

void CDistanceObject::buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting)
{ // Overridden from CSyncObject
    if (setObjectCanSync(true))
    {
        // Set routing:
        SSyncRoute r;
        r.objHandle=_objectHandle;
        r.objType=sim_syncobj_distance;
        setSyncMsgRouting(parentRouting,r);

        // Build remote distance object:
        void* data[2];
        data[0]=&_entity1Handle;
        data[1]=&_entity2Handle;
        sendRandom(data,2,sim_syncobj_distanceobject_create);

        // Update the remote object:
        _setExplicitHandling_send(_explicitHandling);
        _setObjectName_send(_objectName.c_str());
        _setThreshold_send(_threshold);
        _setThresholdEnabled_send(_thresholdEnabled);
        _setDisplaySegment_send(_displaySegment);
        _setSegmentWidth_send(_segmentWidth);

        // Update the color object:
        _segmentColor.buildUpdateAndPopulateSynchronizationObject(getSyncMsgRouting());
    }
}

void CDistanceObject::connectSynchronizationObject()
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
    }
}

void CDistanceObject::removeSynchronizationObject(bool localReferencesToItOnly)
{ // Overridden from CSyncObject
    if (getObjectCanSync())
    {
        setObjectCanSync(false);

        if (!localReferencesToItOnly)
        {
            // Delete remote distance object:
            sendVoid(sim_syncobj_distanceobject_delete);
        }
    }
}
