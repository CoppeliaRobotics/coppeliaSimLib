#pragma once

#include "distanceObject.h"
#include "syncObject.h"

class _CDistanceObjectContainer_ : public CSyncObject
{
public:
    _CDistanceObjectContainer_();
    virtual ~_CDistanceObjectContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    size_t getObjectCount() const;
    CDistanceObject* getObjectFromIndex(size_t index) const;
    CDistanceObject* getObjectFromHandle(int objectHandle) const;
    CDistanceObject* getObjectFromName(const char* objName) const;

protected:
    virtual void _addObject(CDistanceObject* newDistObj);
    virtual void _removeObject(int objectHandle);

private:
    std::vector<CDistanceObject*> _distanceObjects;
};

