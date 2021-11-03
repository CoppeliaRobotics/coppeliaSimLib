#pragma once

#include "_syncObject_.h"

class CSyncObject : public _CSyncObject_
{
public:
    CSyncObject();
    virtual ~CSyncObject();

    virtual void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    virtual void connectSynchronizationObject();
    virtual void removeSynchronizationObject(bool localReferencesToItOnly);
};
