
#pragma once

#include "regDist.h"
#include "mainCont.h"

class CRegisteredDistances : public CMainCont 
{
public:
    CRegisteredDistances();
    virtual ~CRegisteredDistances();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void displayDistanceSegments();
    int addNewObject(int obj1ID,int obj2ID,std::string objName);
    void addObject(CRegDist* newDistObj,bool objectIsACopy);
    void addObjectWithSuffixOffset(CRegDist* newDistObj,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    bool removeObject(int objID);
    CRegDist* getObject(int objID);
    CRegDist* getObject(std::string objName);
    void removeAllDistanceObjects();
    void resetAllDistances(bool exceptExplicitHandling);
    void announceObjectWillBeErased(int objID);
    void announceCollectionWillBeErased(int groupID);
    void setUpDefaultValues();
    float handleAllDistances(bool exceptExplicitHandling);
    bool setObjectName(int objID,std::string newName);

    // Variable that need to be serialized on an individual basis:
    std::vector<CRegDist*> distanceObjects;
};

