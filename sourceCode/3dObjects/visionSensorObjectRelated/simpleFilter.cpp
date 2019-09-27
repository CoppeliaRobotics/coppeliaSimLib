#include "vrepMainHeader.h"
#include "simpleFilter.h"
#include "tt.h"
#include "imageProcess.h"
#include "v_rep_internal.h"
#include "pluginContainer.h"
#include "v_repStrings.h"
#include "MyMath.h"
#include "visionSensor.h"
#include <boost/lexical_cast.hpp>

std::vector<SExternalFilterInfo> CSimpleFilter::_externalFilters;

CSimpleFilter::CSimpleFilter()
{
    _filterType=-1;
    _customFilterHeader=-1;
    _customFilterID=-1;
    _customFilterName="";
    _enabled=true;
    _filterVersion=0;
    static int uniqueIDCounter=0;
    _uniqueID=uniqueIDCounter++;
}

CSimpleFilter::~CSimpleFilter()
{

}

int CSimpleFilter::getUniqueID()
{
    return(_uniqueID);
}

void CSimpleFilter::readAllExternalFilters()
{ // routine is static!
    static bool _externalFiltersWereRead=false;
    if (!_externalFiltersWereRead)
    {
        void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_imagefilter_enumreset,nullptr,nullptr,nullptr);
        delete[] (char*)retVal;
        int replyData[4];
        retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_imagefilter_enumerate,nullptr,nullptr,replyData);
        while (retVal!=nullptr)
        {
            SExternalFilterInfo f;
            f.name=std::string((char*)retVal);
            delete[] (char*)retVal;
            f.header=replyData[0];
            f.id=replyData[1];
            _externalFilters.push_back(f);
            retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_imagefilter_enumerate,nullptr,nullptr,replyData);
        }
        _externalFiltersWereRead=true;
    }
}

int CSimpleFilter::_getFilterTypeAndNameFromHeaderAndID(int header,int id,std::string& name)
{
    for (int i=0;i<int(_externalFilters.size());i++)
    {
        if ((_externalFilters[i].header==header)&&(_externalFilters[i].id==id))
        {
            name=_externalFilters[i].name;
            return(sim_filtercomponent_customized+i);
        }
    }
    return(-1); // not found!
}

void CSimpleFilter::setEnabled(bool e)
{
    _enabled=e;
}

bool CSimpleFilter::getEnabled()
{
    return(_enabled);
}

void CSimpleFilter::getParameters(std::vector<unsigned char>& byteParams,std::vector<int>& intParams,std::vector<float>& floatParams,int& filterVersion)
{
    filterVersion=_filterVersion;
    byteParams.assign(_byteParameters.begin(),_byteParameters.end());
    intParams.assign(_intParameters.begin(),_intParameters.end());
    floatParams.assign(_floatParameters.begin(),_floatParameters.end());
}

void CSimpleFilter::setParameters(const std::vector<unsigned char>& byteParams,const std::vector<int>& intParams,const std::vector<float>& floatParams,int filterVersion)
{
    _filterVersion=filterVersion;
    _byteParameters.assign(byteParams.begin(),byteParams.end());
    _intParameters.assign(intParams.begin(),intParams.end());
    _floatParameters.assign(floatParams.begin(),floatParams.end());
}

void CSimpleFilter::getCustomFilterParameters(std::vector<unsigned char>& params)
{
    params.assign(_customFilterParameters.begin(),_customFilterParameters.end());
}

void CSimpleFilter::setCustomFilterParameters(const std::vector<unsigned char>& params)
{
    _customFilterParameters.assign(params.begin(),params.end());
}

void CSimpleFilter::setFilterType(int t)
{ // Is called during loading!! (keep simple)
    _filterType=t;
    _byteParameters.clear();
    _floatParameters.clear();
    _intParameters.clear();

    if (_filterType>=sim_filtercomponent_customized)
    {
        int ind=_filterType-sim_filtercomponent_customized;
        if (ind<int(_externalFilters.size()))
        {
            _customFilterHeader=_externalFilters[ind].header;
            _customFilterID=_externalFilters[ind].id;
            _customFilterName=_externalFilters[ind].name;
        }
        else
        { // error
            _filterType=-1; // to indicate that we have to try to reload it later
            _customFilterHeader=0;
            _customFilterID=-1;
            _customFilterName="Error";
        }
    }

    // Now set default parameter values
    if (_filterType==sim_filtercomponent_uniformimage)
    {
        _floatParameters.push_back(1.0f); // r
        _floatParameters.push_back(1.0f); // g
        _floatParameters.push_back(1.0f); // b
    }
    if (_filterType==sim_filtercomponent_3x3filter)
    {
        _intParameters.push_back(1); // number of passes
        // We calculate a Gaussian blur:
        // 9 matrix elements, plus one element for the multiplication factor
        _floatParameters.resize(9+1,1.0f);
        const float sigma=2.0f;
        float tot=0.0f;
        for (int i=-1;i<2;i++)
        {
            for (int j=-1;j<2;j++)
            {
                float v=pow(2.7182818f,-(i*i+j*j)/(2.0f*sigma*sigma))/(2.0f*piValue_f*sigma*sigma);
                _floatParameters[i+1+(j+1)*3]=v;
                tot+=v;
            }
        }
        for (int i=0;i<9;i++)
            _floatParameters[i]/=tot;
    }
    if (_filterType==sim_filtercomponent_5x5filter)
    {
        _intParameters.push_back(1); // number of passes
        // We calculate a Gaussian blur:
        // 25 matrix elements, plus one element for the multiplication factor
        _floatParameters.resize(25+1,1.0f);
        const float sigma=2.0f;
        float tot=0.0f;
        for (int i=-2;i<3;i++)
        {
            for (int j=-2;j<3;j++)
            {
                float v=pow(2.7182818f,-(i*i+j*j)/(2.0f*sigma*sigma))/(2.0f*piValue_f*sigma*sigma);
                _floatParameters[i+2+(j+2)*5]=v;
                tot+=v;
            }
        }
        for (int i=0;i<25;i++)
            _floatParameters[i]/=tot;
    }
    if (_filterType==sim_filtercomponent_keeporremovecolors)
    {
        _byteParameters.push_back(1+2); // bit 0: keep (otherwise remove), bit 1: rgb (otherwise hsl), bit 2: copy removed part to buffer 1
        _floatParameters.push_back(0.5f); // r or h value
        _floatParameters.push_back(0.5f); // g or s value
        _floatParameters.push_back(0.5f); // b or l value
        _floatParameters.push_back(0.25); // r or h tolerance (x --> +-x)
        _floatParameters.push_back(0.25); // g or s tolerance (x --> +-x)
        _floatParameters.push_back(0.25); // b or l tolerance (x --> +-x)
    }
    if (_filterType==sim_filtercomponent_scaleandoffsetcolors)
    {
        _byteParameters.push_back(2); // bit 0: not used yet, bit 1: rgb (otherwise hsl)
        _floatParameters.push_back(0.0f); // r or h offset (before scaling)
        _floatParameters.push_back(0.0f); // g or s offset (before scaling)
        _floatParameters.push_back(0.0f); // b or l offset (before scaling)
        _floatParameters.push_back(1.2f); // r or h scale
        _floatParameters.push_back(1.2f); // g or s scale
        _floatParameters.push_back(1.2f); // b or l scale
        _floatParameters.push_back(0.0f); // r or h offset (after scaling)
        _floatParameters.push_back(0.0f); // g or s offset (after scaling)
        _floatParameters.push_back(0.0f); // b or l offset (after scaling)
    }
    if (_filterType==sim_filtercomponent_correlationwithbuffer1)
    {
        _byteParameters.push_back(8+16); // 8: overlay display, 16: greyscale calculations
        _floatParameters.push_back(0.5f); // sub-image size X (template)
        _floatParameters.push_back(0.5f); // sub-image size Y (template)
        _floatParameters.push_back(1.0f); // search frame X
        _floatParameters.push_back(1.0f); // search frame Y
    }
    if (_filterType==sim_filtercomponent_shift)
    {
        _byteParameters.push_back(0); // bit 0: if set, then no wrapping
        _floatParameters.push_back(0.5f); // x-shift (-1 to 1)
        _floatParameters.push_back(0.5f); // y-shift (-1 to 1)
    }
    if (_filterType==sim_filtercomponent_circularcut)
    {
        _byteParameters.push_back(0); // bit 0: if set, cut part is copied to buffer 1
        _floatParameters.push_back(1.0f); // circle radius relative to smallest half-size
    }
    if (_filterType==sim_filtercomponent_colorsegmentation)
        _floatParameters.push_back(0.3f); // max color-color distance
    if (_filterType==sim_filtercomponent_blobextraction)
    {
        _byteParameters.push_back(1); // bit0: use different color for each blob
        _floatParameters.push_back(0.1f); // threshold
        _floatParameters.push_back(0.0f); // min blob size
    }
    if (_filterType==sim_filtercomponent_imagetocoord)
    {
        _intParameters.push_back(32); // number of points along x
        _intParameters.push_back(32); // number of points along y
        _byteParameters.push_back(0); // BitCoded. bit 1 indicates we want overlap with visible pixels
    }
    if (_filterType==sim_filtercomponent_velodyne)
    {
        _intParameters.push_back(32); // number of points along x
        _intParameters.push_back(32); // number of points along y
        _intParameters.push_back(0); // reserved
        _intParameters.push_back(0); // reserved
        _floatParameters.push_back(30.0f*piValue_f/180.0f); // vertical scan angle
        _floatParameters.push_back(0.0f); // reserved
        _floatParameters.push_back(0.0f); // reserved
    }

    if (_filterType==sim_filtercomponent_pixelchange)
    {
        _floatParameters.push_back(0.1f); // threshold in percent
    }
    if (_filterType==sim_filtercomponent_rectangularcut)
    {
        _byteParameters.push_back(0); // bit 0: if set, cut part is copied to buffer 1
        _floatParameters.push_back(0.9f); // x-size (0-1)
        _floatParameters.push_back(0.9f); // y-size (0-1)
    }
    if (_filterType==sim_filtercomponent_resize)
    {
        _floatParameters.push_back(1.2f); // x-scale
        _floatParameters.push_back(1.2f); // y-scale
    }
    if (_filterType==sim_filtercomponent_rotate)
    {
        _floatParameters.push_back(45.0f*degToRad_f); // rotation angle
    }
    if (_filterType==sim_filtercomponent_edge)
        _floatParameters.push_back(0.1f); // threshold (0 to 1)
    if (_filterType==sim_filtercomponent_intensityscale)
    {
        _intParameters.push_back(1); // 0=grey scale, 1=intensity scale
        _floatParameters.push_back(0.0f); // scale start
        _floatParameters.push_back(1.0f); // scale end
    }
    if (_filterType==sim_filtercomponent_binary)
    {
        _byteParameters.push_back(4+8); // 4:trigger enabled, 8:overlay display
        _floatParameters.push_back(0.1f); // threshold
        _floatParameters.push_back(0.0f); // free to use
        _floatParameters.push_back(0.0f); // free to use
        _floatParameters.push_back(0.0f); // free to use
        _floatParameters.push_back(0.5f); // '1' proportion for trigger
        _floatParameters.push_back(0.25f); // '1' proportion tolerance for trigger
        _floatParameters.push_back(0.5f); // x center of mass for trigger
        _floatParameters.push_back(0.25f); // x center of mass tolerance for trigger
        _floatParameters.push_back(0.5f); // y center of mass for trigger
        _floatParameters.push_back(0.25f); // y center of mass tolerance for trigger
        _floatParameters.push_back(0.0f); // orientation for trigger
        _floatParameters.push_back(90.01f*degToRad_f); // orientation tolerance for trigger
        _floatParameters.push_back(0.9f); // roundness for trigger
    }
}

int CSimpleFilter::getFilterType()
{
    if (_filterType==-1)
        _filterType=_getFilterTypeAndNameFromHeaderAndID(_customFilterHeader,_customFilterID,_customFilterName);
    return(_filterType);
}

void CSimpleFilter::setCustomFilterInfo(int header,int id)
{
    _customFilterHeader=header;
    _customFilterID=id;
}

void CSimpleFilter::getCustomFilterInfo(int& header,int& id)
{
    header=_customFilterHeader;
    id=_customFilterID;
}

CSimpleFilter* CSimpleFilter::copyYourself()
{
    CSimpleFilter* newFilter=new CSimpleFilter();
    newFilter->_filterType=_filterType;
    newFilter->_customFilterHeader=_customFilterHeader;
    newFilter->_customFilterID=_customFilterID;
    newFilter->_customFilterName=_customFilterName;
    newFilter->_enabled=_enabled;
    newFilter->_filterVersion=_filterVersion;

    newFilter->_byteParameters.assign(_byteParameters.begin(),_byteParameters.end());
    newFilter->_intParameters.assign(_intParameters.begin(),_intParameters.end());
    newFilter->_floatParameters.assign(_floatParameters.begin(),_floatParameters.end());
    newFilter->_customFilterParameters.assign(_customFilterParameters.begin(),_customFilterParameters.end());
    
    return(newFilter);
}

void CSimpleFilter::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Sft"); // KEEP THIS FIRST!!!!
            ar << _filterType << _customFilterHeader << _customFilterID;
            ar.flush();

            ar.storeDataName("Sfv"); // KEEP THIS SECOND!!!!
            ar << _filterVersion;
            ar.flush();

            ar.storeDataName("Cfn");
            ar << _customFilterName;
            ar.flush();

            if (_byteParameters.size()!=0)
            {
                ar.storeDataName("Pab");
                ar << int(_byteParameters.size());
                for (int i=0;i<int(_byteParameters.size());i++)
                    ar << _byteParameters[i];
                ar.flush();
            }
            if (_intParameters.size()!=0)
            {
                ar.storeDataName("Pai");
                ar << int(_intParameters.size());
                for (int i=0;i<int(_intParameters.size());i++)
                    ar << _intParameters[i];
                ar.flush();
            }
            if (_floatParameters.size()!=0)
            {
                ar.storeDataName("Paf");
                ar << int(_floatParameters.size());
                for (int i=0;i<int(_floatParameters.size());i++)
                    ar << _floatParameters[i];
                ar.flush();
            }
            if (_customFilterParameters.size()!=0)
            {
                ar.storeDataName("Cfp");
                ar << int(_customFilterParameters.size());
                for (int i=0;i<int(_customFilterParameters.size());i++)
                    ar << _customFilterParameters[i];
                ar.flush();
            }

            ar.storeDataName("Sfe");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_enabled);
            ar << nothing;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Sft")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _filterType;
                        setFilterType(_filterType); // to reserve default space of byte-, int- and float-parameters!
                        // above command overwrites header, id and name!!
                        ar >> _customFilterHeader >> _customFilterID;
                    }
                    if (theName.compare("Sfv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _filterVersion;
                    }
                    if (theName.compare("Cfn")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _customFilterName;
                    }
                    if (theName.compare("Pab")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int s;
                        ar >> s;
                        unsigned char ddd;
                        for (int i=0;i<s;i++)
                        {
                            ar >> ddd;
                            if (i<int(_byteParameters.size())) // make sure we use the latest vector size
                                _byteParameters[i]=ddd;
                        }
                    }
                    if (theName.compare("Pai")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int s;
                        ar >> s;
                        int ddd;
                        for (int i=0;i<s;i++)
                        {
                            ar >> ddd;
                            if (i<int(_intParameters.size())) // make sure we use the latest vector size
                                _intParameters[i]=ddd;
                        }
                    }
                    if (theName.compare("Paf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int s;
                        ar >> s;
                        float ddd;
                        for (int i=0;i<s;i++)
                        {
                            ar >> ddd;
                            if (i<int(_floatParameters.size())) // make sure we use the latest vector size
                                _floatParameters[i]=ddd;
                        }
                    }
                    if (theName.compare("Cfp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        int s;
                        ar >> s;
                        _customFilterParameters.clear();
                        unsigned char ddd;
                        for (int i=0;i<s;i++)
                        {
                            ar >> ddd;
                            _customFilterParameters.push_back(ddd);
                        }
                    }
                    if (theName=="Sfe")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _enabled=SIM_IS_BIT_SET(nothing,0);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            // Following is important so that we load the correct name and current filter type according to the extension modules:
            if ((_filterType<0)||(_filterType>=sim_filtercomponent_customized))
                _filterType=_getFilterTypeAndNameFromHeaderAndID(_customFilterHeader,_customFilterID,_customFilterName);
        }
    }
}

std::string CSimpleFilter::getSpecificFilterString(int filterType)
{
    if (filterType==sim_filtercomponent_originalimage)
        return(IDS_FILTER_ORIGINAL_IMAGE_TO_WORK_IMAGE);
    if (filterType==sim_filtercomponent_originaldepth)
        return(IDS_FILTER_ORIGINAL_DEPTH_IMAGE_TO_WORK_IMAGE);
    if (filterType==sim_filtercomponent_uniformimage)
        return(IDS_FILTER_UNIFORM_IMAGE_TO_WORK_IMAGE);
    if (filterType==sim_filtercomponent_tooutput)
        return(IDS_FILTER_WORK_IMAGE_TO_OUTPUT_IMAGE);
    if (filterType==sim_filtercomponent_todepthoutput)
        return(IDS_FILTER_WORK_IMAGE_TO_OUTPUT_DEPTH_IMAGE);
    if (filterType==sim_filtercomponent_tobuffer1)
        return(IDS_FILTER_WORK_IMAGE_TO_BUFFER1);
    if (filterType==sim_filtercomponent_tobuffer2)
        return(IDS_FILTER_WORK_IMAGE_TO_BUFFER2);
    if (filterType==sim_filtercomponent_frombuffer1)
        return(IDS_FILTER_BUFFER1_TO_WORK_IMAGE);
    if (filterType==sim_filtercomponent_frombuffer2)
        return(IDS_FILTER_BUFFER2_TO_WORK_IMAGE);

    if (filterType==sim_filtercomponent_swapwithbuffer1)
        return(IDS_FILTER_SWAP_WORK_WITH_BUFFER1);
    if (filterType==sim_filtercomponent_addtobuffer1)
        return(IDS_FILTER_ADD_WORK_TO_BUFFER1);
    if (filterType==sim_filtercomponent_subtractfrombuffer1)
        return(IDS_FILTER_SUBTRACT_WORK_FROM_BUFFER1);

    if (filterType==sim_filtercomponent_swapbuffers)
        return(IDS_SWAP_BUFFER1_WITH_BUFFER2);
    if (filterType==sim_filtercomponent_addbuffer1)
        return(IDS_FILTER_ADD_BUFFER1_TO_WORK_IMAGE);
    if (filterType==sim_filtercomponent_subtractbuffer1)
        return(IDS_FILTER_SUBTRACT_BUFFER1_FROM_WORK_IMAGE);
    if (filterType==sim_filtercomponent_multiplywithbuffer1)
        return(IDS_FILTER_MULTIPLY_WORK_IMAGE_WITH_BUFFER1);
    if (filterType==sim_filtercomponent_horizontalflip)
        return(IDS_FILTER_FLIP_WORK_IMAGE_HORIZONTALLY);
    if (filterType==sim_filtercomponent_verticalflip)
        return(IDS_FILTER_FLIP_WORK_IMAGE_VERTICALLY);
    if (filterType==sim_filtercomponent_3x3filter)
        return(IDS_FILTER_3X3FILTER_WORK_IMAGE);
    if (filterType==sim_filtercomponent_5x5filter)
        return(IDS_FILTER_5X5FILTER_WORK_IMAGE);
    if (filterType==sim_filtercomponent_rectangularcut)
        return(IDS_FILTER_RECTANGULAR_CUT_WORK_IMAGE);
    if (filterType==sim_filtercomponent_normalize)
        return(IDS_FILTER_NORMALIZE_WORK_IMAGE);
    if (filterType==sim_filtercomponent_colorsegmentation)
        return(IDS_COLOR_SEGMENT_WORK_IMAGE);
    if (filterType==sim_filtercomponent_blobextraction)
        return(IDS_BLOB_DETECTION_WORK_IMAGE);
    if (filterType==sim_filtercomponent_imagetocoord)
        return(IDS_DEPTH_TO_COORD_WORK_IMAGE);
    if (filterType==sim_filtercomponent_velodyne)
        return(IDS_DEPTH_TO_VELODYNE_WORK_IMAGE);
    if (filterType==sim_filtercomponent_pixelchange)
        return(IDS_PIXEL_CHANGE_WORK_IMAGE);
    if (filterType==sim_filtercomponent_intensityscale)
        return(IDS_FILTER_INTENSITY_SCALE_WORK_IMAGE);
    if (filterType==sim_filtercomponent_keeporremovecolors)
        return(IDS_FILTER_SELECTIVE_COLORS_WORK_IMAGE);
    if (filterType==sim_filtercomponent_scaleandoffsetcolors)
        return(IDS_FILTER_OFFSET_AND_SCALE_COLORS_WORK_IMAGE);
    if (filterType==sim_filtercomponent_correlationwithbuffer1)
        return(IDS_FILTER_BUFFER1_AND_WORK_IMAGE_CORRELATION);
    if (filterType==sim_filtercomponent_binary)
        return(IDS_FILTER_BINARY_WORK_IMAGE);
    if (filterType==sim_filtercomponent_sharpen)
        return(IDS_FILTER_SHARPEN_WORK_IMAGE);
    if (filterType==sim_filtercomponent_edge)
        return(IDS_FILTER_EDGE_DETECTION_ON_WORK_IMAGE);
    if (filterType==sim_filtercomponent_shift)
        return(IDS_FILTER_REPOSITION_WORK_IMAGE);
    if (filterType==sim_filtercomponent_circularcut)
        return(IDS_FILTER_CIRCULAR_CUT_WORK_IMAGE);
    if (filterType==sim_filtercomponent_resize)
        return(IDS_FILTER_RESIZE_WORK_IMAGE);
    if (filterType==sim_filtercomponent_rotate)
        return(IDS_FILTER_ROTATE_WORK_IMAGE);
    if (filterType>=sim_filtercomponent_customized)
    {
        std::string retString("EXT: ");
        int ind=filterType-sim_filtercomponent_customized;
        if (ind>=int(_externalFilters.size()))
            return(""); // this will indicate an error to the calling routine
        else
            retString+=_externalFilters[ind].name;
        return(retString);
    }
    if (filterType==-1)
        return(""); // same as above
    return("ERROR");
}

std::string CSimpleFilter::getFilterString()
{
    if (_filterType==-1) // try to reload it
        _filterType=_getFilterTypeAndNameFromHeaderAndID(_customFilterHeader,_customFilterID,_customFilterName);
    std::string retString(getSpecificFilterString(_filterType));
    if ( (retString.length()==0)&&((_filterType>=sim_filtercomponent_customized)||(_filterType==-1) ) )
    { // that extension module was not loaded. Display previous name
        retString="EXT: "+_customFilterName+" (NOT LOADED!)";
    }
    return(retString);
}

int CSimpleFilter::getAvailableFilter(int filterTypeIndex)
{
    int i=0;
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_originalimage);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_originaldepth);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_uniformimage);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_tooutput);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_todepthoutput);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_tobuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_tobuffer2);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_frombuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_frombuffer2);


    if (filterTypeIndex==i++)
        return(sim_filtercomponent_swapwithbuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_swapbuffers);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_addbuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_addtobuffer1);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_subtractbuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_subtractfrombuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_multiplywithbuffer1);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_horizontalflip);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_verticalflip);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_rotate);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_shift);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_resize);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_3x3filter);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_5x5filter);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_sharpen);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_edge);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_correlationwithbuffer1);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_rectangularcut);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_circularcut);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_normalize);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_colorsegmentation);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_intensityscale);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_keeporremovecolors);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_scaleandoffsetcolors);

    if (filterTypeIndex==i++)
        return(sim_filtercomponent_blobextraction);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_binary);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_imagetocoord);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_pixelchange);
    if (filterTypeIndex==i++)
        return(sim_filtercomponent_velodyne);


    // External filters now:
    for (int j=0;j<int(_externalFilters.size());j++)
    {
        if (filterTypeIndex==i++)
            return(sim_filtercomponent_customized+j);
    }
    return(-1);
}


bool CSimpleFilter::canFilterBeEdited(int filterType)
{
    if (filterType==sim_filtercomponent_originalimage)
        return(false);
    if (filterType==sim_filtercomponent_originaldepth)
        return(false);
    if (filterType==sim_filtercomponent_uniformimage)
        return(true);

    if (filterType==sim_filtercomponent_tooutput)
        return(false);
    if (filterType==sim_filtercomponent_todepthoutput)
        return(false);
    if (filterType==sim_filtercomponent_tobuffer1)
        return(false);
    if (filterType==sim_filtercomponent_tobuffer1)
        return(false);

    if (filterType==sim_filtercomponent_frombuffer1)
        return(false);
    if (filterType==sim_filtercomponent_frombuffer2)
        return(false);
    if (filterType==sim_filtercomponent_swapwithbuffer1)
        return(false);
    if (filterType==sim_filtercomponent_addtobuffer1)
        return(false);
    if (filterType==sim_filtercomponent_subtractfrombuffer1)
        return(false);

    if (filterType==sim_filtercomponent_swapbuffers)
        return(false);

    if (filterType==sim_filtercomponent_addbuffer1)
        return(false);
    if (filterType==sim_filtercomponent_subtractbuffer1)
        return(false);
    if (filterType==sim_filtercomponent_multiplywithbuffer1)
        return(false);

    if (filterType==sim_filtercomponent_horizontalflip)
        return(false);
    if (filterType==sim_filtercomponent_verticalflip)
        return(false);
    if (filterType==sim_filtercomponent_rotate)
        return(true);
    if (filterType==sim_filtercomponent_shift)
        return(true);
    if (filterType==sim_filtercomponent_resize)
        return(true);

    if (filterType==sim_filtercomponent_3x3filter)
        return(true);
    if (filterType==sim_filtercomponent_5x5filter)
        return(true);
    if (filterType==sim_filtercomponent_rectangularcut)
        return(true);
    if (filterType==sim_filtercomponent_circularcut)
        return(true);

    if (filterType==sim_filtercomponent_normalize)
        return(false);//true);
    if (filterType==sim_filtercomponent_colorsegmentation)
        return(true);
    if (filterType==sim_filtercomponent_blobextraction)
        return(true);
    if (filterType==sim_filtercomponent_imagetocoord)
        return(true);
    if (filterType==sim_filtercomponent_pixelchange)
        return(true);
    if (filterType==sim_filtercomponent_velodyne)
        return(true);


    if (filterType==sim_filtercomponent_intensityscale)
        return(true);
    if (filterType==sim_filtercomponent_keeporremovecolors)
        return(true);
    if (filterType==sim_filtercomponent_scaleandoffsetcolors)
        return(true);
    if (filterType==sim_filtercomponent_correlationwithbuffer1)
        return(true);

    if (filterType==sim_filtercomponent_sharpen)
        return(false);
    if (filterType==sim_filtercomponent_edge)
        return(true);
    if (filterType==sim_filtercomponent_binary)
        return(true);
    if (filterType>=sim_filtercomponent_customized)
    {
        int ind=filterType-sim_filtercomponent_customized;
        if (ind<int(_externalFilters.size()))
            return(_externalFilters[ind].id>=0);

    }
    return(false); // error, shouldn't happen
}

bool CSimpleFilter::canFilterBeEdited()
{
    return(canFilterBeEdited(_filterType));
}

bool CSimpleFilter::processAndTrigger(CVisionSensor* sensor,int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* outputDepthBuffer,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    if (!_enabled)
        return(false);
    if (_filterType==sim_filtercomponent_originalimage)
        return(processAndTrigger_originalImage(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_originaldepth)
        return(processAndTrigger_originalDepth(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_uniformimage)
        return(processAndTrigger_uniformImage(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_tooutput)
        return(processAndTrigger_toOutput(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_todepthoutput)
        return(processAndTrigger_toDepthOutput(sizeX,sizeY,inputImage,inputDepth,outputImage,outputDepthBuffer,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_tobuffer1)
        return(processAndTrigger_toBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_tobuffer2)
        return(processAndTrigger_toBuffer2(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_frombuffer1)
        return(processAndTrigger_fromBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_frombuffer2)
        return(processAndTrigger_fromBuffer2(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_swapwithbuffer1)
        return(processAndTrigger_swapWithBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_addtobuffer1)
        return(processAndTrigger_addToBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_subtractfrombuffer1)
        return(processAndTrigger_subtractFromBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_swapbuffers)
        return(processAndTrigger_swapBuffers(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_addbuffer1)
        return(processAndTrigger_addBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_subtractbuffer1)
        return(processAndTrigger_subtractBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_multiplywithbuffer1)
        return(processAndTrigger_multiplyWithBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_horizontalflip)
        return(processAndTrigger_horizontalFlip(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_verticalflip)
        return(processAndTrigger_verticalFlip(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_rotate)
        return(processAndTrigger_rotate(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_shift)
        return(processAndTrigger_shift(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_resize)
        return(processAndTrigger_resize(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_3x3filter)
        return(processAndTrigger_3x3filter(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_5x5filter)
        return(processAndTrigger_5x5filter(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_rectangularcut)
        return(processAndTrigger_rectangularCut(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_circularcut)
        return(processAndTrigger_circularCut(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_normalize)
        return(processAndTrigger_normalize(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_colorsegmentation)
        return(processAndTrigger_colorSegmentation(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_intensityscale)
        return(processAndTrigger_intensityScale(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_keeporremovecolors)
        return(processAndTrigger_keepOrRemoveColors(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_scaleandoffsetcolors)
        return(processAndTrigger_scaleAndOffsetColors(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_correlationwithbuffer1)
        return(processAndTrigger_correlationWithBuffer1(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));

    if (_filterType==sim_filtercomponent_sharpen)
        return(processAndTrigger_sharpen(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_edge)
        return(processAndTrigger_edge(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_binary)
        return(processAndTrigger_binary(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_blobextraction)
        return(processAndTrigger_blobExtraction(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_imagetocoord)
        return(processAndTrigger_imageToCoord(sensor,sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_pixelchange)
        return(processAndTrigger_pixelChange(sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    if (_filterType==sim_filtercomponent_velodyne)
        return(processAndTrigger_velodyne(sensor,sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));



    if ((_filterType>=sim_filtercomponent_customized)||(_filterType==-1))
        return(processAndTrigger_customized(sensor->getObjectHandle(),sizeX,sizeY,inputImage,inputDepth,outputImage,workImage,returnData,buffer1,buffer2,drawingContainer));
    return(false); // shouldn't happen!!
}

bool CSimpleFilter::processAndTrigger_originalImage(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        workImage[i]=inputImage[i];
    return(false);
}
bool CSimpleFilter::processAndTrigger_originalDepth(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    for (int i=0;i<s;i++)
    {
        workImage[3*i+0]=inputDepth[i];
        workImage[3*i+1]=inputDepth[i];
        workImage[3*i+2]=inputDepth[i];
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_toOutput(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        outputImage[i]=workImage[i];
    return(false);
}
bool CSimpleFilter::processAndTrigger_toDepthOutput(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* outputDepthBuffer,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    for (int i=0;i<s;i++)
        outputDepthBuffer[i]=(workImage[3*i+0]+workImage[3*i+1]+workImage[3*i+2])/3.0;
    return(false);
}
bool CSimpleFilter::processAndTrigger_toBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        buffer1[i]=workImage[i];
    return(false);
}
bool CSimpleFilter::processAndTrigger_toBuffer2(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        buffer2[i]=workImage[i];
    return(false);
}
bool CSimpleFilter::processAndTrigger_swapBuffers(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float tmp;
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        tmp=buffer1[i];
        buffer1[i]=buffer2[i];
        buffer2[i]=tmp;
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_fromBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        workImage[i]=buffer1[i];
    return(false);
}
bool CSimpleFilter::processAndTrigger_fromBuffer2(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        workImage[i]=buffer2[i];
    return(false);
}

bool CSimpleFilter::processAndTrigger_swapWithBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float tmp;
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        tmp=buffer1[i];
        buffer1[i]=workImage[i];
        workImage[i]=tmp;
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_addToBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        buffer1[i]+=workImage[i];
        if (buffer1[i]>1.0f)
            buffer1[i]=1.0f;
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_subtractFromBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        buffer1[i]-=workImage[i];
        if (buffer1[i]<0.0f)
            buffer1[i]=0.0f;
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_addBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        workImage[i]+=buffer1[i];
        if (workImage[i]>1.0f)
            workImage[i]=1.0f;
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_subtractBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        workImage[i]-=buffer1[i];
        if (workImage[i]<0.0f)
            workImage[i]=0.0f;
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_multiplyWithBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
    {
        workImage[i]*=buffer1[i];
        if (workImage[i]>1.0f)
            workImage[i]=1.0f;
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_horizontalFlip(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float tmp;
    for (int i=0;i<sizeX/2;i++)
    {
        for (int j=0;j<sizeY;j++)
        {
            for (int k=0;k<3;k++)
            {
                tmp=workImage[3*(i+j*sizeX)+k];
                workImage[3*(i+j*sizeX)+k]=workImage[3*((sizeX-1-i)+j*sizeX)+k];
                workImage[3*((sizeX-1-i)+j*sizeX)+k]=tmp;
            }
        }
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_verticalFlip(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float tmp;
    for (int i=0;i<sizeX;i++)
    {
        for (int j=0;j<sizeY/2;j++)
        {
            for (int k=0;k<3;k++)
            {
                tmp=workImage[3*(i+j*sizeX)+k];
                workImage[3*(i+j*sizeX)+k]=workImage[3*(i+(sizeY-1-j)*sizeX)+k];
                workImage[3*(i+(sizeY-1-j)*sizeX)+k]=tmp;
            }
        }
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_uniformImage(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    for (int i=0;i<s;i++)
    {
        workImage[3*i+0]=_floatParameters[0];
        workImage[3*i+1]=_floatParameters[1];
        workImage[3*i+2]=_floatParameters[2];
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_normalize(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY*3;
    float maxCol=0.0f;
    float minCol=1.0f;
    for (int i=0;i<s;i++)
    {
        if (workImage[i]>maxCol)
            maxCol=workImage[i];
        if (workImage[i]<minCol)
            minCol=workImage[i];
    }
    if (maxCol-minCol==0.0f)
        return(false); // uniform image!
    float mul=1.0f/(maxCol-minCol);
    for (int i=0;i<s;i++)
        workImage[i]=(workImage[i]-minCol)*mul;//+_floatParameters[0];
    return(false);
}
bool CSimpleFilter::processAndTrigger_colorSegmentation(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    std::vector<float> goodColors;
    float squaredDistance=_floatParameters[0]*_floatParameters[0];
    for (int i=0;i<s;i++)
    {
        bool found=false;
        for (int j=0;j<int(goodColors.size())/3;j++)
        {
            float r=workImage[3*i+0]-goodColors[3*j+0];
            float g=workImage[3*i+1]-goodColors[3*j+1];
            float b=workImage[3*i+2]-goodColors[3*j+2];
            float d=r*r+g*g+b*b;
            if (d<squaredDistance)
            {
                found=true;
                workImage[3*i+0]=goodColors[3*j+0];
                workImage[3*i+1]=goodColors[3*j+1];
                workImage[3*i+2]=goodColors[3*j+2];
                break;
            }
        }
        if (!found)
        {
            goodColors.push_back(workImage[3*i+0]);
            goodColors.push_back(workImage[3*i+1]);
            goodColors.push_back(workImage[3*i+2]);
        }
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_intensityScale(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float b=_floatParameters[0];
    float a=_floatParameters[1]-b;
    // intensity first transformed like: intensity=a*intensity+b
    int s=sizeX*sizeY;
    float intensity;
    float col[3];
    for (int i=0;i<s;i++)
    {
        intensity=(workImage[3*i+0]+workImage[3*i+1]+workImage[3*i+2])/3.0f;
        intensity=a*intensity+b;
        if (_intParameters[0]==0)
        { // grey scale
            workImage[3*i+0]=intensity;
            workImage[3*i+1]=intensity;
            workImage[3*i+2]=intensity;
        }
        if (_intParameters[0]==1)
        { // intensity scale
            tt::getColorFromIntensity(intensity,0,col);
            workImage[3*i+0]=col[0];
            workImage[3*i+1]=col[1];
            workImage[3*i+2]=col[2];
        }
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_keepOrRemoveColors(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    float col[3];
    float rgb[3];
    float lowTol[3]={_floatParameters[0]-_floatParameters[3],_floatParameters[1]-_floatParameters[4],_floatParameters[2]-_floatParameters[5]};
    float upTol[3]={_floatParameters[0]+_floatParameters[3],_floatParameters[1]+_floatParameters[4],_floatParameters[2]+_floatParameters[5]};
    float lowTolUpHue=1.0f+lowTol[0];
    float upTolLowHue=upTol[0]-1.0f;
    for (int i=0;i<s;i++)
    {
        if (_byteParameters[0]&2)
        { // rgb dimension
            col[0]=workImage[3*i+0];
            col[1]=workImage[3*i+1];
            col[2]=workImage[3*i+2];
        }
        else
        { // hsl dimension
            rgb[0]=workImage[3*i+0];
            rgb[1]=workImage[3*i+1];
            rgb[2]=workImage[3*i+2];
            tt::rgbToHsl(rgb,col);
        }
        bool outOfTol;
        if (_byteParameters[0]&2)
        { // rgb dimension
            outOfTol=((col[0]>upTol[0])||
                (col[0]<lowTol[0])||
                (col[1]>upTol[1])||
                (col[1]<lowTol[1])||
                (col[2]>upTol[2])||
                (col[2]<lowTol[2]));
        }
        else
        { // rgb dimension
            outOfTol=((col[1]>upTol[1])||
                (col[1]<lowTol[1])||
                (col[2]>upTol[2])||
                (col[2]<lowTol[2]));
            if ( (!outOfTol)&&((upTol[0]-lowTol[0])<1.0f) )
            { // Check the Hue value (special handling):
                outOfTol=( (col[0]>upTol[0])&&(col[0]<lowTolUpHue) )||( (col[0]<lowTol[0])&&(col[0]>upTolLowHue) );
            }
        }

        if (outOfTol)
        {
            if (_byteParameters[0]&1)
            { // color not within tolerance, we remove it
                if (_byteParameters[0]&4)
                { // we copy the removed part to buffer 1
                    buffer1[3*i+0]=workImage[3*i+0];
                    buffer1[3*i+1]=workImage[3*i+1];
                    buffer1[3*i+2]=workImage[3*i+2];
                }
                workImage[3*i+0]=0.0f;
                workImage[3*i+1]=0.0f;
                workImage[3*i+2]=0.0f;
            }
            else
            { // color within tolerance
                if (_byteParameters[0]&4)
                { // we mark as black in buffer 1 the parts not removed
                    buffer1[3*i+0]=0.0f;
                    buffer1[3*i+1]=0.0f;
                    buffer1[3*i+2]=0.0f;
                }
            }
        }
        else
        {
            if ((_byteParameters[0]&1)==0)
            { // color not within tolerance, we remove it
                if (_byteParameters[0]&4)
                { // we copy the removed part to buffer 1
                    buffer1[3*i+0]=workImage[3*i+0];
                    buffer1[3*i+1]=workImage[3*i+1];
                    buffer1[3*i+2]=workImage[3*i+2];
                }
                workImage[3*i+0]=0.0f;
                workImage[3*i+1]=0.0f;
                workImage[3*i+2]=0.0f;
            }
            else
            { // color within tolerance
                if (_byteParameters[0]&4)
                { // we mark as black in buffer 1 the parts not removed
                    buffer1[3*i+0]=0.0f;
                    buffer1[3*i+1]=0.0f;
                    buffer1[3*i+2]=0.0f;
                }
            }
        }
    }
    return(false);
}
bool CSimpleFilter::processAndTrigger_scaleAndOffsetColors(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    float col[3];
    float rgb[3];
    for (int i=0;i<s;i++)
    {
        if (_byteParameters[0]&2)
        { // rgb dimension
            col[0]=workImage[3*i+0];
            col[1]=workImage[3*i+1];
            col[2]=workImage[3*i+2];
        }
        else
        { // hsl dimension
            rgb[0]=workImage[3*i+0];
            rgb[1]=workImage[3*i+1];
            rgb[2]=workImage[3*i+2];
            tt::rgbToHsl(rgb,col);
        }
        col[0]=_floatParameters[6]+(col[0]+_floatParameters[0])*_floatParameters[3];
        col[1]=_floatParameters[7]+(col[1]+_floatParameters[1])*_floatParameters[4];
        col[2]=_floatParameters[8]+(col[2]+_floatParameters[2])*_floatParameters[5];
        if (col[0]<0.0f)
            col[0]=0.0f;
        if (col[0]>1.0f)
            col[0]=1.0f;
        if (col[1]<0.0f)
            col[1]=0.0f;
        if (col[1]>1.0f)
            col[1]=1.0f;
        if (col[2]<0.0f)
            col[2]=0.0f;
        if (col[2]>1.0f)
            col[2]=1.0f;
        if (_byteParameters[0]&2)
        { // rgb dimension
            workImage[3*i+0]=col[0];
            workImage[3*i+1]=col[1];
            workImage[3*i+2]=col[2];
        }
        else
        { // hsl dimension
            rgb[0]=workImage[3*i+0];
            rgb[1]=workImage[3*i+1];
            rgb[2]=workImage[3*i+2];
            tt::hslToRgb(col,workImage+3*i+0);
        }
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_binary(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
//  int s=sizeX*sizeY;
    float area=0.0f;
    float proportion=0.0f;
    float cmx=0.0f;
    float cmy=0.0f;
    float angle=0.0f;
    float roundness=1.0f;
    for (int i=0;i<sizeX;i++)
    {
        for (int j=0;j<sizeY;j++)
        {
            float intensity=(workImage[3*(i+j*sizeX)+0]+workImage[3*(i+j*sizeX)+1]+workImage[3*(i+j*sizeX)+2])/3.0f;
            if (intensity>=_floatParameters[0])
            { // Binary 1
                workImage[3*(i+j*sizeX)+0]=1.0f;
                workImage[3*(i+j*sizeX)+1]=1.0f;
                workImage[3*(i+j*sizeX)+2]=1.0f;
                area+=1.0f;
                cmx+=float(i);
                cmy+=float(j);
            }
            else
            { // Binary 0
                workImage[3*(i+j*sizeX)+0]=0.0f;
                workImage[3*(i+j*sizeX)+1]=0.0f;
                workImage[3*(i+j*sizeX)+2]=0.0f;
            }
        }
    }
    proportion=area/float(sizeX*sizeY);
    if (area!=0.0f)
    {
        cmx/=area;
        cmy/=area;

        float a=0.0f;
        float b=0.0f;
        float c=0.0f;
        float tmpX,tmpY;
        for (int i=0;i<sizeX;i++)
        {
            for (int j=0;j<sizeY;j++)
            {
                if (workImage[3*(i+j*sizeX)+0]!=0.0f)
                { // Binary 1
                    tmpX=float(i)-cmx;
                    tmpY=float(j)-cmy;
                    a+=tmpX*tmpX;
                    b+=tmpX*tmpY;
                    c+=tmpY*tmpY;
                }
            }
        }
        b*=2.0f;
        if ((b!=0.0f)||(a!=c))
        {
            float denom=sqrt(b*b+(a-c)*(a-c));
            float sin2ThetaMax=-b/denom;
            float sin2ThetaMin=b/denom;
            float cos2ThetaMax=-(a-c)/denom;
            float cos2ThetaMin=(a-c)/denom;
            float iMax=0.5f*(c+a)-0.5f*(a-c)*cos2ThetaMax-0.5f*b*sin2ThetaMax;
            float iMin=0.5f*(c+a)-0.5f*(a-c)*cos2ThetaMin-0.5f*b*sin2ThetaMin;
            roundness=iMin/iMax;
            float theta=CMath::robustAcos(cos2ThetaMin)*0.5f;
            if (sin2ThetaMin<0.0f)
                theta*=-1.0f;
            angle=theta;
            if (_byteParameters[0]&8)
            { // visualize the direction
                float rcm[2]={cmx/float(sizeX),cmy/float(sizeY)};
                float l=0.3f-roundness*0.25f;
                float redCol[3]={1.0f,0.0f,0.0f};
                float ln[4]={rcm[0]-cos(theta)*l,rcm[1]-sin(theta)*l,rcm[0]+cos(theta)*l,rcm[1]+sin(theta)*l};
                CDrawingObject2D* do2D=new CDrawingObject2D(sim_drawing2d_line|sim_drawing2d_blackbkground,ln,redCol,0.0f,0.0f,nullptr);
                drawingContainer.addObject(do2D);

            }
        }
        if (_byteParameters[0]&8)
        { // visualize the CM
            float rcm[2]={cmx/float(sizeX),cmy/float(sizeY)};
            float greenCol[3]={0.0f,1.0f,0.0f};
            CDrawingObject2D* do2D=new CDrawingObject2D(sim_drawing2d_square|sim_drawing2d_blackbkground|sim_drawing2d_pixelsize,rcm,greenCol,6.0f,0,nullptr);
            drawingContainer.addObject(do2D);
        }
    }
    else
    {
        cmx=0.5f;
        cmy=0.5f;
    }
    returnData.clear();
    returnData.push_back(proportion);
    returnData.push_back(cmx/float(sizeX));
    returnData.push_back(cmy/float(sizeY));
    returnData.push_back(angle);
    returnData.push_back(roundness);
    // Now check if we have to trigger:
    if (_byteParameters[0]&4)
    { // we might have to trigger!
        if (fabs(_floatParameters[4]-proportion)<_floatParameters[5])
        { // within proportions
            if (fabs(_floatParameters[6]-cmx/float(sizeX))<_floatParameters[7])
            { // within cm x-pos
                if (fabs(_floatParameters[8]-cmy/float(sizeY))<_floatParameters[9])
                { // within cm y-pos
                    float d=fabs(tt::getAngleMinusAlpha(_floatParameters[10],angle));
                    if (d<_floatParameters[11])
                    { // within angular tolerance
                        if (roundness<=_floatParameters[12])
                        { // not too round!
                            return(true); // we have to trigger!!
                        }
                    }
                }
            }
        }
    }
    return(false); // we don't trigger
}


bool CSimpleFilter::processAndTrigger_blobExtraction(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int s=sizeX*sizeY;
    const float colorsR[12]={1.0f,0.0f,1.0f,0.0f,1.0f,0.0f,1.0f};
    const float colorsG[12]={0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,1.0f};
    const float colorsB[12]={0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f};

    int* cellAppart=new int[s];
    int currentCellID=0;
    std::vector<std::vector<int>*> cellEquivalencies;
    for (int j=0;j<sizeY;j++)
    {
        for (int i=0;i<sizeX;i++)
        {
            float intensity=(workImage[3*(i+j*sizeX)+0]+workImage[3*(i+j*sizeX)+1]+workImage[3*(i+j*sizeX)+2])/3.0f;
            if (intensity>=_floatParameters[0])
            { // Binary 1
                // Check the 4 neighbours:
                int neighbourCellIDs[4]={99999,99999,99999,99999};
                if (i>0)
                {
                    neighbourCellIDs[0]=cellAppart[(i-1)+j*sizeX];
                    if (j>0)
                        neighbourCellIDs[1]=cellAppart[(i-1)+(j-1)*sizeX];
                }
                if (j>0)
                    neighbourCellIDs[2]=cellAppart[i+(j-1)*sizeX];
                if ((i<sizeX-1)&&(j>0))
                    neighbourCellIDs[3]=cellAppart[(i+1)+(j-1)*sizeX];
                int cellID=neighbourCellIDs[0];
                if (neighbourCellIDs[1]<cellID)
                    cellID=neighbourCellIDs[1];
                if (neighbourCellIDs[2]<cellID)
                    cellID=neighbourCellIDs[2];
                if (neighbourCellIDs[3]<cellID)
                    cellID=neighbourCellIDs[3];
                if (cellID==99999)
                {
                    cellID=currentCellID++;
                    cellEquivalencies.push_back(new std::vector<int>);
                    cellEquivalencies[cellEquivalencies.size()-1]->push_back(cellID);
                }
                else
                {
                    for (int k=0;k<4;k++)
                    {
                        if ( (neighbourCellIDs[k]!=99999)&&(neighbourCellIDs[k]!=cellID) )
                        { // Cell is equivalent!
                            int classCellID=-1;
                            for (int l=0;l<int(cellEquivalencies.size());l++)
                            {
                                for (int m=0;m<int(cellEquivalencies[l]->size());m++)
                                {
                                    if (cellEquivalencies[l]->at(m)==cellID)
                                    {
                                        classCellID=l;
                                        break;
                                    }
                                }
                                if (classCellID!=-1)
                                    break;
                            }


                            int classNeighbourCellID=-1;
                            for (int l=0;l<int(cellEquivalencies.size());l++)
                            {
                                for (int m=0;m<int(cellEquivalencies[l]->size());m++)
                                {
                                    if (cellEquivalencies[l]->at(m)==neighbourCellIDs[k])
                                    {
                                        classNeighbourCellID=l;
                                        break;
                                    }
                                }
                                if (classNeighbourCellID!=-1)
                                    break;
                            }
                            if (classCellID!=classNeighbourCellID)
                            { // We have to merge the two classes:
                                cellEquivalencies[classCellID]->insert(cellEquivalencies[classCellID]->end(),cellEquivalencies[classNeighbourCellID]->begin(),cellEquivalencies[classNeighbourCellID]->end());
                                delete cellEquivalencies[classNeighbourCellID];
                                cellEquivalencies.erase(cellEquivalencies.begin()+classNeighbourCellID);
                            }
                        }
                    }
                }
                cellAppart[i+j*sizeX]=cellID;
            }
            else
            { // Binary 0
                if (_byteParameters[0]&1)
                {
                    workImage[3*(i+j*sizeX)+0]=0.0f;
                    workImage[3*(i+j*sizeX)+1]=0.0f;
                    workImage[3*(i+j*sizeX)+2]=0.0f;
                }
                cellAppart[i+j*sizeX]=99999;
            }
        }
    }
    int* classIDs=new int[currentCellID];
    for (int i=0;i<int(cellEquivalencies.size());i++)
    {
        for (int j=0;j<int(cellEquivalencies[i]->size());j++)
            classIDs[cellEquivalencies[i]->at(j)]=i;
    }
    std::vector<std::vector<float>*> vertices;
    const int BLOBDATSIZE=6;
    float* blobData=new float[BLOBDATSIZE*currentCellID];
    for (int i=0;i<currentCellID;i++)
    {
        vertices.push_back(new std::vector<float>);
        blobData[BLOBDATSIZE*i+0]=0.0f; // the number of pixels
    }


    for (int j=0;j<sizeY;j++)
    {
        for (int i=0;i<sizeX;i++)
        {
            int b=cellAppart[i+j*sizeX];
            if (b!=99999)
            {
                float v=0.8f-float(classIDs[b]/7)*0.2f;
                while (v<0.19f)
                    v+=0.7f;
                if (_byteParameters[0]&1)
                {
                    workImage[3*(i+j*sizeX)+0]=colorsR[classIDs[b]%7]*v;
                    workImage[3*(i+j*sizeX)+1]=colorsG[classIDs[b]%7]*v;
                    workImage[3*(i+j*sizeX)+2]=colorsB[classIDs[b]%7]*v;
                }

                if (    (i==0)||(i==sizeX-1)||(j==0)||(j==sizeY-1)||
                        (cellAppart[(i-1)+j*sizeX]==99999)||(cellAppart[(i+1)+j*sizeX]==99999)||
                        (cellAppart[i+(j-1)*sizeX]==99999)||(cellAppart[i+(j+1)*sizeX]==99999)||
                        (cellAppart[(i-1)+(j-1)*sizeX]==99999)||(cellAppart[(i-1)+(j+1)*sizeX]==99999)||
                        (cellAppart[(i+1)+(j-1)*sizeX]==99999)||(cellAppart[(i+1)+(j+1)*sizeX]==99999) )
                {
                    vertices[classIDs[b]]->push_back(float(i));
                    vertices[classIDs[b]]->push_back(float(j));
                }
                blobData[BLOBDATSIZE*classIDs[b]+0]++;
            }
        }
    }
    
    for (int j=0;j<sizeY;j++)
    {
        for (int i=0;i<sizeX;i++)
        {
            int b=cellAppart[i+j*sizeX];
            if (b!=99999)
            {
                float relSize=blobData[BLOBDATSIZE*classIDs[b]+0]/float(sizeX*sizeY); // relative size of the blob
                if (relSize<_floatParameters[1])
                { // the blob is too small
                    workImage[3*(i+j*sizeX)+0]=0.0;
                    workImage[3*(i+j*sizeX)+1]=0.0;
                    workImage[3*(i+j*sizeX)+2]=0.0;
                }
            }
        }
    }

    int blobCount=0;
    returnData.clear();
    returnData.push_back(float(blobCount)); // We have to actualize this at the end!!!
    returnData.push_back(6.0f);
    for (int i=0;i<int(vertices.size());i++)
    { // For each blob...

        if (vertices[i]->size()!=0)
        {
            float relSize=blobData[BLOBDATSIZE*i+0]/float(sizeX*sizeY); // relative size of the blob
            if (relSize>=_floatParameters[1])
            { // the blob is large enough
                blobCount++;
                float bestOrientation[6]={0.0f,999999999999999999999999999999999999.0f,0.0f,0.0f,0.0f,0.0f};
                float previousDa=0.392699081f; // 22.5 degrees
                for (int j=0;j<4;j++)
                { // Try 4 orientations..
                    float a=previousDa*float(j);
                    float cosa=cos(a);
                    float sina=sin(a);
                    float minV[2]={99999.0f,99999.0f};
                    float maxV[2]={-99999.0f,-99999.0f};
                    for (int j=0;j<int(vertices[i]->size()/2);j++)
                    {
                        float ox=(*vertices[i])[2*j+0];
                        float oy=(*vertices[i])[2*j+1];
                        float x=ox*cosa-oy*sina;
                        float y=ox*sina+oy*cosa;
                        if (x<minV[0])
                            minV[0]=x;
                        if (x>maxV[0])
                            maxV[0]=x;
                        if (y<minV[1])
                            minV[1]=y;
                        if (y>maxV[1])
                            maxV[1]=y;
                    }
                    float s=(maxV[0]-minV[0])*(maxV[1]-minV[1]);
                    if (s<bestOrientation[1])
                    {
                        bestOrientation[0]=a;
                        bestOrientation[1]=s;
                        bestOrientation[2]=maxV[0]-minV[0];
                        bestOrientation[3]=maxV[1]-minV[1];
                        float c[2]={(maxV[0]+minV[0])*0.5f,(maxV[1]+minV[1])*0.5f};
                        bestOrientation[4]=c[0]*cos(-a)-c[1]*sin(-a);
                        bestOrientation[5]=c[0]*sin(-a)+c[1]*cos(-a);
                    }
                }

                for (int k=0;k<3;k++) // the desired precision here
                {
                    previousDa/=3.0f;
                    float bestOrientationFromPreviousStep=bestOrientation[0];
                    for (int j=-2;j<=2;j++)
                    { // Try 4 orientations..
                        if (j!=0)
                        {
                            float a=bestOrientationFromPreviousStep+previousDa*float(j);
                            float cosa=cos(a);
                            float sina=sin(a);
                            float minV[2]={99999.0f,99999.0f};
                            float maxV[2]={-99999.0f,-99999.0f};
                            for (int j=0;j<int(vertices[i]->size()/2);j++)
                            {
                                float ox=(*vertices[i])[2*j+0];
                                float oy=(*vertices[i])[2*j+1];
                                float x=ox*cosa-oy*sina;
                                float y=ox*sina+oy*cosa;
                                if (x<minV[0])
                                    minV[0]=x;
                                if (x>maxV[0])
                                    maxV[0]=x;
                                if (y<minV[1])
                                    minV[1]=y;
                                if (y>maxV[1])
                                    maxV[1]=y;
                            }
                            float s=(maxV[0]-minV[0])*(maxV[1]-minV[1]);
                            if (s<bestOrientation[1])
                            {
                                bestOrientation[0]=a;
                                bestOrientation[1]=s;
                                bestOrientation[2]=maxV[0]-minV[0];
                                bestOrientation[3]=maxV[1]-minV[1];
                                float c[2]={(maxV[0]+minV[0])*0.5f,(maxV[1]+minV[1])*0.5f};
                                bestOrientation[4]=c[0]*cos(-a)-c[1]*sin(-a);
                                bestOrientation[5]=c[0]*sin(-a)+c[1]*cos(-a);
                            }
                        }
                    }

                }

                bestOrientation[0]=-bestOrientation[0]; // it is inversed!
                bestOrientation[4]+=0.5f; // b/c of pixel precision
                bestOrientation[5]+=0.5f; // b/c of pixel precision

                float white[3]={1.0f,1.0f,1.0f};
                float c[2]={bestOrientation[4]/sizeX,bestOrientation[5]/sizeY};
                float v2[2]={bestOrientation[2]*0.5f/sizeX,bestOrientation[3]*0.5f/sizeY};
                float cosa=cos(bestOrientation[0]);
                float sina=sin(bestOrientation[0]);
                float ln[16]={  c[0]+v2[0]*cosa-v2[1]*sina, c[1]+v2[0]*sina+v2[1]*cosa,   c[0]-v2[0]*cosa-v2[1]*sina, c[1]-v2[0]*sina+v2[1]*cosa,
                                c[0]-v2[0]*cosa-v2[1]*sina, c[1]-v2[0]*sina+v2[1]*cosa,   c[0]-v2[0]*cosa+v2[1]*sina, c[1]-v2[0]*sina-v2[1]*cosa,
                                c[0]-v2[0]*cosa+v2[1]*sina, c[1]-v2[0]*sina-v2[1]*cosa,   c[0]+v2[0]*cosa+v2[1]*sina, c[1]+v2[0]*sina-v2[1]*cosa,
                                c[0]+v2[0]*cosa+v2[1]*sina, c[1]+v2[0]*sina-v2[1]*cosa,   c[0]+v2[0]*cosa-v2[1]*sina, c[1]+v2[0]*sina+v2[1]*cosa};
                CDrawingObject2D* do2D=new CDrawingObject2D(sim_drawing2d_line|sim_drawing2d_blackbkground,ln,white,0.0f,0.0f,nullptr);
                drawingContainer.addObject(do2D);
                do2D=new CDrawingObject2D(sim_drawing2d_line|sim_drawing2d_blackbkground,ln+4,white,0.0f,0.0f,nullptr);
                drawingContainer.addObject(do2D);
                do2D=new CDrawingObject2D(sim_drawing2d_line|sim_drawing2d_blackbkground,ln+8,white,0.0f,0.0f,nullptr);
                drawingContainer.addObject(do2D);
                do2D=new CDrawingObject2D(sim_drawing2d_line|sim_drawing2d_blackbkground,ln+12,white,0.0f,0.0f,nullptr);
                drawingContainer.addObject(do2D);
                std::string txt;
                txt=boost::lexical_cast<std::string>(blobCount);

                do2D=new CDrawingObject2D(sim_drawing2d_text,c,white,0.02f,0.0f,(char*)txt.c_str());
                drawingContainer.addObject(do2D);

                returnData.push_back(relSize);
                returnData.push_back(bestOrientation[0]);
                returnData.push_back(c[0]);
                returnData.push_back(c[1]);
                returnData.push_back(v2[0]*2.0f);
                returnData.push_back(v2[1]*2.0f);
            }
        }
    }

    delete[] blobData;
    for (int i=0;i<int(vertices.size());i++)
        delete vertices[i]; 

    delete[] classIDs;
    for (int i=0;i<int(cellEquivalencies.size());i++)
        delete cellEquivalencies[i];

    delete[] cellAppart;

    returnData[0]=float(blobCount);
    // the other return data is filled-in here above!

    return(false); // we don't trigger
}

bool CSimpleFilter::processAndTrigger_sharpen(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float m[9]={-0.1111f,-0.1111f,-0.1111f,  -0.1111f,+1.8888f,-0.1111f,  -0.1111f,-0.1111f,-0.1111f};
    float* im2=CImageProcess::createRGBImage(sizeX,sizeY);
    CImageProcess::filter3x3RgbImage(sizeX,sizeY,workImage,im2,m);
    CImageProcess::copyRGBImage(sizeX,sizeY,im2,workImage);
    CImageProcess::clampRgbImage(sizeX,sizeY,workImage,0.0f,1.0f);
    CImageProcess::deleteImage(im2);
    return(false);
}
bool CSimpleFilter::processAndTrigger_edge(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float m[9]={-3.0f,-3.0f,-3.0f,  -3.0f,24.0f,-3.0f,  -3.0f,-3.0f,-3.0f};
    float* im2=CImageProcess::createRGBImage(sizeX,sizeY);
    CImageProcess::filter3x3RgbImage(sizeX,sizeY,workImage,im2,m);
    CImageProcess::copyRGBImage(sizeX,sizeY,im2,workImage);
    int s=sizeX*sizeY;
    for (int i=0;i<s;i++)
    {
        float intens=(workImage[3*i+0]+workImage[3*i+1]+workImage[3*i+2])/3.0f;
        if (intens>_floatParameters[0])
        {
            workImage[3*i+0]=1.0f;
            workImage[3*i+1]=1.0f;
            workImage[3*i+2]=1.0f;
        }
        else
        {
            workImage[3*i+0]=0.0f;
            workImage[3*i+1]=0.0f;
            workImage[3*i+2]=0.0f;
        }
    }
    CImageProcess::deleteImage(im2);
    return(false);
}


bool CSimpleFilter::processAndTrigger_correlationWithBuffer1(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float winSize[2]={_floatParameters[0],_floatParameters[1]};
    float frameSize[2]={tt::getLimitedFloat(winSize[0],1.0f,_floatParameters[2]),tt::getLimitedFloat(winSize[1],1.0f,_floatParameters[3])};
    int winSizei[2]={int(winSize[0]*sizeX+0.499f),int(winSize[1]*sizeY+0.499f)};
    if (winSizei[0]<1)
        winSizei[0]=1;
    if (winSizei[1]<1)
        winSizei[1]=1;
    int frameSizei[2]={int(frameSize[0]*sizeX+0.499f),int(frameSize[1]*sizeY+0.499f)};
    int borderSize[2]={((frameSizei[0]-winSizei[0])/2)&0xfffe,((frameSizei[1]-winSizei[1])/2)&0xfffe};
    frameSizei[0]=borderSize[0]*2+1;
    frameSizei[1]=borderSize[1]*2+1;
    int framePos[2]={(sizeX-winSizei[0]-2*borderSize[0])/2,(sizeY-winSizei[1]-2*borderSize[1])/2};

    float* coIm=CImageProcess::createIntensityImage(2*borderSize[0]+1,2*borderSize[1]+1);
    int coIms=(2*borderSize[0]+1)*(2*borderSize[1]+1);
    for (int i=0;i<coIms;i++)
        coIm[i]=0.0f;
    float minMaxV[2]={SIM_MAX_FLOAT,0.0f};
    if (_byteParameters[0]&16)
    { // We want greyscale calculations (a bit faster)
        int sss=sizeX*sizeY;
        for (int i=0;i<sss;i++)
        { // we put work image intensity to its red component, and buffer1 intensity to work image green component:
            workImage[3*i+0]=(workImage[3*i+0]+workImage[3*i+1]+workImage[3*i+2])/3.0f;
            workImage[3*i+1]=(buffer1[3*i+0]+buffer1[3*i+1]+buffer1[3*i+2])/3.0f;
        }
    }
    for (int displX=-borderSize[0];displX<=borderSize[0];displX++)
    {
        for (int displY=-borderSize[1];displY<=borderSize[1];displY++)
        {
            float c=0.0f;
            for (int i=0;i<winSizei[0];i++)
            {
                int currentX=framePos[0]+borderSize[0]+i;
                int previousX=currentX+displX;
                for (int j=0;j<winSizei[1];j++)
                {
                    int currentY=framePos[1]+borderSize[1]+j;
                    int previousY=currentY+displY;
                    float d;
                    int currentInd=3*(currentX+currentY*sizeX);
                    int previousInd=3*(previousX+previousY*sizeX);
                    if ((_byteParameters[0]&16)==0)
                    { // color calculations
                        d=workImage[currentInd+0]-buffer1[previousInd+0];
                        c+=d*d;
                        d=workImage[currentInd+1]-buffer1[previousInd+1];
                        c+=d*d;
                        d=workImage[currentInd+2]-buffer1[previousInd+2];
                        c+=d*d;
                    }
                    else
                    { // intensity calculations
                        d=workImage[currentInd+0]-workImage[previousInd+1];
                        c+=d*d;
                    }
                }
            }
            coIm[displX+borderSize[0]+(displY+borderSize[1])*frameSizei[0]]=c;

            if ( ( ((sizeX&1)!=0)||(displX!=0) )&&( ((sizeY&1)!=0)||(displY!=0) ) )
            { // above condition important for later (we remove the center if even image size)
                if (c<minMaxV[0])
                    minMaxV[0]=c;
                if (c>minMaxV[1])
                    minMaxV[1]=c;
            }

        }
    }
    // Clear the work image:
    int s=sizeX*sizeY*3;
    for (int i=0;i<s;i++)
        workImage[i]=0.0f;
    int correctionX,correctionY;
    if (minMaxV[1]-minMaxV[0]!=0.0f)
    {
        for (int displX=-borderSize[0];displX<=borderSize[0];displX++)
        {
            if ( ((sizeX&1)==0)&&(displX>0) )
                correctionX=-1;
            else
                correctionX=0;
            for (int displY=-borderSize[1];displY<=borderSize[1];displY++)
            {
                if ( ((sizeY&1)==0)&&(displY>0) )
                    correctionY=-1;
                else
                    correctionY=0;
                float v=coIm[displX+borderSize[0]+(displY+borderSize[1])*frameSizei[0]];
                v=1.0f-(v-minMaxV[0])/(minMaxV[1]-minMaxV[0]);
                int ind=3*(sizeX/2+displX+correctionX+(sizeY/2+displY+correctionY)*sizeX);
                workImage[ind+0]=v;
                workImage[ind+1]=v;
                workImage[ind+2]=v;
            }
        }
    }

    if (_byteParameters[0]&8)
    {
        float rcm[2]={0.5f,0.5f};
        float redCol[3]={1.0f,0.0f,0.0f};
        CDrawingObject2D* do2D=new CDrawingObject2D(sim_drawing2d_circle|sim_drawing2d_blackbkground|sim_drawing2d_pixelsize,rcm,redCol,20.0f,0,nullptr);
        drawingContainer.addObject(do2D);
    }

    CImageProcess::deleteImage(coIm);
    return(false);
}

bool CSimpleFilter::processAndTrigger_shift(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float* im2=CImageProcess::createRGBImage(sizeX,sizeY);
    CImageProcess::copyRGBImage(sizeX,sizeY,workImage,im2);
    float xShiftT=_floatParameters[0]*float(sizeX);
    float yShiftT=_floatParameters[1]*float(sizeY);
    if (xShiftT>=0.0f)
        xShiftT+=0.5f;
    else
        xShiftT-=0.5f;
    if (yShiftT>=0.0f)
        yShiftT+=0.5f;
    else
        yShiftT-=0.5f;
    int xShift=int(xShiftT);
    int yShift=int(yShiftT);
    if (xShift>sizeX)
        xShift=sizeX;
    if (xShift<-sizeX)
        xShift=-sizeX;
    if (yShift>sizeY)
        yShift=sizeY;
    if (yShift<-sizeY)
        yShift=-sizeY;
    int cpx,cpy;
    int ppx,ppy;
    for (int i=0;i<sizeX;i++)
    {
        cpx=i+xShift;
        ppx=cpx;
        if (ppx>=sizeX)
            ppx-=sizeX;
        if (ppx<0)
            ppx+=sizeX;
        for (int j=0;j<sizeY;j++)
        {
            cpy=j+yShift;
            ppy=cpy;
            if (ppy>=sizeY)
                ppy-=sizeY;
            if (ppy<0)
                ppy+=sizeY;
            if (((cpx<0)||(cpx>=sizeX)||(cpy<0)||(cpy>=sizeY))&&_byteParameters[0])
            { // we remove that area (black)
                workImage[3*(ppx+ppy*sizeX)+0]=0.0f;
                workImage[3*(ppx+ppy*sizeX)+1]=0.0f;
                workImage[3*(ppx+ppy*sizeX)+2]=0.0f;
            }
            else
            {
                workImage[3*(ppx+ppy*sizeX)+0]=im2[3*(i+j*sizeX)+0];
                workImage[3*(ppx+ppy*sizeX)+1]=im2[3*(i+j*sizeX)+1];
                workImage[3*(ppx+ppy*sizeX)+2]=im2[3*(i+j*sizeX)+2];
            }
        }
    }
    CImageProcess::deleteImage(im2);
    return(false);
}

bool CSimpleFilter::processAndTrigger_circularCut(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    int smallestSize=SIM_MIN(sizeX,sizeY);
    float centerX=float(sizeX)/2.0f;
    float centerY=float(sizeY)/2.0f;
    float radSquared=_floatParameters[0]*float(smallestSize)*0.5f;
    radSquared*=radSquared;
    float dx,dy;
    for (int i=0;i<sizeX;i++)
    {
        dx=float(i)+0.5f-centerX;
        dx*=dx;
        for (int j=0;j<sizeY;j++)
        {
            dy=float(j)+0.5f-centerY;
            dy*=dy;
            if (dy+dx>radSquared)
            {
                if (_byteParameters[0]&1)
                {
                    buffer1[3*(i+j*sizeX)+0]=workImage[3*(i+j*sizeX)+0];
                    buffer1[3*(i+j*sizeX)+1]=workImage[3*(i+j*sizeX)+1];
                    buffer1[3*(i+j*sizeX)+2]=workImage[3*(i+j*sizeX)+2];
                }
                workImage[3*(i+j*sizeX)+0]=0.0f;
                workImage[3*(i+j*sizeX)+1]=0.0f;
                workImage[3*(i+j*sizeX)+2]=0.0f;
            }
            else
            {
                if (_byteParameters[0]&1)
                {
                    buffer1[3*(i+j*sizeX)+0]=0.0f;
                    buffer1[3*(i+j*sizeX)+1]=0.0f;
                    buffer1[3*(i+j*sizeX)+2]=0.0f;
                }
            }
        }
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_resize(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float* im=CImageProcess::createRGBImage(sizeX,sizeY);
    CImageProcess::copyRGBImage(sizeX,sizeY,workImage,im);
    float* cntIm=CImageProcess::createIntensityImage(sizeX,sizeY);
    int s=sizeX*sizeY;
    for (int i=0;i<s;i++)
    {
        cntIm[i]=0.0f;
        workImage[3*i+0]=0.0f;
        workImage[3*i+1]=0.0f;
        workImage[3*i+2]=0.0f;
    }
    float centerX=float(sizeX)/2.0f;
    float centerY=float(sizeY)/2.0f;
    float dx,dy;
    int npx,npy;
    for (int i=0;i<sizeX;i++)
    {
        dx=(float(i)+0.5f-centerX)/_floatParameters[0];
        npx=int(centerX+dx);    
        if ((npx>=0)&&(npx<sizeX))
        {
            for (int j=0;j<sizeY;j++)
            {
                dy=(float(j)+0.5f-centerY)/_floatParameters[1];
                npy=int(centerY+dy);    
                if ((npy>=0)&&(npy<sizeY))
                {
                    workImage[3*(i+j*sizeX)+0]+=im[3*(npx+npy*sizeX)+0];
                    workImage[3*(i+j*sizeX)+1]+=im[3*(npx+npy*sizeX)+1];
                    workImage[3*(i+j*sizeX)+2]+=im[3*(npx+npy*sizeX)+2];
                    cntIm[i+j*sizeX]+=1.0f;
                }
            }
        }
    }
    for (int i=0;i<s;i++)
    {
        if (cntIm[i]>0.1f)
        {
            workImage[3*i+0]/=cntIm[i];
            workImage[3*i+1]/=cntIm[i];
            workImage[3*i+2]/=cntIm[i];
        }
    }
    CImageProcess::deleteImage(cntIm);
    CImageProcess::deleteImage(im);
    return(false);
}

bool CSimpleFilter::processAndTrigger_rotate(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float* im=CImageProcess::createRGBImage(sizeX,sizeY);
    CImageProcess::copyRGBImage(sizeX,sizeY,workImage,im);
    int ss=sizeX*sizeY*3;
    for (int i=0;i<ss;i++)
        workImage[i]=0.0f;
    float centerX=float(sizeX)/2.0f;
    float centerY=float(sizeY)/2.0f;
    float dx,dy;
    int npx,npy;
    float dxp0;
    float dxp1;
    float dyp0;
    float dyp1;
    float c=cos(-_floatParameters[0]);
    float s=sin(-_floatParameters[0]);
    for (int i=0;i<sizeX;i++)
    {
        dx=float(i)+0.5f-centerX;
        dxp0=dx*c;
        dyp0=dx*s;
        for (int j=0;j<sizeY;j++)
        {
            dy=float(j)+0.5f-centerY;
            dxp1=-dy*s;
            dyp1=dy*c;
            npx=int(centerX+dxp0+dxp1);
            npy=int(centerY+dyp0+dyp1); 
            if ((npy>=0)&&(npy<sizeY)&&(npx>=0)&&(npx<sizeX))
            {
                workImage[3*(i+j*sizeX)+0]+=im[3*(npx+npy*sizeX)+0];
                workImage[3*(i+j*sizeX)+1]+=im[3*(npx+npy*sizeX)+1];
                workImage[3*(i+j*sizeX)+2]+=im[3*(npx+npy*sizeX)+2];
            }
        }
    }
    CImageProcess::deleteImage(im);
    return(false);
}

bool CSimpleFilter::processAndTrigger_3x3filter(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float m[9];
    for (int i=0;i<9;i++)
        m[i]=_floatParameters[i]*_floatParameters[9]; // don't forget the multiplier!!
    float* im2=CImageProcess::createRGBImage(sizeX,sizeY);
    for (int i=0;i<_intParameters[0]/2;i++)
    {
        CImageProcess::filter3x3RgbImage(sizeX,sizeY,workImage,im2,m);
        CImageProcess::filter3x3RgbImage(sizeX,sizeY,im2,workImage,m);
    }
    if (_intParameters[0]&1)
    {
        CImageProcess::filter3x3RgbImage(sizeX,sizeY,workImage,im2,m);
        CImageProcess::copyRGBImage(sizeX,sizeY,im2,workImage);
    }
    CImageProcess::clampRgbImage(sizeX,sizeY,workImage,0.0f,1.0f);
    CImageProcess::deleteImage(im2);
    return(false);
}

bool CSimpleFilter::processAndTrigger_5x5filter(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float m[25];
    for (int i=0;i<25;i++)
        m[i]=_floatParameters[i]*_floatParameters[25]; // don't forget the multiplier!!
    float* im2=CImageProcess::createRGBImage(sizeX,sizeY);
    for (int i=0;i<_intParameters[0]/2;i++)
    {
        CImageProcess::filter5x5RgbImage(sizeX,sizeY,workImage,im2,m);
        CImageProcess::filter5x5RgbImage(sizeX,sizeY,im2,workImage,m);
    }
    if (_intParameters[0]&1)
    {
        CImageProcess::filter5x5RgbImage(sizeX,sizeY,workImage,im2,m);
        CImageProcess::copyRGBImage(sizeX,sizeY,im2,workImage);
    }
    CImageProcess::clampRgbImage(sizeX,sizeY,workImage,0.0f,1.0f);
    CImageProcess::deleteImage(im2);
    return(false);
}

bool CSimpleFilter::processAndTrigger_rectangularCut(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float centerX=float(sizeX)/2.0f;
    float centerY=float(sizeY)/2.0f;
    float dx,dy;
    float hdx=_floatParameters[0]*0.5f;
    float hdy=_floatParameters[1]*0.5f;
    for (int i=0;i<sizeX;i++)
    {
        dx=float(i)+0.5f-centerX;
        for (int j=0;j<sizeY;j++)
        {
            dy=float(j)+0.5f-centerY;
            if ((fabs(dx)>hdx*float(sizeX))||(fabs(dy)>hdy*float(sizeY)))
            {
                if (_byteParameters[0]&1)
                {
                    buffer1[3*(i+j*sizeX)+0]=workImage[3*(i+j*sizeX)+0];
                    buffer1[3*(i+j*sizeX)+1]=workImage[3*(i+j*sizeX)+1];
                    buffer1[3*(i+j*sizeX)+2]=workImage[3*(i+j*sizeX)+2];
                }
                workImage[3*(i+j*sizeX)+0]=0.0f;
                workImage[3*(i+j*sizeX)+1]=0.0f;
                workImage[3*(i+j*sizeX)+2]=0.0f;
            }
            else
            {
                if (_byteParameters[0]&1)
                {
                    buffer1[3*(i+j*sizeX)+0]=0.0f;
                    buffer1[3*(i+j*sizeX)+1]=0.0f;
                    buffer1[3*(i+j*sizeX)+2]=0.0f;
                }
            }
        }
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_customized(int objectHandle,int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    void* ptrs[7]={(void*)inputImage,(void*)inputDepth,workImage,buffer1,buffer2,outputImage,nullptr};
    if (_customFilterParameters.size()!=0)
        ptrs[6]=&_customFilterParameters[0];
    int auxData[5]={_customFilterHeader,_customFilterID,sizeX,sizeY,objectHandle};
    int retVals[4]={-1,-1,-1,-1};
    void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_imagefilter_process,auxData,ptrs,retVals);
    if ((retVals[1]!=0)&&(retVal!=nullptr))
    { // we register the auxiliary information (that could be a vector, a direction, etc.)
        returnData.clear();
        for (int i=0;i<retVals[1];i++)
            returnData.push_back(((float*)retVal)[i]);
    }
    delete[] (char*)retVal;
    return(retVals[0]>0);
}


bool CSimpleFilter::processAndTrigger_imageToCoord(CVisionSensor* sensor,int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float depthThresh=sensor->getNearClippingPlane();
    float depthRange=sensor->getFarClippingPlane()-depthThresh;
    float farthestValue=sensor->getFarClippingPlane();
    float xAngle=sensor->getViewAngle();
    float yAngle=xAngle;
    float ratio=float(sizeX)/float(sizeY);
    if (sizeX>sizeY)
        yAngle=2.0f*(float)atan(tan(xAngle/2.0f)/ratio);
    else
        xAngle=2.0f*(float)atan(tan(xAngle/2.0f)*ratio);

    float xS=sensor->getOrthoViewSize();
    float yS=xS;
    if (sizeX>sizeY)
        yS=xS/ratio;
    else
        xS=xS*ratio;


    int xPtCnt=_intParameters[0];
    int yPtCnt=_intParameters[1];
    bool angleDivision=(_byteParameters[0]&1)==0;

    returnData.clear();
    returnData.push_back(float(xPtCnt));
    returnData.push_back(float(yPtCnt));


    if (sensor->getPerspectiveOperation())
    {
        float yDist=0.0f;
        float dy=0.0f;
        if (yPtCnt>1)
        {
            dy=yAngle/float(yPtCnt-1);
            yDist=-yAngle*0.5f;
        }
        float dx=0.0f;
        if (xPtCnt>1)
            dx=xAngle/float(xPtCnt-1);

        float xAlpha=0.5f/(tan(xAngle*0.5f));
        float yAlpha=0.5f/(tan(yAngle*0.5f));

        float xBeta=2.0f*tan(xAngle*0.5f);
        float yBeta=2.0f*tan(yAngle*0.5f);

        for (int j=0;j<yPtCnt;j++)
        {
            float tanYDistTyAlpha;
            int yRow;
            if (angleDivision)
            {
                tanYDistTyAlpha=tan(yDist)*yAlpha;
                yRow=int((tanYDistTyAlpha+0.5f)*(sizeY-0.5f));
            }
            else
                yRow=int((0.5f+yDist/yAngle)*(sizeY-0.5f));

            float xDist=0.0f;
            if (xPtCnt>1)
                xDist=-xAngle*0.5f;
            for (int i=0;i<xPtCnt;i++)
            {
                C3Vector v;
                if (angleDivision)
                {
                    float tanXDistTxAlpha=tan(xDist)*xAlpha;
                    int xRow=int((0.5f-tanXDistTxAlpha)*(sizeX-0.5f));
                    int indexP=3*(xRow+yRow*sizeX);
                    float intensity=(workImage[indexP+0]+workImage[indexP+1]+workImage[indexP+2])/3.0f;
                    float zDist=depthThresh+intensity*depthRange;
                    v.set(tanXDistTxAlpha*xBeta*zDist,tanYDistTyAlpha*yBeta*zDist,zDist);
                }
                else
                {
                    int xRow=int((0.5f-xDist/xAngle)*(sizeX-0.5f));
                    int indexP=3*(xRow+yRow*sizeX);
                    float intensity=(workImage[indexP+0]+workImage[indexP+1]+workImage[indexP+2])/3.0f;
                    float zDist=depthThresh+intensity*depthRange;
                    v.set(tan(xAngle*0.5f)*xDist/(xAngle*0.5f)*zDist,tan(yAngle*0.5f)*yDist/(yAngle*0.5f)*zDist,zDist);
                }

                float l=v.getLength();
                if (l>farthestValue)
                {
                    v=(v/l)*farthestValue;
                    returnData.push_back(v(0));
                    returnData.push_back(v(1));
                    returnData.push_back(v(2));
                    returnData.push_back(farthestValue);
                }
                else
                {
                    returnData.push_back(v(0));
                    returnData.push_back(v(1));
                    returnData.push_back(v(2));
                    returnData.push_back(l);
                }
                xDist+=dx;
            }
            yDist+=dy;
        }
    }
    else
    {
        float yDist=0.0f;
        float dy=0.0f;
        if (yPtCnt>1)
        {
            dy=yS/float(yPtCnt-1);
            yDist=-yS*0.5f;
        }
        float dx=0.0f;
        if (xPtCnt>1)
            dx=xS/float(xPtCnt-1);

        for (int j=0;j<yPtCnt;j++)
        {
            int yRow=int(((yDist+yS*0.5f)/yS)*(sizeY-0.5f));

            float xDist=0.0f;
            if (xPtCnt>1)
                xDist=-xS*0.5f;
            for (int i=0;i<xPtCnt;i++)
            {
                int xRow=int((1.0f-((xDist+xS*0.5f)/xS))*(sizeX-0.5f));
                int indexP=3*(xRow+yRow*sizeX);
                float intensity=(workImage[indexP+0]+workImage[indexP+1]+workImage[indexP+2])/3.0f;
                float zDist=depthThresh+intensity*depthRange;
                returnData.push_back(xDist);
                returnData.push_back(yDist);
                returnData.push_back(zDist);
                returnData.push_back(zDist);
                xDist+=dx;
            }
            yDist+=dy;
        }
    }

    return(false); // we don't trigger
}

bool CSimpleFilter::processAndTrigger_pixelChange(int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float thresh=_floatParameters[0];
    for (int j=0;j<sizeY;j++)
    {
        for (int i=0;i<sizeX;i++)
        {
            float buffIntens=(buffer1[3*(i+j*sizeX)+0]+buffer1[3*(i+j*sizeX)+1]+buffer1[3*(i+j*sizeX)+2])/3.0f;
            float imgIntens=(workImage[3*(i+j*sizeX)+0]+workImage[3*(i+j*sizeX)+1]+workImage[3*(i+j*sizeX)+2])/3.0f;
            float diff=imgIntens-buffIntens;
            if (fabs(diff)/buffIntens>=thresh)
            {
                buffer1[3*(i+j*sizeX)+0]=imgIntens;
                buffer1[3*(i+j*sizeX)+1]=imgIntens;
                buffer1[3*(i+j*sizeX)+2]=imgIntens;
                if (diff>0)
                {
                    workImage[3*(i+j*sizeX)+0]=1.0f;
                    workImage[3*(i+j*sizeX)+1]=1.0f;
                    workImage[3*(i+j*sizeX)+2]=1.0f;
                    returnData.push_back(1.0f);
                }
                else
                {
                    workImage[3*(i+j*sizeX)+0]=0.0f;
                    workImage[3*(i+j*sizeX)+1]=0.0f;
                    workImage[3*(i+j*sizeX)+2]=0.0f;
                    returnData.push_back(-1.0f);
                }
                returnData.push_back(float(i));
                returnData.push_back(float(j));
            }
            else
            {
                workImage[3*(i+j*sizeX)+0]=0.5f;
                workImage[3*(i+j*sizeX)+1]=0.5f;
                workImage[3*(i+j*sizeX)+2]=0.5f;
            }
        }
    }
    return(false);
}

bool CSimpleFilter::processAndTrigger_velodyne(CVisionSensor* sensor,int sizeX,int sizeY,const float* inputImage,const float* inputDepth,float* outputImage,float* workImage,std::vector<float>& returnData,float* buffer1,float* buffer2,CDrawingContainer2D& drawingContainer)
{
    float depthThresh=sensor->getNearClippingPlane();
    float depthRange=sensor->getFarClippingPlane()-depthThresh;
    float farthestValue=sensor->getFarClippingPlane();
    float xAngle=sensor->getViewAngle();
    float yAngle=xAngle;
    float ratio=float(sizeX)/float(sizeY);
    if (sizeX>sizeY)
        yAngle=2.0f*(float)atan(tan(xAngle/2.0f)/ratio);
    else
        xAngle=2.0f*(float)atan(tan(xAngle/2.0f)*ratio);

    int xPtCnt=_intParameters[0];
    int yPtCnt=_intParameters[1];
    float vAngle=_floatParameters[0];

    returnData.clear();
    returnData.push_back(float(xPtCnt));
    returnData.push_back(float(yPtCnt));

    //if (sensor->getPerspectiveOperation())
    {
        float dx=0.0f;
        if (xPtCnt>1)
            dx=xAngle/float(xPtCnt-1);
        float xDist=0.0f;
        if (xPtCnt>1)
            xDist=-xAngle*0.5f;

        float xAlpha=0.5f/(tan(xAngle*0.5f));

        float xBeta=2.0f*tan(xAngle*0.5f);
        float yBeta=2.0f*tan(yAngle*0.5f);

        for (int j=0;j<xPtCnt;j++)
        {
            float h=1.0f/cos(xDist);

            float yDist=0.0f;
            float dy=0.0f;
            if (yPtCnt>1)
            {
                dy=vAngle/float(yPtCnt-1);
                yDist=-vAngle*0.5f;
            }

            float tanXDistTxAlpha=tan(xDist)*xAlpha;
            int xRow=int((0.5f-tanXDistTxAlpha)*(sizeX-0.5f));

            if (xRow<0)
                xRow=0;
            if (xRow>=sizeX)
                xRow=sizeX-1;

            float yAlpha=0.5f/(tan(yAngle*0.5f));

            for (int i=0;i<yPtCnt;i++)
            {
                float tanYDistTyAlpha=tan(yDist)*h*yAlpha;
                int yRow=int((tanYDistTyAlpha+0.5f)*(sizeY-0.5f));
                if (yRow<0)
                    yRow=0;
                if (yRow>=sizeY)
                    yRow=sizeY-1;
                int indexP=3*(xRow+yRow*sizeX);
                float intensity=(workImage[indexP+0]+workImage[indexP+1]+workImage[indexP+2])/3.0f;
                float zDist=depthThresh+intensity*depthRange;
                C3Vector v(tanXDistTxAlpha*xBeta*zDist,tanYDistTyAlpha*yBeta*zDist,zDist);
                float l=v.getLength();
                if (l>farthestValue)
                {
                    v=(v/l)*farthestValue;
                    returnData.push_back(v(0));
                    returnData.push_back(v(1));
                    returnData.push_back(v(2));
                    returnData.push_back(farthestValue);
                }
                else
                {
                    returnData.push_back(v(0));
                    returnData.push_back(v(1));
                    returnData.push_back(v(2));
                    returnData.push_back(l);
                }
                yDist+=dy;
            }
            xDist+=dx;
        }
    }

    return(false); // we don't trigger
}
