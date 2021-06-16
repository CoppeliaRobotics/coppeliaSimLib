#pragma once

#include <map>
#include "sceneObject.h"
#include "syncObject.h"

class CJoint;
class CDummy;
class CGraph;
class CLight;
class CCamera;
class CProxSensor;
class CVisionSensor;
class CShape;
class CForceSensor;
class COctree;
class CPointCloud;
// Old objects:
class CMirror;
class CPath_old;
class CMill;

enum {
    sim_syncobj_sceneobjectcont_selection=0,
};

class _CSceneObjectContainer_ : public CSyncObject
{
public:
    _CSceneObjectContainer_();
    virtual ~_CSceneObjectContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    bool doesObjectExist(const CSceneObject* obj) const;
    int getObjectSequence(const CSceneObject* object) const;
    size_t getObjectCount() const;
    CSceneObject* getObjectFromIndex(size_t index) const;
    CSceneObject* getObjectFromHandle(int objectHandle) const;
    CSceneObject* getObjectFromPath(CSceneObject* emittingObject,const char* objectAliasAndPath,int index,CSceneObject* proxy) const;
    CSceneObject* getObjectFromName_old(const char* objectName) const;
    CSceneObject* getObjectFromAltName_old(const char* objectAltName) const;
    int getObjectHandleFromName_old(const char* objectName) const;
    int getObjects_hierarchyOrder(std::vector<CSceneObject*>& allObjects);

    size_t getOrphanCount() const;
    size_t getJointCount() const;
    size_t getDummyCount() const;
    size_t getMirrorCount() const;
    size_t getGraphCount() const;
    size_t getLightCount() const;
    size_t getCameraCount() const;
    size_t getProximitySensorCount() const;
    size_t getVisionSensorCount() const;
    size_t getShapeCount() const;
    size_t getSimpleShapeCount() const;
    size_t getCompoundShapeCount() const;
    size_t getPathCount() const;
    size_t getMillCount() const;
    size_t getForceSensorCount() const;
    size_t getOctreeCount() const;
    size_t getPointCloudCount() const;
    CSceneObject* getOrphanFromIndex(size_t index) const;
    CJoint* getJointFromIndex(size_t index) const;
    CDummy* getDummyFromIndex(size_t index) const;
    CMirror* getMirrorFromIndex(size_t index) const;
    CGraph* getGraphFromIndex(size_t index) const;
    CLight* getLightFromIndex(size_t index) const;
    CCamera* getCameraFromIndex(size_t index) const;
    CProxSensor* getProximitySensorFromIndex(size_t index) const;
    CVisionSensor* getVisionSensorFromIndex(size_t index) const;
    CShape* getShapeFromIndex(size_t index) const;
    CPath_old* getPathFromIndex(size_t index) const;
    CMill* getMillFromIndex(size_t index) const;
    CForceSensor* getForceSensorFromIndex(size_t index) const;
    COctree* getOctreeFromIndex(size_t index) const;
    CPointCloud* getPointCloudFromIndex(size_t index) const;
    CDummy* getDummyFromHandle(int objectHandle) const;
    CJoint* getJointFromHandle(int objectHandle) const;
    CMirror* getMirrorFromHandle(int objectHandle) const;
    COctree* getOctreeFromHandle(int objectHandle) const;
    CPointCloud* getPointCloudFromHandle(int objectHandle) const;
    CShape* getShapeFromHandle(int objectHandle) const;
    CProxSensor* getProximitySensorFromHandle(int objectHandle) const;
    CVisionSensor* getVisionSensorFromHandle(int objectHandle) const;
    CCamera* getCameraFromHandle(int objectHandle) const;
    CLight* getLightFromHandle(int objectHandle) const;
    CGraph* getGraphFromHandle(int objectHandle) const;
    CPath_old* getPathFromHandle(int objectHandle) const;
    CMill* getMillFromHandle(int objectHandle) const;
    CForceSensor* getForceSensorFromHandle(int objectHandle) const;

    size_t getSelectionCount() const;
    int getObjectHandleFromSelectionIndex(size_t index) const;
    const std::vector<int>* getSelectedObjectHandlesPtr() const;
    bool isObjectSelected(int objectHandle) const;
    void getSelectedObjects(std::vector<CSceneObject*>& selection) const;
    int getLastSelectionHandle(const std::vector<int>* selection=nullptr) const;
    bool isObjectInSelection(int objectHandle,const std::vector<int>* selection=nullptr) const;
    CSceneObject* getLastSelectionObject(const std::vector<int>* selection=nullptr) const;
    CMirror* getLastSelectionMirror() const;
    COctree* getLastSelectionOctree() const;
    CPointCloud* getLastSelectionPointCloud() const;
    CShape* getLastSelectionShape() const;
    CJoint* getLastSelectionJoint() const;
    CGraph* getLastSelectionGraph() const;
    CCamera* getLastSelectionCamera() const;
    CLight* getLastSelectionLight() const;
    CDummy* getLastSelectionDummy() const;
    CProxSensor* getLastSelectionProxSensor() const;
    CVisionSensor* getLastSelectionVisionSensor() const;
    CPath_old* getLastSelectionPath() const;
    CMill* getLastSelectionMill() const;
    CForceSensor* getLastSelectionForceSensor() const;
    size_t getShapeCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getSimpleShapeCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getJointCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getGraphCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getDummyCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getProxSensorCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getVisionSensorCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getPathCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getMillCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getForceSensorCountInSelection(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAnOctree(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAPointCloud(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAShape(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionASimpleShape(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAJoint(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAGraph(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionADummy(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAProxSensor(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAVisionSensor(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAPath(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAMill(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAForceSensor(const std::vector<int>* selection=nullptr) const;

    virtual bool setObjectName_old(CSceneObject* object,const char* newName,bool allowNameAdjustment);
    virtual bool setObjectAltName_old(CSceneObject* object,const char* newAltName,bool allowNameAdjustment);

    virtual bool setObjectSequence(CSceneObject* object,int order);
    virtual bool setSelectedObjectHandles(const std::vector<int>* v);

protected:
    virtual void _addObject(CSceneObject* object);
    virtual void _removeObject(CSceneObject* object);
    void _addToOrphanObjects(CSceneObject* object);
    void _removeFromOrphanObjects(CSceneObject* object);
    CSceneObject* _getObjectInTree(CSceneObject* treeBase,const char* objectAliasAndPath,int& index) const;

    virtual void _setSelectedObjectHandles_send(const std::vector<int>* v) const;

private:
    std::vector<CSceneObject*> _orphanObjects;

    std::vector<CSceneObject*> _allObjects; // only used for iterating in a RANDOM manner over objects
    std::map<int,CSceneObject*> _objectHandleMap; // only used for fast access!
    std::map<std::string,CSceneObject*> _objectNameMap_old; // only used for fast access!
    std::map<std::string,CSceneObject*> _objectAltNameMap_old; // only used for fast access!

    // only used for iterating in a RANDOM manner over specific objects:
    std::vector<CJoint*> _jointList;
    std::vector<CDummy*> _dummyList;
    std::vector<CGraph*> _graphList;
    std::vector<CLight*> _lightList;
    std::vector<CCamera*> _cameraList;
    std::vector<CProxSensor*> _proximitySensorList;
    std::vector<CVisionSensor*> _visionSensorList;
    std::vector<CShape*> _shapeList;
    std::vector<CForceSensor*> _forceSensorList;
    std::vector<COctree*> _octreeList;
    std::vector<CPointCloud*> _pointCloudList;
    // Old objects:
    std::vector<CMirror*> _mirrorList;
    std::vector<CPath_old*> _pathList;
    std::vector<CMill*> _millList;

    std::vector<int> _selectedObjectHandles;
};


