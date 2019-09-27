
#pragma once

#include "vrepMainHeader.h"
#include "ikGroup.h"
#include "mainCont.h"

class CRegisterediks : public CMainCont 
{
public:
    CRegisterediks();
    virtual ~CRegisterediks();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    void addIkGroup(CikGroup* anIkGroup,bool objectIsACopy);
    void addIkGroupWithSuffixOffset(CikGroup* anIkGroup,bool objectIsACopy,int suffixOffset);
    void announceCollectionWillBeErased(int groupID);

    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);

    CikGroup* getIkGroup(int groupID);
    CikGroup* getIkGroup(std::string groupName);
    void removeIkGroup(int ikGroupID);
    void removeAllIkGroups();
    void announceObjectWillBeErased(int objID);
    void announceIkGroupWillBeErased(int ikGroupID);
    int computeAllIkGroups(bool exceptExplicitHandling);
    void resetCalculationResults();

    // Variable that need to be serialized on an individual basis:
    std::vector<CikGroup*> ikGroups;
};
