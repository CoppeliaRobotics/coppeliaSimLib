
#include <global.h>
#include <interfaceStackContainer.h>

CInterfaceStackContainer::CInterfaceStackContainer()
{
}

CInterfaceStackContainer::~CInterfaceStackContainer()
{
    for (size_t i=0;i<_allStacks.size();i++)
        delete _allStacks[i];
}

CInterfaceStack* CInterfaceStackContainer::createStack()
{
    CInterfaceStack* stack=new CInterfaceStack(1,1,"");
    int id=SIM_IDSTART_INTERFACESTACK;
    while (getStack(id)!=nullptr)
        id++;
    _allStacks.push_back(stack);
    stack->setId(id);
    return(stack);
}

CInterfaceStack* CInterfaceStackContainer::createStackCopy(CInterfaceStack* original)
{
    int id=SIM_IDSTART_INTERFACESTACK;
    while (getStack(id)!=nullptr)
        id++;
    CInterfaceStack* copy=original->copyYourself();
    _allStacks.push_back(copy);
    copy->setId(id);
    return(copy);
}

bool CInterfaceStackContainer::destroyStack(CInterfaceStack* stack)
{
    for (size_t i=0;i<_allStacks.size();i++)
    {
        if (_allStacks[i]==stack)
        {
            delete stack;
            _allStacks.erase(_allStacks.begin()+i);
            return(true);
        }
    }
    return(false);
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
