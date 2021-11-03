#include "_distanceObject_old.h"
#include "simConst.h"

_CDistanceObject_old::_CDistanceObject_old()
{
    _entity1Handle=-1;
    _entity2Handle=-1;
    _objectHandle=-1;
    _displaySegment=true;
    _explicitHandling=false;
    _threshold=0.5f;
    _thresholdEnabled=false;
    _segmentWidth=2;
}

_CDistanceObject_old::~_CDistanceObject_old()
{
}

void _CDistanceObject_old::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_color)
        { // this message is for the color object
            routing.erase(routing.begin());
            _segmentColor.synchronizationMsg(routing,msg);
        }
    }
    else
    { // message is for this distance object
        /*
        if (msg.msg==sim_syncobj_distanceobject_create)
        {
            setObjectHandle(((int*)msg.data)[0]);
            return;
        }
        */
        if (msg.msg==sim_syncobj_distanceobject_setexplicithandling)
        {
            setExplicitHandling(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setobjectname)
        {
            setObjectName(((char*)msg.data),true);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setthreshold)
        {
            setThreshold(((float*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setthresholdenabled)
        {
            setThresholdEnabled(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setsegmentwidth)
        {
            setSegmentWidth(((int*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_distanceobject_setdisplaysegment)
        {
            setDisplaySegment(((bool*)msg.data)[0]);
            return;
        }
    }
}

int _CDistanceObject_old::getObjectHandle() const
{
    return(_objectHandle);
}

int _CDistanceObject_old::getEntity1Handle() const
{
    return(_entity1Handle);
}

int _CDistanceObject_old::getEntity2Handle() const
{
    return(_entity2Handle);
}

std::string _CDistanceObject_old::getObjectName() const
{
    return(_objectName);
}

bool _CDistanceObject_old::setObjectName(const char* newName,bool check)
{
    bool diff=(_objectName!=newName);
    if (diff)
    {
        _objectName=newName;
        if (getObjectCanSync())
            _setObjectName_send(newName);
    }
    return(diff);
}

bool _CDistanceObject_old::setThreshold(float tr)
{
    bool diff=(_threshold!=tr);
    if (diff)
    {
        _threshold=tr;
        if (getObjectCanSync())
            _setThreshold_send(tr);
    }
    return(diff);
}

float _CDistanceObject_old::getTreshhold() const
{
    return(_threshold);
}

bool _CDistanceObject_old::setThresholdEnabled(bool enabled)
{
    bool diff=(_thresholdEnabled!=enabled);
    if (diff)
    {
        _thresholdEnabled=enabled;
        if (getObjectCanSync())
            _setThresholdEnabled_send(enabled);
    }
    return(diff);
}

bool _CDistanceObject_old::getTreshholdEnabled() const
{
    return(_thresholdEnabled);
}

bool _CDistanceObject_old::setDisplaySegment(bool display)
{
    bool diff=(_displaySegment!=display);
    if (diff)
    {
        _displaySegment=display;
        if (getObjectCanSync())
            _setDisplaySegment_send(display);
    }
    return(diff);
}

bool _CDistanceObject_old::setExplicitHandling(bool explicitHandl)
{
    bool diff=(_explicitHandling!=explicitHandl);
    if (diff)
    {
        _explicitHandling=explicitHandl;
        if (getObjectCanSync())
            _setExplicitHandling_send(explicitHandl);
    }
    return(diff);
}

bool _CDistanceObject_old::getExplicitHandling() const
{
    return(_explicitHandling);
}

bool _CDistanceObject_old::getDisplaySegment() const
{
    return(_displaySegment);
}

bool _CDistanceObject_old::setSegmentWidth(int w)
{
    bool diff=(_segmentWidth!=w);
    if (diff)
    {
        _segmentWidth=w;
        if (getObjectCanSync())
            _setSegmentWidth_send(w);
    }
    return(diff);
}

int _CDistanceObject_old::getSegmentWidth() const
{
    return(_segmentWidth);
}

CColorObject* _CDistanceObject_old::getSegmentColor()
{
    return(&_segmentColor);
}

bool _CDistanceObject_old::setObjectHandle(int newHandle)
{
    bool diff=(_objectHandle!=newHandle);
    if (diff)
        _objectHandle=newHandle;
    return(diff);
}

void _CDistanceObject_old::_setObjectName_send(const char* newName) const
{
}

void _CDistanceObject_old::_setThreshold_send(float tr) const
{
}

void _CDistanceObject_old::_setThresholdEnabled_send(bool enabled) const
{
}

void _CDistanceObject_old::_setDisplaySegment_send(bool display) const
{
}

void _CDistanceObject_old::_setExplicitHandling_send(bool explicitHandl) const
{
}

void _CDistanceObject_old::_setSegmentWidth_send(int w) const
{
}

