#pragma once

#include "collisionObject.h"
#include "syncObject.h"

class _CCollisionObjectContainer_ : public CSyncObject
{
public:
    _CCollisionObjectContainer_();
    virtual ~_CCollisionObjectContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    size_t getObjectCount() const;
    CCollisionObject* getObjectFromIndex(size_t index) const;
    CCollisionObject* getObjectFromHandle(int objectHandle) const;
    CCollisionObject* getObjectFromName(const char* objName) const;

protected:
    virtual void _addObject(CCollisionObject* newCollObj);
    virtual void _removeObject(int objectHandle);

private:
    std::vector<CCollisionObject*> _collisionObjects;
};
