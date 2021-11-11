#pragma once

#include "shape.h"
#include "proximitySensor.h"
#include "visionSensor.h"
#include "camera.h"
#include "graph.h"
#include "path_old.h"
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
    CScriptObject* childScript;
    CScriptObject* customizationScript;
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
    void announceScriptWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);

    // Old:
    void announceIkGroupWillBeErased(int ikGroupHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void announceCollisionWillBeErased(int collisionHandle);
    void announceDistanceWillBeErased(int distanceHandle);

    void addObjectToScene(CSceneObject* newObject,bool objectIsACopy,bool generateAfterCreateCallback);
    void addObjectToSceneWithSuffixOffset(CSceneObject* newObject,bool objectIsACopy,int suffixOffset,bool generateAfterCreateCallback);
    bool eraseObject(CSceneObject* it,bool generateBeforeAfterDeleteCallback);
    void eraseSeveralObjects(const std::vector<int>& objectHandles,bool generateBeforeAfterDeleteCallback);
    void removeAllObjects(bool generateBeforeAfterDeleteCallback);
    void actualizeObjectInformation();
    void enableObjectActualization(bool e);

    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1,int suffix2);

    void setTextureDependencies();
    void removeSceneDependencies();

    void pushAllInitialEvents() const;

    void getAllCollidableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects);
    void getAllMeasurableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects);
    void getAllDetectableObjectsFromSceneExcept(const std::vector<CSceneObject*>* exceptionObjects,std::vector<CSceneObject*>& objects,int detectableMask);

    CSceneObject* readSceneObject(CSer& ar,const char* name,bool& noHit);
    void writeSceneObject(CSer& ar,CSceneObject* it);
    bool readAndAddToSceneSimpleXmlSceneObjects(CSer& ar,CSceneObject* parentObject,const C7Vector& localFramePreCorrection,std::vector<SSimpleXmlSceneObject>& simpleXmlObjects);
    void writeSimpleXmlSceneObjectTree(CSer& ar,const CSceneObject* object);

    bool setObjectAlias(CSceneObject* object,const char* newAlias,bool allowNameAdjustment);
    void setObjectParent(CSceneObject* object,CSceneObject* newParent,bool keepInPlace);

    // Overridden from _CSceneObjectContainer_:
    bool setObjectSequence(CSceneObject* object,int order);
    bool setSelectedObjectHandles(const std::vector<int>* v);
    bool setObjectName_old(CSceneObject* object,const char* newName,bool allowNameAdjustment);
    bool setObjectAltName_old(CSceneObject* object,const char* newAltName,bool allowNameAdjustment);

    void setObjectAbsolutePose(int objectHandle,const C7Vector& v,bool keepChildrenInPlace);
    void setObjectAbsoluteOrientation(int objectHandle,const C3Vector& euler);
    void setObjectAbsolutePosition(int objectHandle,const C3Vector& p);

    CSceneObject* getObjectFromUniqueId(int uniqueId) const;
    int getHighestObjectHandle() const;


    // Selection:
    bool isSelectionSame(std::vector<int>& sel,bool actualize) const;

    void selectObject(int objectHandle);
    void selectAllObjects();
    void deselectObjects();
    void addObjectToSelection(int objectHandle);
    void removeObjectFromSelection(int objectHandle);
    void xorAddObjectToSelection(int objectHandle);
    void removeFromSelectionAllExceptModelBase(bool keepObjectsSelectedThatAreNotBuiltOnAModelBase);

protected:
    void _handleOrderIndexOfOrphans();

    // Overridden from _CSceneObjectContainer_:
    void _addObject(CSceneObject* object);
    void _removeObject(CSceneObject* object);

private:
    CShape* _readSimpleXmlShape(CSer& ar,C7Vector& desiredLocalFrame);
    CShape* _createSimpleXmlShape(CSer& ar,bool noHeightfield,const char* itemType,bool checkSibling);
    void _writeSimpleXmlShape(CSer& ar,CShape* shape);
    void _writeSimpleXmlSimpleShape(CSer& ar,const char* originalShapeName,CShape* shape,const C7Vector& frame);

    bool _objectActualizationEnabled;
    int _nextObjectHandle;

};


