
#pragma once

#include "vrepMainHeader.h"

class CSharedFloatVector_data; // forward declaration

class CSharedFloatVector
{
public:

    CSharedFloatVector(); // create new data
    CSharedFloatVector(CSharedFloatVector* dataToShare); // shares the data
    CSharedFloatVector(CSharedFloatVector_data* dataToShare); // shares the data
    virtual ~CSharedFloatVector();

    const std::vector<float>& getRead(); // get the data for reading
    std::vector<float>& getWrite(); // get the data for writing
    CSharedFloatVector* getDuplicate(); // returns a copy that shares the data
    void replaceWithCopyOf(CSharedFloatVector* replacementToCopyFrom); // replaces the data with the provided data that will be shared

    static int releaseUnusedData();

private:
    CSharedFloatVector_data* _data;

    static std::vector<CSharedFloatVector_data*> _allData;
};

//---------------------------------------------------------
class CSharedFloatVector_data
{
public:
    CSharedFloatVector_data()
    {
        _refCnt=0;
    };

    CSharedFloatVector_data(std::vector<float>* toCopyFrom)
    {
        _refCnt=0;
        _data.resize(toCopyFrom->size());
        std::copy(toCopyFrom->begin(),toCopyFrom->end(),_data.begin());
    };

    ~CSharedFloatVector_data()
    {
    };

    CSharedFloatVector_data* copyYourself()
    {
        CSharedFloatVector_data* retVal=new CSharedFloatVector_data(&_data);
        return(retVal);
    };

    std::vector<float>& getData() { return(_data); };

    int getRefCounter() { return(_refCnt); };

    void increaseRefCounter() { _refCnt++; };

    void decreaseRefCounter() { _refCnt--; };

private:
    int _refCnt;
    std::vector<float> _data;
};
//---------------------------------------------------------
