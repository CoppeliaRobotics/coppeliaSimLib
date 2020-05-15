#include "_collectionElement_.h"
#include "simConst.h"

_CCollectionElement_::_CCollectionElement_()
{
    _mainObjectHandle=-1;
    _elementType=-1;
    _elementHandle=-1;
    _additive=false;
}

_CCollectionElement_::~_CCollectionElement_()
{
}

void _CCollectionElement_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_

}

bool _CCollectionElement_::getIsAdditive() const
{
    return(_additive);
}

int _CCollectionElement_::getMainObject() const
{
    return(_mainObjectHandle);
}

int _CCollectionElement_::getElementType() const
{
    return(_elementType);
}

int _CCollectionElement_::getElementHandle() const
{
    return(_elementHandle);
}

void _CCollectionElement_::setElementHandle(int newHandle)
{
    _elementHandle=newHandle;
}

void _CCollectionElement_::setMainObject(int mo)
{ // only used in special situations
    _mainObjectHandle=mo;
}
