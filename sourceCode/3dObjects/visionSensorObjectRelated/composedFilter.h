// Needed for backward compatibility

#pragma once

#include "simpleFilter.h"
#include "ser.h"

class CComposedFilter
{
public:

    CComposedFilter();
    virtual ~CComposedFilter();

    void serialize(CSer& ar);
    void removeAllSimpleFilters();

    std::string scriptEquivalent;

protected:
    void _prepareScriptEquivalent();

    std::vector<CSimpleFilter*> _allSimpleFilters;
};
