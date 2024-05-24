#include <sceneObjectContainer.h>
#include <app.h>
#include <addOperations.h>
#include <sceneObjectOperations.h>
#include <fileOperations.h>
#include <tt.h>
#include <utils.h>
#include <mesh.h>
#include <sstream>
#include <iostream>
#include <simFlavor.h>
#include <unordered_set>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CSceneObjectContainer::CSceneObjectContainer()
{
    embeddedScriptContainer = new CEmbeddedScriptContainer();
    _objectActualizationEnabled = true;
    _nextObjectHandle = SIM_IDSTART_SCENEOBJECT;
    _objectCreationCounter = 0;
    _objectDestructionCounter = 0;
    _hierarchyChangeCounter = 0;
}

CSceneObjectContainer::~CSceneObjectContainer()
{ // beware, the current world could be nullptr
    eraseAllObjects(true); // should already have been done
    delete embeddedScriptContainer;
}

void CSceneObjectContainer::simulationAboutToStart()
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->simulationAboutToStart();
    embeddedScriptContainer->simulationAboutToStart();
}

void CSceneObjectContainer::simulationAboutToEnd()
{
    embeddedScriptContainer->simulationAboutToEnd(); // destroys the main script (and subsequently all child scripts)

    for (size_t i = 0; i < getObjectCount(sim_object_script_type); i++)
    {
        CScript* it = getScriptFromIndex(i);
        it->scriptObject->simulationAboutToEnd(); // destroys child script states
    }
}

void CSceneObjectContainer::simulationEnded()
{
    embeddedScriptContainer->simulationEnded();
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->simulationEnded_restoreHierarchy();

    // Do following from base to tip:
    std::vector<CSceneObject *> toExplore;
    for (size_t i = 0; i < getOrphanCount(); i++)
        toExplore.push_back(getOrphanFromIndex(i));
    while (toExplore.size() != 0)
    {
        CSceneObject *obj = toExplore[0];
        toExplore.erase(toExplore.begin());
        obj->simulationEnded();
        for (size_t i = 0; i < obj->getChildCount(); i++)
            toExplore.push_back(obj->getChildFromIndex(i));
    }
}

void CSceneObjectContainer::announceObjectWillBeErased(const CSceneObject *object)
{
    TRACE_INTERNAL;
    embeddedScriptContainer->announceObjectWillBeErased(object);
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        it->announceObjectWillBeErased(object, false); // also send this to self
    }
}

void CSceneObjectContainer::announceScriptWillBeErased(int scriptHandle, bool simulationScript,
                                                       bool sceneSwitchPersistentScript)
{
    TRACE_INTERNAL;
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        it->announceScriptWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript, false);
    }
}

void CSceneObjectContainer::announceIkGroupWillBeErased(int ikGroupHandle)
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->announceIkObjectWillBeErased(ikGroupHandle,
                                                            false); // this never triggers scene object destruction!
}

void CSceneObjectContainer::announceCollectionWillBeErased(int collectionHandle)
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->announceCollectionWillBeErased(collectionHandle,
                                                              false); // this never triggers scene object destruction!
}

void CSceneObjectContainer::announceCollisionWillBeErased(int collisionHandle)
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->announceCollisionWillBeErased(collisionHandle,
                                                             false); // this never triggers scene object destruction!
}

void CSceneObjectContainer::announceDistanceWillBeErased(int distanceHandle)
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->announceDistanceWillBeErased(distanceHandle,
                                                            false); // this never triggers scene object destruction!
}

int CSceneObjectContainer::addObjectToScene(CSceneObject *newObject, bool objectIsACopy,
                                            bool generateAfterCreateCallback)
{
    return (addObjectToSceneWithSuffixOffset(newObject, objectIsACopy, 1, generateAfterCreateCallback));
}

int CSceneObjectContainer::addObjectToSceneWithSuffixOffset(CSceneObject *newObject, bool objectIsACopy,
                                                            int suffixOffset, bool generateAfterCreateCallback)
{
    App::currentWorld->environment->setSceneCanBeDiscardedWhenNewSceneOpened(false); // 4/3/2012

    std::string newObjName = newObject->getObjectName_old();
    if (objectIsACopy)
        newObjName = tt::generateNewName_hash(newObjName.c_str(), suffixOffset);
    else
    {
        if (getObjectFromName_old(newObjName.c_str()) != nullptr)
        {
            // Following faster with many objects:
            std::string baseName(tt::getNameWithoutSuffixNumber(newObjName.c_str(), false));
            int initialSuffix = tt::getNameSuffixNumber(newObjName.c_str(), false);
            std::vector<int> suffixes;
            std::vector<int> dummyValues;
            for (size_t i = 0; i < getObjectCount(); i++)
            {
                std::string baseNameIt(
                    tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(), false));
                if (baseName.compare(baseNameIt) == 0)
                {
                    suffixes.push_back(
                        tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(), false));
                    dummyValues.push_back(0);
                }
            }
            tt::orderAscending(suffixes, dummyValues);
            int lastS = -1;
            for (size_t i = 0; i < suffixes.size(); i++)
            {
                if ((suffixes[i] > initialSuffix) && (suffixes[i] > lastS + 1))
                    break;
                lastS = suffixes[i];
            }
            newObjName = tt::generateNewName_noHash(baseName.c_str(), lastS + 1 + 1);
        }
        // Following was too slow with many objects:
        //      while (getObject(newObjName)!=nullptr)
        //          newObjName=tt::generateNewName_noHash(newObjName);
    }
    newObject->setObjectName_direct_old(newObjName.c_str());

    // Same but with the alt object names:
    std::string newObjAltName = newObject->getObjectAltName_old();
    if (getObjectFromAltName_old(newObjAltName.c_str()) != nullptr)
    {
        // Following faster with many objects:
        std::string baseAltName(tt::getNameWithoutSuffixNumber(newObjAltName.c_str(), false));
        int initialSuffix = tt::getNameSuffixNumber(newObjAltName.c_str(), false);
        std::vector<int> suffixes;
        std::vector<int> dummyValues;
        for (size_t i = 0; i < getObjectCount(); i++)
        {
            std::string baseAltNameIt(
                tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectAltName_old().c_str(), false));
            if (baseAltName.compare(baseAltNameIt) == 0)
            {
                suffixes.push_back(
                    tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectAltName_old().c_str(), false));
                dummyValues.push_back(0);
            }
        }
        tt::orderAscending(suffixes, dummyValues);
        int lastS = -1;
        for (size_t i = 0; i < suffixes.size(); i++)
        {
            if ((suffixes[i] > initialSuffix) && (suffixes[i] > lastS + 1))
                break;
            lastS = suffixes[i];
        }
        newObjAltName = tt::generateNewName_noHash(baseAltName.c_str(), lastS + 1 + 1);
    }
    // Following was too slow with many objects:
    //      while (getObjectFromAltName(newObjAltName)!=nullptr)
    //          newObjAltName=tt::generateNewName_noHash(newObjAltName);
    newObject->setObjectAltName_direct_old(newObjAltName.c_str());

    int objectHandle = _nextObjectHandle;
#if SIM_PROGRAM_VERSION_NB < 40800
    // We make sure that new script objects do not have a handle below sim_scripttype_sandboxscript+1, so that we
    // can still allow the legacy call arguments for V4.7 with simCallScriptFunction, simCallScriptFunctionEx, simExecuteScriptString,
    // before dropping the legacy call argument support with V4.8 and later
    while ( (getObjectFromHandle(objectHandle) != nullptr) || ((newObject->getObjectType() == sim_object_script_type) && (objectHandle <= sim_scripttype_sandboxscript)) )
#else
    while (getObjectFromHandle(objectHandle) != nullptr)
#endif
    {
        objectHandle++;
        if (objectHandle >= (SIM_IDEND_SCENEOBJECT - SIM_IDSTART_SCENEOBJECT))
            objectHandle = SIM_IDSTART_SCENEOBJECT;
    }
    _nextObjectHandle = objectHandle + 1;
    if (_nextObjectHandle >= (SIM_IDEND_SCENEOBJECT - SIM_IDSTART_SCENEOBJECT))
        _nextObjectHandle = SIM_IDSTART_SCENEOBJECT;

    newObject->setObjectHandle(objectHandle);
    newObject->setObjectUniqueId();

    _addObject(newObject);

    if (newObject->getObjectType() == sim_object_graph_type)
    { // If the simulation is running, we have to empty the buffer!!! (otherwise we might have old and new data mixed
      // together (e.g. old data in future, new data in present!)
        if ((App::currentWorld->simulation != nullptr) && (!App::currentWorld->simulation->isSimulationStopped()))
        {
            CGraph *graph = (CGraph *)newObject;
            graph->resetGraph();
        }
    }

    if (generateAfterCreateCallback)
    {
        CInterfaceStack *stack = App::worldContainer->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();

        std::vector<int> hand;
        hand.push_back(newObject->getObjectHandle());
        stack->pushTextOntoStack("objects");
        stack->pushInt32ArrayOntoStack(hand.data(), hand.size());
        stack->insertDataIntoStackTable();

        // Following for backward compatibility:
        stack->pushTextOntoStack("objectHandles");
        stack->pushTableOntoStack();
        stack->pushInt32OntoStack(1); // key or index
        stack->pushInt32OntoStack(newObject->getObjectHandle());
        stack->insertDataIntoStackTable();
        stack->insertDataIntoStackTable();
        // --------------------------------------

        App::worldContainer->callScripts(sim_syscb_aftercreate, stack, nullptr);
        App::worldContainer->interfaceStackContainer->destroyStack(stack);
    }
    App::worldContainer->setModificationFlag(2); // object created
    newObject->recomputeModelInfluencedValues();

    newObject->setIsInScene(true);
    if (App::worldContainer->getEventsEnabled())
        newObject->pushObjectCreationEvent();
    return (objectHandle);
}

void CSceneObjectContainer::eraseObject(CSceneObject *it, bool generateBeforeAfterDeleteCallback)
{
    if (it != nullptr)
    {
        std::vector<int> l;
        l.push_back(it->getObjectHandle());
        eraseObjects(&l, generateBeforeAfterDeleteCallback);
    }
}

void CSceneObjectContainer::eraseObjects(const std::vector<int>* objectHandles, bool generateBeforeAfterDeleteCallback, bool delayed /*= false*/)
{ // if objectHandles is nullptr, then objects marked as delayed deletion will be removed by generating the callbacks
    if (objectHandles == nullptr)
    {
        if (_delayedDestructionObjects.size() > 0)
        {
            std::vector<int> toDelete(_delayedDestructionObjects);
            _delayedDestructionObjects.clear();
            eraseObjects(&toDelete, true);
        }
    }
    else
    {
        if (delayed)
            _delayedDestructionObjects.insert(_delayedDestructionObjects.end(), objectHandles->begin(), objectHandles->end());
        else
        {
            std::vector<int> toDestroy;
            for (size_t i = 0; i < objectHandles->size(); i++)
            {
                CSceneObject *it = getObjectFromHandle(objectHandles->at(i));
                if (it->canDestroyNow())
                    toDestroy.push_back(objectHandles->at(i));
            }

            if (toDestroy.size() > 0)
            {
                CInterfaceStack *stack = nullptr;
                if (generateBeforeAfterDeleteCallback)
                {
                    stack = App::worldContainer->interfaceStackContainer->createStack();
                    stack->pushTableOntoStack();

                    stack->pushTextOntoStack("objects");
                    stack->pushInt32ArrayOntoStack(toDestroy.data(), toDestroy.size());
                    stack->insertDataIntoStackTable();

                    stack->pushTextOntoStack("allObjects");
                    stack->pushBoolOntoStack(toDestroy.size() == getObjectCount());
                    stack->insertDataIntoStackTable();

                    // Following for backward compatibility:
                    stack->pushTextOntoStack("objectHandles");
                    stack->pushTableOntoStack();
                    for (size_t i = 0; i < toDestroy.size(); i++)
                    {
                        CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(toDestroy[i]);
                        if ((it != nullptr) && it->setBeforeDeleteCallbackSent())
                        { // send the message only once. This routine can be reentrant!
                            stack->pushInt32OntoStack(toDestroy[i]); // key or index
                            stack->pushBoolOntoStack(true);
                            stack->insertDataIntoStackTable();
                        }
                    }
                    stack->insertDataIntoStackTable();
                    // --------------------------------------

                    App::worldContainer->callScripts(sim_syscb_beforedelete, stack, nullptr);
                }

                for (size_t i = 0; i < toDestroy.size(); i++)
                {
                    CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(toDestroy[i]);
                    if (it != nullptr)
                    {
                        // We announce the object will be erased:
                        App::worldContainer->announceObjectWillBeErased(it); // this may trigger other "interesting" things, such as customization script runs, etc.
                        App::worldContainer->pushSceneObjectRemoveEvent(it);
                        _removeObject(it);
                    }
                }

                if (generateBeforeAfterDeleteCallback)
                {
                    App::worldContainer->callScripts(sim_syscb_afterdelete, stack, nullptr);
                    App::worldContainer->interfaceStackContainer->destroyStack(stack);
                }

                App::worldContainer->setModificationFlag(1); // object erased
            }
            if (toDestroy.size() != objectHandles->size())
                App::logMsg(sim_verbosity_errors, "object removal can't be triggered from within the object itself: some objects were not removed.");
        }
    }
}

void CSceneObjectContainer::eraseAllObjects(bool generateBeforeAfterDeleteCallback)
{
    removeSceneDependencies();

#ifdef SIM_WITH_GUI
    if (GuiApp::mainWindow != nullptr)
        GuiApp::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD, nullptr);
#endif
    std::vector<int> l;
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int objHandle = getObjectFromIndex(i)->getObjectHandle();
        CScript* script = getScriptFromHandle(objHandle);
        if (script != nullptr)
            script->scriptObject->setTemporarilySuspended(true);
        l.push_back(objHandle);
    }
    eraseObjects(&l, generateBeforeAfterDeleteCallback);
    // ideally we want to always use different object handles so that if the user erases an object and
    // creates a new one just after, the erased object's handle is not reused. That's why we have
    // the _nextObjectHandle variable.
    // The undo/redo functionality however doesn't like that, since at each undo/redo, all objects
    // get a new handle, and so the scene memory print looks different, even if the scenes are
    // identical: the undo/redo then marks a new restore point, which is not correct.
    // So, finally, when the whole scene gets emptied at least we make sure that all handles
    // start from the beginning:
    _nextObjectHandle = SIM_IDSTART_SCENEOBJECT;
}

int CSceneObjectContainer::addDefaultScript(int scriptType, bool threaded, bool lua)
{
    int retVal = -1;
    std::string filenameAndPath(App::folders->getSystemPath() + "/");

    if (scriptType == sim_scripttype_mainscript)
        retVal = embeddedScriptContainer->insertDefaultScript(scriptType, threaded, lua);
    else
    {
        if (scriptType == sim_scripttype_childscript)
        {
            if (threaded)
                filenameAndPath += DEFAULT_THREADEDCHILDSCRIPT;
            else
                filenameAndPath += DEFAULT_NONTHREADEDCHILDSCRIPT;
        }
        if (scriptType == sim_scripttype_customizationscript)
        {
            if (threaded)
                filenameAndPath += DEFAULT_THREADEDCUSTOMIZATIONSCRIPT;
            else
                filenameAndPath += DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT;
        }

        if (filenameAndPath.size() > 0)
        {
            if (lua)
                filenameAndPath += ".lua";
            else
                filenameAndPath += ".py";
            std::string scriptTxt;
            if (VFile::doesFileExist(filenameAndPath.c_str()))
            {
                try
                {
                    VFile file(filenameAndPath.c_str(), VFile::READ | VFile::SHARE_DENY_NONE);
                    VArchive archive(&file, VArchive::LOAD);
                    size_t archiveLength = file.getLength();
                    scriptTxt.resize(archiveLength);
                    for (size_t i = 0; i < archiveLength; i++)
                        archive >> scriptTxt[i];
                    archive.close();
                    file.close();
                }
                catch (VFILE_EXCEPTION_TYPE e)
                {
                    VFile::reportAndHandleFileExceptionError(e);
                    scriptTxt = "Default script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
                }
            }
            else
                scriptTxt = "Default script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!

            CScript* it = new CScript(scriptType, scriptTxt.c_str(), 0);
            retVal = addObjectToScene(it, false, true);
        }
    #ifdef SIM_WITH_GUI
        GuiApp::setLightDialogRefreshFlag();
    #endif
    }
    return (retVal);
}

void CSceneObjectContainer::actualizeObjectInformation()
{
    if (_objectActualizationEnabled)
    {
        // We actualize the direct linked joint list of each joint:
        for (size_t i = 0; i < getObjectCount(sim_object_joint_type); i++)
        {
            CJoint *it = getJointFromIndex(i);
            std::vector<CJoint *> joints;
            for (size_t j = 0; j < getObjectCount(sim_object_joint_type); j++)
            {
                CJoint *anAct = getJointFromIndex(j);
                if (anAct != it)
                {
                    if ((anAct->getJointMode() == sim_jointmode_dependent) ||
                        (anAct->getJointMode() == sim_jointmode_reserved_previously_ikdependent))
                    {
                        if (anAct->getDependencyMasterJointHandle() == it->getObjectHandle())
                            joints.push_back(anAct);
                    }
                }
            }
            it->setDirectDependentJoints(joints);
        }

        App::currentWorld->collections->actualizeAllCollections();

        for (size_t i = 0; i < getObjectCount(sim_object_shape_type); i++)
            getShapeFromIndex(i)->clearLastParentForLocalGlobalRespondable();

        App::currentWorld->textureContainer->updateAllDependencies();
#ifdef SIM_WITH_GUI
        GuiApp::setRebuildHierarchyFlag();
#endif
    }
}

void CSceneObjectContainer::enableObjectActualization(bool e)
{
    _objectActualizationEnabled = e;
}

void CSceneObjectContainer::getMinAndMaxNameSuffixes(int &minSuffix, int &maxSuffix) const
{
    minSuffix = -1;
    maxSuffix = -1;
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int s = tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(), true);
        if (i == 0)
        {
            minSuffix = s;
            maxSuffix = s;
        }
        else
        {
            if (s < minSuffix)
                minSuffix = s;
            if (s > maxSuffix)
                maxSuffix = s;
        }
    }
}

bool CSceneObjectContainer::canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int s1 = tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(), true);
        if (s1 == suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(), true));
            for (size_t j = 0; j < getObjectCount(); j++)
            {
                int s2 = tt::getNameSuffixNumber(getObjectFromIndex(j)->getObjectName_old().c_str(), true);
                if (s2 == suffix2)
                {
                    std::string name2(
                        tt::getNameWithoutSuffixNumber(getObjectFromIndex(j)->getObjectName_old().c_str(), true));
                    if (name1 == name2)
                        return (false); // NO! We would have a name clash!
                }
            }
        }
    }
    return (true);
}

void CSceneObjectContainer::setSuffix1ToSuffix2(int suffix1, int suffix2)
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int s1 = tt::getNameSuffixNumber(getObjectFromIndex(i)->getObjectName_old().c_str(), true);
        if (s1 == suffix1)
        {
            CSceneObject *obj = getObjectFromIndex(i);
            std::string name1(tt::getNameWithoutSuffixNumber(obj->getObjectName_old().c_str(), true));
            setObjectName_old(obj, tt::generateNewName_hash(name1.c_str(), suffix2 + 1).c_str(), true);
        }
    }
}

int CSceneObjectContainer::getObjectCreationCounter() const
{
    return (_objectCreationCounter);
}

int CSceneObjectContainer::getObjectDestructionCounter() const
{
    return (_objectDestructionCounter);
}

int CSceneObjectContainer::getHierarchyChangeCounter() const
{
    return (_hierarchyChangeCounter);
}

void CSceneObjectContainer::setTextureDependencies()
{ // here we cannot use shapeList, because that list may not yet be actualized (e.g. during a scene/model load
  // operation)!!
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        if (it->getObjectType() == sim_object_shape_type)
        {
            if (((CShape *)it)->getMesh() != nullptr)
                ((CShape *)it)->getMesh()->setTextureDependencies(it->getObjectHandle());
        }
    }
}

void CSceneObjectContainer::removeSceneDependencies()
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->removeSceneDependencies();
    embeddedScriptContainer->removeAllScripts();
}

void CSceneObjectContainer::checkObjectIsInstanciated(CSceneObject *obj, const char *location) const
{
    if (obj == nullptr)
    {
        App::logMsg(sim_verbosity_errors, (std::string("internal error: scene object is NULL in '") + location +
                                           "'. Please report this error")
                                              .c_str());
        App::beep();
    }
    else
    {
        if (getObjectFromHandle(obj->getObjectHandle()) == nullptr)
        {
            App::logMsg(sim_verbosity_errors, (std::string("internal error: scene object not instanciated in '") +
                                               location + "'. Please report this error")
                                                  .c_str());
            App::beep();
        }
    }
}

void CSceneObjectContainer::pushGenesisEvents() const
{
    std::vector<CSceneObject *> orderedObjects;
    for (size_t i = 0; i < getOrphanCount(); i++)
        orderedObjects.push_back(getOrphanFromIndex(i));

    for (size_t i = 0; i < orderedObjects.size(); i++)
    {
        CSceneObject *obj = orderedObjects[i];
        for (size_t j = 0; j < obj->getChildCount(); j++)
            orderedObjects.push_back(obj->getChildFromIndex(j));
    }

    for (size_t i = 0; i < orderedObjects.size(); i++)
        orderedObjects[i]->pushObjectCreationEvent();
}

void CSceneObjectContainer::getAllCollidableObjectsFromSceneExcept(const std::vector<CSceneObject *> *exceptionObjects,
                                                                   std::vector<CSceneObject *> &objects)
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        if (it->isPotentiallyCollidable())
        {
            if (it->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_collidable)
            { // Make sure we don't have it in the exception list:
                bool okToAdd = true;
                if (exceptionObjects != nullptr)
                {
                    for (size_t j = 0; j < exceptionObjects->size(); j++)
                    {
                        if (exceptionObjects->at(j) == it)
                        {
                            okToAdd = false;
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

void CSceneObjectContainer::getAllMeasurableObjectsFromSceneExcept(const std::vector<CSceneObject *> *exceptionObjects,
                                                                   std::vector<CSceneObject *> &objects)
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        if (it->isPotentiallyMeasurable())
        {
            if (it->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_measurable)
            { // Make sure we don't have it in the exception list:
                bool okToAdd = true;
                if (exceptionObjects != nullptr)
                {
                    for (size_t j = 0; j < exceptionObjects->size(); j++)
                    {
                        if (exceptionObjects->at(j) == it)
                        {
                            okToAdd = false;
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

void CSceneObjectContainer::getAllDetectableObjectsFromSceneExcept(const std::vector<CSceneObject *> *exceptionObjects,
                                                                   std::vector<CSceneObject *> &objects,
                                                                   int detectableMask)
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        if (it->isPotentiallyDetectable())
        {
            if ((it->getCumulativeObjectSpecialProperty() & detectableMask) || (detectableMask == -1))
            { // Make sure we don't have it in the exception list:
                bool okToAdd = true;
                if (exceptionObjects != nullptr)
                {
                    for (size_t j = 0; j < exceptionObjects->size(); j++)
                    {
                        if (exceptionObjects->at(j) == it)
                        {
                            okToAdd = false;
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

CSceneObject *CSceneObjectContainer::readSceneObject(CSer &ar, const char *name, bool &noHit)
{
    if (ar.isBinary())
    {
        int byteQuantity;
        std::string theName(name);
        if (theName.compare(SER_SHAPE) == 0)
        {
            ar >> byteQuantity;
            CShape *myNewObject = new CShape();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_JOINT) == 0)
        {
            ar >> byteQuantity;
            CJoint *myNewObject = new CJoint();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_GRAPH) == 0)
        {
            ar >> byteQuantity;
            CGraph *myNewObject = new CGraph();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_CAMERA) == 0)
        {
            ar >> byteQuantity;
            CCamera *myNewObject = new CCamera();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_LIGHT) == 0)
        {
            ar >> byteQuantity;
            CLight *myNewObject = new CLight();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_MIRROR) == 0)
        {
            if (CSimFlavor::getBoolVal(18))
                App::logMsg(sim_verbosity_errors, "Contains mirrors...");
            ar >> byteQuantity;
            CMirror *myNewObject = new CMirror();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_OCTREE) == 0)
        {
            ar >> byteQuantity;
            COcTree *myNewObject = new COcTree();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_POINTCLOUD) == 0)
        {
            ar >> byteQuantity;
            CPointCloud *myNewObject = new CPointCloud();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_DUMMY) == 0)
        {
            ar >> byteQuantity;
            CDummy *myNewObject = new CDummy();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_SCRIPT) == 0)
        {
            ar >> byteQuantity;
            CScript *myNewObject = new CScript();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_PROXIMITYSENSOR) == 0)
        {
            ar >> byteQuantity;
            CProxSensor *myNewObject = new CProxSensor();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_VISIONSENSOR) == 0)
        {
            ar >> byteQuantity;
            CVisionSensor *myNewObject = new CVisionSensor();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_PATH_OLD) == 0)
        {
            if (CSimFlavor::getBoolVal(18))
                App::logMsg(sim_verbosity_errors, "Contains path objects...");
            ar >> byteQuantity;
            CPath_old *myNewObject = new CPath_old();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_MILL) == 0)
        {
            if (CSimFlavor::getBoolVal(18))
                App::logMsg(sim_verbosity_errors, "Contains mills...");
            ar >> byteQuantity;
            CMill *myNewObject = new CMill();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }
        if (theName.compare(SER_FORCESENSOR) == 0)
        {
            ar >> byteQuantity;
            CForceSensor *myNewObject = new CForceSensor();
            myNewObject->serialize(ar);
            noHit = false;
            return (myNewObject);
        }

        // If we arrived here it means that maybe we have a new scene object type that this CoppeliaSim doesn't
        // understand yet. We try to replace it with a dummy (2009/12/09):
        unsigned char dat[14];
        if (ar.readBytesButKeepPointerUnchanged(dat, 14) != 14)
            return (nullptr); // No, this is not a scene object! (not enough to read)
        if ((dat[4] == '3') && (dat[5] == 'd') && (dat[6] == 'o') && (dat[11] == 57) && (dat[12] == 58) &&
            (dat[13] == 59))
        {                       // yes we have a scene object of an unknown type!
            ar >> byteQuantity; // Undo/redo will never arrive here
            CDummy *newUnknownType = new CDummy();
            newUnknownType->loadUnknownObjectType(ar);
            noHit = false;
            return (newUnknownType);
        }
    }
    else
    {
        if (ar.xmlPushChildNode(SERX_SHAPE, false))
        {
            CShape *myNewObject = new CShape();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_JOINT, false))
        {
            CJoint *myNewObject = new CJoint();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_GRAPH, false))
        {
            CGraph *myNewObject = new CGraph();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_CAMERA, false))
        {
            CCamera *myNewObject = new CCamera();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_LIGHT, false))
        {
            CLight *myNewObject = new CLight();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_MIRROR, false))
        {
            CMirror *myNewObject = new CMirror();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_OCTREE, false))
        {
            COcTree *myNewObject = new COcTree();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_POINTCLOUD, false))
        {
            CPointCloud *myNewObject = new CPointCloud();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_DUMMY, false))
        {
            CDummy *myNewObject = new CDummy();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_PROXIMITYSENSOR, false))
        {
            CProxSensor *myNewObject = new CProxSensor();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_VISIONSENSOR, false))
        {
            CVisionSensor *myNewObject = new CVisionSensor();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_PATH_OLD, false))
        {
            CPath_old *myNewObject = new CPath_old();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_MILL, false))
        {
            CMill *myNewObject = new CMill();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
        if (ar.xmlPushChildNode(SERX_FORCESENSOR, false))
        {
            CForceSensor *myNewObject = new CForceSensor();
            myNewObject->serialize(ar);
            ar.xmlPopNode();
            return (myNewObject);
        }
    }
    return (nullptr); // No, this is not a scene object!
}

void CSceneObjectContainer::writeSceneObject(CSer &ar, CSceneObject *it)
{
    if (ar.isBinary())
    {
        if (it->getObjectType() == sim_object_shape_type)
            ar.storeDataName(SER_SHAPE);
        if (it->getObjectType() == sim_object_joint_type)
            ar.storeDataName(SER_JOINT);
        if (it->getObjectType() == sim_object_graph_type)
            ar.storeDataName(SER_GRAPH);
        if (it->getObjectType() == sim_object_camera_type)
            ar.storeDataName(SER_CAMERA);
        if (it->getObjectType() == sim_object_light_type)
            ar.storeDataName(SER_LIGHT);
        if (it->getObjectType() == sim_object_mirror_type)
            ar.storeDataName(SER_MIRROR);
        if (it->getObjectType() == sim_object_octree_type)
            ar.storeDataName(SER_OCTREE);
        if (it->getObjectType() == sim_object_pointcloud_type)
            ar.storeDataName(SER_POINTCLOUD);
        if (it->getObjectType() == sim_object_dummy_type)
            ar.storeDataName(SER_DUMMY);
        if (it->getObjectType() == sim_object_script_type)
            ar.storeDataName(SER_SCRIPT);
        if (it->getObjectType() == sim_object_proximitysensor_type)
            ar.storeDataName(SER_PROXIMITYSENSOR);
        if (it->getObjectType() == sim_object_visionsensor_type)
            ar.storeDataName(SER_VISIONSENSOR);
        if (it->getObjectType() == sim_object_path_type)
            ar.storeDataName(SER_PATH_OLD);
        if (it->getObjectType() == sim_object_mill_type)
            ar.storeDataName(SER_MILL);
        if (it->getObjectType() == sim_object_forcesensor_type)
            ar.storeDataName(SER_FORCESENSOR);
        ar.setCountingMode();
        it->serialize(ar);
        if (ar.setWritingMode())
            it->serialize(ar);
    }
    else
    {
        xmlNode *node;
        if (it->getObjectType() == sim_object_shape_type)
            ar.xmlPushNewNode(SERX_SHAPE);
        if (it->getObjectType() == sim_object_joint_type)
            ar.xmlPushNewNode(SERX_JOINT);
        if (it->getObjectType() == sim_object_graph_type)
            ar.xmlPushNewNode(SERX_GRAPH);
        if (it->getObjectType() == sim_object_camera_type)
            ar.xmlPushNewNode(SERX_CAMERA);
        if (it->getObjectType() == sim_object_light_type)
            ar.xmlPushNewNode(SERX_LIGHT);
        if (it->getObjectType() == sim_object_mirror_type)
            ar.xmlPushNewNode(SERX_MIRROR);
        if (it->getObjectType() == sim_object_octree_type)
            ar.xmlPushNewNode(SERX_OCTREE);
        if (it->getObjectType() == sim_object_pointcloud_type)
            ar.xmlPushNewNode(SERX_POINTCLOUD);
        if (it->getObjectType() == sim_object_dummy_type)
            ar.xmlPushNewNode(SERX_DUMMY);
        if (it->getObjectType() == sim_object_proximitysensor_type)
            ar.xmlPushNewNode(SERX_PROXIMITYSENSOR);
        if (it->getObjectType() == sim_object_visionsensor_type)
            ar.xmlPushNewNode(SERX_VISIONSENSOR);
        if (it->getObjectType() == sim_object_path_type)
            ar.xmlPushNewNode(SERX_PATH_OLD);
        if (it->getObjectType() == sim_object_mill_type)
            ar.xmlPushNewNode(SERX_MILL);
        if (it->getObjectType() == sim_object_forcesensor_type)
            ar.xmlPushNewNode(SERX_FORCESENSOR);
        it->serialize(ar);
        ar.xmlPopNode();
    }
}

bool CSceneObjectContainer::readAndAddToSceneSimpleXmlSceneObjects(CSer &ar, CSceneObject *parentObject,
                                                                   const C7Vector &localFramePreCorrection,
                                                                   std::vector<SSimpleXmlSceneObject> &simpleXmlObjects)
{
    bool retVal = true;
    bool isScene = (ar.getFileType() == CSer::filetype_csim_xml_simplescene_file);
    std::vector<std::string> allChildNodeNames;
    ar.xmlGetAllChildNodeNames(allChildNodeNames);
    for (size_t nodeIt = 0; nodeIt < allChildNodeNames.size(); nodeIt++)
    {
        std::string nm(allChildNodeNames[nodeIt]);
        if (nodeIt == 0)
            ar.xmlPushChildNode(nm.c_str(), false);
        else
            ar.xmlPushSiblingNode(nm.c_str(), false);
        std::string objNames(
            "shape*joint*graph*camera*dummy*proximitySensor*visionSensor*forceSensor*path*light*ocTree*pointCloud");
        if (objNames.find(nm) != std::string::npos)
        {
            C7Vector desiredLocalFrame;
            CSceneObject *obj = nullptr;
            if (nm.compare("shape") == 0)
                obj = _readSimpleXmlShape(ar, desiredLocalFrame); // special, added to scene already. Can fail
            if (nm.compare("joint") == 0)
            {
                CJoint *myNewObject = new CJoint();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("graph") == 0)
            {
                CGraph *myNewObject = new CGraph();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("camera") == 0)
            {
                CCamera *myNewObject = new CCamera();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("dummy") == 0)
            {
                CDummy *myNewObject = new CDummy();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("script") == 0)
            {
                CScript *myNewObject = new CScript();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("proximitySensor") == 0)
            {
                CProxSensor *myNewObject = new CProxSensor();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("visionSensor") == 0)
            {
                CVisionSensor *myNewObject = new CVisionSensor();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("forceSensor") == 0)
            {
                CForceSensor *myNewObject = new CForceSensor();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("path") == 0)
            {
                CPath_old *myNewObject = new CPath_old();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("light") == 0)
            {
                CLight *myNewObject = new CLight();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("ocTree") == 0)
            {
                COcTree *myNewObject = new COcTree();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }
            if (nm.compare("pointCloud") == 0)
            {
                CPointCloud *myNewObject = new CPointCloud();
                myNewObject->serialize(ar);
                obj = myNewObject;
                addObjectToScene(obj, false, true);
            }

            if (obj != nullptr)
            {
                if (obj->getObjectType() != sim_object_shape_type)
                    desiredLocalFrame = obj->getLocalTransformation();
                C7Vector localFramePreCorrectionForChildren(obj->getLocalTransformation().getInverse() *
                                                            desiredLocalFrame);
                obj->setLocalTransformation(localFramePreCorrection * obj->getLocalTransformation());

                // Handle attached child scripts:
                CScriptObject *childScript = nullptr;
                if (ar.xmlPushChildNode("childScript", false))
                {
                    childScript = new CScriptObject(sim_scripttype_childscript);
                    childScript->serialize(ar);
                    ar.xmlPopNode();
                }

                // Handle attached customization scripts:
                CScriptObject *customizationScript = nullptr;
                if (ar.xmlPushChildNode("customizationScript", false))
                {
                    customizationScript = new CScriptObject(sim_scripttype_customizationscript);
                    customizationScript->serialize(ar);
                    ar.xmlPopNode();
                }

                SSimpleXmlSceneObject xmlobj;
                xmlobj.object = obj;
                xmlobj.parentObject = parentObject;
                xmlobj.childScript = childScript;
                xmlobj.customizationScript = customizationScript;
                simpleXmlObjects.push_back(xmlobj);

                // Possibly recurse:
                readAndAddToSceneSimpleXmlSceneObjects(ar, obj, localFramePreCorrectionForChildren, simpleXmlObjects);
            }
        }
        if (nodeIt == allChildNodeNames.size() - 1)
            ar.xmlPopNode();
    }
    return (retVal);
}

void CSceneObjectContainer::writeSimpleXmlSceneObjectTree(CSer &ar, const CSceneObject *object)
{
    if (object->getObjectType() == sim_object_shape_type)
    {
        CShape *obj = (CShape *)object;
        ar.xmlPushNewNode("shape");
        _writeSimpleXmlShape(ar, obj);
    }
    if (object->getObjectType() == sim_object_joint_type)
    {
        CJoint *obj = (CJoint *)object;
        ar.xmlPushNewNode("joint");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_graph_type)
    {
        CGraph *obj = (CGraph *)object;
        ar.xmlPushNewNode("graph");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_camera_type)
    {
        CCamera *obj = (CCamera *)object;
        ar.xmlPushNewNode("camera");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_dummy_type)
    {
        CDummy *obj = (CDummy *)object;
        ar.xmlPushNewNode("dummy");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_script_type)
    {
        CScript *obj = (CScript *)object;
        ar.xmlPushNewNode("script");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_proximitysensor_type)
    {
        CProxSensor *obj = (CProxSensor *)object;
        ar.xmlPushNewNode("proximitySensor");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_path_type)
    {
        CPath_old *obj = (CPath_old *)object;
        ar.xmlPushNewNode("path");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_visionsensor_type)
    {
        CVisionSensor *obj = (CVisionSensor *)object;
        ar.xmlPushNewNode("visionSensor");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_forcesensor_type)
    {
        CForceSensor *obj = (CForceSensor *)object;
        ar.xmlPushNewNode("forceSensor");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_light_type)
    {
        CLight *obj = (CLight *)object;
        ar.xmlPushNewNode("light");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_octree_type)
    {
        COcTree *obj = (COcTree *)object;
        ar.xmlPushNewNode("ocTree");
        obj->serialize(ar);
    }
    if (object->getObjectType() == sim_object_pointcloud_type)
    {
        CPointCloud *obj = (CPointCloud *)object;
        ar.xmlPushNewNode("pointCloud");
        obj->serialize(ar);
    }

    CScriptObject *script = embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript, object->getObjectHandle());
    if (script != nullptr)
    {
        ar.xmlPushNewNode("childScript");
        script->serialize(ar);
        ar.xmlPopNode();
    }
    script = embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript, object->getObjectHandle());
    if (script != nullptr)
    {
        ar.xmlPushNewNode("customizationScript");
        script->serialize(ar);
        ar.xmlPopNode();
    }

    for (size_t i = 0; i < object->getChildCount(); i++)
        writeSimpleXmlSceneObjectTree(ar, object->getChildFromIndex(i));

    ar.xmlPopNode();
}

bool CSceneObjectContainer::setObjectParent(CSceneObject *object, CSceneObject *newParent, bool keepInPlace)
{
    TRACE_INTERNAL;
    bool retVal = false;
    if ((newParent == nullptr) || (!newParent->hasAncestor(object)))
    {
        retVal = true;
        checkObjectIsInstanciated(object, __func__);
        if (newParent != nullptr)
            checkObjectIsInstanciated(newParent, __func__);
        CSceneObject *oldParent = object->getParent();
        if (oldParent != newParent)
        {
            _hierarchyChangeCounter++;
            C7Vector absTr(object->getCumulativeTransformation());
            if (oldParent != nullptr)
            {
                oldParent->removeChild(object);
                if (oldParent->getObjectType() == sim_object_joint_type)
                    ((CJoint *)oldParent)->setIntrinsicTransformationError(C7Vector::identityTransformation);
                if (oldParent->getObjectType() == sim_object_forcesensor_type)
                    ((CForceSensor *)oldParent)->setIntrinsicTransformationError(C7Vector::identityTransformation);
            }
            else
                _removeFromOrphanObjects(object);
            if (newParent != nullptr)
            {
                newParent->addChild(object);
                if (newParent->getObjectType() == sim_object_joint_type)
                    ((CJoint *)newParent)->setIntrinsicTransformationError(C7Vector::identityTransformation);
                if (newParent->getObjectType() == sim_object_forcesensor_type)
                    ((CForceSensor *)newParent)->setIntrinsicTransformationError(C7Vector::identityTransformation);
            }
            else
                _addToOrphanObjects(object);
            object->setParent(newParent);
            object->recomputeModelInfluencedValues();
            _handleOrderIndexOfOrphans();

            if (keepInPlace)
            {
                C7Vector parentTr(object->getFullParentCumulativeTransformation());
                object->setLocalTransformation(parentTr.getInverse() * absTr);
            }

            App::currentWorld->sceneObjects->actualizeObjectInformation();
        }
    }
    return (retVal);
}

bool CSceneObjectContainer::setObjectAlias(CSceneObject *object, const char *newAlias, bool allowNameAdjustment)
{
    bool retVal = false;
    checkObjectIsInstanciated(object, __func__);
    if (allowNameAdjustment || tt::isAliasValid(newAlias))
    {
        std::string nm(tt::getValidAlias(newAlias));
        if (nm != object->getObjectAlias())
        {
            retVal = true;
            object->setObjectAlias_direct(nm.c_str());
            if (object->getParent() == nullptr)
                _handleOrderIndexOfOrphans();
            else
                object->getParent()->handleOrderIndexOfChildren();
        }
    }
    if (retVal)
    {
        std::string newName(object->getObjectAlias());
        int sn = tt::getNameSuffixNumber(object->getObjectName_old().c_str(), true);
        if (sn >= 0)
            newName = newName + "#" + std::to_string(sn);
        setObjectName_old(object, newName.c_str(), true);
#ifdef SIM_WITH_GUI
        GuiApp::setRebuildHierarchyFlag();
#endif
    }
    return (retVal);
}

bool CSceneObjectContainer::setObjectSequence(CSceneObject *object, int order)
{
    bool retVal = false;
    CSceneObject *parent = object->getParent();
    if (parent == nullptr)
    {
        if ( (order < int(_orphanObjects.size())) && (order >= -int(_orphanObjects.size())) )
        {
            if (order < 0)
                order += int(_orphanObjects.size()); // neg. value: counting from back
            for (size_t i = 0; i < _orphanObjects.size(); i++)
            {
                if (_orphanObjects[i] == object)
                {
                    if (order != i)
                    {
                        _orphanObjects.erase(_orphanObjects.begin() + i);
                        _orphanObjects.insert(_orphanObjects.begin() + order, object);
                        _handleOrderIndexOfOrphans();
                    }
                    retVal = true;
                    break;
                }
            }
        }
    }
    else
        retVal = parent->setChildSequence(object, order);
#ifdef SIM_WITH_GUI
    if (retVal)
    {
        GuiApp::setFullDialogRefreshFlag();
        GuiApp::setRebuildHierarchyFlag();
    }
#endif
    return (retVal);
}

bool CSceneObjectContainer::setObjectName_old(CSceneObject *object, const char *newName, bool allowNameAdjustment)
{
    checkObjectIsInstanciated(object, __func__);
    std::string nm(newName);
    bool retVal = false;
    if (nm != object->getObjectName_old())
    {
        if (allowNameAdjustment || tt::isObjectNameValid_old(newName, !tt::isHashFree(newName)))
        {
            tt::removeIllegalCharacters(nm, true);
            bool renamed = false;
            while (getObjectFromName_old(nm.c_str()) != nullptr)
            {
                renamed = true;
                nm = tt::generateNewName_hashOrNoHash(nm.c_str(), !tt::isHashFree(nm.c_str()));
            }
            if (allowNameAdjustment || (!renamed))
            {
                std::map<std::string, CSceneObject *>::iterator it = _objectNameMap_old.find(nm);
                if (it == _objectNameMap_old.end() && (nm.size() != 0))
                {
                    _objectNameMap_old.erase(object->getObjectName_old());
                    _objectNameMap_old[nm] = object;
                    object->setObjectName_direct_old(nm.c_str());
                    retVal = true;
                }
            }
        }
    }
    return (retVal);
}

bool CSceneObjectContainer::setObjectAltName_old(CSceneObject *object, const char *newName, bool allowNameAdjustment)
{
    checkObjectIsInstanciated(object, __func__);
    std::string nm(newName);
    bool retVal = false;
    if (allowNameAdjustment || tt::isObjectNameValid_old(newName, false))
    {
        tt::removeAltNameIllegalCharacters(nm);
        bool renamed = false;
        while (getObjectFromAltName_old(nm.c_str()) != nullptr)
        {
            renamed = true;
            nm = tt::generateNewName_noHash(nm.c_str());
        }
        if (allowNameAdjustment || (!renamed))
        {
            std::map<std::string, CSceneObject *>::iterator it = _objectAltNameMap_old.find(nm);
            if (it == _objectNameMap_old.end() && (nm.size() != 0))
            {
                _objectAltNameMap_old.erase(object->getObjectAltName_old());
                _objectAltNameMap_old[nm] = object;
                object->setObjectAltName_direct_old(nm.c_str());
                retVal = true;
            }
        }
    }
    return (retVal);
}

bool CSceneObjectContainer::setSelectedObjectHandles(const std::vector<int> *v)
{
    std::vector<int> prevSel(getSelectedObjectHandlesPtr()->begin(), getSelectedObjectHandlesPtr()->end());

    bool diff = false;
    if (v == nullptr)
        diff = (_selectedObjectHandles.size() > 0);
    else
    {
        diff = (v->size() != _selectedObjectHandles.size());
        if (!diff)
        {
            for (size_t i = 0; i < _selectedObjectHandles.size(); i++)
            {
                if (_selectedObjectHandles[i] != v->at(i))
                {
                    diff = true;
                    break;
                }
            }
        }
    }
    if (diff)
    {
        // First make sure that handles are valid
        std::vector<int> w;
        if (v != nullptr)
        {
            for (size_t i = 0; i < v->size(); i++)
            {
                CSceneObject *it = getObjectFromHandle(v->at(i));
                if (it != nullptr)
                    w.push_back(it->getObjectHandle());
            }
        }
        _selectedObjectHandles.clear();
        if (v != nullptr)
            _selectedObjectHandles.assign(w.begin(), w.end());

        for (size_t i = 0; i < prevSel.size(); i++)
        {
            CSceneObject *it = getObjectFromHandle(prevSel[i]);
            it->setSelected(false);
        }
        for (size_t i = 0; i < getSelectedObjectHandlesPtr()->size(); i++)
        {
            CSceneObject *it = getObjectFromHandle(getSelectedObjectHandlesPtr()->at(i));
            it->setSelected(true);
        }
    }
    return (diff);
}

void CSceneObjectContainer::_handleOrderIndexOfOrphans()
{
    std::map<std::string, int> nameMap;
    std::vector<int> co(getOrphanCount());
    for (size_t i = 0; i < getOrphanCount(); i++)
    {
        CSceneObject *child = getOrphanFromIndex(i);
        std::string hn(child->getObjectAlias());
        std::map<std::string, int>::iterator it = nameMap.find(hn);
        if (it == nameMap.end())
            nameMap[hn] = 0;
        else
            nameMap[hn]++;
        co[i] = nameMap[hn];
    }
    for (size_t i = 0; i < getOrphanCount(); i++)
    {
        CSceneObject *child = getOrphanFromIndex(i);
        std::string hn(child->getObjectAlias());
        std::map<std::string, int>::iterator it = nameMap.find(hn);
        if (nameMap[hn] == 0)
            co[i] = -1; // means unique with that name, with that parent
        child->setChildOrder(co[i]);
    }
#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
    GuiApp::setRebuildHierarchyFlag();
#endif
}

void CSceneObjectContainer::setObjectAbsolutePose(int objectHandle, const C7Vector &v, bool keepChildrenInPlace)
{
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if (it != nullptr)
    {
        C7Vector childPreTr(it->getFullLocalTransformation());
        C7Vector parentInverse(it->getFullParentCumulativeTransformation().getInverse());
        it->setLocalTransformation(parentInverse * v);
        if (keepChildrenInPlace)
        {
            childPreTr = it->getFullLocalTransformation().getInverse() * childPreTr;
            for (size_t i = 0; i < it->getChildCount(); i++)
            {
                CSceneObject *child = it->getChildFromIndex(i);
                child->setLocalTransformation(childPreTr * child->getLocalTransformation());
            }
        }
    }
}

void CSceneObjectContainer::setObjectAbsolutePosition(int objectHandle, const C3Vector &p)
{
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if (it != nullptr)
    {
        C7Vector cumul(it->getCumulativeTransformation());
        C7Vector parentInverse(it->getFullParentCumulativeTransformation().getInverse());
        cumul.X = p;
        it->setLocalTransformation(parentInverse * cumul);
    }
}

void CSceneObjectContainer::setObjectAbsoluteOrientation(int objectHandle, const C3Vector &euler)
{
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if (it != nullptr)
    {
        C7Vector cumul(it->getCumulativeTransformation());
        C7Vector parentInverse(it->getFullParentCumulativeTransformation().getInverse());
        cumul.Q.setEulerAngles(euler);
        it->setLocalTransformation(parentInverse * cumul);
    }
}

int CSceneObjectContainer::getHighestObjectHandle() const
{
    int highest = -1;
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int h = getObjectFromIndex(i)->getObjectHandle();
        if (h > highest)
            highest = h;
    }
    return (highest);
}

bool CSceneObjectContainer::isSelectionSame(std::vector<int> &sel, bool actualize) const
{
    bool retVal = true;
    const std::vector<int> *_sel = getSelectedObjectHandlesPtr();
    if (_sel->size() == sel.size())
    {
        for (size_t i = 0; i < _sel->size(); i++)
        {
            if (_sel->at(i) != sel[i])
            {
                retVal = false;
                break;
            }
        }
    }
    else
        retVal = false;
    if ((!retVal) && actualize)
        sel.assign(_sel->begin(), _sel->end());
    return (retVal);
}

void CSceneObjectContainer::selectObject(int objectHandle)
{
    std::vector<int> sel;
    sel.push_back(objectHandle);
    if (setSelectedObjectHandles(&sel))
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->editModeContainer->announceObjectSelectionChanged();
        GuiApp::setLightDialogRefreshFlag();
#endif
    }
}

void CSceneObjectContainer::selectAllObjects()
{
    std::vector<int> sel;
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        sel.push_back(it->getObjectHandle());
    }
    if (setSelectedObjectHandles(&sel))
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->editModeContainer->announceObjectSelectionChanged();
        GuiApp::setLightDialogRefreshFlag();
#endif
    }
}

void CSceneObjectContainer::deselectObjects()
{
    if (setSelectedObjectHandles(nullptr))
    {
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            GuiApp::mainWindow->editModeContainer->announceObjectSelectionChanged();
        GuiApp::setLightDialogRefreshFlag();
#endif
    }
}

void CSceneObjectContainer::addModelObjects(std::vector<int> &selection) const
{
    std::unordered_set<int> objectsInOutputList;
    for (size_t i = 0; i < selection.size(); i++)
    {
        CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(selection[i]);
        objectsInOutputList.insert(it->getObjectHandle());
        if (it->getModelBase())
        {
            std::vector<CSceneObject *> newObjs;
            it->getAllObjectsRecursive(&newObjs, false, true);
            for (size_t j = 0; j < newObjs.size(); j++)
            {
                CSceneObject *it2 = newObjs[j];
                if (objectsInOutputList.find(it2->getObjectHandle()) == objectsInOutputList.end())
                {
                    selection.insert(selection.begin(), it2->getObjectHandle());
                    objectsInOutputList.insert(it2->getObjectHandle());
                }
            }
        }
    }
}

void CSceneObjectContainer::addCompatibilityScripts(std::vector<int> &selection) const
{
    std::unordered_set<int> objectsInOutputList;
    for (size_t i = 0; i < selection.size(); i++)
    {
        CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(selection[i]);
        objectsInOutputList.insert(it->getObjectHandle());
    }
    for (size_t i = 0; i < _scriptList.size(); i++)
    {
        CScript* s = _scriptList[i];
        if (objectsInOutputList.find(s->getObjectHandle()) == objectsInOutputList.end())
        { // script not (yet) in selection
            CSceneObject* parent = s->getParent();
            int p = -1;
            if (parent != nullptr)
                p = parent->getObjectHandle();
            if (objectsInOutputList.find(p) != objectsInOutputList.end())
            { // parent is in selection
                if (s->scriptObject->getParentIsProxy())
                { // script in compatibility mode. Ok, we add it (mainly for backw. compat. copy operations)
                    objectsInOutputList.insert(s->getObjectHandle());
                    selection.push_back(s->getObjectHandle());
                }
            }
        }
    }
}

void CSceneObjectContainer::addObjectToSelection(int objectHandle)
{
    if (objectHandle >= 0)
    {
        if (objectHandle >= NON_OBJECT_PICKING_ID_PATH_PTS_START) // individual path points!
        {
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->editModeContainer->pathPointManipulation->addPathPointToSelection_nonEditMode(
                    objectHandle);
#endif
        }
        else
        {
            CSceneObject *it = getObjectFromHandle(objectHandle);
            if (it != nullptr)
            {
                if (!it->getSelected())
                {
                    std::vector<int> _sel(getSelectedObjectHandlesPtr()[0]);
                    _sel.push_back(objectHandle);
                    App::currentWorld->buttonBlockContainer->aSceneObjectWasSelected(objectHandle);
                    if (setSelectedObjectHandles(&_sel))
                    {
#ifdef SIM_WITH_GUI
                        if (GuiApp::mainWindow != nullptr)
                            GuiApp::mainWindow->editModeContainer->announceObjectSelectionChanged();
#endif
                    }
                }
            }
        }
    }
#ifdef SIM_WITH_GUI
    GuiApp::setLightDialogRefreshFlag();
#endif
}

void CSceneObjectContainer::removeObjectFromSelection(int objectHandle)
{
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if (it != nullptr)
    {
        if (it->getSelected())
        {
            std::vector<int> sel(getSelectedObjectHandlesPtr()[0]);
            for (size_t i = 0; i < sel.size(); i++)
            {
                if (sel[i] == objectHandle)
                {
                    sel.erase(sel.begin() + i);
                    if (setSelectedObjectHandles(&sel))
                    {
#ifdef SIM_WITH_GUI
                        if (GuiApp::mainWindow != nullptr)
                            GuiApp::mainWindow->editModeContainer->announceObjectSelectionChanged();
                        GuiApp::setLightDialogRefreshFlag();
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
    if (objectHandle != -1)
    {
        if (objectHandle >= NON_OBJECT_PICKING_ID_PATH_PTS_START) // individual path points!
        {
#ifdef SIM_WITH_GUI
            if (GuiApp::mainWindow != nullptr)
                GuiApp::mainWindow->editModeContainer->pathPointManipulation->xorAddPathPointToSelection_nonEditMode(
                    objectHandle);
#endif
        }
        else
        {
            CSceneObject *theObject = getObjectFromHandle(objectHandle);
            if (theObject != nullptr)
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
    std::vector<CSceneObject *> modelBases;
    std::vector<CSceneObject *> nonModelBasesBuildOnNothing;
    for (size_t i = 0; i < sel.size(); i++)
    {
        CSceneObject *it = getObjectFromHandle(sel[i]);
        if (it->getModelBase())
            modelBases.push_back(it);
        else
        {
            CSceneObject *objIt = it;
            bool addIt = true;
            while (objIt->getParent() != nullptr)
            {
                objIt = objIt->getParent();
                if (objIt->getModelBase())
                {
                    addIt = false;
                    break;
                }
            }
            if (addIt)
                nonModelBasesBuildOnNothing.push_back(it);
        }
    }
    for (size_t i = 0; i < modelBases.size(); i++)
    {
        CSceneObject *it = modelBases[i];
        bool isIndependentBase = true;
        while (it->getParent() != nullptr)
        {
            it = it->getParent();
            for (size_t j = 0; j < modelBases.size(); j++)
            {
                if (modelBases[j] == it)
                {
                    isIndependentBase = false;
                    break;
                }
            }
        }
        if (isIndependentBase)
            addObjectToSelection(modelBases[i]->getObjectHandle());
    }
    if (keepObjectsSelectedThatAreNotBuiltOnAModelBase)
    {
        for (size_t i = 0; i < nonModelBasesBuildOnNothing.size(); i++)
            addObjectToSelection(nonModelBasesBuildOnNothing[i]->getObjectHandle());
    }
}

CShape *CSceneObjectContainer::_readSimpleXmlShape(CSer &ar, C7Vector &desiredLocalFrame)
{
    CDummy *dummy = new CDummy();
    dummy->serialize(ar); // we later transfer the common data to the shape object
    desiredLocalFrame = dummy->getFullLocalTransformation();
    CShape *shape = _createSimpleXmlShape(ar, false, nullptr, false);
    if (shape != nullptr)
    {
        if (ar.xmlPushChildNode("dynamics", false))
        {
            int m;
            if (ar.xmlGetNode_int("respondableMask", m, false))
                shape->setDynamicCollisionMask((unsigned short)m);
            C3Vector vel;
            if (ar.xmlGetNode_floats("initialLinearVelocity", vel.data, 3, false))
                shape->setInitialDynamicLinearVelocity(vel);
            if (ar.xmlGetNode_floats("initialAngularVelocity", vel.data, 3, false))
                shape->setInitialDynamicAngularVelocity(vel * piValue / 180.0);
            double mass = 1.0;
            if (ar.xmlGetNode_float("mass", mass, false))
            {
                if (mass < 0.0000001)
                    mass = 0.0000001;
                shape->getMesh()->setMass(mass);
            }

            // Deprecated:
            // -------------
            C3Vector pmoment_old(0.1, 0.1, 0.1);
            ar.xmlGetNode_floats("principalMomentOfInertia", pmoment_old.data, 3, false);
            C4X4Matrix inertiaFrame_old;
            inertiaFrame_old.setIdentity();
            bool useOldInertiaFrame = false;
            if (ar.xmlPushChildNode("localInertiaFrame", false))
            {
                useOldInertiaFrame = true;
                ar.xmlGetNode_floats("position", inertiaFrame_old.X.data, 3, false);
                C3Vector euler;
                if (ar.xmlGetNode_floats("euler", euler.data, 3, false))
                {
                    euler(0) *= piValue / 180.0;
                    euler(1) *= piValue / 180.0;
                    euler(2) *= piValue / 180.0;
                    inertiaFrame_old.M.setEulerAngles(euler);
                }
                ar.xmlPopNode();
                shape->getMesh()->setCOM(inertiaFrame_old.X);
            }
            C3X3Matrix iMatrix;
            iMatrix.clear();
            double inertia[9] = {0.1, 0.0, 0.0, 0.0, 0.1, 0.0, 0.0, 0.0, 0.1};
            if (ar.xmlGetNode_floats("inertia", inertia, 6, false))
            {
                iMatrix(0, 0) = inertia[0];
                iMatrix(1, 0) = inertia[1];
                iMatrix(2, 0) = inertia[2];
                iMatrix(0, 1) = inertia[1];
                iMatrix(1, 1) = inertia[3];
                iMatrix(2, 1) = inertia[4];
                iMatrix(0, 2) = inertia[2];
                iMatrix(1, 2) = inertia[4];
                iMatrix(2, 2) = inertia[5];
            }
            else
            {
                iMatrix(0, 0) = pmoment_old(0);
                iMatrix(1, 1) = pmoment_old(1);
                iMatrix(2, 2) = pmoment_old(2);
            }
            if (useOldInertiaFrame)
            {
                iMatrix = inertiaFrame_old.M * iMatrix * inertiaFrame_old.M.getTranspose();
                iMatrix /= mass; // in CoppeliaSim we work with the "massless inertia"
                shape->getMesh()->setInertia(iMatrix);
            }
            // -------------
            if (ar.xmlGetNode_floats("inertiaMatrix", inertia, 9, false))
            {
                for (size_t i = 0; i < 3; i++)
                {
                    for (size_t j = 0; j < 3; j++)
                        iMatrix(i, j) = inertia[i * 3 + j];
                }
                iMatrix /= mass; // in CoppeliaSim we work with the "massless inertia"
                shape->getMesh()->setInertia(iMatrix);
            }
            C3Vector com;
            com.clear();
            if (ar.xmlGetNode_floats("centerOfMass", com.data, 3, false))
                shape->getMesh()->setCOM(com);

            if (ar.xmlPushChildNode("switches", false))
            {
                bool b;
                if (ar.xmlGetNode_bool("static", b, false))
                    shape->setStatic(b);
                if (ar.xmlGetNode_bool("respondable", b, false))
                    shape->setRespondable(b);
                if (ar.xmlGetNode_bool("startSleeping", b, false))
                    shape->setStartInDynamicSleeping(b);
                if (ar.xmlGetNode_bool("setToDynamicIfGetsParent", b, false))
                    shape->setSetAutomaticallyToNonStaticIfGetsParent(b);
                ar.xmlPopNode();
            }
            if (ar.xmlPushChildNode("material", false))
            {
                shape->getDynMaterial()->serialize(ar);
                ar.xmlPopNode();
            }
            ar.xmlPopNode();
        }
        C7Vector tr(shape->getFullCumulativeTransformation());
        shape->acquireCommonPropertiesFromObject_simpleXMLLoading(dummy);
        setObjectAlias(shape, dummy->getObjectAlias().c_str(), true);
        setObjectName_old(shape, dummy->getObjectName_old().c_str(), true);
        setObjectAltName_old(shape, dummy->getObjectAltName_old().c_str(), true);
        shape->setLocalTransformation(dummy->getFullCumulativeTransformation() * tr);

        // We cannot decided of the position of the shape (the position is selected at the center of the shape, or at
        // least it used to be like that) But we can decide of the orientation of the shape (most of the time), so do it
        // here (we simply reorient the shape's bounding box):
        if ((!shape->getMesh()->isPure()) || (shape->isCompound()))
        {
            C7Vector oldAbsTr(shape->getCumulativeTransformation());
            C7Vector oldAbsTr2(dummy->getCumulativeTransformation().getInverse() * oldAbsTr);
            C7Vector x(oldAbsTr2 * oldAbsTr.getInverse());
            shape->setLocalTransformation(oldAbsTr2);
            shape->alignBB("world");
            C7Vector newAbsTr2(shape->getCumulativeTransformation());
            C7Vector newAbsTr(x.getInverse() * newAbsTr2);
            shape->setLocalTransformation(newAbsTr);
        }
    }
    delete dummy;
    return (shape);
}

CShape *CSceneObjectContainer::_createSimpleXmlShape(CSer &ar, bool noHeightfield, const char *itemType,
                                                     bool checkSibling)
{
    CShape *retVal = nullptr;
    bool loadVisualAttributes = false;
    int t = -1;
    if ((!checkSibling) && ((itemType == nullptr) || (std::string("primitive").compare(itemType) == 0)) &&
        ar.xmlPushChildNode("primitive", false))
        t = 0;
    else
    {
        if ((itemType != nullptr) && std::string("primitive").compare(itemType) == 0)
        {
            if (checkSibling)
            {
                if (ar.xmlPushSiblingNode("primitive", false))
                    t = 0;
                else
                    ar.xmlPopNode();
            }
        }
    }
    if (t == 0)
    {
        loadVisualAttributes = true;
        int primitiveType = 0;
        ar.xmlGetNode_enum("type", primitiveType, false, "cuboid", 0, "sphere", 1, "cylinder", 2, "cone", 3, "plane", 4,
                           "disc", 5, "capsule", 6);
        double sizes[3] = {0.1, 0.1, 0.1};
        ar.xmlGetNode_floats("size", sizes, 3, false);
        C7Vector tr;
        tr.setIdentity();
        if (ar.xmlPushChildNode("localFrame", false))
        {
            ar.xmlGetNode_floats("position", tr.X.data, 3, false);
            C3Vector euler;
            if (ar.xmlGetNode_floats("euler", euler.data, 3, false))
            {
                euler(0) *= piValue / 180.0;
                euler(1) *= piValue / 180.0;
                euler(2) *= piValue / 180.0;
                tr.Q.setEulerAngles(euler);
            }
            ar.xmlPopNode();
        }
        int pType = -1;
        C3Vector s(tt::getLimitedFloat(0.00001, 100000.0, sizes[0]), tt::getLimitedFloat(0.00001, 100000.0, sizes[1]),
                   tt::getLimitedFloat(0.00001, 100000.0, sizes[2]));
        if (primitiveType == 0) // cuboid
            pType = sim_primitiveshape_cuboid;
        if (primitiveType == 1) // sphere
        {
            pType = sim_primitiveshape_spheroid;
            s(1) = s(0);
            s(2) = s(0);
        }
        if (primitiveType == 2) // cylinder
        {
            pType = sim_primitiveshape_cylinder;
            s(1) = s(0);
        }
        if (primitiveType == 3) // cone
        {
            pType = sim_primitiveshape_cone;
            s(1) = s(0);
        }
        if (primitiveType == 4) // plane
            pType = sim_primitiveshape_plane;
        if (primitiveType == 5) // disc
            pType = sim_primitiveshape_disc;
        if (primitiveType == 6) // capsule
        {
            pType = sim_primitiveshape_capsule;
            s(1) = s(0);
        }
        retVal = CAddOperations::addPrimitiveShape(pType, s);
        retVal->setLocalTransformation(tr);
    }
    if (!noHeightfield)
    {
        if ((!checkSibling) && ((itemType == nullptr) || (std::string("heightfield").compare(itemType) == 0)) &&
            ar.xmlPushChildNode("heightfield", false))
            t = 1;
        else
        {
            if ((itemType != nullptr) && std::string("heightfield").compare(itemType) == 0)
            {
                if (checkSibling)
                {
                    if (ar.xmlPushSiblingNode("heightfield", false))
                        t = 1;
                    else
                        ar.xmlPopNode();
                }
            }
        }
    }
    if (t == 1)
    {
        loadVisualAttributes = true;
        int size[2] = {2, 2};
        double meshSize = 0.1;
        std::vector<double> data;
        ar.xmlGetNode_ints("size", size, 2, false);
        if (ar.xmlGetNode_float("gridStep", meshSize, false))
            tt::limitValue(0.00001, 10.0, meshSize);
        ar.xmlGetNode_floats("data", data, false);
        if (data.size() != size[0] * size[1])
        {
            size[0] = 2;
            size[1] = 2;
            data.clear();
            data.push_back(0.0);
            data.push_back(0.0);
            data.push_back(0.0);
            data.push_back(0.0);
        }
        std::vector<std::vector<double> *> allData;
        for (int i = 0; i < size[1]; i++)
        {
            std::vector<double> *vect = new std::vector<double>;
            for (int j = 0; j < size[0]; j++)
                vect->push_back(data[i * size[0] + j]);
            allData.push_back(vect);
        }
        int newShapeHandle =
            CFileOperations::createHeightfield(size[0], meshSize / double(size[0] - 1), allData, 0.0, 0);
        for (size_t i = 0; i < allData.size(); i++)
            delete allData[i];
        retVal = getShapeFromHandle(newShapeHandle);
    }
    if ((!checkSibling) && ((itemType == nullptr) || (std::string("mesh").compare(itemType) == 0)) &&
        ar.xmlPushChildNode("mesh", false))
        t = 2;
    else
    {
        if ((itemType != nullptr) && std::string("mesh").compare(itemType) == 0)
        {
            if (checkSibling)
            {
                if (ar.xmlPushSiblingNode("mesh", false))
                    t = 2;
                else
                    ar.xmlPopNode();
            }
        }
    }
    if (t == 2)
    {
        loadVisualAttributes = true;
        std::string str;
        if (ar.xmlGetNode_string("fileName", str, false))
        { // try to load from file first
            std::string filename(ar.getFilenamePath() + str);
            if (App::worldContainer->pluginContainer->isAssimpPluginAvailable())
            {
                if (VFile::doesFileExist(filename.c_str()))
                {
                    int cnt = 0;
                    int *shapes = App::worldContainer->pluginContainer->assimp_importShapes(filename.c_str(), 512, 1.0,
                                                                                            1, 32 + 128 + 256, &cnt);
                    if (shapes != nullptr)
                    {
                        int newShapeHandle = shapes[0];
                        delete[] shapes;
                        retVal = getShapeFromHandle(newShapeHandle);
                    }
                }
            }
        }
        if (retVal == nullptr)
        { // try to load from vertices and indices list:
            std::vector<double> vertices;
            std::vector<int> indices;
            if (ar.xmlGetNode_floats("vertices", vertices, false))
            {
                if (ar.xmlGetNode_ints("indices", indices, false))
                {
                    bool ok = true;
                    while ((vertices.size() % 3) != 0)
                        vertices.pop_back();
                    while ((indices.size() % 3) != 0)
                        indices.pop_back();
                    for (size_t i = 0; i < indices.size(); i++)
                    {
                        if (indices[i] < 0)
                            indices[i] = int(vertices.size() / 3) + indices[i];
                        if ((indices[i] < 0) || (indices[i] >= (vertices.size() / 3)))
                            ok = false;
                    }
                    if (ok)
                    {
                        retVal = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
                        addObjectToScene(retVal, false, true);
                    }
                }
            }
        }
        if (retVal != nullptr)
        {
            C7Vector tr;
            tr.setIdentity();
            if (ar.xmlPushChildNode("localFrame", false))
            {
                ar.xmlGetNode_floats("position", tr.X.data, 3, false);
                C3Vector euler;
                if (ar.xmlGetNode_floats("euler", euler.data, 3, false))
                {
                    euler(0) *= piValue / 180.0;
                    euler(1) *= piValue / 180.0;
                    euler(2) *= piValue / 180.0;
                    tr.Q.setEulerAngles(euler);
                }
                ar.xmlPopNode();
            }
            retVal->setLocalTransformation(tr * retVal->getFullLocalTransformation());
        }
    }
    if ((!checkSibling) && ((itemType == nullptr) || (std::string("compound").compare(itemType) == 0)) &&
        ar.xmlPushChildNode("compound", false))
        t = 3;
    else
    {
        if ((itemType != nullptr) && std::string("compound").compare(itemType) == 0)
        {
            if (checkSibling)
            {
                if (ar.xmlPushSiblingNode("compound", false))
                    t = 3;
                else
                    ar.xmlPopNode();
            }
        }
    }

    if (t == 3)
    { // compound
        std::vector<int> allShapes;
        CShape *it = _createSimpleXmlShape(ar, true, "primitive", false);
        if (it != nullptr)
        {
            allShapes.push_back(it->getObjectHandle());
            while (true)
            {
                it = _createSimpleXmlShape(ar, true, "primitive", true);
                if (it != nullptr)
                    allShapes.push_back(it->getObjectHandle());
                else
                    break;
            }
        }
        it = _createSimpleXmlShape(ar, true, "mesh", false);
        if (it != nullptr)
        {
            allShapes.push_back(it->getObjectHandle());
            while (true)
            {
                it = _createSimpleXmlShape(ar, true, "mesh", true);
                if (it != nullptr)
                    allShapes.push_back(it->getObjectHandle());
                else
                    break;
            }
        }
        it = _createSimpleXmlShape(ar, true, "compound", false);
        if (it != nullptr)
        {
            allShapes.push_back(it->getObjectHandle());
            while (true)
            {
                it = _createSimpleXmlShape(ar, true, "compound", true);
                if (it != nullptr)
                    allShapes.push_back(it->getObjectHandle());
                else
                    break;
            }
        }
        if (allShapes.size() >= 1)
        {
            int newShapeHandle = -1;
            if (allShapes.size() >= 2)
            {
                newShapeHandle = CSceneObjectOperations::groupSelection(&allShapes);
                CShape *itt = getShapeFromHandle(newShapeHandle);
                itt->relocateFrame("world");
            }
            else
                newShapeHandle = allShapes[0];
            retVal = getShapeFromHandle(newShapeHandle);
        }
        if (itemType == nullptr)
            ar.xmlPopNode();
    }
    if ((retVal != nullptr) && (t != -1) && loadVisualAttributes)
    {
        double v;
        if (ar.xmlGetNode_float("shadingAngle", v, false))
        { // checkHere
            retVal->getSingleMesh()->setShadingAngle(v * piValue / 180.0);
            retVal->getSingleMesh()->setEdgeThresholdAngle(v * piValue / 180.0);
        }
        retVal->setVisibleEdges(false);
        bool b;
        if (ar.xmlGetNode_bool("culling", b, false))
            retVal->setCulling(b);
        if (ar.xmlGetNode_bool("wireframe", b, false))
            retVal->getSingleMesh()->setWireframe_OLD(b);
        if (ar.xmlPushChildNode("color", false))
        {
            int rgb[3];
            if (ar.xmlGetNode_ints("ambientDiffuse", rgb, 3, false))
                retVal->setColor(nullptr, sim_colorcomponent_ambient_diffuse, double(rgb[0]) / 255.1,
                                 double(rgb[1]) / 255.1, double(rgb[2]) / 255.1);
            if (ar.xmlGetNode_ints("specular", rgb, 3, false))
                retVal->setColor(nullptr, sim_colorcomponent_specular, double(rgb[0]) / 255.1, double(rgb[1]) / 255.1,
                                 double(rgb[2]) / 255.1);
            if (ar.xmlGetNode_ints("emission", rgb, 3, false))
                retVal->setColor(nullptr, sim_colorcomponent_emission, double(rgb[0]) / 255.1, double(rgb[1]) / 255.1,
                                 double(rgb[2]) / 255.1);
            ar.xmlPopNode();
        }
        if (itemType == nullptr)
            ar.xmlPopNode();
    }
    return (retVal);
}

void CSceneObjectContainer::_writeSimpleXmlShape(CSer &ar, CShape *shape)
{
    shape->serialize(ar); // will only serialize the common part. The rest has to be done here:

    std::vector<CShape *> allComponents;

    CShape *copy = (CShape *)shape->copyYourself();
    copy->setLocalTransformation(shape->getFullCumulativeTransformation());

    addObjectToScene(copy, false, false);
    if (copy->isCompound())
    {
        int h = copy->getObjectHandle();
        std::vector<int> finalSel;
        std::vector<int> previousSel;
        std::vector<int> sel;
        previousSel.push_back(h);
        sel.push_back(h);
        while (sel.size() != 0)
        {
            CSceneObjectOperations::ungroupSelection(&sel);
            for (size_t i = 0; i < previousSel.size(); i++)
            {
                int previousID = previousSel[i];
                bool present = false;
                for (size_t j = 0; j < sel.size(); j++)
                {
                    if (sel[j] == previousID)
                    {
                        present = true;
                        break;
                    }
                }
                if ((!present) &&
                    (h !=
                     previousID)) // the original shape will be added at the very end for correct ordering (see below)
                    finalSel.push_back(previousID); // this is a simple shape (not a group)
            }
            previousSel.assign(sel.begin(), sel.end());
        }
        finalSel.push_back(h);
        for (size_t i = 0; i < finalSel.size(); i++)
            allComponents.push_back(getShapeFromHandle(finalSel[i]));
    }
    else
        allComponents.push_back(copy);

    ar.xmlAddNode_comment(" one of following tags is required: 'compound', 'primitive', 'heightfield' or 'mesh'. "
                          "'compound' itself requires at least two of those tags as children ",
                          false);
    if (allComponents.size() > 1)
    {
        ar.xmlPushNewNode("compound");
        for (size_t i = 0; i < allComponents.size(); i++)
            _writeSimpleXmlSimpleShape(ar, shape->getObjectAliasAndHandle().c_str(), allComponents[i],
                                       shape->getFullCumulativeTransformation());
        ar.xmlPopNode();
    }
    else
        _writeSimpleXmlSimpleShape(ar, shape->getObjectAliasAndHandle().c_str(), allComponents[0],
                                   shape->getFullCumulativeTransformation());

    for (size_t i = 0; i < allComponents.size(); i++)
        eraseObject(allComponents[i], false);

    ar.xmlPushNewNode("dynamics");

    ar.xmlAddNode_int("respondableMask", shape->getDynamicCollisionMask());
    ar.xmlAddNode_floats("initialLinearVelocity", shape->getInitialDynamicLinearVelocity().data, 3);
    C3Vector vel(shape->getInitialDynamicAngularVelocity() * 180.0 / piValue);
    ar.xmlAddNode_floats("initialAngularVelocity", vel.data, 3);
    ar.xmlAddNode_float("mass", shape->getMesh()->getMass());

    // Deprecated:
    ar.xmlAddNode_comment(" 'localInertiaframe' tag: deprecated, for backward compatibility ", false);
    C3Vector diagI;
    C7Vector tr(shape->getMesh()->getDiagonalInertiaInfo(diagI));
    ar.xmlPushNewNode("localInertiaFrame");
    ar.xmlAddNode_floats("position", tr.X.data, 3);
    C3Vector euler(tr.Q.getEulerAngles());
    euler *= 180.0 / piValue;
    ar.xmlAddNode_floats("euler", euler.data, 3);
    ar.xmlPopNode();
    ar.xmlAddNode_comment(" 'principalMomentOfInertia' tag: deprecated, for backward compatibility ", false);
    ar.xmlAddNode_floats("principalMomentOfInertia", diagI.data, 3);

    ar.xmlAddNode_floats("centerOfMass", shape->getMesh()->getCOM().data, 3);
    C3X3Matrix _im(shape->getMesh()->getInertia());
    _im *= shape->getMesh()->getMass();
    double im[9];
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < 3; j++)
            im[i * 3 + j] = _im(i, j);
    }
    ar.xmlAddNode_floats("inertiaMatrix", im, 9);

    ar.xmlPushNewNode("switches");
    ar.xmlAddNode_bool("static", shape->getStatic());
    ar.xmlAddNode_bool("respondable", shape->getRespondable());
    ar.xmlAddNode_bool("startSleeping", shape->getStartInDynamicSleeping());
    ar.xmlAddNode_bool("setToDynamicIfGetsParent", shape->getSetAutomaticallyToNonStaticIfGetsParent());
    ar.xmlPopNode();

    ar.xmlPushNewNode("material");
    shape->getDynMaterial()->serialize(ar);
    ar.xmlPopNode();

    ar.xmlPopNode();
}

void CSceneObjectContainer::_writeSimpleXmlSimpleShape(CSer &ar, const char *originalShapeName, CShape *shape,
                                                       const C7Vector &frame)
{
    CMesh *geom = shape->getSingleMesh();
    if (geom->getPurePrimitiveType() == sim_primitiveshape_none)
    { // mesh
        ar.xmlPushNewNode("mesh");
        C7Vector trOld(shape->getFullLocalTransformation());
        C7Vector x(frame.getInverse() * trOld);
        shape->setLocalTransformation(C7Vector::identityTransformation);
        ar.xmlAddNode_comment(" one of following tags is required: 'fileName' or 'vertices' and 'indices' ", false);
        if (App::worldContainer->pluginContainer->isAssimpPluginAvailable() &&
            (!ar.xmlSaveDataInline(geom->getVerticesForDisplayAndDisk()->size() + geom->getIndices()->size() * 4)))
        {
            int shapeHandle = shape->getObjectHandle();
            std::string filename(ar.getFilenameBase() + "_mesh_" + originalShapeName +
                                 utils::getIntString(false, ar.getIncrementCounter()) + ".dae");
            bool wireframe = shape->getShapeWireframe_OLD();
            if (wireframe)
                shape->setShapeWireframe_OLD(
                    false); // The Assimp plugin will ignore wireframe shapes and not write them!!
            App::worldContainer->pluginContainer->assimp_exportShapes(
                &shapeHandle, 1, (ar.getFilenamePath() + filename).c_str(), "collada", 1.0, 1, 256);
            if (wireframe)
                shape->setShapeWireframe_OLD(true);
            ar.xmlAddNode_string("fileName", filename.c_str());
        }
        else
        {
            /*
            std::vector<double> v;
            std::vector<int> ind;
            geom->getCumulativeMeshes(C7Vector::identityTransformation,v,&ind,nullptr);
            ar.xmlAddNode_floats("vertices",v);
            ar.xmlAddNode_ints("indices",geom->getIndices()[0]);
            */
            std::vector<float> v;
            v.resize(geom->getVertices()->size());
            for (size_t i = 0; i < geom->getVertices()->size() / 3; i++)
            {
                C3Vector w;
                w.setData(&geom->getVertices()[0][3 * i]);
                w *= geom->getBB(nullptr);
                v[3 * i + 0] = (float)w(0);
                v[3 * i + 1] = (float)w(1);
                v[3 * i + 2] = (float)w(2);
            }
            ar.xmlAddNode_floats("vertices", v);
            ar.xmlAddNode_ints("indices", geom->getIndices()[0]);
        }

        shape->setLocalTransformation(trOld); // restore it
        ar.xmlPushNewNode("localFrame");
        C7Vector tr(x);
        ar.xmlAddNode_floats("position", tr.X.data, 3);
        C3Vector euler(tr.Q.getEulerAngles() * 180.0 / piValue);
        ar.xmlAddNode_floats("euler", euler.data, 3);
        ar.xmlPopNode();
    }
    else if (geom->getPurePrimitiveType() == sim_primitiveshape_heightfield)
    { // heightfield
        ar.xmlPushNewNode("heightfield");

        ar.xmlAddNode_comment(" 'size' tag: required ", false);
        ar.xmlAddNode_2int("size", geom->_heightfieldXCount, geom->_heightfieldYCount);
        C3Vector s;
        geom->getPurePrimitiveSizes(s);
        double gridStep = s(0) / double(geom->_heightfieldXCount - 1);
        ar.xmlAddNode_float("gridStep", gridStep);
        ar.xmlAddNode_comment(" 'data' tag: required. has to contain size[0]*size[1] values ", false);
        ar.xmlAddNode_floats("data", geom->_heightfieldHeights);
    }
    else
    { // primitive
        ar.xmlPushNewNode("primitive");

        ar.xmlAddNode_comment(" 'type' tag: required. Can be one of following: 'cuboid', 'sphere', 'cylinder', 'cone', "
                              "'plane', 'disc' or 'capsule'",
                              false);
        if (geom->getPurePrimitiveType() == sim_primitiveshape_cuboid)
            ar.xmlAddNode_string("type", "cuboid");
        if (geom->getPurePrimitiveType() == sim_primitiveshape_spheroid)
            ar.xmlAddNode_string("type", "sphere");
        if (geom->getPurePrimitiveType() == sim_primitiveshape_cylinder)
            ar.xmlAddNode_string("type", "cylinder");
        if (geom->getPurePrimitiveType() == sim_primitiveshape_cone)
            ar.xmlAddNode_string("type", "cone");
        if (geom->getPurePrimitiveType() == sim_primitiveshape_plane)
            ar.xmlAddNode_string("type", "plane");
        if (geom->getPurePrimitiveType() == sim_primitiveshape_disc)
            ar.xmlAddNode_string("type", "disc");
        if (geom->getPurePrimitiveType() == sim_primitiveshape_capsule)
            ar.xmlAddNode_string("type", "capsule");

        C3Vector s;
        geom->getPurePrimitiveSizes(s);
        ar.xmlAddNode_floats("size", s.data, 3);
        ar.xmlPushNewNode("localFrame");
        C7Vector tr(frame.getInverse() * shape->getFullCumulativeTransformation() * geom->getBB(nullptr));
        ar.xmlAddNode_floats("position", tr.X.data, 3);
        C3Vector euler(tr.Q.getEulerAngles() * 180.0 / piValue);
        ar.xmlAddNode_floats("euler", euler.data, 3);
        ar.xmlPopNode();
    }

    // now the visual attributes:
    ar.xmlAddNode_float("shadingAngle", geom->getShadingAngle() * 180.0 / piValue);
    ar.xmlAddNode_bool("culling", geom->getCulling());
    ar.xmlAddNode_bool("wireframe", geom->getWireframe_OLD());

    ar.xmlPushNewNode("color");
    int rgb[3];
    for (size_t l = 0; l < 3; l++)
        rgb[l] = int(geom->color.getColorsPtr()[l] * 255.1);
    ar.xmlAddNode_ints("ambientDiffuse", rgb, 3);
    for (size_t l = 0; l < 3; l++)
        rgb[l] = int(geom->color.getColorsPtr()[6 + l] * 255.1);
    ar.xmlAddNode_ints("specular", rgb, 3);
    for (size_t l = 0; l < 3; l++)
        rgb[l] = int(geom->color.getColorsPtr()[9 + l] * 255.1);
    ar.xmlAddNode_ints("emission", rgb, 3);
    ar.xmlPopNode();

    ar.xmlPopNode(); // "primitive" or "mesh" or "heightField"
}

void CSceneObjectContainer::_addObject(CSceneObject *object)
{
    object->setSelected(false);

    _orphanObjects.push_back(object);
    _allObjects.push_back(object);
    _objectHandleMap[object->getObjectHandle()] = object;
    _objectNameMap_old[object->getObjectName_old()] = object;
    _objectAltNameMap_old[object->getObjectAltName_old()] = object;
    int t = object->getObjectType();
    if (t == sim_object_joint_type)
        _jointList.push_back((CJoint *)object);
    if (t == sim_object_dummy_type)
        _dummyList.push_back((CDummy *)object);
    if (t == sim_object_script_type)
        _scriptList.push_back((CScript *)object);
    if (t == sim_object_graph_type)
        _graphList.push_back((CGraph *)object);
    if (t == sim_object_light_type)
        _lightList.push_back((CLight *)object);
    if (t == sim_object_camera_type)
        _cameraList.push_back((CCamera *)object);
    if (t == sim_object_proximitysensor_type)
        _proximitySensorList.push_back((CProxSensor *)object);
    if (t == sim_object_visionsensor_type)
        _visionSensorList.push_back((CVisionSensor *)object);
    if (t == sim_object_shape_type)
        _shapeList.push_back((CShape *)object);
    if (t == sim_object_forcesensor_type)
        _forceSensorList.push_back((CForceSensor *)object);
    if (t == sim_object_octree_type)
        _octreeList.push_back((COcTree *)object);
    if (t == sim_object_pointcloud_type)
        _pointCloudList.push_back((CPointCloud *)object);
    if (t == sim_object_mirror_type)
        _mirrorList.push_back((CMirror *)object);
    if (t == sim_object_path_type)
        _pathList.push_back((CPath_old *)object);
    if (t == sim_object_mill_type)
        _millList.push_back((CMill *)object);

    _handleOrderIndexOfOrphans();

#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
    GuiApp::setRebuildHierarchyFlag();
#endif

    actualizeObjectInformation();

    object->buildOrUpdate_oldIk();

    _objectCreationCounter++;
}

void CSceneObjectContainer::handleDataCallbacks()
{
    for (size_t j = 0; j < _allObjects.size(); j++)
    {
        CSceneObject* obj = _allObjects[j];
        std::map<std::string, bool> dataItems;
        if (obj->getCustomDataEvents(dataItems))
        {
            std::vector<CScriptObject*> scripts;
            obj->getAttachedScripts(scripts, -1, true);
            obj->getAttachedScripts(scripts, -1, false);
            CInterfaceStack *stack = App::worldContainer->interfaceStackContainer->createStack();
            stack->pushTableOntoStack();
            for (const auto &r : dataItems)
                stack->insertKeyBoolIntoStackTable(r.first.c_str(), r.second);
            for (size_t i = 0; i < scripts.size(); i++)
                scripts[i]->systemCallScript(sim_syscb_data, stack, nullptr);
            App::worldContainer->interfaceStackContainer->destroyStack(stack);
        }
        obj->clearCustomDataEvents();
    }
}

bool CSceneObjectContainer::shouldTemporarilySuspendMainScript()
{
    bool retVal = false;
    std::vector<int> scriptHandles;
    getScriptsToExecute(scriptHandles, -1, false, false);
    for (size_t i = 0; i < scriptHandles.size(); i++)
    {
        CScriptObject *it = getScriptObjectFromHandle(scriptHandles[i]);
        if (it != nullptr)
        { // could have been erased in the mean time!
            if (it->shouldTemporarilySuspendMainScript())
                retVal = true;
        }
    }
    bool b = embeddedScriptContainer->shouldTemporarilySuspendMainScript();
    return (retVal | b);
}

size_t CSceneObjectContainer::getScriptsToExecute(std::vector<int> &scriptHandles, int scriptType, bool legacyEmbeddedScripts, bool reverseOrder) const
{ // returns all non-disabled scripts, from leaf to root. With scriptType==-1, returns child and customization scripts (but ALL custom. execute last)
    std::vector<CSceneObject *> objects;

    std::vector<CSceneObject *> objectsNormalPriority;
    std::vector<CSceneObject *> objectsLastPriority;
    for (size_t i = 0; i < getOrphanCount(); i++)
    {
        CSceneObject *it = getOrphanFromIndex(i);
        if (it->getScriptExecPriority() == sim_scriptexecorder_first)
            objects.push_back(it);
        if (it->getScriptExecPriority() == sim_scriptexecorder_normal)
            objectsNormalPriority.push_back(it);
        if (it->getScriptExecPriority() == sim_scriptexecorder_last)
            objectsLastPriority.push_back(it);
    }
    objects.insert(objects.end(), objectsNormalPriority.begin(), objectsNormalPriority.end());
    objects.insert(objects.end(), objectsLastPriority.begin(), objectsLastPriority.end());

    std::vector<SScriptInfo> childScripts;
    int childScriptsMaxDepth = -1;
    if ( (scriptType == -1) || ((scriptType & 0x0f) == sim_scripttype_childscript) )
    {
        int t = scriptType;
        if (t == -1)
            t = sim_scripttype_childscript;
        for (size_t i = 0; i < objects.size(); i++)
            childScriptsMaxDepth = std::max<int>(childScriptsMaxDepth, objects[i]->getScriptsInTree(childScripts, t, legacyEmbeddedScripts, 0));
    }

    std::vector<SScriptInfo> customizationScripts;
    int customizationScriptsMaxDepth = -1;
    if ( (scriptType == -1) || (scriptType == sim_scripttype_customizationscript) )
    {
        for (size_t i = 0; i < objects.size(); i++)
            customizationScriptsMaxDepth = std::max<int>(customizationScriptsMaxDepth, objects[i]->getScriptsInTree(customizationScripts, sim_scripttype_customizationscript, legacyEmbeddedScripts, 0));
    }

    if (childScriptsMaxDepth >= 0)
    {
        if (reverseOrder)
        {
            for (int depth = 0; depth <= childScriptsMaxDepth; depth++)
            {
                for (size_t i = 0; i < childScripts.size(); i++)
                {
                    if (childScripts[i].depth == depth)
                        scriptHandles.push_back(childScripts[i].scriptHandle);
                }
            }
        }
        else
        {
            for (int depth = childScriptsMaxDepth; depth >= 0; depth--)
            {
                for (size_t i = 0; i < childScripts.size(); i++)
                {
                    if (childScripts[i].depth == depth)
                        scriptHandles.push_back(childScripts[i].scriptHandle);
                }
            }
        }
    }

    if (customizationScriptsMaxDepth >= 0)
    {
        if (reverseOrder)
        {
            for (int depth = 0; depth <= customizationScriptsMaxDepth; depth++)
            {
                for (size_t i = 0; i < customizationScripts.size(); i++)
                {
                    if (customizationScripts[i].depth == depth)
                        scriptHandles.push_back(customizationScripts[i].scriptHandle);
                }
            }
        }
        else
        {
            for (int depth = customizationScriptsMaxDepth; depth >= 0; depth--)
            {
                for (size_t i = 0; i < customizationScripts.size(); i++)
                {
                    if (customizationScripts[i].depth == depth)
                        scriptHandles.push_back(customizationScripts[i].scriptHandle);
                }
            }
        }
    }

    return (scriptHandles.size());
}

void CSceneObjectContainer::callScripts(int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch /*=nullptr*/, int scriptToExclude /*=-1*/)
{
    bool doNotInterrupt = !CScriptObject::isSystemCallbackInterruptible(callType);
    if (CScriptObject::isSystemCallbackInReverseOrder(callType))
    { // reverse order

        // main script
        if (!App::currentWorld->simulation->isSimulationStopped())
        {
            CScriptObject *script = embeddedScriptContainer->getMainScript();
            if ((script != nullptr) && (script->hasSystemFunctionOrHook(callType) || script->getOldCallMode()))
            {
                if (script->getScriptHandle() != scriptToExclude)
                    script->systemCallMainScript(callType, inStack, outStack);
            }
        }

        // child + customization scripts (legacy + new):
        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
            callScripts_noMainScript(-1, callType, inStack, outStack, objectBranch, scriptToExclude);
    }
    else
    { // regular order, from unimportant, to most important

        // child + customization scripts (legacy + new):
        callScripts_noMainScript(-1, callType, inStack, outStack, objectBranch, scriptToExclude);

        // main script
        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
        {
            if (!App::currentWorld->simulation->isSimulationStopped())
            {
                CScriptObject *script = embeddedScriptContainer->getMainScript();
                if ((script != nullptr) && (script->hasSystemFunctionOrHook(callType) || script->getOldCallMode()))
                {
                    if (script->getScriptHandle() != scriptToExclude)
                        script->systemCallMainScript(callType, inStack, outStack);
                }
            }
        }
    }
}

int CSceneObjectContainer::callScripts_noMainScript(int scriptType, int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch /*=nullptr*/, int scriptToExclude /*=-1*/)
{
    int retVal = 0;
    bool doNotInterrupt = !CScriptObject::isSystemCallbackInterruptible(callType);
    if (CScriptObject::isSystemCallbackInReverseOrder(callType))
    { // reverse order

        retVal += _callScripts(scriptType, callType, inStack, outStack, objectBranch, scriptToExclude);

        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
            retVal += embeddedScriptContainer->callScripts_noMainScript(scriptType, callType, inStack, outStack, objectBranch, scriptToExclude);
    }
    else
    { // regular order, from unimportant, to most important

        retVal += embeddedScriptContainer->callScripts_noMainScript(scriptType, callType, inStack, outStack, objectBranch, scriptToExclude);

        if (doNotInterrupt || (outStack == nullptr) || (outStack->getStackSize() == 0))
            retVal += _callScripts(scriptType, callType, inStack, outStack, objectBranch, scriptToExclude);
    }
    return retVal;
}


int CSceneObjectContainer::_callScripts(int scriptType, int callType, CInterfaceStack *inStack, CInterfaceStack *outStack,
                                           CSceneObject *objectBranch /*=nullptr*/, int scriptToExclude /*=-1*/)
{ // with objectBranch!=nullptr, will return the chain starting at objectBranch up to the main script
    TRACE_INTERNAL;

    int cnt = 0;

    std::vector<int> scriptHandles;
    if (objectBranch == nullptr)
        getScriptsToExecute(scriptHandles, scriptType, false, CScriptObject::isSystemCallbackInReverseOrder(callType));
    else
        objectBranch->getScriptsInChain(scriptHandles, scriptType, false);
    bool canInterrupt = CScriptObject::isSystemCallbackInterruptible(callType);
    for (size_t i = 0; i < scriptHandles.size(); i++)
    {
        CScript *script = getScriptFromHandle(scriptHandles[i]);
        if ((script != nullptr) && (scriptHandles[i] != scriptToExclude))
        { // the script could have been erased in the mean time
            if (script->scriptObject->getThreadedExecution_oldThreads())
            { // is an old, threaded script
                if (callType == sim_scriptthreadresume_launch)
                {
                    if (script->scriptObject->launchThreadedChildScript_oldThreads())
                        cnt++;
                }
                else
                    cnt += script->scriptObject->resumeThreadedChildScriptIfLocationMatch_oldThreads(callType);
            }
            else if (script->scriptObject->hasSystemFunctionOrHook(callType) || script->scriptObject->getOldCallMode())
            { // has the function
                if (script->scriptObject->systemCallScript(callType, inStack, outStack) == 1)
                {
                    cnt++;
                    if (canInterrupt && (outStack != nullptr) && (outStack->getStackSize() != 0))
                        break;
                }
            }
            else
            { // has not the function. Check if we need to support old callbacks:
                int compatCall = -1;
                if (callType == sim_syscb_dyn)
                    compatCall = sim_syscb_dyncallback;
                if (callType == sim_syscb_contact)
                    compatCall = sim_syscb_contactcallback;
                if ((compatCall != -1) && script->scriptObject->hasSystemFunctionOrHook(compatCall))
                {
                    if (script->scriptObject->systemCallScript(compatCall, inStack, outStack) == 1)
                    {
                        cnt++;
                        if (canInterrupt && (outStack != nullptr) && (outStack->getStackSize() != 0))
                            break;
                    }
                }
            }
        }
    }

    return cnt;
}


void CSceneObjectContainer::_removeObject(CSceneObject *object)
{ // Overridden from _CSceneObjectContainer_
    object->setIsInScene(false);
    setObjectParent(object, nullptr, true);
    object->remove_oldIk();
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        if (_allObjects[i] == object)
        {
            _allObjects.erase(_allObjects.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < _orphanObjects.size(); i++)
    {
        if (_orphanObjects[i] == object)
        {
            _orphanObjects.erase(_orphanObjects.begin() + i);
            break;
        }
    }
    int t = object->getObjectType();
    std::vector<CSceneObject *> *list;
    if (t == sim_object_joint_type)
        list = (std::vector<CSceneObject *> *)&_jointList;
    if (t == sim_object_dummy_type)
        list = (std::vector<CSceneObject *> *)&_dummyList;
    if (t == sim_object_script_type)
        list = (std::vector<CSceneObject *> *)&_scriptList;
    if (t == sim_object_graph_type)
        list = (std::vector<CSceneObject *> *)&_graphList;
    if (t == sim_object_light_type)
        list = (std::vector<CSceneObject *> *)&_lightList;
    if (t == sim_object_camera_type)
        list = (std::vector<CSceneObject *> *)&_cameraList;
    if (t == sim_object_proximitysensor_type)
        list = (std::vector<CSceneObject *> *)&_proximitySensorList;
    if (t == sim_object_visionsensor_type)
        list = (std::vector<CSceneObject *> *)&_visionSensorList;
    if (t == sim_object_shape_type)
        list = (std::vector<CSceneObject *> *)&_shapeList;
    if (t == sim_object_forcesensor_type)
        list = (std::vector<CSceneObject *> *)&_forceSensorList;
    if (t == sim_object_octree_type)
        list = (std::vector<CSceneObject *> *)&_octreeList;
    if (t == sim_object_pointcloud_type)
        list = (std::vector<CSceneObject *> *)&_pointCloudList;
    if (t == sim_object_mirror_type)
        list = (std::vector<CSceneObject *> *)&_mirrorList;
    if (t == sim_object_path_type)
        list = (std::vector<CSceneObject *> *)&_pathList;
    if (t == sim_object_mill_type)
        list = (std::vector<CSceneObject *> *)&_millList;
    for (size_t i = 0; i < list->size(); i++)
    {
        if (list->at(i) == object)
        {
            list->erase(list->begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < _selectedObjectHandles.size(); i++)
    {
        if (_selectedObjectHandles[i] == object->getObjectHandle())
        {
            _selectedObjectHandles.erase(_selectedObjectHandles.begin() + i);
            break;
        }
    }

    _objectHandleMap.erase(object->getObjectHandle());
    _objectNameMap_old.erase(object->getObjectName_old());
    _objectAltNameMap_old.erase(object->getObjectAltName_old());
    delete object;
    _handleOrderIndexOfOrphans();

    actualizeObjectInformation();
#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
    GuiApp::setRebuildHierarchyFlag();
#endif

    _objectDestructionCounter++;
}

void CSceneObjectContainer::buildOrUpdate_oldIk()
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        it->buildOrUpdate_oldIk();
    }
}

void CSceneObjectContainer::connect_oldIk()
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        it->connect_oldIk();
    }
}

void CSceneObjectContainer::remove_oldIk()
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        CSceneObject *it = getObjectFromIndex(i);
        it->remove_oldIk();
    }
}

bool CSceneObjectContainer::doesObjectExist(const CSceneObject *obj) const
{
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        if (obj == _allObjects[i])
            return (true);
    }
    return (false);
}

size_t CSceneObjectContainer::getObjectCount(int type /* = -1 */) const
{
    size_t retVal = 0;
    if (type == -1)
        retVal = _allObjects.size();
    else
    {
        if (type == sim_object_joint_type)
            retVal = _jointList.size();
        if (type == sim_object_dummy_type)
            retVal = _dummyList.size();
        if (type == sim_object_script_type)
            retVal = _scriptList.size();
        if (type == sim_object_mirror_type)
            retVal = _mirrorList.size();
        if (type == sim_object_graph_type)
            retVal = _graphList.size();
        if (type == sim_object_light_type)
            retVal = _lightList.size();
        if (type == sim_object_camera_type)
            retVal = _cameraList.size();
        if (type == sim_object_proximitysensor_type)
            retVal = _proximitySensorList.size();
        if (type == sim_object_visionsensor_type)
            retVal = _visionSensorList.size();
        if (type == sim_object_shape_type)
            retVal = _shapeList.size();
        if (type == sim_object_path_type)
            retVal = _pathList.size();
        if (type == sim_object_mill_type)
            retVal = _millList.size();
        if (type == sim_object_forcesensor_type)
            retVal = _forceSensorList.size();
        if (type == sim_object_octree_type)
            retVal = _octreeList.size();
        if (type == sim_object_pointcloud_type)
            retVal = _pointCloudList.size();
    }
    return retVal;
}

CSceneObject *CSceneObjectContainer::getObjectFromIndex(size_t index) const
{
    CSceneObject *retVal = nullptr;
    if (index < _allObjects.size())
        retVal = _allObjects[index];
    return (retVal);
}

CSceneObject *CSceneObjectContainer::getObjectFromHandle(int objectHandle) const
{
    std::map<int, CSceneObject *>::const_iterator it = _objectHandleMap.find(objectHandle);
    if (it != _objectHandleMap.end())
        return (it->second);
    return (nullptr);
}

CSceneObject *CSceneObjectContainer::getObjectFromUid(long long int objectUid) const
{ // not efficient. For now
    for (size_t i = 0; i < _allObjects.size(); i++)
    {
        if (_allObjects[i]->getObjectUid() == objectUid)
            return (_allObjects[i]);
    }
    return (nullptr);
}

int CSceneObjectContainer::getObjects_hierarchyOrder(std::vector<CSceneObject *> &allObjects)
{
    int retVal = 0;
    for (size_t i = 0; i < _orphanObjects.size(); i++)
        retVal += _orphanObjects[i]->getHierarchyTreeObjects(allObjects);
    return (retVal);
}

CSceneObject *CSceneObjectContainer::getObjectFromName_old(const char *objectName) const
{
    std::map<std::string, CSceneObject *>::const_iterator it = _objectNameMap_old.find(objectName);
    if (it != _objectNameMap_old.end())
        return (it->second);
    return (nullptr);
}

CSceneObject *CSceneObjectContainer::getObjectFromAltName_old(const char *objectAltName) const
{
    std::map<std::string, CSceneObject *>::const_iterator it = _objectAltNameMap_old.find(objectAltName);
    if (it != _objectAltNameMap_old.end())
        return (it->second);
    return (nullptr);
}

int CSceneObjectContainer::getObjectHandleFromName_old(const char *objectName) const
{
    int retVal = -1;
    CSceneObject *obj = getObjectFromName_old(objectName);
    if (obj != nullptr)
        retVal = obj->getObjectHandle();
    return (retVal);
}

CSceneObject *CSceneObjectContainer::getOrphanFromIndex(size_t index) const
{
    CSceneObject *retVal = nullptr;
    if (index < _orphanObjects.size())
        retVal = _orphanObjects[index];
    return (retVal);
}

CJoint *CSceneObjectContainer::getJointFromIndex(size_t index) const
{
    CJoint *retVal = nullptr;
    if (index < _jointList.size())
        retVal = _jointList[index];
    return (retVal);
}

CDummy *CSceneObjectContainer::getDummyFromIndex(size_t index) const
{
    CDummy *retVal = nullptr;
    if (index < _dummyList.size())
        retVal = _dummyList[index];
    return (retVal);
}

CScript *CSceneObjectContainer::getScriptFromIndex(size_t index) const
{
    CScript *retVal = nullptr;
    if (index < _scriptList.size())
        retVal = _scriptList[index];
    return (retVal);
}

CMirror *CSceneObjectContainer::getMirrorFromIndex(size_t index) const
{
    CMirror *retVal = nullptr;
    if (index < _mirrorList.size())
        retVal = _mirrorList[index];
    return (retVal);
}

CGraph *CSceneObjectContainer::getGraphFromIndex(size_t index) const
{
    CGraph *retVal = nullptr;
    if (index < _graphList.size())
        retVal = _graphList[index];
    return (retVal);
}

CLight *CSceneObjectContainer::getLightFromIndex(size_t index) const
{
    CLight *retVal = nullptr;
    if (index < _lightList.size())
        retVal = _lightList[index];
    return (retVal);
}

CCamera *CSceneObjectContainer::getCameraFromIndex(size_t index) const
{
    CCamera *retVal = nullptr;
    if (index < _cameraList.size())
        retVal = _cameraList[index];
    return (retVal);
}

CProxSensor *CSceneObjectContainer::getProximitySensorFromIndex(size_t index) const
{
    CProxSensor *retVal = nullptr;
    if (index < _proximitySensorList.size())
        retVal = _proximitySensorList[index];
    return (retVal);
}

CVisionSensor *CSceneObjectContainer::getVisionSensorFromIndex(size_t index) const
{
    CVisionSensor *retVal = nullptr;
    if (index < _visionSensorList.size())
        retVal = _visionSensorList[index];
    return (retVal);
}

CShape *CSceneObjectContainer::getShapeFromIndex(size_t index) const
{
    CShape *retVal = nullptr;
    if (index < _shapeList.size())
        retVal = _shapeList[index];
    return (retVal);
}

CPath_old *CSceneObjectContainer::getPathFromIndex(size_t index) const
{
    CPath_old *retVal = nullptr;
    if (index < _pathList.size())
        retVal = _pathList[index];
    return (retVal);
}

CMill *CSceneObjectContainer::getMillFromIndex(size_t index) const
{
    CMill *retVal = nullptr;
    if (index < _millList.size())
        retVal = _millList[index];
    return (retVal);
}

CForceSensor *CSceneObjectContainer::getForceSensorFromIndex(size_t index) const
{
    CForceSensor *retVal = nullptr;
    if (index < _forceSensorList.size())
        retVal = _forceSensorList[index];
    return (retVal);
}

COcTree *CSceneObjectContainer::getOctreeFromIndex(size_t index) const
{
    COcTree *retVal = nullptr;
    if (index < _octreeList.size())
        retVal = _octreeList[index];
    return (retVal);
}

CPointCloud *CSceneObjectContainer::getPointCloudFromIndex(size_t index) const
{
    CPointCloud *retVal = nullptr;
    if (index < _pointCloudList.size())
        retVal = _pointCloudList[index];
    return (retVal);
}

CDummy *CSceneObjectContainer::getDummyFromHandle(int objectHandle) const
{
    CDummy *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_dummy_type))
        retVal = (CDummy *)it;
    return (retVal);
}

CScriptObject *CSceneObjectContainer::getScriptObjectFromHandle(int handle) const
{
    CScriptObject *retVal = nullptr;
    if (handle <= SIM_IDEND_SCENEOBJECT)
    { // scene object scripts
        CScript* it = getScriptFromHandle(handle);
        if (it != nullptr)
            retVal = it->scriptObject;
    }
    else
    { // main script (and old non-scene object scripts)
        retVal = embeddedScriptContainer->getScriptObjectFromHandle(handle);
    }
    return (retVal);
}

CScriptObject *CSceneObjectContainer::getScriptObjectFromUid(int uid) const
{
    CScriptObject *retVal = nullptr;
    for (size_t i = 0; i < _scriptList.size(); i++)
    {
        CScript* it = _scriptList[i];
        if (it->scriptObject->getScriptUid() == uid)
        {
            retVal = it->scriptObject;
            break;
        }
    }
    if (retVal == nullptr)
    { // main script (and old non-scene object scripts)
        retVal = embeddedScriptContainer->getScriptObjectFromUid(uid);
    }
    return (retVal);
}

CScript *CSceneObjectContainer::getScriptFromHandle(int objectHandle) const
{
    CScript *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_script_type))
        retVal = (CScript *)it;
    return (retVal);
}

CJoint *CSceneObjectContainer::getJointFromHandle(int objectHandle) const
{
    CJoint *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_joint_type))
        retVal = (CJoint *)it;
    return (retVal);
}

CShape *CSceneObjectContainer::getShapeFromHandle(int objectHandle) const
{
    CShape *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_shape_type))
        retVal = (CShape *)it;
    return (retVal);
}

CMirror *CSceneObjectContainer::getMirrorFromHandle(int objectHandle) const
{
    CMirror *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_mirror_type))
        retVal = (CMirror *)it;
    return (retVal);
}

COcTree *CSceneObjectContainer::getOctreeFromHandle(int objectHandle) const
{
    COcTree *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_octree_type))
        retVal = (COcTree *)it;
    return (retVal);
}

CPointCloud *CSceneObjectContainer::getPointCloudFromHandle(int objectHandle) const
{
    CPointCloud *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_pointcloud_type))
        retVal = (CPointCloud *)it;
    return (retVal);
}

CProxSensor *CSceneObjectContainer::getProximitySensorFromHandle(int objectHandle) const
{
    CProxSensor *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_proximitysensor_type))
        retVal = (CProxSensor *)it;
    return (retVal);
}

CVisionSensor *CSceneObjectContainer::getVisionSensorFromHandle(int objectHandle) const
{
    CVisionSensor *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_visionsensor_type))
        retVal = (CVisionSensor *)it;
    return (retVal);
}

CPath_old *CSceneObjectContainer::getPathFromHandle(int objectHandle) const
{
    CPath_old *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_path_type))
        retVal = (CPath_old *)it;
    return (retVal);
}

CMill *CSceneObjectContainer::getMillFromHandle(int objectHandle) const
{
    CMill *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_mill_type))
        retVal = (CMill *)it;
    return (retVal);
}

CForceSensor *CSceneObjectContainer::getForceSensorFromHandle(int objectHandle) const
{
    CForceSensor *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_forcesensor_type))
        retVal = (CForceSensor *)it;
    return (retVal);
}

CCamera *CSceneObjectContainer::getCameraFromHandle(int objectHandle) const
{
    CCamera *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_camera_type))
        retVal = (CCamera *)it;
    return (retVal);
}

CLight *CSceneObjectContainer::getLightFromHandle(int objectHandle) const
{
    CLight *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_light_type))
        retVal = (CLight *)it;
    return (retVal);
}

CGraph *CSceneObjectContainer::getGraphFromHandle(int objectHandle) const
{
    CGraph *retVal = nullptr;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if ((it != nullptr) && (it->getObjectType() == sim_object_graph_type))
        retVal = (CGraph *)it;
    return (retVal);
}

size_t CSceneObjectContainer::getObjectCountInSelection(int objectType /*=-1*/, const std::vector<int> *selection /*=nullptr*/) const
{
    size_t counter = 0;
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    for (size_t i = 0; i < sel->size(); i++)
    {
        CSceneObject *it = getObjectFromHandle(sel->at(i));
        if ( (it != nullptr) && ( (objectType == -1) || (it->getObjectType() == objectType) ) )
            counter++;
    }
    return (counter);
}

size_t CSceneObjectContainer::getSimpleShapeCountInSelection(const std::vector<int> *selection /*=nullptr*/) const
{
    size_t counter = 0;
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    for (size_t i = 0; i < sel->size(); i++)
    {
        CShape *it = getShapeFromHandle(sel->at(i));
        if (it != nullptr)
        {
            if (!it->isCompound())
                counter++;
        }
    }
    return (counter);
}

bool CSceneObjectContainer::isLastSelectionOfType(int objectType, const std::vector<int> *selection /*=nullptr*/) const
{
    size_t counter = 0;
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    if (sel->size() == 0)
        return (false);
    CSceneObject *it = getObjectFromHandle(sel->at(sel->size() - 1));
    if ( (it != nullptr) && (it->getObjectType() == objectType) )
        return (true);
    return (false);
}

bool CSceneObjectContainer::isLastSelectionASimpleShape(const std::vector<int> *selection /*=nullptr*/) const
{
    size_t counter = 0;
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    if (sel->size() == 0)
        return (false);
    CShape *it = getShapeFromHandle(sel->at(sel->size() - 1));
    if (it != nullptr)
    {
        if (!it->isCompound())
            return (true);
    }
    return (false);
}

CSceneObject *CSceneObjectContainer::getLastSelectionObject(const std::vector<int> *selection /*=nullptr*/) const
{
    CSceneObject *retVal = nullptr;
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    if (sel->size() != 0)
        retVal = getObjectFromHandle(sel->at(sel->size() - 1));
    return (retVal);
}

CMirror *CSceneObjectContainer::getLastSelectionMirror() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_mirror_type)
            return ((CMirror *)it);
    }
    return (nullptr);
}

COcTree *CSceneObjectContainer::getLastSelectionOctree() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_octree_type)
            return ((COcTree *)it);
    }
    return (nullptr);
}

CPointCloud *CSceneObjectContainer::getLastSelectionPointCloud() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_pointcloud_type)
            return ((CPointCloud *)it);
    }
    return (nullptr);
}

CShape *CSceneObjectContainer::getLastSelectionShape() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_shape_type)
            return ((CShape *)it);
    }
    return (nullptr);
}

CJoint *CSceneObjectContainer::getLastSelectionJoint() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_joint_type)
            return ((CJoint *)it);
    }
    return (nullptr);
}

CGraph *CSceneObjectContainer::getLastSelectionGraph() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_graph_type)
            return ((CGraph *)it);
    }
    return (nullptr);
}

CCamera *CSceneObjectContainer::getLastSelectionCamera() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_camera_type)
            return ((CCamera *)it);
    }
    return (nullptr);
}

CLight *CSceneObjectContainer::getLastSelectionLight() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_light_type)
            return ((CLight *)it);
    }
    return (nullptr);
}

CDummy *CSceneObjectContainer::getLastSelectionDummy() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_dummy_type)
            return ((CDummy *)it);
    }
    return (nullptr);
}

CScript *CSceneObjectContainer::getLastSelectionScript() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_script_type)
            return ((CScript *)it);
    }
    return (nullptr);
}

CProxSensor *CSceneObjectContainer::getLastSelectionProxSensor() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_proximitysensor_type)
            return ((CProxSensor *)it);
    }
    return (nullptr);
}

CVisionSensor *CSceneObjectContainer::getLastSelectionVisionSensor() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_visionsensor_type)
            return ((CVisionSensor *)it);
    }
    return (nullptr);
}

CPath_old *CSceneObjectContainer::getLastSelectionPath() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_path_type)
            return ((CPath_old *)it);
    }
    return (nullptr);
}

CMill *CSceneObjectContainer::getLastSelectionMill() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_mill_type)
            return ((CMill *)it);
    }
    return (nullptr);
}

CForceSensor *CSceneObjectContainer::getLastSelectionForceSensor() const
{
    CSceneObject *it = getLastSelectionObject();
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_object_forcesensor_type)
            return ((CForceSensor *)it);
    }
    return (nullptr);
}

bool CSceneObjectContainer::hasSelectionChanged()
{
    bool retVal = (_lastSelection != _selectedObjectHandles);
    _lastSelection.assign(_selectedObjectHandles.begin(), _selectedObjectHandles.end());
    return (retVal);
}

bool CSceneObjectContainer::isObjectSelected(int objectHandle) const
{
    bool retVal = false;
    CSceneObject *it = getObjectFromHandle(objectHandle);
    if (it != nullptr)
        retVal = it->getSelected();
    return (retVal);
}

void CSceneObjectContainer::getSelectedObjectHandles(std::vector<int> &selection, int objectType /*=-1*/,
                                                     bool includeModelObjects /*=false*/,
                                                     bool onlyVisibleModelObjects /*=false*/) const
{
    std::vector<CSceneObject *> sel;
    getSelectedObjects(sel, objectType, includeModelObjects, onlyVisibleModelObjects);
    selection.clear();
    for (size_t i = 0; i < sel.size(); i++)
        selection.push_back(sel[i]->getObjectHandle());
}

void CSceneObjectContainer::getSelectedObjects(std::vector<CSceneObject *> &selection, int objectType /*=-1*/,
                                               bool includeModelObjects /*=false*/,
                                               bool onlyVisibleModelObjects /*=false*/) const
{
    std::unordered_set<int> objectsInInputList;
    std::unordered_set<int> objectsInOutputList;
    selection.clear();
    const std::vector<int> *smallSel = getSelectedObjectHandlesPtr();

    std::vector<CSceneObject *> models;
    for (size_t i = 0; i < smallSel->size(); i++)
    {
        int h = smallSel->at(i);
        CSceneObject *it = getObjectFromHandle(smallSel->at(i));
        objectsInInputList.insert(h);
        if (includeModelObjects)
        { // We split the task in 2: first non-model objects, later we add model objects.
            if (!it->getModelBase())
            {
                if ((objectType == -1) || (objectType == it->getObjectType()))
                {
                    selection.push_back(it);
                    objectsInOutputList.insert(h);
                }
            }
            else
                models.push_back(it);
        }
        else
        { // only objects of the desired type
            if ((objectType == -1) || (objectType == it->getObjectType()))
            {
                selection.push_back(it);
                objectsInOutputList.insert(h);
            }
        }
    }

    if (includeModelObjects)
    {
        for (size_t i = 0; i < models.size(); i++)
        {
            CSceneObject *it = models[i];
            std::vector<CSceneObject *> newObjs;
            it->getAllObjectsRecursive(&newObjs, false, true);
            size_t inSelCnt = 0;
            for (size_t j = 0; j < newObjs.size(); j++)
            {
                CSceneObject *nit = newObjs[j];
                if (objectsInInputList.find(nit->getObjectHandle()) != objectsInInputList.end())
                    inSelCnt++; // that model child was anyways selected
                if ((objectType == -1) || (objectType == nit->getObjectType()))
                {
                    if (objectsInOutputList.find(nit->getObjectHandle()) == objectsInOutputList.end())
                    {
                        if ((!onlyVisibleModelObjects) ||
                            ((!nit->isObjectPartOfInvisibleModel()) &&
                             (App::currentWorld->environment->getActiveLayers() & nit->getVisibilityLayer())))
                        {
                            objectsInOutputList.insert(nit->getObjectHandle());
                            selection.insert(selection.begin(),
                                             nit); // to the front, to preserve somewhat the selection order
                        }
                    }
                }
            }
            // Now handle the model object itself. It is added, if correct type and visible, and not yet present.
            // If it is not visible, then we only add it, if all of its children were anyway selected initially:
            bool allModelChildrenInitiallySelected = (newObjs.size() == inSelCnt);
            if ((objectType == -1) || (objectType == it->getObjectType()))
            {
                if (objectsInOutputList.find(it->getObjectHandle()) == objectsInOutputList.end())
                {
                    if (allModelChildrenInitiallySelected || (!onlyVisibleModelObjects) ||
                        ((!it->isObjectPartOfInvisibleModel()) &&
                         (App::currentWorld->environment->getActiveLayers() & it->getVisibilityLayer())))
                    {
                        objectsInOutputList.insert(it->getObjectHandle());
                        selection.insert(selection.begin(),
                                         it); // to the front, to preserve somewhat the selection order
                    }
                }
            }
        }
    }
}

int CSceneObjectContainer::getLastSelectionHandle(const std::vector<int> *selection /*=nullptr*/) const
{
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    if (sel->size() == 0)
        return (-1);
    return ((*sel)[(sel->size() - 1)]);
}

bool CSceneObjectContainer::isObjectInSelection(int objectHandle, const std::vector<int> *selection /*=nullptr*/) const
{
    const std::vector<int> *sel = &_selectedObjectHandles;
    if (selection != nullptr)
        sel = selection;
    for (size_t i = 0; i < sel->size(); i++)
    {
        if (sel->at(i) == objectHandle)
            return (true);
    }
    return (false);
}

size_t CSceneObjectContainer::getSimpleShapeCount() const
{
    size_t counter = 0;
    for (size_t i = 0; i < _shapeList.size(); i++)
    {
        CShape *it = getShapeFromIndex(i);
        if (!it->isCompound())
            counter++;
    }
    return (counter);
}

size_t CSceneObjectContainer::getCompoundShapeCount() const
{
    return (getObjectCount(sim_object_shape_type) - getSimpleShapeCount());
}

size_t CSceneObjectContainer::getOrphanCount() const
{
    return (_orphanObjects.size());
}

void CSceneObjectContainer::_addToOrphanObjects(CSceneObject *object)
{
    _orphanObjects.push_back(object);
}

void CSceneObjectContainer::_removeFromOrphanObjects(CSceneObject *object)
{
    for (size_t i = 0; i < _orphanObjects.size(); i++)
    {
        if (_orphanObjects[i] == object)
        {
            _orphanObjects.erase(_orphanObjects.begin() + i);
            break;
        }
    }
}

int CSceneObjectContainer::getObjectSequence(const CSceneObject *object, int* totalSiblings /*= nullptr*/) const
{
    CSceneObject *parent = object->getParent();
    if (parent != nullptr)
        return (parent->getChildSequence(object, totalSiblings));
    else
    {
        for (size_t i = 0; i < _orphanObjects.size(); i++)
        {
            if (_orphanObjects[i] == object)
            {
                if (totalSiblings != nullptr)
                    totalSiblings[0] = int(_orphanObjects.size());
                return (int(i));
            }
        }
    }
    if (totalSiblings != nullptr)
        totalSiblings[0] = 0;
    return (-1);
}

const std::vector<int> *CSceneObjectContainer::getSelectedObjectHandlesPtr() const
{
    return (&_selectedObjectHandles);
}

void CSceneObjectContainer::popLastSelection()
{
    std::vector<int> sel(_selectedObjectHandles);
    if (sel.size() > 0)
    {
        sel.pop_back();
        setSelectedObjectHandles(&sel);
    }
}

size_t CSceneObjectContainer::getSelectionCount() const
{
    return (_selectedObjectHandles.size());
}

int CSceneObjectContainer::getObjectHandleFromSelectionIndex(size_t index) const
{
    return (_selectedObjectHandles[index]);
}

CSceneObject *CSceneObjectContainer::_getObjectFromComplexPath(const CSceneObject *emittingObject, std::string &path,
                                                               int index) const
{ // for e.g. "/objectA{i}/objectB{j}/objectC{k}", returns objectA{i}, and path "./objectB{j}/objectC{k}"
    size_t p1 = path.find("{");
    size_t p2 = path.find("}");
    if ((p1 != std::string::npos) && (p2 != std::string::npos) && (p2 > p1 + 1))
    {
        if ((p2 != path.size() - 1) || (index == -1))
        {
            std::string nb(path.begin() + p1 + 1, path.begin() + p2);
            int iv;
            if (tt::getValidInt(nb.c_str(), iv))
            {
                std::string pa(path.begin(), path.begin() + p1);
                path.erase(path.begin(), path.begin() + p2 + 1);
                if (path.size() > 0)
                    path = "." + path;
                return (_getObjectFromSimplePath(emittingObject, pa.c_str(), iv));
            }
        }
    }
    std::string ppath(path);
    path.clear();
    return (_getObjectFromSimplePath(emittingObject, ppath.c_str(), index));
}

CSceneObject *CSceneObjectContainer::_getObjectFromSimplePath(const CSceneObject *emittingObject,
                                                              const char *objectAliasAndPath, int index) const
{
    std::string nm(objectAliasAndPath);
    CSceneObject *retVal = nullptr;
    if ((nm.size() > 0) && ((nm[0] == '/') || (nm[0] == '.') || (nm[0] == ':')))
    {
        const CSceneObject *emObj = nullptr;
        if (nm[0] == '/')
        {
            nm.erase(0, 1);
            if ((nm[0] == '.') || (nm[0] == ':') || (nm[0] == '/'))
                return (nullptr);
        }
        else
        {
            emObj = emittingObject;
            if ((nm == ":") || (nm == "."))
            {
                if (nm == ":")
                {
                    while ((emObj != nullptr) && (!emObj->getModelBase()))
                        emObj = emObj->getParent();
                }
                return ((CSceneObject *)emObj);
            }
            else
            {
                if (nm.compare(0, 2, "./") == 0)
                    nm.erase(0, 2);
                if (nm.compare(0, 2, ":/") == 0)
                {
                    nm.erase(0, 2);
                    while ((emObj != nullptr) && (!emObj->getModelBase()))
                        emObj = emObj->getParent();
                }
                while ((nm.compare(0, 2, "::") == 0) || (nm.compare(0, 2, "..") == 0))
                {
                    if (emObj == nullptr)
                        return (nullptr);
                    bool model = (nm.compare(0, 2, "::") == 0);
                    nm.erase(0, 2);
                    if (model)
                    { // Get the first parent that is model (including itself):
                        while ((emObj != nullptr) && (!emObj->getModelBase()))
                            emObj = emObj->getParent();
                        if (emObj == nullptr)
                            return (nullptr);
                    }
                    emObj = emObj->getParent();
                    if (model)
                    { // Get the next parent that is model (excluding itself):
                        while ((emObj != nullptr) && (!emObj->getModelBase()))
                            emObj = emObj->getParent();
                    }
                    if (nm.size() == 0)
                        return ((CSceneObject *)emObj); // e.g. "../.."
                    if (nm[0] == '/')
                        nm.erase(0, 1);
                    else
                        return (nullptr); // bad string (expected "../" or "::/")
                }
            }
        }
        retVal = _getObjectInTree(emObj, nm.c_str(), index);
    }
    return (retVal);
}

CSceneObject *CSceneObjectContainer::getObjectFromPath(const CSceneObject *emittingObject,
                                                       const char *objectAliasAndPath, int index) const
{
    std::string path(objectAliasAndPath);
    CSceneObject *retVal = nullptr;
    while (path.size() > 0)
    {
        CSceneObject *it = _getObjectFromComplexPath(emittingObject, path, index);
        if (it != nullptr)
        {
            if (path.size() == 0)
                return (it);
            emittingObject = it;
        }
        else
            break;
    }
    return (nullptr);
}

CSceneObject *CSceneObjectContainer::_getObjectInTree(const CSceneObject *treeBase, const char *objectAliasAndPath,
                                                      int &index) const
{ // recursive. objectAliasAndPath as "objectName/objectName" with optional wildcards and order info, e.g.
  // "objectName[0]/objectName[0]"
    std::vector<CSceneObject *> toExplore;
    if (treeBase == nullptr)
    {
        for (size_t i = 0; i < getOrphanCount(); i++)
            toExplore.push_back(getOrphanFromIndex(i));
    }
    else
    {
        for (size_t i = 0; i < treeBase->getChildCount(); i++)
            toExplore.push_back(treeBase->getChildFromIndex(i));
    }
    std::string fullname(objectAliasAndPath);
    size_t sp = fullname.find("/");
    std::string objName(fullname.substr(0, sp));
    if (sp == std::string::npos)
        fullname.clear();
    else
        fullname = fullname.substr(sp + 1);
    int nameIndex = 0;
    bool usingNameIndex = false;
    size_t ob = objName.find('[');
    if (ob != std::string::npos)
    {
        usingNameIndex = true;
        if (objName[objName.size() - 1] == ']')
        {
            objName.erase(objName.size() - 1, 1);
            std::string nv(objName.substr(ob + 1));
            if (tt::getValidInt(nv.c_str(), nameIndex)) // if nb is not valid, won't be able to match any object name
                objName.erase(ob);
        }
    }
    std::vector<CSceneObject *> nextLevelExploration;
    for (size_t i = 0; i < toExplore.size(); i++)
    {
        CSceneObject *it = toExplore[i];
        std::string name(it->getObjectAlias());
        bool doNextLevelExploration = true;
        if (utils::doStringMatch_wildcard(objName.c_str(), name.c_str()))
        {
            doNextLevelExploration = !usingNameIndex;
            if (nameIndex == 0)
            {
                if (fullname.size() == 0)
                { // we can't explore further
                    if (index > 0)
                        index--;
                    else
                        return (it);
                }
                else
                { // we can explore further:
                    CSceneObject *r = _getObjectInTree(it, fullname.c_str(), index);
                    if (r != nullptr)
                        return (r);
                }
            }
            if (usingNameIndex)
                nameIndex--;
        }
        if (doNextLevelExploration)
            nextLevelExploration.push_back(it);
    }
    for (size_t i = 0; i < nextLevelExploration.size(); i++)
    {
        CSceneObject *r = _getObjectInTree(nextLevelExploration[i], objectAliasAndPath, index);
        if (r != nullptr)
            return (r);
    }
    return (nullptr);
}

void CSceneObjectContainer::resetScriptFlagCalledInThisSimulationStep()
{
    embeddedScriptContainer->resetScriptFlagCalledInThisSimulationStep();
    for (size_t i = 0; i < _scriptList.size(); i++)
        _scriptList[i]->scriptObject->resetCalledInThisSimulationStep();
}

int CSceneObjectContainer::getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts)
{
    int cnt = embeddedScriptContainer->getCalledScriptsCountInThisSimulationStep(onlySimulationScripts);

    for (size_t i = 0; i < _scriptList.size(); i++)
    {
        if (_scriptList[i]->scriptObject->getCalledInThisSimulationStep())
        {
            if ((!onlySimulationScripts) || (_scriptList[i]->scriptObject->getScriptType() == sim_scripttype_childscript))
                cnt++;
        }
    }
    return (cnt);
}
