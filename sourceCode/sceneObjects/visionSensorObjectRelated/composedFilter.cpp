// Needed for backward compatibility

#include "composedFilter.h"
#include "simConst.h"

CComposedFilter::CComposedFilter()
{
}

CComposedFilter::~CComposedFilter()
{
    removeAllSimpleFilters();
}

void CComposedFilter::removeAllSimpleFilters()
{
    for (int i=0;i<int(_allSimpleFilters.size());i++)
        delete _allSimpleFilters[i];
    _allSimpleFilters.clear();
}

void CComposedFilter::serialize(CSer& ar)
{
    if (!ar.isStoring())
    {       // Loading
        int byteQuantity;
        std::string theName="";
        while (theName.compare(SER_END_OF_OBJECT)!=0)
        {
            theName=ar.readDataName();
            if (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                bool noHit=true;
                if (theName.compare("Sfr")==0)
                {
                    noHit=false;
                    ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                    CSimpleFilter* it=new CSimpleFilter();
                    it->serialize(ar);
                    _allSimpleFilters.push_back(it);
                }
                if (noHit)
                    ar.loadUnknownData();
            }
        }
    }
    _prepareScriptEquivalent();
}
void CComposedFilter::_prepareScriptEquivalent()
{
    if (_allSimpleFilters.size()!=0)
    {
        scriptEquivalent+="function sysCall_vision(inData)\n";
        scriptEquivalent+="    -- callback function automatically added for backward compatibility\n    -- (vision sensor have no filters anymore, but rather a callback function where image processing can be performed)\n    local retVal={}\n    retVal.trigger=false\n    retVal.packedPackets={}\n";
        for (size_t i=0;i<_allSimpleFilters.size();i++)
        {
            CSimpleFilter* it=_allSimpleFilters[i];
            scriptEquivalent+="    ";
            scriptEquivalent+=it->getCodeEquivalent();
            scriptEquivalent+="\n";
        }
        scriptEquivalent+="    return retVal\nend\n\n";
        if ( (_allSimpleFilters.size()==2)&&(_allSimpleFilters[0]->getFilterType()==sim_filtercomponent_originalimage_deprecated)&&(_allSimpleFilters[1]->getFilterType()==sim_filtercomponent_tooutput_deprecated) )
            scriptEquivalent.clear();
    }
    removeAllSimpleFilters();
}


