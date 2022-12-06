#pragma once

#include "simTypes.h"
#include <vector>
#include "4Vector.h"

struct SSyncRoute
{
    unsigned char objType;
    int objHandle;
};

class _CSyncObject_
{
public:

    _CSyncObject_();
    virtual ~_CSyncObject_();

    void setSyncMsgRouting(const std::vector<SSyncRoute>* ancestorRoute,const SSyncRoute& ownIdentity);
    const std::vector<SSyncRoute>* getSyncMsgRouting() const;
    bool isRoutingSet() const;

    bool getObjectCanSync() const;
    bool setObjectCanSync(bool s);

    static void setOverallSyncEnabled(bool e);
    static bool getOverallSyncEnabled();

protected:  

    bool _objectCanSync;

    std::vector<SSyncRoute> _routing;

    SSyncRt _rt;

    static bool _overallSyncEnabled;
};
