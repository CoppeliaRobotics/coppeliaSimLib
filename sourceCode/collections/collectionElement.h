#pragma once

#include "ser.h"

class CCollectionElement
{
public:
    CCollectionElement(int mainObjectHandle,int collectionType,bool add);
    virtual ~CCollectionElement();

    std::string getMainObjectTempName() const;

    void addOrRemoveYourObjects(std::vector<int>* theObjects) const;
    void addOrRemoveAnObject(std::vector<int>* theObjects,int objectHandle) const;
    bool isSame(CCollectionElement* it) const;
    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::map<int,int>* map);
    CCollectionElement* copyYourself() const;


    int getElementHandle() const;
    int getMainObject() const;
    bool getIsAdditive() const;
    int getElementType() const;

    void setElementHandle(int newHandle);
    void setMainObject(int mo);

private:
    std::string _objectTempName;
    int _elementHandle;
    int _elementType;
    int _mainObjectHandle;
    bool _additive;
};
