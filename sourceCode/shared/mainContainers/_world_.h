#pragma once

#include "collectionContainer.h"
#include "distanceObjectContainer.h"
#include "collisionObjectContainer.h"
#include "ikGroupContainer.h"
#include "sceneObjectContainer.h"

#include "syncObject.h"

enum {
    sim_syncobj_world_empty=0,
};

class _CWorld_ : public CSyncObject
{
public:
    _CWorld_();
    virtual ~_CWorld_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    int getWorldHandle() const;

    virtual void setWorldHandle(int handle);

    virtual void initializeWorld();
    virtual void deleteWorld();

    CCollectionContainer* collections;
    CDistanceObjectContainer* distances;
    CCollisionObjectContainer* collisions;
    CIkGroupContainer* ikGroups;
    CSceneObjectContainer* sceneObjects;

private:
    int _worldHandle;
};
