#pragma once

#include "ser.h"
#include "3Vector.h"
#include "7Vector.h"
#include "sceneObject.h"

enum { /* Mujoco dummy float params */
    simi_mujoco_dummy_range1=0,
    simi_mujoco_dummy_range2,
    simi_mujoco_dummy_solreflimit1,
    simi_mujoco_dummy_solreflimit2,
    simi_mujoco_dummy_solimplimit1,
    simi_mujoco_dummy_solimplimit2,
    simi_mujoco_dummy_solimplimit3,
    simi_mujoco_dummy_solimplimit4,
    simi_mujoco_dummy_solimplimit5,
    simi_mujoco_dummy_margin,
    simi_mujoco_dummy_springlength,
    simi_mujoco_dummy_stiffness,
    simi_mujoco_dummy_damping,
};

enum { /* Mujoco dummy int params */
    simi_mujoco_dummy_bitcoded=0,
    simi_mujoco_dummy_proxyjointid,
};

enum { /* Mujoco dummy bool params */
    simi_mujoco_dummy_limited=1,
};


class CDummy : public CSceneObject
{
public:

    CDummy();
    virtual ~CDummy();

    // Overridden from CSyncObject
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(floatDouble scalingFactor);
    void scaleObjectNonIsometrically(floatDouble x,floatDouble y,floatDouble z);
    void serialize(CSer& ar);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);

    bool getFreeOnPathTrajectory() const;
    floatDouble getVirtualDistanceOffsetOnPath() const;
    floatDouble getVirtualDistanceOffsetOnPath_variationWhenCopy() const;
    std::string getLinkedDummyLoadAlias() const;
    std::string getLinkedDummyLoadName_old() const;
    floatDouble getDummySize() const;
    bool getAssignedToParentPath() const;
    bool getAssignedToParentPathOrientation() const;
    int getLinkedDummyHandle() const;
    int getLinkType() const;

    CColorObject* getDummyColor();
    void loadUnknownObjectType(CSer& ar);

    bool setAssignedToParentPath(bool assigned);
    bool setAssignedToParentPathOrientation(bool assigned);
    void setLinkedDummyHandle(int handle,bool check);
    bool setLinkType(int lt,bool check);
    void setDummySize(floatDouble s);

    void setFreeOnPathTrajectory(bool isFree);
    void setVirtualDistanceOffsetOnPath(floatDouble off);
    void setVirtualDistanceOffsetOnPath_variationWhenCopy(floatDouble off);

    floatDouble getEngineFloatParam(int what,bool* ok) const;
    int getEngineIntParam(int what,bool* ok) const;
    bool getEngineBoolParam(int what,bool* ok) const;
    bool setEngineFloatParam(int what,floatDouble v);
    bool setEngineIntParam(int what,int v);
    bool setEngineBoolParam(int what,bool v);

    void copyEnginePropertiesTo(CDummy* target);

protected:
    void getMujocoFloatParams(std::vector<floatDouble>& p) const;
    void getMujocoIntParams(std::vector<int>& p) const;
    void setMujocoFloatParams(const std::vector<floatDouble>& p,bool reflectToLinkedDummy=true);
    void setMujocoIntParams(const std::vector<int>& p,bool reflectToLinkedDummy=true);

    void _reflectPropToLinkedDummy() const;
    void _setLinkedDummyHandle_sendOldIk(int h) const;
    void _setLinkType_sendOldIk(int t) const;

    bool _freeOnPathTrajectory;
    floatDouble _virtualDistanceOffsetOnPath;
    floatDouble _virtualDistanceOffsetOnPath_variationWhenCopy;
    std::string _linkedDummyLoadAlias;
    std::string _linkedDummyLoadName_old;

    CColorObject _dummyColor;
    floatDouble _dummySize;
    int _linkedDummyHandle;
    int _linkType;
    bool _assignedToParentPath;
    bool _assignedToParentPathOrientation;

    std::vector<floatDouble> _mujocoFloatParams;
    std::vector<int> _mujocoIntParams;
};
