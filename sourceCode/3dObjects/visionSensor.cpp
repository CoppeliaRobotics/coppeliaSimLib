
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "visionSensor.h"
#include "v_rep_internal.h"
#include "global.h"
#include "tt.h"
#include "meshManip.h"
#include "imageProcess.h"
#include "sceneObjectOperations.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>
#include "vDateTime.h"
#include "vVarious.h"
#include "ttUtil.h"
#include "threadPool.h"
#include "easyLock.h"
#include "app.h"
#include "pluginContainer.h"
#include "visionSensorRendering.h"
#ifdef SIM_WITH_OPENGL
#include "rendering.h"
#include "oGL.h"
#include <QtOpenGL>
#endif

#define DEFAULT_RENDERING_ATTRIBUTES (sim_displayattribute_renderpass|sim_displayattribute_forbidwireframe|sim_displayattribute_forbidedges|sim_displayattribute_originalcolors|sim_displayattribute_ignorelayer|sim_displayattribute_forvisionsensor)
#define DEFAULT_RAYTRACING_ATTRIBUTES (sim_displayattribute_renderpass|sim_displayattribute_forbidwireframe|sim_displayattribute_forbidedges|sim_displayattribute_originalcolors|sim_displayattribute_ignorelayer|sim_displayattribute_forvisionsensor)

CVisionSensor::CVisionSensor()
{
    commonInit();
}

unsigned char* CVisionSensor::getRgbBufferPointer()
{
    return(_rgbBuffer);
}

float* CVisionSensor::getDepthBufferPointer()
{
    return(_depthBuffer);
}

std::string CVisionSensor::getObjectTypeInfo() const
{
    return(IDSOGL_VISION_SENSOR);
}
std::string CVisionSensor::getObjectTypeInfoExtended() const
{
    std::string retVal;
    retVal=IDSOGL_VISION_SENSOR_SAMPLING_RESOLUTION;
    retVal+=boost::lexical_cast<std::string>(_resolutionX)+"x";
    retVal+=boost::lexical_cast<std::string>(_resolutionY)+")";
    return(retVal);
}
bool CVisionSensor::isPotentiallyCollidable() const
{
    return(false);
}
bool CVisionSensor::isPotentiallyMeasurable() const
{
    return(false);
}
bool CVisionSensor::isPotentiallyDetectable() const
{
    return(false);
}
bool CVisionSensor::isPotentiallyRenderable() const
{
    return(false);
}
bool CVisionSensor::isPotentiallyCuttable() const
{
    return(false);
}

float* CVisionSensor::readPortionOfImage(int posX,int posY,int sizeX,int sizeY,int rgbGreyOrDepth)
{
    if ( (posX<0)||(posY<0)||(sizeX<1)||(sizeY<1)||(posX+sizeX>_resolutionX)||(posY+sizeY>_resolutionY) )
        return(nullptr);
    float* buff;
    if (rgbGreyOrDepth==0)
        buff=new float[sizeX*sizeY*3];
    else
        buff=new float[sizeX*sizeY];
    int p=0;
    for (int j=posY;j<posY+sizeY;j++)
    {
        for (int i=posX;i<posX+sizeX;i++)
        {
            if (rgbGreyOrDepth==0)
            { // RGB
                buff[3*p+0]=float(_rgbBuffer[3*(j*_resolutionX+i)+0])/255.0f;
                buff[3*p+1]=float(_rgbBuffer[3*(j*_resolutionX+i)+1])/255.0f;
                buff[3*p+2]=float(_rgbBuffer[3*(j*_resolutionX+i)+2])/255.0f;
            }
            else
            {
                if (rgbGreyOrDepth==1)
                { // Greyscale
                    buff[p]=float(_rgbBuffer[3*(j*_resolutionX+i)+0])/255.0f;
                    buff[p]+=float(_rgbBuffer[3*(j*_resolutionX+i)+1])/255.0f;
                    buff[p]+=float(_rgbBuffer[3*(j*_resolutionX+i)+2])/255.0f;
                    buff[p]/=3.0f;
                }
                else
                    buff[p]=_depthBuffer[j*_resolutionX+i];
            }
            p++;
        }
    }
    return(buff);
}

unsigned char* CVisionSensor::readPortionOfCharImage(int posX,int posY,int sizeX,int sizeY,float cutoffRgba,bool imgIsGreyScale)
{
    if ( (posX<0)||(posY<0)||(sizeX<1)||(sizeY<1)||(posX+sizeX>_resolutionX)||(posY+sizeY>_resolutionY) )
        return(nullptr);
    unsigned char* buff=nullptr;
    if (cutoffRgba==0.0f)
    {
        if (imgIsGreyScale)
            buff=new unsigned char[sizeX*sizeY];
        else
            buff=new unsigned char[sizeX*sizeY*3];
        int p=0;
        for (int j=posY;j<posY+sizeY;j++)
        {
            for (int i=posX;i<posX+sizeX;i++)
            {
                if (imgIsGreyScale)
                {
                    unsigned int v=_rgbBuffer[3*(j*_resolutionX+i)+0];
                    v+=_rgbBuffer[3*(j*_resolutionX+i)+1];
                    v+=_rgbBuffer[3*(j*_resolutionX+i)+2];
                    buff[p]=(unsigned char)(v/3);
                }
                else
                {
                    buff[3*p+0]=_rgbBuffer[3*(j*_resolutionX+i)+0];
                    buff[3*p+1]=_rgbBuffer[3*(j*_resolutionX+i)+1];
                    buff[3*p+2]=_rgbBuffer[3*(j*_resolutionX+i)+2];
                }
                p++;
            }
        }
    }
    else
    {
        if (imgIsGreyScale)
            buff=new unsigned char[sizeX*sizeY*2];
        else
            buff=new unsigned char[sizeX*sizeY*4];
        int p=0;
        for (int j=posY;j<posY+sizeY;j++)
        {
            for (int i=posX;i<posX+sizeX;i++)
            {
                if (imgIsGreyScale)
                {
                    unsigned int v=_rgbBuffer[3*(j*_resolutionX+i)+0];
                    v+=_rgbBuffer[3*(j*_resolutionX+i)+1];
                    v+=_rgbBuffer[3*(j*_resolutionX+i)+2];
                    buff[2*p+0]=(unsigned char)(v/3);
                    if (_depthBuffer[j*_resolutionX+i]>cutoffRgba)
                        buff[2*p+1]=0;
                    else
                        buff[2*p+1]=255;
                }
                else
                {
                    buff[4*p+0]=_rgbBuffer[3*(j*_resolutionX+i)+0];
                    buff[4*p+1]=_rgbBuffer[3*(j*_resolutionX+i)+1];
                    buff[4*p+2]=_rgbBuffer[3*(j*_resolutionX+i)+2];
                    if (_depthBuffer[j*_resolutionX+i]>cutoffRgba)
                        buff[4*p+3]=0;
                    else
                        buff[4*p+3]=255;
                }
                p++;
            }
        }
    }
    return(buff);
}

bool CVisionSensor::getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    minV(0)=-0.5f*_size(0);
    maxV(0)=0.5f*_size(0);
    minV(1)=-0.5f*_size(1);
    maxV(1)=0.5f*_size(1);
    minV(2)=-_size(2);
    maxV(2)=0.0f;

    C3Vector c,f,e;
    getSensingVolumeCorners(c,f);
    for (float fi=-1.0f;fi<2.0f;fi+=2.0f)
    {
        for (float fj=-1.0f;fj<2.0f;fj+=2.0f)
        {
            e(0)=fi*c(0);
            e(1)=fj*c(1);
            e(2)=c(2);
            minV.keepMin(e);
            maxV.keepMax(e);
            e(0)=fi*f(0);
            e(1)=fj*f(1);
            e(2)=f(2);
            minV.keepMin(e);
            maxV.keepMax(e);
        }
    }
    return(true);
}

bool CVisionSensor::getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const
{
    return(getFullBoundingBox(minV,maxV));
}

void CVisionSensor::commonInit()
{
    setObjectType(sim_object_visionsensor_type);
    _nearClippingPlane=0.01f;
    _farClippingPlane=10.0f;
    _viewAngle=60.0f*degToRad_f;
    _orthoViewSize=0.01f;
    _showFogIfAvailable=true;
    _useLocalLights=false;
    _rayTracingTextureName=(unsigned int)-1;

    layer=VISION_SENSOR_LAYER;
    _detectableEntityID=-1;
    _localObjectSpecialProperty=0;
    _explicitHandling=false;
    _showVolumeWhenNotDetecting=true;
    _showVolumeWhenDetecting=true;

    _ignoreRGBInfo=false;
    _ignoreDepthInfo=false;
    _computeImageBasicStats=true;
    _renderMode=sim_rendermode_opengl; // visible
    _attributesForRendering=DEFAULT_RENDERING_ATTRIBUTES;

    _initialValuesInitialized=false;

    _useExternalImage=false;
    _useSameBackgroundAsEnvironment=true;

    _composedFilter=new CComposedFilter();
    CSimpleFilter* it=new CSimpleFilter();
    it->setFilterType(sim_filtercomponent_originalimage);
    _composedFilter->insertSimpleFilter(it);
    it=new CSimpleFilter();
    it->setFilterType(sim_filtercomponent_tooutput);
    _composedFilter->insertSimpleFilter(it);

    _defaultBufferValues[0]=0.0f;
    _defaultBufferValues[1]=0.0f;
    _defaultBufferValues[2]=0.0f;

    sensorResult.sensorWasTriggered=false;
    sensorResult.sensorResultIsValid=false;
    sensorResult.calcTimeInMs=0;
    sensorAuxiliaryResult.clear();
    color.setDefaultValues();
    color.setColor(0.05f,0.42f,1.0f,sim_colorcomponent_ambient_diffuse);
    activeColor.setColorsAllBlack();
    activeColor.setColor(1.0f,0.0f,0.0f,sim_colorcomponent_emission);
    activeColor.setFlash(true);

    _resolutionX=32;
    _resolutionY=32;
    _desiredResolution[0]=32;
    _desiredResolution[1]=32;
    _size(0)=_orthoViewSize;
    _size(1)=_size(0);
    _size(2)=_size(0)*3.0f;
    _perspectiveOperation=false;
    if (_extensionString.size()!=0)
        _extensionString+=" ";
    _extensionString+="povray {focalBlur {false} focalDist {2.00} aperture{0.05} blurSamples{10}}";

    // Following means full-size for windowed external renderings:
    _extWindowedViewSize[0]=0;
    _extWindowedViewSize[1]=0;

    _extWindowedViewPos[0]=0;
    _extWindowedViewPos[1]=0;

#ifdef SIM_WITH_OPENGL
    _contextFboAndTexture=nullptr;
#endif

    _rgbBuffer=nullptr;
    _depthBuffer=nullptr;
    _reserveBuffers();

    _objectManipulationModePermissions=0x013;

    _objectName=IDSOGL_VISION_U_SENSOR;
    _objectAltName=tt::getObjectAltNameFromObjectName(_objectName);
}

void CVisionSensor::setUseExternalImage(bool u)
{
    _useExternalImage=u;
}

bool CVisionSensor::getUseExternalImage()
{
    return(_useExternalImage);
}

bool CVisionSensor::getInternalRendering()
{
    return(_renderMode<sim_rendermode_povray);
}

bool CVisionSensor::getApplyExternalRenderedImage()
{
    return( (_renderMode==sim_rendermode_povray)||(_renderMode==sim_rendermode_extrenderer)||(_renderMode==sim_rendermode_opengl3) );
}

CComposedFilter* CVisionSensor::getComposedFilter()
{
    return(_composedFilter);
}

void CVisionSensor::setComposedFilter(CComposedFilter* newFilter)
{
    delete _composedFilter;
    _composedFilter=newFilter;
}

CVisualParam* CVisionSensor::getColor(bool colorWhenActive)
{
    if (colorWhenActive)
        return(&activeColor);
    return(&color);
}

void CVisionSensor::setExtWindowSizeAndPos(int sizeX,int sizeY,int posX,int posY)
{
    _extWindowedViewSize[0]=sizeX;
    _extWindowedViewSize[1]=sizeY;
    _extWindowedViewPos[0]=posX;
    _extWindowedViewPos[1]=posY;
}

void CVisionSensor::getExtWindowSizeAndPos(int& sizeX,int& sizeY,int& posX,int& posY)
{
    sizeX=_extWindowedViewSize[0];
    sizeY=_extWindowedViewSize[1];
    posX=_extWindowedViewPos[0];
    posY=_extWindowedViewPos[1];
}

CVisionSensor::~CVisionSensor()
{
#ifdef SIM_WITH_OPENGL
    _removeGlContextAndFboAndTextureObjectIfNeeded();
#endif
    if (_rayTracingTextureName!=(unsigned int)-1)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=DESTROY_GL_TEXTURE_UITHREADCMD;
        cmdIn.uintParams.push_back(_rayTracingTextureName);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        _rayTracingTextureName=(unsigned int)-1;
    }

    delete[] _depthBuffer;
    delete[] _rgbBuffer;

    delete _composedFilter;
}

float CVisionSensor::getCalculationTime()
{
    return(float(sensorResult.calcTimeInMs)*0.001f);
}

void CVisionSensor::_reserveBuffers()
{
    delete[] _depthBuffer;
    delete[] _rgbBuffer;
    _depthBuffer=new float[_resolutionX*_resolutionY];
    _rgbBuffer=new unsigned char[3*_resolutionX*_resolutionY];
    _clearBuffers();
#ifdef SIM_WITH_OPENGL
    _removeGlContextAndFboAndTextureObjectIfNeeded();
#endif
}

void CVisionSensor::_clearBuffers()
{
    for (int i=0;i<_resolutionX*_resolutionY;i++)
    {
        if (_useSameBackgroundAsEnvironment)
        {
            _rgbBuffer[3*i+0]=(unsigned char)(App::ct->environment->fogBackgroundColor[0]*255.1f);
            _rgbBuffer[3*i+1]=(unsigned char)(App::ct->environment->fogBackgroundColor[1]*255.1f);
            _rgbBuffer[3*i+2]=(unsigned char)(App::ct->environment->fogBackgroundColor[2]*255.1f);
        }
        else
        {
            _rgbBuffer[3*i+0]=(unsigned char)(_defaultBufferValues[0]*255.1f);
            _rgbBuffer[3*i+1]=(unsigned char)(_defaultBufferValues[1]*255.1f);
            _rgbBuffer[3*i+2]=(unsigned char)(_defaultBufferValues[2]*255.1f);
        }
    }
    for (int i=0;i<_resolutionX*_resolutionY;i++)
        _depthBuffer[i]=1.0f;
}

bool CVisionSensor::getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const
{
    vertices.clear();
    indices.clear();
    return(false); // for now
}

void CVisionSensor::getSensingVolumeCorners(C3Vector& sizeAndPosClose,C3Vector& sizeAndPosFar) const
{
    float r=float(_resolutionY)/float(_resolutionX);
    if (_perspectiveOperation)
    {
        if (r<=1.0f)
        { // x is bigger
            sizeAndPosFar(0)=tan(_viewAngle/2.0f)*_farClippingPlane;
            sizeAndPosFar(1)=sizeAndPosFar(0)*r;
            sizeAndPosFar(2)=_farClippingPlane;

            sizeAndPosClose(0)=tan(_viewAngle/2.0f)*_nearClippingPlane;
            sizeAndPosClose(1)=sizeAndPosClose(0)*r;
            sizeAndPosClose(2)=_nearClippingPlane;
        }
        else
        { // y is bigger
            sizeAndPosFar(1)=tan(_viewAngle/2.0f)*_farClippingPlane;
            sizeAndPosFar(0)=sizeAndPosFar(1)/r;
            sizeAndPosFar(2)=_farClippingPlane;

            sizeAndPosClose(1)=tan(_viewAngle/2.0f)*_nearClippingPlane;
            sizeAndPosClose(0)=sizeAndPosClose(1)/r;
            sizeAndPosClose(2)=_nearClippingPlane;
        }
    }
    else
    {
        if (r<=1.0f)
            sizeAndPosFar(0)=_orthoViewSize/2.0f; // x is bigger
        else
            sizeAndPosFar(0)=_orthoViewSize/(2.0f*r); // y is bigger
        sizeAndPosFar(1)=sizeAndPosFar(0)*r;
        sizeAndPosFar(2)=_farClippingPlane;

        sizeAndPosClose(0)=sizeAndPosFar(0);
        sizeAndPosClose(1)=sizeAndPosFar(1);
        sizeAndPosClose(2)=_nearClippingPlane;
    }
}

void CVisionSensor::getRealResolution(int r[2])
{
    r[0]=_resolutionX;
    r[1]=_resolutionY;
}

void CVisionSensor::setDesiredResolution(int r[2])
{
    tt::limitValue(1,4096,r[0]);
    tt::limitValue(1,4096,r[1]);
    _desiredResolution[0]=r[0];
    _desiredResolution[1]=r[1];
    _resolutionX=_desiredResolution[0];
    _resolutionY=_desiredResolution[1];
    _reserveBuffers();
    if (_composedFilter!=nullptr)
        _composedFilter->removeBuffers();
}

void CVisionSensor::getDesiredResolution(int r[2])
{
    r[0]=_desiredResolution[0];
    r[1]=_desiredResolution[1];
}

void CVisionSensor::setUseEnvironmentBackgroundColor(bool s)
{
    _useSameBackgroundAsEnvironment=s;
}

bool CVisionSensor::getUseEnvironmentBackgroundColor()
{
    return(_useSameBackgroundAsEnvironment);
}

void CVisionSensor::setSize(const C3Vector& s)
{
    _size=s;
    for (int i=0;i<3;i++)
        tt::limitValue(0.0001f,100.0f,_size(i));
}

C3Vector CVisionSensor::getSize()
{
    return(_size);
}

void CVisionSensor::setPerspectiveOperation(bool p)
{
    _perspectiveOperation=p;
}

bool CVisionSensor::getPerspectiveOperation()
{
    return(_perspectiveOperation);
}

void CVisionSensor::setExplicitHandling(bool explicitHandl)
{
    _explicitHandling=explicitHandl;
}

bool CVisionSensor::getExplicitHandling()
{
    return (_explicitHandling);
}

void CVisionSensor::setShowVolumeWhenNotDetecting(bool s)
{
    _showVolumeWhenNotDetecting=s;
}
bool CVisionSensor::getShowVolumeWhenNotDetecting()
{
    return(_showVolumeWhenNotDetecting);
}
void CVisionSensor::setShowVolumeWhenDetecting(bool s)
{
    _showVolumeWhenDetecting=s;
}
bool CVisionSensor::getShowVolumeWhenDetecting()
{
    return(_showVolumeWhenDetecting);
}

void CVisionSensor::setIgnoreRGBInfo(bool ignore)
{
    _ignoreRGBInfo=ignore;
}
bool CVisionSensor::getIgnoreRGBInfo()
{
    return(_ignoreRGBInfo);
}

void CVisionSensor::setComputeImageBasicStats(bool c)
{
    _computeImageBasicStats=c;
}

bool CVisionSensor::getComputeImageBasicStats()
{
    return(_computeImageBasicStats);
}

void CVisionSensor::setIgnoreDepthInfo(bool ignore)
{
    _ignoreDepthInfo=ignore;
}
bool CVisionSensor::getIgnoreDepthInfo()
{
    return(_ignoreDepthInfo);
}

void CVisionSensor::setRenderMode(int mode)
{
    if (_renderMode!=mode)
        _attributesForRendering=DEFAULT_RENDERING_ATTRIBUTES;
    _renderMode=mode;
    if (mode==sim_rendermode_povray)
    {
        _ignoreDepthInfo=true;
        _attributesForRendering=DEFAULT_RAYTRACING_ATTRIBUTES;
    }
}

int CVisionSensor::getRenderMode()
{
    return(_renderMode);
}

void CVisionSensor::setAttributesForRendering(int attr)
{
    _attributesForRendering=attr;
}

int CVisionSensor::getAttributesForRendering()
{
    return(_attributesForRendering);
}

void CVisionSensor::setDetectableEntityID(int objID)
{
    _detectableEntityID=objID;
}

int CVisionSensor::getDetectableEntityID()
{
    return(_detectableEntityID);
}

void CVisionSensor::setDefaultBufferValues(const float v[3])
{
    for (int i=0;i<3;i++)
        _defaultBufferValues[i]=v[i];
}

void CVisionSensor::getDefaultBufferValues(float v[3])
{
    for (int i=0;i<3;i++)
        v[i]=_defaultBufferValues[i];
}

void CVisionSensor::resetSensor()
{
    sensorAuxiliaryResult.clear();
    sensorResult.sensorWasTriggered=false;
    sensorResult.sensorResultIsValid=false;
    sensorResult.calcTimeInMs=0;
    for (int i=0;i<3;i++)
    {
        sensorResult.sensorDataRed[i]=0;
        sensorResult.sensorDataGreen[i]=0;
        sensorResult.sensorDataBlue[i]=0;
        sensorResult.sensorDataIntensity[i]=0;
        sensorResult.sensorDataDepth[i]=0.0f;
    }
    if (!_useExternalImage) // condition added on 2010/12/21
        _clearBuffers();
}

bool CVisionSensor::setExternalImage(const float* img,bool imgIsGreyScale)
{
    if (imgIsGreyScale)
    {
        int n=_resolutionX*_resolutionY;
        for (int i=0;i<n;i++)
        {
            unsigned char v=(unsigned char)(img[i]*255.1f);
            _rgbBuffer[3*i+0]=v;
            _rgbBuffer[3*i+1]=v;
            _rgbBuffer[3*i+2]=v;
        }
    }
    else
    {
        int n=_resolutionX*_resolutionY*3;
        for (int i=0;i<n;i++)
            _rgbBuffer[i]=(unsigned char)(img[i]*255.1f);
    }
    bool returnValue=_computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values

#ifdef SIM_WITH_OPENGL
    if (_contextFboAndTexture==nullptr)
        createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(false);

    if (_contextFboAndTexture!=nullptr)
        _contextFboAndTexture->textureObject->setImage(false,false,true,_rgbBuffer); // Update the texture
#endif
    return(returnValue);
}

bool CVisionSensor::setExternalCharImage(const unsigned char* img,bool imgIsGreyScale)
{
    if (imgIsGreyScale)
    {
        int n=_resolutionX*_resolutionY;
        for (int i=0;i<n;i++)
        {
            _rgbBuffer[3*i+0]=img[i];
            _rgbBuffer[3*i+1]=img[i];
            _rgbBuffer[3*i+2]=img[i];
        }
    }
    else
    {
        int n=_resolutionX*_resolutionY*3;
        for (int i=0;i<n;i++)
            _rgbBuffer[i]=img[i];
    }
    bool returnValue=_computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values

#ifdef SIM_WITH_OPENGL
    if (_contextFboAndTexture==nullptr)
        createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(false);

    if (_contextFboAndTexture!=nullptr)
        _contextFboAndTexture->textureObject->setImage(false,false,true,_rgbBuffer); // Update the texture
#endif
    return(returnValue);
}

bool CVisionSensor::handleSensor()
{
    FUNCTION_DEBUG;
    sensorAuxiliaryResult.clear();
    sensorResult.sensorWasTriggered=false;
    sensorResult.sensorResultIsValid=false;
    sensorResult.calcTimeInMs=0;
    for (int i=0;i<3;i++)
    {
        sensorResult.sensorDataRed[i]=0;
        sensorResult.sensorDataGreen[i]=0;
        sensorResult.sensorDataBlue[i]=0;
        sensorResult.sensorDataIntensity[i]=0;
        sensorResult.sensorDataDepth[i]=0.0f;
    }
    if (!App::ct->mainSettings->visionSensorsEnabled)
        return(false);
    if (_useExternalImage) // those 2 lines added on 2010/12/12
        return(false);
    int stTime=VDateTime::getTimeInMs();
    detectEntity(_detectableEntityID,_detectableEntityID==-1,false,false,false);
#ifdef SIM_WITH_OPENGL
    if (_contextFboAndTexture!=nullptr)
        _contextFboAndTexture->textureObject->setImage(false,false,true,_rgbBuffer); // Update the texture
#endif
    sensorResult.calcTimeInMs=VDateTime::getTimeDiffInMs(stTime);
    return(sensorResult.sensorWasTriggered);
}

bool CVisionSensor::checkSensor(int entityID,bool overrideRenderableFlagsForNonCollections)
{ // This function should only be used by simCheckVisionSensor(Ex) functions! It will temporarily buffer current result
    if (_useExternalImage) // added those 2 lines on 2010/12/21
        return(false);

    // 1. We save current state:
    SHandlingResult cop;
    std::vector<std::vector<float> > sensorAuxiliaryResultCop(sensorAuxiliaryResult);
    sensorAuxiliaryResult.clear();
    cop.sensorWasTriggered=sensorResult.sensorWasTriggered;
    cop.sensorResultIsValid=sensorResult.sensorResultIsValid;
    cop.calcTimeInMs=sensorResult.calcTimeInMs;
    for (int i=0;i<3;i++)
    {
        cop.sensorDataRed[i]=sensorResult.sensorDataRed[i];
        cop.sensorDataGreen[i]=sensorResult.sensorDataGreen[i];
        cop.sensorDataBlue[i]=sensorResult.sensorDataBlue[i];
        cop.sensorDataIntensity[i]=sensorResult.sensorDataIntensity[i];
        cop.sensorDataDepth[i]=sensorResult.sensorDataDepth[i];
    }
    unsigned char* copIm=new unsigned char[_resolutionX*_resolutionY*3];
    float* copDep=new float[_resolutionX*_resolutionY];
    for (int i=0;i<_resolutionX*_resolutionY;i++)
    {
        copIm[3*i+0]=_rgbBuffer[3*i+0];
        copIm[3*i+1]=_rgbBuffer[3*i+1];
        copIm[3*i+2]=_rgbBuffer[3*i+2];
        copDep[i]=_depthBuffer[i];
    }
    // 2. Do the detection:
    bool all=(entityID==-1);
    bool retVal=detectEntity(entityID,all,false,false,overrideRenderableFlagsForNonCollections); // We don't swap image buffers!
    // 3. Restore previous state:
    sensorResult.sensorWasTriggered=cop.sensorWasTriggered;
    sensorResult.sensorResultIsValid=cop.sensorResultIsValid;
    sensorResult.calcTimeInMs=cop.calcTimeInMs;
    sensorAuxiliaryResult.assign(sensorAuxiliaryResultCop.begin(),sensorAuxiliaryResultCop.end());
    for (int i=0;i<3;i++)
    {
        sensorResult.sensorDataRed[i]=cop.sensorDataRed[i];
        sensorResult.sensorDataGreen[i]=cop.sensorDataGreen[i];
        sensorResult.sensorDataBlue[i]=cop.sensorDataBlue[i];
        sensorResult.sensorDataIntensity[i]=cop.sensorDataIntensity[i];
        sensorResult.sensorDataDepth[i]=cop.sensorDataDepth[i];
    }
    for (int i=0;i<_resolutionX*_resolutionY;i++)
    {
        _rgbBuffer[3*i+0]=copIm[3*i+0];
        _rgbBuffer[3*i+1]=copIm[3*i+1];
        _rgbBuffer[3*i+2]=copIm[3*i+2];
        _depthBuffer[i]=copDep[i];
    }
    // 4. Release memory
    delete[] copIm;
    delete[] copDep;

    return(retVal);
}

float* CVisionSensor::checkSensorEx(int entityID,bool imageBuffer,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections)
{ // This function should only be used by simCheckVisionSensor(Ex) functions! It will temporarily buffer current result
    if (_useExternalImage) // added those 2 lines on 2010/12/21
        return(nullptr);

    // 1. We save current state:
    SHandlingResult cop;
    std::vector<std::vector<float> > sensorAuxiliaryResultCop(sensorAuxiliaryResult);
    sensorAuxiliaryResult.clear();
    cop.sensorWasTriggered=sensorResult.sensorWasTriggered;
    cop.sensorResultIsValid=sensorResult.sensorResultIsValid;
    cop.calcTimeInMs=sensorResult.calcTimeInMs;
    for (int i=0;i<3;i++)
    {
        cop.sensorDataRed[i]=sensorResult.sensorDataRed[i];
        cop.sensorDataGreen[i]=sensorResult.sensorDataGreen[i];
        cop.sensorDataBlue[i]=sensorResult.sensorDataBlue[i];
        cop.sensorDataIntensity[i]=sensorResult.sensorDataIntensity[i];
        cop.sensorDataDepth[i]=sensorResult.sensorDataDepth[i];
    }
    unsigned char* copIm=new unsigned char[_resolutionX*_resolutionY*3];
    float* copDep=new float[_resolutionX*_resolutionY];
    for (int i=0;i<_resolutionX*_resolutionY;i++)
    {
        copIm[3*i+0]=_rgbBuffer[3*i+0];
        copIm[3*i+1]=_rgbBuffer[3*i+1];
        copIm[3*i+2]=_rgbBuffer[3*i+2];
        copDep[i]=_depthBuffer[i];
    }
    // 2. Do the detection:
    bool all=(entityID==-1);
    detectEntity(entityID,all,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections); // we don't swap image buffers!
    // 3. Prepare return buffer:
    float* retBuffer=nullptr;
    int l=_resolutionX*_resolutionY;
    if (imageBuffer)
        l*=3;
    retBuffer=new float[l];
    if (imageBuffer)
    {
        for (int i=0;i<l;i++)
            retBuffer[i]=float(_rgbBuffer[i])/255.0f;
    }
    else
    {
        for (int i=0;i<l;i++)
            retBuffer[i]=_depthBuffer[i];
    }
    // 4. Restore previous state:
    sensorAuxiliaryResult.assign(sensorAuxiliaryResultCop.begin(),sensorAuxiliaryResultCop.end());
    sensorResult.sensorWasTriggered=cop.sensorWasTriggered;
    sensorResult.sensorResultIsValid=cop.sensorResultIsValid;
    sensorResult.calcTimeInMs=cop.calcTimeInMs;
    for (int i=0;i<3;i++)
    {
        sensorResult.sensorDataRed[i]=cop.sensorDataRed[i];
        sensorResult.sensorDataGreen[i]=cop.sensorDataGreen[i];
        sensorResult.sensorDataBlue[i]=cop.sensorDataBlue[i];
        sensorResult.sensorDataIntensity[i]=cop.sensorDataIntensity[i];
        sensorResult.sensorDataDepth[i]=cop.sensorDataDepth[i];
    }
    for (int i=0;i<_resolutionX*_resolutionY;i++)
    {
        _rgbBuffer[3*i+0]=copIm[3*i+0];
        _rgbBuffer[3*i+1]=copIm[3*i+1];
        _rgbBuffer[3*i+2]=copIm[3*i+2];
        _depthBuffer[i]=copDep[i];
    }
    // 5. Release memory
    delete[] copIm;
    delete[] copDep;

    return(retBuffer);
}

bool CVisionSensor::detectEntity(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections)
{ // if entityID is -1, all detectable objects are rendered!
    FUNCTION_DEBUG;
    bool retVal=false;
    App::ct->calcInfo->visionSensorSimulationStart();

    // Following strange construction needed so that we can
    // do all the initialization/rendering in the UI thread:
    // - if not using an offscreen type (otherwise big problems and crashes)
    // - if this is not called from the main simulation thread or the UI thread (otherwise, hangs eventually (linked to QOpenGLContext that requires event handling somehow))

    // On Linux, depending on the openGl version, drivers and GPU, there can be many crashes if we do not handle
    // vision sensors in the UI thread. So, we handle everything in the UI thread by default:
    bool onlyGuiThread=true;
#ifdef SIM_WITH_GUI
    if (App::mainWindow==nullptr)
    { // headless
        if (App::userSettings->visionSensorsUseGuiThread_headless==0)
            onlyGuiThread=false;
    }
    else
    { // windowed
        if (App::userSettings->visionSensorsUseGuiThread_windowed==0)
            onlyGuiThread=false;
    }
#else
    if (App::userSettings->visionSensorsUseGuiThread_headless==0)
        onlyGuiThread=false;
#endif

    bool ui=VThread::isCurrentThreadTheUiThread();
    bool noAuxThread=VThread::isCurrentThreadTheUiThread()||VThread::isCurrentThreadTheMainSimulationThread();
    bool offscreen=(App::userSettings->offscreenContextType<1);

    if ( ui || ((noAuxThread&&offscreen)&&(!onlyGuiThread)) )
        detectEntity2(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections);
    else
        detectVisionSensorEntity_executedViaUiThread(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections);

    retVal=_computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values
    sensorResult.sensorWasTriggered=retVal;

    App::ct->calcInfo->visionSensorSimulationEnd(retVal);
    return(retVal);
}

void CVisionSensor::detectEntity2(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections)
{ // if entityID is -1, all detectable objects are rendered!
    FUNCTION_DEBUG;

    std::vector<int> activeMirrors;

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        int activeMirrorCnt=_getActiveMirrors(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,overrideRenderableFlagsForNonCollections,_attributesForRendering,activeMirrors);
        createGlContextAndFboAndTextureObjectIfNeeded(activeMirrorCnt>0);

        if (!_contextFboAndTexture->offscreenContext->makeCurrent())
        { // we could not make current in the current thread: we erase the context and create a new one:
            _removeGlContextAndFboAndTextureObjectIfNeeded();
            createGlContextAndFboAndTextureObjectIfNeeded(activeMirrorCnt>0);
            _contextFboAndTexture->offscreenContext->makeCurrent();
        }
        _contextFboAndTexture->frameBufferObject->switchToFbo();
    }
#endif

    if (!_useExternalImage)
        renderForDetection(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections,activeMirrors);

    if (getInternalRendering())
    {
#ifdef SIM_WITH_OPENGL
        if (!_useExternalImage)
        {
            if (!_ignoreRGBInfo)
            {
                glPixelStorei(GL_PACK_ALIGNMENT,1);
                glReadPixels(0,0,_resolutionX,_resolutionY,GL_RGB,GL_UNSIGNED_BYTE,_rgbBuffer);
                glPixelStorei(GL_PACK_ALIGNMENT,4); // important to restore! Really?
            }
            if (!_ignoreDepthInfo)
            {
                glReadPixels(0,0,_resolutionX,_resolutionY,GL_DEPTH_COMPONENT,GL_FLOAT,_depthBuffer);
                // Convert this depth info into values corresponding to linear depths (if perspective mode):
                if (_perspectiveOperation)
                {
                    float farMinusNear= _farClippingPlane-_nearClippingPlane;
                    float farDivFarMinusNear=_farClippingPlane/farMinusNear;
                    float nearTimesFar=_nearClippingPlane*_farClippingPlane;
                    int v=_resolutionX*_resolutionY;
                    for (int i=0;i<v;i++)
                        _depthBuffer[i]=((nearTimesFar/(farMinusNear*(farDivFarMinusNear-_depthBuffer[i])))-_nearClippingPlane)/farMinusNear;
                }
            }
        }

        if (App::userSettings->useGlFinish_visionSensors) // false by default!
            glFinish(); // Might be important later (synchronization problems)
                // removed on 2009/12/09 upon recomendation of gamedev community
                // re-put on 2010/01/11 because it slows down some graphic cards in a non-proportional way (e.g. 1 object=x ms, 5 objects=20x ms)
                // re-removed again (by default) on 31/01/2013. Thanks a lot to Cedric Pradalier for pointing problems appearing with the NVidia drivers

        _contextFboAndTexture->frameBufferObject->switchToNonFbo();
        _contextFboAndTexture->offscreenContext->doneCurrent(); // Important to free it if we want to use this context from a different thread!
#endif
    }
    else
        _extRenderer_retrieveImage();
}

bool CVisionSensor::_extRenderer_prepareView(int extRendererIndex)
{   // Set-up the resolution, clear color, camera properties and camera pose:
    bool retVal=CPluginContainer::selectExtRenderer(extRendererIndex);

    void* data[30];
    if ((_renderMode!=sim_rendermode_extrendererwindowed)&&(_renderMode!=sim_rendermode_opengl3windowed))
    { // When the view is not windowed:
        _extWindowedViewSize[0]=_resolutionX;
        _extWindowedViewSize[1]=_resolutionY;
    }

    data[0]=_extWindowedViewSize; // for windowed views, this value is also a return value
    data[1]=_extWindowedViewSize+1; // for windowed views, this value is also a return value
    if (_useSameBackgroundAsEnvironment)
        data[2]=App::ct->environment->fogBackgroundColor;
    else
        data[2]=_defaultBufferValues;
    C7Vector tr(getCumulativeTransformation());
    data[3]=tr.X.data;
    data[4]=tr.Q.data;
    int options=0;
    float xAngle_size;
    float yAngle_size;
    float ratio=(float)(_resolutionX/(float)_resolutionY);
    if (_perspectiveOperation)
    {
        if (ratio>1.0f)
        {
            xAngle_size=_viewAngle;
            yAngle_size=2.0f*(float)atan(tan(_viewAngle/2.0f)/ratio);
        }
        else
        {
            xAngle_size=2.0f*(float)atan(tan(_viewAngle/2.0f)*ratio);
            yAngle_size=_viewAngle;
        }
    }
    else
    {
        options|=1;
        if (ratio>1.0f)
        {
            xAngle_size=_orthoViewSize;
            yAngle_size=_orthoViewSize/ratio;
        }
        else
        {
            xAngle_size=_orthoViewSize*ratio;
            yAngle_size=_orthoViewSize;
        }
    }
    data[5]=&options;
    data[6]=&xAngle_size;
    data[7]=&yAngle_size;
    data[8]=&_viewAngle;
    data[9]=&_nearClippingPlane;
    data[10]=&_farClippingPlane;
    data[11]=App::ct->environment->ambientLightColor;
    data[12]=App::ct->environment->fogBackgroundColor;
    int fogType=App::ct->environment->getFogType();
    float fogStart=App::ct->environment->getFogStart();
    float fogEnd=App::ct->environment->getFogEnd();
    float fogDensity=App::ct->environment->getFogDensity();
    bool fogEnabled=App::ct->environment->getFogEnabled();
    data[13]=&fogType;
    data[14]=&fogStart;
    data[15]=&fogEnd;
    data[16]=&fogDensity;
    data[17]=&fogEnabled;
    data[18]=&_orthoViewSize;
    data[19]=&_objectHandle;
    data[20]=_extWindowedViewPos;
    data[21]=_extWindowedViewPos+1;

    // Following actually free since V-REP 3.3.0
    // But the older PovRay plugin version crash without this:
    float povFogDist=4.0f;
    float povFogTransp=0.5f;
    bool povFocalBlurEnabled=false;
    float povFocalLength,povAperture;
    int povBlurSamples;
    data[22]=&povFogDist;
    data[23]=&povFogTransp;
    data[24]=&povFocalBlurEnabled;
    data[25]=&povFocalLength;
    data[26]=&povAperture;
    data[27]=&povBlurSamples;

    CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_start,data);
    return(retVal);
}

void CVisionSensor::_extRenderer_prepareLights()
{   // Set-up the lights:
    for (unsigned int li=0;li<App::ct->objCont->lightList.size();li++)
    {
        CLight* light=App::ct->objCont->getLight(App::ct->objCont->lightList[li]);
        if (light->getLightActive())
        {
            void* data[20];
            int lightType=light->getLightType();
            data[0]=&lightType;
            float cutoffAngle=light->getSpotCutoffAngle();
            data[1]=&cutoffAngle;
            int spotExponent=light->getSpotExponent();
            data[2]=&spotExponent;
            data[3]=light->getColor(true)->colors;
            float constAttenuation=light->getAttenuationFactor(CONSTANT_ATTENUATION);
            data[4]=&constAttenuation;
            float linAttenuation=light->getAttenuationFactor(LINEAR_ATTENUATION);
            data[5]=&linAttenuation;
            float quadAttenuation=light->getAttenuationFactor(QUADRATIC_ATTENUATION);
            data[6]=&quadAttenuation;
            C7Vector tr(light->getCumulativeTransformation());
            data[7]=tr.X.data;
            data[8]=tr.Q.data;
            float lightSize=light->getLightSize();
            data[9]=&lightSize;
            bool lightIsVisible=light->getShouldObjectBeDisplayed(_objectHandle,0);
            data[11]=&lightIsVisible;
            int lightHandle=light->getObjectHandle();
            data[13]=&lightHandle;

            // Following actually free since V-REP 3.3.0
            // But the older PovRay plugin version crash without this:
            float povFadeXDist=0.0;
            bool povNoShadow=false;
            data[10]=&povFadeXDist;
            data[12]=&povNoShadow;

            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_light,data);
        }
    }
}

void CVisionSensor::_extRenderer_prepareMirrors()
{
    for (unsigned int li=0;li<App::ct->objCont->mirrorList.size();li++)
    {
        CMirror* mirror=App::ct->objCont->getMirror(App::ct->objCont->mirrorList[li]);
        bool visible=mirror->getShouldObjectBeDisplayed(_objectHandle,_attributesForRendering);
        if (mirror->getIsMirror()&&visible)
        {
            bool active=mirror->getActive()&&(!App::ct->mainSettings->mirrorsDisabled);
            C7Vector tr=mirror->getCumulativeTransformationPart1_forDisplay(false);
            float w_=mirror->getMirrorWidth()/2.0f;
            float h_=mirror->getMirrorHeight()/2.0f;
            float vertices[18]={w_,-h_,0.0005f,w_,h_,0.0005f,-w_,-h_,0.0005f,-w_,-h_,0.0005f,w_,h_,0.0005f,-w_,h_,0.0005f};
            int verticesCnt=6;
            float normals[18]={0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f};
            for (int i=0;i<6;i++)
            {
                C3Vector v(vertices+i*3);
                v*=tr;
                C3Vector n(normals+i*3);
                n=tr.Q*n;
                vertices[i*3+0]=v(0);
                vertices[i*3+1]=v(1);
                vertices[i*3+2]=v(2);
                normals[i*3+0]=n(0);
                normals[i*3+1]=n(1);
                normals[i*3+2]=n(2);
            }
            void* data[20];
            data[0]=vertices;
            data[1]=&verticesCnt;
            data[2]=normals;
            float colors[15];
            colors[0]=mirror->mirrorColor[0];
            colors[1]=mirror->mirrorColor[1];
            colors[2]=mirror->mirrorColor[2];
            colors[6]=0.0f;
            colors[7]=0.0f;
            colors[8]=0.0f;
            colors[9]=0.0f;
            colors[10]=0.0f;
            colors[11]=0.0f;
            colors[12]=0.0f;
            colors[13]=0.0f;
            colors[14]=0.0f;
            data[3]=colors;
            bool translucid=false;
            data[4]=&translucid;
            float opacityFactor=1.0f;
            data[5]=&opacityFactor;
            const char* povMaterial={"mirror"};
            data[6]=(char*)povMaterial;
            data[7]=&active;
            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_triangles,data);
            C3Vector shift=tr.Q.getMatrix().axis[2]*(-0.001f);
            for (int i=0;i<6;i++)
            {
                C3Vector v(vertices+i*3);
                v+=shift;
                vertices[i*3+0]=v(0);
                vertices[i*3+1]=v(1);
                vertices[i*3+2]=v(2);
            }
            active=false;
            CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_triangles,data);
        }
    }
}

void CVisionSensor::_extRenderer_retrieveImage()
{   // Fetch the finished image:
    void* data[20];
    data[0]=_rgbBuffer;
    data[1]=_depthBuffer;
    bool readRgb=!_ignoreRGBInfo;
    data[2]=&readRgb;
    bool readDepth=!_ignoreDepthInfo;
    data[3]=&readDepth;
    CPluginContainer::extRenderer(sim_message_eventcallback_extrenderer_stop,data);
}

void CVisionSensor::renderForDetection(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections,const std::vector<int>& activeMirrors)
{ // if entityID==-1, all objects that can be detected are rendered. 
    FUNCTION_DEBUG;

    _currentPerspective=_perspectiveOperation;

    if (getInternalRendering())
    {
#ifdef SIM_WITH_OPENGL
        int currentWinSize[2]={_resolutionX,_resolutionY};
        glViewport(0,0,_resolutionX,_resolutionY);

        if (_renderMode!=sim_rendermode_colorcoded)
        {
            if (_useSameBackgroundAsEnvironment)
                glClearColor(App::ct->environment->fogBackgroundColor[0],App::ct->environment->fogBackgroundColor[1],App::ct->environment->fogBackgroundColor[2],0.0f);
            else
                glClearColor(_defaultBufferValues[0],_defaultBufferValues[1],_defaultBufferValues[2],0.0f);
        }
        else
            glClearColor(1.0f,1.0f,1.0f,0.0f); // for color coding we need a clear color perfectly white

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glRenderMode(GL_RENDER);

        float ratio=(float)(currentWinSize[0]/(float)currentWinSize[1]);
        if (_perspectiveOperation)
        {
            if (ratio>1.0f)
            {
                float a=2.0f*(float)atan(tan(_viewAngle/2.0f)/ratio)*radToDeg_f;
                ogl::perspectiveSpecial(a,ratio,_nearClippingPlane,_farClippingPlane);
            }
            else
                ogl::perspectiveSpecial(_viewAngle*radToDeg_f,ratio,_nearClippingPlane,_farClippingPlane);
        }
        else
        {
            if (ratio>1.0f)
                glOrtho(-_orthoViewSize*0.5f,_orthoViewSize*0.5f,-_orthoViewSize*0.5f/ratio,_orthoViewSize*0.5f/ratio,_nearClippingPlane,_farClippingPlane);
            else
                glOrtho(-_orthoViewSize*0.5f*ratio,_orthoViewSize*0.5f*ratio,-_orthoViewSize*0.5f,_orthoViewSize*0.5f,_nearClippingPlane,_farClippingPlane);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        C4X4Matrix m4(getCumulativeTransformation().getMatrix());
        // The following 6 instructions have the same effect as gluLookAt()
        m4.inverse();
        m4.rotateAroundY(piValue_f);
        float m4_[4][4];
        m4.copyTo(m4_);
        CMeshManip::transposeMatrix_4x4Array(m4_);
        glLoadMatrixf((float*)m4_);

        if (_renderMode==sim_rendermode_opengl)
        { // visible
            App::ct->environment->activateAmbientLight(true);
            App::ct->environment->activateFogIfEnabled(this,false);
            _activateNonAmbientLights(-1,this);
        }
        else
        {
            App::ct->environment->activateAmbientLight(false);
            App::ct->environment->deactivateFog();
            _activateNonAmbientLights(-2,nullptr);
        }

        glInitNames();
        glPushName(-1);
        glLoadName(-1);

        glShadeModel(GL_SMOOTH);

        if (_renderMode!=sim_rendermode_colorcoded)
        { // visible & aux channels
            glEnable(GL_DITHER);
        }
        else
        {
            glDisable(GL_DITHER);
            ogl::disableLighting_useWithCare(); // only temporarily
        }

        _handleMirrors(activeMirrors,entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections);
#endif
    }
    else
    {
#ifdef SIM_WITH_GUI
        if (!_extRenderer_prepareView(_renderMode-sim_rendermode_povray))
        {
            if (_renderMode==sim_rendermode_povray)
            {
                static bool alreadyShown=false;
                if (!alreadyShown)
                    App::uiThread->messageBox_information(App::mainWindow,"POV-Ray plugin",strTranslate("The POV-Ray plugin was not found, or could not be loaded. You can find the required binary and source code at https://github.com/CoppeliaRobotics/v_repExtPovRay"),VMESSAGEBOX_OKELI);
                alreadyShown=true;
            }
        }
#endif
        _extRenderer_prepareLights();
        _extRenderer_prepareMirrors();
    }

    // Set data for view frustum culling
    _planesCalculated=false;
    _currentViewSize[0]=_resolutionX;
    _currentViewSize[1]=_resolutionY;
    if ((_renderMode==sim_rendermode_extrendererwindowed)||(_renderMode==sim_rendermode_opengl3windowed))
    { // We have a windowed view (the window's size is different from the vision sensor's resolution)
        _currentViewSize[0]=_extWindowedViewSize[0];
        _currentViewSize[1]=_extWindowedViewSize[1];
    }

    if ((_renderMode==sim_rendermode_povray)||(_renderMode==sim_rendermode_opengl3)||(_renderMode==sim_rendermode_opengl3windowed))
        setFrustumCullingTemporarilyDisabled(true); // important with ray-tracers and similar

    // Draw objects:
    _drawObjects(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections);

    if ((_renderMode==sim_rendermode_povray)||(_renderMode==sim_rendermode_opengl3)||(_renderMode==sim_rendermode_opengl3windowed))
        setFrustumCullingTemporarilyDisabled(false); // important with ray-tracers and similar

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        if (_renderMode==sim_rendermode_colorcoded)
        { // reset to default
            ogl::enableLighting_useWithCare();
            glEnable(GL_DITHER);
        }

        App::ct->environment->deactivateFog();
        glRenderMode(GL_RENDER);
    }
#endif
}

void CVisionSensor::_drawObjects(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections)
{ // if entityID==-1, all objects that can be detected are rendered. 
    FUNCTION_DEBUG;
    // Very unelegant routine. Needs badly refactoring!

    int rendAttrib=_attributesForRendering;
    if (_renderMode==sim_rendermode_colorcoded)
        rendAttrib|=sim_displayattribute_colorcoded;
    if (_renderMode==sim_rendermode_auxchannels)
        rendAttrib|=sim_displayattribute_useauxcomponent;

    std::vector<C3DObject*> toRender;
    C3DObject* viewBoxObject=_getInfoOfWhatNeedsToBeRendered(entityID,detectAll,rendAttrib,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,overrideRenderableFlagsForNonCollections,toRender);

    //************ Viewbox thing ***************
    if (viewBoxObject!=nullptr)
    { // we set the same position as the camera, but we keep the initial orientation
        // If the camera is in ortho view mode, we additionally shift it along the viewing axis
        // to be sure we don't cover anything visible with the far side of the box (the near side is clipped by model settings)
        C4Vector rel(viewBoxObject->getLocalTransformationPart1().Q);
        C7Vector cam(getCumulativeTransformation());
        if (!_currentPerspective)
        { // This doesn't work!!
            C3Vector minV,maxV;
            bool first=true;
            viewBoxObject->getGlobalMarkingBoundingBox(getCumulativeTransformation().getInverse(),minV,maxV,first,true,false);
            float shift=getFarClippingPlane()-0.505f*(maxV(2)-minV(2)); // just a bit more than half!
            cam.X+=cam.Q.getMatrix().axis[2]*shift;
        }
        C7Vector newLocal(viewBoxObject->getParentCumulativeTransformation().getInverse()*cam);
        newLocal.Q=rel;
        viewBoxObject->setLocalTransformation(newLocal);
    }
    //***************************************

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        _prepareAuxClippingPlanes();
        _enableAuxClippingPlanes(-1);

        if ((rendAttrib&sim_displayattribute_noopenglcallbacks)==0)
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,0,rendAttrib,_objectHandle,-1);
    }
#endif

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering()) // for now
    {
        // first non-transparent attached drawing objects:
        for (int i=0;i<int(toRender.size());i++)
            App::ct->drawingCont->drawObjectsParentedWith(false,false,toRender[i]->getObjectHandle(),rendAttrib,getCumulativeTransformation().getMatrix());

        // Now the same as above but for non-attached drawing objects:
        App::ct->drawingCont->drawObjectsParentedWith(false,false,-1,rendAttrib,getCumulativeTransformation().getMatrix());

        // Point clouds:
        App::ct->pointCloudCont->renderYour3DStuff_nonTransparent(this,rendAttrib);

        // Ghost objects:
        App::ct->ghostObjectCont->renderYour3DStuff_nonTransparent(this,rendAttrib);

        // particles:
        App::ct->dynamicsContainer->renderYour3DStuff(this,rendAttrib);
    }
#endif

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        if ((rendAttrib&sim_displayattribute_noopenglcallbacks)==0)
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,1,rendAttrib,_objectHandle,-1);

        _disableAuxClippingPlanes();
    }
#endif

    // Rendering the scene objects:
    for (int i=0;i<int(toRender.size());i++)
    {
        if (!entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects)
        { // attached non-transparent objects were rendered before
            if (getInternalRendering())
            {
#ifdef SIM_WITH_OPENGL
                toRender[i]->display(this,rendAttrib);
#endif
            }
            else
            {
                if (toRender[i]->getObjectType()==sim_object_shape_type)
                    ((CShape*)toRender[i])->display_extRenderer(this,rendAttrib);
            }
        }
#ifdef SIM_WITH_OPENGL
        else
        { // we render the scene twice when we have edges, since antialiasing might not be beautiful otherwise
            int atr=sim_displayattribute_renderpass;
            toRender[i]->display(this,atr|sim_displayattribute_forbidedges); // attached non-transparent objects were rendered before
            if (!hideEdgesIfModel)
                toRender[i]->display(this,atr); // attached object were rendered before
        }
#endif
    }

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        _enableAuxClippingPlanes(-1);

        if ((rendAttrib&sim_displayattribute_noopenglcallbacks)==0)
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,2,rendAttrib,_objectHandle,-1);
    }
#endif

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering()) // for now
    {
        // Transparent attached drawing objects:
        for (int i=0;i<int(toRender.size());i++)
            App::ct->drawingCont->drawObjectsParentedWith(false,true,toRender[i]->getObjectHandle(),rendAttrib,getCumulativeTransformation().getMatrix());

        // Now the same as above but for non-attached drawing objects:
        App::ct->drawingCont->drawObjectsParentedWith(false,true,-1,rendAttrib,getCumulativeTransformation().getMatrix());

        // Ghost objects:
        App::ct->ghostObjectCont->renderYour3DStuff_transparent(this,rendAttrib);
    }
#endif

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        if ((rendAttrib&sim_displayattribute_noopenglcallbacks)==0)
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,3,rendAttrib,_objectHandle,-1);
    }
#endif

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering()) // for now
    {
        // overlay attached drawing objects:
        for (int i=0;i<int(toRender.size());i++)
            App::ct->drawingCont->drawObjectsParentedWith(true,true,toRender[i]->getObjectHandle(),rendAttrib,getCumulativeTransformation().getMatrix());

        // Now the same as above but for non-attached drawing objects:
        App::ct->drawingCont->drawObjectsParentedWith(true,true,-1,rendAttrib,getCumulativeTransformation().getMatrix());

        // Ghosts:
        App::ct->ghostObjectCont->renderYour3DStuff_overlay(this,rendAttrib);
    }
#endif

#ifdef SIM_WITH_OPENGL
    if (getInternalRendering())
    {
        if ((rendAttrib&sim_displayattribute_noopenglcallbacks)==0)
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,4,rendAttrib,_objectHandle,-1);

        _disableAuxClippingPlanes();

        if ((rendAttrib&sim_displayattribute_noopenglcallbacks)==0)
            CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_opengl,5,rendAttrib,_objectHandle,-1);
    }
#endif
}

C3DObject* CVisionSensor::_getInfoOfWhatNeedsToBeRendered(int entityID,bool detectAll,int rendAttrib,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,std::vector<C3DObject*>& toRender)
{
    C3DObject* object=App::ct->objCont->getObjectFromHandle(entityID);
    CRegCollection* collection=nullptr;
    std::vector<int> transparentObjects;
    std::vector<float> transparentObjectsDist;
    C7Vector camTrInv(getCumulativeTransformationPart1().getInverse());
    C3DObject* viewBoxObject=nullptr;

    if (object==nullptr)
    {
        collection=App::ct->collections->getCollection(entityID);
        if (collection!=nullptr)
        {
            bool overridePropertyFlag=collection->getOverridesObjectMainProperties();
            for (int i=0;i<int(collection->collectionObjects.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(collection->collectionObjects[i]);
                if (it!=nullptr)
                {
                    if  ( ((it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0)||overridePropertyFlag||(rendAttrib&sim_displayattribute_ignorerenderableflag) )
                    { // supposed to be rendered
                        if (it->getObjectType()==sim_object_shape_type)
                        {
                            CShape* sh=(CShape*)it;
                            if (sh->getContainsTransparentComponent())
                            {
                                C7Vector obj(it->getCumulativeTransformationPart1());
                                transparentObjectsDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                                transparentObjects.push_back(it->getObjectHandle());
                            }
                            else
                                toRender.push_back(it);
                        }
                        else
                        {
                            if (it->getObjectType()==sim_object_mirror_type)
                            {
                                CMirror* mir=(CMirror*)it;
                                if (mir->getContainsTransparentComponent())
                                {
                                    C7Vector obj(it->getCumulativeTransformationPart1());
                                    transparentObjectsDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                                    transparentObjects.push_back(it->getObjectHandle());
                                }
                                else
                                    toRender.push_back(it);
                            }
                            else
                                toRender.push_back(it);
                        }
                        if (it->getParentObject()!=nullptr)
                        { // We need this because the dummy that is the base of the skybox is not renderable!
                            if (it->getParentObject()->getObjectName()==IDSOGL_SKYBOX_DO_NOT_RENAME)
                                viewBoxObject=it->getParentObject();
                        }
                    }
                }
            }
        }
        else
        { // Here we want to detect all detectable objects maybe:
            if (detectAll)
            {
                for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                    if (it!=nullptr)
                    {
                        if  ( ( (it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0 )||overrideRenderableFlagsForNonCollections||(rendAttrib&sim_displayattribute_ignorerenderableflag) )
                        { // supposed to be rendered
                            if (it->getObjectType()==sim_object_shape_type)
                            {
                                CShape* sh=(CShape*)it;
                                if (sh->getContainsTransparentComponent())
                                {
                                    C7Vector obj(it->getCumulativeTransformationPart1());
                                    transparentObjectsDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                                    transparentObjects.push_back(it->getObjectHandle());
                                }
                                else
                                    toRender.push_back(it);
                            }
                            else
                            {
                                if (it->getObjectType()==sim_object_mirror_type)
                                {
                                    CMirror* mir=(CMirror*)it;
                                    if (mir->getContainsTransparentComponent())
                                    {
                                        C7Vector obj(it->getCumulativeTransformationPart1());
                                        transparentObjectsDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                                        transparentObjects.push_back(it->getObjectHandle());
                                    }
                                    else
                                        toRender.push_back(it);
                                }
                                else
                                    toRender.push_back(it);
                            }
                            if (it->getParentObject()!=nullptr)
                            { // We need this because the dummy that is the base of the skybox is not renderable!
                                if (it->getParentObject()->getObjectName()==IDSOGL_SKYBOX_DO_NOT_RENAME)
                                    viewBoxObject=it->getParentObject();
                            }
                        }
                    }
                }
            }
        }
    }
    else
    { // We want to detect a single object (no collection not all objects in the scene)
        if (!entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects)
        {
            if  ( ( (object->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0 )||overrideRenderableFlagsForNonCollections||(rendAttrib&sim_displayattribute_ignorerenderableflag) )
            {
                toRender.push_back(object);
                if (object->getParentObject()!=nullptr)
                { // We need this because the dummy that is the base of the skybox is not renderable!
                    if (object->getParentObject()->getObjectName()==IDSOGL_SKYBOX_DO_NOT_RENAME)
                        viewBoxObject=object->getParentObject();
                }
            }
        }
        else
        { // we have a model here that we want to render. We render also non-renderable object. And only those currently visible:
            std::vector<int> rootSel;
            rootSel.push_back(object->getObjectHandle());
            CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);
            for (int i=0;i<int(rootSel.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(rootSel[i]);
                if (App::ct->mainSettings->getActiveLayers()&it->layer)
                { // ok, currently visible
                    if (it->getObjectType()==sim_object_shape_type)
                    {
                        CShape* sh=(CShape*)it;
                        if (sh->getContainsTransparentComponent())
                        {
                            C7Vector obj(it->getCumulativeTransformationPart1());
                            transparentObjectsDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                            transparentObjects.push_back(it->getObjectHandle());
                        }
                        else
                            toRender.push_back(it);
                    }
                    else
                    {
                        if (it->getObjectType()==sim_object_mirror_type)
                        {
                            CMirror* mir=(CMirror*)it;
                            if (mir->getContainsTransparentComponent())
                            {
                                C7Vector obj(it->getCumulativeTransformationPart1());
                                transparentObjectsDist.push_back(-(camTrInv*obj).X(2)-it->getTransparentObjectDistanceOffset());
                                transparentObjects.push_back(it->getObjectHandle());
                            }
                            else
                                toRender.push_back(it);
                        }
                        else
                            toRender.push_back(it);
                    }
                }
            }
        }
    }

    tt::orderAscending(transparentObjectsDist,transparentObjects);
    for (int i=0;i<int(transparentObjects.size());i++)
        toRender.push_back(App::ct->objCont->getObjectFromHandle(transparentObjects[i]));

    return(viewBoxObject);
}

int CVisionSensor::_getActiveMirrors(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,int rendAttrib,std::vector<int>& activeMirrors)
{
    if (App::ct->objCont->mirrorList.size()==0)
        return(0);
    if (App::ct->mainSettings->mirrorsDisabled)
        return(0);
    if (_renderMode!=sim_rendermode_opengl)
        return(0);

    C3DObject* object=App::ct->objCont->getObjectFromHandle(entityID);
    CRegCollection* collection=nullptr;
    std::vector<C3DObject*> toRender;
    if (object==nullptr)
    {
        collection=App::ct->collections->getCollection(entityID);
        if (collection!=nullptr)
        {
            bool overridePropertyFlag=collection->getOverridesObjectMainProperties();
            for (int i=0;i<int(collection->collectionObjects.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(collection->collectionObjects[i]);
                if (it!=nullptr)
                {
                    if  ( ((it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0)||overridePropertyFlag||(rendAttrib&sim_displayattribute_ignorerenderableflag) )
                    { // supposed to be rendered
                        toRender.push_back(it);
                    }
                }
            }
        }
        else
        { // Here we want to detect all detectable objects maybe:
            if (detectAll)
            {
                for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                    if (it!=nullptr)
                    {
                        if  ( ( (it->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0 )||overrideRenderableFlagsForNonCollections||(rendAttrib&sim_displayattribute_ignorerenderableflag) )
                        { // supposed to be rendered
                            toRender.push_back(it);
                        }
                    }
                }
            }
        }
    }
    else
    { // We want to detect a single object (no collection not all objects in the scene)
        if (!entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects)
        {
            if  ( ( (object->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_renderable)!=0 )||overrideRenderableFlagsForNonCollections||(rendAttrib&sim_displayattribute_ignorerenderableflag) )
            {
                toRender.push_back(object);
            }
        }
        else
        { // we have a model here that we want to render. We render also non-renderable object. And only those currently visible:
            std::vector<int> rootSel;
            rootSel.push_back(object->getObjectHandle());
            CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);
            for (int i=0;i<int(rootSel.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(rootSel[i]);
                if (App::ct->mainSettings->getActiveLayers()&it->layer)
                { // ok, currently visible
                    toRender.push_back(it);
                }
            }
        }
    }
    int retVal=0;
    for (int i=0;i<int(toRender.size());i++)
    {
        C3DObject* it=toRender[i];
        if (it->getObjectType()==sim_object_mirror_type)
        {
            CMirror* mi=(CMirror*)it;
            if (mi->getActive()&&mi->getIsMirror())
            {
                activeMirrors.push_back(it->getObjectHandle());
                retVal++;
            }
        }
    }
    return(retVal);
}

void CVisionSensor::scaleObject(float scalingFactor)
{
    _nearClippingPlane*=scalingFactor;
    _farClippingPlane*=scalingFactor;

    _orthoViewSize*=scalingFactor;
    _size=_size*scalingFactor;
    scaleObjectMain(scalingFactor);
}

void CVisionSensor::scaleObjectNonIsometrically(float x,float y,float z)
{
    _nearClippingPlane*=z;
    _farClippingPlane*=z;

    float avg=sqrt(x*y);

    _orthoViewSize*=avg;
    _size*=cbrt(x*y*z);
    scaleObjectNonIsometricallyMain(avg,avg,z);
}

void CVisionSensor::removeSceneDependencies()
{
    removeSceneDependenciesMain();
    _detectableEntityID=-1;
}

C3DObject* CVisionSensor::copyYourself()
{   
    CVisionSensor* newVisionSensor=(CVisionSensor*)copyYourselfMain();

    delete newVisionSensor->_composedFilter;
    newVisionSensor->_composedFilter=_composedFilter->copyYourself();

    newVisionSensor->_viewAngle=_viewAngle;
    newVisionSensor->_orthoViewSize=_orthoViewSize;
    newVisionSensor->_nearClippingPlane=_nearClippingPlane;
    newVisionSensor->_farClippingPlane=_farClippingPlane;
    newVisionSensor->_showFogIfAvailable=_showFogIfAvailable;
    newVisionSensor->_useLocalLights=_useLocalLights;

    newVisionSensor->_resolutionX=_resolutionX;
    newVisionSensor->_resolutionY=_resolutionY;
    newVisionSensor->_desiredResolution[0]=_desiredResolution[0];
    newVisionSensor->_desiredResolution[1]=_desiredResolution[1];
    for (int i=0;i<3;i++) // Important to do it before setting the sensor type
        newVisionSensor->_defaultBufferValues[i]=_defaultBufferValues[i];
    newVisionSensor->_perspectiveOperation=_perspectiveOperation;
    newVisionSensor->_size=_size;
    newVisionSensor->_detectableEntityID=_detectableEntityID;
    newVisionSensor->_useExternalImage=_useExternalImage;
    newVisionSensor->_useSameBackgroundAsEnvironment=_useSameBackgroundAsEnvironment;

    newVisionSensor->_explicitHandling=_explicitHandling;
    newVisionSensor->_showVolumeWhenNotDetecting=_showVolumeWhenNotDetecting;
    newVisionSensor->_showVolumeWhenDetecting=_showVolumeWhenDetecting;

    newVisionSensor->_ignoreRGBInfo=_ignoreRGBInfo;
    newVisionSensor->_ignoreDepthInfo=_ignoreDepthInfo;
    newVisionSensor->_computeImageBasicStats=_computeImageBasicStats;
    newVisionSensor->_renderMode=_renderMode;
    newVisionSensor->_attributesForRendering=_attributesForRendering;

    for (int i=0;i<3;i++)
    {
        newVisionSensor->sensorResult.sensorDataRed[i]=0;
        newVisionSensor->sensorResult.sensorDataGreen[i]=0;
        newVisionSensor->sensorResult.sensorDataBlue[i]=0;
        newVisionSensor->sensorResult.sensorDataIntensity[i]=0;
        newVisionSensor->sensorResult.sensorDataDepth[i]=0.0f;
    }
    newVisionSensor->sensorAuxiliaryResult.assign(sensorAuxiliaryResult.begin(),sensorAuxiliaryResult.end());
    newVisionSensor->sensorResult.sensorWasTriggered=false;
    newVisionSensor->sensorResult.sensorResultIsValid=false;

    color.copyYourselfInto(&newVisionSensor->color);
    activeColor.copyYourselfInto(&newVisionSensor->activeColor);
    newVisionSensor->_reserveBuffers(); // important!

    newVisionSensor->_initialValuesInitialized=_initialValuesInitialized;
    newVisionSensor->_initialExplicitHandling=_initialExplicitHandling;

    return(newVisionSensor);
}

bool CVisionSensor::announceObjectWillBeErased(int objectHandle,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    // This routine can be called for objCont-objects, but also for objects
    // in the copy-buffer!! So never make use of any 
    // 'ct::objCont->getObject(objectHandle)'-call or similar
    // Return value true means 'this' has to be erased too!
    if (_detectableEntityID==objectHandle)
        _detectableEntityID=-1;
    bool retVal=announceObjectWillBeErasedMain(objectHandle,copyBuffer);
    return(retVal);
}

void CVisionSensor::announceCollectionWillBeErased(int groupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if (_detectableEntityID==groupID)
        _detectableEntityID=-1;
    announceCollectionWillBeErasedMain(groupID,copyBuffer);
}
void CVisionSensor::announceCollisionWillBeErased(int collisionID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceCollisionWillBeErasedMain(collisionID,copyBuffer);
}
void CVisionSensor::announceDistanceWillBeErased(int distanceID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceDistanceWillBeErasedMain(distanceID,copyBuffer);
}
void CVisionSensor::announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceIkObjectWillBeErasedMain(ikGroupID,copyBuffer);
}
void CVisionSensor::announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer)
{   // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    announceGcsObjectWillBeErasedMain(gcsObjectID,copyBuffer);
}
void CVisionSensor::performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // New_Object_ID=map[Old_Object_ID]
    performObjectLoadingMappingMain(map,loadingAmodel);
    if (_detectableEntityID<SIM_IDSTART_COLLECTION)
        _detectableEntityID=App::ct->objCont->getLoadingMapping(map,_detectableEntityID);
}
void CVisionSensor::performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollectionLoadingMappingMain(map,loadingAmodel);
    if (_detectableEntityID>=SIM_IDSTART_COLLECTION)
        _detectableEntityID=App::ct->objCont->getLoadingMapping(map,_detectableEntityID);
}
void CVisionSensor::performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performCollisionLoadingMappingMain(map,loadingAmodel);
}
void CVisionSensor::performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{ // If (map[2*i]==Old_Group_ID) then New_Group_ID=map[2*i+1]
    performDistanceLoadingMappingMain(map,loadingAmodel);
}
void CVisionSensor::performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel)
{
    performIkLoadingMappingMain(map,loadingAmodel);
}
void CVisionSensor::performGcsLoadingMapping(std::vector<int>* map)
{
    performGcsLoadingMappingMain(map);
}
void CVisionSensor::performTextureObjectLoadingMapping(std::vector<int>* map)
{
    performTextureObjectLoadingMappingMain(map);
}
void CVisionSensor::performDynMaterialObjectLoadingMapping(std::vector<int>* map)
{
    performDynMaterialObjectLoadingMappingMain(map);
}
void CVisionSensor::bufferMainDisplayStateVariables()
{
    bufferMainDisplayStateVariablesMain();
}
void CVisionSensor::bufferedMainDisplayStateVariablesToDisplay()
{
    bufferedMainDisplayStateVariablesToDisplayMain();
}

void CVisionSensor::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    initializeInitialValuesMain(simulationIsRunning);
    _initialValuesInitialized=simulationIsRunning;
    for (int i=0;i<3;i++)
    {
        sensorResult.sensorDataRed[i]=0;
        sensorResult.sensorDataGreen[i]=0;
        sensorResult.sensorDataBlue[i]=0;
        sensorResult.sensorDataIntensity[i]=0;
        sensorResult.sensorDataDepth[i]=0.0f;
    }
    sensorAuxiliaryResult.clear();
    sensorResult.sensorWasTriggered=false;
    sensorResult.sensorResultIsValid=false;
    sensorResult.calcTimeInMs=0;
    if (_composedFilter!=nullptr)
        _composedFilter->initializeInitialValues(simulationIsRunning);
    if (simulationIsRunning)
    {
        _initialExplicitHandling=_explicitHandling;
    }
    else
    {
#ifdef SIM_WITH_OPENGL
        _removeGlContextAndFboAndTextureObjectIfNeeded();
#endif
        if (_composedFilter!=nullptr)
        {
            _composedFilter->drawingContainer.removeAllObjects();
            _composedFilter->simulationEnded();
        }
    }
}


void CVisionSensor::simulationAboutToStart()
{
    initializeInitialValues(true);
}

void CVisionSensor::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
#ifdef SIM_WITH_OPENGL
    _removeGlContextAndFboAndTextureObjectIfNeeded();
#endif
    if (_composedFilter!=nullptr)
    {
        _composedFilter->drawingContainer.removeAllObjects();
        _composedFilter->simulationEnded();
    }
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd()&&((getCumulativeModelProperty()&sim_modelproperty_not_reset)==0))
    {
        _explicitHandling=_initialExplicitHandling;
    }
    _initialValuesInitialized=false;
    simulationEndedMain();
}

bool CVisionSensor::_computeDefaultReturnValuesAndApplyFilters()
{
    bool trigger=false;

    sensorAuxiliaryResult.clear();
    sensorResult.sensorResultIsValid=true;
    sensorResult.sensorWasTriggered=false;

    if (_computeImageBasicStats&&(_renderMode!=sim_rendermode_colorcoded))
    {
        unsigned int cumulRed=0;
        unsigned int cumulGreen=0;
        unsigned int cumulBlue=0;
        unsigned int cumulIntensity=0;
        float cumulDepth=0.0f;

        // Initialize the min/max values with first element:
        for (int i=0;i<2;i++)
        {
            sensorResult.sensorDataRed[i]=_rgbBuffer[0];
            sensorResult.sensorDataGreen[i]=_rgbBuffer[1];
            sensorResult.sensorDataBlue[i]=_rgbBuffer[2];
            sensorResult.sensorDataIntensity[i]=(_rgbBuffer[0]+_rgbBuffer[1]+_rgbBuffer[2])/3;
            sensorResult.sensorDataDepth[i]=_depthBuffer[0];
        }

        int v=_resolutionX*_resolutionY;
        for (int i=0;i<v;i++)
        {
            unsigned int intens=(_rgbBuffer[3*i+0]+_rgbBuffer[3*i+1]+_rgbBuffer[3*i+2])/3;
            cumulRed+=_rgbBuffer[3*i+0];
            cumulGreen+=_rgbBuffer[3*i+1];
            cumulBlue+=_rgbBuffer[3*i+2];
            cumulDepth+=_depthBuffer[i];
            cumulIntensity+=intens;
            // we actualize min/max values:
            //r = (x < y) ? x : y
            // Red
            if (_rgbBuffer[3*i+0]<sensorResult.sensorDataRed[0])
                sensorResult.sensorDataRed[0]=_rgbBuffer[3*i+0];
            if (_rgbBuffer[3*i+0]>sensorResult.sensorDataRed[1])
                sensorResult.sensorDataRed[1]=_rgbBuffer[3*i+0];
            // Green
            if (_rgbBuffer[3*i+1]<sensorResult.sensorDataGreen[0])
                sensorResult.sensorDataGreen[0]=_rgbBuffer[3*i+1];
            if (_rgbBuffer[3*i+1]>sensorResult.sensorDataGreen[1])
                sensorResult.sensorDataGreen[1]=_rgbBuffer[3*i+1];
            // Blue
            if (_rgbBuffer[3*i+2]<sensorResult.sensorDataBlue[0])
                sensorResult.sensorDataBlue[0]=_rgbBuffer[3*i+2];
            if (_rgbBuffer[3*i+2]>sensorResult.sensorDataBlue[1])
                sensorResult.sensorDataBlue[1]=_rgbBuffer[3*i+2];
            // Intensity
            if (intens<sensorResult.sensorDataIntensity[0])
                sensorResult.sensorDataIntensity[0]=intens;
            if (intens>sensorResult.sensorDataIntensity[1])
                sensorResult.sensorDataIntensity[1]=intens;
            // Depth
            if (_depthBuffer[i]<sensorResult.sensorDataDepth[0])
                sensorResult.sensorDataDepth[0]=_depthBuffer[i];
            if (_depthBuffer[i]>sensorResult.sensorDataDepth[1])
                sensorResult.sensorDataDepth[1]=_depthBuffer[i];
        }
        unsigned char averageRed=cumulRed/v;
        unsigned char averageGreen=cumulGreen/v;
        unsigned char averageBlue=cumulBlue/v;
        unsigned char averageIntensity=cumulIntensity/v;
        float averageDepth=cumulDepth/float(v);
        // We set-up average values:
        sensorResult.sensorDataRed[2]=averageRed;
        sensorResult.sensorDataGreen[2]=averageGreen;
        sensorResult.sensorDataBlue[2]=averageBlue;
        sensorResult.sensorDataIntensity[2]=averageIntensity;
        sensorResult.sensorDataDepth[2]=averageDepth;

        // We prepare the auxiliary values:
        std::vector<float> defaultResults;
        defaultResults.push_back(float(sensorResult.sensorDataIntensity[0])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataRed[0])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataGreen[0])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataBlue[0])/255.0f);
        defaultResults.push_back(sensorResult.sensorDataDepth[0]);

        defaultResults.push_back(float(sensorResult.sensorDataIntensity[1])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataRed[1])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataGreen[1])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataBlue[1])/255.0f);
        defaultResults.push_back(sensorResult.sensorDataDepth[1]);

        defaultResults.push_back(float(sensorResult.sensorDataIntensity[2])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataRed[2])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataGreen[2])/255.0f);
        defaultResults.push_back(float(sensorResult.sensorDataBlue[2])/255.0f);
        defaultResults.push_back(sensorResult.sensorDataDepth[2]);
        sensorAuxiliaryResult.push_back(defaultResults);
    }
    else
    { // We do not want to produce those values
        for (int i=0;i<3;i++)
        {
            sensorResult.sensorDataRed[i]=0;
            sensorResult.sensorDataGreen[i]=0;
            sensorResult.sensorDataBlue[i]=0;
            sensorResult.sensorDataIntensity[i]=0;
            sensorResult.sensorDataDepth[i]=0.0f;
        }
        if (_renderMode!=sim_rendermode_colorcoded)
        {
            std::vector<float> defaultResults(0.0f,15);
            sensorAuxiliaryResult.push_back(defaultResults);
        }
        else
        {
            unsigned int r=(SIM_IDEND_3DOBJECT-SIM_IDSTART_3DOBJECT)+1;
            unsigned char* visibleIds=new unsigned char[r];
            for (unsigned int i=0;i<r;i++)
                visibleIds[i]=0;
            int v=_resolutionX*_resolutionY;
            for (int i=0;i<v;i++)
            {
                unsigned int id=_rgbBuffer[3*i+0]+(_rgbBuffer[3*i+1]<<8)+(_rgbBuffer[3*i+2]<<16);
                if (id<r)
                    visibleIds[id]=1;
            }
            std::vector<float> defaultResults;
            for (unsigned int i=0;i<r;i++)
            {
                if (visibleIds[i]!=0)
                    defaultResults.push_back(i+0.5f);
            }
            sensorAuxiliaryResult.push_back(defaultResults);
            delete[] visibleIds;
        }
    }

    bool applyFilter=(_composedFilter->getSimpleFilterCount()>0);
    if (applyFilter)
    {
        if ( (_composedFilter->getSimpleFilterCount()==2)&&(_composedFilter->getSimpleFilter(0)->getFilterType()==sim_filtercomponent_originalimage)&&(_composedFilter->getSimpleFilter(1)->getFilterType()==sim_filtercomponent_tooutput) )
            applyFilter=false; // since this represents the identity filter
    }
    if (applyFilter)
    {
        float* outputImage=CImageProcess::createRGBImage(_resolutionX,_resolutionY);
        float* outputDepthBuffer=nullptr;
        if (_composedFilter->includesDepthBufferModification())
            outputDepthBuffer=CImageProcess::createIntensityImage(_resolutionX,_resolutionY);
        int s=_resolutionX*_resolutionY*3;
        float* imageBuffer=new float[s];
        for (int i=0;i<s;i++)
            imageBuffer[i]=float(_rgbBuffer[i])/255.0f;
        trigger=_composedFilter->processAndTrigger(this,_resolutionX,_resolutionY,imageBuffer,_depthBuffer,outputImage,outputDepthBuffer,sensorAuxiliaryResult);
        delete[] imageBuffer;
        for (int i=0;i<s;i++)
            _rgbBuffer[i]=(unsigned char)(outputImage[i]*255.1f);
        if (_composedFilter->includesDepthBufferModification())
        {
            int s=_resolutionX*_resolutionY;
            for (int i=0;i<s;i++)
               _depthBuffer[i]=outputDepthBuffer[i];
            CImageProcess::deleteImage(outputDepthBuffer);
        }
        CImageProcess::deleteImage(outputImage);
    }
    return(trigger);
}

void CVisionSensor::serialize(CSer& ar)
{
    serializeMain(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Ccp");
            ar << _orthoViewSize << _viewAngle;
            ar << _nearClippingPlane << _farClippingPlane;
            ar.flush();

            ar.storeDataName("Res");
            ar << _desiredResolution[0] << _desiredResolution[1];
            ar.flush();

            ar.storeDataName("Dox");
            ar << _detectableEntityID;
            ar.flush();

            ar.storeDataName("Db2");
            ar << _defaultBufferValues[0] << _defaultBufferValues[1] << _defaultBufferValues[2];
            ar.flush();

            ar.storeDataName("Siz");
            ar << _size(0) << _size(1) << _size(2);
            ar.flush();

            ar.storeDataName("Rmd");
            ar << _renderMode;
            ar.flush();

            ar.storeDataName("Afr");
            ar << _attributesForRendering;
            ar.flush();

            ar.storeDataName("Va3");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_perspectiveOperation);
            SIM_SET_CLEAR_BIT(nothing,1,_explicitHandling);
            SIM_SET_CLEAR_BIT(nothing,2,!_showFogIfAvailable);
    //12/12/2011        SIM_SET_CLEAR_BIT(nothing,3,_detectAllDetectable);
            SIM_SET_CLEAR_BIT(nothing,4,_showVolumeWhenNotDetecting);
            SIM_SET_CLEAR_BIT(nothing,5,_showVolumeWhenDetecting);
            SIM_SET_CLEAR_BIT(nothing,6,_useLocalLights);
            SIM_SET_CLEAR_BIT(nothing,7,_useSameBackgroundAsEnvironment);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Va2");
            nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_useExternalImage);
            SIM_SET_CLEAR_BIT(nothing,1,_ignoreRGBInfo);
            SIM_SET_CLEAR_BIT(nothing,2,_ignoreDepthInfo);
            // RESERVED SIM_SET_CLEAR_BIT(nothing,3,_povFocalBlurEnabled);
            SIM_SET_CLEAR_BIT(nothing,4,!_computeImageBasicStats);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            color.serialize(ar,0);
            if (ar.setWritingMode())
                color.serialize(ar,0);

            ar.storeDataName("Cl2");
            ar.setCountingMode();
            activeColor.serialize(ar,0);
            if (ar.setWritingMode())
                activeColor.serialize(ar,0);

    // RESERVED     ar.storeDataName("Cl3");

    // RESERVED     ar.storeDataName("Cl4");

            ar.storeDataName("Cfr");
            ar.setCountingMode();
            _composedFilter->serialize(ar);
            if (ar.setWritingMode())
                _composedFilter->serialize(ar);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool povFocalBlurEnabled_backwardCompatibility_3_2_2016=false;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Ccp")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _orthoViewSize >> _viewAngle;
                        ar >> _nearClippingPlane >> _farClippingPlane;
                    }
                    if (theName.compare("Res")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _desiredResolution[0] >> _desiredResolution[1];
                        setDesiredResolution(_desiredResolution); // we need to set the real resolution!
                    }
                    if (theName.compare("Dox")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _detectableEntityID;
                    }
                    if (theName.compare("Db2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _defaultBufferValues[0] >> _defaultBufferValues[1] >> _defaultBufferValues[2];
                    }
                    if (theName.compare("Siz")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _size(0) >> _size(1) >> _size(2);
                    }
                    if (theName.compare("Rmd")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _renderMode;
                    }
                    if (theName.compare("Afr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _attributesForRendering;
                    }
                    if (theName=="Var")
                    { // keep a while for backward compatibility (2010/07/17)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _perspectiveOperation=SIM_IS_BIT_SET(nothing,0);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,1);
                        bool hideDetectionVolume=SIM_IS_BIT_SET(nothing,2);
                        _useSameBackgroundAsEnvironment=SIM_IS_BIT_SET(nothing,7);
                        if (hideDetectionVolume)
                        {
                            _showVolumeWhenNotDetecting=false;
                            _showVolumeWhenDetecting=false;
                        }
                    }
                    if (theName=="Va3")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _perspectiveOperation=SIM_IS_BIT_SET(nothing,0);
                        _explicitHandling=SIM_IS_BIT_SET(nothing,1);
                        _showFogIfAvailable=!SIM_IS_BIT_SET(nothing,2);
                        _showVolumeWhenNotDetecting=SIM_IS_BIT_SET(nothing,4);
                        _showVolumeWhenDetecting=SIM_IS_BIT_SET(nothing,5);
                        _useLocalLights=SIM_IS_BIT_SET(nothing,6);
                        _useSameBackgroundAsEnvironment=SIM_IS_BIT_SET(nothing,7);
                    }
                    if (theName=="Va2")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _useExternalImage=SIM_IS_BIT_SET(nothing,0);
                        _ignoreRGBInfo=SIM_IS_BIT_SET(nothing,1);
                        _ignoreDepthInfo=SIM_IS_BIT_SET(nothing,2);
                        povFocalBlurEnabled_backwardCompatibility_3_2_2016=SIM_IS_BIT_SET(nothing,3);
                        _computeImageBasicStats=!SIM_IS_BIT_SET(nothing,4);
                    }
                    if (theName.compare("Pv1")==0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        float povFocalDistance, povAperture;
                        int povBlurSamples;
                        ar >> povFocalDistance >> povAperture >> povBlurSamples;
                        _extensionString="povray {focalBlur {";
                        if (povFocalBlurEnabled_backwardCompatibility_3_2_2016)
                            _extensionString+="true} focalDist {";
                        else
                            _extensionString+="false} focalDist {";
                        _extensionString+=tt::FNb(0,povFocalDistance,3,false);
                        _extensionString+="} aperture {";
                        _extensionString+=tt::FNb(0,povAperture,3,false);
                        _extensionString+="} blurSamples {";
                        _extensionString+=tt::FNb(0,povBlurSamples,false);
                        _extensionString+="}}";
                    }
                    if (theName.compare("Cl1")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        color.serialize(ar,0);
                    }
                    if (theName.compare("Cl2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        activeColor.serialize(ar,0);
                    }
                    if (theName.compare("Cfr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity; // never use that info, unless loading unknown data!!!! (undo/redo stores dummy info in there)
                        delete _composedFilter;
                        _composedFilter=new CComposedFilter();
                        _composedFilter->serialize(ar);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            _reserveBuffers();

            if (ar.getSerializationVersionThatWroteThisFile()<17)
            { // on 29/08/2013 we corrected all default lights. So we need to correct for that change:
                CTTUtil::scaleColorUp_(color.colors);
                CTTUtil::scaleColorUp_(activeColor.colors);
            }
        }
    }
}

void CVisionSensor::serializeWExtIk(CExtIkSer& ar)
{
    serializeWExtIkMain(ar);
    CDummy::serializeWExtIkStatic(ar);
}

void CVisionSensor::detectVisionSensorEntity_executedViaUiThread(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections)
{
    FUNCTION_DEBUG;
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        detectEntity2(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=DETECT_VISION_SENSOR_ENTITY_UITHREADCMD;
        cmdIn.objectParams.push_back(this);
        cmdIn.intParams.push_back(entityID);
        cmdIn.boolParams.push_back(detectAll);
        cmdIn.boolParams.push_back(entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects);
        cmdIn.boolParams.push_back(hideEdgesIfModel);
        cmdIn.boolParams.push_back(overrideRenderableFlagsForNonCollections);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CVisionSensor::display(CViewableBase* renderingObject,int displayAttrib)
{
    FUNCTION_INSIDE_DEBUG("CVisionSensor::display");
    EASYLOCK(_objectMutex);
    displayVisionSensor(this,renderingObject,displayAttrib);
}

#ifdef SIM_WITH_OPENGL
void CVisionSensor::createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(bool useStencilBuffer)
{
    FUNCTION_DEBUG;
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        createGlContextAndFboAndTextureObjectIfNeeded(useStencilBuffer);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CREATE_GL_CONTEXT_FBO_TEXTURE_IF_NEEDED_UITHREADCMD;
        cmdIn.objectParams.push_back(this);
        cmdIn.boolParams.push_back(useStencilBuffer);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CVisionSensor::createGlContextAndFboAndTextureObjectIfNeeded(bool useStencilBuffer)
{
    FUNCTION_DEBUG;
    if ((_contextFboAndTexture!=nullptr)&&(_contextFboAndTexture->frameBufferObject->getUsingStencilBuffer()!=useStencilBuffer))
        _removeGlContextAndFboAndTextureObjectIfNeeded(); // if settings have changed (e.g. mirror was added), remove everything

    if (_contextFboAndTexture==nullptr)
    { // our objects are not yet there. Build them:

#ifdef USING_QOPENGLWIDGET
        QOpenGLWidget* otherWidgetToShareResourcesWith=nullptr;
#else
        QGLWidget* otherWidgetToShareResourcesWith=nullptr;
#endif
#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            otherWidgetToShareResourcesWith=App::mainWindow->openglWidget;
#endif

        // By default, we use QT_WINDOW_HIDE_TP, since
        // QT_OFFSCREEN_TP causes problems on certain GPUs, e.g.:
        // Intel Graphics Media Accelerator 3150.
        // In Headless mode under Linux, we use a offscreen type by default,
        // because otherwise even hidden windows are visible somehow
        int offscreenContextType=COffscreenGlContext::QT_WINDOW_HIDE_TP;
#ifdef LIN_VREP
#ifdef SIM_WITH_GUI
        if (App::mainWindow==nullptr) // headless mode
#endif
            offscreenContextType=COffscreenGlContext::QT_OFFSCREEN_TP;
#endif
        if (App::userSettings->offscreenContextType!=-1)
        {
            if (App::userSettings->offscreenContextType==0)
                offscreenContextType=COffscreenGlContext::QT_OFFSCREEN_TP;
            if (App::userSettings->offscreenContextType==1)
                offscreenContextType=COffscreenGlContext::QT_WINDOW_SHOW_TP;
            if (App::userSettings->offscreenContextType==2)
                offscreenContextType=COffscreenGlContext::QT_WINDOW_HIDE_TP;
        }

        // By default:
        // - we use Qt FBOs on Mac (the non-Qt FBOs always caused problems there)
        // - we use non-Qt FBOs on Windows and Linux (Qt FBOs on Linux cause a lot of problems, e.g.: NVIDIA Geforce 9600M GS
#ifdef WIN_VREP
        bool nativeFbo=true;
#endif
#ifdef MAC_VREP
        bool nativeFbo=false;
#endif
#ifdef LIN_VREP
        bool nativeFbo=true;
#endif
        if (App::userSettings->fboType!=-1)
        {
            if (App::userSettings->fboType==0)
                nativeFbo=true;
            if (App::userSettings->fboType==1)
                nativeFbo=false;
        }

        _contextFboAndTexture=new CVisionSensorGlStuff(_resolutionX,_resolutionY,offscreenContextType,!nativeFbo,otherWidgetToShareResourcesWith,useStencilBuffer,!App::userSettings->oglCompatibilityTweak1,App::userSettings->desiredOpenGlMajor,App::userSettings->desiredOpenGlMinor);
    }
}

void CVisionSensor::_removeGlContextAndFboAndTextureObjectIfNeeded()
{
    FUNCTION_DEBUG;
    if (_contextFboAndTexture!=nullptr)
    {
        if (_contextFboAndTexture->canDeleteNow())
            delete _contextFboAndTexture;
        else
            _contextFboAndTexture->deleteLater(); // We are in the wrong thread to delete it here
        _contextFboAndTexture=nullptr;
    }
}

void CVisionSensor::_handleMirrors(const std::vector<int>& activeMirrors,int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections)
{
    if (activeMirrors.size()==0)
        return;

    C7Vector camTr(getCumulativeTransformation());
    C7Vector camTri(camTr.getInverse());

    setFrustumCullingTemporarilyDisabled(true);
    // Prep stencil buffer:
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear (GL_STENCIL_BUFFER_BIT);
    int drawOk=1;

    std::vector<int> allMirrors;
    std::vector<float> allMirrorDist;
    for (int mir=0;mir<int(activeMirrors.size());mir++)
    {
        CMirror* myMirror=App::ct->objCont->getMirror(activeMirrors[mir]);
        C7Vector mmtr(myMirror->getCumulativeTransformation());
        mmtr=camTri*mmtr;
        allMirrors.push_back(activeMirrors[mir]);
        allMirrorDist.push_back(mmtr.X(2));
    }
    tt::orderAscending(allMirrorDist,allMirrors);

    for (int mir=0;mir<int(allMirrors.size());mir++)
    {
        CMirror* myMirror=App::ct->objCont->getMirror(allMirrors[mir]);

        C7Vector mtr(myMirror->getCumulativeTransformation());
        C7Vector mtri(mtr.getInverse());
        C3Vector mtrN(mtr.Q.getMatrix().axis[2]);
        C4Vector mtrAxis=mtr.Q.getAngleAndAxisNoChecking();
        C4Vector mtriAxis=mtri.Q.getAngleAndAxisNoChecking();
        float d=(mtrN*mtr.X);
        C3Vector v0(+myMirror->getMirrorWidth()*0.5f,-myMirror->getMirrorHeight()*0.5f,0.0f);
        C3Vector v1(+myMirror->getMirrorWidth()*0.5f,+myMirror->getMirrorHeight()*0.5f,0.0f);
        C3Vector v2(-myMirror->getMirrorWidth()*0.5f,+myMirror->getMirrorHeight()*0.5f,0.0f);
        C3Vector v3(-myMirror->getMirrorWidth()*0.5f,-myMirror->getMirrorHeight()*0.5f,0.0f);
        v0*=mtr;
        v1*=mtr;
        v2*=mtr;
        v3*=mtr;

        C3Vector MirrCam(camTr.X-mtr.X);
        bool inFrontOfMirror=(((MirrCam*mtrN)>0.0f)&&myMirror->getActive());

        glStencilFunc(GL_ALWAYS, drawOk, drawOk); // we can draw everywhere
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // we draw drawOk where depth test passes
        glDepthMask(GL_FALSE);
        glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
        glBegin (GL_QUADS);
        glVertex3fv(v0.data);
        glVertex3fv(v1.data);
        glVertex3fv(v2.data);
        glVertex3fv(v3.data);
        glEnd ();
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

        // Enable stencil masking:
        glStencilFunc(GL_EQUAL, drawOk, drawOk); // we draw only where stencil is drawOk
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // Draw the mirror view:
        if (inFrontOfMirror)
        {
            glEnable(GL_CLIP_PLANE0);
            double cpv[4]={-mtrN(0),-mtrN(1),-mtrN(2),d};
            glClipPlane(GL_CLIP_PLANE0,cpv);
            glPushMatrix();
            glTranslatef(mtr.X(0),mtr.X(1),mtr.X(2));
            glRotatef(mtrAxis(0)*radToDeg_f,mtrAxis(1),mtrAxis(2),mtrAxis(3));
            glScalef (1., 1., -1.);
            glTranslatef(mtri.X(0),mtri.X(1),mtri.X(2));
            glRotatef(mtriAxis(0)*radToDeg_f,mtriAxis(1),mtriAxis(2),mtriAxis(3));
            glFrontFace (GL_CW);
            CMirror::currentMirrorContentBeingRendered=myMirror->getObjectHandle();
            _drawObjects(entityID,detectAll,entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,hideEdgesIfModel,overrideRenderableFlagsForNonCollections);
            CMirror::currentMirrorContentBeingRendered=-1;
            glFrontFace (GL_CCW);
            glPopMatrix();
            glDisable(GL_CLIP_PLANE0);
        }

        // Now draw the mirror overlay:
        glPushAttrib (0xffffffff);
        ogl::disableLighting_useWithCare(); // only temporarily
        glDepthFunc(GL_ALWAYS);
        if (inFrontOfMirror)
        {
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glColor4f (myMirror->mirrorColor[0],myMirror->mirrorColor[1],myMirror->mirrorColor[2],1.0f-myMirror->getReflectance());
        glBegin (GL_QUADS);
        glVertex3fv(v0.data);
        glVertex3fv(v1.data);
        glVertex3fv(v2.data);
        glVertex3fv(v3.data);
        glEnd ();
        glPopAttrib();
        ogl::enableLighting_useWithCare();
        glDepthFunc(GL_LEQUAL);
        drawOk++;
    }
    glDisable(GL_STENCIL_TEST);
    setFrustumCullingTemporarilyDisabled(false);
}

void CVisionSensor::lookAt(CSView* viewObject,int viewPos[2],int viewSize[2])
{   // viewPos and viewSize can be nullptr.
    FUNCTION_DEBUG;
    int currentWinSize[2];
    int currentWinPos[2];
    if (viewObject!=nullptr)
    {
        viewObject->getViewSize(currentWinSize);
        viewObject->getViewPosition(currentWinPos);
    }
    else
    {
        currentWinSize[0]=viewSize[0];
        currentWinSize[1]=viewSize[1];
        currentWinPos[0]=viewPos[0];
        currentWinPos[1]=viewPos[1];
    }

    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    glEnable(GL_SCISSOR_TEST);
    glViewport(currentWinPos[0],currentWinPos[1],currentWinSize[0],currentWinSize[1]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,currentWinSize[0],0.0f,currentWinSize[1],-1.0f,1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glDisable(GL_DEPTH_TEST);


    glClearColor(0.3f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    if ( (_contextFboAndTexture!=nullptr)||getApplyExternalRenderedImage() )
    {
        float r0=float(currentWinSize[1])/float(currentWinSize[0]);
        float r1=float(_resolutionY)/float(_resolutionX);
        int c0[2];
        int c1[2];
        if (r1>=r0)
        {
            c0[1]=0;
            c1[1]=currentWinSize[1];
            int d=int(float(currentWinSize[1])/r1);
            c0[0]=(currentWinSize[0]-d)/2;
            c1[0]=c0[0]+d;

        }
        else
        {
            c0[0]=0;
            c1[0]=currentWinSize[0];
            int d=int(float(currentWinSize[0])*r1);
            c0[1]=(currentWinSize[1]-d)/2;
            c1[1]=c0[1]+d;
        }

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
        float texCorners[4]={0.0f,0.0f,1.0f,1.0f};

        if (getApplyExternalRenderedImage())
        {
            if (_rayTracingTextureName==(unsigned int)-1)
                glGenTextures(1,&_rayTracingTextureName);
            glBindTexture(GL_TEXTURE_2D,_rayTracingTextureName);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,_resolutionX,_resolutionY,0,GL_RGB,GL_UNSIGNED_BYTE,_rgbBuffer);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // keep to GL_LINEAR here!!
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,_rayTracingTextureName);
            glColor3f(1.0f,1.0f,1.0f);
        }
        else
        {
            _startTextureDisplay(_contextFboAndTexture->textureObject,false,0,false,false);
            if (_rayTracingTextureName!=(unsigned int)-1)
            {
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=DESTROY_GL_TEXTURE_UITHREADCMD;
                cmdIn.uintParams.push_back(_rayTracingTextureName);
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                _rayTracingTextureName=(unsigned int)-1;
            }
        }

        glBegin(GL_QUADS);
        glTexCoord2f(texCorners[0],texCorners[1]);
        glVertex3i(c0[0],c0[1],0);
        glTexCoord2f(texCorners[0],texCorners[3]);
        glVertex3i(c0[0],c1[1],0);
        glTexCoord2f(texCorners[2],texCorners[3]);
        glVertex3i(c1[0],c1[1],0);
        glTexCoord2f(texCorners[2],texCorners[1]);
        glVertex3i(c1[0],c0[1],0);
        glEnd();
        if (_rayTracingTextureName!=(unsigned int)-1)
            glDisable(GL_TEXTURE_2D);
        else
            _endTextureDisplay();

        if (_composedFilter!=nullptr)
            _composedFilter->displayOverlay(c0,c1);
    }


    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
}

CTextureObject* CVisionSensor::getTextureObject()
{
    if (_contextFboAndTexture!=nullptr)
        return(_contextFboAndTexture->textureObject);
    return(nullptr);
}
#endif
