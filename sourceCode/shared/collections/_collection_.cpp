#include "_collection_.h"
#include "simConst.h"

_CCollection_::_CCollection_()
{
    _collectionHandle=-1;
}

_CCollection_::~_CCollection_()
{
    while (_collectionElements.size()>0)
        _removeCollectionElementFromHandle(_collectionElements[0]->getElementHandle());
}

void _CCollection_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing.size()>0)
    {
        if (routing[0].objType==sim_syncobj_collectionelement)
        { // Msg is for a collection element
            CCollectionElement* el=getElementFromHandle(routing[0].objHandle);
            if (el==nullptr)
            {
                if ( (msg.msg==sim_syncobj_collectionelement_create)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    el=new CCollectionElement(((int*)msg.data)[0],((int*)msg.data)[1],((bool*)msg.data)[2]);
                    el->setElementHandle(routing[0].objHandle);
                    _addCollectionElement(el);
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_collectionelement_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeCollectionElementFromHandle(routing[0].objHandle);
                else
                {
                    routing.erase(routing.begin());
                    el->synchronizationMsg(routing,msg);
                }
            }
        }
    }
    else
    { // Msg is for this collection
        if (msg.msg==sim_syncobj_collection_setoverrideproperties)
        {
            setOverridesObjectMainProperties(((bool*)msg.data)[0]);
            return;
        }
        if (msg.msg==sim_syncobj_collection_setobjectname)
        {
            setCollectionName(((char*)msg.data),true);
            return;
        }
    }
}

bool _CCollection_::getOverridesObjectMainProperties() const
{
    return(_overridesObjectMainProperties);
}

bool _CCollection_::setOverridesObjectMainProperties(bool o)
{
    bool diff=(_overridesObjectMainProperties!=o);
    if (diff)
    {
        _overridesObjectMainProperties=o;
        if (getObjectCanSync())
            _setOverridesObjectMainProperties_send(o);
    }
    return(diff);
}

size_t _CCollection_::getElementCount() const
{
    return(_collectionElements.size());
}

CCollectionElement* _CCollection_::getElementFromIndex(size_t index) const
{
    CCollectionElement* retVal=nullptr;
    if (index<_collectionElements.size())
        retVal=_collectionElements[index];
    return(retVal);
}

CCollectionElement* _CCollection_::getElementFromHandle(int collectionElementHandle) const
{
    for (size_t i=0;i<_collectionElements.size();i++)
    {
        if (_collectionElements[i]->getElementHandle()==collectionElementHandle)
            return(_collectionElements[i]);
    }
    return(nullptr);
}

void _CCollection_::_addCollectionElement(CCollectionElement* collectionElement)
{
    _collectionElements.push_back(collectionElement);
}

void _CCollection_::_removeCollectionElementFromHandle(int collectionElementHandle)
{
    for (size_t i=0;i<_collectionElements.size();i++)
    {
        if (_collectionElements[i]->getElementHandle()==collectionElementHandle)
        {
            delete _collectionElements[i];
            _collectionElements.erase(_collectionElements.begin()+i);
            break;
        }
    }
}

int _CCollection_::getCollectionHandle() const
{
    return(_collectionHandle);
}

bool _CCollection_::setCollectionHandle(int newHandle)
{
    bool diff=(_collectionHandle!=newHandle);
    _collectionHandle=newHandle;
    return(diff);
}

std::string _CCollection_::getCollectionName() const
{
    return(_collectionName);
}

bool _CCollection_::setCollectionName(const char* newName,bool check)
{
    bool diff=(_collectionName!=newName);
    if (diff)
    {
        _collectionName=newName;
        if (getObjectCanSync())
            _setCollectionName_send(newName);
    }
    return(diff);
}

void _CCollection_::_setCollectionName_send(const char* newName)
{
}

void _CCollection_::_setOverridesObjectMainProperties_send(bool o)
{
}
