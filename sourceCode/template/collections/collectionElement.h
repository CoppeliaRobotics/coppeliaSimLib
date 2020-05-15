#pragma once

#include "_collectionElement_.h"

class CCollectionElement : public _CCollectionElement_
{
public:
    CCollectionElement(int mainObjectHandle,int collectionType,bool add);
    virtual ~CCollectionElement();

private:
    static int cnt;
};
