#pragma once

#include "ikGroup_old.h"
#include "syncObject.h"

enum {
    sim_syncobj_ikgroupcont_shift=0,
};

class _CIkGroupContainer_ : public CSyncObject
{
public:
    _CIkGroupContainer_();
    virtual ~_CIkGroupContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    virtual bool shiftIkGroup(int ikGroupHandle,bool shiftUp);

    size_t getObjectCount() const;
    CIkGroup_old* getObjectFromHandle(int ikGroupHandle) const;
    CIkGroup_old* getObjectFromIndex(size_t index) const;
    CIkGroup_old* getObjectFromName(const char* ikGroupName) const;

protected:
    virtual void _addIkGroup(CIkGroup_old* anIkGroup);
    virtual void _removeIkGroup(int ikGroupHandle);

    virtual void _shiftIkGroup_send(int ikGroupHandle,bool shiftUp);

private:
    std::vector<CIkGroup_old*> _ikGroups;
};
