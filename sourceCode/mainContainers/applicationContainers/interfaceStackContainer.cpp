#include <global.h>
#include <interfaceStackContainer.h>

CInterfaceStackContainer::CInterfaceStackContainer()
{
}

CInterfaceStackContainer::~CInterfaceStackContainer()
{
    for (size_t i = 0; i < _allStacks.size(); i++)
        delete _allStacks[i];
}

CInterfaceStack* CInterfaceStackContainer::createStack()
{
    CInterfaceStack* stack = new CInterfaceStack(1, 1, "");
    int id = sim_object_stackstart;
    while (getStack(id) != nullptr)
        id++;
    _allStacks.push_back(stack);
    stack->setId(id);
    return (stack);
}

CInterfaceStack* CInterfaceStackContainer::createStackCopy(const CInterfaceStack* original)
{
    int id = sim_object_stackstart;
    while (getStack(id) != nullptr)
        id++;
    CInterfaceStack* copy = original->copyYourself();
    _allStacks.push_back(copy);
    copy->setId(id);
    return (copy);
}

bool CInterfaceStackContainer::destroyStack(CInterfaceStack* stack)
{
    for (size_t i = 0; i < _allStacks.size(); i++)
    {
        if (_allStacks[i] == stack)
        {
            delete stack;
            _allStacks.erase(_allStacks.begin() + i);
            return (true);
        }
    }
    return (false);
}

bool CInterfaceStackContainer::destroyStack(int id)
{
    for (size_t i = 0; i < _allStacks.size(); i++)
    {
        if (_allStacks[i]->getObjectHandle() == id)
        {
            delete _allStacks[i];
            _allStacks.erase(_allStacks.begin() + i);
            return (true);
        }
    }
    return (false);
}

CInterfaceStack* CInterfaceStackContainer::getStack(int id) const
{
    for (size_t i = 0; i < _allStacks.size(); i++)
    {
        if (_allStacks[i]->getObjectHandle() == id)
            return (_allStacks[i]);
    }
    return nullptr;
}

int CInterfaceStackContainer::setBoolProperty_t(long long int target, const char* ppName, bool pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setBoolProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getBoolProperty_t(long long int target, const char* ppName, bool& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getBoolProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setIntProperty_t(long long int target, const char* ppName, int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setIntProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getIntProperty_t(long long int target, const char* ppName, int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getIntProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setLongProperty_t(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setLongProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getLongProperty_t(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getLongProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setHandleProperty_t(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setHandleProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getHandleProperty_t(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getHandleProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setFloatProperty_t(long long int target, const char* ppName, double pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setFloatProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getFloatProperty_t(long long int target, const char* ppName, double& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getFloatProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setStringProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setStringProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getStringProperty_t(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getStringProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setBufferProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setBufferProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getBufferProperty_t(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getBufferProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setIntArray2Property_t(long long int target, const char* ppName, const int* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setIntArray2Property(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getIntArray2Property_t(long long int target, const char* ppName, int* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getIntArray2Property(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setVector3Property_t(long long int target, const char* ppName, const C3Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setVector3Property(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getVector3Property_t(long long int target, const char* ppName, C3Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getVector3Property(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setMatrixProperty_t(long long int target, const char* ppName, const CMatrix& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setMatrixProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getMatrixProperty_t(long long int target, const char* ppName, CMatrix& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getMatrixProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setQuaternionProperty_t(long long int target, const char* ppName, const C4Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setQuaternionProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getQuaternionProperty_t(long long int target, const char* ppName, C4Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getQuaternionProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setPoseProperty_t(long long int target, const char* ppName, const C7Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setPoseProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getPoseProperty_t(long long int target, const char* ppName, C7Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getPoseProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setColorProperty_t(long long int target, const char* ppName, const float* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setColorProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getColorProperty_t(long long int target, const char* ppName, float* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getColorProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setFloatArrayProperty_t(long long int target, const char* ppName, const std::vector<double>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setFloatArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getFloatArrayProperty_t(long long int target, const char* ppName, std::vector<double>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getFloatArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setIntArrayProperty_t(long long int target, const char* ppName, const std::vector<int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setIntArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getIntArrayProperty_t(long long int target, const char* ppName, std::vector<int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getIntArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setHandleArrayProperty_t(long long int target, const char* ppName, const std::vector<long long int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setHandleArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getHandleArrayProperty_t(long long int target, const char* ppName, std::vector<long long int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getHandleArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::setStringArrayProperty_t(long long int target, const char* ppName, const std::vector<std::string>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->setStringArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::getStringArrayProperty_t(long long int target, const char* ppName, std::vector<std::string>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getStringArrayProperty(ppName, pState);
    return retVal;
}

int CInterfaceStackContainer::removeProperty_t(long long int target, const char* ppName)
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->removeProperty(ppName);
    return retVal;
}

int CInterfaceStackContainer::getPropertyName_t(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getPropertyName(index, pName, appartenance, excludeFlags);
    return retVal;
}

int CInterfaceStackContainer::getPropertyInfo_t(long long int target, const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = sim_propertyret_unknowntarget;
    CInterfaceStack* stack = getStack(target);
    if (stack != nullptr)
        retVal = stack->getPropertyInfo(ppName, info, infoTxt);
    return retVal;
}
