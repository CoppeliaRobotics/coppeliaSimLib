#pragma once

#include <simLib/simTypes.h>
#include <string>
#include <vector>
#include <map>

struct SPluginCallback
{
    std::string funcName;
    std::string calltips;
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
    bool addCallback(const char* funcName,const char* calltips,void(*callback)(struct SScriptCallBack* cb));

protected:
    std::vector<SPluginCallback> _allCallbacks_vect;
    std::map<std::string,SPluginCallback*> _allCallbacks_map;
};
