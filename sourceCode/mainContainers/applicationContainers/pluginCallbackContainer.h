#pragma once

#include <simLib/simTypes.h>
#include <string>
#include <vector>
#include <map>

struct SPluginCallback
{
    std::string funcName;
    void(*callback)(struct SScriptCallBack* p);
};

class CPluginCallbackContainer
{
public:
    CPluginCallbackContainer();
    virtual ~CPluginCallbackContainer();

    SPluginCallback* getCallbackFromName(const char* name);
    SPluginCallback* getCallbackFromIndex(size_t ind);

    void clear();
    bool addCallback(const char* funcName,void(*callback)(struct SScriptCallBack* cb));

protected:
    std::vector<SPluginCallback> _allCallbacks_vect;
    std::map<std::string,uint16_t> _allCallbacks_map;
};
