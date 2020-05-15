#pragma once

#include "collectionElement.h"
#include "syncObject.h"
#include <vector>

enum {
    sim_syncobj_collection_create=0,
    sim_syncobj_collection_delete,
    sim_syncobj_collection_setobjectname,
    sim_syncobj_collection_setoverrideproperties,
};

class _CCollection_ : public CSyncObject
{
public:
    _CCollection_();
    virtual ~_CCollection_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    size_t getElementCount() const;
    CCollectionElement* getElementFromIndex(size_t index) const;
    CCollectionElement* getElementFromHandle(int collectionElementHandle) const;

    int getCollectionHandle() const;
    std::string getCollectionName() const;
    bool getOverridesObjectMainProperties() const;

    virtual bool setCollectionHandle(int newHandle);

    virtual bool setCollectionName(const char* newName,bool check);
    virtual bool setOverridesObjectMainProperties(bool o);

protected:
    virtual void _addCollectionElement(CCollectionElement* collectionElement);
    virtual void _removeCollectionElementFromHandle(int collectionElementHandle);

    virtual void _setCollectionName_send(const char* newName);
    virtual void _setOverridesObjectMainProperties_send(bool o);

    int _collectionHandle;
    std::string _collectionName;
    bool _overridesObjectMainProperties;

private:
    std::vector<CCollectionElement*> _collectionElements;
};
