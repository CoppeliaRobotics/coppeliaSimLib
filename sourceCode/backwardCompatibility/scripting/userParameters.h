
#pragma once

#include <ser.h>

struct SUserParamEntry {
    std::string name;
    std::string unit;
    std::string value; // paramValue can contain embedded 0's since 2011/01/23
    int properties; // bit0 set: value not visible during simulation, bit1 set: value is not reset at simulation end, bit2 set: contains embedded 0's (since 2011/01/23)
};

class CUserParameters
{
public:
    CUserParameters();
    virtual ~CUserParameters();

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationEnded();

    void clearInitialParameters();

    // For following 2 functions, the paramName must be present for the function to return true:
    bool setParameterValue(const char* paramName,const char* paramValue,size_t paramValueLength); // paramValue can contain embedded 0's since 2011/01/23
    bool getParameterValue(const char* paramName,std::string& paramValue);

    // For following 3 functions, the paramName does not need to be present:
    void addParameterValue(const char* paramName,const char* unitText,const char* paramValue,size_t paramValueLength); // paramValue can contain embedded 0's since 2011/01/23
    bool removeParameterValue(const char* paramName);
    bool removeParameterValue(int index);
    int getParameterIndex(const char* paramName);
    void moveItem(int index,int newIndex);

    CUserParameters* copyYourself();
    void serialize(CSer& ar);

    std::vector<SUserParamEntry> userParamEntries;
protected:

    std::vector<SUserParamEntry> _initialUserParamEntries;
    bool _initialValuesInitialized;
};
