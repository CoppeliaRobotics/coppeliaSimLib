
#pragma once

#include "vrepMainHeader.h"
#include "visualParam.h"

class CRegCollision
{
public:

    CRegCollision(int obj1ID,int obj2ID,std::string objName,int objID);
    virtual ~CRegCollision();

    void displayCollisionContour();
    void initializeInitialValues(bool simulationIsRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool isSame(int obj1ID,int obj2ID) const;
    int getObjectID() const;
    std::string getObjectName() const;
    std::string getObjectPartnersName() const;
    void setObjectName(std::string newName);
    void setObjectID(int newID);
    int getObject1ID() const;
    int getObject2ID() const;
    void clearCollisionResult();
    bool getCollisionResult() const;
    bool isCollisionResultValid() const;
    float getCalculationTime() const;
    bool handleCollision();
    int readCollision(int collObjHandles[2]) const;
    void serialize(CSer& ar);
    void performObjectLoadingMapping(std::vector<int>* map);
    void performCollectionLoadingMapping(std::vector<int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceCollectionWillBeErased(int groupID,bool copyBuffer);
    CRegCollision* copyYourself();
    void setColliderChangesColor(bool changes);
    bool getColliderChangesColor() const;
    void setCollideeChangesColor(bool changes);
    bool getCollideeChangesColor() const;
    void setExhaustiveDetection(bool exhaustive);
    bool getExhaustiveDetection() const;
    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling() const;
    int getCollisionColor(int entityID) const;
    bool canComputeCollisionContour() const;
    std::string getUniquePersistentIdString() const;

    void setContourWidth(int w);
    int getContourWidth() const;

    std::vector<float>* getIntersectionsPtr();

    CVisualParam contourColor;

protected:  
    // Variables which need to be serialized and copied:
    int object1ID;
    int object2ID;
    std::string objectName;
    std::string _uniquePersistentIdString;
    int objectID;
    int _countourWidth;
    // Various:
    bool collisionResult;
    bool _collisionResultValid;
    int _collObjectHandles[2];

    int _calcTimeInMs;

    bool colliderChangesColor;
    bool collideeChangesColor;
    bool detectAllCollisions;
    bool explicitHandling;
    std::vector<float> intersections;

    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
};
