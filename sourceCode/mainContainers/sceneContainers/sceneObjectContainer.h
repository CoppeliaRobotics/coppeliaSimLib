#pragma once

#include "shape.h"
#include "proximitySensor.h"
#include "visionSensor.h"
#include "camera.h"
#include "graph.h"
#include "path.h"
#include "mirror.h"
#include "octree.h"
#include "pointCloud.h"
#include "mill.h"
#include "forceSensor.h"
#include "sceneObject.h"
#include "jointObject.h"
#include "_sceneObjectContainer_.h"

struct SSimpleXmlSceneObject
{
    CSceneObject* object;
    CSceneObject* parentObject;
    CLuaScriptObject* childScript;
    CLuaScriptObject* customizationScript;
};

class CSceneObjectContainer : public _CSceneObjectContainer_
{
public:
    CSceneObjectContainer();
    virtual ~CSceneObjectContainer();

    void buildUpdateAndPopulateSynchronizationObjects();
    void connectSynchronizationObjects();
    void removeSynchronizationObjects(bool localReferencesToItOnly);

    void simulationAboutToStart();
    void simulationEnded();

    void announceObjectWillBeErased(int objectHandle);
    void announceIkGroupWillBeErased(int ikGroupHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void announceCollisionWillBeErased(int collisionHandle);
    void announceDistanceWillBeErased(int distanceHandle);

    bool addObjectToScene(CSceneObject* newObject,bool objectIsACopy,bool generateAfterCreateCallback);
    bool addObjectToSceneWithSuffixOffset(CSceneObject* newObject,bool objectIsACopy,int suffixOffset,bool generateAfterCreateCallback);
    bool eraseObject(CSceneObject* it,bool generateBeforeAfterDeleteCallback);
    void eraseSeveralObjects(const std::vector<CSceneObject*>& objects,bool generateBeforeAfterDeleteCallback);
    void eraseSeveralObjects(const std::vector<int>& objectHandles,bool generateBeforeAfterDeleteCallback);
    void removeAllObjects(bool generateBeforeAfterDeleteCallback);
    void actualizeObjectInformation();
    void enableObjectActualization(bool e);

    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1,int suffix2);

    void setTextureDependencies();
    void removeSceneDependencies();

    void getAllCollidableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects);
    void getAllMeasurableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects);
    void getAllDetectableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects,int detectableMask);

    void exportIkContent(CExtIkSer& ar);

    CSceneObject* readSceneObject(CSer& ar,const char* name,bool& noHit);
    void writeSceneObject(CSer& ar,CSceneObject* it);
    bool readAndAddToSceneSimpleXmlSceneObjects(CSer& ar,CSceneObject* parentObject,const C7Vector& localFramePreCorrection,std::vector<SSimpleXmlSceneObject>& simpleXmlObjects);
    void writeSimpleXmlSceneObjectTree(CSer& ar,const CSceneObject* object);

    void objectWasRenamed(int objectHandle,const char* oldName,const char* newName,bool altName);
    void objectGotNewParent(int objectHandle,int oldParentHandle,int newParentHandle);

    void setObjectAbsolutePose(int objectHandle,const C7Vector& v,bool keepChildrenInPlace);
    void setObjectAbsoluteOrientation(int objectHandle,const C3Vector& euler);
    void setObjectAbsolutePosition(int objectHandle,const C3Vector& p);

    // Getting objects:
    int getObjectHandleFromName(const char* objectName) const;
    int getObjectHandleFromAltName(const char* objectAltName) const;
    CSceneObject* getObjectFromName(const char* name) const;
    CSceneObject* getObjectFromAltName(const char* altName) const;
    CSceneObject* getObjectFromUniqueId(int uniqueID) const;
    int getHighestObjectHandle() const;

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
    CPath* getPathFromIndex(size_t index) const;
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
    CPath* getPathFromHandle(int objectHandle) const;
    CMill* getMillFromHandle(int objectHandle) const;
    CForceSensor* getForceSensorFromHandle(int objectHandle) const;

    // Selection:
    bool isObjectSelected(int objectHandle) const;
    bool isSelectionSame(std::vector<int>& sel,bool actualize) const;

    void selectObject(int objectHandle);
    void selectAllObjects();
    void deselectObjects();
    void addObjectToSelection(int objectHandle);
    void removeObjectFromSelection(int objectHandle);
    void xorAddObjectToSelection(int objectHandle);

    void getSelectedObjects(std::vector<CSceneObject*>& selection) const;
    int getLastSelectionHandle(const std::vector<int>* selection=nullptr) const;

    bool isObjectInSelection(int objectHandle,const std::vector<int>* selection=nullptr) const;
    void removeFromSelectionAllExceptModelBase(bool keepObjectsSelectedThatAreNotBuiltOnAModelBase);

    size_t getMirrorCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getOctreeCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getPointCloudCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getShapeCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getSimpleShapeCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getCompoundCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getJointCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getGraphCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getCameraCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getLightCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getDummyCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getProxSensorCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getVisionSensorCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getPathCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getMillCountInSelection(const std::vector<int>* selection=nullptr) const;
    size_t getForceSensorCountInSelection(const std::vector<int>* selection=nullptr) const;

    bool isLastSelectionAMirror(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAnOctree(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAPointCloud(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAShape(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionASimpleShape(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionACompound(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAJoint(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAGraph(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionACamera(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionALight(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionADummy(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAProxSensor(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAVisionSensor(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAPath(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAMill(const std::vector<int>* selection=nullptr) const;
    bool isLastSelectionAForceSensor(const std::vector<int>* selection=nullptr) const;

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
    CPath* getLastSelectionPath() const;
    CMill* getLastSelectionMill() const;
    CForceSensor* getLastSelectionForceSensor() const;

protected:
    // Overridden from _CSceneObjectContainer_:
    void _addObject(CSceneObject* object);
    bool _removeObject(int objectHandle);

private:
    CShape* _readSimpleXmlShape(CSer& ar,C7Vector& desiredLocalFrame);
    CShape* _createSimpleXmlShape(CSer& ar,bool noHeightfield,const char* itemType,bool checkSibling);
    void _writeSimpleXmlShape(CSer& ar,CShape* shape);
    void _writeSimpleXmlSimpleShape(CSer& ar,const char* originalShapeName,CShape* shape,const C7Vector& frame);

    bool _objectActualizationEnabled;
    int _nextObjectHandle;

    std::map<std::string,int> _objectNameMap;
    std::map<std::string,int> _objectAltNameMap;

    std::vector<int> _orphanList;

    std::vector<int> _jointList;
    std::vector<int> _dummyList;
    std::vector<int> _mirrorList;
    std::vector<int> _graphList;
    std::vector<int> _lightList;
    std::vector<int> _cameraList;
    std::vector<int> _proximitySensorList;
    std::vector<int> _visionSensorList;
    std::vector<int> _shapeList;
    std::vector<int> _pathList;
    std::vector<int> _millList;
    std::vector<int> _forceSensorList;
    std::vector<int> _octreeList;
    std::vector<int> _pointCloudList;
};


