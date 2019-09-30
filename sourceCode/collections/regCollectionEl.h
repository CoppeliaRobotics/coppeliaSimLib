#pragma once

#include "ser.h"

class CRegCollectionEl  
{
public:
    CRegCollectionEl(int objID,int grType,bool add);
    virtual ~CRegCollectionEl();

    void addOrRemoveYourObjects(std::vector<int>* theObjects);
    void addOrRemoveAnObject(std::vector<int>* theObjects,int objectID);
    bool isAdditive();
    int getMainObject();
    int getCollectionType();
    int getSubCollectionID();
    void setSubCollectionID(int newID);
    bool isSame(CRegCollectionEl* it);
    void serialize(CSer& ar);
    void performObjectLoadingMapping(std::vector<int>* map);
    bool announceObjectWillBeErased(int objID);
    CRegCollectionEl* copyYourself();

private:
    // Variables which need to be serialized and copied
    int objectID;
    int groupType;
    bool additive;
    int subGroupID;
};
