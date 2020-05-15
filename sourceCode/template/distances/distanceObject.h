#pragma once

#include "_distanceObject_.h"

class CDistanceObject : public _CDistanceObject_
{
public:
    CDistanceObject(int entity1Handle,int entity2Handle);
    virtual ~CDistanceObject();

private:
    static int cnt;
};
