#include "_collectionContainer_.h"
#include "simConst.h"

_CCollectionContainer_::_CCollectionContainer_()
{
    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_collection;
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CCollectionContainer_::~_CCollectionContainer_()
{ // beware, the current world could be nullptr
    while (_allCollections.size()!=0)
        _removeCollection(_allCollections[0]->getCollectionHandle());
}

size_t _CCollectionContainer_::getObjectCount() const
{
    return(_allCollections.size());
}

CCollection* _CCollectionContainer_::getObjectFromIndex(size_t index) const
{
    CCollection* retVal=nullptr;
    if (index<_allCollections.size())
        retVal=_allCollections[index];
    return(retVal);
}

CCollection* _CCollectionContainer_::getObjectFromHandle(int collectionHandle) const
{
    CCollection* retVal=nullptr;
    for (size_t i=0;i<_allCollections.size();i++)
    {
        if (_allCollections[i]->getCollectionHandle()==collectionHandle)
        {
            retVal=_allCollections[i];
            break;
        }
    }
    return(retVal);
}

CCollection* _CCollectionContainer_::getObjectFromName(const char* collectionName) const
{
    CCollection* retVal=nullptr;
    for (size_t i=0;i<_allCollections.size();i++)
    {
        if (_allCollections[i]->getCollectionName().compare(collectionName)==0)
        {
            retVal=_allCollections[i];
            break;
        }
    }
    return(retVal);
}

void _CCollectionContainer_::_removeCollection(int collectionHandle)
{
    for (size_t i=0;i<_allCollections.size();i++)
    {
        if (_allCollections[i]->getCollectionHandle()==collectionHandle)
        {
            delete _allCollections[i];
            _allCollections.erase(_allCollections.begin()+i);
            break;
        }
    }
}

void _CCollectionContainer_::_addCollection(CCollection* collection)
{
    _allCollections.push_back(collection);
}

void _CCollectionContainer_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (routing[0].objType==sim_syncobj_collection)
    {
        if (routing[0].objHandle!=-1)
        { // Msg is for a collection
            CCollection* collection=getObjectFromHandle(routing[0].objHandle);
            if (collection==nullptr)
            {
                if ( (msg.msg==sim_syncobj_collection_create)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                {
                    collection=new CCollection();
                    collection->setCollectionHandle(routing[0].objHandle);
                    _addCollection(collection);
                }
            }
            else
            {
                if ( (msg.msg==sim_syncobj_collection_delete)&&(routing.size()==1) ) // check also size (some msgs have same ids in different scopes)
                    _removeCollection(routing[0].objHandle);
                else
                {
                    routing.erase(routing.begin());
                    collection->synchronizationMsg(routing,msg);
                }
            }
        }
        else
        { // Msg is for this collection container
        }
    }
}
