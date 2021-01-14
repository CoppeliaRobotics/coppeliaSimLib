#pragma once

#include "_distanceObject_old.h"

class CDistanceObject_old : public _CDistanceObject_old
{
public:
    CDistanceObject_old();
    CDistanceObject_old(int entity1Handle,int entity2Handle);
    virtual ~CDistanceObject_old();

    // Overridden from CSyncObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();
    void removeSynchronizationObject(bool localReferencesToItOnly);

    void displayDistanceSegment();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool isSame(int entity1Handle,int entity2Handle) const;
    bool getDistanceResult(float dist[7]) const;
    float getCalculationTime() const;
    float readDistance() const;
    std::string getUniquePersistentIdString() const;
    std::string getObjectDescriptiveName() const;
    void clearDistanceResult();
    float handleDistance();

    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    void performCollectionLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    bool announceCollectionWillBeErased(int collectionHandle,bool copyBuffer);
    CDistanceObject_old* copyYourself();

    // Overridden from _CDistanceObject_:
    bool setObjectName(const char* newName,bool check);
    bool setThreshold(float tr);
    bool setSegmentWidth(int w);

private:
    void _commonInit();

    std::string _uniquePersistentIdString;
    float _distanceResult[7];
    float _distance;
    int _distanceBuffer[4];
    bool _distanceIsValid;
    int _calcTimeInMs;
    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
};
