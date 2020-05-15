#include "app.h"
#include "simLib.h"
#include "simConst.h"

CWorldContainer* App::worldContainer=nullptr;
CWorld* App::currentWorld=nullptr;

App::App()
{
}

App::~App()
{
}

void App::initialize()
{
    worldContainer=new CWorldContainer();
    currentWorld=worldContainer->currentWorld;
}

void App::deinitialize()
{
    delete worldContainer;
    worldContainer=nullptr;
    currentWorld=nullptr;
}

