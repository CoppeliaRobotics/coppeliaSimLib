#include "simInternal.h"
#include "copyBuffer.h"
#include "tt.h"
#include "mesh.h"
#include "simStrings.h"
#include "app.h"

CCopyBuffer::CCopyBuffer()
{
    clearBuffer();
    _copyIsForPasting=false;
}

CCopyBuffer::~CCopyBuffer()
{
    clearMemorizedBuffer();
    clearBuffer();
}

bool CCopyBuffer::isCopyForPasting()
{
    return(_copyIsForPasting);
}

void CCopyBuffer::clearBuffer()
{
    TRACE_INTERNAL;
    // Here we delete all objects contained in the buffers. We don't
    // have to (and we shouldn't!!) call 'announceObjectWillBeErased'
    // since all objects are erased at the same time!
    for (size_t i=0;i<objectBuffer.size();i++)
        delete objectBuffer[i];
    objectBuffer.clear();
    for (size_t i=0;i<luaScriptBuffer.size();i++)
        delete luaScriptBuffer[i];
    luaScriptBuffer.clear();
    for (size_t i=0;i<textureObjectBuffer.size();i++)
        delete textureObjectBuffer[i];
    textureObjectBuffer.clear();

    // Old:
    for (size_t i=0;i<collectionBuffer.size();i++)
        delete collectionBuffer[i];
    collectionBuffer.clear();
    for (size_t i=0;i<collisionBuffer.size();i++)
        delete collisionBuffer[i];
    collisionBuffer.clear();
    for (size_t i=0;i<distanceBuffer.size();i++)
        delete distanceBuffer[i];
    distanceBuffer.clear();
    for (size_t i=0;i<ikGroupBuffer.size();i++)
        delete ikGroupBuffer[i];
    ikGroupBuffer.clear();
    for (size_t i=0;i<pathPlanningTaskBuffer.size();i++)
        delete pathPlanningTaskBuffer[i];
    pathPlanningTaskBuffer.clear();
    for (size_t i=0;i<buttonBlockBuffer.size();i++)
        delete buttonBlockBuffer[i];
    buttonBlockBuffer.clear();
}

void CCopyBuffer::clearMemorizedBuffer()
{
    for (size_t i=0;i<objectBuffer_memorized.size();i++)
        delete objectBuffer_memorized[i];
    objectBuffer_memorized.clear();
    for (size_t i=0;i<luaScriptBuffer_memorized.size();i++)
        delete luaScriptBuffer_memorized[i];
    luaScriptBuffer_memorized.clear();
    for (size_t i=0;i<textureObjectBuffer_memorized.size();i++)
        delete textureObjectBuffer_memorized[i];
    textureObjectBuffer_memorized.clear();

    // Old:
    for (size_t i=0;i<collectionBuffer_memorized.size();i++)
        delete collectionBuffer_memorized[i];
    collectionBuffer_memorized.clear();
    for (size_t i=0;i<collisionBuffer_memorized.size();i++)
        delete collisionBuffer_memorized[i];
    collisionBuffer_memorized.clear();
    for (size_t i=0;i<distanceBuffer_memorized.size();i++)
        delete distanceBuffer_memorized[i];
    distanceBuffer_memorized.clear();
    for (size_t i=0;i<ikGroupBuffer_memorized.size();i++)
        delete ikGroupBuffer_memorized[i];
    ikGroupBuffer_memorized.clear();
    for (size_t i=0;i<pathPlanningTaskBuffer_memorized.size();i++)
        delete pathPlanningTaskBuffer_memorized[i];
    pathPlanningTaskBuffer_memorized.clear();
    for (size_t i=0;i<buttonBlockBuffer_memorized.size();i++)
        delete buttonBlockBuffer_memorized[i];
    buttonBlockBuffer_memorized.clear();
}

void CCopyBuffer::memorizeBuffer()
{
    // 1. We delete previously memorized objects:
    clearMemorizedBuffer();

    // 2. we copy all objects to the memorized buffer:
    for (size_t i=0;i<objectBuffer.size();i++)
        objectBuffer_memorized.push_back(objectBuffer[i]->copyYourself());
    for (size_t i=0;i<luaScriptBuffer.size();i++)
        luaScriptBuffer_memorized.push_back(luaScriptBuffer[i]->copyYourself());
    for (size_t i=0;i<textureObjectBuffer.size();i++)
        textureObjectBuffer_memorized.push_back(textureObjectBuffer[i]->copyYourself());

    // Old:
    for (size_t i=0;i<collectionBuffer.size();i++)
        collectionBuffer_memorized.push_back(collectionBuffer[i]->copyYourself());
    for (size_t i=0;i<collisionBuffer.size();i++)
        collisionBuffer_memorized.push_back(collisionBuffer[i]->copyYourself());
    for (size_t i=0;i<distanceBuffer.size();i++)
        distanceBuffer_memorized.push_back(distanceBuffer[i]->copyYourself());
    for (size_t i=0;i<ikGroupBuffer.size();i++)
        ikGroupBuffer_memorized.push_back(ikGroupBuffer[i]->copyYourself());
    for (size_t i=0;i<pathPlanningTaskBuffer.size();i++)
        pathPlanningTaskBuffer_memorized.push_back(pathPlanningTaskBuffer[i]->copyYourself());
    for (size_t i=0;i<buttonBlockBuffer.size();i++)
        buttonBlockBuffer_memorized.push_back(buttonBlockBuffer[i]->copyYourself());

    _bufferIsFromLockedScene_memorized=_bufferIsFromLockedScene;
}

void CCopyBuffer::restoreBuffer()
{
    // 1. We clear the buffer:
    clearBuffer();
    // 2. We copy all memorized objects to the buffer:
    for (size_t i=0;i<objectBuffer_memorized.size();i++)
        objectBuffer.push_back(objectBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<luaScriptBuffer_memorized.size();i++)
        luaScriptBuffer.push_back(luaScriptBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<textureObjectBuffer_memorized.size();i++)
        textureObjectBuffer.push_back(textureObjectBuffer_memorized[i]->copyYourself());

    // Old:
    for (size_t i=0;i<collectionBuffer_memorized.size();i++)
        collectionBuffer.push_back(collectionBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<collisionBuffer_memorized.size();i++)
        collisionBuffer.push_back(collisionBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<distanceBuffer_memorized.size();i++)
        distanceBuffer.push_back(distanceBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<ikGroupBuffer_memorized.size();i++)
        ikGroupBuffer.push_back(ikGroupBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<pathPlanningTaskBuffer_memorized.size();i++)
        pathPlanningTaskBuffer.push_back(pathPlanningTaskBuffer_memorized[i]->copyYourself());
    for (size_t i=0;i<buttonBlockBuffer_memorized.size();i++)
        buttonBlockBuffer.push_back(buttonBlockBuffer_memorized[i]->copyYourself());

    _bufferIsFromLockedScene=_bufferIsFromLockedScene_memorized;
}

int CCopyBuffer::pasteBuffer(bool intoLockedScene,int selectionMode)
{ // return -1 means the operation cannot procceed because the scene is not locked (but buffer is), 0=empty buffer, 1=successful
    // This function is very similar to a model-loading operation:
    // Everything is inserted (sceneObjects, collections, etc. ) and then
    // the mapping is performed

    // selectionMode: 0=clearSelection, 1=selectAllPasted, 2=selectOnlyModels, 3=selectOnlyModelsAndOrphans
    TRACE_INTERNAL;
    if (isBufferEmpty())
        return(0);
    if (_bufferIsFromLockedScene)
    {
        if (!intoLockedScene)
            return(-1);
    }
//-------------------------------- Buffer copy -----------------------------------------    
    _copyIsForPasting=true;
    // First we need to copy the copy-buffers:
    std::vector<CSceneObject*> objectCopy;
    for (size_t i=0;i<objectBuffer.size();i++)
        objectCopy.push_back(objectBuffer[i]->copyYourself());

    std::vector<CLuaScriptObject*> luaScriptCopy;
    for (size_t i=0;i<luaScriptBuffer.size();i++)
        luaScriptCopy.push_back(luaScriptBuffer[i]->copyYourself());

    std::vector<CTextureObject*> textureObjectCopy;
    for (size_t i=0;i<textureObjectBuffer.size();i++)
        textureObjectCopy.push_back(textureObjectBuffer[i]->copyYourself());


    // Old:
    std::vector<CCollection*> collectionCopy;
    for (size_t i=0;i<collectionBuffer.size();i++)
        collectionCopy.push_back(collectionBuffer[i]->copyYourself());

    std::vector<CCollisionObject*> collisionCopy;
    for (size_t i=0;i<collisionBuffer.size();i++)
        collisionCopy.push_back(collisionBuffer[i]->copyYourself());

    std::vector<CDistanceObject*> distanceCopy;
    for (size_t i=0;i<distanceBuffer.size();i++)
        distanceCopy.push_back(distanceBuffer[i]->copyYourself());

    std::vector<CIkGroup*> ikGroupCopy;
    for (size_t i=0;i<ikGroupBuffer.size();i++)
        ikGroupCopy.push_back(ikGroupBuffer[i]->copyYourself());

    std::vector<CPathPlanningTask*> pathPlanningTaskCopy;
    for (size_t i=0;i<pathPlanningTaskBuffer.size();i++)
        pathPlanningTaskCopy.push_back(pathPlanningTaskBuffer[i]->copyYourself());

    std::vector<CButtonBlock*> buttonBlockCopy;
    for (size_t i=0;i<buttonBlockBuffer.size();i++)
        buttonBlockCopy.push_back(buttonBlockBuffer[i]->copyYourself());

    _copyIsForPasting=false;
    
//--------------------------------------------------------------------------------------

    // Following for backward compatibility (leave empty):
    std::vector<CDynMaterialObject*> dynMaterialObjectCopy;

    // And we add everything to the scene:
    App::currentWorld->addGeneralObjectsToWorldAndPerformMappings(&objectCopy,
                                                    &collectionCopy,
                                                    &collisionCopy,
                                                    &distanceCopy,
                                                    &ikGroupCopy,
                                                    &pathPlanningTaskCopy,
                                                    &buttonBlockCopy,
                                                    &luaScriptCopy,
                                                    textureObjectCopy,
                                                    dynMaterialObjectCopy,
                                                    true,SIM_PROGRAM_VERSION_NB,false);

    if (selectionMode==0)
        App::currentWorld->sceneObjects->deselectObjects();
    if ((selectionMode==2)||(selectionMode==3))
        App::currentWorld->sceneObjects->removeFromSelectionAllExceptModelBase(selectionMode==3);

    CInterfaceStack stack;
    stack.pushTableOntoStack();
    stack.pushStringOntoStack("objectHandles",0);
    stack.pushTableOntoStack();
    for (size_t i=0;i<objectCopy.size();i++)
    {
        stack.pushNumberOntoStack(double(i+1)); // key or index
        stack.pushNumberOntoStack(objectCopy[i]->getObjectHandle());
        stack.insertDataIntoStackTable();
    }
    stack.insertDataIntoStackTable();
    App::worldContainer->callScripts(sim_syscb_aftercreate,&stack);

    return(1);
}

bool CCopyBuffer::isBufferEmpty()
{
    return(objectBuffer.size()==0);
}

void CCopyBuffer::copyCurrentSelection(std::vector<int>* sel,bool fromLockedScene)
{   
    TRACE_INTERNAL;
    // We copy the current selection in a way that the copied data (sceneObjects,
    // collections, collisions, etc.) is self-consistent: Should the entire scene be
    // cleared, then the buffer could be inserted into the scene without any
    // modification (except for different object handles).

    if (sel->size()==0)
        return;

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif
    clearBuffer();
    _bufferIsFromLockedScene=fromLockedScene;
    std::vector<CSceneObject*> selObj;
    selObj.reserve(sel->size());
    selObj.clear();
    for (size_t i=0;i<sel->size();i++)
        selObj.push_back(App::currentWorld->sceneObjects->getObjectFromHandle(sel->at(i)));
    objectBuffer.reserve(selObj.size());
    objectBuffer.clear();

    CInterfaceStack stack;
    stack.pushTableOntoStack();
    stack.pushStringOntoStack("objectHandles",0);
    stack.pushTableOntoStack();
    for (size_t i=0;i<sel->size();i++)
    {
        stack.pushNumberOntoStack(double(sel->at(i))); // key or index
        stack.pushBoolOntoStack(true);
        stack.insertDataIntoStackTable();
    }
    stack.insertDataIntoStackTable();

    App::worldContainer->callScripts(sim_syscb_beforecopy,&stack);

    for (size_t i=0;i<selObj.size();i++)
    {
        CSceneObject* original=selObj[i];
        CSceneObject* it=original->copyYourself();
        objectBuffer.push_back(it);
    }

    App::worldContainer->callScripts(sim_syscb_aftercopy,&stack);
    // sceneObjects are copied. We need to prepare the parenting info:
    for (size_t i=0;i<selObj.size();i++)
    {
        CSceneObject* original=selObj[i];
        CSceneObject* originalParent=original->getParent();
        CSceneObject* newParent=original->getFirstParentInSelection(&selObj);
        if (originalParent!=newParent)
        { // We have to change position/orientation
            C7Vector cumul(original->getCumulativeTransformation());
            C7Vector nParentInv;
            if (newParent==nullptr)
                nParentInv.setIdentity();
            else
                nParentInv=newParent->getFullCumulativeTransformation().getInverse();
            objectBuffer[i]->setLocalTransformation(nParentInv*cumul);
        }
        // Now we prepare the index of the new parent (used later)
        if (newParent!=nullptr)
        {
            bool found=false;
            for (size_t j=0;j<selObj.size();j++)
            {
                if (selObj[j]->getObjectHandle()==newParent->getObjectHandle())
                {
                    objectBuffer[i]->setParentHandle_forSerializationOnly(objectBuffer[j]->getObjectHandle());
                    // The following is important for model-serialization!!!
                    objectBuffer[i]->setParent(objectBuffer[j],false);
                    found=true;
                    break;
                }
            }
        }
        else
            objectBuffer[i]->setParentHandle_forSerializationOnly(-1);
    }

    // Other object copy:
    for (size_t i=0;i<App::currentWorld->embeddedScriptContainer->allScripts.size();i++)
    { // Copy only child scripts or customization scripts:
        int st=App::currentWorld->embeddedScriptContainer->allScripts[i]->getScriptType();
        if ( ( (st==sim_scripttype_childscript)||(st==sim_scripttype_customizationscript) )&&(App::currentWorld->embeddedScriptContainer->allScripts[i]->getObjectHandleThatScriptIsAttachedTo()!=-1) )
            luaScriptBuffer.push_back(App::currentWorld->embeddedScriptContainer->allScripts[i]->copyYourself());
    }
    for (size_t i=0;i<App::currentWorld->textureContainer->_allTextureObjects.size();i++)
        textureObjectBuffer.push_back(App::currentWorld->textureContainer->_allTextureObjects[i]->copyYourself());

    // Old:
    for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
    { // copy old the old collections (i.e. those that were created via the GUI):
        CCollection* coll=App::currentWorld->collections->getObjectFromIndex(i);
        if (coll->getCreatorHandle()==-2)
            collectionBuffer.push_back(coll->copyYourself());
    }
    for (size_t i=0;i<App::currentWorld->collisions->getObjectCount();i++)
        collisionBuffer.push_back(App::currentWorld->collisions->getObjectFromIndex(i)->copyYourself());
    for (size_t i=0;i<App::currentWorld->distances->getObjectCount();i++)
        distanceBuffer.push_back(App::currentWorld->distances->getObjectFromIndex(i)->copyYourself());
    for (size_t i=0;i<App::currentWorld->ikGroups->getObjectCount();i++)
        ikGroupBuffer.push_back(App::currentWorld->ikGroups->getObjectFromIndex(i)->copyYourself());
    for (size_t i=0;i<App::currentWorld->pathPlanning->allObjects.size();i++)
        pathPlanningTaskBuffer.push_back(App::currentWorld->pathPlanning->allObjects[i]->copyYourself());
    for (size_t i=0;i<App::currentWorld->buttonBlockContainer->allBlocks.size();i++)
    {
        if ( ((App::currentWorld->buttonBlockContainer->allBlocks[i]->getAttributes()&sim_ui_property_systemblock)==0)&&(App::currentWorld->buttonBlockContainer->allBlocks[i]->getObjectIDAttachedTo()!=-1) )
            buttonBlockBuffer.push_back(App::currentWorld->buttonBlockContainer->allBlocks[i]->copyYourself());
    }

    std::vector<int> unselected;
    for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
    {
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromIndex(i);
        if (!App::currentWorld->sceneObjects->isObjectInSelection(obj->getObjectHandle(),sel))
            unselected.push_back(obj->getObjectHandle());
    }

    // Now we make sure the linked info is consistent: we announce to the selected objects
    // (in the copy buffer) that all unselected objects will be erased:
    // This will in turn also erase general objects (which might trigger other object erase)
    for (size_t i=0;i<unselected.size();i++)
        _announceObjectWillBeErased(unselected[i]);
}

void CCopyBuffer::serializeCurrentSelection(CSer &ar,std::vector<int>* sel,C7Vector& modelTr,C3Vector& modelBBSize,float modelNonDefaultTranslationStepSize)
{
    // This is used when saving a model. When saving a model, we basically perform
    // the same operations as for copying a selection. Since we will make use of the
    // copy buffers, we have to save them first (in case they are not empty) in
    // temporary buffers:
    _backupBuffers_temp();

    copyCurrentSelection(sel,false); // here we indicate that the scene is not locked,it doesn't matter (should have been checked before anyway)

//--------------------------- Here we serialize the buffer content -------------------

    // **** Following needed to save existing calculation structures:
    App::currentWorld->environment->setSaveExistingCalculationStructuresTemp(false);
    if (App::currentWorld->environment->getSaveExistingCalculationStructures())
    {
    // removed on 10/9/2014 App::currentWorld->environment->setSaveExistingCalculationStructures(false); // we clear that flag
        App::currentWorld->environment->setSaveExistingCalculationStructuresTemp(true);
    }
    // ************************************************************

    CMesh::clearTempVerticesIndicesNormalsAndEdges();


    //***************************************************

    //------------------------------------------------------------
    if (ar.isBinary())
    {
        ar.storeDataName(SER_MODEL_THUMBNAIL_INFO);
        ar.setCountingMode();
        App::currentWorld->environment->modelThumbnail_notSerializedHere.serializeAdditionalModelInfos(ar,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
        if (ar.setWritingMode())
            App::currentWorld->environment->modelThumbnail_notSerializedHere.serializeAdditionalModelInfos(ar,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
    }
    else
    {
        ar.xmlPushNewNode(SERX_MODEL_THUMBNAIL_INFO);
        App::currentWorld->environment->modelThumbnail_notSerializedHere.serializeAdditionalModelInfos(ar,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
        ar.xmlPopNode();
    }
    //------------------------------------------------------------

    if (ar.isBinary())
    {
        ar.storeDataName(SER_MODEL_THUMBNAIL);
        ar.setCountingMode();
        App::currentWorld->environment->modelThumbnail_notSerializedHere.serialize(ar,false);
        if (ar.setWritingMode())
            App::currentWorld->environment->modelThumbnail_notSerializedHere.serialize(ar,false);
    }
    else
    {
        ar.xmlPushNewNode(SERX_MODEL_THUMBNAIL);
        App::currentWorld->environment->modelThumbnail_notSerializedHere.serialize(ar,false);
        ar.xmlPopNode();
    }

    // DynMaterial objects:
    // We only save this for backward compatibility, but not needed for CoppeliaSim's from 3.4.0 on:
    //------------------------------------------------------------
    if (ar.isBinary())
    {
        int dynObjId=SIM_IDSTART_DYNMATERIAL_OLD;
        for (size_t i=0;i<objectBuffer.size();i++)
        {
            if (objectBuffer[i]->getObjectType()==sim_object_shape_type)
            {
                CShape* it=(CShape*)objectBuffer[i];
                CDynMaterialObject* mat=it->getDynMaterial();
                it->getMeshWrapper()->setDynMaterialId_OLD(dynObjId);
                mat->setObjectID(dynObjId++);
                ar.storeDataName(SER_DYNMATERIAL);
                ar.setCountingMode();
                mat->serialize(ar);
                if (ar.setWritingMode())
                    mat->serialize(ar);
            }
        }
    }
    //------------------------------------------------------------

    // We store texture objects:
    for (size_t i=0;i<textureObjectBuffer.size();i++)
    {
        if (ar.isBinary())
            App::currentWorld->textureContainer->storeTextureObject(ar,textureObjectBuffer[i]);
        else
        {
            ar.xmlPushNewNode(SERX_TEXTURE);
            App::currentWorld->textureContainer->storeTextureObject(ar,textureObjectBuffer[i]);
            ar.xmlPopNode();
        }
    }

    // Now we store all vertices, indices, normals and edges (there might be duplicates, and we don't wanna waste disk space)
    if (ar.isBinary())
    {
        CMesh::clearTempVerticesIndicesNormalsAndEdges();
        for (size_t i=0;i<objectBuffer.size();i++)
        {
            if (objectBuffer[i]->getObjectType()==sim_object_shape_type)
                ((CShape*)objectBuffer[i])->prepareVerticesIndicesNormalsAndEdgesForSerialization();
        }
        ar.storeDataName(SER_VERTICESINDICESNORMALSEDGES);
        ar.setCountingMode();
        CMesh::serializeTempVerticesIndicesNormalsAndEdges(ar);
        if (ar.setWritingMode())
            CMesh::serializeTempVerticesIndicesNormalsAndEdges(ar);
        CMesh::clearTempVerticesIndicesNormalsAndEdges();
    }

    // Now we store all sceneObjects:
    for (size_t i=0;i<objectBuffer.size();i++)
    {
        if (ar.isBinary())
            App::currentWorld->sceneObjects->writeSceneObject(ar,objectBuffer[i]);
        else
        {
            ar.xmlPushNewNode(SERX_SCENEOBJECT);
            App::currentWorld->sceneObjects->writeSceneObject(ar,objectBuffer[i]);
            ar.xmlPopNode();
        }
    }

    // Old:
    // Here we store the collections:
    for (size_t i=0;i<collectionBuffer.size();i++)
    {
        if (ar.isBinary())
        {
            ar.storeDataName(SER_COLLECTION);
            ar.setCountingMode();
            collectionBuffer[i]->serialize(ar);
            if (ar.setWritingMode())
                collectionBuffer[i]->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_COLLECTION);
            collectionBuffer[i]->serialize(ar);
            ar.xmlPopNode();
        }
    }
    // Here we store the collision objects:
    for (size_t i=0;i<collisionBuffer.size();i++)
    {
        if (ar.isBinary())
        {
            ar.storeDataName(SER_COLLISION);
            ar.setCountingMode();
            collisionBuffer[i]->serialize(ar);
            if (ar.setWritingMode())
                collisionBuffer[i]->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_COLLISION);
            collisionBuffer[i]->serialize(ar);
            ar.xmlPopNode();
        }
    }
    // Here we store the distance objects:
    for (size_t i=0;i<distanceBuffer.size();i++)
    {
        if (ar.isBinary())
        {
            ar.storeDataName(SER_DISTANCE);
            ar.setCountingMode();
            distanceBuffer[i]->serialize(ar);
            if (ar.setWritingMode())
                distanceBuffer[i]->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_DISTANCE);
            distanceBuffer[i]->serialize(ar);
            ar.xmlPopNode();
        }
    }
    // Here we store the ik objects:
    for (size_t i=0;i<ikGroupBuffer.size();i++)
    {
        if (ar.isBinary())
        {
            ar.storeDataName(SER_IK);
            ar.setCountingMode();
            ikGroupBuffer[i]->serialize(ar);
            if (ar.setWritingMode())
                ikGroupBuffer[i]->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_IK);
            ikGroupBuffer[i]->serialize(ar);
            ar.xmlPopNode();
        }
    }
    if (ar.isBinary())
    { // following for backward compatibility, but not supported anymore:
        // Here we store the path planning objects:
        for (size_t i=0;i<pathPlanningTaskBuffer.size();i++)
        {
            ar.storeDataName(SER_PATH_PLANNING);
            ar.setCountingMode();
            pathPlanningTaskBuffer[i]->serialize(ar);
            if (ar.setWritingMode())
                pathPlanningTaskBuffer[i]->serialize(ar);
        }
        // Here we store the button block objects:
        if (!App::userSettings->disableOpenGlBasedCustomUi)
        {
            for (size_t i=0;i<buttonBlockBuffer.size();i++)
            {
                ar.storeDataName(SER_BUTTON_BLOCK_OLD);
                ar.setCountingMode();
                buttonBlockBuffer[i]->serialize(ar);
                if (ar.setWritingMode())
                    buttonBlockBuffer[i]->serialize(ar);
            }
        }
    }


    // Here we store the script objects:
    for (size_t i=0;i<luaScriptBuffer.size();i++)
    {
        if (ar.isBinary())
        {
            ar.storeDataName(SER_LUA_SCRIPT);
            ar.setCountingMode();
            luaScriptBuffer[i]->serialize(ar);
            if (ar.setWritingMode())
                luaScriptBuffer[i]->serialize(ar);
        }
        else
        {
            ar.xmlPushNewNode(SERX_LUA_SCRIPT);
            luaScriptBuffer[i]->serialize(ar);
            ar.xmlPopNode();
        }
    }

    // Here we have finished:
    if (ar.isBinary())
        ar.storeDataName(SER_END_OF_FILE);

    // We empty the used buffers (not needed anymore, since it was saved):
    clearBuffer();

    // Now we copy the original content of the buffers back:
    _restoreBuffers_temp();

    CMesh::clearTempVerticesIndicesNormalsAndEdges();
}

void CCopyBuffer::_backupBuffers_temp()
{
    objectBuffer_tempSer.assign(objectBuffer.begin(),objectBuffer.end());
    objectBuffer.clear();
    luaScriptBuffer_tempSer.assign(luaScriptBuffer.begin(),luaScriptBuffer.end());
    luaScriptBuffer.clear();
    textureObjectBuffer_tempSer.assign(textureObjectBuffer.begin(),textureObjectBuffer.end());
    textureObjectBuffer.clear();

    // Old:
    collectionBuffer_tempSer.assign(collectionBuffer.begin(),collectionBuffer.end());
    collectionBuffer.clear();
    collisionBuffer_tempSer.assign(collisionBuffer.begin(),collisionBuffer.end());
    collisionBuffer.clear();
    distanceBuffer_tempSer.assign(distanceBuffer.begin(),distanceBuffer.end());
    distanceBuffer.clear();
    ikGroupBuffer_tempSer.assign(ikGroupBuffer.begin(),ikGroupBuffer.end());
    ikGroupBuffer.clear();
    pathPlanningTaskBuffer_tempSer.assign(pathPlanningTaskBuffer.begin(),pathPlanningTaskBuffer.end());
    pathPlanningTaskBuffer.clear();
    buttonBlockBuffer_tempSer.assign(buttonBlockBuffer.begin(),buttonBlockBuffer.end());
    buttonBlockBuffer.clear();
}

void CCopyBuffer::_restoreBuffers_temp()
{
    objectBuffer.assign(objectBuffer_tempSer.begin(),objectBuffer_tempSer.end());
    objectBuffer_tempSer.clear();
    luaScriptBuffer.assign(luaScriptBuffer_tempSer.begin(),luaScriptBuffer_tempSer.end());
    luaScriptBuffer_tempSer.clear();
    textureObjectBuffer.assign(textureObjectBuffer_tempSer.begin(),textureObjectBuffer_tempSer.end());
    textureObjectBuffer_tempSer.clear();

    // Old:
    collectionBuffer.assign(collectionBuffer_tempSer.begin(),collectionBuffer_tempSer.end());
    collectionBuffer_tempSer.clear();
    collisionBuffer.assign(collisionBuffer_tempSer.begin(),collisionBuffer_tempSer.end());
    collisionBuffer_tempSer.clear();
    distanceBuffer.assign(distanceBuffer_tempSer.begin(),distanceBuffer_tempSer.end());
    distanceBuffer_tempSer.clear();
    ikGroupBuffer.assign(ikGroupBuffer_tempSer.begin(),ikGroupBuffer_tempSer.end());
    ikGroupBuffer_tempSer.clear();
    pathPlanningTaskBuffer.assign(pathPlanningTaskBuffer_tempSer.begin(),pathPlanningTaskBuffer_tempSer.end());
    pathPlanningTaskBuffer_tempSer.clear();
    buttonBlockBuffer.assign(buttonBlockBuffer_tempSer.begin(),buttonBlockBuffer_tempSer.end());
    buttonBlockBuffer_tempSer.clear();
}

void CCopyBuffer::_eraseObjectInBuffer(int objectID)
{
    _announceObjectWillBeErased(objectID);
    for (size_t i=0;i<objectBuffer.size();i++)
    {
        if (objectBuffer[i]->getObjectHandle()==objectID)
        {
            delete objectBuffer[i];
            objectBuffer.erase(objectBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_eraseScriptInBuffer(int objectID)
{
    _announceScriptWillBeErased(objectID,false,false);
    for (size_t i=0;i<luaScriptBuffer.size();i++)
    {
        if (luaScriptBuffer[i]->getScriptHandle()==objectID)
        {
            delete luaScriptBuffer[i];
            luaScriptBuffer.erase(luaScriptBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_eraseTextureObjectInBuffer(int objectID)
{
    for (size_t i=0;i<textureObjectBuffer.size();i++)
    {
        if (textureObjectBuffer[i]->getObjectID()==objectID)
        {
            delete textureObjectBuffer[i];
            textureObjectBuffer.erase(textureObjectBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_erase2DElementInBuffer(int objectID)
{ // Old
    _announce2DElementWillBeErased(objectID);
    for (size_t i=0;i<buttonBlockBuffer.size();i++)
    {
        if (buttonBlockBuffer[i]->getBlockID()==objectID)
        {
            delete buttonBlockBuffer[i];
            buttonBlockBuffer.erase(buttonBlockBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_erasePathPlanningTaskInBuffer(int objectID)
{ // Old
    for (size_t i=0;i<pathPlanningTaskBuffer.size();i++)
    {
        if (pathPlanningTaskBuffer[i]->getObjectID()==objectID)
        {
            delete pathPlanningTaskBuffer[i];
            pathPlanningTaskBuffer.erase(pathPlanningTaskBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_eraseCollisionInBuffer(int objectID)
{ // Old
    _announceCollisionWillBeErased(objectID);
    for (size_t i=0;i<collisionBuffer.size();i++)
    {
        if (collisionBuffer[i]->getObjectHandle()==objectID)
        {
            delete collisionBuffer[i];
            collisionBuffer.erase(collisionBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_eraseDistanceInBuffer(int objectID)
{ // Old
    _announceDistanceWillBeErased(objectID);
    for (size_t i=0;i<distanceBuffer.size();i++)
    {
        if (distanceBuffer[i]->getObjectHandle()==objectID)
        {
            delete distanceBuffer[i];
            distanceBuffer.erase(distanceBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_eraseCollectionInBuffer(int objectID)
{ // Old
    _announceCollectionWillBeErased(objectID);
    for (size_t i=0;i<collectionBuffer.size();i++)
    {
        if (collectionBuffer[i]->getCollectionHandle()==objectID)
        {
            delete collectionBuffer[i];
            collectionBuffer.erase(collectionBuffer.begin()+i);
            break;
        }
    }
}

void CCopyBuffer::_eraseIkObjectInBuffer(int objectID)
{ // Old
    _announceIkGroupWillBeErased(objectID);
    for (size_t i=0;i<ikGroupBuffer.size();i++)
    {
        if (ikGroupBuffer[i]->getObjectHandle()==objectID)
        {
            delete ikGroupBuffer[i];
            ikGroupBuffer.erase(ikGroupBuffer.begin()+i);
            break;
        }
    }
}

//------------------ Object destruction announcement -------------------------
void CCopyBuffer::_announceObjectWillBeErased(int objectID)
{
    // First objects that won't trigger any more destructions:
    size_t i=0;
    while (i<objectBuffer.size())
    {
        CSceneObject* it=objectBuffer[i];
        if (it->announceObjectWillBeErased(objectID,true))
        { // We should never enter here since one obj destruction cannot trigger another obj destruction (anymore, no more versatiles!) 
#ifdef SIM_WITH_GUI
            App::uiThread->messageBox_critical(App::mainWindow,IDSNOTR_APPLICATION_ERROR,IDSNOTR_STRANGE_ERROR7,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#else
            App::logMsg(sim_verbosity_errors,"%s",IDSNOTR_STRANGE_ERROR7);
#endif
            _eraseObjectInBuffer(it->getObjectHandle()); 
            i=0; // ordering may have changed!
        }
        else
            i++;
    }

    i=0;
    while (i<luaScriptBuffer.size())
    {
        CLuaScriptObject* it=luaScriptBuffer[i];
        if (it->announceSceneObjectWillBeErased(objectID,true))
        {
            _eraseScriptInBuffer(it->getScriptHandle());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }

    i=0;
    while (i<textureObjectBuffer.size())
    {
        CTextureObject* it=textureObjectBuffer[i];
        if (it->announceGeneralObjectWillBeErased(objectID,-1))
        {
            _eraseTextureObjectInBuffer(it->getObjectID());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }

    // Old:
    i=0;
    while (i<buttonBlockBuffer.size())
    {
        CButtonBlock* it=buttonBlockBuffer[i];
        if (it->announceSceneObjectWillBeErased(objectID,true)||(it->getObjectIDAttachedTo()==-1))
        {
            _erase2DElementInBuffer(it->getBlockID());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }

    i=0;
    while (i<pathPlanningTaskBuffer.size())
    {
        CPathPlanningTask* it=pathPlanningTaskBuffer[i];
        if (it->announceObjectWillBeErased(objectID,true))
        {
            _erasePathPlanningTaskInBuffer(it->getObjectID());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }

    i=0;
    while (i<collisionBuffer.size())
    {
        CCollisionObject* it=collisionBuffer[i];
        if (it->announceObjectWillBeErased(objectID,true))
        {
            _eraseCollisionInBuffer(it->getObjectHandle());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }

    i=0;
    while (i<distanceBuffer.size())
    {
        CDistanceObject* it=distanceBuffer[i];
        if (it->announceObjectWillBeErased(objectID,true))
        {
            _eraseDistanceInBuffer(it->getObjectHandle());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }
    
    i=0;
    while (i<collectionBuffer.size())
    {
        CCollection* it=collectionBuffer[i];
        if (it->announceObjectWillBeErased(objectID,true))
        {
            _eraseCollectionInBuffer(it->getCollectionHandle());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }

    i=0;
    while (i<ikGroupBuffer.size())
    {
        CIkGroup* it=ikGroupBuffer[i];
        if (it->announceObjectWillBeErased(objectID,true))
        {
            _eraseIkObjectInBuffer(it->getObjectHandle());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }
}

void CCopyBuffer::_announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript)
{
    for (size_t i=0;i<objectBuffer.size();i++)
        objectBuffer[i]->announceScriptWillBeErased(scriptHandle,simulationScript,sceneSwitchPersistentScript,true); // this never triggers scene object destruction!
}

void CCopyBuffer::_announceCollectionWillBeErased(int collectionID)
{ // Old
    for (size_t i=0;i<objectBuffer.size();i++)
        objectBuffer[i]->announceCollectionWillBeErased(collectionID,true); // this never triggers scene object destruction!

    size_t i=0;
    while (i<collisionBuffer.size())
    {
        CCollisionObject* it=collisionBuffer[i];
        if (it->announceCollectionWillBeErased(collectionID,true))
        {
            _eraseCollisionInBuffer(it->getObjectHandle());
            i=0; // Ordering may have changed
        }
        else
            i++;
    }

    i=0;
    while (i<distanceBuffer.size())
    {
        CDistanceObject* it=distanceBuffer[i];
        if (it->announceCollectionWillBeErased(collectionID,true))
        {
            _eraseDistanceInBuffer(it->getObjectHandle());
            i=0; // Ordering may have changed
        }
        else
            i++;
    }

    i=0;
    while (i<pathPlanningTaskBuffer.size())
    {
        CPathPlanningTask* it=pathPlanningTaskBuffer[i];
        if (it->announceCollectionWillBeErased(collectionID,true))
        {
            _erasePathPlanningTaskInBuffer(it->getObjectID());
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}


void CCopyBuffer::_announceIkGroupWillBeErased(int ikGroupID)
{ // Old
    for (size_t i=0;i<objectBuffer.size();i++)
        objectBuffer[i]->announceIkObjectWillBeErased(ikGroupID,true); // this never triggers scene object destruction!

    size_t i=0;
    while (i<ikGroupBuffer.size())
    {
        CIkGroup* it=ikGroupBuffer[i];
        if (it->announceIkGroupWillBeErased(ikGroupID,true))
        {
            _eraseIkObjectInBuffer(it->getObjectHandle());
            i=0; // Ordering may have changed
        }
        else
            i++;
    }
}

void CCopyBuffer::_announceCollisionWillBeErased(int collisionID)
{ // Old
    for (size_t i=0;i<objectBuffer.size();i++)
        objectBuffer[i]->announceCollisionWillBeErased(collisionID,true); // this never triggers scene object destruction!
}

void CCopyBuffer::_announceDistanceWillBeErased(int distanceID)
{ // Old
    for (size_t i=0;i<objectBuffer.size();i++)
        objectBuffer[i]->announceDistanceWillBeErased(distanceID,true); // this never triggers scene object destruction!
}

void CCopyBuffer::_announce2DElementWillBeErased(int elementID)
{ // Old
    size_t i=0;
    while (i<textureObjectBuffer.size())
    {
        CTextureObject* it=textureObjectBuffer[i];
        if (it->announceGeneralObjectWillBeErased(elementID,-1))
        {
            _eraseTextureObjectInBuffer(it->getObjectID());
            i=0; // Ordering may have changed!
        }
        else
            i++;
    }
}

