#include "_dummy_.h"
#include "simConst.h"

_CDummy_::_CDummy_()
{
    _objectType=sim_object_dummy_type;
    _localObjectSpecialProperty=0;

    _dummySize=0.01f;
    _assignedToParentPath=false;
    _assignedToParentPathOrientation=false;
    _linkedDummyHandle=-1;
    _linkType=sim_dummy_linktype_ik_tip_target;
}

_CDummy_::~_CDummy_()
{
}

void _CDummy_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_color)
        { // this message is for the color object
            routing.erase(routing.begin());
            _dummyColor.synchronizationMsg(routing,msg);
        }
    }
    else
    {
        if (msg.msg>=sim_syncobj_sceneobject_cmdstart)
        { // message is for this sceneObject
            _CSceneObject_::synchronizationMsg(routing,msg);
        }
        else
        { // message is for this dummy
            if (msg.msg==sim_syncobj_dummy_size)
            {
                setDummySize(((float*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_linkeddummy)
            {
                setLinkedDummyHandle(((int*)msg.data)[0],true);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_linktype)
            {
                setLinkType(((int*)msg.data)[0],true);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_assignedparentpath)
            {
                setAssignedToParentPath(((bool*)msg.data)[0]);
                return;
            }
            if (msg.msg==sim_syncobj_dummy_assignedparentpathorientation)
            {
                setAssignedToParentPathOrientation(((bool*)msg.data)[0]);
                return;
            }
        }
    }
}

bool _CDummy_::setDummySize(float s)
{
    bool diff=(_dummySize!=s);
    if (diff)
    {
        if (getObjectCanChange())
            _dummySize=s;
        if (getObjectCanSync())
            _setDummySize_send(s);
    }
    return(diff&&getObjectCanChange());
}

void _CDummy_::_setDummySize_send(float s) const
{
    sendFloat(s,sim_syncobj_dummy_size);
}

bool _CDummy_::setLinkedDummyHandle(int h,bool check)
{
    bool diff=(_linkedDummyHandle!=h);
    if (diff)
    {
        if (getObjectCanChange())
            _linkedDummyHandle=h;
        if (getObjectCanSync())
            _setLinkedDummyHandle_send(h);
    }
    return(diff&&getObjectCanChange());
}

void _CDummy_::_setLinkedDummyHandle_send(int h) const
{
    sendInt32(h,sim_syncobj_dummy_linkeddummy);
}

bool _CDummy_::setLinkType(int lt,bool check)
{
    bool diff=(_linkType!=lt);
    if (diff)
    {
        if (getObjectCanChange())
            _linkType=lt;
        if (getObjectCanSync())
            _setLinkType_send(lt);
    }
    return(diff&&getObjectCanChange());
}

void _CDummy_::_setLinkType_send(int t) const
{
    sendInt32(t,sim_syncobj_dummy_linktype);
}

bool _CDummy_::setAssignedToParentPath(bool a)
{
    bool diff=(_assignedToParentPath!=a);
    if (diff)
    {
        if (getObjectCanChange())
            _assignedToParentPath=a;
        if (getObjectCanSync())
            _setAssignedToParentPath_send(a);
    }
    return(diff&&getObjectCanChange());
}

void _CDummy_::_setAssignedToParentPath_send(bool a) const
{
    sendBool(a,sim_syncobj_dummy_assignedparentpath);
}

bool _CDummy_::setAssignedToParentPathOrientation(bool a)
{
    bool diff=(_assignedToParentPathOrientation!=a);
    if (diff)
    {
        if (getObjectCanChange())
            _assignedToParentPathOrientation=a;
        if (getObjectCanSync())
            _setAssignedToParentPathOrientation_send(a);
    }
    return(diff&&getObjectCanChange());
}

void _CDummy_::_setAssignedToParentPathOrientation_send(bool a) const
{
    sendBool(a,sim_syncobj_dummy_assignedparentpathorientation);
}

bool _CDummy_::getAssignedToParentPath() const
{
    return(_assignedToParentPath);
}

bool _CDummy_::getAssignedToParentPathOrientation() const
{
    return(_assignedToParentPathOrientation);
}

float _CDummy_::getDummySize() const
{
    return(_dummySize);
}

CColorObject* _CDummy_::getDummyColor()
{
    return(&_dummyColor);
}

int _CDummy_::getLinkType() const
{
    return(_linkType);
}


int _CDummy_::getLinkedDummyHandle() const
{
    return(_linkedDummyHandle);
}

/*
void _CDummy_::_connectIkPluginCounterpart_()
{
    // Synchronize with IK plugin:
    if ( (_linkedDummyHandle!=-1)&&(_linkType==sim_dummy_linktype_ik_tip_target) )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(_linkedDummyHandle);
        CPluginContainer::ikPlugin_setLinkedDummy(_ikPluginCounterpartHandle_,it->getIkPluginCounterpartHandle());
    }
}

void _CDummy_::_rebuildIkPluginCounterpart_(int counterpartHandle)
{
    // Synchronize with IK plugin:
    _ikPluginCounterpartHandle_=counterpartHandle;
}
*/
