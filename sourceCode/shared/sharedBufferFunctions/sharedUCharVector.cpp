
#include "sharedUCharVector.h"

std::vector<CSharedUCharVector_data*> CSharedUCharVector::_allData;

CSharedUCharVector::CSharedUCharVector()
{
    _data=new CSharedUCharVector_data();
    _data->increaseRefCounter();
    _allData.push_back(_data);
}

CSharedUCharVector::CSharedUCharVector(CSharedUCharVector* dataToShare)
{
    _data=dataToShare->_data;
    _data->increaseRefCounter();
}

CSharedUCharVector::CSharedUCharVector(CSharedUCharVector_data* dataToShare)
{
    _data=dataToShare;
    _data->increaseRefCounter();
}

CSharedUCharVector::~CSharedUCharVector()
{
    _data->decreaseRefCounter();
//  if (notGui)
        releaseUnusedData();
}

const std::vector<unsigned char>& CSharedUCharVector::getRead()
{
    return(_data->getData());
}

std::vector<unsigned char>& CSharedUCharVector::getWrite()
{
    if (_data->getRefCounter()==1)
        return(_data->getData()); // data not shared right now

    // The data is shared. We duplicate and disconnect it:
    CSharedUCharVector_data* newData=_data->copyYourself();
    _data->decreaseRefCounter();
    _data=newData;
    _data->increaseRefCounter();
    _allData.push_back(_data);
    return(_data->getData());
}

CSharedUCharVector* CSharedUCharVector::getDuplicate()
{
    CSharedUCharVector* retVal=new CSharedUCharVector(_data);
    return(retVal);
}

void CSharedUCharVector::replaceWithCopyOf(CSharedUCharVector* replacementToCopyFrom)
{
    _data->decreaseRefCounter();
//  if (notGui)
        releaseUnusedData();
    _data=replacementToCopyFrom->_data;
    _data->increaseRefCounter();
}

int CSharedUCharVector::releaseUnusedData()
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
