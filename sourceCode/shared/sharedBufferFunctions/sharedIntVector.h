
#pragma once

#include "vrepMainHeader.h"

class CSharedIntVector_data; // forward declaration

class CSharedIntVector
{
public:

    CSharedIntVector(); // create new data
    CSharedIntVector(CSharedIntVector* dataToShare); // shares the data
    CSharedIntVector(CSharedIntVector_data* dataToShare); // shares the data
    virtual ~CSharedIntVector();

    const std::vector<int>& getRead(); // get the data for reading
    std::vector<int>& getWrite(); // get the data for writing
    CSharedIntVector* getDuplicate(); // returns a copy that shares the data
    void replaceWithCopyOf(CSharedIntVector* replacementToCopyFrom); // replaces the data with the provided data that will be shared

    static int releaseUnusedData();

private:
    CSharedIntVector_data* _data;

    static std::vector<CSharedIntVector_data*> _allData;
};

//---------------------------------------------------------
class CSharedIntVector_data
{
public:
    CSharedIntVector_data()
    {
        _refCnt=0;
    };

    CSharedIntVector_data(std::vector<int>* toCopyFrom)
    {
        _refCnt=0;
        _data.resize(toCopyFrom->size());
        std::copy(toCopyFrom->begin(),toCopyFrom->end(),_data.begin());
    };

    ~CSharedIntVector_data()
    {
    };

    CSharedIntVector_data* copyYourself()
    {
        CSharedIntVector_data* retVal=new CSharedIntVector_data(&_data);
        return(retVal);
    };

    std::vector<int>& getData() { return(_data); };

    int getRefCounter() { return(_refCnt); };

    void increaseRefCounter() { _refCnt++; };

    void decreaseRefCounter() { _refCnt--; };

private:
    int _refCnt;
    std::vector<int> _data;
};
//---------------------------------------------------------
