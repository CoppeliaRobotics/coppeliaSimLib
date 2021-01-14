#pragma once

#include "colorObject.h"
#include "syncObject.h"
#include <string>

enum {
    sim_syncobj_distanceobject_create=0,
    sim_syncobj_distanceobject_delete,
    sim_syncobj_distanceobject_setobjectname,
    sim_syncobj_distanceobject_setexplicithandling,
    sim_syncobj_distanceobject_setthreshold,
    sim_syncobj_distanceobject_setthresholdenabled,
    sim_syncobj_distanceobject_setsegmentwidth,
    sim_syncobj_distanceobject_setdisplaysegment,
};

class _CDistanceObject_old : public CSyncObject
{
public:
    _CDistanceObject_old();
    virtual ~_CDistanceObject_old();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    int getObjectHandle() const;
    int getEntity1Handle() const;
    int getEntity2Handle() const;
    std::string getObjectName() const;
    float getTreshhold() const;
    bool getTreshholdEnabled() const;
    bool getDisplaySegment() const;
    bool getExplicitHandling() const;
    int getSegmentWidth() const;

    CColorObject* getSegmentColor();

    virtual bool setObjectHandle(int newHandle);

    virtual bool setObjectName(const char* newName,bool check);
    virtual bool setThreshold(float tr);
    virtual bool setThresholdEnabled(bool enabled);
    virtual bool setDisplaySegment(bool display);
    virtual bool setExplicitHandling(bool explicitHandl);
    virtual bool setSegmentWidth(int w);

protected:
    virtual void _setObjectName_send(const char* newName) const;
    virtual void _setThreshold_send(float tr) const;
    virtual void _setThresholdEnabled_send(bool enabled) const;
    virtual void _setDisplaySegment_send(bool display) const;
    virtual void _setExplicitHandling_send(bool explicitHandl) const;
    virtual void _setSegmentWidth_send(int w) const;

    int _entity1Handle;
    int _entity2Handle;
    std::string _objectName;
    int _objectHandle;
    int _segmentWidth;
    float _threshold;
    bool _thresholdEnabled;
    bool _displaySegment;
    bool _explicitHandling;

    CColorObject _segmentColor;
};
