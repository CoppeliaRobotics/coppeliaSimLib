// Needed for backward compatibility
#pragma once

#include "ser.h"

class CSimpleFilter
{
public:

    CSimpleFilter();
    virtual ~CSimpleFilter();

    void serialize(CSer& ar);

    void setFilterType(int t);
    int getFilterType();
    std::string getCodeEquivalent();

protected:
    bool _enabled;
    int _filterType;
    int _customFilterHeader;
    int _customFilterID;
    std::string _customFilterName;
    int _filterVersion;
    std::vector<int> _intParameters;
    std::vector<floatDouble> _floatParameters;
    std::vector<unsigned char> _byteParameters;
    std::vector<unsigned char> _customFilterParameters;
    int _uniqueID;
};
