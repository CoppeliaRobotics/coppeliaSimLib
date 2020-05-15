#pragma once

#include "worldContainer.h"
#include "simConst.h"
#include "simLib.h"

class App
{ // static class
public:
    App();
    virtual ~App();

    static void initialize();
    static void deinitialize();

    static CWorldContainer* worldContainer;
    static CWorld* currentWorld; // actually worldContainer->currentWorld
};
