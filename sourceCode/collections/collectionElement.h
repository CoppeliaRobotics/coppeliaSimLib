#pragma once

#include "ser.h"
#include "_collectionElement_.h"

class CCollectionElement : public _CCollectionElement_
{
public:
    CCollectionElement(int mainObjectHandle,int collectionType,bool add);
    virtual ~CCollectionElement();

    // Overridden from CSyncObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();
    void removeSynchronizationObject();

    std::string getMainObjectTempName() const;

    void addOrRemoveYourObjects(std::vector<int>* theObjects) const;
    void addOrRemoveAnObject(std::vector<int>* theObjects,int objectHandle) const;
    bool isSame(CCollectionElement* it) const;
    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    CCollectionElement* copyYourself() const;

private:
    std::string _objectTempName;
};
