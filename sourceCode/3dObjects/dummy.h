
#pragma once

#include "vrepMainHeader.h"
#include "ser.h"
#include "3DObject.h"
#include "3Vector.h"
#include "7Vector.h"

class CDummy : public C3DObject  
{
public:

    CDummy();
    virtual ~CDummy();

    // Following functions are inherited from 3DObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    C3DObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar);
    static void serializeWExtIkStatic(CExtIkSer& ar);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer);
    void performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performGcsLoadingMapping(std::vector<int>* map);
    void performTextureObjectLoadingMapping(std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(std::vector<int>* map);
    void bufferMainDisplayStateVariables();
    void bufferedMainDisplayStateVariablesToDisplay();
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationIsRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    bool isPotentiallyCuttable() const;
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel);

    // SPECIAL FUNCTION TO GUARANTEE FORWARD COMPATIBILITY WHEN LOADING OBJECT TYPES THAT DON'T EXIST YET!
    void loadUnknownObjectType(CSer& ar);

    // Various functions
    void setSize(float theNewSize);
    float getSize() const;

    void setAssignedToParentPath(bool assigned);
    bool getAssignedToParentPath() const;
    void setAssignedToParentPathOrientation(bool assigned);
    bool getAssignedToParentPathOrientation() const;
    void setFreeOnPathTrajectory(bool isFree);
    bool getFreeOnPathTrajectory() const;
    void setVirtualDistanceOffsetOnPath(float off);
    float getVirtualDistanceOffsetOnPath() const;

    void setVirtualDistanceOffsetOnPath_variationWhenCopy(float off);
    float getVirtualDistanceOffsetOnPath_variationWhenCopy() const;

    void setLinkedDummyID(int id,bool setDirectly);
    void setLinkType(int lt,bool setDirectly);
    int getLinkedDummyID() const;
    int getLinkType() const;
    C7Vector getTempLocalTransformation() const;
    void setTempLocalTransformation(const C7Vector& tr);

    CVisualParam* getColor();

protected:
    CVisualParam color;
    float size;
    int _linkedDummyID;
    int _linkType;
    bool _assignedToParentPath;
    bool _assignedToParentPathOrientation;
    bool _freeOnPathTrajectory;
    float _virtualDistanceOffsetOnPath;
    float _virtualDistanceOffsetOnPath_variationWhenCopy;
    C7Vector _localTransformation_temp; // used for IK (needed when the dummy is freely sliding on a path object!)
};
