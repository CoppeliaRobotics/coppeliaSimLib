#include "sceneObjectContainer.h"
#include "app.h"
#include "addOperations.h"
#include "sceneObjectOperations.h"
#include "fileOperations.h"
#include "tt.h"
#include "ttUtil.h"
#include "pluginContainer.h"
#include "mesh.h"
#include <sstream>
#include <iostream>

CSceneObjectContainer::CSceneObjectContainer()
{
    _objectActualizationEnabled=true;
    _nextObjectHandle=SIM_IDSTART_SCENEOBJECT;
}

CSceneObjectContainer::~CSceneObjectContainer()
{ // beware, the current world could be nullptr
    removeAllObjects(false); // should already have been done
}

void CSceneObjectContainer::simulationAboutToStart()
{
    CSceneObject::incrementModelPropertyValidityNumber();
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->simulationAboutToStart();
}

void CSceneObjectContainer::simulationEnded()
{
    for (size_t i=0;i<getObjectCount();i++)
       getObjectFromIndex(i)->simulationEnded();
    CSceneObject::incrementModelPropertyValidityNumber();
}

void CSceneObjectContainer::announceObjectWillBeErased(int objectHandle)
{
    TRACE_INTERNAL;
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        if (it->getObjectHandle()!=objectHandle)
            it->announceObjectWillBeErased(objectHandle,false);
    }
}

void CSceneObjectContainer::announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    TRACE_INTERNAL;
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        it->announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript,false);
    }
}

void CSceneObjectContainer::announceIkGroupWillBeErased(int ikGroupHandle)
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->announceIkObjectWillBeErased(ikGroupHandle,false); // this never triggers scene object destruction!
}

void CSceneObjectContainer::announceCollectionWillBeErased(int collectionHandle)
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->announceCollectionWillBeErased(collectionHandle,false); // this never triggers scene object destruction!
}

void CSceneObjectContainer::announceCollisionWillBeErased(int collisionHandle)
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->announceCollisionWillBeErased(collisionHandle,false); // this never triggers scene object destruction!
}

void CSceneObjectContainer::announceDistanceWillBeErased(int distanceHandle)
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->announceDistanceWillBeErased(distanceHandle,false); // this never triggers scene object destruction!
}

bool CSceneObjectContainer::addObjectToScene(CSceneObject* newObject,bool objectIsACopy,bool generateAfterCreateCallback)
{
    return(addObjectToSceneWithSuffixOffset(newObject,objectIsACopy,1,generateAfterCreateCallback));
}

bool CSceneObjectContainer::addObjectToSceneWithSuffixOffset(CSceneObject* newObject,bool objectIsACopy,int suffixOffset,bool generateAfterCreateCallback)
{
    App::currentWorld->environment->setSceneCanBeDiscardedWhenNewSceneOpened(false); // 4/3/2012

    std::string newObjName=newObject->getObjectName_old();
    if (objectIsACopy)
        newObjName=tt::generateNewName_hash(newObjName.c_str(),suffixOffset);
    else
    {
        if (getObjectFromName(newObjName.c_str())!=nullptr)
        {
            // Following faster with many objects:
            std::string baseName(tt::getNameWithoutSuffixNumber(newObjName.c_str(),false));
            int initialSuffix=tt::getNameSuffixNumber(newObjName.c_str(),false);
            std::vector<int> suffixes;
            std::vector<int> dummyValues;
            for (size_t i=0;i<getObjectCount();i++)
            {
                std::string baseNameIt(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(),false));
                if (baseName.compare(baseNameIt)==0)
                {
                    suffixes.push_back(tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(),false));
                    dummyValues.push_back(0);
                }
            }
            tt::orderAscending(suffixes,dummyValues);
            int lastS=-1;
            for (size_t i=0;i<suffixes.size();i++)
            {
                if ( (suffixes[i]>initialSuffix)&&(suffixes[i]>lastS+1) )
                    break;
                lastS=suffixes[i];
            }
            newObjName=tt::generateNewName_noHash(baseName.c_str(),lastS+1+1);
        }
        // Following was too slow with many objects:
        //      while (getObject(newObjName)!=nullptr)
        //          newObjName=tt::generateNewName_noHash(newObjName);
    }
    newObject->setObjectName_direct_old(newObjName.c_str());

    // Same but with the alt object names:
    std::string newObjAltName=newObject->getObjectAltName_old();
    if (getObjectFromAltName(newObjAltName.c_str())!=nullptr)
    {
        // Following faster with many objects:
        std::string baseAltName(tt::getNameWithoutSuffixNumber(newObjAltName.c_str(),false));
        int initialSuffix=tt::getNameSuffixNumber(newObjAltName.c_str(),false);
        std::vector<int> suffixes;
        std::vector<int> dummyValues;
        for (size_t i=0;i<getObjectCount();i++)
        {
            std::string baseAltNameIt(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectAltName_old().c_str(),false));
            if (baseAltName.compare(baseAltNameIt)==0)
            {
                suffixes.push_back(tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectAltName_old().c_str(),false));
                dummyValues.push_back(0);
            }
        }
        tt::orderAscending(suffixes,dummyValues);
        int lastS=-1;
        for (size_t i=0;i<suffixes.size();i++)
        {
            if ( (suffixes[i]>initialSuffix)&&(suffixes[i]>lastS+1) )
                break;
            lastS=suffixes[i];
        }
        newObjAltName=tt::generateNewName_noHash(baseAltName.c_str(),lastS+1+1);
    }
    // Following was too slow with many objects:
    //      while (getObjectFromAltName(newObjAltName)!=nullptr)
    //          newObjAltName=tt::generateNewName_noHash(newObjAltName);
    newObject->setObjectAltName_direct_old(newObjAltName.c_str());

    // Give the object a new handle
    int handle=_nextObjectHandle;
    while (getObjectFromHandle(handle)!=nullptr)
    {
        handle++;
        if (handle>=(SIM_IDEND_SCENEOBJECT-SIM_IDSTART_SCENEOBJECT))
            handle=SIM_IDSTART_SCENEOBJECT;
    }
    _nextObjectHandle=handle+1;
    if (_nextObjectHandle>=(SIM_IDEND_SCENEOBJECT-SIM_IDSTART_SCENEOBJECT))
        _nextObjectHandle=SIM_IDSTART_SCENEOBJECT;

    newObject->setObjectHandle(handle);

    _addObject(newObject);

    if (newObject->getObjectType()==sim_object_graph_type)
    { // If the simulation is running, we have to empty the buffer!!! (otherwise we might have old and new data mixed together (e.g. old data in future, new data in present!)
        if ((App::currentWorld->simulation!=nullptr)&&(!App::currentWorld->simulation->isSimulationStopped()))
        {
            CGraph* graph=(CGraph*)newObject;
            graph->resetGraph();
        }
    }

    if (generateAfterCreateCallback)
    {
        CInterfaceStack stack;
        stack.pushTableOntoStack();
        stack.pushStringOntoStack("objectHandles",0);
        stack.pushTableOntoStack();
        stack.pushNumberOntoStack(double(1)); // key or index
        stack.pushNumberOntoStack(newObject->getObjectHandle());
        stack.insertDataIntoStackTable();
        stack.insertDataIntoStackTable();
        App::worldContainer->callScripts(sim_syscb_aftercreate,&stack);
    }
    App::worldContainer->setModificationFlag(2); // object created

    return(true);
}

bool CSceneObjectContainer::eraseObject(CSceneObject* it,bool generateBeforeAfterDeleteCallback)
{
    deselectObjects();

    if (it==nullptr)
        return(false);

    CInterfaceStack stack;
    if (generateBeforeAfterDeleteCallback)
    {
        stack.pushTableOntoStack();
        stack.pushStringOntoStack("objectHandles",0);
        stack.pushTableOntoStack();
        stack.pushNumberOntoStack(double(it->getObjectHandle())); // key or index
        stack.pushBoolOntoStack(true);
        stack.insertDataIntoStackTable();
        stack.insertDataIntoStackTable();
        stack.pushStringOntoStack("allObjects",0);
        stack.pushBoolOntoStack(getObjectCount()==1);
        stack.insertDataIntoStackTable();
        App::worldContainer->callScripts(sim_syscb_beforedelete,&stack);
    }

    // We announce the object will be erased:
    App::currentWorld->announceObjectWillBeErased(it->getObjectHandle()); // this may trigger other "interesting" things, such as customization script runs, etc.
    deselectObjects(); // to make sure, since above might have changed selection again

    _removeObject(it);

    App::worldContainer->setModificationFlag(1); // object erased

    if (generateBeforeAfterDeleteCallback)
        App::worldContainer->callScripts(sim_syscb_afterdelete,&stack);

    return(true);
}

void CSceneObjectContainer::eraseSeveralObjects(const std::vector<int>& objectHandles,bool generateBeforeAfterDeleteCallback)
{
    if (objectHandles.size()>0)
    {
        CInterfaceStack stack;
        if (generateBeforeAfterDeleteCallback)
        {
            stack.pushTableOntoStack();
            stack.pushStringOntoStack("objectHandles",0);
            stack.pushTableOntoStack();
            for (size_t i=0;i<objectHandles.size();i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandles[i]);
                if ( (it!=nullptr)&&it->setBeforeDeleteCallbackSent() )
                { // send the message only once. This routine can be reentrant!
                    stack.pushInt32OntoStack(objectHandles[i]); // key or index
                    stack.pushBoolOntoStack(true);
                    stack.insertDataIntoStackTable();
                }
            }
            stack.insertDataIntoStackTable();
            stack.pushStringOntoStack("allObjects",0);
            stack.pushBoolOntoStack(objectHandles.size()==getObjectCount());
            stack.insertDataIntoStackTable();
            App::worldContainer->callScripts(sim_syscb_beforedelete,&stack);
        }

        for (size_t i=0;i<objectHandles.size();i++)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandles[i]);
            if (it!=nullptr)
                eraseObject(it,false);
        }

        if (generateBeforeAfterDeleteCallback)
            App::worldContainer->callScripts(sim_syscb_afterdelete,&stack);
    }
}

void CSceneObjectContainer::removeAllObjects(bool generateBeforeAfterDeleteCallback)
{
#ifdef SIM_WITH_GUI
    if (App::mainWindow!=nullptr)
        App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr);
#endif
    if (getObjectCount()>0)
        deselectObjects();

    std::vector<int> toRemove;
    for (size_t i=0;i<getObjectCount();i++)
        toRemove.push_back(getObjectFromIndex(i)->getObjectHandle());
    eraseSeveralObjects(toRemove,generateBeforeAfterDeleteCallback);
    // The above destroys all sceneObjects, and normally automatically
    // all resources linked to them. So we don't have to destroy the resources

    // Following new since 30/7/2014:
    // ideally we want to always use different object handles so that if the user erases an object and
    // creates a new one just after, the erased object's handle is not reused. That's why we have
    // the _nextObjectHandle variable.
    // The undo/redo functionality however doesn't like that, since at each undo/redo, all objects
    // get a new handle, and so the scene memory print looks different, even if the scenes are
    // identical: the undo/redo then marks a new restore point, which is not correct.
    // So, finally, when the whole scene gets emptied at least we make sure that all handles
    // start from the beginning:
    _nextObjectHandle=SIM_IDSTART_SCENEOBJECT;
}

void CSceneObjectContainer::actualizeObjectInformation()
{
    if (_objectActualizationEnabled)
    {
        // We actualize the direct linked joint list of each joint:
        for (size_t i=0;i<getJointCount();i++)
        {
            CJoint* it=getJointFromIndex(i);
            std::vector<CJoint*> joints;
            for (size_t j=0;j<getJointCount();j++)
            {
                CJoint* anAct=getJointFromIndex(j);
                if (anAct!=it)
                {
                    if ((anAct->getJointMode()==sim_jointmode_dependent)||(anAct->getJointMode()==sim_jointmode_reserved_previously_ikdependent))
                    {
                        if (anAct->getDependencyMasterJointHandle()==it->getObjectHandle())
                            joints.push_back(anAct);
                    }
                }
            }
            it->setDirectDependentJoints(joints);
        }

        App::currentWorld->collections->actualizeAllCollections();

        for (size_t i=0;i<getShapeCount();i++)
            getShapeFromIndex(i)->clearLastParentForLocalGlobalRespondable();

        App::currentWorld->textureContainer->updateAllDependencies();
        CSceneObject::incrementModelPropertyValidityNumber();
        App::setRebuildHierarchyFlag();


    }
}

void CSceneObjectContainer::enableObjectActualization(bool e)
{
    _objectActualizationEnabled=e;
}

void CSceneObjectContainer::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const
{
    minSuffix=-1;
    maxSuffix=-1;
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(),true);
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

bool CSceneObjectContainer::canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(),true));
            for (size_t j=0;j<getObjectCount();j++)
            {
                int s2=tt::getNameSuffixNumber(getObjectFromIndex(j)->getObjectName_old().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(getObjectFromIndex(j)->getObjectName_old().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CSceneObjectContainer::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        int s1=tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(),true);
        if (s1==suffix1)
        {
            CSceneObject* obj=getObjectFromIndex(i);
            std::string name1(tt::getNameWithoutSuffixNumber(obj->getObjectName_old().c_str(),true));
            setObjectName_old(obj,tt::generateNewName_hash(name1.c_str(),suffix2+1).c_str(),true);
        }
    }
}

void CSceneObjectContainer::setTextureDependencies()
{ // here we cannot use shapeList, because that list may not yet be actualized (e.g. during a scene/model load operation)!!
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        if (it->getObjectType()==sim_object_shape_type)
        {
            if (((CShape*)it)->getMeshWrapper()!=nullptr)
                ((CShape*)it)->getMeshWrapper()->setTextureDependencies(it->getObjectHandle());
        }
    }
}

void CSceneObjectContainer::removeSceneDependencies()
{
    for (size_t i=0;i<getObjectCount();i++)
        getObjectFromIndex(i)->removeSceneDependencies();
}

void CSceneObjectContainer::getAllCollidableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        if (it->isPotentiallyCollidable())
        {
            if (it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_collidable)
            { // Make sure we don't have it in the exception list:
                bool okToAdd=true;
                if (exceptionObjects!=nullptr)
                {
                    for (size_t j=0;j<exceptionObjects->size();j++)
                    {
                        if (exceptionObjects->at(j)==it)
                        {
                            okToAdd=false;
                            break;
                        }
                    }
                }
                if (okToAdd)
                    objects.push_back(it);
            }
        }
    }
}

void CSceneObjectContainer::getAllMeasurableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        if (it->isPotentiallyMeasurable())
        {
            if (it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)
            { // Make sure we don't have it in the exception list:
                bool okToAdd=true;
                if (exceptionObjects!=nullptr)
                {
                    for (size_t j=0;j<exceptionObjects->size();j++)
                    {
                        if (exceptionObjects->at(j)==it)
                        {
                            okToAdd=false;
                            break;
                        }
                    }
                }
                if (okToAdd)
                    objects.push_back(it);
            }
        }
    }
}

void CSceneObjectContainer::getAllDetectableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects,int detectableMask)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        if (it->isPotentiallyDetectable())
        {
            if ( (it->getCumulativeObjectSpecialProperty()&detectableMask)||(detectableMask==-1) )
            { // Make sure we don't have it in the exception list:
                bool okToAdd=true;
                if (exceptionObjects!=nullptr)
                {
                    for (size_t j=0;j<exceptionObjects->size();j++)
                    {
                        if (exceptionObjects->at(j)==it)
                        {
                            okToAdd=false;
                            break;
                        }
                    }
                }
                if (okToAdd)
                    objects.push_back(it);
            }
        }
    }
}

void CSceneObjectContainer::exportIkContent(CExtIkSer& ar)
{
    ar.writeInt(int(getObjectCount()));

    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        ar.writeInt(it->getObjectType());
        it->serializeWExtIk(ar);
    }
}

CSceneObject* CSceneObjectContainer::readSceneObject(CSer& ar,const char* name,bool& noHit)
{
    if (ar.isBinary())
    {
        int byteQuantity;
        std::string theName(name);
        if (theName.compare(SER_SHAPE)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CShape* myNewObject=new CShape();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_JOINT)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CJoint* myNewObject=new CJoint();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_GRAPH)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CGraph* myNewObject=new CGraph();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_CAMERA)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CCamera* myNewObject=new CCamera();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_LIGHT)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CLight* myNewObject=new CLight();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_MIRROR)==0)
        {
            if (App::userSettings->xrTest==123456789)
                App::logMsg(sim_verbosity_errors,"Contains mirrors...");
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CMirror* myNewObject=new CMirror();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_OCTREE)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            COctree* myNewObject=new COctree();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_POINTCLOUD)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CPointCloud* myNewObject=new CPointCloud();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_DUMMY)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CDummy* myNewObject=new CDummy();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_PROXIMITYSENSOR)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CProxSensor* myNewObject=new CProxSensor();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_VISIONSENSOR)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CVisionSensor* myNewObject=new CVisionSensor();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_PATH_OLD)==0)
        {
            if (App::userSettings->xrTest==123456789)
                App::logMsg(sim_verbosity_errors,"Contains path objects...");
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CPath_old* myNewObject=new CPath_old();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_MILL)==0)
        {
            if (App::userSettings->xrTest==123456789)
                App::logMsg(sim_verbosity_errors,"Contains mills...");
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CMill* myNewObject=new CMill();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }
        if (theName.compare(SER_FORCESENSOR)==0)
        {
            ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
            CForceSensor* myNewObject=new CForceSensor();
            myNewObject->serialize(ar);
            noHit=false;
            return(myNewObject);
        }

        // If we arrived here it means that maybe we have a new scene object type that this CoppeliaSim doesn't understand yet.
        // We try to replace it with a dummy (2009/12/09):
        unsigned char dat[14];
        if (ar.readBytesButKeepPointerUnchanged(dat,14)!=14)
            return(nullptr); // No, this is not a scene object! (not enough to read)
        if ((dat[4]=='3')&&(dat[5]=='d')&&(dat[6]=='o')&&(dat[11]==57)&&(dat[12]==58)&&(dat[13]==59))
        { // yes we have a scene object of an unknown type!
            ar >> byteQuantity; // Undo/redo will never arrive here
            CDummy* newUnknownType=new CDummy();
            newUnknownType->loadUnknownObjectType(ar);
            noHit=false;
            return(newUnknownType);
        }
    }
    else
    {
        if (ar.xmlPushChildNode(SERX_SHAPE,false))
        {
            CShape* myNewObject=new CShape();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_JOINT,false))
        {
            CJoint* myNewObject=new CJoint();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_GRAPH,false))
        {
            CGraph* myNewObject=new CGraph();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_CAMERA,false))
        {
            CCamera* myNewObject=new CCamera();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_LIGHT,false))
        {
            CLight* myNewObject=new CLight();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_MIRROR,false))
        {
            CMirror* myNewObject=new CMirror();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_OCTREE,false))
        {
            COctree* myNewObject=new COctree();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_POINTCLOUD,false))
        {
            CPointCloud* myNewObject=new CPointCloud();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_DUMMY,false))
        {
            CDummy* myNewObject=new CDummy();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_PROXIMITYSENSOR,false))
        {
            CProxSensor* myNewObject=new CProxSensor();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_VISIONSENSOR,false))
        {
            CVisionSensor* myNewObject=new CVisionSensor();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_PATH_OLD,false))
        {
            CPath_old* myNewObject=new CPath_old();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_MILL,false))
        {
            CMill* myNewObject=new CMill();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_FORCESENSOR,false))
        {
            CForceSensor* myNewObject=new CForceSensor();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return(myNewObject);
        }
    }
    return(nullptr); // No, this is not a scene object!
}

void CSceneObjectContainer::writeSceneObject(CSer& ar,CSceneObject* it)
{
    if (ar.isBinary())
    {
        if (it->getObjectType()==sim_object_shape_type)
            ar.storeDataName(SER_SHAPE);
        if (it->getObjectType()==sim_object_joint_type)
            ar.storeDataName(SER_JOINT);
        if (it->getObjectType()==sim_object_graph_type)
            ar.storeDataName(SER_GRAPH);
        if (it->getObjectType()==sim_object_camera_type)
            ar.storeDataName(SER_CAMERA);
        if (it->getObjectType()==sim_object_light_type)
            ar.storeDataName(SER_LIGHT);
        if (it->getObjectType()==sim_object_mirror_type)
            ar.storeDataName(SER_MIRROR);
        if (it->getObjectType()==sim_object_octree_type)
            ar.storeDataName(SER_OCTREE);
        if (it->getObjectType()==sim_object_pointcloud_type)
            ar.storeDataName(SER_POINTCLOUD);
        if (it->getObjectType()==sim_object_dummy_type)
            ar.storeDataName(SER_DUMMY);
        if (it->getObjectType()==sim_object_proximitysensor_type)
            ar.storeDataName(SER_PROXIMITYSENSOR);
        if (it->getObjectType()==sim_object_visionsensor_type)
            ar.storeDataName(SER_VISIONSENSOR);
        if (it->getObjectType()==sim_object_path_type)
            ar.storeDataName(SER_PATH_OLD);
        if (it->getObjectType()==sim_object_mill_type)
            ar.storeDataName(SER_MILL);
        if (it->getObjectType()==sim_object_forcesensor_type)
            ar.storeDataName(SER_FORCESENSOR);
        ar.setCountingMode();
        it->serialize(ar);
        if (ar.setWritingMode())
            it->serialize(ar);
    }
    else
    {
        xmlNode* node;
        if (it->getObjectType()==sim_object_shape_type)
            ar.xmlPushNewNode(SERX_SHAPE);
        if (it->getObjectType()==sim_object_joint_type)
            ar.xmlPushNewNode(SERX_JOINT);
        if (it->getObjectType()==sim_object_graph_type)
            ar.xmlPushNewNode(SERX_GRAPH);
        if (it->getObjectType()==sim_object_camera_type)
            ar.xmlPushNewNode(SERX_CAMERA);
        if (it->getObjectType()==sim_object_light_type)
            ar.xmlPushNewNode(SERX_LIGHT);
        if (it->getObjectType()==sim_object_mirror_type)
            ar.xmlPushNewNode(SERX_MIRROR);
        if (it->getObjectType()==sim_object_octree_type)
            ar.xmlPushNewNode(SERX_OCTREE);
        if (it->getObjectType()==sim_object_pointcloud_type)
            ar.xmlPushNewNode(SERX_POINTCLOUD);
        if (it->getObjectType()==sim_object_dummy_type)
            ar.xmlPushNewNode(SERX_DUMMY);
        if (it->getObjectType()==sim_object_proximitysensor_type)
            ar.xmlPushNewNode(SERX_PROXIMITYSENSOR);
        if (it->getObjectType()==sim_object_visionsensor_type)
            ar.xmlPushNewNode(SERX_VISIONSENSOR);
        if (it->getObjectType()==sim_object_path_type)
            ar.xmlPushNewNode(SERX_PATH_OLD);
        if (it->getObjectType()==sim_object_mill_type)
            ar.xmlPushNewNode(SERX_MILL);
        if (it->getObjectType()==sim_object_forcesensor_type)
            ar.xmlPushNewNode(SERX_FORCESENSOR);
        it->serialize(ar);
        ar.xmlPopNode();
    }
}

bool CSceneObjectContainer::readAndAddToSceneSimpleXmlSceneObjects(CSer& ar,CSceneObject* parentObject,const C7Vector& localFramePreCorrection,std::vector<SSimpleXmlSceneObject>& simpleXmlObjects)
{
    bool retVal=true;
    bool isScene=(ar.getFileType()==CSer::filetype_csim_xml_simplescene_file);
    std::vector<std::string> allChildNodeNames;
    ar.xmlGetAllChildNodeNames(allChildNodeNames);
    for (size_t nodeIt=0;nodeIt<allChildNodeNames.size();nodeIt++)
    {
        std::string nm(allChildNodeNames[nodeIt]);
        if (nodeIt==0)
            ar.xmlPushChildNode(nm.c_str(),false);
        else
            ar.xmlPushSiblingNode(nm.c_str(),false);
        std::string objNames("shape*joint*graph*camera*dummy*proximitySensor*visionSensor*forceSensor*path*light*ocTree*pointCloud");
        if (objNames.find(nm)!=std::string::npos)
        {
            C7Vector desiredLocalFrame;
            CSceneObject* obj=nullptr;
            if (nm.compare("shape")==0)
                obj=_readSimpleXmlShape(ar,desiredLocalFrame); // special, added to scene already. Can fail
            if (nm.compare("joint")==0)
            {
                CJoint* myNewObject=new CJoint();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("graph")==0)
            {
                CGraph* myNewObject=new CGraph();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("camera")==0)
            {
                CCamera* myNewObject=new CCamera();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("dummy")==0)
            {
                CDummy* myNewObject=new CDummy();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("proximitySensor")==0)
            {
                CProxSensor* myNewObject=new CProxSensor();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("visionSensor")==0)
            {
                CVisionSensor* myNewObject=new CVisionSensor();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("forceSensor")==0)
            {
                CForceSensor* myNewObject=new CForceSensor();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("path")==0)
            {
                CPath_old* myNewObject=new CPath_old();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("light")==0)
            {
                CLight* myNewObject=new CLight();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("ocTree")==0)
            {
                COctree* myNewObject=new COctree();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }
            if (nm.compare("pointCloud")==0)
            {
                CPointCloud* myNewObject=new CPointCloud();
                myNewObject->serialize(ar);
                obj=myNewObject;
                addObjectToScene(obj,false,true);
            }

            if (obj!=nullptr)
            {
                if (obj->getObjectType()!=sim_object_shape_type)
                    desiredLocalFrame=obj->getLocalTransformation();
                C7Vector localFramePreCorrectionForChildren(obj->getLocalTransformation().getInverse()*desiredLocalFrame);
                obj->setLocalTransformation(localFramePreCorrection*obj->getLocalTransformation());

                // Handle attached child scripts:
                CLuaScriptObject* childScript=nullptr;
                if (ar.xmlPushChildNode("childScript",false))
                {
                    childScript=new CLuaScriptObject(sim_scripttype_childscript);
                    childScript->serialize(ar);
                    ar.xmlPopNode();
                }

                // Handle attached customization scripts:
                CLuaScriptObject* customizationScript=nullptr;
                if (ar.xmlPushChildNode("customizationScript",false))
                {
                    customizationScript=new CLuaScriptObject(sim_scripttype_customizationscript);
                    customizationScript->serialize(ar);
                    ar.xmlPopNode();
                }

                SSimpleXmlSceneObject xmlobj;
                xmlobj.object=obj;
                xmlobj.parentObject=parentObject;
                xmlobj.childScript=childScript;
                xmlobj.customizationScript=customizationScript;
                simpleXmlObjects.push_back(xmlobj);

                // Possibly recurse:
                readAndAddToSceneSimpleXmlSceneObjects(ar,obj,localFramePreCorrectionForChildren,simpleXmlObjects);
            }
        }
        if (nodeIt==allChildNodeNames.size()-1)
            ar.xmlPopNode();
    }
    return(retVal);
}

void CSceneObjectContainer::writeSimpleXmlSceneObjectTree(CSer& ar,const CSceneObject* object)
{
    if (object->getObjectType()==sim_object_shape_type)
    {
        CShape* obj=(CShape*)object;
        ar.xmlPushNewNode("shape");
        _writeSimpleXmlShape(ar,obj);
    }
    if (object->getObjectType()==sim_object_joint_type)
    {
        CJoint* obj=(CJoint*)object;
        ar.xmlPushNewNode("joint");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_graph_type)
    {
        CGraph* obj=(CGraph*)object;
        ar.xmlPushNewNode("graph");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_camera_type)
    {
        CCamera* obj=(CCamera*)object;
        ar.xmlPushNewNode("camera");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_dummy_type)
    {
        CDummy* obj=(CDummy*)object;
        ar.xmlPushNewNode("dummy");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_proximitysensor_type)
    {
        CProxSensor* obj=(CProxSensor*)object;
        ar.xmlPushNewNode("proximitySensor");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_path_type)
    {
        CPath_old* obj=(CPath_old*)object;
        ar.xmlPushNewNode("path");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_visionsensor_type)
    {
        CVisionSensor* obj=(CVisionSensor*)object;
        ar.xmlPushNewNode("visionSensor");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_forcesensor_type)
    {
        CForceSensor* obj=(CForceSensor*)object;
        ar.xmlPushNewNode("forceSensor");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_light_type)
    {
        CLight* obj=(CLight*)object;
        ar.xmlPushNewNode("light");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_octree_type)
    {
        COctree* obj=(COctree*)object;
        ar.xmlPushNewNode("ocTree");
        obj->serialize(ar);
    }
    if (object->getObjectType()==sim_object_pointcloud_type)
    {
        CPointCloud* obj=(CPointCloud*)object;
        ar.xmlPushNewNode("pointCloud");
        obj->serialize(ar);
    }

    CLuaScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_child(object->getObjectHandle());
    if (script!=nullptr)
    {
        ar.xmlPushNewNode("childScript");
        script->serialize(ar);
        ar.xmlPopNode();
    }
    script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo_customization(object->getObjectHandle());
    if (script!=nullptr)
    {
        ar.xmlPushNewNode("customizationScript");
        script->serialize(ar);
        ar.xmlPopNode();
    }

    for (size_t i=0;i<object->getChildCount();i++)
        writeSimpleXmlSceneObjectTree(ar,object->getChildFromIndex(i));

    ar.xmlPopNode();
}

void CSceneObjectContainer::setObjectParent(CSceneObject* object,CSceneObject* newParent,bool keepInPlace)
{
    CSceneObject* oldParent=object->getParent();
    if (oldParent!=newParent)
    {
        C7Vector absTr(object->getFullCumulativeTransformation());
        if (oldParent!=nullptr)
            oldParent->removeChild(object);
        else
            _removeFromOrphanObjects(object);
        if (newParent!=nullptr)
            newParent->addChild(object);
        else
            _addToOrphanObjects(object);
        object->setParent(newParent);
        _handleOrderIndexOfOrphans();

        if (keepInPlace)
        {
            C7Vector parentTr(object->getFullParentCumulativeTransformation());
            object->setLocalTransformation(parentTr.getInverse()*absTr);
        }

        App::currentWorld->sceneObjects->actualizeObjectInformation();
    }
}

bool CSceneObjectContainer::setObjectAlias(CSceneObject* object,const char* newAlias,bool allowNameAdjustment)
{
    bool retVal=false;
    if (allowNameAdjustment||tt::isAliasValid(newAlias))
    {
        std::string nm(tt::getValidAlias(newAlias));
        if (nm!=object->getObjectAlias())
        {
            retVal=true;
            object->setObjectAlias_direct(nm.c_str());
            //setObjectSequence(object,-1); // like inserting it in last position
            if (object->getParent()==nullptr)
                _handleOrderIndexOfOrphans();
            else
                object->getParent()->handleOrderIndexOfChildren();
        }
    }
    return(retVal);
}

bool CSceneObjectContainer::setObjectSequence(CSceneObject* object,int order)
{ // overridden from _CSceneObjectContainer_
    bool retVal=false;
    CSceneObject* parent=object->getParent();
    if (parent==nullptr)
    {
        retVal=_CSceneObjectContainer_::setObjectSequence(object,order);
        if (retVal)
            _handleOrderIndexOfOrphans();
    }
    else
        retVal=parent->setChildSequence(object,order);
    if (retVal)
    {
        App::setFullDialogRefreshFlag();
        App::setRebuildHierarchyFlag();
    }
    return(retVal);
}

bool CSceneObjectContainer::setObjectName_old(CSceneObject* object,const char* newName,bool allowNameAdjustment)
{ // overridden from _CSceneObjectContainer_
    std::string nm(newName);
    bool retVal=false;
    if (allowNameAdjustment||tt::isObjectNameValid_old(newName,!tt::isHashFree(newName)))
    {
        tt::removeIllegalCharacters(nm,true);
        bool renamed=false;
        while (getObjectFromName(nm.c_str())!=nullptr)
        {
            renamed=true;
            nm=tt::generateNewName_hashOrNoHash(nm.c_str(),!tt::isHashFree(nm.c_str()));
        }
        if (allowNameAdjustment||(!renamed))
        {
            retVal=_CSceneObjectContainer_::setObjectName_old(object,nm.c_str(),allowNameAdjustment);
            if (retVal)
                object->setObjectName_direct_old(nm.c_str());
        }
    }
    return(retVal);
}

bool CSceneObjectContainer::setObjectAltName_old(CSceneObject* object,const char* newName,bool allowNameAdjustment)
{ // overridden from _CSceneObjectContainer_
    std::string nm(newName);
    bool retVal=false;
    if (allowNameAdjustment||tt::isObjectNameValid_old(newName,false))
    {
        tt::removeAltNameIllegalCharacters(nm);
        bool renamed=false;
        while (getObjectFromAltName(nm.c_str())!=nullptr)
        {
            renamed=true;
            nm=tt::generateNewName_noHash(nm.c_str());
        }
        if (allowNameAdjustment||(!renamed))
        {
            retVal=_CSceneObjectContainer_::setObjectAltName_old(object,nm.c_str(),allowNameAdjustment);
            if (retVal)
                object->setObjectAltName_direct_old(nm.c_str());
        }
    }
    return(retVal);
}

bool CSceneObjectContainer::setSelectedObjectHandles(const std::vector<int>* v)
{ // overridden from _CSceneObjectContainer_
    std::vector<int> prevSel(getSelectedObjectHandlesPtr()->begin(),getSelectedObjectHandlesPtr()->end());
    bool retVal=_CSceneObjectContainer_::setSelectedObjectHandles(v);
    if (retVal)
    {
        for (size_t i=0;i<prevSel.size();i++)
        {
            CSceneObject* it=getObjectFromHandle(prevSel[i]);
            it->setSelected(false);
        }
        for (size_t i=0;i<getSelectedObjectHandlesPtr()->size();i++)
        {
            CSceneObject* it=getObjectFromHandle(getSelectedObjectHandlesPtr()->at(i));
            it->setSelected(true);
        }
    }
    return(retVal);
}

void CSceneObjectContainer::_handleOrderIndexOfOrphans()
{
    std::map<std::string,int> nameMap;
    for (size_t i=0;i<getOrphanCount();i++)
    {
        CSceneObject* child=getOrphanFromIndex(i);
        std::string hn(child->getObjectAlias());
        std::map<std::string,int>::iterator it=nameMap.find(hn);
        if (it==nameMap.end())
            nameMap[hn]=0;
        else
            nameMap[hn]++;
        child->setChildOrder(nameMap[hn]);
    }
    for (size_t i=0;i<getOrphanCount();i++)
    {
        CSceneObject* child=getOrphanFromIndex(i);
        std::string hn(child->getObjectAlias());
        std::map<std::string,int>::iterator it=nameMap.find(hn);
        if (nameMap[hn]==0)
            child->setChildOrder(-1); // means unique with that name, with that parent
    }
    App::setFullDialogRefreshFlag();
    App::setRebuildHierarchyFlag();
}

void CSceneObjectContainer::setObjectAbsolutePose(int objectHandle,const C7Vector& v,bool keepChildrenInPlace)
{
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if (it!=nullptr)
    {
        C7Vector childPreTr(it->getFullLocalTransformation());
        C7Vector parentInverse(it->getFullParentCumulativeTransformation().getInverse());
        it->setLocalTransformation(parentInverse*v);
        if (keepChildrenInPlace)
        {
            childPreTr=it->getFullLocalTransformation().getInverse()*childPreTr;
            for (size_t i=0;i<it->getChildCount();i++)
            {
                CSceneObject* child=it->getChildFromIndex(i);
                child->setLocalTransformation(childPreTr*child->getLocalTransformation());
            }
        }
    }
}

void CSceneObjectContainer::setObjectAbsolutePosition(int objectHandle,const C3Vector& p)
{
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if (it!=nullptr)
    {
        C7Vector cumul(it->getCumulativeTransformation());
        C7Vector parentInverse(it->getFullParentCumulativeTransformation().getInverse());
        cumul.X=p;
        it->setLocalTransformation(parentInverse*cumul);
    }
}

void CSceneObjectContainer::setObjectAbsoluteOrientation(int objectHandle,const C3Vector& euler)
{
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if (it!=nullptr)
    {
        C7Vector cumul(it->getCumulativeTransformation());
        C7Vector parentInverse(it->getFullParentCumulativeTransformation().getInverse());
        cumul.Q.setEulerAngles(euler);
        it->setLocalTransformation(parentInverse*cumul);
    }
}

CSceneObject* CSceneObjectContainer::getObjectFromUniqueId(int uniqueID) const
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        if (it!=nullptr)
        {
            if (it->getUniqueID()==uniqueID)
                return(it);
        }
    }
    return(nullptr);
}

int CSceneObjectContainer::getHighestObjectHandle() const
{
    int highest=-1;
    for (size_t i=0;i<getObjectCount();i++)
    {
        int h=getObjectFromIndex(i)->getObjectHandle();
        if (h>highest)
            highest=h;
    }
    return(highest);
}


bool CSceneObjectContainer::isSelectionSame(std::vector<int>& sel,bool actualize) const
{
    bool retVal=true;
    const std::vector<int>* _sel=getSelectedObjectHandlesPtr();
    if (_sel->size()==sel.size())
    {
        for (size_t i=0;i<_sel->size();i++)
        {
            if (_sel->at(i)!=sel[i])
            {
                retVal=false;
                break;
            }
        }
    }
    else
        retVal=false;
    if ( (!retVal)&&actualize)
        sel.assign(_sel->begin(),_sel->end());
    return(retVal);
}

void CSceneObjectContainer::selectObject(int objectHandle)
{
    std::vector<int> sel;
    sel.push_back(objectHandle);
    if (setSelectedObjectHandles(&sel))
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->editModeContainer->announceObjectSelectionChanged();
#endif
        App::setLightDialogRefreshFlag();
    }
}

void CSceneObjectContainer::selectAllObjects()
{
    std::vector<int> sel;
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        sel.push_back(it->getObjectHandle());
    }
    if (setSelectedObjectHandles(&sel))
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->editModeContainer->announceObjectSelectionChanged();
#endif
        App::setLightDialogRefreshFlag();
    }
}

void CSceneObjectContainer::deselectObjects()
{
    if (setSelectedObjectHandles(nullptr))
    {
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->editModeContainer->announceObjectSelectionChanged();
#endif
        App::setLightDialogRefreshFlag();
    }
}

void CSceneObjectContainer::addObjectToSelection(int objectHandle)
{
    if (objectHandle>=0)
    {
        if (objectHandle>=NON_OBJECT_PICKING_ID_PATH_PTS_START) // individual path points!
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->editModeContainer->pathPointManipulation->addPathPointToSelection_nonEditMode(objectHandle);
#endif
        }
        else
        {
            CSceneObject* it=getObjectFromHandle(objectHandle);
            if (it!=nullptr)
            {
                if (!it->getSelected())
                {
                    std::vector<int> _sel(getSelectedObjectHandlesPtr()[0]);
                    _sel.push_back(objectHandle);
                    App::currentWorld->buttonBlockContainer->aSceneObjectWasSelected(objectHandle);
                    if (setSelectedObjectHandles(&_sel))
                    {
#ifdef SIM_WITH_GUI
                        if (App::mainWindow!=nullptr)
                            App::mainWindow->editModeContainer->announceObjectSelectionChanged();
#endif
                    }
                }
            }
        }
    }
    App::setLightDialogRefreshFlag();
}

void CSceneObjectContainer::removeObjectFromSelection(int objectHandle)
{
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if (it!=nullptr)
    {
        if (it->getSelected())
        {
            std::vector<int> sel(getSelectedObjectHandlesPtr()[0]);
            for (size_t i=0;i<sel.size();i++)
            {
                if (sel[i]==objectHandle)
                {
                    sel.erase(sel.begin()+i);
                    if (setSelectedObjectHandles(&sel))
                    {
#ifdef SIM_WITH_GUI
                        if (App::mainWindow!=nullptr)
                            App::mainWindow->editModeContainer->announceObjectSelectionChanged();
                        App::setLightDialogRefreshFlag();
#endif
                    }
                    break;
                }
            }
        }
    }
}

void CSceneObjectContainer::xorAddObjectToSelection(int objectHandle)
{
    if (objectHandle!=-1)
    {
        if (objectHandle>=NON_OBJECT_PICKING_ID_PATH_PTS_START) // individual path points!
        {
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->editModeContainer->pathPointManipulation->xorAddPathPointToSelection_nonEditMode(objectHandle);
#endif
        }
        else
        {
            CSceneObject* theObject=getObjectFromHandle(objectHandle);
            if (theObject!=nullptr)
            {
                if (!theObject->getSelected())
                    addObjectToSelection(objectHandle);
                else
                    removeObjectFromSelection(objectHandle);
            }
        }
    }
    else
        deselectObjects();
}

void CSceneObjectContainer::removeFromSelectionAllExceptModelBase(bool keepObjectsSelectedThatAreNotBuiltOnAModelBase)
{ // One model base per hierarchy tree!
    TRACE_INTERNAL;
    std::vector<int> sel(getSelectedObjectHandlesPtr()[0]);
    deselectObjects();
    std::vector<CSceneObject*> modelBases;
    std::vector<CSceneObject*> nonModelBasesBuildOnNothing;
    for (size_t i=0;i<sel.size();i++)
    {
        CSceneObject* it=getObjectFromHandle(sel[i]);
        if (it->getModelBase())
            modelBases.push_back(it);
        else
        {
            CSceneObject* objIt=it;
            bool addIt=true;
            while (objIt->getParent()!=nullptr)
            {
                objIt=objIt->getParent();
                if (objIt->getModelBase())
                {
                    addIt=false;
                    break;
                }
            }
            if (addIt)
                nonModelBasesBuildOnNothing.push_back(it);
        }
    }
    for (size_t i=0;i<modelBases.size();i++)
    {
        CSceneObject* it=modelBases[i];
        bool isIndependentBase=true;
        while (it->getParent()!=nullptr)
        {
            it=it->getParent();
            for (size_t j=0;j<modelBases.size();j++)
            {
                if (modelBases[j]==it)
                {
                    isIndependentBase=false;
                    break;
                }
            }
        }
        if (isIndependentBase)
            addObjectToSelection(modelBases[i]->getObjectHandle());
    }
    if (keepObjectsSelectedThatAreNotBuiltOnAModelBase)
    {
        for (size_t i=0;i<nonModelBasesBuildOnNothing.size();i++)
            addObjectToSelection(nonModelBasesBuildOnNothing[i]->getObjectHandle());
    }
}

CShape* CSceneObjectContainer::_readSimpleXmlShape(CSer& ar,C7Vector& desiredLocalFrame)
{
    CDummy* dummy=new CDummy();
    dummy->serialize(ar); // we later transfer the common data to the shape object
    desiredLocalFrame=dummy->getFullLocalTransformation();
    CShape* shape=_createSimpleXmlShape(ar,false,nullptr,false);
    if (shape!=nullptr)
    {
        if (ar.xmlPushChildNode("dynamics",false))
        {
            int m;
            if (ar.xmlGetNode_int("respondableMask",m,false))
                shape->setDynamicCollisionMask((unsigned short)m);
            C3Vector vel;
            if (ar.xmlGetNode_floats("initialLinearVelocity",vel.data,3,false))
                shape->setInitialDynamicLinearVelocity(vel);
            if (ar.xmlGetNode_floats("initialAngularVelocity",vel.data,3,false))
                shape->setInitialDynamicAngularVelocity(vel*piValue_f/180.0f);
            float mass=1.0f;
            if (ar.xmlGetNode_float("mass",mass,false))
            {
                if (mass<0.0000001f)
                    mass=0.0000001f;
                shape->getMeshWrapper()->setMass(mass);
            }

            C3Vector pmoment(0.1f,0.1f,0.1f);
            ar.xmlGetNode_floats("principalMomentOfInertia",pmoment.data,3,false);
            C7Vector inertiaFrame;
            inertiaFrame.setIdentity();
            if (ar.xmlPushChildNode("localInertiaFrame",false))
            {
                ar.xmlGetNode_floats("position",inertiaFrame.X.data,3,false);
                C3Vector euler;
                if (ar.xmlGetNode_floats("euler",euler.data,3,false))
                {
                    euler(0)*=piValue_f/180.0f;
                    euler(1)*=piValue_f/180.0f;
                    euler(2)*=piValue_f/180.0f;
                    inertiaFrame.Q.setEulerAngles(euler);
                }
                ar.xmlPopNode();
            }
            float inertia[6]={0.1f,0.0f,0.0f,0.1f,0.0f,0.1f};
            bool hasInertia=false;
            hasInertia=ar.xmlGetNode_floats("inertia",inertia,6,false);
            if (ar.xmlPushChildNode("switches",false))
            {
                bool b;
                if (ar.xmlGetNode_bool("static",b,false))
                    shape->setShapeIsDynamicallyStatic(b);
                if (ar.xmlGetNode_bool("respondable",b,false))
                    shape->setRespondable(b);
                if (ar.xmlGetNode_bool("startSleeping",b,false))
                    shape->setStartInDynamicSleeping(b);
                if (ar.xmlGetNode_bool("setToDynamicIfGetsParent",b,false))
                    shape->setSetAutomaticallyToNonStaticIfGetsParent(b);
                ar.xmlPopNode();
            }
            if (ar.xmlPushChildNode("material",false))
            {
                shape->getDynMaterial()->serialize(ar);
                ar.xmlPopNode();
            }
            ar.xmlPopNode();

            C3Vector com(inertiaFrame.X);
            C4X4Matrix tr;
            tr.setIdentity();
            C4Vector rot;
            rot.setIdentity();
            if (hasInertia)
            {
                C3X3Matrix m;
                m.axis[0](0)=inertia[0];
                m.axis[1](0)=inertia[1];
                m.axis[2](0)=inertia[2];
                m.axis[0](1)=inertia[1];
                m.axis[1](1)=inertia[3];
                m.axis[2](1)=inertia[4];
                m.axis[0](2)=inertia[2];
                m.axis[1](2)=inertia[4];
                m.axis[2](2)=inertia[5];
                m/=mass; // in CoppeliaSim we work with the "massless inertia"
                CMeshWrapper::findPrincipalMomentOfInertia(m,rot,pmoment);
            }
            if (pmoment(0)<0.0000001f)
                pmoment(0)=0.0000001f;
            if (pmoment(1)<0.0000001f)
                pmoment(1)=0.0000001f;
            if (pmoment(2)<0.0000001f)
                pmoment(0)=0.0000001f;
            shape->getMeshWrapper()->setPrincipalMomentsOfInertia(pmoment);
            shape->getMeshWrapper()->setLocalInertiaFrame(shape->getFullCumulativeTransformation().getInverse()*tr.getTransformation()*C7Vector(rot,com));
        }
        C7Vector tr(shape->getFullCumulativeTransformation());
        shape->acquireCommonPropertiesFromObject_simpleXMLLoading(dummy);
        setObjectAlias(shape,dummy->getObjectAlias().c_str(),true);
        setObjectName_old(shape,dummy->getObjectName_old().c_str(),true);
        setObjectAltName_old(shape,dummy->getObjectAltName_old().c_str(),true);
        shape->setLocalTransformation(dummy->getFullCumulativeTransformation()*tr);

        // We cannot decided of the position of the shape (the position is selected at the center of the shape)
        // But we can decide of the orientation of the shape (most of the time), so do it here (we simply reorient the shape's bounding box):
        if ( (!shape->getMeshWrapper()->isPure())||(shape->isCompound()) )
        {
            C7Vector oldAbsTr(shape->getCumulativeTransformation());
            C7Vector oldAbsTr2(dummy->getCumulativeTransformation().getInverse()*oldAbsTr);
            C7Vector x(oldAbsTr2*oldAbsTr.getInverse());
            shape->setLocalTransformation(oldAbsTr2);
            shape->alignBoundingBoxWithWorld();
            C7Vector newAbsTr2(shape->getCumulativeTransformation());
            C7Vector newAbsTr(x.getInverse()*newAbsTr2);
            shape->setLocalTransformation(newAbsTr);
        }
    }
    delete dummy;
    return(shape);
}

CShape* CSceneObjectContainer::_createSimpleXmlShape(CSer& ar,bool noHeightfield,const char* itemType,bool checkSibling)
{
    CShape* retVal=nullptr;
    bool loadVisualAttributes=false;
    int t=-1;
    if ( (!checkSibling)&&((itemType==nullptr)||(std::string("primitive").compare(itemType)==0))&&ar.xmlPushChildNode("primitive",false))
        t=0;
    else
    {
        if ( (itemType!=nullptr)&&std::string("primitive").compare(itemType)==0 )
        {
            if (checkSibling)
            {
                if (ar.xmlPushSiblingNode("primitive",false))
                    t=0;
                else
                    ar.xmlPopNode();
            }
        }
    }
    if (t==0)
    {
        loadVisualAttributes=true;
        int primitiveType=0;
        ar.xmlGetNode_enum("type",primitiveType,false,"cuboid",0,"sphere",1,"cylinder",2,"cone",3,"plane",4,"disc",5);
        float sizes[3]={0.1f,0.1f,0.1f};
        ar.xmlGetNode_floats("size",sizes,3,false);
        C7Vector tr;
        tr.setIdentity();
        if (ar.xmlPushChildNode("localFrame",false))
        {
            ar.xmlGetNode_floats("position",tr.X.data,3,false);
            C3Vector euler;
            if (ar.xmlGetNode_floats("euler",euler.data,3,false))
            {
                euler(0)*=piValue_f/180.0f;
                euler(1)*=piValue_f/180.0f;
                euler(2)*=piValue_f/180.0f;
                tr.Q.setEulerAngles(euler);
            }
            ar.xmlPopNode();
        }
        int pType=-1;
        bool cone=false;
        C3Vector s(tt::getLimitedFloat(0.00001f,100000.0f,sizes[0]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[1]),tt::getLimitedFloat(0.00001f,100000.0f,sizes[2]));
        int openEnds=0;
        int faces=0;
        int sides=32;
        if (primitiveType==0) // cuboid
            pType=1;
        if (primitiveType==1) // sphere
        {
            pType=2;
            faces=16;
            s(1)=s(0);
            s(2)=s(0);
        }
        if (primitiveType==2) // cylinder
        {
            pType=3;
            s(1)=s(0);
        }
        if (primitiveType==3) // cone
        {
            pType=3;
            s(1)=s(0);
            cone=true;
        }
        if (primitiveType==4) // plane
            pType=0;
        if (primitiveType==5) // disc
            pType=4;
        retVal=CAddOperations::addPrimitiveShape(pType,s,nullptr,faces,sides,0,true,openEnds,true,true,cone,1000.0f);
        retVal->setLocalTransformation(tr);
    }
    if (!noHeightfield)
    {
        if ( (!checkSibling)&&((itemType==nullptr)||(std::string("heightfield").compare(itemType)==0))&&ar.xmlPushChildNode("heightfield",false))
            t=1;
        else
        {
            if ( (itemType!=nullptr)&&std::string("heightfield").compare(itemType)==0 )
            {
                if (checkSibling)
                {
                    if (ar.xmlPushSiblingNode("heightfield",false))
                        t=1;
                    else
                        ar.xmlPopNode();
                }
            }
        }
    }
    if (t==1)
    {
        loadVisualAttributes=true;
        int size[2]={2,2};
        float meshSize=0.1f;
        std::vector<float> data;
        ar.xmlGetNode_ints("size",size,2,false);
        if (ar.xmlGetNode_float("gridStep",meshSize,false))
            tt::limitValue(0.00001f,10.0f,meshSize);
        ar.xmlGetNode_floats("data",data,false);
        if (data.size()!=size[0]*size[1])
        {
            size[0]=2;
            size[1]=2;
            data.clear();
            data.push_back(0.0f);
            data.push_back(0.0f);
            data.push_back(0.0f);
            data.push_back(0.0f);
        }
        std::vector<std::vector<float>*> allData;
        for (int i=0;i<size[1];i++)
        {
            std::vector<float>* vect=new std::vector<float>;
            for (int j=0;j<size[0];j++)
                vect->push_back(data[i*size[0]+j]);
            allData.push_back(vect);
        }
        int newShapeHandle=CFileOperations::apiAddHeightfieldToScene(size[0],meshSize/float(size[0]-1),allData,0.0f,0);
        for (size_t i=0;i<allData.size();i++)
            delete allData[i];
        retVal=getShapeFromHandle(newShapeHandle);
    }
    if ( (!checkSibling)&&((itemType==nullptr)||(std::string("mesh").compare(itemType)==0))&&ar.xmlPushChildNode("mesh",false))
        t=2;
    else
    {
        if ( (itemType!=nullptr)&&std::string("mesh").compare(itemType)==0 )
        {
            if (checkSibling)
            {
                if (ar.xmlPushSiblingNode("mesh",false))
                    t=2;
                else
                    ar.xmlPopNode();
            }
        }
    }
    if (t==2)
    {
        loadVisualAttributes=true;
        std::string str;
        if (ar.xmlGetNode_string("fileName",str,false))
        { // try to load from file first
            std::string filename(ar.getFilenamePath()+str);
            if (CPluginContainer::isAssimpPluginAvailable())
            {
                if (VFile::doesFileExist(filename.c_str()))
                {
                    int cnt=0;
                    int* shapes=CPluginContainer::assimp_importShapes(filename.c_str(),512,1.0f,1,32+128+256,&cnt);
                    if (shapes!=nullptr)
                    {
                        int newShapeHandle=shapes[0];
                        delete[] shapes;
                        retVal=getShapeFromHandle(newShapeHandle);
                    }
                }
            }
        }
        if (retVal==nullptr)
        { // try to load from vertices and indices list:
            std::vector<float> vertices;
            std::vector<int> indices;
            if (ar.xmlGetNode_floats("vertices",vertices,false))
            {
                if (ar.xmlGetNode_ints("indices",indices,false))
                {
                    bool ok=true;
                    while ( (vertices.size()%3)!=0 )
                        vertices.pop_back();
                    while ( (indices.size()%3)!=0 )
                        indices.pop_back();
                    for (size_t i=0;i<indices.size();i++)
                    {
                        if (indices[i]<0)
                            indices[i]=(vertices.size()/3)+indices[i];
                        if ( (indices[i]<0)||(indices[i]>=(vertices.size()/3)) )
                            ok=false;
                    }
                    if (ok)
                    {
                        retVal=new CShape(nullptr,vertices,indices,nullptr,nullptr);
                        addObjectToScene(retVal,false,true);
                    }
                }
            }
        }
        if (retVal!=nullptr)
        {
            C7Vector tr;
            tr.setIdentity();
            if (ar.xmlPushChildNode("localFrame",false))
            {
                ar.xmlGetNode_floats("position",tr.X.data,3,false);
                C3Vector euler;
                if (ar.xmlGetNode_floats("euler",euler.data,3,false))
                {
                    euler(0)*=piValue_f/180.0f;
                    euler(1)*=piValue_f/180.0f;
                    euler(2)*=piValue_f/180.0f;
                    tr.Q.setEulerAngles(euler);
                }
                ar.xmlPopNode();
            }
            retVal->setLocalTransformation(tr*retVal->getFullLocalTransformation());
        }
    }
    if ( (!checkSibling)&&((itemType==nullptr)||(std::string("compound").compare(itemType)==0))&&ar.xmlPushChildNode("compound",false))
        t=3;
    else
    {
        if ( (itemType!=nullptr)&&std::string("compound").compare(itemType)==0 )
        {
            if (checkSibling)
            {
                if (ar.xmlPushSiblingNode("compound",false))
                    t=3;
                else
                    ar.xmlPopNode();
            }
        }
    }

    if (t==3)
    { // compound
        std::vector<int> allShapes;
        CShape* it=_createSimpleXmlShape(ar,true,"primitive",false);
        if (it!=nullptr)
        {
            allShapes.push_back(it->getObjectHandle());
            while (true)
            {
                it=_createSimpleXmlShape(ar,true,"primitive",true);
                if (it!=nullptr)
                    allShapes.push_back(it->getObjectHandle());
                else
                    break;
            }
        }
        it=_createSimpleXmlShape(ar,true,"mesh",false);
        if (it!=nullptr)
        {
            allShapes.push_back(it->getObjectHandle());
            while (true)
            {
                it=_createSimpleXmlShape(ar,true,"mesh",true);
                if (it!=nullptr)
                    allShapes.push_back(it->getObjectHandle());
                else
                    break;
            }
        }
        it=_createSimpleXmlShape(ar,true,"compound",false);
        if (it!=nullptr)
        {
            allShapes.push_back(it->getObjectHandle());
            while (true)
            {
                it=_createSimpleXmlShape(ar,true,"compound",true);
                if (it!=nullptr)
                    allShapes.push_back(it->getObjectHandle());
                else
                    break;
            }
        }
        if (allShapes.size()>=1)
        {
            int newShapeHandle=-1;
            if (allShapes.size()>=2)
                newShapeHandle=CSceneObjectOperations::groupSelection(&allShapes,false);
            else
                newShapeHandle=allShapes[0];
            retVal=getShapeFromHandle(newShapeHandle);
        }
        if (itemType==nullptr)
            ar.xmlPopNode();
    }
    if ( (retVal!=nullptr)&&(t!=-1)&&loadVisualAttributes )
    {
        float v;
        if (ar.xmlGetNode_float("shadingAngle",v,false))
        { // checkHere
            retVal->getSingleMesh()->setGouraudShadingAngle(v*piValue_f/180.0f);
            retVal->getSingleMesh()->setEdgeThresholdAngle(v*piValue_f/180.0f);
        }
        retVal->setVisibleEdges(false);
        bool b;
        if (ar.xmlGetNode_bool("culling",b,false))
            retVal->setCulling(b);
        if (ar.xmlGetNode_bool("wireframe",b,false))
            retVal->getSingleMesh()->setWireframe(b);
        if (ar.xmlPushChildNode("color",false))
        {
            int rgb[3];
            if (ar.xmlGetNode_ints("ambientDiffuse",rgb,3,false))
                retVal->getSingleMesh()->color.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_ambient_diffuse);
            if (ar.xmlGetNode_ints("specular",rgb,3,false))
                retVal->getSingleMesh()->color.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_specular);
            if (ar.xmlGetNode_ints("emission",rgb,3,false))
                retVal->getSingleMesh()->color.setColor(float(rgb[0])/255.1f,float(rgb[1])/255.1f,float(rgb[2])/255.1f,sim_colorcomponent_emission);
            ar.xmlPopNode();
        }
        if (itemType==nullptr)
            ar.xmlPopNode();
    }
    return(retVal);
}

void CSceneObjectContainer::_writeSimpleXmlShape(CSer& ar,CShape* shape)
{
    shape->serialize(ar); // will only serialize the common part. The rest has to be done here:

    std::vector<CShape*> allComponents;

    CShape* copy=(CShape*)shape->copyYourself();
    copy->setLocalTransformation(shape->getFullCumulativeTransformation());

    addObjectToScene(copy,false,false);
    if (copy->isCompound())
    {
        int h=copy->getObjectHandle();
        std::vector<int> finalSel;
        std::vector<int> previousSel;
        std::vector<int> sel;
        previousSel.push_back(h);
        sel.push_back(h);
        while (sel.size()!=0)
        {
            CSceneObjectOperations::ungroupSelection(&sel,false);
            for (size_t i=0;i<previousSel.size();i++)
            {
                int previousID=previousSel[i];
                bool present=false;
                for (size_t j=0;j<sel.size();j++)
                {
                    if (sel[j]==previousID)
                    {
                        present=true;
                        break;
                    }
                }
                if ((!present)&&(h!=previousID)) // the original shape will be added at the very end for correct ordering (see below)
                    finalSel.push_back(previousID); // this is a simple shape (not a group)
            }
            previousSel.assign(sel.begin(),sel.end());
        }
        finalSel.push_back(h);
        for (size_t i=0;i<finalSel.size();i++)
            allComponents.push_back(getShapeFromHandle(finalSel[i]));
    }
    else
        allComponents.push_back(copy);

    ar.xmlAddNode_comment(" one of following tags is required: 'compound', 'primitive', 'heightfield' or 'mesh'. 'compound' itself requires at least two of those tags as children ",false);
    if (allComponents.size()>1)
    {
        ar.xmlPushNewNode("compound");
        for (size_t i=0;i<allComponents.size();i++)
            _writeSimpleXmlSimpleShape(ar,shape->getObjectAliasAndHandle().c_str(),allComponents[i],shape->getFullCumulativeTransformation());
        ar.xmlPopNode();
    }
    else
        _writeSimpleXmlSimpleShape(ar,shape->getObjectAliasAndHandle().c_str(),allComponents[0],shape->getFullCumulativeTransformation());

    for (size_t i=0;i<allComponents.size();i++)
        eraseObject(allComponents[i],false);

    ar.xmlPushNewNode("dynamics");

    ar.xmlAddNode_int("respondableMask",shape->getDynamicCollisionMask());
    ar.xmlAddNode_floats("initialLinearVelocity",shape->getInitialDynamicLinearVelocity().data,3);
    C3Vector vel(shape->getInitialDynamicAngularVelocity()*180.0f/piValue_f);
    ar.xmlAddNode_floats("initialAngularVelocity",vel.data,3);
    ar.xmlAddNode_float("mass",shape->getMeshWrapper()->getMass());
    C7Vector tr(shape->getMeshWrapper()->getLocalInertiaFrame());

    ar.xmlPushNewNode("localInertiaFrame");
    ar.xmlAddNode_floats("position",tr.X.data,3);
    C3Vector euler(tr.Q.getEulerAngles());
    euler*=180.0f/piValue_f;
    ar.xmlAddNode_floats("euler",euler.data,3);
    ar.xmlPopNode();

    ar.xmlAddNode_floats("principalMomentOfInertia",shape->getMeshWrapper()->getPrincipalMomentsOfInertia().data,3);
    ar.xmlPushNewNode("switches");
    ar.xmlAddNode_bool("static",shape->getShapeIsDynamicallyStatic());
    ar.xmlAddNode_bool("respondable",shape->getRespondable());
    ar.xmlAddNode_bool("startSleeping",shape->getStartInDynamicSleeping());
    ar.xmlAddNode_bool("setToDynamicIfGetsParent",shape->getSetAutomaticallyToNonStaticIfGetsParent());
    ar.xmlPopNode();

    ar.xmlPushNewNode("material");
    shape->getDynMaterial()->serialize(ar);
    ar.xmlPopNode();

    ar.xmlPopNode();
}

void CSceneObjectContainer::_writeSimpleXmlSimpleShape(CSer& ar,const char* originalShapeName,CShape* shape,const C7Vector& frame)
{
    CMesh* geom=shape->getSingleMesh();
    if (geom->getPurePrimitiveType()==sim_pure_primitive_none)
    { // mesh
        ar.xmlPushNewNode("mesh");
        C7Vector trOld(shape->getFullLocalTransformation());
        C7Vector x(frame.getInverse()*trOld);
        shape->setLocalTransformation(C7Vector::identityTransformation);
//        shape->setLocalTransformation(geom->getVerticeLocalFrame().getInverse()); // we temporarily want the shape's pose so that the mesh appears at the origin, for export
        ar.xmlAddNode_comment(" one of following tags is required: 'fileName' or 'vertices' and 'indices' ",false);
        if ( CPluginContainer::isAssimpPluginAvailable()&&(!ar.xmlSaveDataInline(geom->getVertices()->size()+geom->getIndices()->size()*4)) )
        {
            int shapeHandle=shape->getObjectHandle();
            std::string filename(ar.getFilenameBase()+"_mesh_"+originalShapeName+tt::FNb(ar.getIncrementCounter())+".dae");
            bool wireframe=shape->getShapeWireframe();
            if (wireframe)
                shape->setShapeWireframe(false); // The Assimp plugin will ignore wireframe shapes and not write them!!
            CPluginContainer::assimp_exportShapes(&shapeHandle,1,(ar.getFilenamePath()+filename).c_str(),"collada",1.0f,1,256);
            if (wireframe)
                shape->setShapeWireframe(true);
            ar.xmlAddNode_string("fileName",filename.c_str());
        }
        else
        {
            std::vector<float> v;
            v.resize(geom->getVertices()->size());
            for (size_t i=0;i<geom->getVertices()->size()/3;i++)
            {
                C3Vector w(&geom->getVertices()[0][3*i]);
                w*=geom->getVerticeLocalFrame();
                v[3*i+0]=w(0);
                v[3*i+1]=w(1);
                v[3*i+2]=w(2);
            }
            ar.xmlAddNode_floats("vertices",v);
            ar.xmlAddNode_ints("indices",geom->getIndices()[0]);
        }

        shape->setLocalTransformation(trOld); // restore it
        ar.xmlPushNewNode("localFrame");
        C7Vector tr(x);
        ar.xmlAddNode_floats("position",tr.X.data,3);
        C3Vector euler(tr.Q.getEulerAngles()*180.0f/piValue_f);
        ar.xmlAddNode_floats("euler",euler.data,3);
        ar.xmlPopNode();
    }
    else if (geom->getPurePrimitiveType()==sim_pure_primitive_heightfield)
    { // heightfield
        ar.xmlPushNewNode("heightfield");

        ar.xmlAddNode_comment(" 'size' tag: required ",false);
        ar.xmlAddNode_2int("size",geom->_heightfieldXCount,geom->_heightfieldYCount);
        C3Vector s;
        geom->getPurePrimitiveSizes(s);
        float gridStep=s(0)/float(geom->_heightfieldXCount-1);
        ar.xmlAddNode_float("gridStep",gridStep);
        ar.xmlAddNode_comment(" 'data' tag: required. has to contain size[0]*size[1] values ",false);
        ar.xmlAddNode_floats("data",geom->_heightfieldHeights);
    }
    else
    { // primitive
        ar.xmlPushNewNode("primitive");

        ar.xmlAddNode_comment(" 'type' tag: required. Can be one of following: 'cuboid', 'sphere', 'cylinder', 'cone', 'plane' or 'disc' ",false);
        if (geom->getPurePrimitiveType()==sim_pure_primitive_cuboid)
            ar.xmlAddNode_string("type","cuboid");
        if (geom->getPurePrimitiveType()==sim_pure_primitive_spheroid)
            ar.xmlAddNode_string("type","sphere");
        if (geom->getPurePrimitiveType()==sim_pure_primitive_cylinder)
            ar.xmlAddNode_string("type","cylinder");
        if (geom->getPurePrimitiveType()==sim_pure_primitive_cone)
            ar.xmlAddNode_string("type","cone");
        if (geom->getPurePrimitiveType()==sim_pure_primitive_plane)
            ar.xmlAddNode_string("type","plane");
        if (geom->getPurePrimitiveType()==sim_pure_primitive_disc)
            ar.xmlAddNode_string("type","disc");

        C3Vector s;
        geom->getPurePrimitiveSizes(s);
        ar.xmlAddNode_floats("size",s.data,3);
        ar.xmlPushNewNode("localFrame");
        C7Vector tr(frame.getInverse()*shape->getFullCumulativeTransformation()*geom->getVerticeLocalFrame()); // 'geom->getVerticeLocalFrame()' indicates also the origin of primitives
        ar.xmlAddNode_floats("position",tr.X.data,3);
        C3Vector euler(tr.Q.getEulerAngles()*180.0f/piValue_f);
        ar.xmlAddNode_floats("euler",euler.data,3);
        ar.xmlPopNode();
    }

    // now the visual attributes:
    ar.xmlAddNode_float("shadingAngle",geom->getGouraudShadingAngle()*180.0f/piValue_f);
    ar.xmlAddNode_bool("culling",geom->getCulling());
    ar.xmlAddNode_bool("wireframe",geom->getWireframe());

    ar.xmlPushNewNode("color");
    int rgb[3];
    for (size_t l=0;l<3;l++)
        rgb[l]=int(geom->color.getColorsPtr()[l]*255.1f);
    ar.xmlAddNode_ints("ambientDiffuse",rgb,3);
    for (size_t l=0;l<3;l++)
        rgb[l]=int(geom->color.getColorsPtr()[6+l]*255.1f);
    ar.xmlAddNode_ints("specular",rgb,3);
    for (size_t l=0;l<3;l++)
        rgb[l]=int(geom->color.getColorsPtr()[9+l]*255.1f);
    ar.xmlAddNode_ints("emission",rgb,3);
    ar.xmlPopNode();

    ar.xmlPopNode(); // "primitive" or "mesh" or "heightField"
}

void CSceneObjectContainer::_addObject(CSceneObject* object)
{ // Overridden from _CSceneObjectContainer_
    object->setSelected(false);
    _CSceneObjectContainer_::_addObject(object);
    _handleOrderIndexOfOrphans();

    App::setFullDialogRefreshFlag();
    App::setRebuildHierarchyFlag();

    actualizeObjectInformation();

    if (object->setObjectCanSync(true))
        object->buildUpdateAndPopulateSynchronizationObject(nullptr);
}

void CSceneObjectContainer::_removeObject(CSceneObject* object)
{ // Overridden from _CSceneObjectContainer_
    setObjectParent(object,nullptr,true);
    object->removeSynchronizationObject(false);
    _CSceneObjectContainer_::_removeObject(object);
    _handleOrderIndexOfOrphans();

    CSceneObject::incrementModelPropertyValidityNumber();
    actualizeObjectInformation();
    App::setFullDialogRefreshFlag();
    App::setRebuildHierarchyFlag();
}

void CSceneObjectContainer::buildUpdateAndPopulateSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        it->buildUpdateAndPopulateSynchronizationObject(nullptr);
    }
}

void CSceneObjectContainer::connectSynchronizationObjects()
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        it->connectSynchronizationObject();
    }
}

void CSceneObjectContainer::removeSynchronizationObjects(bool localReferencesToItOnly)
{
    for (size_t i=0;i<getObjectCount();i++)
    {
        CSceneObject* it=getObjectFromIndex(i);
        it->removeSynchronizationObject(localReferencesToItOnly);
    }
}
