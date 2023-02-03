#include <_world_.h>
#include <simConst.h>

_CWorld_::_CWorld_()
{
    collections=nullptr;
    distances=nullptr;
    collisions=nullptr;
    ikGroups=nullptr;
    sceneObjects=nullptr;

    _worldHandle=-1;

    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_world;
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CWorld_::~_CWorld_()
{
}

void _CWorld_::setWorldHandle(int handle)
{
    _worldHandle=handle;
}

int _CWorld_::getWorldHandle() const
{
    return(_worldHandle);
}

void _CWorld_::initializeWorld()
{
    ikGroups=new CIkGroupContainer();
    collections=new CCollectionContainer();
    distances=new CDistanceObjectContainer_old();
    collisions=new CCollisionObjectContainer_old();
    sceneObjects=new CSceneObjectContainer();
}

void _CWorld_::deleteWorld()
{
    delete sceneObjects;
    sceneObjects=nullptr;
    delete collisions;
    collisions=nullptr;
    delete distances;
    distances=nullptr;
    delete collections;
    collections=nullptr;
    delete ikGroups;
    ikGroups=nullptr;
}
