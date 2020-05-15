#include "_ikGroupContainer_.h"
#include "simConst.h"

_CIkGroupContainer_::_CIkGroupContainer_()
{
    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_ikgroup;
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CIkGroupContainer_::~_CIkGroupContainer_()
{
    while (_ikGroups.size()!=0)
        _removeIkGroup(_ikGroups[0]->getObjectHandle());
}

size_t _CIkGroupContainer_::getObjectCount() const
{
    return(_ikGroups.size());
}

CIkGroup* _CIkGroupContainer_::getObjectFromHandle(int ikGroupHandle) const
{
     for (size_t i=0;i<_ikGroups.size();i++)
     {
        if (_ikGroups[i]->getObjectHandle()==ikGroupHandle)
            return(_ikGroups[i]);
     }
     return(nullptr);
}

CIkGroup* _CIkGroupContainer_::getObjectFromIndex(size_t index) const
{
    CIkGroup* retVal=nullptr;
    if (index<_ikGroups.size())
        retVal=_ikGroups[index];
    return(retVal);
}

CIkGroup* _CIkGroupContainer_::getObjectFromName(const char* ikGroupName) const
{
    for (size_t i=0;i<_ikGroups.size();i++)
    {
        if (_ikGroups[i]->getObjectName().compare(ikGroupName)==0)
            return(_ikGroups[i]);
    }
    return(nullptr);
}

void _CIkGroupContainer_::_addIkGroup(CIkGroup* anIkGroup)
{
    _ikGroups.push_back(anIkGroup);
}

void _CIkGroupContainer_::_removeIkGroup(int ikGroupHandle)
{
    for (size_t i=0;i<_ikGroups.size();i++)
    {
        if (_ikGroups[i]->getObjectHandle()==ikGroupHandle)
        {
            delete _ikGroups[i];
            _ikGroups.erase(_ikGroups.begin()+i);
            break;
        }
    }
}

bool _CIkGroupContainer_::shiftIkGroup(int ikGroupHandle,bool shiftUp)
{
    bool diff=false;
    CIkGroup* it=getObjectFromHandle(ikGroupHandle);
    for (size_t i=0;i<_ikGroups.size();i++)
    {
        CIkGroup* ikg=_ikGroups[i];
        if (shiftUp)
        {
            if ( (ikg->getObjectHandle()==ikGroupHandle)&&(i>0) )
            {
                if (getObjectCanChange())
                {
                    _ikGroups.erase(_ikGroups.begin()+i);
                    _ikGroups.insert(_ikGroups.begin()+i-1,it);
                }
                diff=true;
                break;
            }
        }
        else
        {
            if ( (ikg->getObjectHandle()==ikGroupHandle)&&(i<_ikGroups.size()-1) )
            {
                if (getObjectCanChange())
                {
                    _ikGroups.erase(_ikGroups.begin()+i);
                    _ikGroups.insert(_ikGroups.begin()+i+1,it);
                }
                diff=true;
                break;
            }
        }
    }
    if (diff)
    {
        if (getObjectCanSync())
            _shiftIkGroup_send(ikGroupHandle,shiftUp);
    }
    return(diff&&getObjectCanSync());
}

void _CIkGroupContainer_::_shiftIkGroup_send(int ikGroupHandle,bool shiftUp)
{
    int h=ikGroupHandle;
    if (!shiftUp)
        h=-1-h;
    sendInt32(h,sim_syncobj_ikgroupcont_shift);

}

void _CIkGroupContainer_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing[0].objType==sim_syncobj_ikgroup)
    {
        if (routing[0].objHandle!=-1)
        { // Msg is for an IK group
            CIkGroup* ikGroup=getObjectFromHandle(routing[0].objHandle);
            if (ikGroup==nullptr)
            {
                if ( (msg.msg==sim_syncobj_ikgroup_create)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    ikGroup=new CIkGroup();
                    ikGroup->setObjectHandle(routing[0].objHandle);
                    _addIkGroup(ikGroup);
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_ikgroup_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeIkGroup(routing[0].objHandle);
                else
                {
                    routing.erase(routing.begin());
                    ikGroup->synchronizationMsg(routing,msg);
                }
            }
        }
        else
        { // Msg is for this IK group container
            if (msg.msg==sim_syncobj_ikgroupcont_shift)
            {
                int h=((int*)msg.data)[0];
                bool up=true;
                if (h<0)
                {
                    h=-h+1;
                    up=false;
                }
                shiftIkGroup(h,up);
            }
        }
    }
}
