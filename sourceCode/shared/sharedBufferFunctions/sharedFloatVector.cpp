
#include "sharedFloatVector.h"

std::vector<CSharedFloatVector_data*> CSharedFloatVector::_allData;

CSharedFloatVector::CSharedFloatVector()
{
    _data=new CSharedFloatVector_data();
    _data->increaseRefCounter();
    _allData.push_back(_data);
}

CSharedFloatVector::CSharedFloatVector(CSharedFloatVector* dataToShare)
{
    _data=dataToShare->_data;
    _data->increaseRefCounter();
}

CSharedFloatVector::CSharedFloatVector(CSharedFloatVector_data* dataToShare)
{
    _data=dataToShare;
    _data->increaseRefCounter();
}

CSharedFloatVector::~CSharedFloatVector()
{
    _data->decreaseRefCounter();
//  if (notGui)
        releaseUnusedData();
}

const std::vector<float>& CSharedFloatVector::getRead()
{
    return(_data->getData());
}

std::vector<float>& CSharedFloatVector::getWrite()
{
    if (_data->getRefCounter()==1)
        return(_data->getData()); // data not shared right now

    // The data is shared. We duplicate and disconnect it:
    CSharedFloatVector_data* newData=_data->copyYourself();
    _data->decreaseRefCounter();
    _data=newData;
    _data->increaseRefCounter();
    _allData.push_back(_data);
    return(_data->getData());
}

CSharedFloatVector* CSharedFloatVector::getDuplicate()
{
    CSharedFloatVector* retVal=new CSharedFloatVector(_data);
    return(retVal);
}

void CSharedFloatVector::replaceWithCopyOf(CSharedFloatVector* replacementToCopyFrom)
{
    _data->decreaseRefCounter();
//  if (notGui)
        releaseUnusedData();
    _data=replacementToCopyFrom->_data;
    _data->increaseRefCounter();
}

int CSharedFloatVector::releaseUnusedData()
{
    size_t i=0;
    while (i<_allData.size())
    {
        if (_allData[i]->getRefCounter()==0)
            delete _allData[i];
        else
            i++;
    }
    return(int(_allData.size()));
}
