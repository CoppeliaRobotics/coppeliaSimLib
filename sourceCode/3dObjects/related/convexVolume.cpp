
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "convexVolume.h"
#include "tt.h"
#include "global.h"
#include "4X4Matrix.h"
#include "6Vector.h"

CConvexVolume::CConvexVolume()
{
    commonInit();
}

CConvexVolume::~CConvexVolume()
{
}

void CConvexVolume::commonInit()
{
    _smallestDistanceAllowed=0.1f;
    _smallestDistanceEnabled=false;

    _volumeComputationTemporarilyDisabled=false;
    _volumeType=CYLINDER_TYPE_CONVEX_VOLUME;
    offset=0.1f;
    range=0.2f;
    radius=0.1f;
    radiusFar=0.2f;
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
        retM.X=(pt0+pt1+pt2)/3.0f;
    else
        retM.X=(pt1+pt2)/2.0f;
    return(retM);
}

void CConvexVolume::add3Values(std::vector<float>& vect,const C4X4Matrix& transf,float x,float y,float z)
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

            ar.storeDataName("Ss3");
            ar << offset << range << xSize << ySize << xSizeFar << ySizeFar;
            ar << radius << radiusFar << angle;
            ar.flush();

            ar.storeDataName("Sia");
            ar << insideAngleThing;
            ar.flush();

            ar.storeDataName("Ssi");
            ar << faceNumber << faceNumberFar << subdivisions << subdivisionsFar;
            ar.flush();

            ar.storeDataName("Wtf");
            ar << _smallestDistanceAllowed;
            ar.flush();

            ar.storeDataName("Par");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_smallestDistanceEnabled);
            ar << nothing;
            ar.flush();

            // Following added on 2010/04/28 because recomputing those values takes long:
            //***************************************************************************
            ar.storeDataName("Nc0");
            ar << int(planesInside.size());
            for (int i=0;i<int(planesInside.size());i++)
                ar << planesInside[i];
            ar.flush();

            ar.storeDataName("Nc1");
            ar << int(planesOutside.size());
            for (int i=0;i<int(planesOutside.size());i++)
                ar << planesOutside[i];
            ar.flush();

            ar.storeDataName("Nc2");
            ar << int(normalsInside.size());
            for (int i=0;i<int(normalsInside.size());i++)
                ar << normalsInside[i];
            ar.flush();

            ar.storeDataName("Nc3");
            ar << int(normalsOutside.size());
            for (int i=0;i<int(normalsOutside.size());i++)
                ar << normalsOutside[i];
            ar.flush();

            ar.storeDataName("Nc4");
            ar << int(volumeEdges.size());
            for (int i=0;i<int(volumeEdges.size());i++)
                ar << volumeEdges[i];
            ar.flush();

            ar.storeDataName("Nc5");
            ar << int(nonDetectingVolumeEdges.size());
            for (int i=0;i<int(nonDetectingVolumeEdges.size());i++)
                ar << nonDetectingVolumeEdges[i];
            ar.flush();

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
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> offset >> range >> xSize >> ySize >> xSizeFar >> ySizeFar;
                        ar >> radius >> radiusFar >> angle;
                    }
                    if (theName.compare("Sia")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> insideAngleThing;
                    }
                    if (theName.compare("Ssi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> faceNumber >> faceNumberFar >> subdivisions >> subdivisionsFar;
                    }
                    if (theName.compare("Wtf")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _smallestDistanceAllowed;
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
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        planesInside.clear();
                        float dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar >> dum;
                            planesInside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc1")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        planesOutside.clear();
                        float dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar >> dum;
                            planesOutside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc2")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        normalsInside.clear();
                        float dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar >> dum;
                            normalsInside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc3")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        normalsOutside.clear();
                        float dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar >> dum;
                            normalsOutside.push_back(dum);
                        }
                    }
                    if (theName.compare("Nc4")==0)
                    {
                        recomputeVolumes=false;
                        noHit=false;
                        ar >> byteQuantity;
                        int vectSize;
                        ar >> vectSize;
                        volumeEdges.clear();
                        float dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar >> dum;
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
                        float dum;
                        for (int i=0;i<vectSize;i++)
                        {
                            ar >> dum;
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
}

bool CConvexVolume::getVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const
{ 
    if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
    {
        minV(0)=-0.001f;
        maxV(0)=+0.001f;
        minV(1)=-0.001f;
        maxV(1)=+0.001f;
        minV(2)=0.0f;
        if (offset<0.0f)
            minV(2)=offset;
        maxV(2)=offset+range;
        if (maxV(2)<0.0f)
            maxV(2)=0.0f;
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        float maxX=SIM_MAX(xSize,xSizeFar)/2.0f;
        minV(0)=-maxX;
        maxV(0)=maxX;
        float maxY=SIM_MAX(ySize,ySizeFar)/2.0f;
        minV(1)=-maxY;
        maxV(1)=maxY;
        minV(2)=0.0f;
        if (offset<0.0f)
            minV(2)=offset;
        maxV(2)=offset+range;
        if (maxV(2)<0.0f)
            maxV(2)=0.0f;
    }
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        float maxRad=SIM_MAX(radius,radiusFar);
        minV(0)=-maxRad;
        maxV(0)=maxRad;
        minV(1)=-maxRad;
        maxV(1)=maxRad;
        minV(2)=0.0f;
        if (offset<0.0f)
            minV(2)=offset;
        maxV(2)=offset+range;
        if (maxV(2)<0.0f)
            maxV(2)=0.0f;
    }
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        minV(1)=-ySize/2.0f;
        maxV(1)=ySize/2.0f;
        float tmp;
        if (angle<piValue_f*1.03f)
            tmp=(radius+range)*sin(angle*0.5f);
        else
            tmp=radius+range;
        minV(0)=-tmp;
        maxV(0)=tmp;
        if (angle<piValue_f*1.03f)
        {
            minV(2)=0.0f;
            if (offset<0.0f)
                minV(2)=offset;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0f)
                maxV(2)=0.0f;
        }
        else
        {
            minV(2)=offset-radius-range;
            if (minV(2)>0.0f)
                minV(2)=0.0f;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0f)
                maxV(2)=0.0f;
        }
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        float tmp;
        if (angle<piValue_f*1.03f)
            tmp=(radius+range)*sin(angle*0.5f);
        else
            tmp=radius+range;
        minV(0)=-tmp;
        maxV(0)=tmp;
        minV(1)=-tmp;
        maxV(1)=tmp;
        if (angle<piValue_f*1.03f)
        {
            minV(2)=0.0f;
            if (offset<0.0f)
                minV(2)=offset;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0f)
                maxV(2)=0.0f;
        }
        else
        {
            minV(2)=offset-radius-range;
            if (minV(2)>0.0f)
                minV(2)=0.0f;
            maxV(2)=offset+radius+range;
            if (maxV(2)<0.0f)
                maxV(2)=0.0f;
        }
    }
    return(true);
}

void CConvexVolume::scaleVolume(float scalingFactor)
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

void CConvexVolume::scaleVolumeNonIsometrically(float x,float y,float z,float& xRet,float& yRet,float& zRet)
{
    float xy=sqrt(x*y);
    float xyz=cbrt(x*y*z);
    float xz=sqrt(x*z);
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

void CConvexVolume::addAPlane(std::vector<float>* volume,std::vector<float>* normals,float nL,const C4X4Matrix& m,bool inside)
{   // normals can be nullptr.
    // Always z-axis:
    volume->push_back(m.M.axis[2](0));
    volume->push_back(m.M.axis[2](1));
    volume->push_back(m.M.axis[2](2));
    volume->push_back(-(m.M.axis[2]*m.X));
    if (normals!=nullptr)
    {
        C6Vector v(m.X,m.X+(m.M.axis[2]*nL));
        for (int i=0;i<6;i++)
            normals->push_back(v(i));
    }
}

void CConvexVolume::setVolumeType(int theType,int objectTypeTheVolumeIsFor,float pointSize)
{
    _volumeType=theType;
    setDefaultVolumeParameters(objectTypeTheVolumeIsFor,pointSize);
    computeVolumes();
}

int CConvexVolume::getVolumeType()
{
    return(_volumeType);
}

void CConvexVolume::setOffset(float theOffset)
{
    tt::limitValue(-1000.0f,1000.0f,theOffset);
    offset=theOffset;
    computeVolumes();
}

float CConvexVolume::getOffset()
{
    return(offset);
}

void CConvexVolume::setRange(float theRange)
{
    tt::limitValue(0.00001f,1000.0f,theRange);
    range=theRange;
    solveInterferences();
    computeVolumes();
}

float CConvexVolume::getRange()
{
    return(range);
}

void CConvexVolume::setXSize(float theXSize)
{
    tt::limitValue(0.00001f,1000.0f,theXSize);
    xSize=theXSize;
    computeVolumes();
}

float CConvexVolume::getXSize()
{
    return(xSize);
}

void CConvexVolume::setYSize(float theYSize)
{
    tt::limitValue(0.00001f,1000.0f,theYSize);
    ySize=theYSize;
    computeVolumes();
}

float CConvexVolume::getYSize()
{
    return(ySize);
}

void CConvexVolume::setXSizeFar(float theXSizeFar)
{
    tt::limitValue(0.00001f,1000.0f,theXSizeFar);
    xSizeFar=theXSizeFar;
    computeVolumes();
}

float CConvexVolume::getXSizeFar()
{
    return(xSizeFar);
}

void CConvexVolume::setYSizeFar(float theYSizeFar)
{
    tt::limitValue(0.00001f,1000.0f,theYSizeFar);
    ySizeFar=theYSizeFar;
    computeVolumes();
}

float CConvexVolume::getYSizeFar()
{
    return(ySizeFar);
}

void CConvexVolume::setRadius(float theRadius)
{
    tt::limitValue(0.0f,1000.0f,theRadius);
    if ( (_volumeType==DISC_TYPE_CONVEX_VOLUME)||(_volumeType==CONE_TYPE_CONVEX_VOLUME) )
    {
        if (theRadius<0.00001f)
            theRadius=0.0f;
    }
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        if (theRadius<0.00001f)
            theRadius=0.00001f;
    }
    radius=theRadius;
    solveInterferences();
    computeVolumes();
}

float CConvexVolume::getRadius()
{
    return(radius);
}

void CConvexVolume::setRadiusFar(float theRadiusFar)
{
    tt::limitValue(0.0f,1000.0f,theRadiusFar);
    radiusFar=theRadiusFar;
    computeVolumes();
}

float CConvexVolume::getRadiusFar()
{
    return(radiusFar);
}

void CConvexVolume::setAngle(float theAngle)
{
    tt::limitValue(0.001f*degToRad_f,piValTimes2_f,theAngle);
    if ( (_volumeType==DISC_TYPE_CONVEX_VOLUME)||(_volumeType==CONE_TYPE_CONVEX_VOLUME) )
    {
        if ( (theAngle>piValue_f)&&(theAngle<piValue_f*1.01f) )
            theAngle=piValue_f;
        if (theAngle>=piValue_f*1.01f)
            theAngle=piValTimes2_f;
    }
    angle=theAngle;
    solveInterferences();
    computeVolumes();
}

void CConvexVolume::setInsideAngleThing(float theAngle)
{
    tt::limitValue(0.0f,0.999f,theAngle);
    if (theAngle<0.01f)
        theAngle=0.0f;
    insideAngleThing=theAngle;
    if (theAngle!=0.0f)
    {
        if (angle>piValue_f*1.01f)
            angle=piValue_f;
    }
    solveInterferences();
    computeVolumes();
}

void CConvexVolume::setSmallestDistanceEnabled(bool e)
{
    _smallestDistanceEnabled=e;
    computeVolumes();
}

bool CConvexVolume::getSmallestDistanceEnabled()
{
    return(_smallestDistanceEnabled);
}

void CConvexVolume::setSmallestDistanceAllowed(float d)
{
    tt::limitValue(0.0001f,10.0f,d);
    _smallestDistanceAllowed=d;
    computeVolumes();
}

float CConvexVolume::getSmallestDistanceAllowed()
{
    return(_smallestDistanceAllowed);
}

void CConvexVolume::solveInterferences()
{
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        float dA2=angle/faceNumberFar;
        float r2=(radius+range)*cos(dA2/2.0f);
        while ( (r2<=radius)&&(faceNumberFar<PROXSENSOR_MAX_FACE_NUMBER) )
        { // We have an interference here!
            faceNumberFar=faceNumberFar+1;
            dA2=angle/faceNumberFar;
            r2=(radius+range)*cos(dA2/2.0f);
        }
        while (r2<=radius)
        { // We have to increase the range now:
            range=1.25f*range;
            r2=(radius+range)*cos(dA2/2.0f);
        }
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        float dA2=angle/(subdivisionsFar+1);
        float r2=(radius+range)*cos(dA2/4.0f);
        while ( (r2<=radius)&&(subdivisionsFar<PROXSENSOR_MAX_SUBDIVISIONS) )
        { // We have an interference here!
            subdivisionsFar=subdivisionsFar+1;
            dA2=angle/(subdivisionsFar+1);
            r2=(radius+range)*cos(dA2/4.0f);
        }
        while (r2<=radius)
        { // We have to increase the range now:
            range=1.25f*range;
            r2=(radius+range)*cos(dA2/4.0f);
        }
    }
}

float CConvexVolume::getAngle()
{
    return(angle);
}

float CConvexVolume::getInsideAngleThing()
{
    return(insideAngleThing);
}

void CConvexVolume::setFaceNumber(int theFaceNumber)
{
    tt::limitValue(1,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
        tt::limitValue(3,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        if (angle>piValue_f*1.01f)
            tt::limitValue(3,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
        tt::limitValue(3,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumber);
    faceNumber=theFaceNumber;
    computeVolumes();
}

int CConvexVolume::getFaceNumber()
{
    return(faceNumber);
}

void CConvexVolume::setFaceNumberFar(int theFaceNumberFar)
{
    tt::limitValue(1,PROXSENSOR_MAX_FACE_NUMBER,theFaceNumberFar);
    faceNumberFar=theFaceNumberFar;
    solveInterferences();
    computeVolumes();
}

int CConvexVolume::getFaceNumberFar()
{
    return(faceNumberFar);
}

void CConvexVolume::setSubdivisions(int theSubdivisions)
{
    tt::limitValue(0,PROXSENSOR_MAX_SUBDIVISIONS,theSubdivisions);
    subdivisions=theSubdivisions;
    computeVolumes();
}

int CConvexVolume::getSubdivisions()
{
    return(subdivisions);
}

void CConvexVolume::setSubdivisionsFar(int theSubdivisionsFar)
{
    tt::limitValue(0,PROXSENSOR_MAX_SUBDIVISIONS,theSubdivisionsFar);
    subdivisionsFar=theSubdivisionsFar;
    solveInterferences();
    computeVolumes();
}

int CConvexVolume::getSubdivisionsFar()
{
    return(subdivisionsFar);
}

void CConvexVolume::setDefaultVolumeParameters(int objectTypeTheVolumeIsFor,float pointSize)
{
    insideAngleThing=0.0f;
    _smallestDistanceAllowed=0.1f;
    _smallestDistanceEnabled=false;
    if (objectTypeTheVolumeIsFor==sim_object_proximitysensor_type)
    {
        if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
        {
            offset=0.1f;
            range=0.2f;
            radius=0.1f;
            radiusFar=0.2f;
            faceNumber=32;
        }
        if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
        {
            offset=0.0f;
            range=0.2f;
            radius=0.1f;
            angle=piValD2_f;
            ySize=0.05f;
            ySizeFar=0.1f;
            faceNumber=16;
            faceNumberFar=32;
        }
        if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
        {
            offset=0.1f;
            range=0.2f;
            xSize=0.2f;
            ySize=0.1f;
            xSizeFar=0.4f;
            ySizeFar=0.2f;
        }
        if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
        {
            offset=0.1f;
            range=0.4f;
        }
        if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
        {
            offset=0.0f;
            radius=pointSize/2.0f;
            range=0.3f-radius;
            angle=piValD2_f;
            faceNumber=32;
            subdivisions=1;
            subdivisionsFar=16;
            _smallestDistanceEnabled=true;
        }
    }
    if (objectTypeTheVolumeIsFor==sim_object_mill_type)
    {
        if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
        {
            offset=0.0f;
            range=0.05f;
            radius=0.015f;
            radiusFar=0.015f;
            faceNumber=32;
        }
        if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
        {
            offset=0.0f;
            range=0.05f;
            radius=0.0f;
            angle=30.0f*degToRad_f;
            ySize=0.015f;
            ySizeFar=0.015f;
            faceNumber=1;
            faceNumberFar=4;
        }
        if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
        {
            offset=0.0f;
            range=0.05f;
            xSize=0.02f;
            ySize=0.02f;
            xSizeFar=0.02f;
            ySizeFar=0.02f;
        }
        if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
        { // Should not be possible
            offset=0.0f;
            range=0.05f;
        }
        if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
        {
            offset=0.0f;
            radius=0.0f;    // no outside volume here
            range=0.05f;
            angle=30.0f*degToRad_f;
            faceNumber=16;
            subdivisions=1;
            subdivisionsFar=4;
        }
    }
}

void CConvexVolume::computeVolumeEdges(std::vector<float>& edges)
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
        float alpha=piValue_f/faceNumber;
        for (int i=0;i<faceNumber;i++)
        {
            add3Values(edges,m,+cos(alpha)*radius,-sin(alpha)*radius,offset);
            add3Values(edges,m,+cos(alpha)*radius,+sin(alpha)*radius,offset);
            add3Values(edges,m,+cos(alpha)*radius,+sin(alpha)*radius,offset);
            add3Values(edges,m,+cos(alpha)*radiusFar,+sin(alpha)*radiusFar,offset+range);
            add3Values(edges,m,+cos(alpha)*radiusFar,+sin(alpha)*radiusFar,offset+range);
            add3Values(edges,m,+cos(alpha)*radiusFar,-sin(alpha)*radiusFar,offset+range);
            tr.buildZRotation(piValTimes2_f/faceNumber);
            m=m*tr;
        }
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        add3Values(edges,m,+xSize/2.0f,-ySize/2.0f,offset);
        add3Values(edges,m,+xSize/2.0f,+ySize/2.0f,offset);
        add3Values(edges,m,+xSize/2.0f,+ySize/2.0f,offset);
        add3Values(edges,m,-xSize/2.0f,+ySize/2.0f,offset);
        add3Values(edges,m,-xSize/2.0f,+ySize/2.0f,offset);
        add3Values(edges,m,-xSize/2.0f,-ySize/2.0f,offset);
        add3Values(edges,m,-xSize/2.0f,-ySize/2.0f,offset);
        add3Values(edges,m,+xSize/2.0f,-ySize/2.0f,offset);
        add3Values(edges,m,+xSize/2.0f,-ySize/2.0f,offset);
        add3Values(edges,m,+xSizeFar/2.0f,-ySizeFar/2.0f,offset+range);
        add3Values(edges,m,+xSizeFar/2.0f,-ySizeFar/2.0f,offset+range);
        add3Values(edges,m,+xSizeFar/2.0f,+ySizeFar/2.0f,offset+range);
        add3Values(edges,m,+xSizeFar/2.0f,+ySizeFar/2.0f,offset+range);
        add3Values(edges,m,-xSizeFar/2.0f,+ySizeFar/2.0f,offset+range);
        add3Values(edges,m,-xSizeFar/2.0f,+ySizeFar/2.0f,offset+range);
        add3Values(edges,m,-xSizeFar/2.0f,-ySizeFar/2.0f,offset+range);
        add3Values(edges,m,-xSizeFar/2.0f,-ySizeFar/2.0f,offset+range);
        add3Values(edges,m,+xSizeFar/2.0f,-ySizeFar/2.0f,offset+range);
        add3Values(edges,m,+xSize/2.0f,+ySize/2.0f,offset);
        add3Values(edges,m,+xSizeFar/2.0f,+ySizeFar/2.0f,offset+range);
        add3Values(edges,m,-xSize/2.0f,+ySize/2.0f,offset);
        add3Values(edges,m,-xSizeFar/2.0f,+ySizeFar/2.0f,offset+range);
        add3Values(edges,m,-xSize/2.0f,-ySize/2.0f,offset);
        add3Values(edges,m,-xSizeFar/2.0f,-ySizeFar/2.0f,offset+range);
    }

    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        tr.buildTranslation(0.0f,0.0f,offset);
        m=m*tr;
        float angleStart=angle/2.0f;
        float angleStart2=angleStart*insideAngleThing;
        float dA=angleStart/faceNumber;
        float dAd=dA*radToDeg_f;
        if (insidePart)
        {
            dA=angleStart2/faceNumber;
            dAd=dA*radToDeg_f;
        }
        float radiusSinAngleStart=radius*sin(angleStart);
        float radiusSinAngleStartDa=radius*sin(angleStart-dA);
        float radiusCosAngleStart=radius*cos(angleStart);
        float radiusCosAngleStartDa=radius*cos(angleStart-dA);
        float halfSize=ySize/2.0f;
        float aaa=cos(angleStart)*radius;
        float bbb=sin(angleStart)*radius;
        float bb2=tan(angleStart2)*aaa;
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

                    if (angle<piValue_f*0.999f)
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
                        tr.buildYRotation(-dAd*degToRad_f);
                        m=m*tr;
                    }
                    m=mSaved2;
                    tr.buildZRotation(piValue_f);
                    m=m*tr;
                }
            }
            m=mSaved;
        }
        if (!insidePart)
        {
            add3Values(edges,m,0.0f,-halfSize,radius);
            add3Values(edges,m,0.0f,+halfSize,radius);
        }
        // The far part:
        float dA2=angleStart/faceNumberFar;
        float dAd2=dA2*radToDeg_f;
        if (insidePart)
        {
            dA2=(angleStart-angleStart2)/faceNumberFar;
            dAd2=dA2*radToDeg_f;
        }
        float r=radius+range;
        float radiusSinAngleStart2=r*sin(angleStart);
        float radiusSinAngleStartDa2=r*sin(angleStart-dA2);
        float radiusCosAngleStart2=r*cos(angleStart);
        float radiusCosAngleStartDa2=r*cos(angleStart-dA2);
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
                tr.buildYRotation(-dAd2*degToRad_f);
                m=m*tr;
            }
            m=mSaved2;
            tr.buildZRotation(piValue_f);
            m=m*tr;
        }
        m=mSaved;
        if (!insidePart)
        {
            add3Values(edges,m,0.0f,-halfSize,r);
            add3Values(edges,m,0.0f,+halfSize,r);
        }


        // The borders:
        mSaved=m;
        if (angle<piValue_f*1.03f)
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
                tr.buildZRotation(piValue_f);
                m=m*tr;
            }
        }
        m=mSaved;
    }

    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        tr.buildTranslation(0.0f,0.0f,offset);
        m=m*tr;
        float aD2=angle/2.0f;
        float aD4=insideAngleThing*aD2;
        if (angle>piValue_f*1.01f)
        {
            aD2=piValue_f/2.0f; // Then we draw 2 hemishperes
            insidePart=false;
        }
        float piDF=piValue_f/faceNumber;
        float cPiDF=cos(piDF);
        float sPiDF=sin(piDF);
        float sAD2=sin(aD2);
        float cAD2=cos(aD2);
        float sAD4=sin(aD4);
        float cAD4=cos(aD4);

        float tAD2=tan(aD2);
        float tAD4=tan(aD4);
        float r=radius;
        float R=radius+range;
        float rp=r*cAD2;
        for (int pass=0;pass<2;pass++)
        { // This loop is to draw 2 hemispheres if angle>piValue
            for (int i=0;i<faceNumber;i++)
            {
                // The edges:
                if (angle<piValue_f*0.999f)
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
                float da=aD2/(subdivisions+1);
                if (radius>0.0000000001f)
                {
                    if (insidePart)
                    {
                        if (angle<piValue_f*0.999f)
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
                tr.buildZRotation(piValue_f*2.0f/faceNumber);
                m=m*tr;
            }
            if (angle<piValue_f*1.01f)
                break;
            // We have to prepare to draw the other hemisphere:
            tr.buildTranslation(0.0f,0.0f,-offset);
            m=m*tr;
            tr.buildXRotation(piValue_f);
            m=m*tr;
            tr.buildTranslation(0.0f,0.0f,-offset);
            m=m*tr;
        }
    }
}

void CConvexVolume::computeVolumes()
{
    if (_volumeComputationTemporarilyDisabled)
        return;
    const float normalsLength=0.03f;
    planesInside.clear();
    planesOutside.clear();
    normalsInside.clear();
    normalsOutside.clear();
    
    std::vector<float>* nInside=nullptr;
    std::vector<float>* nOutside=nullptr;

    nonDetectingVolumeEdges.clear();
    volumeEdges.clear();

    if (_smallestDistanceEnabled)
    {
        std::vector<float> volEdges;
        computeVolumeEdges(volEdges);
        getCloseAndFarVolumeEdges(volEdges,_smallestDistanceAllowed,nonDetectingVolumeEdges,volumeEdges);
    }
    else
        computeVolumeEdges(volumeEdges);

    C4X4Matrix m;
    if (_volumeType==CYLINDER_TYPE_CONVEX_VOLUME)
    {
        planesInside.reserve((2+faceNumber)*4);
        m.buildTranslation(0.0f,0.0f,offset+range);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(piValue_f);
        m.translate(0.0f,0.0f,offset);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        float alpha=piValue_f/faceNumber;
        m.buildYRotation(piValue_f/2.0f+atan(cos(alpha)*(radiusFar-radius)/range));
        m.translate(cos(alpha)*(radius+radiusFar)/2.0f,0.0f,offset+range/2.0f);
        for (int i=0;i<faceNumber;i++)
        {
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            m.rotateAroundZ(2.0f*alpha);
        }
    }
    if (_volumeType==PYRAMID_TYPE_CONVEX_VOLUME)
    {
        planesInside.reserve(6*4);
        m.buildTranslation(0.0f,0.0f,offset+range);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(piValue_f);
        m.translate(0.0f,0.0f,offset);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        float alphaV=atan((ySizeFar-ySize)/(2.0f*range));
        float alphaH=atan((xSizeFar-xSize)/(2.0f*range));
        m.buildXRotation(-piValue_f/2.0f-alphaV);
        m.translate(0.0f,+(ySizeFar+ySize)/4.0f,offset+range/2.0f);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildXRotation(piValue_f/2.0f+alphaV);
        m.translate(0.0f,-(ySizeFar+ySize)/4.0f,offset+range/2.0f);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(-piValue_f/2.0f-alphaH);
        m.translate(-(xSizeFar+xSize)/4.0f,0.0f,offset+range/2.0f);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildYRotation(piValue_f/2.0f+alphaH);
        m.translate(+(xSizeFar+xSize)/4.0f,0.0f,offset+range/2.0f);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
    }
    if (_volumeType==DISC_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        planesInside.reserve((2+faceNumber+faceNumberFar+2)*4);
        // Top and bottom parts:
        m.buildXRotation(-piValue_f/2.0f);
        m.translate(0.0f,ySize/2.0f,offset+radius+range/2.0f);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        m.buildXRotation(+piValue_f/2.0f);
        m.translate(0.0f,-ySize/2.0f,offset+radius+range/2.0f);
        addAPlane(&planesInside,nInside,normalsLength,m,true);
        // Far part:
        float dA2=0.5f*angle/faceNumberFar;
        if (insidePart)
            dA2=0.5f*angle*(1.0f-insideAngleThing)/faceNumberFar;
        float ddd=0.0f;
        if (insidePart)
            ddd=0.5f*angle*insideAngleThing;
        float r2=(radius+range)*cos(dA2/2.0f);
        for (int i=0;i<faceNumberFar;i++)
        {
            // From left
            m.buildTranslation(0.0f,0.0f,r2);
            m.rotateAroundY((i+0.5f)*dA2+ddd);
            m.translate(0.0f,0.0f,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            // From right
            m.buildTranslation(0.0f,0.0f,r2);
            m.rotateAroundY(-(i+0.5f)*dA2-ddd);
            m.translate(0.0f,0.0f,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
        }
        // Close part:
        float dA=0.5f*angle/faceNumber;
        float r=radius*cos(dA/2.0f);
        if (radius>0.0000000001f)
        {
            if (!insidePart)
            {
                for (int i=0;i<faceNumber;i++)
                {
                    // From left
                    m.buildTranslation(0.0f,0.0f,r);
                    m.rotateAroundY((i+0.5f)*dA);
                    m.translate(0.0f,0.0f,offset);
                    addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                    // From right
                    m.buildTranslation(0.0f,0.0f,r);
                    m.rotateAroundY(-(i+0.5f)*dA);
                    m.translate(0.0f,0.0f,offset);
                    addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                }
            }
            else
            {
                m.buildYRotation(piValue_f);
                m.translate(0.0f,0.0f,offset+radius*cos(0.5f*angle));
                addAPlane(&planesInside,nInside,normalsLength,m,true);
            }
        }
        // The closing part (if angle != piValTimes2_f)
        if (angle<piValue_f*1.03f)
        {
            m.buildYRotation(piValue_f/2.0f);
            m.translate(0.0f,0.0f,radius+range/2.0f);
            m.rotateAroundY(angle/2.0f);
            m.translate(0.0f,0.0f,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            m.buildYRotation(-piValue_f/2.0f);
            m.translate(0.0f,0.0f,radius+range/2.0f);
            m.rotateAroundY(-angle/2.0f);
            m.translate(0.0f,0.0f,offset);
            addAPlane(&planesInside,nInside,normalsLength,m,true);
            if (insidePart)
            {
                m.buildYRotation(piValue_f/2.0f);
                m.translate(0.0f,0.0f,radius+range/2.0f);
                m.rotateAroundY(angle*insideAngleThing/2.0f);
                m.translate(0.0f,0.0f,offset);
                addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                m.buildYRotation(-piValue_f/2.0f);
                m.translate(0.0f,0.0f,radius+range/2.0f);
                m.rotateAroundY(-angle*insideAngleThing/2.0f);
                m.translate(0.0f,0.0f,offset);
                addAPlane(&planesOutside,nOutside,normalsLength,m,false);
            }
        }
    }
    if (_volumeType==CONE_TYPE_CONVEX_VOLUME)
    {
        bool insidePart=(insideAngleThing>=0.01f);
        planesInside.reserve((faceNumber+(subdivisions+1)*faceNumber+(subdivisionsFar+1)*faceNumber)*4);
        float aD2=angle/2.0f;
        float aD4=insideAngleThing*aD2;
        if (angle>piValue_f*1.01f)
        {
            aD2=piValue_f/2.0f; // Then we draw 2 hemishperes
            insidePart=false;
        }
        float piDF=piValue_f/faceNumber;
        float cPiDF=cos(piDF);
        float sPiDF=sin(piDF);
        float cAD2=cos(aD2);
        float r=radius;
        float R=radius+range;
        float alpha=atan(tan(aD2)*cPiDF);
        float alpha2=atan(tan(aD4)*cPiDF);
        // First the sides:
        if (angle<piValue_f*1.01f)
        {
            C4X4Matrix mm;
            m.buildYRotation(piValue_f/2.0f);
            m.translate(0.0f,0.0f,radius+0.3f*range);
            m.rotateAroundY(alpha);
            m.translate(0.0f,0.0f,offset);
            mm.buildYRotation(piValue_f/2.0f);
            mm.translate(0.0f,0.0f,radius+0.3f*range);
            mm.rotateAroundY(alpha2);
            mm.translate(0.0f,0.0f,offset);
            for (int i=0;i<faceNumber;i++)
            {
                if (angle<piValue_f*0.999f)
                    addAPlane(&planesInside,nInside,normalsLength,m,true); // We have something smaller than a hemisphere
                m.rotateAroundZ(piDF*2.0f);
                if (insidePart)
                    addAPlane(&planesOutside,nOutside,normalsLength,mm,false);
                mm.rotateAroundZ(piDF*2.0f);
            }
            if (angle>=piValue_f*0.999f)
            { // Here we have a 'perfect' hemisphere
                m.buildYRotation(piValue_f);
                m.translate(0.0f,0.0f,offset);
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
                    m.buildYRotation(piValue_f);
                    m.translate(0.0f,0.0f,r*cAD2+offset);
                    addAPlane(&planesInside,nInside,normalsLength,m,true);
                }
                else
                {
                    float da=aD2/(subdivisions+1);
                    if (radius>0.0000000001f)
                    {
                        for (int j=0;j<=subdivisions;j++)
                        {
                            float p1[3]={r*sin(aD2-j*da)*cPiDF,+r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da)};
                            float p2[3]={r*sin(aD2-(j+1)*da)*cPiDF,+r*sin(aD2-(j+1)*da)*sPiDF,r*cos(aD2-(j+1)*da)};
                            float p3[3]={r*sin(aD2-j*da)*cPiDF,-r*sin(aD2-j*da)*sPiDF,r*cos(aD2-j*da)};
                            m=getTheMatrix(p1,p2,p3,j==subdivisions);
                            m.rotateAroundZ(2.0f*i*piValue_f/faceNumber);
                            if (pass==1)
                                m.rotateAroundX(piValue_f);
                            m.translate(0.0f,0.0f,offset);
                            addAPlane(&planesOutside,nOutside,normalsLength,m,false);
                        }
                    }
                }

                // The far part:
                float da=(aD2-aD4)/(subdivisionsFar+1);
                if (!insidePart)
                    da=aD2/(subdivisionsFar+1);
                for (int j=0;j<=subdivisionsFar;j++)
                {
                    float p1[3]={R*sin(aD2-j*da)*cPiDF,+R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da)};
                    float p2[3]={R*sin(aD2-(j+1)*da)*cPiDF,+R*sin(aD2-(j+1)*da)*sPiDF,R*cos(aD2-(j+1)*da)};
                    float p3[3]={R*sin(aD2-j*da)*cPiDF,-R*sin(aD2-j*da)*sPiDF,R*cos(aD2-j*da)};
                    if (insidePart)
                        m=getTheMatrix(p1,p2,p3,false);
                    else
                        m=getTheMatrix(p1,p2,p3,j==subdivisionsFar);
                    m.rotateAroundZ(2.0f*i*piValue_f/faceNumber);
                    if (pass==1)
                        m.rotateAroundX(piValue_f);
                    m.translate(0.0f,0.0f,offset);
                    addAPlane(&planesInside,nInside,normalsLength,m,true);
                }
            }
            if (angle<piValue_f*1.01f)
                break;
        }
    }

    if (_smallestDistanceEnabled)
    {
        if (_volumeType==RAY_TYPE_CONVEX_VOLUME)
        {
            float l0=offset;
            float l1=offset+range;
            float m0=-_smallestDistanceAllowed;
            float m1=+_smallestDistanceAllowed;
            // We search for the interesection:
            if ((l0<m1)&&(l1>m0))
            { // we have an intersection!
                l0=SIM_MAX(l0,m0);
                l1=SIM_MIN(l1,m1);
                nonDetectingVolumeEdges.push_back(0.0f);
                nonDetectingVolumeEdges.push_back(0.0f);
                nonDetectingVolumeEdges.push_back(l0);
                nonDetectingVolumeEdges.push_back(0.0f);
                nonDetectingVolumeEdges.push_back(0.0f);
                nonDetectingVolumeEdges.push_back(l1);
            }
        }
        else
        {
            std::vector<float> volEdges;
            computeVolumeEdges(volEdges);
            getCloseAndFarVolumeEdges(volEdges,_smallestDistanceAllowed,nonDetectingVolumeEdges,volumeEdges);

            std::vector<float> sphere;
            generateSphereEdges(sphere,_smallestDistanceAllowed);
            removeEdgesNotInsideVolume(sphere,planesInside,false);
            removeEdgesNotInsideVolume(sphere,planesOutside,true);
            for (int i=0;i<int(sphere.size());i++)
                nonDetectingVolumeEdges.push_back(sphere[i]);
        }
    }

    // We mix the planes (faster calculations!)
    //*****************************************************
    std::vector<float> pTmp(planesInside);
    std::vector<float> nTmp(normalsInside);
    planesInside.clear();
    normalsInside.clear();
    while (pTmp.size()!=0)
    {
        int i=int(SIM_RAND_FLOAT*float(pTmp.size()))/4;
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
        int i=int(SIM_RAND_FLOAT*float(pTmp.size()/4));
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

void CConvexVolume::getCloseAndFarVolumeEdges(std::vector<float>& allEdges,float distance,std::vector<float>& closeEdges,std::vector<float>& farEdges)
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
        float a=v0*dv;
        float b=dv*dv;
        float c=4.0f*a*a-4.0f*b*((v0*v0)-distance*distance);
        bool noIntersection=true;
        if (c>0.0f)
        { // we have two intersection points (for the ray!)
            // we calculate parameter t:
            float t0=(-2.0f*a-sqrtf(c))/(2*b);
            float t1=(-2.0f*a+sqrtf(c))/(2*b);
            if ( (t0<1.0f)&&(t1>0.0f) )
            {
                if ( (t0>0.0f)||(t1<1.0f) )
                { // The segment is intersecting!
                    noIntersection=false;
                    // 3 cases:
                    if (t0<=0.0f)
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
                    else if (t1>=1.0f)
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

void CConvexVolume::removeEdgesNotInsideVolume(std::vector<float>& edges,std::vector<float>& planes,bool invertSides)
{
    std::vector<float> theEdges(edges);
    edges.clear();

    for (int ed=0;ed<int(theEdges.size())/6;ed++)
    {
        C3Vector v0(&theEdges[6*ed+0]);
        C3Vector v1(&theEdges[6*ed+3]);
        bool addIt=(!invertSides)||(planes.size()==0);
        for (int pl=0;pl<int(planes.size())/4;pl++)
        {
            C4X4Matrix m;
            m.M.axis[2].set(&planes[4*pl+0]);
            if (fabs(m.M.axis[2]*C3Vector::unitXVector)<0.9f)
                m.M.axis[0]=(C3Vector::unitXVector^m.M.axis[2]).getNormalized();
            else
                m.M.axis[0]=(C3Vector::unitYVector^m.M.axis[2]).getNormalized();
            m.M.axis[1]=(m.M.axis[2]^m.M.axis[0]).getNormalized();
            m.X=m.M.axis[2]*-(planes[4*pl+3]);
            C4X4Matrix mInv(m.getInverse());

            C3Vector v0w(mInv*v0);
            C3Vector v1w(mInv*v1);
            if ( (v0w(2)<=0.0f)&&(v1w(2)<=0.0f) )
            { // segment lies inside one plane
            }
            else if ( (v0w(2)>=0.0f)&&(v1w(2)>=0.0f) )
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
                if (v0w(2)>0.0f)
                { // we keep v0w below
                    v1w=v0w;
                    v0w=mInv*v1;
                }
                C3Vector dv(v1w-v0w);
                float t=fabs(v0w(2))/dv(2);
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

void CConvexVolume::generateSphereEdges(std::vector<float>& edges,float radius)
{
    int subDivW=12;
    int subDivV=12;
    float dw=piValTimes2_f/float(subDivW);
    float dv=piValue_f/float(subDivV);

    float v0=0.0f;
    float v1=dv;
    for (int i=0;i<subDivV;i++)
    {
        float w0=0.0f;
        float w1=dw;
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
