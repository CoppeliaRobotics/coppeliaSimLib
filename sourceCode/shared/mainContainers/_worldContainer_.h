#pragma once

#include "world.h"
#include "syncObject.h"

class _CWorldContainer_ : public CSyncObject
{
public:
    _CWorldContainer_();
    virtual ~_CWorldContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    CWorld* currentWorld;
};
