#pragma once

#include "colorObject.h"
#include "ser.h"

class CCollisionObject_old
{
public:
    CCollisionObject_old();
    CCollisionObject_old(int entity1Handle,int entity2Handle);
    virtual ~CCollisionObject_old();

    void displayCollisionContour();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool isSame(int entity1Handle,int entity2Handle) const;
    void clearCollisionResult();
    bool handleCollision();
    int readCollision(int collObjHandles[2]) const;
    bool getCollisionResult() const;
    bool isCollisionResultValid() const;
    float getCalculationTime() const;
    std::string getUniquePersistentIdString() const;
    void _clearCollisionResult();
    void _setCollisionResult(bool result,int calcTime,int obj1Handle,int obj2Handle,const std::vector<float>& intersect);

    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    void performCollectionLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    bool announceCollectionWillBeErased(int collectionHandle,bool copyBuffer);
    CCollisionObject_old* copyYourself();
    int getCollisionColor(int entityHandle) const;
    bool canComputeCollisionContour() const;
    std::string getObjectDescriptiveName() const;

    int getObjectHandle() const;
    int getEntity1Handle() const;
    int getEntity2Handle() const;
    std::string getObjectName() const;
    bool getColliderChangesColor() const;
    bool getCollideeChangesColor() const;
    bool getExhaustiveDetection() const;
    bool getExplicitHandling() const;
    int getContourWidth() const;
    const std::vector<float>* getIntersections() const;

    CColorObject* getContourColor();

    bool setObjectHandle(int newHandle);
    bool setObjectName(const char* newName,bool check);
    bool setColliderChangesColor(bool changes);
    bool setCollideeChangesColor(bool changes);
    bool setExhaustiveDetection(bool exhaustive);
    bool setExplicitHandling(bool explicitHandl);
    bool setContourWidth(int w);
    bool setIntersections(const std::vector<float>* intersections=nullptr);

private:
    void _commonInit();

    std::string _uniquePersistentIdString;

    bool _collisionResult;
    bool _collisionResultValid;
    int _collObjectHandles[2];
    int _calcTimeInMs;
    bool _initialValuesInitialized;
    bool _initialExplicitHandling;

    int _objectHandle;
    int _entity1Handle;
    int _entity2Handle;
    std::string _objectName;
    int _countourWidth;
    bool _colliderChangesColor;
    bool _collideeChangesColor;
    bool _detectAllCollisions;
    bool _explicitHandling;

    CColorObject _contourColor;
    std::vector<float> _intersections;
};
