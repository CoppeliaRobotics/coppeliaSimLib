
#include "sharedIntVector.h"

std::vector<CSharedIntVector_data*> CSharedIntVector::_allData;

CSharedIntVector::CSharedIntVector()
{
    _data=new CSharedIntVector_data();
    _data->increaseRefCounter();
    _allData.push_back(_data);
}

CSharedIntVector::CSharedIntVector(CSharedIntVector* dataToShare)
{
    _data=dataToShare->_data;
    _data->increaseRefCounter();
}

CSharedIntVector::CSharedIntVector(CSharedIntVector_data* dataToShare)
{
    _data=dataToShare;
    _data->increaseRefCounter();
}

CSharedIntVector::~CSharedIntVector()
{
    _data->decreaseRefCounter();
//  if (notGui)
        releaseUnusedData();
}

const std::vector<int>& CSharedIntVector::getRead()
{
    return(_data->getData());
}

std::vector<int>& CSharedIntVector::getWrite()
{
    if (_data->getRefCounter()==1)
        return(_data->getData()); // data not shared right now

    // The data is shared. We duplicate and disconnect it:
    CSharedIntVector_data* newData=_data->copyYourself();
    _data->decreaseRefCounter();
    _data=newData;
    _data->increaseRefCounter();
    _allData.push_back(_data);
    return(_data->getData());
}

CSharedIntVector* CSharedIntVector::getDuplicate()
{
    CSharedIntVector* retVal=new CSharedIntVector(_data);
    return(retVal);
}

void CSharedIntVector::replaceWithCopyOf(CSharedIntVector* replacementToCopyFrom)
{
    _data->decreaseRefCounter();
//  if (notGui)
        releaseUnusedData();
    _data=replacementToCopyFrom->_data;
    _data->increaseRefCounter();
}

int CSharedIntVector::releaseUnusedData()
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
