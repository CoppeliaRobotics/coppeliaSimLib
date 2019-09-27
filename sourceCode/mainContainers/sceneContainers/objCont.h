
// This file needs serious refactoring!!

#pragma once

#include "vrepMainHeader.h"
#include "shape.h"
#include "proximitySensor.h"
#include "visionSensor.h"
#include "camera.h"
#include "graph.h"
#include "path.h"
#include "mirror.h"
#include "octree.h"
#include "pointCloud.h"
#include "regCollection.h"
#include "regCollision.h"
#include "regDist.h"
#include "pathPlanningTask_old.h"
#include "motionPlanningTask_old.h"
#include "buttonBlock.h"
#include "luaScriptObject.h"
#include "pathCont.h"
#include "constraintSolverObject.h"
#include "mill.h"
#include "forceSensor.h"
#include "3DObject.h"
#include "jointObject.h"
#include "ikGroup.h"
#include "mainCont.h"
#include "xmlSer.h"

class CObjCont : public CMainCont 
{
public:
    CObjCont();
    virtual ~CObjCont();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    C3DObject* getSelectedObject();

    CMirror* getMirror(int objectHandle);
    COctree* getOctree(int objectHandle);
    CPointCloud* getPointCloud(int objectHandle);
    CShape* getShape(int objectHandle);
    CProxSensor* getProximitySensor(int objectHandle);
    CVisionSensor* getVisionSensor(int objectHandle);
    CCamera* getCamera(int objectHandle);
    CLight* getLight(int objectHandle);
    CGraph* getGraph(int objectHandle);
    CPath* getPath(int objectHandle);
    CMill* getMill(int objectHandle);
    CForceSensor* getForceSensor(int objectHandle);

    C3DObject* getObjectWithUniqueID(int uniqueID);

    std::string getSimilarNameWithHighestSuffix(std::string objectName,bool dash);

    bool addObjectToScene(C3DObject* newObject,bool objectIsACopy,bool generateAfterCreateCallback);
    bool addObjectToSceneWithSuffixOffset(C3DObject* newObject,bool objectIsACopy,int suffixOffset,bool generateAfterCreateCallback);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);

    void cleanupDashNames(int suffix);
    void _getAllObjectsSmallestAndBiggestSuffix(int& smallestSuffix,int& biggestSuffix);
    bool _canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void _setSuffix1ToSuffix2(int suffix1,int suffix2);

    void addObjectsToSceneAndPerformMappings(std::vector<C3DObject*>* loadedObjectList,
                                                    std::vector<CRegCollection*>* loadedGroupList,
                                                    std::vector<CRegCollision*>* loadedCollisionList,
                                                    std::vector<CRegDist*>* loadedDistanceList,
                                                    std::vector<CikGroup*>* loadedIkGroupList,
                                                    std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
                                                    std::vector<CMotionPlanningTask*>* loadedMotionPlanningTaskList,
                                                    std::vector<CButtonBlock*>* loadedButtonBlockList,
                                                    std::vector<CLuaScriptObject*>* loadedLuaScriptList,
                                                    std::vector<CConstraintSolverObject*>* loadedConstraintSolverObjectList,
                                                    std::vector<CTextureObject*>& loadedTextureObjectList,
                                                    std::vector<CDynMaterialObject*>& loadedDynMaterialObjectList,
                                                    bool model,int fileVrepVersion,bool forceModelAsCopy);

    int getSuffixOffsetForObjectToAdd(std::vector<C3DObject*>* loadedObjectList,
        std::vector<CRegCollection*>* loadedGroupList,
        std::vector<CRegCollision*>* loadedCollisionList,
        std::vector<CRegDist*>* loadedDistanceList,
        std::vector<CikGroup*>* loadedIkGroupList,
        std::vector<CPathPlanningTask*>* loadedPathPlanningTaskList,
        std::vector<CMotionPlanningTask*>* loadedMotionPlanningTaskList,
        std::vector<CButtonBlock*>* loadedButtonBlockList,
        std::vector<CLuaScriptObject*>* loadedLuaScriptList,
        std::vector<CConstraintSolverObject*>* loadedConstraintSolverObjectList);

    void appendLoadOperationIssue(const char* text,int objectId);

    // Object selection routines:
    bool selectObject(int objectHandle);
    void deselectObjects();
    void addObjectToSelection(int objectHandle);
    void removeObjectFromSelection(int objectHandle);
    void xorAddObjectToSelection(int objectHandle);
    bool isObjectSelected(int objectHandle);
    void selectAllObjects();


    void actualizeMechanismIDs();

    int getSelSize();
    int getSelID(int i);

    C3DObject* load3DObject(CSer& ar,std::string theName,bool &noHit);
    void store3DObject(CSer& ar,C3DObject* it);
    bool loadScene(CSer& ar,bool forUndoRedoOperation);
    void saveScene(CSer& ar);
    void exportIkContent(CExtIkSer& ar);
    bool loadModel(CSer& ar,bool justLoadThumbnail,bool forceModelAsCopy,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize);
    bool loadModelOrScene(CSer& ar,bool selectLoaded,bool isScene,bool justLoadThumbnail,bool forceModelAsCopy,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize);
    void setAbsoluteAngle(int objectHandle,float angle,int index);
    void setAbsolutePosition(int objectHandle,float pos,int index);
    void setAbsoluteAngles(int objectHandle,const C3Vector& euler);
    void setAbsolutePosition(int objectHandle,const C3Vector& p);

    bool getAllShapesAndDummiesFromScene(std::vector<C3DObject*>& objects,int propMask);
    bool getAllShapesFromScene(std::vector<C3DObject*>& objects,int propMask);

    void getAllCollidableObjectsFromSceneExcept(const std::vector<C3DObject*>* exceptionObjects,std::vector<C3DObject*>& objects);
    void getAllMeasurableObjectsFromSceneExcept(const std::vector<C3DObject*>* exceptionObjects,std::vector<C3DObject*>& objects);
    void getAllDetectableObjectsFromSceneExcept(const std::vector<C3DObject*>* exceptionObjects,std::vector<C3DObject*>& objects,int detectableMask);

    bool getAllShapesAndDummiesFromSceneExcept(std::vector<C3DObject*>& exceptionObjects,std::vector<C3DObject*>& objects,int propMask);

    void prepareFastLoadingMapping(std::vector<int>& map);

    // Object selection info routines:
    int getMirrorNumberInSelection();
    int getOctreeNumberInSelection();
    int getPointCloudNumberInSelection();
    int getShapeNumberInSelection();
    int getSimpleShapeNumberInSelection();
    int getCompoundNumberInSelection();
    int getJointNumberInSelection();
    int getGraphNumberInSelection();
    int getCameraNumberInSelection();
    int getLightNumberInSelection();
    int getDummyNumberInSelection();
    int getProxSensorNumberInSelection();
    int getVisionSensorNumberInSelection();
    int getPathNumberInSelection();
    int getMillNumberInSelection();
    int getForceSensorNumberInSelection();
    bool isLastSelectionAMirror();
    bool isLastSelectionAnOctree();
    bool isLastSelectionAPointCloud();
    bool isLastSelectionAShape();
    bool isLastSelectionASimpleShape();
    bool isLastSelectionACompound();
    bool isLastSelectionAJoint();
    bool isLastSelectionAGraph();
    bool isLastSelectionACamera();
    bool isLastSelectionALight();
    bool isLastSelectionADummy();
    bool isLastSelectionAProxSensor();
    bool isLastSelectionAVisionSensor();
    bool isLastSelectionAPath();
    bool isLastSelectionAMill();
    bool isLastSelectionAForceSensor();
    C3DObject* getLastSelection_object();
    CMirror* getLastSelection_mirror();
    COctree* getLastSelection_octree();
    CPointCloud* getLastSelection_pointCloud();
    CShape* getLastSelection_shape();
    CJoint* getLastSelection_joint();
    CGraph* getLastSelection_graph();
    CCamera* getLastSelection_camera();
    CLight* getLastSelection_light();
    CDummy* getLastSelection_dummy();
    CProxSensor* getLastSelection_proxSensor();
    CVisionSensor* getLastSelection_visionSensor();
    CPath* getLastSelection_path();
    CMill* getLastSelection_mill();
    CForceSensor* getLastSelection_forceSensor();
    int getLastSelectionID();
    int getMirrorNumberInSelection(std::vector<int>* selection);
    int getOctreeNumberInSelection(std::vector<int>* selection);
    int getPointCloudNumberInSelection(std::vector<int>* selection);
    int getShapeNumberInSelection(std::vector<int>* selection);
    int getSimpleShapeNumberInSelection(std::vector<int>* selection);
    int getCompoundNumberInSelection(std::vector<int>* selection);
    int getJointNumberInSelection(std::vector<int>* selection);
    int getGraphNumberInSelection(std::vector<int>* selection);
    int getCameraNumberInSelection(std::vector<int>* selection);
    int getLightNumberInSelection(std::vector<int>* selection);
    int getDummyNumberInSelection(std::vector<int>* selection);
    int getProxSensorNumberInSelection(std::vector<int>* selection);
    int getVisionSensorNumberInSelection(std::vector<int>* selection);
    int getPathNumberInSelection(std::vector<int>* selection);
    int getMillNumberInSelection(std::vector<int>* selection);
    int getForceSensorNumberInSelection(std::vector<int>* selection);
    bool isLastSelectionAMirror(std::vector<int>* selection);
    bool isLastSelectionAnOctree(std::vector<int>* selection);
    bool isLastSelectionAPointCloud(std::vector<int>* selection);
    bool isLastSelectionAShape(std::vector<int>* selection);
    bool isLastSelectionASimpleShape(std::vector<int>* selection);
    bool isLastSelectionACompound(std::vector<int>* selection);
    bool isLastSelectionAJoint(std::vector<int>* selection);
    bool isLastSelectionAGraph(std::vector<int>* selection);
    bool isLastSelectionACamera(std::vector<int>* selection);
    bool isLastSelectionALight(std::vector<int>* selection);
    bool isLastSelectionADummy(std::vector<int>* selection);
    bool isLastSelectionAProxSensor(std::vector<int>* selection);
    bool isLastSelectionAVisionSensor(std::vector<int>* selection);
    bool isLastSelectionAPath(std::vector<int>* selection);
    bool isLastSelectionAMill(std::vector<int>* selection);
    bool isLastSelectionAForceSensor(std::vector<int>* selection);
    C3DObject* getLastSelection(std::vector<int>* selection);
    int getLastSelectionID(std::vector<int>* selection);
    void getSelectedObjects(std::vector<C3DObject*>& selection);
    void getSelectedObjects(std::vector<int>& selection);
    void setSelectedObjects(const std::vector<int>& selection);

    void announceCollectionWillBeErased(int groupID);
    void announceCollisionWillBeErased(int collisionID);
    void announceDistanceWillBeErased(int distanceID);
    void announceGcsObjectWillBeErased(int gcsObjectID);
    void announce2DElementWillBeErased(int elementID);
    void announce2DElementButtonWillBeErased(int elementID,int buttonID);

    bool doesObjectExist(C3DObject* obj);
    bool isObjectInSelection(int objectID);
    bool isObjectInSelection(int objectID,std::vector<int>* selection);
    void removeFromSelectionAllExceptModelBase(bool keepObjectsSelectedThatAreNotBuiltOnAModelBase);

    bool isSelectionSame(std::vector<int>& sel,bool actualize);

    int getRenderingPosition(int objID);
    void changePoseAndKeepRestInPlace(int objectHandle,C7Vector& m,bool changePositionX,bool changePositionY,bool changePositionZ,bool changeOrientation);

    void removeAllObjects(bool generateBeforeAfterDeleteCallback);
    void actualizeObjectInformation();

    int getLoadingMapping(std::vector<int>* map,int oldVal);

    int getObjectHandleFromName(const char* objectName) const;
    int getObjectHandleFromAltName(const char* objectAltName) const;
    C3DObject* getObjectFromHandle(int objectHandle) const;
    CDummy* getDummy(int objectHandle) const;
    CJoint* getJoint(int objectHandle) const;
    C3DObject* getObjectFromName(const char* name) const;
    C3DObject* getObjectFromAltName(const char* altName) const;
    bool renameObject(int objectHandle,const char* newName);
    bool altRenameObject(int objectHandle,const char* newName);

    bool makeObjectChildOf(C3DObject* childObject,C3DObject* parentObject);
    void setAbsoluteConfiguration(int objectHandle,const C7Vector& v,bool keepChildrenInPlace);

    void removeSceneDependencies();

    int getHighestObjectHandle();
    bool eraseObject(C3DObject* it,bool generateBeforeAfterDeleteCallback);
    void eraseSeveralObjects(const std::vector<int>& objHandles,bool generateBeforeAfterDeleteCallback);

    void announceObjectWillBeErased(int objectID);
    void announceIkGroupWillBeErased(int ikGroupID);

    void enableObjectActualization(bool e);

    std::vector<C3DObject*> _objectHandleIndex;
    std::map<std::string,int> _objectNameMap;
    std::map<std::string,int> _objectAltNameMap;

    bool _objectActualizationEnabled;

    int _nextObjectHandle;

    std::vector<int> objectList;
    std::vector<int> orphanList;

    std::vector<int> jointList;
    std::vector<int> dummyList;
    std::vector<int> mirrorList;
    std::vector<int> graphList;
    std::vector<int> lightList;
    std::vector<int> cameraList;
    std::vector<int> proximitySensorList;
    std::vector<int> visionSensorList;
    std::vector<int> shapeList;
    std::vector<int> pathList;
    std::vector<int> millList;
    std::vector<int> forceSensorList;
    std::vector<int> octreeList;
    std::vector<int> pointCloudList;


    std::vector<int> selectedObjectsWhenSaving;

    C3Vector _ikManipulationStartPosRel;
    C3Vector _ikManipulationCurrentPosAbs;

private:
    std::vector<int> _selectedObjectHandles;
    std::vector<unsigned char> _selectedObjectsBool;

    std::string _loadOperationIssuesToBeDisplayed;
    std::vector<int> _loadOperationIssuesToBeDisplayed_objectHandles;
};

inline bool CObjCont::isObjectSelected(int objectHandle)
{
    if (getObjectFromHandle(objectHandle)==nullptr)
        return(false);
    return((_selectedObjectsBool[objectHandle>>3]&(1<<(objectHandle&7)))!=0);
}
