
#pragma once

#include "interfaceStack.h"

class CInterfaceStackContainer
{
public:
    CInterfaceStackContainer();
    virtual ~CInterfaceStackContainer();

    int addStack(CInterfaceStack* stack);
    bool destroyStack(int id);
    CInterfaceStack* getStack(int id);

protected:
    std::vector<CInterfaceStack*> _allStacks;
};
