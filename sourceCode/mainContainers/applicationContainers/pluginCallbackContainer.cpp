#include <pluginCallbackContainer.h>

CPluginCallbackContainer::CPluginCallbackContainer()
{
}

CPluginCallbackContainer::~CPluginCallbackContainer()
{
    clear();
}

SPluginCallback* CPluginCallbackContainer::getCallbackFromName(const char* name)
{
    SPluginCallback* retVal = nullptr;
    auto it = _allCallbacks_map.find(name);
    if (it != _allCallbacks_map.end())
        retVal = &_allCallbacks_vect[it->second];
    return (retVal);
}

SPluginCallback* CPluginCallbackContainer::getCallbackFromIndex(size_t ind)
{
    SPluginCallback* retVal = nullptr;
    if (ind < _allCallbacks_vect.size())
        retVal = &_allCallbacks_vect[ind];
    return (retVal);
}

void CPluginCallbackContainer::clear()
{
    _allCallbacks_vect.clear();
    _allCallbacks_map.clear();
}

bool CPluginCallbackContainer::addCallback(const char* funcName, void (*callback)(struct SScriptCallBack* cb))
{
    bool retVal = false;
    SPluginCallback* it = getCallbackFromName(funcName);
    if (it == nullptr)
    {
        retVal = true;
        SPluginCallback s;
        s.funcName = funcName;
        s.callback = callback;
        _allCallbacks_map[funcName] = _allCallbacks_vect.size();
        _allCallbacks_vect.push_back(s);
    }
    else
        it->callback = callback; // callback already exists. Replace it
    return (retVal);
}
