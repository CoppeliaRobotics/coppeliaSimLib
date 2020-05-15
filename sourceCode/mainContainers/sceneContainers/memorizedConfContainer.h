#pragma once

#include "memorizedConf.h"

class CMemorizedConfContainer 
{
public:
    CMemorizedConfContainer();
    virtual ~CMemorizedConfContainer();
    void memorize();
    void restoreMemorized();
    void removeMemorized();
    bool isBufferEmpty();

    char* getConfigurationTree(int objID);
    void setConfigurationTree(const char* data);

    char* getObjectConfiguration(int objID);
    void setObjectConfiguration(const char* data);

private:
    std::vector<CMemorizedConf*> configurations;
};
