#include "_sceneObjectContainer_.h"
#include "jointObject.h"
#include "dummy.h"
#include "simConst.h"
#include "app.h"

_CSceneObjectContainer_::_CSceneObjectContainer_()
{
    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_dummy; // doesn't matter, as long as it is a scene object
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CSceneObjectContainer_::~_CSceneObjectContainer_()
{
    for (size_t i=0;i<_allObjects.size();i++)
        _removeObject(_allObjects[i]);
}

bool _CSceneObjectContainer_::doesObjectExist(const CSceneObject* obj) const
{
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (obj==_allObjects[i])
            return(true);
    }
    return(false);
}

size_t _CSceneObjectContainer_::getObjectCount() const
{
    return(_allObjects.size());
}

CSceneObject* _CSceneObjectContainer_::getObjectFromIndex(size_t index) const
{
    CSceneObject* retVal=nullptr;
    if (index<_allObjects.size())
        retVal=_allObjects[index];
    return(retVal);
}

CSceneObject* _CSceneObjectContainer_::getObjectFromHandle(int objectHandle) const
{
    std::map<int,CSceneObject*>::const_iterator it=_objectHandleMap.find(objectHandle);
    if (it!=_objectHandleMap.end())
        return(it->second);
    return(nullptr);
}

int _CSceneObjectContainer_::getObjects_hierarchyOrder(std::vector<CSceneObject*>& allObjects)
{
    int retVal=0;
    for (size_t i=0;i<_orphanObjects.size();i++)
        retVal+=_orphanObjects[i]->getHierarchyTreeObjects(allObjects);
    return(retVal);
}

CSceneObject* _CSceneObjectContainer_::getObjectFromName(const char* objectName) const
{
    std::map<std::string,CSceneObject*>::const_iterator it=_objectNameMap.find(objectName);
    if (it!=_objectNameMap.end())
        return(it->second);
    return(nullptr);
}

CSceneObject* _CSceneObjectContainer_::getObjectFromAltName(const char* objectAltName) const
{
    std::map<std::string,CSceneObject*>::const_iterator it=_objectAltNameMap.find(objectAltName);
    if (it!=_objectAltNameMap.end())
        return(it->second);
    return(nullptr);
}

int _CSceneObjectContainer_::getObjectHandleFromName(const char* objectName) const
{
    int retVal=-1;
    CSceneObject* obj=getObjectFromName(objectName);
    if (obj!=nullptr)
        retVal=obj->getObjectHandle();
    return(retVal);
}

CSceneObject* _CSceneObjectContainer_::getOrphanFromIndex(size_t index) const
{
    CSceneObject* retVal=nullptr;
    if (index<_orphanObjects.size())
        retVal=_orphanObjects[index];
    return(retVal);
}

CJoint* _CSceneObjectContainer_::getJointFromIndex(size_t index) const
{
    CJoint* retVal=nullptr;
    if (index<_jointList.size())
        retVal=_jointList[index];
    return(retVal);
}

CDummy* _CSceneObjectContainer_::getDummyFromIndex(size_t index) const
{
    CDummy* retVal=nullptr;
    if (index<_dummyList.size())
        retVal=_dummyList[index];
    return(retVal);
}

CMirror* _CSceneObjectContainer_::getMirrorFromIndex(size_t index) const
{
    CMirror* retVal=nullptr;
    if (index<_mirrorList.size())
        retVal=_mirrorList[index];
    return(retVal);
}

CGraph* _CSceneObjectContainer_::getGraphFromIndex(size_t index) const
{
    CGraph* retVal=nullptr;
    if (index<_graphList.size())
        retVal=_graphList[index];
    return(retVal);
}

CLight* _CSceneObjectContainer_::getLightFromIndex(size_t index) const
{
    CLight* retVal=nullptr;
    if (index<_lightList.size())
        retVal=_lightList[index];
    return(retVal);
}

CCamera* _CSceneObjectContainer_::getCameraFromIndex(size_t index) const
{
    CCamera* retVal=nullptr;
    if (index<_cameraList.size())
        retVal=_cameraList[index];
    return(retVal);
}

CProxSensor* _CSceneObjectContainer_::getProximitySensorFromIndex(size_t index) const
{
    CProxSensor* retVal=nullptr;
    if (index<_proximitySensorList.size())
        retVal=_proximitySensorList[index];
    return(retVal);
}

CVisionSensor* _CSceneObjectContainer_::getVisionSensorFromIndex(size_t index) const
{
    CVisionSensor* retVal=nullptr;
    if (index<_visionSensorList.size())
        retVal=_visionSensorList[index];
    return(retVal);
}

CShape* _CSceneObjectContainer_::getShapeFromIndex(size_t index) const
{
    CShape* retVal=nullptr;
    if (index<_shapeList.size())
        retVal=_shapeList[index];
    return(retVal);
}

CPath_old* _CSceneObjectContainer_::getPathFromIndex(size_t index) const
{
    CPath_old* retVal=nullptr;
    if (index<_pathList.size())
        retVal=_pathList[index];
    return(retVal);
}

CMill* _CSceneObjectContainer_::getMillFromIndex(size_t index) const
{
    CMill* retVal=nullptr;
    if (index<_millList.size())
        retVal=_millList[index];
    return(retVal);
}

CForceSensor* _CSceneObjectContainer_::getForceSensorFromIndex(size_t index) const
{
    CForceSensor* retVal=nullptr;
    if (index<_forceSensorList.size())
        retVal=_forceSensorList[index];
    return(retVal);
}

COctree* _CSceneObjectContainer_::getOctreeFromIndex(size_t index) const
{
    COctree* retVal=nullptr;
    if (index<_octreeList.size())
        retVal=_octreeList[index];
    return(retVal);
}

CPointCloud* _CSceneObjectContainer_::getPointCloudFromIndex(size_t index) const
{
    CPointCloud* retVal=nullptr;
    if (index<_pointCloudList.size())
        retVal=_pointCloudList[index];
    return(retVal);
}

CDummy* _CSceneObjectContainer_::getDummyFromHandle(int objectHandle) const
{
    CDummy* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_dummy_type) )
         retVal=(CDummy*)it;
    return(retVal);
}

CJoint* _CSceneObjectContainer_::getJointFromHandle(int objectHandle) const
{
    CJoint* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_joint_type) )
         retVal=(CJoint*)it;
    return(retVal);
}

CShape* _CSceneObjectContainer_::getShapeFromHandle(int objectHandle) const
{
    CShape* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_shape_type) )
         retVal=(CShape*)it;
    return(retVal);
}

CMirror* _CSceneObjectContainer_::getMirrorFromHandle(int objectHandle) const
{
    CMirror* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_mirror_type) )
         retVal=(CMirror*)it;
    return(retVal);
}

COctree* _CSceneObjectContainer_::getOctreeFromHandle(int objectHandle) const
{
    COctree* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_octree_type) )
         retVal=(COctree*)it;
    return(retVal);
}

CPointCloud* _CSceneObjectContainer_::getPointCloudFromHandle(int objectHandle) const
{
    CPointCloud* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_pointcloud_type) )
         retVal=(CPointCloud*)it;
    return(retVal);
}

CProxSensor* _CSceneObjectContainer_::getProximitySensorFromHandle(int objectHandle) const
{
    CProxSensor* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_proximitysensor_type) )
         retVal=(CProxSensor*)it;
    return(retVal);
}

CVisionSensor* _CSceneObjectContainer_::getVisionSensorFromHandle(int objectHandle) const
{
    CVisionSensor* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_visionsensor_type) )
         retVal=(CVisionSensor*)it;
    return(retVal);
}

CPath_old* _CSceneObjectContainer_::getPathFromHandle(int objectHandle) const
{
    CPath_old* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_path_type) )
         retVal=(CPath_old*)it;
    return(retVal);
}

CMill* _CSceneObjectContainer_::getMillFromHandle(int objectHandle) const
{
    CMill* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_mill_type) )
         retVal=(CMill*)it;
    return(retVal);
}

CForceSensor* _CSceneObjectContainer_::getForceSensorFromHandle(int objectHandle) const
{
    CForceSensor* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_forcesensor_type) )
         retVal=(CForceSensor*)it;
    return(retVal);
}

CCamera* _CSceneObjectContainer_::getCameraFromHandle(int objectHandle) const
{
    CCamera* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_camera_type) )
         retVal=(CCamera*)it;
    return(retVal);
}

CLight* _CSceneObjectContainer_::getLightFromHandle(int objectHandle) const
{
    CLight* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_light_type) )
         retVal=(CLight*)it;
    return(retVal);
}

CGraph* _CSceneObjectContainer_::getGraphFromHandle(int objectHandle) const
{
    CGraph* retVal=nullptr;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if ( (it!=nullptr)&&(it->getObjectType()==sim_object_graph_type) )
         retVal=(CGraph*)it;
    return(retVal);
}

size_t _CSceneObjectContainer_::getShapeCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CShape* it=getShapeFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getSimpleShapeCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CShape* it=getShapeFromHandle(sel->at(i));
        if (it!=nullptr)
        {
            if (!it->isCompound())
                counter++;
        }
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getJointCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CJoint* it=getJointFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getGraphCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CGraph* it=getGraphFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getDummyCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CDummy* it=getDummyFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getProxSensorCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CProxSensor* it=getProximitySensorFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getVisionSensorCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CVisionSensor* it=getVisionSensorFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getPathCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CPath_old* it=getPathFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getMillCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CMill* it=getMillFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getForceSensorCountInSelection(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        CForceSensor* it=getForceSensorFromHandle(sel->at(i));
        if (it!=nullptr)
            counter++;
    }
    return (counter);
}

bool _CSceneObjectContainer_::isLastSelectionAnOctree(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    COctree* it=getOctreeFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAPointCloud(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CPointCloud* it=getPointCloudFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAShape(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CShape* it=getShapeFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionASimpleShape(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CShape* it=getShapeFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
    {
        if (!it->isCompound())
            return(true);
    }
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAJoint(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CJoint* it=getJointFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAGraph(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CGraph* it=getGraphFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionADummy(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CDummy* it=getDummyFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAProxSensor(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CProxSensor* it=getProximitySensorFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAVisionSensor(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CVisionSensor* it=getVisionSensorFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAPath(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CPath_old* it=getPathFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAMill(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CMill* it=getMillFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

bool _CSceneObjectContainer_::isLastSelectionAForceSensor(const std::vector<int>* selection/*=nullptr*/) const
{
    size_t counter=0;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(false);
    CForceSensor* it=getForceSensorFromHandle(sel->at(sel->size()-1));
    if (it!=nullptr)
        return(true);
    return(false);
}

CSceneObject* _CSceneObjectContainer_::getLastSelectionObject(const std::vector<int>* selection/*=nullptr*/) const
{
    CSceneObject* retVal=nullptr;
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()!=0)
        retVal=getObjectFromHandle(sel->at(sel->size()-1));
    return(retVal);
}


CMirror* _CSceneObjectContainer_::getLastSelectionMirror() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_mirror_type)
            return((CMirror*)it);
    }
    return(nullptr);
}

COctree* _CSceneObjectContainer_::getLastSelectionOctree() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_octree_type)
            return((COctree*)it);
    }
    return(nullptr);
}

CPointCloud* _CSceneObjectContainer_::getLastSelectionPointCloud() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_pointcloud_type)
            return((CPointCloud*)it);
    }
    return(nullptr);
}

CShape* _CSceneObjectContainer_::getLastSelectionShape() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_shape_type)
            return((CShape*)it);
    }
    return(nullptr);
}

CJoint* _CSceneObjectContainer_::getLastSelectionJoint() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_joint_type)
            return((CJoint*)it);
    }
    return(nullptr);
}

CGraph* _CSceneObjectContainer_::getLastSelectionGraph() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_graph_type)
            return((CGraph*)it);
    }
    return(nullptr);
}

CCamera* _CSceneObjectContainer_::getLastSelectionCamera() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_camera_type)
            return((CCamera*)it);
    }
    return(nullptr);
}

CLight* _CSceneObjectContainer_::getLastSelectionLight() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_light_type)
            return((CLight*)it);
    }
    return(nullptr);
}

CDummy* _CSceneObjectContainer_::getLastSelectionDummy() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_dummy_type)
            return((CDummy*)it);
    }
    return(nullptr);
}

CProxSensor* _CSceneObjectContainer_::getLastSelectionProxSensor() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_proximitysensor_type)
            return((CProxSensor*)it);
    }
    return(nullptr);
}

CVisionSensor* _CSceneObjectContainer_::getLastSelectionVisionSensor() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_visionsensor_type)
            return((CVisionSensor*)it);
    }
    return(nullptr);
}

CPath_old* _CSceneObjectContainer_::getLastSelectionPath() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_path_type)
            return((CPath_old*)it);
    }
    return(nullptr);
}

CMill* _CSceneObjectContainer_::getLastSelectionMill() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_mill_type)
            return((CMill*)it);
    }
    return(nullptr);
}

CForceSensor* _CSceneObjectContainer_::getLastSelectionForceSensor() const
{
    CSceneObject* it=getLastSelectionObject();
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_forcesensor_type)
            return((CForceSensor*)it);
    }
    return(nullptr);
}

bool _CSceneObjectContainer_::isObjectSelected(int objectHandle) const
{
    bool retVal=false;
    CSceneObject* it=getObjectFromHandle(objectHandle);
    if (it!=nullptr)
        retVal=it->getSelected();
    return(retVal);
}

void _CSceneObjectContainer_::getSelectedObjects(std::vector<CSceneObject*>& selection) const
{
    selection.clear();
    const std::vector<int>* _sel=&_selectedObjectHandles;
    for (size_t i=0;i<_sel->size();i++)
    {
        CSceneObject* it=getObjectFromHandle(_sel->at(i));
        if (it!=nullptr)
            selection.push_back(it);
    }
}

int _CSceneObjectContainer_::getLastSelectionHandle(const std::vector<int>* selection/*=nullptr*/) const
{
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    if (sel->size()==0)
        return(-1);
    return((*sel)[(sel->size()-1)]);
}

bool _CSceneObjectContainer_::isObjectInSelection(int objectHandle,const std::vector<int>* selection/*=nullptr*/) const
{
    const std::vector<int>* sel=&_selectedObjectHandles;
    if (selection!=nullptr)
        sel=selection;
    for (size_t i=0;i<sel->size();i++)
    {
        if (sel->at(i)==objectHandle)
            return(true);
    }
    return(false);
}

size_t _CSceneObjectContainer_::getJointCount() const
{
    return(_jointList.size());
}

size_t _CSceneObjectContainer_::getDummyCount() const
{
    return(_dummyList.size());
}

size_t _CSceneObjectContainer_::getMirrorCount() const
{
    return(_mirrorList.size());
}

size_t _CSceneObjectContainer_::getGraphCount() const
{
    return(_graphList.size());
}

size_t _CSceneObjectContainer_::getLightCount() const
{
    return(_lightList.size());
}

size_t _CSceneObjectContainer_::getCameraCount() const
{
    return(_cameraList.size());
}

size_t _CSceneObjectContainer_::getProximitySensorCount() const
{
    return(_proximitySensorList.size());
}

size_t _CSceneObjectContainer_::getVisionSensorCount() const
{
    return(_visionSensorList.size());
}

size_t _CSceneObjectContainer_::getShapeCount() const
{
    return(_shapeList.size());
}

size_t _CSceneObjectContainer_::getSimpleShapeCount() const
{
    size_t counter=0;
    for (size_t i=0;i<_shapeList.size();i++)
    {
        CShape* it=getShapeFromIndex(i);
        if (!it->isCompound())
            counter++;
    }
    return (counter);
}

size_t _CSceneObjectContainer_::getCompoundShapeCount() const
{
    return(getShapeCount()-getSimpleShapeCount());
}

size_t _CSceneObjectContainer_::getPathCount() const
{
    return(_pathList.size());
}

size_t _CSceneObjectContainer_::getMillCount() const
{
    return(_millList.size());
}

size_t _CSceneObjectContainer_::getForceSensorCount() const
{
    return(_forceSensorList.size());
}

size_t _CSceneObjectContainer_::getOctreeCount() const
{
    return(_octreeList.size());
}

size_t _CSceneObjectContainer_::getPointCloudCount() const
{
    return(_pointCloudList.size());
}

size_t _CSceneObjectContainer_::getOrphanCount() const
{
    return(_orphanObjects.size());
}

void _CSceneObjectContainer_::_addToOrphanObjects(CSceneObject* object)
{
    _orphanObjects.push_back(object);
}

void _CSceneObjectContainer_::_removeFromOrphanObjects(CSceneObject* object)
{
    for (size_t i=0;i<_orphanObjects.size();i++)
    {
        if (_orphanObjects[i]==object)
        {
            _orphanObjects.erase(_orphanObjects.begin()+i);
            break;
        }
    }
}

bool _CSceneObjectContainer_::setObjectName(CSceneObject* object,const char* newName,bool allowNameAdjustment)
{
    std::map<std::string,CSceneObject*>::iterator it=_objectNameMap.find(newName);
    if (it==_objectNameMap.end()&&(strlen(newName)!=0))
    {
        _objectNameMap.erase(object->getObjectName());
        _objectNameMap[newName]=object;
        return(true);
    }
    return(false);
}

bool _CSceneObjectContainer_::setObjectAltName(CSceneObject* object,const char* newName,bool allowNameAdjustment)
{
    std::map<std::string,CSceneObject*>::iterator it=_objectAltNameMap.find(newName);
    if (it==_objectNameMap.end()&&(strlen(newName)!=0))
    {
        _objectAltNameMap.erase(object->getObjectAltName());
        _objectAltNameMap[newName]=object;
        return(true);
    }
    return(false);
}

void _CSceneObjectContainer_::_addObject(CSceneObject* object)
{
    _orphanObjects.push_back(object);
    _allObjects.push_back(object);
    _objectHandleMap[object->getObjectHandle()]=object;
    _objectNameMap[object->getObjectName()]=object;
    _objectAltNameMap[object->getObjectAltName()]=object;
    int t=object->getObjectType();
    if (t==sim_object_joint_type)
        _jointList.push_back((CJoint*)object);
    if (t==sim_object_dummy_type)
        _dummyList.push_back((CDummy*)object);
    if (t==sim_object_graph_type)
        _graphList.push_back((CGraph*)object);
    if (t==sim_object_light_type)
        _lightList.push_back((CLight*)object);
    if (t==sim_object_camera_type)
        _cameraList.push_back((CCamera*)object);
    if (t==sim_object_proximitysensor_type)
        _proximitySensorList.push_back((CProxSensor*)object);
    if (t==sim_object_visionsensor_type)
        _visionSensorList.push_back((CVisionSensor*)object);
    if (t==sim_object_shape_type)
        _shapeList.push_back((CShape*)object);
    if (t==sim_object_forcesensor_type)
        _forceSensorList.push_back((CForceSensor*)object);
    if (t==sim_object_octree_type)
        _octreeList.push_back((COctree*)object);
    if (t==sim_object_pointcloud_type)
        _pointCloudList.push_back((CPointCloud*)object);
    if (t==sim_object_mirror_type)
        _mirrorList.push_back((CMirror*)object);
    if (t==sim_object_path_type)
        _pathList.push_back((CPath_old*)object);
    if (t==sim_object_mill_type)
        _millList.push_back((CMill*)object);
}

void _CSceneObjectContainer_::_removeObject(CSceneObject* object)
{
    for (size_t i=0;i<_allObjects.size();i++)
    {
        if (_allObjects[i]==object)
        {
            _allObjects.erase(_allObjects.begin()+i);
            break;
        }
    }
    for (size_t i=0;i<_orphanObjects.size();i++)
    {
        if (_orphanObjects[i]==object)
        {
            _orphanObjects.erase(_orphanObjects.begin()+i);
            break;
        }
    }
    int t=object->getObjectType();
    std::vector<CSceneObject*>* list;
    if (t==sim_object_joint_type)
        list=(std::vector<CSceneObject*>*)&_jointList;
    if (t==sim_object_dummy_type)
        list=(std::vector<CSceneObject*>*)&_dummyList;
    if (t==sim_object_graph_type)
        list=(std::vector<CSceneObject*>*)&_graphList;
    if (t==sim_object_light_type)
        list=(std::vector<CSceneObject*>*)&_lightList;
    if (t==sim_object_camera_type)
        list=(std::vector<CSceneObject*>*)&_cameraList;
    if (t==sim_object_proximitysensor_type)
        list=(std::vector<CSceneObject*>*)&_proximitySensorList;
    if (t==sim_object_visionsensor_type)
        list=(std::vector<CSceneObject*>*)&_visionSensorList;
    if (t==sim_object_shape_type)
        list=(std::vector<CSceneObject*>*)&_shapeList;
    if (t==sim_object_forcesensor_type)
        list=(std::vector<CSceneObject*>*)&_forceSensorList;
    if (t==sim_object_octree_type)
        list=(std::vector<CSceneObject*>*)&_octreeList;
    if (t==sim_object_pointcloud_type)
        list=(std::vector<CSceneObject*>*)&_pointCloudList;
    if (t==sim_object_mirror_type)
        list=(std::vector<CSceneObject*>*)&_mirrorList;
    if (t==sim_object_path_type)
        list=(std::vector<CSceneObject*>*)&_pathList;
    if (t==sim_object_mill_type)
        list=(std::vector<CSceneObject*>*)&_millList;
    for (size_t i=0;i<list->size();i++)
    {
        if (list->at(i)==object)
        {
            list->erase(list->begin()+i);
            break;
        }
    }
    for (size_t i=0;i<_selectedObjectHandles.size();i++)
    {
        if (_selectedObjectHandles[i]==object->getObjectHandle())
        {
            _selectedObjectHandles.erase(_selectedObjectHandles.begin()+i);
            break;
        }
    }

    _objectHandleMap.erase(object->getObjectHandle());
    _objectNameMap.erase(object->getObjectName());
    _objectAltNameMap.erase(object->getObjectAltName());
}

const std::vector<int>* _CSceneObjectContainer_::getSelectedObjectHandlesPtr() const
{
    return(&_selectedObjectHandles);
}

bool _CSceneObjectContainer_::setSelectedObjectHandles(const std::vector<int>* v)
{
    bool diff=false;
    if (v==nullptr)
        diff=(_selectedObjectHandles.size()>0);
    else
    {
        diff=(v->size()!=_selectedObjectHandles.size());
        if (!diff)
        {
            for (size_t i=0;i<_selectedObjectHandles.size();i++)
            {
                if (_selectedObjectHandles[i]!=v->at(i))
                {
                    diff=true;
                    break;
                }
            }
        }
    }
    if (diff)
    {
        // First make sure that handles are valid
        std::vector<int> w;
        if (v!=nullptr)
        {
            for (size_t i=0;i<v->size();i++)
            {
                CSceneObject* it=getObjectFromHandle(v->at(i));
                if (it!=nullptr)
                    w.push_back(it->getObjectHandle());
            }
        }
        if (getObjectCanChange())
        {
            _selectedObjectHandles.clear();
            if (v!=nullptr)
                _selectedObjectHandles.assign(w.begin(),w.end());
        }
        if (getObjectCanSync())
            _setSelectedObjectHandles_send(&w);
    }
    return(diff&&getObjectCanChange());
}

void _CSceneObjectContainer_::_setSelectedObjectHandles_send(const std::vector<int>* v) const
{
    if ( (v==nullptr)||(v->size()==0) )
        sendInt32Array(nullptr,0,sim_syncobj_sceneobjectcont_selection);
    else
        sendInt32Array(&v->at(0),v->size(),sim_syncobj_sceneobjectcont_selection);
}

size_t _CSceneObjectContainer_::getSelectionCount() const
{
    return(_selectedObjectHandles.size());
}

int _CSceneObjectContainer_::getObjectHandleFromSelectionIndex(size_t index) const
{
    return(_selectedObjectHandles[index]);
}

void _CSceneObjectContainer_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if ( (routing[0].objType>=sim_syncobj_sceneobjectstart)&&(routing[0].objType<=sim_syncobj_sceneobjectend) )
    {
        if (routing[0].objHandle!=-1)
        { // Msg is for an object
            CSceneObject* obj=getObjectFromHandle(routing[0].objHandle);
            if (obj==nullptr)
            {
                if (routing.size()==1) // check also size (some msgs have same ids in different scopes)
                {
                    if (msg.msg==sim_syncobj_sceneobject_create)
                    {
                        int handle=routing[0].objHandle;
                        if (App::currentWorld->sceneObjects->getObjectFromHandle(handle)==nullptr)
                        {
                            if (routing[0].objType==sim_syncobj_joint)
                                obj=new CJoint(((int*)msg.data)[0]);
                            if (routing[0].objType==sim_syncobj_dummy)
                                obj=new CDummy();
                            obj->setObjectHandle(handle);
                            _addObject(obj);
                        }
                        else
                        {
#ifndef SIM_LIB
                            simAddLog("Sync",sim_verbosity_errors,"trying to create a scene object with a handle that already exists.");
#endif
                        }
                    }
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_sceneobject_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeObject(getObjectFromHandle(routing[0].objHandle));
                else
                {
                    routing.erase(routing.begin());
                    obj->synchronizationMsg(routing,msg);
                }
            }
        }
        else
        { // Msg is for this object container
            if (msg.msg==sim_syncobj_sceneobjectcont_selection)
            {
                std::vector<int> v(((int*)msg.data),((int*)msg.data)+msg.dataSize);
                setSelectedObjectHandles(&v);
                return;
            }
        }
    }
}
