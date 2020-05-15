#include "sceneObjectContainer.h"
#include "app.h"

CSceneObjectContainer::CSceneObjectContainer()
{
    simAddLog("Sync",sim_verbosity_debug,"scene object container created.");
}

CSceneObjectContainer::~CSceneObjectContainer()
{
    simAddLog("Sync",sim_verbosity_debug,"scene object container destroyed.");
}
