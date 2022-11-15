#include "simInternal.h"
#include "convexVolume.h"
#include "tt.h"
#include "global.h"
#include "4X4Matrix.h"
#include <algorithm>

CConvexVolume::CConvexVolume()
{
    commonInit();
}

CConvexVolume::~CConvexVolume()
{
}

void CConvexVolume::commonInit()
{
    _smallestDistanceAllowed=0.1;
    _smallestDistanceEnabled=false;

    _volumeComputationTemporarilyDisabled=false;
    _volumeType=CYLINDER_TYPE_CONVEX_VOLUME;
    offset=0.1;
    range=0.2;
    radius=0.1;
    radiusFar=0.2;
    faceNumber=32;
}

void CConvexVolume::disableVolumeComputation(bool disableIt)
{ // Careful with this function (normally only used for compatibility with older serialization versions of the CProxSensor class (2009/08/14))
    _volumeComputationTemporarilyDisabled=disableIt;
}

C4X4Matrix CConvexVolume::getTheMatrix(const C3Vector& pt0,const C3Vector& pt1,const C3Vector& pt2,bool tri)
{   // The returned matrix will have its z-axis normal to the specified plane.
    // X and Y-axis are arbitrary, but the axis-system is consistent
    // This function returns a matrix used to specify a plane (addAPlane)
    C4X4Matrix retM;
    C3Vector v1(pt1-pt0);
    C3Vector v2(pt2-pt0);
    // We get the Z-axis:
    retM.M.axis[2]=(v1^v2).getNormalized();
    // Now an arbitrary X-axis:
    retM.M.axis[0]=v1.getNormalized();
    // Now the Y-axis:
    retM.M.axis[1]=retM.M.axis[2]^retM.M.axis[0];
    // Now the translation part:
    if (tri)
        retM.X=(pt0+pt1+pt2)/3.0;
    else
        retM.X=(pt1+pt2)/2.0;
    return(retM);
}

void CConvexVolume::add3Values(std::vector<floatDouble>& vect,const C4X4Matrix& transf,floatDouble x,floatDouble y,floatDouble z)
{
    C3Vector v(x,y,z);
    v=transf*v;
    vect.push_back(v(0));
    vect.push_back(v(1));
    vect.push_back(v(2));
}

CConvexVolume* CConvexVolume::copyYourself()
{
    CConvexVolume* newConvexVolume=new CConvexVolume();

    newConvexVolume->_volumeType=_volumeType;
    newConvexVolume->offset=offset;
    newConvexVolume->range=range;
    newConvexVolume->xSize=xSize;
    newConvexVolume->ySize=ySize;
    newConvexVolume->xSizeFar=xSizeFar;
    newConvexVolume->ySizeFar=ySizeFar;
    newConvexVolume->radius=radius;
    newConvexVolume->radiusFar=radiusFar;
    newConvexVolume->angle=angle;
    newConvexVolume->insideAngleThing=insideAngleThing;
    newConvexVolume->faceNumber=faceNumber;
    newConvexVolume->faceNumberFar=faceNumberFar;
    newConvexVolume->subdivisions=subdivisions;
    newConvexVolume->subdivisionsFar=subdivisionsFar;
    newConvexVolume->_smallestDistanceEnabled=_smallestDistanceEnabled;
    newConvexVolume->_smallestDistanceAllowed=_smallestDistanceAllowed;

    newConvexVolume->planesInside.insert(newConvexVolume->planesInside.end(),planesInside.begin(),planesInside.end());
    newConvexVolume->planesOutside.insert(newConvexVolume->planesOutside.end(),planesOutside.begin(),planesOutside.end());
    newConvexVolume->normalsInside.insert(newConvexVolume->normalsInside.end(),normalsInside.begin(),normalsInside.end());
    newConvexVolume->normalsOutside.insert(newConvexVolume->normalsOutside.end(),normalsOutside.begin(),normalsOutside.end());
    newConvexVolume->volumeEdges.insert(newConvexVolume->volumeEdges.end(),volumeEdges.begin(),volumeEdges.end());
    newConvexVolume->nonDetectingVolumeEdges.insert(newConvexVolume->nonDetectingVolumeEdges.end(),nonDetectingVolumeEdges.begin(),nonDetectingVolumeEdges.end());

    return(newConvexVolume);
}

void CConvexVolume::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Snt");
            ar << _volumeType;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Ss3");
            ar.flt() << (floatFloat)offset << (floatFloat)range << (floatFloat)xSize << (floatFloat)ySize << (floatFloat)xSizeFar << (floatFloat)ySizeFar;
            ar.flt() << (floatFloat)radius << (floatFloat)radiusFar << (floatFloat)angle;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_s3");
            ar.dbl() << offset << range << xSize << ySize << xSizeFar << ySizeFar;
            ar.dbl() << radius << radiusFar << angle;
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Sia");
            ar.flt() << (floatFloat)insideAngleThing;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_ia");
            ar.dbl() << insideAngleThing;
            ar.flush();
#endif

            ar.storeDataName("Ssi");
            ar << faceNumber << faceNumberFar << subdivisions << subdivisionsFar;
            ar.flush();

#ifdef TMPOPERATION
            ar.storeDataName("Wtf");
            ar.flt() << (floatFloat)_smallestDistanceAllowed;
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_tf");
            ar.dbl() << _smallestDistanceAllowed;
            ar.flush();
#endif

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_smallestDistanceEnabled);
            ar << nothing;
            ar.flush();

            // Following added on 2010/04/28 because recomputing those values takes long:
            //***************************************************************************
#ifdef TMPOPERATION
            ar.storeDataName("Nc0");
            ar << int(planesInside.size());
            for (int i=0;i<int(planesInside.size());i++)
                ar.flt() << (floatFloat)planesInside[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_c0");
            ar << int(planesInside.size());
            for (int i=0;i<int(planesInside.size());i++)
                ar.dbl() << planesInside[i];
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Nc1");
            ar << int(planesOutside.size());
            for (int i=0;i<int(planesOutside.size());i++)
                ar.flt() << (floatFloat)planesOutside[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_c1");
            ar << int(planesOutside.size());
            for (int i=0;i<int(planesOutside.size());i++)
                ar.dbl() << planesOutside[i];
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Nc2");
            ar << int(normalsInside.size());
            for (int i=0;i<int(normalsInside.size());i++)
                ar.flt() << (floatFloat)normalsInside[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_c2");
            ar << int(normalsInside.size());
            for (int i=0;i<int(normalsInside.size());i++)
                ar.dbl() << normalsInside[i];
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Nc3");
            ar << int(normalsOutside.size());
            for (int i=0;i<int(normalsOutside.size());i++)
                ar.flt() << (floatFloat)normalsOutside[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_c3");
            ar << int(normalsOutside.size());
            for (int i=0;i<int(normalsOutside.size());i++)
                ar.dbl() << normalsOutside[i];
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Nc4");
            ar << int(volumeEdges.size());
            for (int i=0;i<int(volumeEdges.size());i++)
                ar.flt() << (floatFloat)volumeEdges[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_c4");
            ar << int(volumeEdges.size());
            for (int i=0;i<int(volumeEdges.size());i++)
                ar.dbl() << volumeEdges[i];
            ar.flush();
#endif

#ifdef TMPOPERATION
            ar.storeDataName("Nc5");
            ar << int(nonDetectingVolumeEdges.size());
            for (int i=0;i<int(nonDetectingVolumeEdges.size());i++)
                ar.flt() << (floatFloat)nonDetectingVolumeEdges[i];
            ar.flush();
#endif
#ifdef NEWOPERATION
            ar.storeDataName("_c5");
            ar << int(nonDetectingVolumeEdges.size());
            for (int i=0;i<int(nonDetectingVolumeEdges.size());i++)
                ar.dbl() << nonDetectingVolumeEdges[i];
            ar.flush();
#endif

            //***************************************************************************

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            bool recomputeVolumes=true; // for backward compatibility (2010/04/28)
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Snt")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _volumeType;
                    }
                    if (theName.compare("Ss3")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat a,b,c,d,e,f;
                        ar.flt() >> a >> b >> c >> d >> e >> f;
                        offset=(floatDouble)a;
                        range=(floatDouble)b;
                        xSize=(floatDouble)c;
                        ySize=(floatDouble)d;
                        xSizeFar=(floatDouble)e;
                        ySizeFar=(floatDouble)f;
                        ar.flt() >> a >> b >> c;
                        radius=(floatDouble)a;
                        radiusFar=(floatDouble)b;
                        angle=(floatDouble)c;
                    }
                    if (theName.compare("_s3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> offset >> range >> xSize >> ySize >> xSizeFar >> ySizeFar;
                        ar.dbl() >> radius >> radiusFar >> angle;
                    }
                    if (theName.compare("Sia")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat a;
                        ar.flt() >> a;
                        insideAngleThing=(floatDouble)a;
                    }
                    if (theName.compare("_ia")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> insideAngleThing;
                    }
                    if (theName.compare("Ssi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> faceNumber >> faceNumberFar >> subdivisions >> subdivisionsFar;
                    }
                    if (theName.compare("Wtf")==0)
                    { // for backward comp. (flt->dbl)
                        noHit=false;
                        ar >> byteQuantity;
                        floatFloat a;
                        ar.flt() >> a;
                        _smallestDistanceAllowed=(floatDouble)a;
                    }
                    if (theName.compare("_tf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar.dbl() >> _smallestDistanceAllowed;
                    }
                    if (theName=="Par")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _smallestDistanceEnabled=SIM_IS_BIT_SET(nothing,0);
                    }
                    // Following added on 2010/04/28 because recomputing those values takes long:
                    //***************************************************************************
                    if (theName.compare("Nc0")==0)
                    { // for backward comp. (flt->dbl)
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        planesInside.clear();
                        floatFloat dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.flt() >> dum;
                            planesInside.push_back((floatDouble)dum);
                        }
                    }
                    if (theName.compare("_c0")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        planesInside.clear();
                        floatDouble dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.dbl() >> dum;
                            planesInside.push_back(dum);
                        }
                    }

                    if (theName.compare("Nc1")==0)
                    { // for backward comp. (flt->dbl)
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        planesOutside.clear();
                        floatFloat dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.flt() >> dum;
                            planesOutside.push_back((floatDouble)dum);
                        }
                    }
                    if (theName.compare("_c1")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        planesOutside.clear();
                        floatDouble dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.dbl() >> dum;
                            planesOutside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc2")==0)
                    { // for backward comp. (flt->dbl)
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        normalsInside.clear();
                        floatFloat dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.flt() >> dum;
                            normalsInside.push_back((floatDouble)dum);
                        }
                    }
                    if (theName.compare("_c2")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        normalsInside.clear();
                        floatDouble dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.dbl() >> dum;
                            normalsInside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc3")==0)
                    { // for backward comp. (flt->dbl)
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        normalsOutside.clear();
                        floatFloat dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.flt() >> dum;
                            normalsOutside.push_back((floatDouble)dum);
                        }
                    }
                    if (theName.compare("_c3")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        normalsOutside.clear();
                        floatDouble dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.dbl() >> dum;
                            normalsOutside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc4")==0)
                    { // for backward comp. (flt->dbl)
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        volumeEdges.clear();
                        floatFloat dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.flt() >> dum;
                            volumeEdges.push_back((floatDouble)dum);
                        }
                    }
                    if (theName.compare("_c4")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        volumeEdges.clear();
                        floatDouble dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.dbl() >> dum;
                            volumeEdges.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc5")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        nonDetectingVolumeEdges.clear();
                        floatFloat dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.flt() >> dum;
                            nonDetectingVolumeEdges.push_back((floatDouble)dum);
                        }
                    }
                    if (theName.compare("_c5")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        nonDetectingVolumeEdges.clear();
                        floatDouble dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar.dbl() >> dum;
                            nonDetectingVolumeEdges.push_back(dum);
                        }
                    }
                    //***************************************************************************

                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            // For backward compatibility (2010/04/28)
            //**************************************
            if (recomputeVolumes)
                computeVolumes();
            //**************************************
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_enum("type",_volumeType,PYRAMID_TYPE_CONVEX_VOLUME,"pyramid",CYLINDER_TYPE_CONVEX_VOLUME,"cylinder",DISC_TYPE_CONVEX_VOLUME,"disc",CONE_TYPE_CONVEX_VOLUME,"cone",RAY_TYPE_CONVEX_VOLUME,"ray");

            ar.xmlAddNode_float("offset",offset);
            ar.xmlAddNode_float("range",range);
            ar.xmlAddNode_2float("xySizeClose",xSize,ySize);
            ar.xmlAddNode_2float("xySizeFar",xSizeFar,ySizeFar);
            ar.xmlAddNode_2float("closeFarRadius",radius,radiusFar);
            ar.xmlAddNode_float("angle",angle*180.0/piValue);
            ar.xmlAddNode_float("insideAngleScaling",insideAngleThing);
            ar.xmlAddNode_2int("closeFarFaceCount",faceNumber,faceNumberFar);
            ar.xmlAddNode_2int("closeFarSubdivisions",subdivisions,subdivisionsFar);
            ar.xmlAddNode_float("smallestAllowedDistance",_smallestDistanceAllowed);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("smallestDistanceEnabled",_smallestDistanceEnabled);
            ar.xmlPopNode();
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_enum("type",_volumeType,true,"pyramid",PYRAMID_TYPE_CONVEX_VOLUME,"cylinder",CYLINDER_TYPE_CONVEX_VOLUME,"disc",DISC_TYPE_CONVEX_VOLUME,"cone",CONE_TYPE_CONVEX_VOLUME,"ray",RAY_TYPE_CONVEX_VOLUME);

            floatDouble v,w;
            int k,l;
            if (ar.xmlGetNode_float("offset",v,exhaustiveXml))
                setOffset(v,false);
            if (ar.xmlGetNode_float("range",v,exhaustiveXml))
                setRange(v,false);
            if (ar.xmlGetNode_2float("xySizeClose",v,w,exhaustiveXml))
            {
                setXSize(v,false);
                setYSize(w,false);
            }
            if (ar.xmlGetNode_2float("xySizeFar",v,w,exhaustiveXml))
            {
                setXSizeFar(v,false);
                setYSizeFar(w,false);
            }
            if (ar.xmlGetNode_2float("closeFarRadius",v,w,exhaustiveXml))
            {
                setRadius(v,false);
                setRadiusFar(w,false);
            }
            if (ar.xmlGetNode_float("angle",v,exhaustiveXml))
                setAngle(v*piValue/180.0,false);
            if (ar.xmlGetNode_float("insideAngleScaling",v,exhaustiveXml))
                setInsideAngleThing(v,false);
            if (ar.xmlGetNode_2int("closeFarFaceCount",k,l,exhaustiveXml))
            {
                setFaceNumber(k,false);
                setFaceNumberFar(l,false);
            }
            if (ar.xmlGetNode_2int("closeFarSubdivisions",k,l,exhaustiveXml))
            {
                setSubdivisions(k,false);
                setSubdivisionsFar(l,false);
            }
            if (ar.xmlGetNode_float("smallestAllowedDistance",v,exhaustiveXml))
                setSmallestDistanceAllowed(v,false);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("smallestDistanceEnabled",_smallestDistanceEnabled,exhaustiveXml);
                ar.xmlPopNode();
            }
            computeVolumes();
        }
    }
}

bool CConvexVolume::getVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const
{ 
    if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
    {
        minV(0)=-0.001f;
        maxV(0)=+0.001f;
        minV(1)=-0.001f;
        maxV(1)=+0.001f;
        minV(2)=0.0;
        if (offset<0.0)
            minV(2)=offset;
        maxV(2)=offset+range;
        if (maxV(2)<0.0)
            maxV(2)=0.0;
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        floatDouble maxX=std::max<floatDouble>(xSize,xSizeFar)/2.0;
        minV(0)=-maxX;
        maxV(0)=maxX;
        floatDouble maxY=std::max<floatDouble>(ySize,ySizeFar)/2.0;
        minV(1)=-maxY;
        maxV(1)=maxY;
        minV(2)=0.0;
        if (offset<0.0)
            minV(2)=offset;
        maxV(2)=offset+range;
        if (maxV(2)<0.0)
            maxV(2)=0.0;
    }
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        floatDouble maxRad=std::max<floatDouble>(radius,radiusFar);
        minV(0)=-maxRad;
        maxV(0)=maxRad;
        minV(1)=-maxRad;
        maxV(1)=maxRad;
        minV(2)=0.0;
        if (offset<0.0)
            minV(2)=offset;
        maxV(2)=offset+range;
        if (maxV(2)<0.0)
            maxV(2)=0.0;
    }
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        minV(1)=-ySize/2.0;
        maxV(1)=ySize/2.0;
        floatDouble tmp;
        if (angle<piValue*1.03f)
            tmp=(radius+range)*sin(angle*0.5);
        else
            tmp=radius+range;
        minV(0)=-tmp;
        maxV(0)=tmp;
        if (angle<piValue*1.03f)
        {
            minV(2)=0.0;
            if (offset<0.0)
                minV(2)=offset;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0)
                maxV(2)=0.0;
        }
        else
        {
            minV(2)=offset-radius-range;
            if (minV(2)>0.0)
                minV(2)=0.0;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0)
                maxV(2)=0.0;
        }
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        floatDouble tmp;
        if (angle<piValue*1.03f)
            tmp=(radius+range)*sin(angle*0.5);
        else
            tmp=radius+range;
        minV(0)=-tmp;
        maxV(0)=tmp;
        minV(1)=-tmp;
        maxV(1)=tmp;
        if (angle<piValue*1.03f)
        {
            minV(2)=0.0;
            if (offset<0.0)
                minV(2)=offset;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0)
                maxV(2)=0.0;
        }
        else
        {
            minV(2)=offset-radius-range;
            if (minV(2)>0.0)
                minV(2)=0.0;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0)
                maxV(2)=0.0;
        }
    }
    return(true);
}

void CConvexVolume::scaleVolume(floatDouble scalingFactor)
{
    offset*=scalingFactor;
    range*=scalingFactor;
    xSize*=scalingFactor;
    ySize*=scalingFactor;
    xSizeFar*=scalingFactor;
    ySizeFar*=scalingFactor;
    radius*=scalingFactor;
    radiusFar*=scalingFactor;
    _smallestDistanceAllowed*=scalingFactor;
    solveInterferences();
    computeVolumes();
}

void CConvexVolume::scaleVolumeNonIsometrically(floatDouble x,floatDouble y,floatDouble z,floatDouble& xRet,floatDouble& yRet,floatDouble& zRet)
{
    floatDouble xy=sqrt(x*y);
    floatDouble xyz=cbrt(x*y*z);
    floatDouble xz=sqrt(x*z);
    if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
    {
/*
        offset*=xyz;
        range*=xyz;
        _smallestDistanceAllowed*=xyz;
        xRet=xyz;
        yRet=xyz;
        zRet=xyz;
        */
        offset*=z;
        range*=z;
        _smallestDistanceAllowed*=z;
        xRet=1.0;
        yRet=1.0;
        zRet=1.0;
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        offset*=z;
        range*=z;
        xSize*=x;
        ySize*=y;
        xSizeFar*=x;
        ySizeFar*=y;
        _smallestDistanceAllowed*=z;
        xRet=x;
        yRet=y;
        zRet=z;
    }
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        offset*=z;
        range*=z;
        radius*=xy;
        radiusFar*=xy;
        _smallestDistanceAllowed*=z;
        xRet=xy;
        yRet=xy;
        zRet=z;
    }
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        offset*=xz;
        range*=xz;
        radius*=xz;
        ySize*=y;
        _smallestDistanceAllowed*=xz;
        xRet=xz;
        yRet=y;
        zRet=xz;
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        offset*=xyz;
        range*=xyz;
        radius*=xyz;
        _smallestDistanceAllowed*=xyz;
        xRet=xyz;
        yRet=xyz;
        zRet=xyz;
    }
    solveInterferences();
    computeVolumes();
}

void CConvexVolume::addAPlane(std::vector<floatDouble>* volume,std::vector<floatDouble>* normals,floatDouble nL,const C4X4Matrix& m,bool inside)
{   // normals can be nullptr.
    // Always z-axis:
    volume->push_back(m.M.axis[2](0));
    volume->push_back(m.M.axis[2](1));
    volume->push_back(m.M.axis[2](2));
    volume->push_back(-(m.M.axis[2]*m.X));
    if (normals!=nullptr)
    {
        for (int i=0;i<3;i++)
            normals->push_back(m.X(i));
        C3Vector v(m.X+(m.M.axis[2]*nL));
        for (int i=0;i<3;i++)
            normals->push_back(v(i));
    }
}

void CConvexVolume::setVolumeType(int theType,int objectTypeTheVolumeIsFor,floatDouble pointSize)
{
    _volumeType=theType;
    setDefaultVolumeParameters(objectTypeTheVolumeIsFor,pointSize);
    computeVolumes();
}

int CConvexVolume::getVolumeType()
{
    return(_volumeType);
}

void CConvexVolume::setOffset(floatDouble theOffset,bool recomputeVolume/*=true*/)
{
    tt::limitValue(-1000.0,1000.0,theOffset);
    offset=theOffset;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getOffset() const
{
    return(offset);
}

void CConvexVolume::setRange(floatDouble theRange,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.00001f,1000.0,theRange);
    range=theRange;
    solveInterferences();
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getRange() const
{
    return(range);
}

void CConvexVolume::setXSize(floatDouble theXSize,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.00001f,1000.0,theXSize);
    xSize=theXSize;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getXSize() const
{
    return(xSize);
}

void CConvexVolume::setYSize(floatDouble theYSize,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.00001f,1000.0,theYSize);
    ySize=theYSize;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getYSize() const
{
    return(ySize);
}

void CConvexVolume::setXSizeFar(floatDouble theXSizeFar,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.00001f,1000.0,theXSizeFar);
    xSizeFar=theXSizeFar;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getXSizeFar() const
{
    return(xSizeFar);
}

void CConvexVolume::setYSizeFar(floatDouble theYSizeFar,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.00001f,1000.0,theYSizeFar);
    ySizeFar=theYSizeFar;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getYSizeFar() const
{
    return(ySizeFar);
}

void CConvexVolume::setRadius(floatDouble theRadius,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.0,1000.0,theRadius);
    if ( (_volumeType==DISC_TYPE_CONVEX_VOLUME)||(_volumeType==CONE_TYPE_CONVEX_VOLUME) )
    {
        if (theRadius<0.00001f)
            theRadius=0.0;
    }
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        if (theRadius<0.00001f)
            theRadius=0.00001f;
    }
    radius=theRadius;
    solveInterferences();
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getRadius() const
{
    return(radius);
}

void CConvexVolume::setRadiusFar(floatDouble theRadiusFar,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.0,1000.0,theRadiusFar);
    radiusFar=theRadiusFar;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getRadiusFar() const
{
    return(radiusFar);
}

void CConvexVolume::setAngle(floatDouble theAngle,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.001f*degToRad,piValT2,theAngle);
    if ( (_volumeType==DISC_TYPE_CONVEX_VOLUME)||(_volumeType==CONE_TYPE_CONVEX_VOLUME) )
    {
        if ( (theAngle>piValue)&&(theAngle<piValue*1.01f) )
            theAngle=piValue;
        if (theAngle>=piValue*1.01f)
            theAngle=piValT2;
    }
    angle=theAngle;
    solveInterferences();
    if (recomputeVolume)
        computeVolumes();
}

void CConvexVolume::setInsideAngleThing(floatDouble theAngle,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.0,0.999f,theAngle);
    if (theAngle<0.01f)
        theAngle=0.0;
    insideAngleThing=theAngle;
    if (theAngle!=0.0)
    {
        if (angle>piValue*1.01f)
            angle=piValue;
    }
    solveInterferences();
    if (recomputeVolume)
        computeVolumes();
}

void CConvexVolume::setSmallestDistanceEnabled(bool e,bool recomputeVolume/*=true*/)
{
    _smallestDistanceEnabled=e;
    if (recomputeVolume)
        computeVolumes();
}

bool CConvexVolume::getSmallestDistanceEnabled() const
{
    return(_smallestDistanceEnabled);
}

void CConvexVolume::setSmallestDistanceAllowed(floatDouble d,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0.0001f,10.0,d);
    _smallestDistanceAllowed=d;
    if (recomputeVolume)
        computeVolumes();
}

floatDouble CConvexVolume::getSmallestDistanceAllowed() const
{
    return(_smallestDistanceAllowed);
}

void CConvexVolume::solveInterferences()
{
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        floatDouble dA2=angle/faceNumberFar;
        floatDouble r2=(radius+range)*cos(dA2/2.0);
        while ( (r2<=radius)&&(faceNumberFar<PROXSENSOR_MAX_FACE_NUMBER) )
        { // We have an interference here!
            faceNumberFar=faceNumberFar+1;
            dA2=angle/faceNumberFar;
            r2=(radius+range)*cos(dA2/2.0);
        }
        while (r2<=radius)
        { // We have to increase the range now:
            range=1.25f*range;
            r2=(radius+range)*cos(dA2/2.0);
        }
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        floatDouble dA2=angle/(subdivisionsFar+1);
        floatDouble r2=(radius+range)*cos(dA2/4.0);
        while ( (r2<=radius)&&(subdivisionsFar<PROXSENSOR_MAX_SUBDIVISIONS) )
        { // We have an interference here!
            subdivisionsFar=subdivisionsFar+1;
            dA2=angle/(subdivisionsFar+1);
            r2=(radius+range)*cos(dA2/4.0);
        }
        while (r2<=radius)
        { // We have to increase the range now:
            range=1.25f*range;
            r2=(radius+range)*cos(dA2/4.0);
        }
    }
}

floatDouble CConvexVolume::getAngle() const
{
    return(angle);
}

floatDouble CConvexVolume::getInsideAngleThing() const
{
    return(insideAngleThing);
}

void CConvexVolume::setFaceNumber(int theFaceNumber,bool recomputeVolume/*=true*/)
{
    tt::limitValue(1,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
        tt::limitValue(3,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        if (angle>piValue*1.01f)
            tt::limitValue(3,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
        tt::limitValue(3,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    faceNumber=theFaceNumber;
    if (recomputeVolume)
        computeVolumes();
}

int CConvexVolume::getFaceNumber() const
{
    return(faceNumber);
}

void CConvexVolume::setFaceNumberFar(int theFaceNumberFar,bool recomputeVolume/*=true*/)
{
    tt::limitValue(1,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumberFar);
    faceNumberFar=theFaceNumberFar;
    solveInterferences();
    if (recomputeVolume)
        computeVolumes();
}

int CConvexVolume::getFaceNumberFar() const
{
    return(faceNumberFar);
}

void CConvexVolume::setSubdivisions(int theSubdivisions,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0,PROXSENSOR_MAX_SUBDIVISIONS,theSubdivisions);
    subdivisions=theSubdivisions;
    if (recomputeVolume)
        computeVolumes();
}

int CConvexVolume::getSubdivisions() const
{
    return(subdivisions);
}

void CConvexVolume::setSubdivisionsFar(int theSubdivisionsFar,bool recomputeVolume/*=true*/)
{
    tt::limitValue(0,PROXSENSOR_MAX_SUBDIVISIONS,theSubdivisionsFar);
    subdivisionsFar=theSubdivisionsFar;
    solveInterferences();
    if (recomputeVolume)
        computeVolumes();
}

int CConvexVolume::getSubdivisionsFar() const
{
    return(subdivisionsFar);
}

void CConvexVolume::setDefaultVolumeParameters(int objectTypeTheVolumeIsFor,floatDouble pointSize)
{
    insideAngleThing=0.0;
    _smallestDistanceAllowed=0.1;
    _smallestDistanceEnabled=false;
    if (objectTypeTheVolumeIsFor==sim_object_proximitysensor_type)
    {
        if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
        {
            offset=0.1;
            range=0.2;
            radius=0.1;
            radiusFar=0.2;
            faceNumber=32;
        }
        if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
        {
            offset=0.0;
            range=0.2;
            radius=0.1;
            angle=piValD2;
            ySize=0.05f;
            ySizeFar=0.1;
            faceNumber=16;
            faceNumberFar=32;
        }
        if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
        {
            offset=0.1;
            range=0.2;
            xSize=0.2;
            ySize=0.1;
            xSizeFar=0.4;
            ySizeFar=0.2;
        }
        if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
        {
            offset=0.1;
            range=0.4;
        }
        if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
        {
            offset=0.0;
            radius=0.1;
            range=0.3-radius;
            angle=piValD2;
            faceNumber=32;
            subdivisions=1;
            subdivisionsFar=16;
        }
    }
    if (objectTypeTheVolumeIsFor==sim_object_mill_type)
    {
        if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
        {
            offset=0.0;
            range=0.05f;
            radius=0.015f;
            radiusFar=0.015f;
            faceNumber=32;
        }
        if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
        {
            offset=0.0;
            range=0.05f;
            radius=0.0;
            angle=30.0*degToRad;
            ySize=0.015f;
            ySizeFar=0.015f;
            faceNumber=1;
            faceNumberFar=4;
        }
        if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
        {
            offset=0.0;
            range=0.05f;
            xSize=0.02f;
            ySize=0.02f;
            xSizeFar=0.02f;
            ySizeFar=0.02f;
        }
        if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
        { // Should not be possible
            offset=0.0;
            range=0.05f;
        }
        if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
        {
            offset=0.0;
            radius=0.0;    // no outside volume here
            range=0.05f;
            angle=30.0*degToRad;
            faceNumber=16;
            subdivisions=1;
            subdivisionsFar=4;
        }
    }
}

void CConvexVolume::computeVolumeEdges(std::vector<floatDouble>& edges)
{
    edges.clear();
    C4X4Matrix m;
    m.setIdentity();
    C4X4Matrix tr;
    if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
    {
        add3Values(edges,m,0,0,offset);
        add3Values(edges,m,0,0,offset+range);
    }
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        floatDouble alpha=piValue/faceNumber;
        for (int i=0;i<faceNumber;i++)
        {
            add3Values(edges,m,+cos(alpha)*radius,-sin(alpha)*radius,offset);
            add3Values(edges,m,+cos(alpha)*radius,+sin(alpha)*radius,offset);
            add3Values(edges,m,+cos(alpha)*radius,+sin(alpha)*radius,offset);
            add3Values(edges,m,+cos(alpha)*radiusFar,+sin(alpha)*radiusFar,offset+range);
            add3Values(edges,m,+cos(alpha)*radiusFar,+sin(alpha)*radiusFar,offset+range);
            add3Values(edges,m,+cos(alpha)*radiusFar,-sin(alpha)*radiusFar,offset+range);
            tr.buildZRotation(piValT2/faceNumber);
            m=m*tr;
        }
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        add3Values(edges,m,+xSize/2.0,-ySize/2.0,offset);
        add3Values(edges,m,+xSize/2.0,+ySize/2.0,offset);
        add3Values(edges,m,+xSize/2.0,+ySize/2.0,offset);
        add3Values(edges,m,-xSize/2.0,+ySize/2.0,offset);
        add3Values(edges,m,-xSize/2.0,+ySize/2.0,offset);
        add3Values(edges,m,-xSize/2.0,-ySize/2.0,offset);
        add3Values(edges,m,-xSize/2.0,-ySize/2.0,offset);
        add3Values(edges,m,+xSize/2.0,-ySize/2.0,offset);
        add3Values(edges,m,+xSize/2.0,-ySize/2.0,offset);
        add3Values(edges,m,+xSizeFar/2.0,-ySizeFar/2.0,offset+range);
        add3Values(edges,m,+xSizeFar/2.0,-ySizeFar/2.0,offset+range);
        add3Values(edges,m,+xSizeFar/2.0,+ySizeFar/2.0,offset+range);
        add3Values(edges,m,+xSizeFar/2.0,+ySizeFar/2.0,offset+range);
        add3Values(edges,m,-xSizeFar/2.0,+ySizeFar/2.0,offset+range);
        add3Values(edges,m,-xSizeFar/2.0,+ySizeFar/2.0,offset+range);
        add3Values(edges,m,-xSizeFar/2.0,-ySizeFar/2.0,offset+range);
        add3Values(edges,m,-xSizeFar/2.0,-ySizeFar/2.0,offset+range);
        add3Values(edges,m,+xSizeFar/2.0,-ySizeFar/2.0,offset+range);
        add3Values(edges,m,+xSize/2.0,+ySize/2.0,offset);
        add3Values(edges,m,+xSizeFar/2.0,+ySizeFar/2.0,offset+range);
        add3Values(edges,m,-xSize/2.0,+ySize/2.0,offset);
        add3Values(edges,m,-xSizeFar/2.0,+ySizeFar/2.0,offset+range);
        add3Values(edges,m,-xSize/2.0,-ySize/2.0,offset);
        add3Values(edges,m,-xSizeFar/2.0,-ySizeFar/2.0,offset+range);
    }

    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        tr.buildTranslation(0.0,0.0,offset);
        m=m*tr;
        floatDouble angleStart=angle/2.0;
        floatDouble angleStart2=angleStart*insideAngleThing;
        floatDouble dA=angleStart/faceNumber;
        floatDouble dAd=dA*radToDeg;
        if (insidePart)
        {
            dA=angleStart2/faceNumber;
            dAd=dA*radToDeg;
        }
        floatDouble radiusSinAngleStart=radius*sin(angleStart);
        floatDouble radiusSinAngleStartDa=radius*sin(angleStart-dA);
        floatDouble radiusCosAngleStart=radius*cos(angleStart);
        floatDouble radiusCosAngleStartDa=radius*cos(angleStart-dA);
        floatDouble halfSize=ySize/2.0;
        floatDouble aaa=cos(angleStart)*radius;
        floatDouble bbb=sin(angleStart)*radius;
        floatDouble bb2=tan(angleStart2)*aaa;
        // The close part:
        if (radius>0.000001f)
        {
            C4X4Matrix mSaved(m);
            if (insidePart)
            {
                for (int i=-1;i<2;i+=2)
                {
                    add3Values(edges,m,-bbb,i*halfSize,aaa);
                    add3Values(edges,m,-bb2,i*halfSize,aaa);
                    add3Values(edges,m,bbb,i*halfSize,aaa);
                    add3Values(edges,m,bb2,i*halfSize,aaa);

                    if (angle<piValue*0.999f)
                    {
                        add3Values(edges,m,-bbb,-i*halfSize,aaa);
                        add3Values(edges,m,-bbb,i*halfSize,aaa);
                        add3Values(edges,m,bbb,-i*halfSize,aaa);
                        add3Values(edges,m,bbb,i*halfSize,aaa);
                    }
                }
            }
            else
            {
                for (int j=0;j<2;j++)
                {
                    C4X4Matrix mSaved2(m);
                    for (int i=0;i<faceNumber;i++)
                    {
                        add3Values(edges,m,radiusSinAngleStartDa,-halfSize,radiusCosAngleStartDa);
                        add3Values(edges,m,radiusSinAngleStart,-halfSize,radiusCosAngleStart);
                        add3Values(edges,m,radiusSinAngleStart,-halfSize,radiusCosAngleStart);
                        add3Values(edges,m,radiusSinAngleStart,+halfSize,radiusCosAngleStart);
                        add3Values(edges,m,radiusSinAngleStart,+halfSize,radiusCosAngleStart);
                        add3Values(edges,m,radiusSinAngleStartDa,+halfSize,radiusCosAngleStartDa);
                        tr.buildYRotation(-dAd*degToRad);
                        m=m*tr;
                    }
                    m=mSaved2;
                    tr.buildZRotation(piValue);
                    m=m*tr;
                }
            }
            m=mSaved;
        }
        if (!insidePart)
        {
            add3Values(edges,m,0.0,-halfSize,radius);
            add3Values(edges,m,0.0,+halfSize,radius);
        }
        // The far part:
        floatDouble dA2=angleStart/faceNumberFar;
        floatDouble dAd2=dA2*radToDeg;
        if (insidePart)
        {
            dA2=(angleStart-angleStart2)/faceNumberFar;
            dAd2=dA2*radToDeg;
        }
        floatDouble r=radius+range;
        floatDouble radiusSinAngleStart2=r*sin(angleStart);
        floatDouble radiusSinAngleStartDa2=r*sin(angleStart-dA2);
        floatDouble radiusCosAngleStart2=r*cos(angleStart);
        floatDouble radiusCosAngleStartDa2=r*cos(angleStart-dA2);
        C4X4Matrix mSaved(m);
        for (int j=0;j<2;j++)
        {
            C4X4Matrix mSaved2(m);
            for (int i=0;i<faceNumberFar;i++)
            {
                add3Values(edges,m,radiusSinAngleStartDa2,-halfSize,radiusCosAngleStartDa2);
                add3Values(edges,m,radiusSinAngleStart2,-halfSize,radiusCosAngleStart2);
                add3Values(edges,m,radiusSinAngleStart2,-halfSize,radiusCosAngleStart2);
                add3Values(edges,m,radiusSinAngleStart2,+halfSize,radiusCosAngleStart2);
                add3Values(edges,m,radiusSinAngleStart2,+halfSize,radiusCosAngleStart2);
                add3Values(edges,m,radiusSinAngleStartDa2,+halfSize,radiusCosAngleStartDa2);
                if (insidePart)
                {
                    add3Values(edges,m,radiusSinAngleStartDa2,-halfSize,radiusCosAngleStartDa2);
                    add3Values(edges,m,radiusSinAngleStartDa2,+halfSize,radiusCosAngleStartDa2);
                }
                tr.buildYRotation(-dAd2*degToRad);
                m=m*tr;
            }
            m=mSaved2;
            tr.buildZRotation(piValue);
            m=m*tr;
        }
        m=mSaved;
        if (!insidePart)
        {
            add3Values(edges,m,0.0,-halfSize,r);
            add3Values(edges,m,0.0,+halfSize,r);
        }


        // The borders:
        mSaved=m;
        if (angle<piValue*1.03f)
        {
            for (int i=0;i<2;i++)
            {
                add3Values(edges,m,radiusSinAngleStart,-halfSize,radiusCosAngleStart);
                add3Values(edges,m,radiusSinAngleStart2,-halfSize,radiusCosAngleStart2);
                add3Values(edges,m,radiusSinAngleStart,+halfSize,radiusCosAngleStart);
                add3Values(edges,m,radiusSinAngleStart2,+halfSize,radiusCosAngleStart2);
                if (insidePart)
                {   // The inside:
                    add3Values(edges,m,aaa*tan(angleStart2),-halfSize,aaa);
                    add3Values(edges,m,r*sin(angleStart2),-halfSize,r*cos(angleStart2));
                    add3Values(edges,m,aaa*tan(angleStart2),+halfSize,aaa);
                    add3Values(edges,m,r*sin(angleStart2),+halfSize,r*cos(angleStart2));
                    add3Values(edges,m,aaa*tan(angleStart2),-halfSize,aaa);
                    add3Values(edges,m,aaa*tan(angleStart2),+halfSize,aaa);
                }
                tr.buildZRotation(piValue);
                m=m*tr;
            }
        }
        m=mSaved;
    }

    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        tr.buildTranslation(0.0,0.0,offset);
        m=m*tr;
        floatDouble aD2=angle/2.0;
        floatDouble aD4=insideAngleThing*aD2;
        if (angle>piValue*1.01f)
        {
            aD2=piValue/2.0; // Then we draw 2 hemishperes
            insidePart=false;
        }
        floatDouble piDF=piValue/faceNumber;
        floatDouble cPiDF=cos(piDF);
        floatDouble sPiDF=sin(piDF);
        floatDouble sAD2=sin(aD2);
        floatDouble cAD2=cos(aD2);
        floatDouble sAD4=sin(aD4);
        floatDouble cAD4=cos(aD4);

        floatDouble tAD2=tan(aD2);
        floatDouble tAD4=tan(aD4);
        floatDouble r=radius;
        floatDouble R=radius+range;
        floatDouble rp=r*cAD2;
        for (int pass=0;pass<2;pass++)
        { // This loop is to draw 2 hemispheres if angle>piValue
            for (int i=0;i<faceNumber;i++)
            {
                // The edges:
                if (angle<piValue*0.999f)
                {
                    if (insidePart)
                        add3Values(edges,m,rp*tAD2*cPiDF,rp*tAD2*sPiDF,rp);
                    else
                        add3Values(edges,m,r*sAD2*cPiDF,r*sAD2*sPiDF,r*cAD2);
                    add3Values(edges,m,R*sAD2*cPiDF,R*sAD2*sPiDF,R*cAD2);
                }
                if (insidePart)
                {
                    add3Values(edges,m,rp*tAD4*cPiDF,rp*tAD4*sPiDF,rp);
                    add3Values(edges,m,R*sAD4*cPiDF,R*sAD4*sPiDF,R*cAD4);
                }
                // The close part:
                floatDouble da=aD2/(subdivisions+1);
                if (radius>0.0000000001f)
                {
                    if (insidePart)
                    {
                        if (angle<piValue*0.999f)
                        {
                            add3Values(edges,m,rp*tAD2*cPiDF,-rp*tAD2*sPiDF,rp);
                            add3Values(edges,m,rp*tAD2*cPiDF,+rp*tAD2*sPiDF,rp);
                        }
                        add3Values(edges,m,rp*tAD4*cPiDF,-rp*tAD4*sPiDF,rp);
                        add3Values(edges,m,rp*tAD4*cPiDF,+rp*tAD4*sPiDF,rp);
                    }
                    else
                    {
                        for (int j=0;j<=subdivisions;j++)
                        {
                            add3Values(edges,m,r*sin(aD2-j*da)*cPiDF,-r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da));
                            add3Values(edges,m,r*sin(aD2-j*da)*cPiDF,+r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da));
                            add3Values(edges,m,r*sin(aD2-j*da)*cPiDF,+r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da));
                            add3Values(edges,m,r*sin(aD2-(j+1)*da)*cPiDF,+r*sin(aD2-(j+1)*da)*sPiDF,r*cos(aD2-(j+1)*da));
                        }
                    }
                }

                if (insidePart)
                {
                    // The middle far ring:
                    add3Values(edges,m,R*sAD4*cPiDF,-R*sAD4*sPiDF,R*cAD4);
                    add3Values(edges,m,R*sAD4*cPiDF,+R*sAD4*sPiDF,R*cAD4);
                }
                // The far part:
                da=aD2/(subdivisionsFar+1);
                if (insidePart)
                    da=(aD2-aD4)/(subdivisionsFar+1);
                for (int j=0;j<=subdivisionsFar;j++)
                {
                    add3Values(edges,m,R*sin(aD2-j*da)*cPiDF,-R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da));
                    add3Values(edges,m,R*sin(aD2-j*da)*cPiDF,+R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da));
                    add3Values(edges,m,R*sin(aD2-j*da)*cPiDF,+R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da));
                    add3Values(edges,m,R*sin(aD2-(j+1)*da)*cPiDF,+R*sin(aD2-(j+1)*da)*sPiDF,R*cos(aD2-(j+1)*da));
                }
                tr.buildZRotation(piValue*2.0/faceNumber);
                m=m*tr;
            }
            if (angle<piValue*1.01f)
                break;
            // We have to prepare to draw the other hemisphere:
            tr.buildTranslation(0.0,0.0,-offset);
            m=m*tr;
            tr.buildXRotation(piValue);
            m=m*tr;
            tr.buildTranslation(0.0,0.0,-offset);
            m=m*tr;
        }
    }
}

void CConvexVolume::computeVolumes()
{
    if (_volumeComputationTemporarilyDisabled)
        return;
    const floatDouble normalsLength=0.03f;
    planesInside.clear();
    planesOutside.clear();
    normalsInside.clear();
    normalsOutside.clear();
    
    std::vector<floatDouble>* nInside=nullptr;
    std::vector<floatDouble>* nOutside=nullptr;

    nonDetectingVolumeEdges.clear();
    volumeEdges.clear();

    if (_smallestDistanceEnabled)
    {
        std::vector<floatDouble> volEdges;
        computeVolumeEdges(volEdges);
        getCloseAndFarVolumeEdges(volEdges,_smallestDistanceAllowed,nonDetectingVolumeEdges,volumeEdges);
    }
    else
        computeVolumeEdges(volumeEdges);

    C4X4Matrix m;
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        planesInside.reserve((2+faceNumber)*4);
        m.buildTranslation(0.0,0.0,offset+range);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(piValue);
        m.translate(0.0,0.0,offset);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        floatDouble alpha=piValue/faceNumber;
        m.buildYRotation(piValue/2.0+atan(cos(alpha)*(radiusFar-radius)/range));
        m.translate(cos(alpha)*(radius+radiusFar)/2.0,0.0,offset+range/2.0);
        for (int i=0;i<faceNumber;i++)
        {
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            m.rotateAroundZ(2.0*alpha);
        }
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        planesInside.reserve(6*4);
        m.buildTranslation(0.0,0.0,offset+range);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(piValue);
        m.translate(0.0,0.0,offset);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        floatDouble alphaV=atan((ySizeFar-ySize)/(2.0*range));
        floatDouble alphaH=atan((xSizeFar-xSize)/(2.0*range));
        m.buildXRotation(-piValue/2.0-alphaV);
        m.translate(0.0,+(ySizeFar+ySize)/4.0,offset+range/2.0);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildXRotation(piValue/2.0+alphaV);
        m.translate(0.0,-(ySizeFar+ySize)/4.0,offset+range/2.0);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(-piValue/2.0-alphaH);
        m.translate(-(xSizeFar+xSize)/4.0,0.0,offset+range/2.0);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(piValue/2.0+alphaH);
        m.translate(+(xSizeFar+xSize)/4.0,0.0,offset+range/2.0);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
    }
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        planesInside.reserve((2+faceNumber+faceNumberFar+2)*4);
        // Top and bottom parts:
        m.buildXRotation(-piValue/2.0);
        m.translate(0.0,ySize/2.0,offset+radius+range/2.0);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildXRotation(+piValue/2.0);
        m.translate(0.0,-ySize/2.0,offset+radius+range/2.0);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        // Far part:
        floatDouble dA2=0.5*angle/faceNumberFar;
        if (insidePart)
            dA2=0.5*angle*(1.0-insideAngleThing)/faceNumberFar;
        floatDouble ddd=0.0;
        if (insidePart)
            ddd=0.5*angle*insideAngleThing;
        floatDouble r2=(radius+range)*cos(dA2/2.0);
        for (int i=0;i<faceNumberFar;i++)
        {
            // From left
            m.buildTranslation(0.0,0.0,r2);
            m.rotateAroundY((i+0.5)*dA2+ddd);
            m.translate(0.0,0.0,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            // From right
            m.buildTranslation(0.0,0.0,r2);
            m.rotateAroundY(-(i+0.5)*dA2-ddd);
            m.translate(0.0,0.0,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
        }
        // Close part:
        floatDouble dA=0.5*angle/faceNumber;
        floatDouble r=radius*cos(dA/2.0);
        if (radius>0.0000000001f)
        {
            if (!insidePart)
            {
                for (int i=0;i<faceNumber;i++)
                {
                    // From left
                    m.buildTranslation(0.0,0.0,r);
                    m.rotateAroundY((i+0.5)*dA);
                    m.translate(0.0,0.0,offset);
                    addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                    // From right
                    m.buildTranslation(0.0,0.0,r);
                    m.rotateAroundY(-(i+0.5)*dA);
                    m.translate(0.0,0.0,offset);
                    addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                }
            }
            else
            {
                m.buildYRotation(piValue);
                m.translate(0.0,0.0,offset+radius*cos(0.5*angle));
                addAPlane(&planesInside,nInside,normalsLength,m,true);
            }
        }
        // The closing part (if angle != piValT2)
        if (angle<piValue*1.03f)
        {
            m.buildYRotation(piValue/2.0);
            m.translate(0.0,0.0,radius+range/2.0);
            m.rotateAroundY(angle/2.0);
            m.translate(0.0,0.0,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            m.buildYRotation(-piValue/2.0);
            m.translate(0.0,0.0,radius+range/2.0);
            m.rotateAroundY(-angle/2.0);
            m.translate(0.0,0.0,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            if (insidePart)
            {
                m.buildYRotation(piValue/2.0);
                m.translate(0.0,0.0,radius+range/2.0);
                m.rotateAroundY(angle*insideAngleThing/2.0);
                m.translate(0.0,0.0,offset);
                addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                m.buildYRotation(-piValue/2.0);
                m.translate(0.0,0.0,radius+range/2.0);
                m.rotateAroundY(-angle*insideAngleThing/2.0);
                m.translate(0.0,0.0,offset);
                addAPlane(&planesOutside,nOutside,normalsLength,m,false);
            }
        }
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        planesInside.reserve((faceNumber+(subdivisions+1)*faceNumber+(subdivisionsFar+1)*faceNumber)*4);
        floatDouble aD2=angle/2.0;
        floatDouble aD4=insideAngleThing*aD2;
        if (angle>piValue*1.01f)
        {
            aD2=piValue/2.0; // Then we draw 2 hemishperes
            insidePart=false;
        }
        floatDouble piDF=piValue/faceNumber;
        floatDouble cPiDF=cos(piDF);
        floatDouble sPiDF=sin(piDF);
        floatDouble cAD2=cos(aD2);
        floatDouble r=radius;
        floatDouble R=radius+range;
        floatDouble alpha=atan(tan(aD2)*cPiDF);
        floatDouble alpha2=atan(tan(aD4)*cPiDF);
        // First the sides:
        if (angle<piValue*1.01f)
        {
            C4X4Matrix mm;
            m.buildYRotation(piValue/2.0);
            m.translate(0.0,0.0,radius+0.3*range);
            m.rotateAroundY(alpha);
            m.translate(0.0,0.0,offset);
            mm.buildYRotation(piValue/2.0);
            mm.translate(0.0,0.0,radius+0.3*range);
            mm.rotateAroundY(alpha2);
            mm.translate(0.0,0.0,offset);
            for (int i=0;i<faceNumber;i++)
            {
                if (angle<piValue*0.999f)
                    addAPlane(&planesInside,nInside,normalsLength,m,true); // We have something smaller than a hemisphere
                m.rotateAroundZ(piDF*2.0);
                if (insidePart)
                    addAPlane(&planesOutside,nOutside,normalsLength,mm,false);
                mm.rotateAroundZ(piDF*2.0);
            }
            if (angle>=piValue*0.999f)
            { // Here we have a 'perfect' hemisphere
                m.buildYRotation(piValue);
                m.translate(0.0,0.0,offset);
                addAPlane(&planesInside,nInside,normalsLength,m,true);
            }
        }
        // Now the Far part:
        for (int pass=0;pass<2;pass++)
        { // This loop is to draw 2 hemispheres if angle>piValue
            for (int i=0;i<faceNumber;i++)
            {
                // The close part:
                if (insidePart)
                {
                    m.buildYRotation(piValue);
                    m.translate(0.0,0.0,r*cAD2+offset);
                    addAPlane(&planesInside,nInside,normalsLength,m,true);
                }
                else
                {
                    floatDouble da=aD2/(subdivisions+1);
                    if (radius>0.0000000001f)
                    {
                        for (int j=0;j<=subdivisions;j++)
                        {
                            floatDouble p1[3]={r*sin(aD2-j*da)*cPiDF,+r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da)};
                            floatDouble p2[3]={r*sin(aD2-(j+1)*da)*cPiDF,+r*sin(aD2-(j+1)*da)*sPiDF,r*cos(aD2-(j+1)*da)};
                            floatDouble p3[3]={r*sin(aD2-j*da)*cPiDF,-r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da)};
                            m=getTheMatrix(p1,p2,p3,j==subdivisions);
                            m.rotateAroundZ(2.0*i*piValue/faceNumber);
                            if (pass==1)
                                m.rotateAroundX(piValue);
                            m.translate(0.0,0.0,offset);
                            addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                        }
                    }
                }

                // The far part:
                floatDouble da=(aD2-aD4)/(subdivisionsFar+1);
                if (!insidePart)
                    da=aD2/(subdivisionsFar+1);
                for (int j=0;j<=subdivisionsFar;j++)
                {
                    floatDouble p1[3]={R*sin(aD2-j*da)*cPiDF,+R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da)};
                    floatDouble p2[3]={R*sin(aD2-(j+1)*da)*cPiDF,+R*sin(aD2-(j+1)*da)*sPiDF,R*cos(aD2-(j+1)*da)};
                    floatDouble p3[3]={R*sin(aD2-j*da)*cPiDF,-R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da)};
                    if (insidePart)
                        m=getTheMatrix(p1,p2,p3,false);
                    else
                        m=getTheMatrix(p1,p2,p3,j==subdivisionsFar);
                    m.rotateAroundZ(2.0*i*piValue/faceNumber);
                    if (pass==1)
                        m.rotateAroundX(piValue);
                    m.translate(0.0,0.0,offset);
                    addAPlane(&planesInside,nInside,normalsLength,m,true);
                }
            }
            if (angle<piValue*1.01f)
                break;
        }
    }

    if (_smallestDistanceEnabled)
    {
        if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
        {
            floatDouble l0=offset;
            floatDouble l1=offset+range;
            floatDouble m0=-_smallestDistanceAllowed;
            floatDouble m1=+_smallestDistanceAllowed;
            // We search for the interesection:
            if ((l0<m1)&&(l1>m0))
            { // we have an intersection!
                l0=std::max<floatDouble>(l0,m0);
                l1=std::min<floatDouble>(l1,m1);
                nonDetectingVolumeEdges.push_back(0.0);
                nonDetectingVolumeEdges.push_back(0.0);
                nonDetectingVolumeEdges.push_back(l0);
                nonDetectingVolumeEdges.push_back(0.0);
                nonDetectingVolumeEdges.push_back(0.0);
                nonDetectingVolumeEdges.push_back(l1);
            }
        }
        else
        {
            std::vector<floatDouble> volEdges;
            computeVolumeEdges(volEdges);
            getCloseAndFarVolumeEdges(volEdges,_smallestDistanceAllowed,nonDetectingVolumeEdges,volumeEdges);

            std::vector<floatDouble> sphere;
            generateSphereEdges(sphere,_smallestDistanceAllowed);
            removeEdgesNotInsideVolume(sphere,planesInside,false);
            removeEdgesNotInsideVolume(sphere,planesOutside,true);
            for (int i=0;i<int(sphere.size());i++)
                nonDetectingVolumeEdges.push_back(sphere[i]);
        }
    }

    // We mix the planes (faster calculations!)
    //*****************************************************
    std::vector<floatDouble> pTmp(planesInside);
    std::vector<floatDouble> nTmp(normalsInside);
    planesInside.clear();
    normalsInside.clear();
    while (pTmp.size()!=0)
    {
        int i=int(SIM_RAND_FLOAT*floatDouble(pTmp.size()))/4;
        if (i>=int(pTmp.size())/4)
            i--;
        planesInside.push_back(pTmp[4*i+0]);
        planesInside.push_back(pTmp[4*i+1]);
        planesInside.push_back(pTmp[4*i+2]);
        planesInside.push_back(pTmp[4*i+3]);
        pTmp.erase(pTmp.begin()+4*i,pTmp.begin()+4*i+4);
        if (nTmp.size()!=0)
        {
            normalsInside.push_back(nTmp[3*i+0]);
            normalsInside.push_back(nTmp[3*i+1]);
            normalsInside.push_back(nTmp[3*i+2]);
            nTmp.erase(nTmp.begin()+3*i,nTmp.begin()+3*i+3);
        }
    }


    pTmp.clear();
    pTmp.clear();
    pTmp.insert(pTmp.begin(),planesOutside.begin(),planesOutside.end());
    nTmp.insert(nTmp.begin(),normalsOutside.begin(),normalsOutside.end());
    planesOutside.clear();
    normalsOutside.clear();
    while (pTmp.size()!=0)
    {
        int i=int(SIM_RAND_FLOAT*floatDouble(pTmp.size()/4));
        if (i>=int(pTmp.size()/4))
            i--;
        planesOutside.push_back(pTmp[4*i+0]);
        planesOutside.push_back(pTmp[4*i+1]);
        planesOutside.push_back(pTmp[4*i+2]);
        planesOutside.push_back(pTmp[4*i+3]);
        pTmp.erase(pTmp.begin()+4*i,pTmp.begin()+4*i+4);
        if (nTmp.size()!=0)
        {
            normalsOutside.push_back(nTmp[3*i+0]);
            normalsOutside.push_back(nTmp[3*i+1]);
            normalsOutside.push_back(nTmp[3*i+2]);
            nTmp.erase(nTmp.begin()+3*i,nTmp.begin()+3*i+3);
        }
    }
    //*****************************************************
}

void CConvexVolume::getCloseAndFarVolumeEdges(std::vector<floatDouble>& allEdges,floatDouble distance,std::vector<floatDouble>& closeEdges,std::vector<floatDouble>& farEdges)
{
    if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
    {
        for (int i=0;i<int(allEdges.size());i++)
            farEdges.push_back(allEdges[i]);
        return;
    }
    for (int i=0;i<int(allEdges.size())/6;i++)
    {
        C3Vector v0(&allEdges[6*i+0]);
        C3Vector v1(&allEdges[6*i+3]);
        C3Vector dv(v1-v0);
        // 1. We calculate the intersection between above ray and the sphere:
        floatDouble a=v0*dv;
        floatDouble b=dv*dv;
        floatDouble c=4.0*a*a-4.0*b*((v0*v0)-distance*distance);
        bool noIntersection=true;
        if (c>0.0)
        { // we have two intersection points (for the ray!)
            // we calculate parameter t:
            floatDouble t0=(-2.0*a-sqrtf(c))/(2*b);
            floatDouble t1=(-2.0*a+sqrtf(c))/(2*b);
            if ( (t0<1.0)&&(t1>0.0) )
            {
                if ( (t0>0.0)||(t1<1.0) )
                { // The segment is intersecting!
                    noIntersection=false;
                    // 3 cases:
                    if (t0<=0.0)
                    {
                        C3Vector intersection(v0+dv*t1);
                        closeEdges.push_back(v0(0));
                        closeEdges.push_back(v0(1));
                        closeEdges.push_back(v0(2));
                        closeEdges.push_back(intersection(0));
                        closeEdges.push_back(intersection(1));
                        closeEdges.push_back(intersection(2));
                        farEdges.push_back(intersection(0));
                        farEdges.push_back(intersection(1));
                        farEdges.push_back(intersection(2));
                        farEdges.push_back(v1(0));
                        farEdges.push_back(v1(1));
                        farEdges.push_back(v1(2));
                    }
                    else if (t1>=1.0)
                    {
                        C3Vector intersection(v0+dv*t0);
                        closeEdges.push_back(intersection(0));
                        closeEdges.push_back(intersection(1));
                        closeEdges.push_back(intersection(2));
                        closeEdges.push_back(v1(0));
                        closeEdges.push_back(v1(1));
                        closeEdges.push_back(v1(2));
                        farEdges.push_back(v0(0));
                        farEdges.push_back(v0(1));
                        farEdges.push_back(v0(2));
                        farEdges.push_back(intersection(0));
                        farEdges.push_back(intersection(1));
                        farEdges.push_back(intersection(2));
                    }
                    else
                    { // We have 2 segments here
                        C3Vector intersection1(v0+dv*t0);
                        C3Vector intersection2(v0+dv*t1);
                        farEdges.push_back(v0(0));
                        farEdges.push_back(v0(1));
                        farEdges.push_back(v0(2));
                        farEdges.push_back(intersection1(0));
                        farEdges.push_back(intersection1(1));
                        farEdges.push_back(intersection1(2));
                        closeEdges.push_back(intersection1(0));
                        closeEdges.push_back(intersection1(1));
                        closeEdges.push_back(intersection1(2));
                        closeEdges.push_back(intersection2(0));
                        closeEdges.push_back(intersection2(1));
                        closeEdges.push_back(intersection2(2));
                        farEdges.push_back(intersection2(0));
                        farEdges.push_back(intersection2(1));
                        farEdges.push_back(intersection2(2));
                        farEdges.push_back(v1(0));
                        farEdges.push_back(v1(1));
                        farEdges.push_back(v1(2));
                    }
                }
            }
        }
        if (noIntersection)
        {
            if (v0.getLength()<distance)
            { // close part
                closeEdges.push_back(v0(0));
                closeEdges.push_back(v0(1));
                closeEdges.push_back(v0(2));
                closeEdges.push_back(v1(0));
                closeEdges.push_back(v1(1));
                closeEdges.push_back(v1(2));
            }
            else
            { // far part
                farEdges.push_back(v0(0));
                farEdges.push_back(v0(1));
                farEdges.push_back(v0(2));
                farEdges.push_back(v1(0));
                farEdges.push_back(v1(1));
                farEdges.push_back(v1(2));
            }
        }
    }
}

void CConvexVolume::removeEdgesNotInsideVolume(std::vector<floatDouble>& edges,std::vector<floatDouble>& planes,bool invertSides)
{
    std::vector<floatDouble> theEdges(edges);
    edges.clear();

    for (int ed=0;ed<int(theEdges.size())/6;ed++)
    {
        C3Vector v0(&theEdges[6*ed+0]);
        C3Vector v1(&theEdges[6*ed+3]);
        bool addIt=(!invertSides)||(planes.size()==0);
        for (int pl=0;pl<int(planes.size())/4;pl++)
        {
            C4X4Matrix m;
            m.M.axis[2].setData(&planes[4*pl+0]);
            if (fabs(m.M.axis[2]*C3Vector::unitXVector)<0.9)
                m.M.axis[0]=(C3Vector::unitXVector^m.M.axis[2]).getNormalized();
            else
                m.M.axis[0]=(C3Vector::unitYVector^m.M.axis[2]).getNormalized();
            m.M.axis[1]=(m.M.axis[2]^m.M.axis[0]).getNormalized();
            m.X=m.M.axis[2]*-(planes[4*pl+3]);
            C4X4Matrix mInv(m.getInverse());

            C3Vector v0w(mInv*v0);
            C3Vector v1w(mInv*v1);
            if ( (v0w(2)<=0.0)&&(v1w(2)<=0.0) )
            { // segment lies inside one plane
            }
            else if ( (v0w(2)>=0.0)&&(v1w(2)>=0.0) )
            { // segment lies outside one plane --> outside the volume
                if (invertSides)
                { // volume 2, add it and leave
                    edges.push_back(v0(0));
                    edges.push_back(v0(1));
                    edges.push_back(v0(2));
                    edges.push_back(v1(0));
                    edges.push_back(v1(1));
                    edges.push_back(v1(2));
                    break; 
                }
                else
                { // volume 1
                    addIt=false;
                    break;
                }
            }
            else
            { // the segment intersects!
                if (v0w(2)>0.0)
                { // we keep v0w below
                    v1w=v0w;
                    v0w=mInv*v1;
                }
                C3Vector dv(v1w-v0w);
                floatDouble t=fabs(v0w(2))/dv(2);
                C3Vector intersection(v0w+dv*t);
                if (invertSides)
                {
                    v0=m*intersection;
                    v1=m*v1w;
                    edges.push_back(v0(0));
                    edges.push_back(v0(1));
                    edges.push_back(v0(2));
                    edges.push_back(v1(0));
                    edges.push_back(v1(1));
                    edges.push_back(v1(2));
                    // and we continue
                    v1=m*v0w;
                }
                else
                {
                    v0=m*v0w;
                    v1=m*intersection;
                }
            }
        }
        if (addIt)
        {
            edges.push_back(v0(0));
            edges.push_back(v0(1));
            edges.push_back(v0(2));
            edges.push_back(v1(0));
            edges.push_back(v1(1));
            edges.push_back(v1(2));
        }
    }
}

void CConvexVolume::generateSphereEdges(std::vector<floatDouble>& edges,floatDouble radius)
{
    int subDivW=12;
    int subDivV=12;
    floatDouble dw=piValT2/floatDouble(subDivW);
    floatDouble dv=piValue/floatDouble(subDivV);

    floatDouble v0=0.0;
    floatDouble v1=dv;
    for (int i=0;i<subDivV;i++)
    {
        floatDouble w0=0.0;
        floatDouble w1=dw;
        for (int j=0;j<subDivW;j++)
        {
            C3Vector p0(radius*cos(w0)*sin(v0),radius*sin(w0)*sin(v0),radius*cos(v0));
            edges.push_back(p0(0));
            edges.push_back(p0(1));
            edges.push_back(p0(2));
            C3Vector p1(radius*cos(w0)*sin(v1),radius*sin(w0)*sin(v1),radius*cos(v1));
            edges.push_back(p1(0));
            edges.push_back(p1(1));
            edges.push_back(p1(2));
            edges.push_back(p1(0));
            edges.push_back(p1(1));
            edges.push_back(p1(2));
            C3Vector p2(radius*cos(w1)*sin(v1),radius*sin(w1)*sin(v1),radius*cos(v1));
            edges.push_back(p2(0));
            edges.push_back(p2(1));
            edges.push_back(p2(2));
            w0+=dw;
            w1+=dw;
        }
        v0+=dv;
        v1+=dv;
    }
}
