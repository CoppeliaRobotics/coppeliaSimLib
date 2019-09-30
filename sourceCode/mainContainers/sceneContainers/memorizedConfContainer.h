
#pragma once

#include "memorizedConf.h"
#include "mainCont.h"

class CMemorizedConfContainer : public CMainCont 
{
public:
    CMemorizedConfContainer();
    virtual ~CMemorizedConfContainer();
    void memorize();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
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
