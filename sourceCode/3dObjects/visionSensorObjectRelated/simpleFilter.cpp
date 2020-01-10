// Needed for backward compatibility

#include "simpleFilter.h"
#include "simConst.h"
#include "MyMath.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

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

void CSimpleFilter::setFilterType(int t)
{ // Is called during loading!! (keep simple)
    _filterType=t;
    _byteParameters.clear();
    _floatParameters.clear();
    _intParameters.clear();

    if (_filterType>=sim_filtercomponent_customized_deprecated)
        _filterType=-1;

    // Now set default parameter values
    if (_filterType==sim_filtercomponent_uniformimage_deprecated)
    {
        _floatParameters.push_back(1.0f); // r
        _floatParameters.push_back(1.0f); // g
        _floatParameters.push_back(1.0f); // b
    }
    if (_filterType==sim_filtercomponent_3x3filter_deprecated)
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
    if (_filterType==sim_filtercomponent_5x5filter_deprecated)
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
    if (_filterType==sim_filtercomponent_keeporremovecolors_deprecated)
    {
        _byteParameters.push_back(1+2); // bit 0: keep (otherwise remove), bit 1: rgb (otherwise hsl), bit 2: copy removed part to buffer 1
        _floatParameters.push_back(0.5f); // r or h value
        _floatParameters.push_back(0.5f); // g or s value
        _floatParameters.push_back(0.5f); // b or l value
        _floatParameters.push_back(0.25); // r or h tolerance (x --> +-x)
        _floatParameters.push_back(0.25); // g or s tolerance (x --> +-x)
        _floatParameters.push_back(0.25); // b or l tolerance (x --> +-x)
    }
    if (_filterType==sim_filtercomponent_scaleandoffsetcolors_deprecated)
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
    if (_filterType==sim_filtercomponent_correlationwithbuffer1_deprecated)
    {
        _byteParameters.push_back(8+16); // 8: overlay display, 16: greyscale calculations
        _floatParameters.push_back(0.5f); // sub-image size X (template)
        _floatParameters.push_back(0.5f); // sub-image size Y (template)
        _floatParameters.push_back(1.0f); // search frame X
        _floatParameters.push_back(1.0f); // search frame Y
    }
    if (_filterType==sim_filtercomponent_shift_deprecated)
    {
        _byteParameters.push_back(0); // bit 0: if set, then no wrapping
        _floatParameters.push_back(0.5f); // x-shift (-1 to 1)
        _floatParameters.push_back(0.5f); // y-shift (-1 to 1)
    }
    if (_filterType==sim_filtercomponent_circularcut_deprecated)
    {
        _byteParameters.push_back(0); // bit 0: if set, cut part is copied to buffer 1
        _floatParameters.push_back(1.0f); // circle radius relative to smallest half-size
    }
    if (_filterType==sim_filtercomponent_colorsegmentation_deprecated)
        _floatParameters.push_back(0.3f); // max color-color distance
    if (_filterType==sim_filtercomponent_blobextraction_deprecated)
    {
        _byteParameters.push_back(1); // bit0: use different color for each blob
        _floatParameters.push_back(0.1f); // threshold
        _floatParameters.push_back(0.0f); // min blob size
    }
    if (_filterType==sim_filtercomponent_imagetocoord_deprecated)
    {
        _intParameters.push_back(32); // number of points along x
        _intParameters.push_back(32); // number of points along y
        _byteParameters.push_back(0); // BitCoded. bit 1 indicates we want overlap with visible pixels
    }
    if (_filterType==sim_filtercomponent_velodyne_deprecated)
    {
        _intParameters.push_back(32); // number of points along x
        _intParameters.push_back(32); // number of points along y
        _intParameters.push_back(0); // reserved
        _intParameters.push_back(0); // reserved
        _floatParameters.push_back(30.0f*piValue_f/180.0f); // vertical scan angle
        _floatParameters.push_back(0.0f); // reserved
        _floatParameters.push_back(0.0f); // reserved
    }

    if (_filterType==sim_filtercomponent_pixelchange_deprecated)
    {
        _floatParameters.push_back(0.1f); // threshold in percent
    }
    if (_filterType==sim_filtercomponent_rectangularcut_deprecated)
    {
        _byteParameters.push_back(0); // bit 0: if set, cut part is copied to buffer 1
        _floatParameters.push_back(0.9f); // x-size (0-1)
        _floatParameters.push_back(0.9f); // y-size (0-1)
    }
    if (_filterType==sim_filtercomponent_resize_deprecated)
    {
        _floatParameters.push_back(1.2f); // x-scale
        _floatParameters.push_back(1.2f); // y-scale
    }
    if (_filterType==sim_filtercomponent_rotate_deprecated)
    {
        _floatParameters.push_back(45.0f*degToRad_f); // rotation angle
    }
    if (_filterType==sim_filtercomponent_edge_deprecated)
        _floatParameters.push_back(0.1f); // threshold (0 to 1)
    if (_filterType==sim_filtercomponent_intensityscale_deprecated)
    {
        _intParameters.push_back(1); // 0=grey scale, 1=intensity scale
        _floatParameters.push_back(0.0f); // scale start
        _floatParameters.push_back(1.0f); // scale end
    }
    if (_filterType==sim_filtercomponent_binary_deprecated)
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
    return(_filterType);
}

void CSimpleFilter::serialize(CSer& ar)
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
    }
}

std::string CSimpleFilter::getCodeEquivalent()
{
    std::string str;
    if (_filterType==sim_filtercomponent_originalimage_deprecated)
        str="simVision.sensorImgToWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_originaldepth_deprecated)
        str="simVision.sensorDepthMapToWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_uniformimage_deprecated)
        str=boost::str(boost::format("simVision.uniformImgToWorkImg(inData.handle,{%f,%f,%f})") % _floatParameters[0] % _floatParameters[1] % _floatParameters[2]);
    if (_filterType==sim_filtercomponent_tooutput_deprecated)
        str="simVision.workImgToSensorImg(inData.handle,false)";
    if (_filterType==sim_filtercomponent_todepthoutput_deprecated)
        str="simVision.workImgToSensorDepthMap(inData.handle,false)";

    if (_filterType==sim_filtercomponent_tobuffer1_deprecated)
        str="simVision.workImgToBuffer1(inData.handle)";
    if (_filterType==sim_filtercomponent_tobuffer2_deprecated)
        str="simVision.workImgToBuffer2(inData.handle)";

    if (_filterType==sim_filtercomponent_frombuffer1_deprecated)
        str="simVision.buffer1ToWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_frombuffer2_deprecated)
        str="simVision.buffer2ToWorkImg(inData.handle)";

    if (_filterType==sim_filtercomponent_swapwithbuffer1_deprecated)
        str="simVision.swapWorkImgWithBuffer1(inData.handle)";
    if (_filterType==sim_filtercomponent_addtobuffer1_deprecated)
        str="simVision.addWorkImgToBuffer1(inData.handle)";
    if (_filterType==sim_filtercomponent_subtractfrombuffer1_deprecated)
        str="simVision.subtractWorkImgFromBuffer1(inData.handle)";

    if (_filterType==sim_filtercomponent_swapbuffers_deprecated)
        str="simVision.swapBuffers(inData.handle)";

    if (_filterType==sim_filtercomponent_addbuffer1_deprecated)
        str="simVision.addBuffer1ToWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_subtractbuffer1_deprecated)
        str="simVision.subtractBuffer1FromWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_multiplywithbuffer1_deprecated)
        str="simVision.multiplyWorkImgWithBuffer1(inData.handle)";

    if (_filterType==sim_filtercomponent_horizontalflip_deprecated)
        str="simVision.horizontalFlipWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_verticalflip_deprecated)
        str="simVision.verticalFlipWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_rotate_deprecated)
        str=boost::str(boost::format("simVision.rotateWorkImg(inData.handle,%f)") % _floatParameters[0]);
    if (_filterType==sim_filtercomponent_shift_deprecated)
        str=boost::str(boost::format("simVision.shiftWorkImg(inData.handle,{%f,%f},%s)") % _floatParameters[0] % _floatParameters[1] % ( ((_byteParameters[0]&1)==0) ? "true" : "false") );
    if (_filterType==sim_filtercomponent_resize_deprecated)
        str=boost::str(boost::format("simVision.resizeWorkImg(inData.handle,{%f,%f})") % _floatParameters[0] % _floatParameters[1]);
    if (_filterType==sim_filtercomponent_3x3filter_deprecated)
        str=boost::str(boost::format("simVision.matrix3x3OnWorkImg(inData.handle,%i,%f,{%f,%f,%f,%f,%f,%f,%f,%f,%f})") % _intParameters[0] % _floatParameters[9] % _floatParameters[0] % _floatParameters[1] % _floatParameters[2] % _floatParameters[3] % _floatParameters[4] % _floatParameters[5] % _floatParameters[6] % _floatParameters[7] % _floatParameters[8]);
    if (_filterType==sim_filtercomponent_5x5filter_deprecated)
        str=boost::str(boost::format("simVision.matrix5x5OnWorkImg(inData.handle,%i,%f,{%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f})") % _intParameters[0] % _floatParameters[25] % _floatParameters[0] % _floatParameters[1] % _floatParameters[2] % _floatParameters[3] % _floatParameters[4] % _floatParameters[5] % _floatParameters[6] % _floatParameters[7] % _floatParameters[8] % _floatParameters[9] % _floatParameters[10] % _floatParameters[11] % _floatParameters[12] % _floatParameters[13] % _floatParameters[14] % _floatParameters[15] % _floatParameters[16] % _floatParameters[17] % _floatParameters[18] % _floatParameters[19] % _floatParameters[20] % _floatParameters[21] % _floatParameters[22] % _floatParameters[23] % _floatParameters[24]);
    if (_filterType==sim_filtercomponent_rectangularcut_deprecated)
        str=boost::str(boost::format("simVision.rectangularCutWorkImg(inData.handle,{%f,%f},%s)") % _floatParameters[0] % _floatParameters[1] % ( ((_byteParameters[0]&1)!=0) ? "true" : "false"));
    if (_filterType==sim_filtercomponent_circularcut_deprecated)
        str=boost::str(boost::format("simVision.circularCutWorkImg(inData.handle,%f,%s)") % _floatParameters[0] % ( ((_byteParameters[0]&1)!=0) ? "true" : "false"));

    if (_filterType==sim_filtercomponent_normalize_deprecated)
        str="simVision.normalizeWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_colorsegmentation_deprecated)
        str=boost::str(boost::format("simVision.colorSegmentationOnWorkImg(inData.handle,%f)") % _floatParameters[0]);
    if (_filterType==sim_filtercomponent_intensityscale_deprecated)
        str=boost::str(boost::format("simVision.intensityScaleOnWorkImg(inData.handle,%f,%f,%s)") % _floatParameters[0] % _floatParameters[1]  % ( ((_intParameters[0]&1)==0) ? "true" : "false"));
    if (_filterType==sim_filtercomponent_keeporremovecolors_deprecated)
        str=boost::str(boost::format("simVision.selectiveColorOnWorkImg(inData.handle,{%f,%f,%f},{%f,%f,%f},%s,%s,%s)") % _floatParameters[0] % _floatParameters[1] % _floatParameters[2] % _floatParameters[3] % _floatParameters[4] % _floatParameters[5] % ( ((_byteParameters[0]&2)!=0) ? "true" : "false") % ( ((_byteParameters[0]&1)!=0) ? "true" : "false") % ( ((_byteParameters[0]&4)!=0) ? "true" : "false"));
    if (_filterType==sim_filtercomponent_scaleandoffsetcolors_deprecated)
        str=boost::str(boost::format("simVision.scaleAndOffsetWorkImg(inData.handle,{%f,%f,%f},{%f,%f,%f},{%f,%f,%f},%s)") % _floatParameters[0] % _floatParameters[1] % _floatParameters[2] % _floatParameters[3] % _floatParameters[4] % _floatParameters[5] % _floatParameters[6] % _floatParameters[7] % _floatParameters[8] % ( ((_byteParameters[0]&2)!=0) ? "true" : "false"));
//    if (_filterType==sim_filtercomponent_correlationwithbuffer1)
//

    if (_filterType==sim_filtercomponent_sharpen_deprecated)
        str="simVision.sharpenWorkImg(inData.handle)";
    if (_filterType==sim_filtercomponent_edge_deprecated)
        str=boost::str(boost::format("simVision.edgeDetectionOnWorkImg(inData.handle,%f)") % _floatParameters[0]);
    if (_filterType==sim_filtercomponent_binary_deprecated)
        str=boost::str(boost::format("local trig,packedPacket=simVision.binaryWorkImg(inData.handle,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%s) if trig then retVal.trigger=true end if packedPacket then retVal.packedPackets[#retVal.packedPackets+1]=packedPacket end") % _floatParameters[0] % _floatParameters[4] % _floatParameters[5] % _floatParameters[6] % _floatParameters[7] % _floatParameters[8] % _floatParameters[9] % _floatParameters[10] % _floatParameters[11] % _floatParameters[12] % ( ((_byteParameters[0]&4)!=0) ? "true" : "false"));
    if (_filterType==sim_filtercomponent_blobextraction_deprecated)
        str=boost::str(boost::format("local trig,packedPacket=simVision.blobDetectionOnWorkImg(inData.handle,%f,%f,%s) if trig then retVal.trigger=true end if packedPacket then retVal.packedPackets[#retVal.packedPackets+1]=packedPacket end") % _floatParameters[0] % _floatParameters[1] % ( ((_byteParameters[0]&1)!=0) ? "true" : "false"));
    if (_filterType==sim_filtercomponent_imagetocoord_deprecated)
        str=boost::str(boost::format("local trig,packedPacket=simVision.coordinatesFromWorkImg(inData.handle,{%i,%i},%s) if trig then retVal.trigger=true end if packedPacket then retVal.packedPackets[#retVal.packedPackets+1]=packedPacket end") % _intParameters[0] % _intParameters[1] % ( ((_byteParameters[0]&1)!=0) ? "true" : "false"));
    if (_filterType==sim_filtercomponent_pixelchange_deprecated)
        str=boost::str(boost::format("local trig,packedPacket=simVision.changedPixelsOnWorkImg(inData.handle,%f) if trig then retVal.trigger=true end if packedPacket then retVal.packedPackets[#retVal.packedPackets+1]=packedPacket end") % _floatParameters[0]);
    if (_filterType==sim_filtercomponent_velodyne_deprecated)
        str=boost::str(boost::format("local trig,packedPacket=simVision.velodyneDataFromWorkImg(inData.handle,{%i,%i},%f) if trig then retVal.trigger=true end if packedPacket then retVal.packedPackets[#retVal.packedPackets+1]=packedPacket end") % _intParameters[0] % _intParameters[1] % _floatParameters[0]);
    if (!_enabled)
        str=std::string("-- ")+str;
    return(str);
}
