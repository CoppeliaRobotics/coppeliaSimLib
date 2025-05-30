#pragma once

#include <ikGroup_old.h>

class _CIkGroupContainer_
{
  public:
    _CIkGroupContainer_();
    virtual ~_CIkGroupContainer_();

    virtual bool shiftIkGroup(int ikGroupHandle, bool shiftUp);

    size_t getObjectCount() const;
    CIkGroup_old* getObjectFromHandle(int ikGroupHandle) const;
    CIkGroup_old* getObjectFromIndex(size_t index) const;
    CIkGroup_old* getObjectFromName(const char* ikGroupName) const;

  protected:
    virtual void _addIkGroup(CIkGroup_old* anIkGroup);
    virtual void _removeIkGroup(int ikGroupHandle);

    virtual void _shiftIkGroup_send(int ikGroupHandle, bool shiftUp);

  private:
    std::vector<CIkGroup_old*> _ikGroups;
};
