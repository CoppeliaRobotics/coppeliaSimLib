
#include "vrepMainHeader.h"
#include "cuttingRoutine.h"
#include "pluginContainer.h"
#include "app.h"


int CCuttingRoutine::cutEntity(int millID,int entityID,int& cutObject,float& cutSurface,float& cutVolume,bool justForInitialization,bool overrideCuttableFlagIfNonCollection)
{ // entityID==-1 --> checks all objects in the scene. Return value is the number of cut objects
    int cutCount=0;
    cutObject=-1;
    cutSurface=0.0f;
    cutVolume=0.0f;
    C3DObject* object=App::ct->objCont->getObjectFromHandle(entityID);
    CMill* sens=App::ct->objCont->getMill(millID);
    if (sens==nullptr)
        return(0); // should never happen!
    App::ct->calcInfo->millSimulationStart();

    C3Vector minV,maxV;
    sens->getMillingVolumeBoundingBox(minV,maxV);
    C4X4Matrix millingVolumeBox;
    millingVolumeBox.setIdentity();
    millingVolumeBox.X=(maxV+minV)*0.5f;
    C3Vector millingVolumeSize(maxV-minV);

    if (object==nullptr)
        cutCount=_cutGroup(millID,entityID,cutObject,cutSurface,cutVolume,justForInitialization);
    else
    { // Cutting objects:
        if (object->getObjectType()==sim_object_shape_type)
        {
            if (_cutShape(millID,entityID,cutSurface,justForInitialization,overrideCuttableFlagIfNonCollection))
            {
                cutObject=entityID;
                cutCount++;
            }
        }
    }
    App::ct->calcInfo->millSimulationEnd(cutSurface,cutVolume);
    return(cutCount);
}


bool CCuttingRoutine::_cutShape(int millID,int shapeID,float& cutSurface,bool justForInitialization,bool overrideShapeCuttableFlag)
{   // Return value is false if nothing was cut
    cutSurface=0.0f;
    // Get the objects:
    CMill* mill=App::ct->objCont->getMill(millID);
    if (mill==nullptr)
        return(false);
    CShape* shape=App::ct->objCont->getShape(shapeID);
    if (shape==nullptr)
        return(false);
    if ( ((shape->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_cuttable)==0)&&(!overrideShapeCuttableFlag) )
        return(false);

    // Is shape initialized for cutting?
    shape->initializeCalculationStructureIfNeeded();

    C7Vector inv(mill->getCumulativeTransformation().getInverse());
    C4X4Matrix shapeRTM((inv*shape->getCumulativeTransformation()).getMatrix());

    if (!justForInitialization)
        cutSurface=CPluginContainer::mesh_cutNodeWithVolume(shape->geomData->collInfo,shapeRTM,&mill->convexVolume->planesInside);
    return (cutSurface>0.0f);
}

int CCuttingRoutine::_cutGroup(int millID,int groupID,int& cutObject,float& cutSurface,float& cutVolume,bool justForInitialization)
{   // Return value corresponds to the number of objects cut. If groupID==-1, all objects in the scene will be cut
    cutObject=-1;
    cutSurface=0.0f;
    cutVolume=0.0f;
    // Get the objects:
    CMill* mill=App::ct->objCont->getMill(millID);
    if (mill==nullptr)
        return(0);
    std::vector<C3DObject*> group;
    if (!App::ct->collections->getShapesAndVolumesFromCollection(groupID,&group,sim_objectspecialproperty_cuttable,false))
    { // Here we probably want to check all objects in the scene:
        if (groupID!=-1)
            return(0);
        App::ct->objCont->getAllShapesFromScene(group,sim_objectspecialproperty_cuttable);
    }

    int cutCount=0;

    for (int i=0;i<int(group.size());i++)
    { // We process all objects in the group:
        if (group[i]->getObjectType()==sim_object_shape_type)
        {
            float cutSurfaceLocal;
            if (_cutShape(millID,group[i]->getObjectHandle(),cutSurfaceLocal,justForInitialization,true))
            {
                cutSurface+=cutSurfaceLocal;
                cutObject=group[i]->getObjectHandle();
                cutCount++;
            }
        }
    }
    return(cutCount);
}


