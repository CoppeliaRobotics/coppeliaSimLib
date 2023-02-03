#pragma once

#include <world.h>
#include <syncObject.h>

class _CWorldContainer_ : public CSyncObject
{
public:
    _CWorldContainer_();
    virtual ~_CWorldContainer_();

    CWorld* currentWorld;
};
