#pragma once

#include "collection.h"
#include "syncObject.h"

class _CCollectionContainer_ : public CSyncObject
{
public:
    _CCollectionContainer_();
    virtual ~_CCollectionContainer_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    size_t getObjectCount() const;
    CCollection* getObjectFromIndex(size_t index) const;
    CCollection* getObjectFromHandle(int collectionHandle) const;
    CCollection* getObjectFromName(const char* collectionName) const;

protected:
    virtual void _addCollection(CCollection* collection);
    virtual void _removeCollection(int collectionHandle);

private:
    std::vector<CCollection*> _allCollections;
};
