#pragma once

#include "ser.h"
#include "3Vector.h"
#include "7Vector.h"
#include "_dummy_.h"

class CDummy : public _CDummy_
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
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);

    bool getFreeOnPathTrajectory() const;
    float getVirtualDistanceOffsetOnPath() const;
    float getVirtualDistanceOffsetOnPath_variationWhenCopy() const;
    std::string getLinkedDummyLoadAlias() const;
    std::string getLinkedDummyLoadName_old() const;
    C7Vector getTempLocalTransformation() const;

    void loadUnknownObjectType(CSer& ar);

    // Overridden from _CDummy_:
    bool setAssignedToParentPath(bool assigned);
    bool setAssignedToParentPathOrientation(bool assigned);
    bool setLinkedDummyHandle(int handle,bool check);
    bool setLinkType(int lt,bool check);

    void setFreeOnPathTrajectory(bool isFree);
    void setVirtualDistanceOffsetOnPath(float off);
    void setVirtualDistanceOffsetOnPath_variationWhenCopy(float off);

    void setTempLocalTransformation(const C7Vector& tr);

private:
    // Overridden from _CDummy_:
    void _setLinkedDummyHandle_send(int h) const;
    void _setLinkType_send(int t) const;

    bool _freeOnPathTrajectory;
    float _virtualDistanceOffsetOnPath;
    float _virtualDistanceOffsetOnPath_variationWhenCopy;
    C7Vector _localTransformation_temp; // used for IK (needed when the dummy is freely sliding on a path object!)
    std::string _linkedDummyLoadAlias;
    std::string _linkedDummyLoadName_old;
};
