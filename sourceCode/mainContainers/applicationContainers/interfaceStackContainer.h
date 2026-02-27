#pragma once

#include <interfaceStack.h>

class CInterfaceStackContainer
{
  public:
    CInterfaceStackContainer();
    virtual ~CInterfaceStackContainer();

    CInterfaceStack* createStack();
    CInterfaceStack* createStackCopy(const CInterfaceStack* original);
    bool destroyStack(int id);
    bool destroyStack(CInterfaceStack* stack);
    CInterfaceStack* getStack(int id);

  protected:
    std::vector<CInterfaceStack*> _allStacks;
};
