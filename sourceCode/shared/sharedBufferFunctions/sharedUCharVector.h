#pragma once

#include <vector>

class CSharedUCharVector_data; // forward declaration

class CSharedUCharVector
{
public:

    CSharedUCharVector(); // create new data
    CSharedUCharVector(CSharedUCharVector* dataToShare); // shares the data
    CSharedUCharVector(CSharedUCharVector_data* dataToShare); // shares the data
    virtual ~CSharedUCharVector();

    const std::vector<unsigned char>& getRead(); // get the data for reading
    std::vector<unsigned char>& getWrite(); // get the data for writing
    CSharedUCharVector* getDuplicate(); // returns a copy that shares the data
    void replaceWithCopyOf(CSharedUCharVector* replacementToCopyFrom); // replaces the data with the provided data that will be shared

    static int releaseUnusedData();

private:
    CSharedUCharVector_data* _data;

    static std::vector<CSharedUCharVector_data*> _allData;
};

//---------------------------------------------------------
class CSharedUCharVector_data
{
public:
    CSharedUCharVector_data()
    {
        _refCnt=0;
    };

    CSharedUCharVector_data(std::vector<unsigned char>* toCopyFrom)
    {
        _refCnt=0;
        _data.resize(toCopyFrom->size());
        std::copy(toCopyFrom->begin(),toCopyFrom->end(),_data.begin());
    };

    ~CSharedUCharVector_data()
    {
    };

    CSharedUCharVector_data* copyYourself()
    {
        CSharedUCharVector_data* retVal=new CSharedUCharVector_data(&_data);
        return(retVal);
    };

    std::vector<unsigned char>& getData() { return(_data); };

    int getRefCounter() { return(_refCnt); };

    void increaseRefCounter() { _refCnt++; };

    void decreaseRefCounter() { _refCnt--; };

private:
    int _refCnt;
    std::vector<unsigned char> _data;
};
//---------------------------------------------------------
