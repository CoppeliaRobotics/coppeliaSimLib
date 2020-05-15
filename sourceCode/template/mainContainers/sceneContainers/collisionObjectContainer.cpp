#include "collisionObjectContainer.h"
#include "app.h"

CCollisionObjectContainer::CCollisionObjectContainer()
{
    simAddLog("Sync",sim_verbosity_debug,"collision object container created.");
}

CCollisionObjectContainer::~CCollisionObjectContainer()
{
    simAddLog("Sync",sim_verbosity_debug,"collision object container destroyed.");
}

