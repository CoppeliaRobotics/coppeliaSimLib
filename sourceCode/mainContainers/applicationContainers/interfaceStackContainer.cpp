
#include "vrepMainHeader.h"
#include "global.h"
#include "interfaceStackContainer.h"

CInterfaceStackContainer::CInterfaceStackContainer()
{
}

CInterfaceStackContainer::~CInterfaceStackContainer()
{
    for (size_t i=0;i<_allStacks.size();i++)
        delete _allStacks[i];
}

int CInterfaceStackContainer::addStack(CInterfaceStack* stack)
{
    int id=SIM_IDSTART_INTERFACESTACK;
    while (getStack(id)!=nullptr)
        id++;
    _allStacks.push_back(stack);
    stack->setId(id);
    return(id);
}

bool CInterfaceStackContainer::destroyStack(int id)
{
    for (size_t i=0;i<_allStacks.size();i++)
    {
        if (_allStacks[i]->getId()==id)
        {
            delete _allStacks[i];
            _allStacks.erase(_allStacks.begin()+i);
            return(true);
        }
    }
    return(false);
}

CInterfaceStack* CInterfaceStackContainer::getStack(int id)
{
    for (size_t i=0;i<_allStacks.size();i++)
    {
        if (_allStacks[i]->getId()==id)
            return(_allStacks[i]);
    }
    return(nullptr);
}
