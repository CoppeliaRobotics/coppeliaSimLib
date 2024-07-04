#pragma once

#include <map>
#include <shape.h>
#include <proximitySensor.h>
#include <visionSensor.h>
#include <camera.h>
#include <script.h>
#include <graph.h>
#include <path_old.h>
#include <mirror.h>
#include <octree.h>
#include <pointCloud.h>
#include <mill.h>
#include <forceSensor.h>
#include <sceneObject.h>
#include <jointObject.h>
#include <sceneObject.h>
#include <embeddedScriptContainer.h>

class CJoint;
class CDummy;
class CGraph;
class CLight;
class CCamera;
class CProxSensor;
class CVisionSensor;
class CShape;
class CForceSensor;
class COcTree;
class CPointCloud;
// Old objects:
class CMirror;
class CPath_old;
class CMill;

struct SSimpleXmlSceneObject
{
    CSceneObject *object;
    CSceneObject *parentObject;
    CScriptObject *childScript;
    CScriptObject *customizationScript;
};

class CSceneObjectContainer
{
  public:
    CSceneObjectContainer();
    virtual ~CSceneObjectContainer();

    bool doesObjectExist(const CSceneObject *obj) const;
    int getObjectSequence(const CSceneObject *object, int* totalSiblings = nullptr) const;
    size_t getObjectCount(int type = -1) const;
    CSceneObject *getObjectFromIndex(size_t index) const;
    CSceneObject *getObjectFromHandle(int objectHandle) const;
    CSceneObject *getObjectFromUid(long long int objectUid) const;
    CSceneObject *getObjectFromPath(const CSceneObject *emittingObject, const char *objectAliasAndPath,
                                    int index) const;
    CSceneObject *getObjectFromName_old(const char *objectName) const;
    CSceneObject *getObjectFromAltName_old(const char *objectAltName) const;
    int getObjectHandleFromName_old(const char *objectName) const;
    int getObjects_hierarchyOrder(std::vector<CSceneObject *> &allObjects);

    size_t getOrphanCount() const;
    size_t getSimpleShapeCount() const;
    size_t getCompoundShapeCount() const;

    CSceneObject *getOrphanFromIndex(size_t index) const;
    CJoint *getJointFromIndex(size_t index) const;
    CDummy *getDummyFromIndex(size_t index) const;
    CScript *getScriptFromIndex(size_t index) const;
    CMirror *getMirrorFromIndex(size_t index) const;
    CGraph *getGraphFromIndex(size_t index) const;
    CLight *getLightFromIndex(size_t index) const;
    CCamera *getCameraFromIndex(size_t index) const;
    CProxSensor *getProximitySensorFromIndex(size_t index) const;
    CVisionSensor *getVisionSensorFromIndex(size_t index) const;
    CShape *getShapeFromIndex(size_t index) const;
    CPath_old *getPathFromIndex(size_t index) const;
    CMill *getMillFromIndex(size_t index) const;
    CForceSensor *getForceSensorFromIndex(size_t index) const;
    COcTree *getOctreeFromIndex(size_t index) const;
    CPointCloud *getPointCloudFromIndex(size_t index) const;

    CScriptObject *getScriptObjectFromHandle(int handle) const;
    CScriptObject *getScriptObjectFromUid(int uid) const;

    CDummy *getDummyFromHandle(int objectHandle) const;
    CScript *getScriptFromHandle(int objectHandle) const;
    CJoint *getJointFromHandle(int objectHandle) const;
    CMirror *getMirrorFromHandle(int objectHandle) const;
    COcTree *getOctreeFromHandle(int objectHandle) const;
    CPointCloud *getPointCloudFromHandle(int objectHandle) const;
    CShape *getShapeFromHandle(int objectHandle) const;
    CProxSensor *getProximitySensorFromHandle(int objectHandle) const;
    CVisionSensor *getVisionSensorFromHandle(int objectHandle) const;
    CCamera *getCameraFromHandle(int objectHandle) const;
    CLight *getLightFromHandle(int objectHandle) const;
    CGraph *getGraphFromHandle(int objectHandle) const;
    CPath_old *getPathFromHandle(int objectHandle) const;
    CMill *getMillFromHandle(int objectHandle) const;
    CForceSensor *getForceSensorFromHandle(int objectHandle) const;

    bool hasSelectionChanged();
    size_t getSelectionCount() const;
    int getObjectHandleFromSelectionIndex(size_t index) const;
    const std::vector<int> *getSelectedObjectHandlesPtr() const;
    bool isObjectSelected(int objectHandle) const;
    void getSelectedObjects(std::vector<CSceneObject *> &selection, int objectType = -1,
                            bool includeModelObjects = false, bool onlyVisibleModelObjects = false) const;
    void getSelectedObjectHandles(std::vector<int> &selection, int objectType = -1, bool includeModelObjects = false,
                                  bool onlyVisibleModelObjects = false) const;
    int getLastSelectionHandle(const std::vector<int> *selection = nullptr) const;
    bool isObjectInSelection(int objectHandle, const std::vector<int> *selection = nullptr) const;
    void popLastSelection();

    CSceneObject *getLastSelectionObject(const std::vector<int> *selection = nullptr) const;
    CMirror *getLastSelectionMirror() const;
    COcTree *getLastSelectionOctree() const;
    CPointCloud *getLastSelectionPointCloud() const;
    CShape *getLastSelectionShape() const;
    CJoint *getLastSelectionJoint() const;
    CGraph *getLastSelectionGraph() const;
    CCamera *getLastSelectionCamera() const;
    CLight *getLastSelectionLight() const;
    CDummy *getLastSelectionDummy() const;
    CScript *getLastSelectionScript() const;
    CProxSensor *getLastSelectionProxSensor() const;
    CVisionSensor *getLastSelectionVisionSensor() const;
    CPath_old *getLastSelectionPath() const;
    CMill *getLastSelectionMill() const;
    CForceSensor *getLastSelectionForceSensor() const;

    size_t getObjectCountInSelection(int objectType = -1, const std::vector<int> *selection = nullptr) const;
    size_t getSimpleShapeCountInSelection(const std::vector<int> *selection = nullptr) const;

    bool isLastSelectionOfType(int objectType, const std::vector<int> *selection = nullptr) const;
    bool isLastSelectionASimpleShape(const std::vector<int> *selection = nullptr) const;

    void buildOrUpdate_oldIk();
    void connect_oldIk();
    void remove_oldIk();

    void simulationAboutToStart();
    void simulationAboutToEnd();
    void simulationEnded();

    void announceObjectWillBeErased(const CSceneObject *object);
    void announceScriptWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);

    void handleDataCallbacks();
    bool shouldTemporarilySuspendMainScript();
    size_t getScriptsToExecute(std::vector<int> &scriptHandles, int scriptType, bool legacyEmbeddedScripts, bool reverseOrder) const;
    void callScripts(int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch = nullptr, int scriptToExclude = -1);
    int callScripts_noMainScript(int scriptType, int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch = nullptr, int scriptToExclude = -1);
    void setScriptsTemporarilySuspended(bool suspended);
    int getSysFuncAndHookCnt(int sysCall) const;
    void setSysFuncAndHookCnt(int sysCall, int cnt);

    // Old:
    void announceIkGroupWillBeErased(int ikGroupHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void announceCollisionWillBeErased(int collisionHandle);
    void announceDistanceWillBeErased(int distanceHandle);

    int addObjectToScene(CSceneObject *newObject, bool objectIsACopy, bool generateAfterCreateCallback);
    int addObjectToSceneWithSuffixOffset(CSceneObject *newObject, bool objectIsACopy, int suffixOffset, bool generateAfterCreateCallback);
    int addDefaultScript(int scriptType, bool threaded, bool lua);
    bool addCommandToOutsideCommandQueues(int commandID, int auxVal1, int auxVal2, int auxVal3, int auxVal4, const double aux2Vals[8], int aux2Count);


    void eraseObject(CSceneObject *it, bool generateBeforeAfterDeleteCallback, bool delayed = false);
    bool eraseObjects(const std::vector<int>* objectHandles, bool generateBeforeAfterDeleteCallback, bool delayed = false);
    void eraseAllObjects(bool generateBeforeAfterDeleteCallback);
    void actualizeObjectInformation();
    void enableObjectActualization(bool e);
    void getMinAndMaxNameSuffixes(int &minSuffix, int &maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1, int suffix2);
    int getObjectCreationCounter() const;
    int getObjectDestructionCounter() const;
    int getHierarchyChangeCounter() const;

    int setBoolProperty(int target, const char* pName, bool pState);
    int getBoolProperty(int target, const char* pName, bool& pState);
    int setInt32Property(int target, const char* pName, int pState);
    int getInt32Property(int target, const char* pName, int& pState);
    int setFloatProperty(int target, const char* pName, double pState);
    int getFloatProperty(int target, const char* pName, double& pState);
    int setStringProperty(int target, const char* pName, const char* pState);
    int getStringProperty(int target, const char* pName, std::string& pState);
    int setBufferProperty(int target, const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(int target, const char* pName, std::string& pState);
    int setVector3Property(int target, const char* pName, const C3Vector& pState);
    int getVector3Property(int target, const char* pName, C3Vector& pState);
    int setQuaternionProperty(int target, const char* pName, const C4Vector& pState);
    int getQuaternionProperty(int target, const char* pName, C4Vector& pState);
    int setPoseProperty(int target, const char* pName, const C7Vector& pState);
    int getPoseProperty(int target, const char* pName, C7Vector& pState);
    int setMatrix3x3Property(int target, const char* pName, const C3X3Matrix& pState);
    int getMatrix3x3Property(int target, const char* pName, C3X3Matrix& pState);
    int setMatrix4x4Property(int target, const char* pName, const C4X4Matrix& pState);
    int getMatrix4x4Property(int target, const char* pName, C4X4Matrix& pState);
    int setColorProperty(int target, const char* pName, const float* pState);
    int getColorProperty(int target, const char* pName, float* pState);
    int setVectorProperty(int target, const char* pName, const double* v, int vL);
    int getVectorProperty(int target, const char* pName, std::vector<double>& pState);
    int removeProperty(int target, const char* pName);
    int getPropertyName(int target, int& index, std::string& pName);
    int getPropertyInfo(int target, const char* pName, int& info, int& size);

    void setTextureDependencies();
    void removeSceneDependencies();

    void checkObjectIsInstanciated(CSceneObject *obj, const char *location) const;
    void pushGenesisEvents() const;

    void getAllCollidableObjectsFromSceneExcept(const std::vector<CSceneObject *> *exceptionObjects,
                                                std::vector<CSceneObject *> &objects);
    void getAllMeasurableObjectsFromSceneExcept(const std::vector<CSceneObject *> *exceptionObjects,
                                                std::vector<CSceneObject *> &objects);
    void getAllDetectableObjectsFromSceneExcept(const std::vector<CSceneObject *> *exceptionObjects,
                                                std::vector<CSceneObject *> &objects, int detectableMask);

    CSceneObject *readSceneObject(CSer &ar, const char *name, bool &noHit);
    void writeSceneObject(CSer &ar, CSceneObject *it);
    bool readAndAddToSceneSimpleXmlSceneObjects(CSer &ar, CSceneObject *parentObject,
                                                const C7Vector &localFramePreCorrection,
                                                std::vector<SSimpleXmlSceneObject> &simpleXmlObjects);
    void writeSimpleXmlSceneObjectTree(CSer &ar, const CSceneObject *object);

    bool setObjectAlias(CSceneObject *object, const char *newAlias, bool allowNameAdjustment);
    bool setObjectParent(CSceneObject *object, CSceneObject *newParent, bool keepInPlace);

    bool setObjectSequence(CSceneObject *object, int order);
    bool setSelectedObjectHandles(const std::vector<int> *v);
    bool setObjectName_old(CSceneObject *object, const char *newName, bool allowNameAdjustment);
    bool setObjectAltName_old(CSceneObject *object, const char *newAltName, bool allowNameAdjustment);

    void setObjectAbsolutePose(int objectHandle, const C7Vector &v, bool keepChildrenInPlace);
    void setObjectAbsoluteOrientation(int objectHandle, const C3Vector &euler);
    void setObjectAbsolutePosition(int objectHandle, const C3Vector &p);

    int getHighestObjectHandle() const;

    void addModelObjects(std::vector<int> &selection) const;
    void addCompatibilityScripts(std::vector<int> &selection) const;

    // Selection:
    bool isSelectionSame(std::vector<int> &sel, bool actualize) const;

    void selectObject(int objectHandle);
    void selectAllObjects();
    void deselectObjects();
    void addObjectToSelection(int objectHandle);
    void removeObjectFromSelection(int objectHandle);
    void xorAddObjectToSelection(int objectHandle);
    void removeFromSelectionAllExceptModelBase(bool keepObjectsSelectedThatAreNotBuiltOnAModelBase);

    void resetScriptFlagCalledInThisSimulationStep();
    int getCalledScriptsCountInThisSimulationStep(bool onlySimulationScripts);

    CEmbeddedScriptContainer *embeddedScriptContainer;

  protected:
    void _handleOrderIndexOfOrphans();
    void _addToOrphanObjects(CSceneObject *object);
    void _removeFromOrphanObjects(CSceneObject *object);
    CSceneObject *_getObjectInTree(const CSceneObject *treeBase, const char *objectAliasAndPath, int &index) const;
    CSceneObject *_getObjectFromSimplePath(const CSceneObject *emittingObject, const char *objectAliasAndPath,
                                           int index) const;
    CSceneObject *_getObjectFromComplexPath(const CSceneObject *emittingObject, std::string &path, int index) const;

    void _addObject(CSceneObject *object);
    void _removeObject(CSceneObject *object);

  private:
    int _callScripts(int scriptType, int callType, CInterfaceStack *inStack, CInterfaceStack *outStack, CSceneObject *objectBranch = nullptr, int scriptToExclude = -1);
    CShape *_readSimpleXmlShape(CSer &ar, C7Vector &desiredLocalFrame);
    CShape *_createSimpleXmlShape(CSer &ar, bool noHeightfield, const char *itemType, bool checkSibling);
    void _writeSimpleXmlShape(CSer &ar, CShape *shape);
    void _writeSimpleXmlSimpleShape(CSer &ar, const char *originalShapeName, CShape *shape, const C7Vector &frame);

    bool _objectActualizationEnabled;
    int _nextObjectHandle;

    int _objectCreationCounter;
    int _objectDestructionCounter;
    int _hierarchyChangeCounter;

    std::vector<int> _delayedDestructionObjects;

    std::vector<CSceneObject *> _orphanObjects;

    std::vector<CSceneObject *> _allObjects;                  // only used for iterating in a RANDOM manner over objects
    std::map<int, CSceneObject *> _objectHandleMap;           // only used for fast access!
    std::map<std::string, CSceneObject *> _objectNameMap_old; // only used for fast access!
    std::map<std::string, CSceneObject *> _objectAltNameMap_old; // only used for fast access!

    // only used for iterating in a RANDOM manner over specific objects:
    std::vector<CJoint *> _jointList;
    std::vector<CDummy *> _dummyList;
    std::vector<CScript *> _scriptList;
    std::vector<CGraph *> _graphList;
    std::vector<CLight *> _lightList;
    std::vector<CCamera *> _cameraList;
    std::vector<CProxSensor *> _proximitySensorList;
    std::vector<CVisionSensor *> _visionSensorList;
    std::vector<CShape *> _shapeList;
    std::vector<CForceSensor *> _forceSensorList;
    std::vector<COcTree *> _octreeList;
    std::vector<CPointCloud *> _pointCloudList;
    // Old objects:
    std::vector<CMirror *> _mirrorList;
    std::vector<CPath_old *> _pathList;
    std::vector<CMill *> _millList;

    std::vector<int> _selectedObjectHandles;
    std::vector<int> _lastSelection; // to keep track of selection changes (async.)

    int _sysFuncAndHookCnt_event;
    int _sysFuncAndHookCnt_dyn;
    int _sysFuncAndHookCnt_contact;
    int _sysFuncAndHookCnt_joint;
};
