
#pragma once

#include "vrepMainHeader.h"
#include "visualParam.h"

class CRegDist  
{
public:
    CRegDist(int obj1ID,int obj2ID,std::string objName,int objID);
    virtual ~CRegDist();

    void displayDistanceSegment();
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
    void clearDistanceResult();
    bool getDistanceResult(float dist[7]) const;
    float getCalculationTime() const;
    float handleDistance();
    float readDistance() const;

    void serialize(CSer& ar);
    void performObjectLoadingMapping(std::vector<int>* map);
    void performCollectionLoadingMapping(std::vector<int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void setTreshhold(float tr);
    float getTreshhold() const;
    void setTreshholdActive(bool active);
    bool getTreshholdActive() const;
    CRegDist* copyYourself();
    void setDisplaySegment(bool display);
    bool getDisplaySegment() const;
    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling() const;
    std::string getUniquePersistentIdString() const;

    void setSegmentWidth(int w);
    int getSegmentWidth() const;

    CVisualParam segmentColor;

protected:  
    // Variables which need to be serialized and copied:
    int object1ID;
    int object2ID;
    std::string objectName;
    std::string _uniquePersistentIdString;
    int objectID;
    int _segmentWidth;
    float treshhold;
    bool treshholdActive;

    // Various:
    float distanceResult[7];
    float _distance;
    int distanceBuffer[4];
    bool distanceIsValid;
    int _calcTimeInMs;
    bool displaySegment;
    bool explicitHandling;

    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
};
