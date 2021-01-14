#pragma once

#include "distanceObject_old.h"
#include "syncObject.h"

class _CDistanceObjectContainer_old : public CSyncObject
{
public:
    _CDistanceObjectContainer_old();
    virtual ~_CDistanceObjectContainer_old();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    size_t getObjectCount() const;
    CDistanceObject_old* getObjectFromIndex(size_t index) const;
    CDistanceObject_old* getObjectFromHandle(int objectHandle) const;
    CDistanceObject_old* getObjectFromName(const char* objName) const;

protected:
    virtual void _addObject(CDistanceObject_old* newDistObj);
    virtual void _removeObject(int objectHandle);

private:
    std::vector<CDistanceObject_old*> _distanceObjects;
};

