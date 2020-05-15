#pragma once

#include "_worldContainer_.h"

class CWorldContainer : public _CWorldContainer_
{
public:
    CWorldContainer();
    virtual ~CWorldContainer();

    // Overridden from _CWorldContainer_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);
};
