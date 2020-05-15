#pragma once

#include <string>
#include "syncObject.h"

enum {
    sim_syncobj_collectionelement_create=0,
    sim_syncobj_collectionelement_delete,
};

class _CCollectionElement_ : public CSyncObject
{
public:
    _CCollectionElement_();
    virtual ~_CCollectionElement_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    int getElementHandle() const;
    int getMainObject() const;
    bool getIsAdditive() const;
    int getElementType() const;

    virtual void setElementHandle(int newHandle);
    virtual void setMainObject(int mo);

protected:
    int _elementHandle;
    int _elementType;
    int _mainObjectHandle;
    bool _additive;
};
