#pragma once

#include "collisionObject_old.h"
#include "syncObject.h"

class _CCollisionObjectContainer_old : public CSyncObject
{
public:
    _CCollisionObjectContainer_old();
    virtual ~_CCollisionObjectContainer_old();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    size_t getObjectCount() const;
    CCollisionObject_old* getObjectFromIndex(size_t index) const;
    CCollisionObject_old* getObjectFromHandle(int objectHandle) const;
    CCollisionObject_old* getObjectFromName(const char* objName) const;

protected:
    virtual void _addObject(CCollisionObject_old* newCollObj);
    virtual void _removeObject(int objectHandle);

private:
    std::vector<CCollisionObject_old*> _collisionObjects;
};
