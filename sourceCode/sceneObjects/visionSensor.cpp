#include <visionSensor.h>
#include <simInternal.h>
#include <global.h>
#include <tt.h>
#include <meshManip.h>
#include <sceneObjectOperations.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>
#include <vDateTime.h>
#include <vVarious.h>
#include <utils.h>
#include <threadPool_old.h>
#include <app.h>
#include <interfaceStackString.h>
#ifdef SIM_WITH_GUI
#include <visionSensorRendering.h>
#include <rendering.h>
#include <oGL.h>
#include <QtOpenGL>
#include <guiApp.h>
#endif

#define DEFAULT_RENDERING_ATTRIBUTES                                                                                   \
    (sim_displayattribute_renderpass | sim_displayattribute_forbidwireframe | sim_displayattribute_forbidedges |       \
     sim_displayattribute_originalcolors | sim_displayattribute_ignorelayer | sim_displayattribute_forvisionsensor)
#define DEFAULT_RAYTRACING_ATTRIBUTES                                                                                  \
    (sim_displayattribute_renderpass | sim_displayattribute_forbidwireframe | sim_displayattribute_forbidedges |       \
     sim_displayattribute_originalcolors | sim_displayattribute_ignorelayer | sim_displayattribute_forvisionsensor)

CVisionSensor::CVisionSensor()
{
    commonInit();
}

unsigned char *CVisionSensor::getRgbBufferPointer()
{
    return _rgbBuffer;
}

float *CVisionSensor::getDepthBufferPointer() const
{
    return _depthBuffer;
}

std::string CVisionSensor::getObjectTypeInfo() const
{
    return "visionSensor";
}
std::string CVisionSensor::getObjectTypeInfoExtended() const
{
    std::string retVal;
    retVal = "vision sensor (sampling resolution: ";
    retVal += boost::lexical_cast<std::string>(_resolution[0]) + "x";
    retVal += boost::lexical_cast<std::string>(_resolution[1]) + ")";
    return (retVal);
}
bool CVisionSensor::isPotentiallyCollidable() const
{
    return (false);
}
bool CVisionSensor::isPotentiallyMeasurable() const
{
    return (false);
}
bool CVisionSensor::isPotentiallyDetectable() const
{
    return (false);
}
bool CVisionSensor::isPotentiallyRenderable() const
{
    return (false);
}

void CVisionSensor::writeImage(const float *buff, int rgbGreyOrDepth)
{
    int p = 0;
    if (rgbGreyOrDepth == 0)
    { // RGB
        for (int j = 0; j < _resolution[1]; j++)
        {
            for (int i = 0; i < _resolution[0]; i++)
            {
                _rgbBuffer[3 * (j * _resolution[0] + i) + 0] = (unsigned char)(buff[3 * p + 0] * 255.1);
                _rgbBuffer[3 * (j * _resolution[0] + i) + 1] = (unsigned char)(buff[3 * p + 1] * 255.1);
                _rgbBuffer[3 * (j * _resolution[0] + i) + 2] = (unsigned char)(buff[3 * p + 2] * 255.1);
                p++;
            }
        }
        _emitImageChangedEvent();
    }
    else if (rgbGreyOrDepth == 1)
    { // Greyscale
        for (int j = 0; j < _resolution[1]; j++)
        {
            for (int i = 0; i < _resolution[0]; i++)
            {
                _rgbBuffer[3 * (j * _resolution[0] + i) + 0] = (unsigned char)(buff[p] * 255.1);
                _rgbBuffer[3 * (j * _resolution[0] + i) + 1] = (unsigned char)(buff[p] * 255.1);
                _rgbBuffer[3 * (j * _resolution[0] + i) + 2] = (unsigned char)(buff[p] * 255.1);
                p++;
            }
        }
        _emitImageChangedEvent();
    }
    else
    { // depth
        for (int j = 0; j < _resolution[1]; j++)
        {
            for (int i = 0; i < _resolution[0]; i++)
            {
                _depthBuffer[j * _resolution[0] + i] = buff[p];
                p++;
            }
        }
        _emitDepthChangedEvent();
    }
}

float *CVisionSensor::readPortionOfImage(int posX, int posY, int sizeX, int sizeY, int rgbGreyOrDepth) const
{
    if ((posX < 0) || (posY < 0) || (sizeX < 1) || (sizeY < 1) || (posX + sizeX > _resolution[0]) || (posY + sizeY > _resolution[1]))
        return (nullptr);
    float *buff;
    if (rgbGreyOrDepth == 0)
        buff = new float[sizeX * sizeY * 3];
    else
        buff = new float[sizeX * sizeY];
    int p = 0;
    if (rgbGreyOrDepth == 0)
    { // RGB
        for (int j = posY; j < posY + sizeY; j++)
        {
            for (int i = posX; i < posX + sizeX; i++)
            {
                buff[3 * p + 0] = float(_rgbBuffer[3 * (j * _resolution[0] + i) + 0]) / 255.0;
                buff[3 * p + 1] = float(_rgbBuffer[3 * (j * _resolution[0] + i) + 1]) / 255.0;
                buff[3 * p + 2] = float(_rgbBuffer[3 * (j * _resolution[0] + i) + 2]) / 255.0;
                p++;
            }
        }
    }
    else if (rgbGreyOrDepth == 1)
    { // Greyscale
        for (int j = posY; j < posY + sizeY; j++)
        {
            for (int i = posX; i < posX + sizeX; i++)
            {
                buff[p] = float(_rgbBuffer[3 * (j * _resolution[0] + i) + 0]) / 255.0;
                buff[p] += float(_rgbBuffer[3 * (j * _resolution[0] + i) + 1]) / 255.0;
                buff[p] += float(_rgbBuffer[3 * (j * _resolution[0] + i) + 2]) / 255.0;
                buff[p] /= 3.0;
                p++;
            }
        }
    }
    else
    { // depth
        for (int j = posY; j < posY + sizeY; j++)
        {
            for (int i = posX; i < posX + sizeX; i++)
            {
                buff[p] = _depthBuffer[j * _resolution[0] + i];
                p++;
            }
        }
    }
    return (buff);
}

bool CVisionSensor::writePortionOfCharImage(const unsigned char *img, int posX, int posY, int sizeX, int sizeY, int option)
{ // option: bit0 set --> greyscale. bit1 set --> with alpha channel, bit2 set --> do not apply processing
    bool retVal = false;
    if ((posX >= 0) && (posY >= 0) && (sizeX >= 1) && (sizeY >= 1) && (posX + sizeX <= _resolution[0]) && (posY + sizeY <= _resolution[1]))
    {
        retVal = true;
        if ((option & 2) == 0)
        { // rgb or greyscale
            int p = 0;
            if ((option & 1) != 0)
            { // greyscale
                for (int j = posY; j < posY + sizeY; j++)
                {
                    for (int i = posX; i < posX + sizeX; i++)
                    {
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 0] = img[p];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 1] = img[p];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 2] = img[p];
                        p++;
                    }
                }
            }
            else
            { // rgb
                for (int j = posY; j < posY + sizeY; j++)
                {
                    for (int i = posX; i < posX + sizeX; i++)
                    {
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 0] = img[3 * p + 0];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 1] = img[3 * p + 1];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 2] = img[3 * p + 2];
                        p++;
                    }
                }
            }
        }
        else
        { // rgba or greyscale+a
            int p = 0;
            if ((option & 1) != 0)
            { // greyscale+a
                for (int j = posY; j < posY + sizeY; j++)
                {
                    for (int i = posX; i < posX + sizeX; i++)
                    {
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 0] = img[2 * p + 0];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 1] = img[2 * p + 0];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 2] = img[2 * p + 0];
                        p++;
                    }
                }
            }
            else
            { // rgba
                for (int j = posY; j < posY + sizeY; j++)
                {
                    for (int i = posX; i < posX + sizeX; i++)
                    {
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 0] = img[4 * p + 0];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 1] = img[4 * p + 1];
                        _rgbBuffer[3 * (j * _resolution[0] + i) + 2] = img[4 * p + 2];
                        p++;
                    }
                }
            }
        }
        _emitImageChangedEvent();
        if ((option & 4) == 0)
            _computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values
#ifdef SIM_WITH_GUI
        if (_contextFboAndTexture == nullptr)
            createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(false);

        if (_contextFboAndTexture != nullptr)
            _contextFboAndTexture->textureObject->setImage(false, false, true, _rgbBuffer); // Update the texture
#endif
    }
    return (retVal);
}

unsigned char *CVisionSensor::readPortionOfCharImage(int posX, int posY, int sizeX, int sizeY, double cutoffRgba,
                                                     int option) const
{
    if ((posX < 0) || (posY < 0) || (sizeX < 1) || (sizeY < 1) || (posX + sizeX > _resolution[0]) ||
        (posY + sizeY > _resolution[1]))
        return (nullptr);
    unsigned char *buff = nullptr;
    if ((option & 2) == 0)
    {
        int p = 0;
        if ((option & 1) != 0)
        {
            buff = new unsigned char[sizeX * sizeY];
            for (int j = posY; j < posY + sizeY; j++)
            {
                for (int i = posX; i < posX + sizeX; i++)
                {
                    unsigned int v = _rgbBuffer[3 * (j * _resolution[0] + i) + 0];
                    v += _rgbBuffer[3 * (j * _resolution[0] + i) + 1];
                    v += _rgbBuffer[3 * (j * _resolution[0] + i) + 2];
                    buff[p] = (unsigned char)(v / 3);
                    p++;
                }
            }
        }
        else
        {
            buff = new unsigned char[sizeX * sizeY * 3];
            for (int j = posY; j < posY + sizeY; j++)
            {
                for (int i = posX; i < posX + sizeX; i++)
                {
                    buff[3 * p + 0] = _rgbBuffer[3 * (j * _resolution[0] + i) + 0];
                    buff[3 * p + 1] = _rgbBuffer[3 * (j * _resolution[0] + i) + 1];
                    buff[3 * p + 2] = _rgbBuffer[3 * (j * _resolution[0] + i) + 2];
                    p++;
                }
            }
        }
    }
    else
    {
        int p = 0;
        if ((option & 1) != 0)
        {
            buff = new unsigned char[sizeX * sizeY * 2];
            for (int j = posY; j < posY + sizeY; j++)
            {
                for (int i = posX; i < posX + sizeX; i++)
                {
                    unsigned int v = _rgbBuffer[3 * (j * _resolution[0] + i) + 0];
                    v += _rgbBuffer[3 * (j * _resolution[0] + i) + 1];
                    v += _rgbBuffer[3 * (j * _resolution[0] + i) + 2];
                    buff[2 * p + 0] = (unsigned char)(v / 3);
                    if (_depthBuffer[j * _resolution[0] + i] > (float)cutoffRgba)
                        buff[2 * p + 1] = 0;
                    else
                        buff[2 * p + 1] = 255;
                    p++;
                }
            }
        }
        else
        {
            buff = new unsigned char[sizeX * sizeY * 4];
            for (int j = posY; j < posY + sizeY; j++)
            {
                for (int i = posX; i < posX + sizeX; i++)
                {
                    buff[4 * p + 0] = _rgbBuffer[3 * (j * _resolution[0] + i) + 0];
                    buff[4 * p + 1] = _rgbBuffer[3 * (j * _resolution[0] + i) + 1];
                    buff[4 * p + 2] = _rgbBuffer[3 * (j * _resolution[0] + i) + 2];
                    if (_depthBuffer[j * _resolution[0] + i] > (float)cutoffRgba)
                        buff[4 * p + 3] = 0;
                    else
                        buff[4 * p + 3] = 255;
                    p++;
                }
            }
        }
    }
    return (buff);
}

void CVisionSensor::computeBoundingBox()
{
    C7Vector fr;
    fr.Q.setIdentity();
    fr.X = C3Vector(0.0, 0.0, -1.0) * _visionSensorSize;
    _setBB(fr, C3Vector(1.0, 1.0, 2.0) * _visionSensorSize * 0.5);
}

void CVisionSensor::setIsInScene(bool s)
{
    CSceneObject::setIsInScene(s);
    if (s)
        color.setEventParams(_objectHandle);
    else
        color.setEventParams(-1);
}

void CVisionSensor::commonInit()
{
    _objectType = sim_sceneobject_visionsensor;
    _nearClippingPlane = 0.01;
    _farClippingPlane = 10.0;
    _viewAngle = 60.0 * degToRad;
    _orthoViewSize = 0.1;
    _showFogIfAvailable = true;
    _useLocalLights = false;
    _inApplyFilterRoutine = false;
    _rayTracingTextureName = (unsigned int)-1;

    _visibilityLayer = VISION_SENSOR_LAYER;
    _detectableEntityHandle = -1;
    _localObjectSpecialProperty = 0;
    _explicitHandling = false;
    _showVolume = true;

    _ignoreRGBInfo = false;
    _ignoreDepthInfo = false;
    _computeImageBasicStats = true;
    _renderMode = sim_rendermode_opengl; // visible
    _attributesForRendering = DEFAULT_RENDERING_ATTRIBUTES;
    _emitImageChangedEventEnabled = false;
    _emitDepthChangedEventEnabled = false;
    _useExternalImage = false;
    _useSameBackgroundAsEnvironment = true;

    _composedFilter = new CComposedFilter();

    _defaultBufferValues[0] = 0.0;
    _defaultBufferValues[1] = 0.0;
    _defaultBufferValues[2] = 0.0;

    sensorResult.sensorWasTriggered = false;
    sensorResult.sensorResultIsValid = false;
    sensorResult.calcTimeInMs = 0;
    sensorAuxiliaryResult.clear();
    color.setDefaultValues();
    color.setColor(0.05f, 0.42f, 1.0f, sim_colorcomponent_ambient_diffuse);

    _resolution[0] = 256;
    _resolution[1] = 256;
    _visionSensorSize = 0.01;
    _perspective = false;
    if (_extensionString.size() != 0)
        _extensionString += " ";
    _extensionString += "povray {focalBlur {false} focalDist {2.00} aperture{0.05} blurSamples{10}}";

    // Following means full-size for windowed external renderings:
    _extWindowedViewSize[0] = 0;
    _extWindowedViewSize[1] = 0;

    _extWindowedViewPos[0] = 0;
    _extWindowedViewPos[1] = 0;

#ifdef SIM_WITH_GUI
    _contextFboAndTexture = nullptr;
#endif

    _rgbBuffer = nullptr;
    _depthBuffer = nullptr;
    _reserveBuffers();

    _objectAlias = getObjectTypeInfo();
    _objectName_old = getObjectTypeInfo();
    _objectAltName_old = tt::getObjectAltNameFromObjectName(_objectName_old.c_str());
    computeBoundingBox();
    computeVolumeVectors();
}

void CVisionSensor::setUseExternalImage(bool u)
{
    bool diff = (_useExternalImage != u);
    if (diff)
    {
        _useExternalImage = u;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_useExtImage.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _useExternalImage);
            App::worldContainer->pushEvent();
        }
    }
}

bool CVisionSensor::getUseExternalImage() const
{
    return _useExternalImage;
}

void CVisionSensor::setEmitImageChangedEvent(bool e)
{
    bool diff = (_emitImageChangedEventEnabled != e);
    if (diff)
    {
        _emitImageChangedEventEnabled = e;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_emitImageChangedEvent.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _emitImageChangedEventEnabled);
            App::worldContainer->pushEvent();
        }
    }
}

void CVisionSensor::setEmitDepthChangedEvent(bool e)
{
    bool diff = (_emitDepthChangedEventEnabled != e);
    if (diff)
    {
        _emitDepthChangedEventEnabled = e;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_emitDepthChangedEvent.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _emitDepthChangedEventEnabled);
            App::worldContainer->pushEvent();
        }
    }
}

void CVisionSensor::_emitImageChangedEvent() const
{
    if (_emitImageChangedEventEnabled && _isInScene && App::worldContainer->getEventsEnabled())
    {
        const char *cmd = propVisionSensor_imageBuffer.name;
        CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->appendKeyBuff(cmd, _rgbBuffer, 3 * _resolution[0] * _resolution[1]);
        App::worldContainer->pushEvent();
    }
}

void CVisionSensor::_emitDepthChangedEvent() const
{
    if (_emitDepthChangedEventEnabled && _isInScene && App::worldContainer->getEventsEnabled())
    {
        const char *cmd = propVisionSensor_depthBuffer.name;
        CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
        ev->appendKeyFloatArray(cmd, _depthBuffer, _resolution[0] * _resolution[1]);
        App::worldContainer->pushEvent();
    }
}

bool CVisionSensor::getInternalRendering() const
{
    return (_renderMode < sim_rendermode_povray);
}

bool CVisionSensor::getApplyExternalRenderedImage() const
{
    return ((_renderMode == sim_rendermode_povray) || (_renderMode == sim_rendermode_extrenderer) ||
            (_renderMode == sim_rendermode_opengl3));
}

CComposedFilter *CVisionSensor::getComposedFilter() const
{
    return (_composedFilter);
}

void CVisionSensor::setComposedFilter(CComposedFilter *newFilter)
{
    delete _composedFilter;
    _composedFilter = newFilter;
}

CColorObject *CVisionSensor::getColor()
{
    return (&color);
}

void CVisionSensor::setExtWindowSizeAndPos(int sizeX, int sizeY, int posX, int posY)
{
    _extWindowedViewSize[0] = sizeX;
    _extWindowedViewSize[1] = sizeY;
    _extWindowedViewPos[0] = posX;
    _extWindowedViewPos[1] = posY;
}

void CVisionSensor::getExtWindowSizeAndPos(int &sizeX, int &sizeY, int &posX, int &posY) const
{
    sizeX = _extWindowedViewSize[0];
    sizeY = _extWindowedViewSize[1];
    posX = _extWindowedViewPos[0];
    posY = _extWindowedViewPos[1];
}

CVisionSensor::~CVisionSensor()
{
#ifdef SIM_WITH_GUI
    _removeGlContextAndFboAndTextureObjectIfNeeded();
    if (_rayTracingTextureName != (unsigned int)-1)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId = DESTROY_GL_TEXTURE_UITHREADCMD;
        cmdIn.uintParams.push_back(_rayTracingTextureName);
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
        _rayTracingTextureName = (unsigned int)-1;
    }
#endif

    delete[] _depthBuffer;
    delete[] _rgbBuffer;

    delete _composedFilter;
}

double CVisionSensor::getCalculationTime() const
{
    return (double(sensorResult.calcTimeInMs) * 0.001);
}

std::string CVisionSensor::getDetectableEntityLoadAlias() const
{
    return (_detectableEntityLoadAlias);
}

std::string CVisionSensor::getDetectableEntityLoadName_old() const
{
    return (_detectableEntityLoadName_old);
}

void CVisionSensor::_reserveBuffers()
{
    delete[] _depthBuffer;
    delete[] _rgbBuffer;
    _depthBuffer = new float[_resolution[0] * _resolution[1]];
    _rgbBuffer = new unsigned char[3 * _resolution[0] * _resolution[1]];
    _clearBuffers();
#ifdef SIM_WITH_GUI
    _removeGlContextAndFboAndTextureObjectIfNeeded();
#endif
}

void CVisionSensor::_clearBuffers()
{
    for (int i = 0; i < _resolution[0] * _resolution[1]; i++)
    {
        if (_useSameBackgroundAsEnvironment)
        {
            _rgbBuffer[3 * i + 0] = (unsigned char)(App::currentWorld->environment->fogBackgroundColor[0] * 255.1);
            _rgbBuffer[3 * i + 1] = (unsigned char)(App::currentWorld->environment->fogBackgroundColor[1] * 255.1);
            _rgbBuffer[3 * i + 2] = (unsigned char)(App::currentWorld->environment->fogBackgroundColor[2] * 255.1);
        }
        else
        {
            _rgbBuffer[3 * i + 0] = (unsigned char)(_defaultBufferValues[0] * 255.1);
            _rgbBuffer[3 * i + 1] = (unsigned char)(_defaultBufferValues[1] * 255.1);
            _rgbBuffer[3 * i + 2] = (unsigned char)(_defaultBufferValues[2] * 255.1);
        }
    }
    _emitImageChangedEvent();
    for (int i = 0; i < _resolution[0] * _resolution[1]; i++)
        _depthBuffer[i] = 1.0;
    _emitDepthChangedEvent();
}

void CVisionSensor::setResolution(const int r[2])
{ // override
    int res[2] = {r[0], r[1]};
    tt::limitValue(1, 4096, res[0]);
    tt::limitValue(1, 4096, res[1]);
    CViewableBase::setResolution(res);
    _reserveBuffers();
}

void CVisionSensor::setUseEnvironmentBackgroundColor(bool s)
{
    bool diff = (_useSameBackgroundAsEnvironment != s);
    if (diff)
    {
        _useSameBackgroundAsEnvironment = s;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_backgroundSameAsEnv.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _useSameBackgroundAsEnvironment);
            App::worldContainer->pushEvent();
        }
    }
}

bool CVisionSensor::getUseEnvironmentBackgroundColor() const
{
    return _useSameBackgroundAsEnvironment;
}

void CVisionSensor::setVisionSensorSize(const double s)
{
    if (_visionSensorSize != s)
    {
        _visionSensorSize = s;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_size.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyDouble(cmd, _visionSensorSize);
            App::worldContainer->pushEvent();
        }
        computeBoundingBox();
    }
}

double CVisionSensor::getVisionSensorSize() const
{
    return (_visionSensorSize);
}

void CVisionSensor::setExplicitHandling(bool eh)
{
    bool diff = (_explicitHandling != eh);
    if (diff)
    {
        _explicitHandling = eh;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_explicitHandling.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _explicitHandling);
            App::worldContainer->pushEvent();
        }
    }
}

bool CVisionSensor::getExplicitHandling() const
{
    return _explicitHandling;
}

void CVisionSensor::setIgnoreRGBInfo(bool ignore)
{
    bool diff = (_ignoreRGBInfo != ignore);
    if (diff)
    {
        _ignoreRGBInfo = ignore;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_ignoreRgbInfo.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _ignoreRGBInfo);
            App::worldContainer->pushEvent();
        }
    }
}

bool CVisionSensor::getIgnoreRGBInfo() const
{
    return _ignoreRGBInfo;
}

void CVisionSensor::setComputeImageBasicStats(bool c)
{
    bool diff = (_computeImageBasicStats != c);
    if (diff)
    {
        _computeImageBasicStats = c;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_omitPacket1.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, !_computeImageBasicStats);
            App::worldContainer->pushEvent();
        }
    }
}

bool CVisionSensor::getComputeImageBasicStats() const
{
    return _computeImageBasicStats;
}

void CVisionSensor::setIgnoreDepthInfo(bool ignore)
{
    bool diff = (_ignoreDepthInfo != ignore);
    if (diff)
    {
        _ignoreDepthInfo = ignore;
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_ignoreDepthInfo.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyBool(cmd, _ignoreDepthInfo);
            App::worldContainer->pushEvent();
        }
    }
}

bool CVisionSensor::getIgnoreDepthInfo() const
{
    return _ignoreDepthInfo;
}

void CVisionSensor::setRenderMode(int mode)
{
    if (_renderMode != mode)
    {
        _renderMode = mode;
        _attributesForRendering = DEFAULT_RENDERING_ATTRIBUTES;
        if (mode == sim_rendermode_povray)
        {
            setIgnoreDepthInfo(true);
            _attributesForRendering = DEFAULT_RAYTRACING_ATTRIBUTES;
        }
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_renderMode.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyInt(cmd, _renderMode);
            App::worldContainer->pushEvent();
        }
    }
}

int CVisionSensor::getRenderMode() const
{
    return _renderMode;
}

void CVisionSensor::setAttributesForRendering(int attr)
{
    _attributesForRendering = attr;
}

int CVisionSensor::getAttributesForRendering() const
{
    return (_attributesForRendering);
}

void CVisionSensor::setDetectableEntityHandle(int entityHandle)
{
    _detectableEntityHandle = entityHandle;
}

int CVisionSensor::getDetectableEntityHandle() const
{
    return (_detectableEntityHandle);
}

void CVisionSensor::setDefaultBufferValues(const float v[3])
{
    bool diff = ( (_defaultBufferValues[0] != v[0]) || (_defaultBufferValues[1] != v[1]) || (_defaultBufferValues[2] != v[2]) );
    if (diff)
    {
        for (int i = 0; i < 3; i++)
            _defaultBufferValues[i] = v[i];
        if (_isInScene && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propVisionSensor_backgroundCol.name;
            CCbor *ev = App::worldContainer->createSceneObjectChangedEvent(this, false, cmd, true);
            ev->appendKeyFloatArray(cmd, _defaultBufferValues, 3);
            App::worldContainer->pushEvent();
        }
    }
}

void CVisionSensor::getDefaultBufferValues(float v[3]) const
{
    for (int i = 0; i < 3; i++)
        v[i] = _defaultBufferValues[i];
}

void CVisionSensor::resetSensor()
{
    sensorAuxiliaryResult.clear();
    sensorResult.sensorWasTriggered = false;
    sensorResult.sensorResultIsValid = false;
    sensorResult.calcTimeInMs = 0;
    for (int i = 0; i < 3; i++)
    {
        sensorResult.sensorDataRed[i] = 0;
        sensorResult.sensorDataGreen[i] = 0;
        sensorResult.sensorDataBlue[i] = 0;
        sensorResult.sensorDataIntensity[i] = 0;
        sensorResult.sensorDataDepth[i] = 0.0;
    }
    if (!_useExternalImage)
        _clearBuffers();
}

bool CVisionSensor::setExternalImage_old(const float *img, bool imgIsGreyScale, bool noProcessing)
{
    if (imgIsGreyScale)
    {
        int n = _resolution[0] * _resolution[1];
        for (int i = 0; i < n; i++)
        {
            unsigned char v = (unsigned char)(img[i] * 255.1);
            _rgbBuffer[3 * i + 0] = v;
            _rgbBuffer[3 * i + 1] = v;
            _rgbBuffer[3 * i + 2] = v;
        }
    }
    else
    {
        int n = _resolution[0] * _resolution[1] * 3;
        for (int i = 0; i < n; i++)
            _rgbBuffer[i] = (unsigned char)(img[i] * 255.1);
    }
    bool returnValue = false;
    if (!noProcessing)
        returnValue = _computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values

#ifdef SIM_WITH_GUI
    if (_contextFboAndTexture == nullptr)
        createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(false);

    if (_contextFboAndTexture != nullptr)
        _contextFboAndTexture->textureObject->setImage(false, false, true, _rgbBuffer); // Update the texture
#endif
    _emitImageChangedEvent();
    return (returnValue);
}

bool CVisionSensor::setExternalCharImage_old(const unsigned char *img, bool imgIsGreyScale, bool noProcessing)
{
    if (imgIsGreyScale)
    {
        int n = _resolution[0] * _resolution[1];
        for (int i = 0; i < n; i++)
        {
            _rgbBuffer[3 * i + 0] = img[i];
            _rgbBuffer[3 * i + 1] = img[i];
            _rgbBuffer[3 * i + 2] = img[i];
        }
    }
    else
    {
        int n = _resolution[0] * _resolution[1] * 3;
        for (int i = 0; i < n; i++)
            _rgbBuffer[i] = img[i];
    }
    bool returnValue = false;
    if (!noProcessing)
        returnValue = _computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values

#ifdef SIM_WITH_GUI
    if (_contextFboAndTexture == nullptr)
        createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(false);

    if (_contextFboAndTexture != nullptr)
        _contextFboAndTexture->textureObject->setImage(false, false, true, _rgbBuffer); // Update the texture
#endif
    _emitImageChangedEvent();
    return (returnValue);
}

void CVisionSensor::setDepthBuffer(const float *img)
{
    int n = _resolution[0] * _resolution[1];
    for (int i = 0; i < n; i++)
        _depthBuffer[i] = img[i];
    _emitDepthChangedEvent();
}

bool CVisionSensor::handleSensor()
{
    TRACE_INTERNAL;
    sensorAuxiliaryResult.clear();
    sensorResult.sensorWasTriggered = false;
    sensorResult.sensorResultIsValid = false;
    sensorResult.calcTimeInMs = 0;
    for (int i = 0; i < 3; i++)
    {
        sensorResult.sensorDataRed[i] = 0;
        sensorResult.sensorDataGreen[i] = 0;
        sensorResult.sensorDataBlue[i] = 0;
        sensorResult.sensorDataIntensity[i] = 0;
        sensorResult.sensorDataDepth[i] = 0.0;
    }
    if (!App::currentWorld->mainSettings_old->visionSensorsEnabled)
        return (false);
    if (_useExternalImage) // those 2 lines added on 2010/12/12
        return (false);
    int stTime = (int)VDateTime::getTimeInMs();
    detectEntity(_detectableEntityHandle, _detectableEntityHandle == -1, false, false, false);
#ifdef SIM_WITH_GUI
    if (_contextFboAndTexture != nullptr)
        _contextFboAndTexture->textureObject->setImage(false, false, true, _rgbBuffer); // Update the texture
#endif
    sensorResult.calcTimeInMs = VDateTime::getTimeDiffInMs(stTime);
    return (sensorResult.sensorWasTriggered);
}

bool CVisionSensor::checkSensor(int entityID, bool overrideRenderableFlagsForNonCollections)
{ // This function should only be used by simCheckVisionSensor(Ex) functions! It will temporarily buffer current result
    if (_useExternalImage) // added those 2 lines on 2010/12/21
        return (false);

    // 1. We save current state:
    SHandlingResult cop;
    std::vector<std::vector<double>> sensorAuxiliaryResultCop(sensorAuxiliaryResult);
    sensorAuxiliaryResult.clear();
    cop.sensorWasTriggered = sensorResult.sensorWasTriggered;
    cop.sensorResultIsValid = sensorResult.sensorResultIsValid;
    cop.calcTimeInMs = sensorResult.calcTimeInMs;
    for (int i = 0; i < 3; i++)
    {
        cop.sensorDataRed[i] = sensorResult.sensorDataRed[i];
        cop.sensorDataGreen[i] = sensorResult.sensorDataGreen[i];
        cop.sensorDataBlue[i] = sensorResult.sensorDataBlue[i];
        cop.sensorDataIntensity[i] = sensorResult.sensorDataIntensity[i];
        cop.sensorDataDepth[i] = sensorResult.sensorDataDepth[i];
    }
    unsigned char *copIm = new unsigned char[_resolution[0] * _resolution[1] * 3];
    float *copDep = new float[_resolution[0] * _resolution[1]];
    for (int i = 0; i < _resolution[0] * _resolution[1]; i++)
    {
        copIm[3 * i + 0] = _rgbBuffer[3 * i + 0];
        copIm[3 * i + 1] = _rgbBuffer[3 * i + 1];
        copIm[3 * i + 2] = _rgbBuffer[3 * i + 2];
        copDep[i] = _depthBuffer[i];
    }
    // 2. Do the detection:
    bool all = (entityID == -1);
    bool retVal = detectEntity(entityID, all, false, false,
                               overrideRenderableFlagsForNonCollections); // We don't swap image buffers!
    // 3. Restore previous state:
    sensorResult.sensorWasTriggered = cop.sensorWasTriggered;
    sensorResult.sensorResultIsValid = cop.sensorResultIsValid;
    sensorResult.calcTimeInMs = cop.calcTimeInMs;
    sensorAuxiliaryResult.assign(sensorAuxiliaryResultCop.begin(), sensorAuxiliaryResultCop.end());
    for (int i = 0; i < 3; i++)
    {
        sensorResult.sensorDataRed[i] = cop.sensorDataRed[i];
        sensorResult.sensorDataGreen[i] = cop.sensorDataGreen[i];
        sensorResult.sensorDataBlue[i] = cop.sensorDataBlue[i];
        sensorResult.sensorDataIntensity[i] = cop.sensorDataIntensity[i];
        sensorResult.sensorDataDepth[i] = cop.sensorDataDepth[i];
    }
    for (int i = 0; i < _resolution[0] * _resolution[1]; i++)
    {
        _rgbBuffer[3 * i + 0] = copIm[3 * i + 0];
        _rgbBuffer[3 * i + 1] = copIm[3 * i + 1];
        _rgbBuffer[3 * i + 2] = copIm[3 * i + 2];
        _depthBuffer[i] = copDep[i];
    }
    // 4. Release memory
    delete[] copIm;
    delete[] copDep;

    return (retVal);
}

float *CVisionSensor::checkSensorEx(int entityID, bool imageBuffer,
                                    bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                    bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections)
{ // This function should only be used by simCheckVisionSensor(Ex) functions! It will temporarily buffer current result
    if (_useExternalImage) // added those 2 lines on 2010/12/21
        return (nullptr);

    // 1. We save current state:
    SHandlingResult cop;
    std::vector<std::vector<double>> sensorAuxiliaryResultCop(sensorAuxiliaryResult);
    sensorAuxiliaryResult.clear();
    cop.sensorWasTriggered = sensorResult.sensorWasTriggered;
    cop.sensorResultIsValid = sensorResult.sensorResultIsValid;
    cop.calcTimeInMs = sensorResult.calcTimeInMs;
    for (int i = 0; i < 3; i++)
    {
        cop.sensorDataRed[i] = sensorResult.sensorDataRed[i];
        cop.sensorDataGreen[i] = sensorResult.sensorDataGreen[i];
        cop.sensorDataBlue[i] = sensorResult.sensorDataBlue[i];
        cop.sensorDataIntensity[i] = sensorResult.sensorDataIntensity[i];
        cop.sensorDataDepth[i] = sensorResult.sensorDataDepth[i];
    }
    unsigned char *copIm = new unsigned char[_resolution[0] * _resolution[1] * 3];
    float *copDep = new float[_resolution[0] * _resolution[1]];
    for (int i = 0; i < _resolution[0] * _resolution[1]; i++)
    {
        copIm[3 * i + 0] = _rgbBuffer[3 * i + 0];
        copIm[3 * i + 1] = _rgbBuffer[3 * i + 1];
        copIm[3 * i + 2] = _rgbBuffer[3 * i + 2];
        copDep[i] = _depthBuffer[i];
    }
    // 2. Do the detection:
    bool all = (entityID == -1);
    detectEntity(entityID, all, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, hideEdgesIfModel,
                 overrideRenderableFlagsForNonCollections); // we don't swap image buffers!
    // 3. Prepare return buffer:
    float *retBuffer = nullptr;
    int l = _resolution[0] * _resolution[1];
    if (imageBuffer)
        l *= 3;
    retBuffer = new float[l];
    if (imageBuffer)
    {
        for (int i = 0; i < l; i++)
            retBuffer[i] = float(_rgbBuffer[i]) / 255.0;
    }
    else
    {
        for (int i = 0; i < l; i++)
            retBuffer[i] = _depthBuffer[i];
    }
    // 4. Restore previous state:
    sensorAuxiliaryResult.assign(sensorAuxiliaryResultCop.begin(), sensorAuxiliaryResultCop.end());
    sensorResult.sensorWasTriggered = cop.sensorWasTriggered;
    sensorResult.sensorResultIsValid = cop.sensorResultIsValid;
    sensorResult.calcTimeInMs = cop.calcTimeInMs;
    for (int i = 0; i < 3; i++)
    {
        sensorResult.sensorDataRed[i] = cop.sensorDataRed[i];
        sensorResult.sensorDataGreen[i] = cop.sensorDataGreen[i];
        sensorResult.sensorDataBlue[i] = cop.sensorDataBlue[i];
        sensorResult.sensorDataIntensity[i] = cop.sensorDataIntensity[i];
        sensorResult.sensorDataDepth[i] = cop.sensorDataDepth[i];
    }
    for (int i = 0; i < _resolution[0] * _resolution[1]; i++)
    {
        _rgbBuffer[3 * i + 0] = copIm[3 * i + 0];
        _rgbBuffer[3 * i + 1] = copIm[3 * i + 1];
        _rgbBuffer[3 * i + 2] = copIm[3 * i + 2];
        _depthBuffer[i] = copDep[i];
    }
    // 5. Release memory
    delete[] copIm;
    delete[] copDep;

    return (retBuffer);
}

bool CVisionSensor::detectEntity(int entityID, bool detectAll,
                                 bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                 bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections)
{ // if entityID is -1, all detectable objects are rendered!
    TRACE_INTERNAL;
    bool retVal = false;
    App::worldContainer->calcInfo->visionSensorSimulationStart();

    bool validRendering = false;
#ifdef SIM_WITH_GUI
    validRendering = detectVisionSensorEntity_executedViaUiThread(
        entityID, detectAll, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, hideEdgesIfModel,
        overrideRenderableFlagsForNonCollections);
#else
#endif

    if (validRendering)
    {
        retVal = _computeDefaultReturnValuesAndApplyFilters(); // this might overwrite the default return values
        sensorResult.sensorWasTriggered = retVal;
    }

    App::worldContainer->calcInfo->visionSensorSimulationEnd(retVal);
    return (retVal);
}

bool CVisionSensor::detectEntity2(int entityID, bool detectAll,
                                  bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                  bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections)
{ // if entityID is -1, all detectable objects are rendered!
    TRACE_INTERNAL;
    bool retVal = false;
    std::vector<int> activeMirrors;

#ifdef SIM_WITH_GUI
    if (getInternalRendering())
    {
        int activeMirrorCnt =
            _getActiveMirrors(entityID, detectAll, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                              overrideRenderableFlagsForNonCollections, _attributesForRendering, activeMirrors);
        createGlContextAndFboAndTextureObjectIfNeeded(activeMirrorCnt > 0);
        _contextFboAndTexture->offscreenContext->makeCurrent();
        _contextFboAndTexture->frameBufferObject->switchToFbo();
    }
#endif

    if (!_useExternalImage)
        retVal = renderForDetection(entityID, detectAll, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                    hideEdgesIfModel, overrideRenderableFlagsForNonCollections, activeMirrors);

    if (getInternalRendering())
    {
#ifdef SIM_WITH_GUI
        if (!_useExternalImage)
        {
            if (!_ignoreRGBInfo)
            {
                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadPixels(0, 0, _resolution[0], _resolution[1], GL_RGB, GL_UNSIGNED_BYTE, _rgbBuffer);
                glPixelStorei(GL_PACK_ALIGNMENT, 4); // important to restore! Really?
                _emitImageChangedEvent();
            }
            if (!_ignoreDepthInfo)
            {
                glReadPixels(0, 0, _resolution[0], _resolution[1], GL_DEPTH_COMPONENT, GL_FLOAT, _depthBuffer);
                // Convert this depth info into values corresponding to linear depths (if perspective mode):
                if (_perspective)
                {
                    float farMinusNear = (float)(_farClippingPlane - _nearClippingPlane);
                    float farDivFarMinusNear = ((float)(_farClippingPlane)) / farMinusNear;
                    float nearTimesFar = (float)(_nearClippingPlane * _farClippingPlane);
                    int v = _resolution[0] * _resolution[1];
                    for (int i = 0; i < v; i++)
                        _depthBuffer[i] = ((nearTimesFar / (farMinusNear * (farDivFarMinusNear - _depthBuffer[i]))) -
                                           _nearClippingPlane) /
                                          farMinusNear;
                }
                _emitDepthChangedEvent();
            }
        }

        if (App::userSettings->useGlFinish_visionSensors) // false by default!
            glFinish();                                   // Might be important later (synchronization problems)
                        // removed on 2009/12/09 upon recomendation of gamedev community
                        // re-put on 2010/01/11 because it slows down some graphic cards in a non-proportional way (e.g.
                        // 1 object=x ms, 5 objects=20x ms) re-removed again (by default) on 31/01/2013. Thanks a lot to
                        // Cedric Pradalier for pointing problems appearing with the NVidia drivers

        _contextFboAndTexture->frameBufferObject->switchToNonFbo();
        _contextFboAndTexture->offscreenContext->doneCurrent();
#endif
    }
    else
        _extRenderer_retrieveImage();
    return (retVal);
}

bool CVisionSensor::_extRenderer_prepareView(int extRendererIndex)
{ // Set-up the resolution, clear color, camera properties and camera pose:
    bool retVal = App::worldContainer->pluginContainer->selectExtRenderer(extRendererIndex);

    void *data[30];
    _extWindowedViewSize[0] = _resolution[0];
    _extWindowedViewSize[1] = _resolution[1];

    data[0] = _extWindowedViewSize;     // for windowed views, this value is also a return value
    data[1] = _extWindowedViewSize + 1; // for windowed views, this value is also a return value
    if (_useSameBackgroundAsEnvironment)
        data[2] = App::currentWorld->environment->fogBackgroundColor;
    else
        data[2] = _defaultBufferValues;
    C7Vector tr(getFullCumulativeTransformation());
    float x[3] = {(float)tr.X(0), (float)tr.X(1), (float)tr.X(2)};
    data[3] = x;
    float q[4] = {(float)tr.Q(0), (float)tr.Q(1), (float)tr.Q(2), (float)tr.Q(3)};
    data[4] = q;
    int options = 0;
    float xAngle_size;
    float yAngle_size;
    float ratio = (float)(_resolution[0] / (float)_resolution[1]);
    if (_perspective)
    {
        if (ratio > 1.0)
        {
            xAngle_size = (float)_viewAngle;
            yAngle_size = 2.0f * atan(tan(_viewAngle / 2.0f) / ratio);
        }
        else
        {
            xAngle_size = 2.0f * atan(tan(_viewAngle / 2.0f) * ratio);
            yAngle_size = (float)_viewAngle;
        }
    }
    else
    {
        options |= 1;
        if (ratio > 1.0)
        {
            xAngle_size = (float)_orthoViewSize;
            yAngle_size = (float)_orthoViewSize / ratio;
        }
        else
        {
            xAngle_size = (float)_orthoViewSize * ratio;
            yAngle_size = (float)_orthoViewSize;
        }
    }
    data[5] = &options;
    data[6] = &xAngle_size;
    data[7] = &yAngle_size;
    float va = (float)_viewAngle;
    data[8] = &va;
    float nc = (float)_nearClippingPlane;
    data[9] = &nc;
    float fc = (float)_farClippingPlane;
    data[10] = &fc;
    data[11] = App::currentWorld->environment->ambientLightColor;
    data[12] = App::currentWorld->environment->fogBackgroundColor;
    int fogType = App::currentWorld->environment->getFogType();
    double fogStart = (float)App::currentWorld->environment->getFogStart();
    double fogEnd = (float)App::currentWorld->environment->getFogEnd();
    double fogDensity = (float)App::currentWorld->environment->getFogDensity();
    bool fogEnabled = App::currentWorld->environment->getFogEnabled();
    data[13] = &fogType;
    data[14] = &fogStart;
    data[15] = &fogEnd;
    data[16] = &fogDensity;
    data[17] = &fogEnabled;
    float ovs = (float)_orthoViewSize;
    data[18] = &ovs;
    data[19] = &_objectHandle;
    data[20] = _extWindowedViewPos;
    data[21] = _extWindowedViewPos + 1;

    // Following actually free since CoppeliaSim 3.3.0
    // But the older PovRay plugin version crash without this:
    float povFogDist = 4.0f;
    float povFogTransp = 0.5f;
    bool povFocalBlurEnabled = false;
    float povFocalLength = 0.0f;
    float povAperture = 0.0f;
    int povBlurSamples = 1;
    data[22] = &povFogDist;
    data[23] = &povFogTransp;
    data[24] = &povFocalBlurEnabled;
    data[25] = &povFocalLength;
    data[26] = &povAperture;
    data[27] = &povBlurSamples;

#ifdef SIM_WITH_GUI
#ifdef USES_QGLWIDGET
    QGLWidget *otherWidgetToShareResourcesWith = nullptr;
#else
    QOpenGLWidget *otherWidgetToShareResourcesWith = nullptr;
#endif
    if (GuiApp::mainWindow != nullptr)
        data[28] = GuiApp::mainWindow->openglWidget;
#endif

    App::worldContainer->pluginContainer->extRenderer(sim_message_eventcallback_extrenderer_start, data);
    return (retVal);
}

void CVisionSensor::_extRenderer_prepareLights()
{ // Set-up the lights:
    for (size_t li = 0; li < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_light); li++)
    {
        CLight *light = App::currentWorld->sceneObjects->getLightFromIndex(li);
        if (light->getLightActive())
        {
            void *data[20];
            int lightType = light->getLightType();
            data[0] = &lightType;
            float cutoffAngle = (float)light->getSpotCutoffAngle();
            data[1] = &cutoffAngle;
            int spotExponent = light->getSpotExponent();
            data[2] = &spotExponent;
            data[3] = light->getColor(true)->getColorsPtr();
            float constAttenuation = (float)light->getAttenuationFactor(CONSTANT_ATTENUATION);
            data[4] = &constAttenuation;
            float linAttenuation = (float)light->getAttenuationFactor(LINEAR_ATTENUATION);
            data[5] = &linAttenuation;
            float quadAttenuation = (float)light->getAttenuationFactor(QUADRATIC_ATTENUATION);
            data[6] = &quadAttenuation;
            C7Vector tr(light->getFullCumulativeTransformation());
            float x[3] = {(float)tr.X(0), (float)tr.X(1), (float)tr.X(2)};
            data[7] = x;
            float q[4] = {(float)tr.Q(0), (float)tr.Q(1), (float)tr.Q(2), (float)tr.Q(3)};
            data[8] = q;
            float lightSize = (float)light->getLightSize();
            data[9] = &lightSize;
            bool lightIsVisible = light->getShouldObjectBeDisplayed(_objectHandle, 0);
            data[11] = &lightIsVisible;
            int lightHandle = light->getObjectHandle();
            data[13] = &lightHandle;

            // Following actually free since CoppeliaSim 3.3.0
            // But the older PovRay plugin version crash without this:
            float povFadeXDist = 0.0f;
            bool povNoShadow = false;
            data[10] = &povFadeXDist;
            data[12] = &povNoShadow;

            App::worldContainer->pluginContainer->extRenderer(sim_message_eventcallback_extrenderer_light, data);
        }
    }
}

void CVisionSensor::_extRenderer_retrieveImage()
{ // Fetch the finished image:
    void *data[20];
    data[0] = _rgbBuffer;
    data[1] = _depthBuffer;
    bool readRgb = !_ignoreRGBInfo;
    data[2] = &readRgb;
    bool readDepth = !_ignoreDepthInfo;
    data[3] = &readDepth;
    App::worldContainer->pluginContainer->extRenderer(sim_message_eventcallback_extrenderer_stop, data);
    if (!_ignoreRGBInfo)
        _emitImageChangedEvent();
    if (!_ignoreDepthInfo)
        _emitDepthChangedEvent();
}

bool CVisionSensor::renderForDetection(int entityID, bool detectAll,
                                       bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                       bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections,
                                       const std::vector<int> &activeMirrors)
{ // if entityID==-1, all objects that can be detected are rendered.
    TRACE_INTERNAL;
    bool retVal = true;
    _currentPerspective = _perspective;

    if (getInternalRendering())
    {
#ifdef SIM_WITH_GUI
        int currentWinSize[2] = {_resolution[0], _resolution[1]};
        glViewport(0, 0, _resolution[0], _resolution[1]);

        if (_renderMode != sim_rendermode_colorcoded)
        {
            if (_useSameBackgroundAsEnvironment)
                glClearColor(App::currentWorld->environment->fogBackgroundColor[0],
                             App::currentWorld->environment->fogBackgroundColor[1],
                             App::currentWorld->environment->fogBackgroundColor[2], 0.0);
            else
                glClearColor(_defaultBufferValues[0], _defaultBufferValues[1], _defaultBufferValues[2], 0.0);
        }
        else
            glClearColor(1.0, 1.0, 1.0, 0.0); // for color coding we need a clear color perfectly white

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glRenderMode(GL_RENDER);

        double ratio = (double)(currentWinSize[0] / (double)currentWinSize[1]);
        if (_perspective)
        {
            if (ratio > 1.0)
            {
                double a = 2.0 * atan(tan(_viewAngle / 2.0) / ratio) * radToDeg;
                ogl::perspectiveSpecial(a, ratio, _nearClippingPlane, _farClippingPlane);
            }
            else
                ogl::perspectiveSpecial(_viewAngle * radToDeg, ratio, _nearClippingPlane, _farClippingPlane);
        }
        else
        {
            if (ratio > 1.0)
                glOrtho(-_orthoViewSize * 0.5, _orthoViewSize * 0.5, -_orthoViewSize * 0.5 / ratio,
                        _orthoViewSize * 0.5 / ratio, _nearClippingPlane, _farClippingPlane);
            else
                glOrtho(-_orthoViewSize * 0.5 * ratio, _orthoViewSize * 0.5 * ratio, -_orthoViewSize * 0.5,
                        _orthoViewSize * 0.5, _nearClippingPlane, _farClippingPlane);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        C4X4Matrix m4(getFullCumulativeTransformation().getMatrix());
        // The following 6 instructions have the same effect as gluLookAt()
        m4.inverse();
        m4.rotateAroundY(piValue);
        CMatrix m4_(m4);
        m4_.transpose();
        glLoadMatrixd(m4_.data.data());

        if (_renderMode == sim_rendermode_opengl)
        { // visible
            App::currentWorld->environment->activateAmbientLight(true);
            App::currentWorld->environment->activateFogIfEnabled(this, false);
            _activateNonAmbientLights(-1, this);
        }
        else
        {
            App::currentWorld->environment->activateAmbientLight(false);
            App::currentWorld->environment->deactivateFog();
            _activateNonAmbientLights(-2, nullptr);
        }

        glInitNames();
        glPushName(-1);
        glLoadName(-1);

        glShadeModel(GL_SMOOTH);

        if (_renderMode != sim_rendermode_colorcoded)
        { // visible & aux channels
            glEnable(GL_DITHER);
        }
        else
        {
            glDisable(GL_DITHER);
            ogl::disableLighting_useWithCare(); // only temporarily
        }

        _handleMirrors(activeMirrors, entityID, detectAll,
                       entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, hideEdgesIfModel,
                       overrideRenderableFlagsForNonCollections);
#endif
    }
    else
    {
        retVal = _extRenderer_prepareView(_renderMode - sim_rendermode_povray);
        _extRenderer_prepareLights();
    }

    // Set data for view frustum culling
    _planesCalculated = false;
    _currentViewSize[0] = _resolution[0];
    _currentViewSize[1] = _resolution[1];

    if ((_renderMode == sim_rendermode_povray) || (_renderMode == sim_rendermode_opengl3))
        setFrustumCullingTemporarilyDisabled(true); // important with ray-tracers and similar

    // Draw objects:
    _drawObjects(entityID, detectAll, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, hideEdgesIfModel,
                 overrideRenderableFlagsForNonCollections);

    if ((_renderMode == sim_rendermode_povray) || (_renderMode == sim_rendermode_opengl3))
        setFrustumCullingTemporarilyDisabled(false); // important with ray-tracers and similar

#ifdef SIM_WITH_GUI
    if (getInternalRendering())
    {
        if (_renderMode == sim_rendermode_colorcoded)
        { // reset to default
            ogl::enableLighting_useWithCare();
            glEnable(GL_DITHER);
        }

        App::currentWorld->environment->deactivateFog();
        glRenderMode(GL_RENDER);
    }
#endif
    return (retVal);
}

void CVisionSensor::_drawObjects(int entityID, bool detectAll,
                                 bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                 bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections)
{ // if entityID==-1, all objects that can be detected are rendered.
    TRACE_INTERNAL;
    // Very unelegant routine. Needs badly refactoring!

    int rendAttrib = _attributesForRendering;
    if (_renderMode == sim_rendermode_colorcoded)
        rendAttrib |= sim_displayattribute_colorcoded;
    if (_renderMode == sim_rendermode_auxchannels)
        rendAttrib |= sim_displayattribute_useauxcomponent;

    std::vector<CSceneObject *> toRender;
    CSceneObject *viewBoxObject;
    if (App::userSettings->enableOldRenderableBehaviour)
        viewBoxObject = _getInfoOfWhatNeedsToBeRendered_old(
            entityID, detectAll, rendAttrib, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
            overrideRenderableFlagsForNonCollections, toRender);
    else
        viewBoxObject = _getInfoOfWhatNeedsToBeRendered(entityID, detectAll, rendAttrib,
                                                        entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                                        overrideRenderableFlagsForNonCollections, toRender);

    //************ Viewbox thing ***************
    if (viewBoxObject != nullptr)
    { // we set the same position as the camera, but we keep the initial orientation
        // If the camera is in ortho view mode, we additionally shift it along the viewing axis
        // to be sure we don't cover anything visible with the far side of the box (the near side is clipped by model
        // settings)
        C4Vector rel(viewBoxObject->getLocalTransformation().Q);
        C7Vector cam(getFullCumulativeTransformation());
        if (!_currentPerspective)
        { // This doesn't work!!
            C3Vector minV(C3Vector::inf);
            C3Vector maxV(C3Vector::ninf);
            viewBoxObject->getModelBB((getCumulativeTransformation() * getBB(nullptr)).getInverse(), minV, maxV, true);
            double shift = _farClippingPlane - 0.505 * (maxV(2) - minV(2)); // just a bit more than half!
            cam.X += cam.Q.getMatrix().axis[2] * shift;
        }
        C7Vector newLocal(viewBoxObject->getFullParentCumulativeTransformation().getInverse() * cam);
        newLocal.Q = rel;
        viewBoxObject->setLocalTransformation(newLocal);
    }
    //***************************************

#ifdef SIM_WITH_GUI
    if (getInternalRendering())
    {
        _prepareAuxClippingPlanes();
        _enableAuxClippingPlanes(-1);
    }
    if (getInternalRendering()) // for now
    {
        // first non-transparent attached drawing objects:
        for (int i = 0; i < int(toRender.size()); i++)
            App::currentWorld->drawingCont->drawObjectsParentedWith(false, false, toRender[i]->getObjectHandle(),
                                                                    rendAttrib,
                                                                    getFullCumulativeTransformation().getMatrix());

        // Now the same as above but for non-attached drawing objects:
        App::currentWorld->drawingCont->drawObjectsParentedWith(false, false, -1, rendAttrib,
                                                                getFullCumulativeTransformation().getMatrix());

        // Point clouds:
        App::currentWorld->pointCloudCont_old->renderYour3DStuff_nonTransparent(this, rendAttrib);

        // Ghost objects:
        App::currentWorld->ghostObjectCont_old->renderYour3DStuff_nonTransparent(this, rendAttrib);

        // particles:
        App::currentWorld->dynamicsContainer->renderYour3DStuff(this, rendAttrib);
    }
    if (getInternalRendering())
        _disableAuxClippingPlanes();
#endif

    // Rendering the scene objects:
    for (int i = 0; i < int(toRender.size()); i++)
    {
        toRender[i]->setForceAlwaysVisible_tmp(
            true); // We have already decided to render this based on vision sensor's settings!
        if (!entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects)
        { // attached non-transparent objects were rendered before
            if (getInternalRendering())
            {
#ifdef SIM_WITH_GUI
                toRender[i]->display(this, rendAttrib);
#endif
            }
            else
            {
                if (toRender[i]->getObjectType() == sim_sceneobject_shape)
                    ((CShape *)toRender[i])->display_extRenderer(this, rendAttrib);
            }
        }
#ifdef SIM_WITH_GUI
        else
        { // we render the scene twice when we have edges, since antialiasing might not be beautiful otherwise
            int atr = sim_displayattribute_renderpass;
            toRender[i]->display(
                this, atr | sim_displayattribute_forbidedges); // attached non-transparent objects were rendered before
            if (!hideEdgesIfModel)
                toRender[i]->display(this, atr); // attached object were rendered before
        }
#endif
        toRender[i]->setForceAlwaysVisible_tmp(false);
    }

#ifdef SIM_WITH_GUI
    if (getInternalRendering())
        _enableAuxClippingPlanes(-1);
    if (getInternalRendering()) // for now
    {
        // Transparent attached drawing objects:
        for (int i = 0; i < int(toRender.size()); i++)
            App::currentWorld->drawingCont->drawObjectsParentedWith(
                false, true, toRender[i]->getObjectHandle(), rendAttrib, getFullCumulativeTransformation().getMatrix());

        // Now the same as above but for non-attached drawing objects:
        App::currentWorld->drawingCont->drawObjectsParentedWith(false, true, -1, rendAttrib,
                                                                getFullCumulativeTransformation().getMatrix());

        // Ghost objects:
        App::currentWorld->ghostObjectCont_old->renderYour3DStuff_transparent(this, rendAttrib);
    }
    if (getInternalRendering()) // for now
    {
        // overlay attached drawing objects:
        for (int i = 0; i < int(toRender.size()); i++)
            App::currentWorld->drawingCont->drawObjectsParentedWith(
                true, true, toRender[i]->getObjectHandle(), rendAttrib, getFullCumulativeTransformation().getMatrix());

        // Now the same as above but for non-attached drawing objects:
        App::currentWorld->drawingCont->drawObjectsParentedWith(true, true, -1, rendAttrib,
                                                                getFullCumulativeTransformation().getMatrix());

        // Ghosts:
        App::currentWorld->ghostObjectCont_old->renderYour3DStuff_overlay(this, rendAttrib);
    }
    if (getInternalRendering())
        _disableAuxClippingPlanes();
#endif
}

CSceneObject *CVisionSensor::_getInfoOfWhatNeedsToBeRendered(
    int entityID, bool detectAll, int rendAttrib, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
    bool overrideRenderableFlagsForNonCollections, std::vector<CSceneObject *> &toRender)
{
    CSceneObject *object = App::currentWorld->sceneObjects->getObjectFromHandle(entityID);
    CCollection *collection = nullptr;
    std::vector<int> transparentObjects;
    std::vector<double> transparentObjectsDist;
    C7Vector camTrInv(getCumulativeTransformation().getInverse());
    CSceneObject *viewBoxObject = nullptr;

    if (object == nullptr)
    {
        collection = App::currentWorld->collections->getObjectFromHandle(entityID);
        if (collection != nullptr)
        {
            bool overridePropertyFlag = collection->getOverridesObjectMainProperties();
            for (size_t i = 0; i < collection->getSceneObjectCountInCollection(); i++)
            {
                CSceneObject *it =
                    App::currentWorld->sceneObjects->getObjectFromHandle(collection->getSceneObjectHandleFromIndex(i));
                if (((it->getObjectType() == sim_sceneobject_shape) ||
                     (it->getObjectType() == sim_sceneobject_octree) ||
                     (it->getObjectType() == sim_sceneobject_pointcloud) ||
                     (it->getObjectType() == sim_sceneobject_path)) &&
                    (overridePropertyFlag || it->isObjectVisible()))
                {
                    if (it->getObjectType() == sim_sceneobject_shape)
                    {
                        CShape *sh = (CShape *)it;
                        if (sh->getContainsTransparentComponent())
                        {
                            C7Vector obj(it->getCumulativeTransformation());
                            transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                             it->getTransparentObjectDistanceOffset());
                            transparentObjects.push_back(it->getObjectHandle());
                        }
                        else
                            toRender.push_back(it);
                    }
                    else
                        toRender.push_back(it);
                    if (it->getParent() != nullptr)
                    { // We need this because the dummy that is the base of the skybox is not renderable!
                        if (it->getParent()->getObjectName_old() == IDSOGL_SKYBOX_DO_NOT_RENAME)
                            viewBoxObject = it->getParent();
                    }
                }
            }
        }
        else
        { // Here we want to detect all visible objects:
            if (detectAll)
            {
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                {
                    CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
                    if (((it->getObjectType() == sim_sceneobject_shape) ||
                         (it->getObjectType() == sim_sceneobject_octree) ||
                         (it->getObjectType() == sim_sceneobject_pointcloud) ||
                         (it->getObjectType() == sim_sceneobject_path)) &&
                        it->isObjectVisible())
                    {
                        if (it->getObjectType() == sim_sceneobject_shape)
                        {
                            CShape *sh = (CShape *)it;
                            if (sh->getContainsTransparentComponent())
                            {
                                C7Vector obj(it->getCumulativeTransformation());
                                transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                 it->getTransparentObjectDistanceOffset());
                                transparentObjects.push_back(it->getObjectHandle());
                            }
                            else
                                toRender.push_back(it);
                        }
                        else
                            toRender.push_back(it);
                        if (it->getParent() != nullptr)
                        { // We need this because the dummy that is the base of the skybox is not renderable!
                            if (it->getParent()->getObjectName_old() == IDSOGL_SKYBOX_DO_NOT_RENAME)
                                viewBoxObject = it->getParent();
                        }
                    }
                }
            }
        }
    }
    else
    { // We want to detect a single object (no collection not all objects in the scene)
        if (!entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects)
        { // normal for a single object. We always render it!
            toRender.push_back(object);
            if (object->getParent() != nullptr)
            { // We need this because the dummy that is the base of the skybox is not renderable!
                if (object->getParent()->getObjectName_old() == IDSOGL_SKYBOX_DO_NOT_RENAME)
                    viewBoxObject = object->getParent();
            }
        }
        else
        { // we have a model here that we want to render. We render also non-renderable object. And only those currently
          // visible:
            std::vector<int> rootSel;
            rootSel.push_back(object->getObjectHandle());
            App::currentWorld->sceneObjects->addModelObjects(rootSel);
            for (int i = 0; i < int(rootSel.size()); i++)
            {
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(rootSel[i]);
                if (App::currentWorld->environment->getActiveLayers() & it->getVisibilityLayer())
                { // ok, currently visible
                    if (it->getObjectType() == sim_sceneobject_shape)
                    {
                        CShape *sh = (CShape *)it;
                        if (sh->getContainsTransparentComponent())
                        {
                            C7Vector obj(it->getCumulativeTransformation());
                            transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                             it->getTransparentObjectDistanceOffset());
                            transparentObjects.push_back(it->getObjectHandle());
                        }
                        else
                            toRender.push_back(it);
                    }
                    else
                    {
                        if (it->getObjectType() == sim_sceneobject_mirror)
                        {
                            CMirror *mir = (CMirror *)it;
                            if (mir->getContainsTransparentComponent())
                            {
                                C7Vector obj(it->getCumulativeTransformation());
                                transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                 it->getTransparentObjectDistanceOffset());
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

    tt::orderAscending(transparentObjectsDist, transparentObjects);
    for (int i = 0; i < int(transparentObjects.size()); i++)
        toRender.push_back(App::currentWorld->sceneObjects->getObjectFromHandle(transparentObjects[i]));

    return (viewBoxObject);
}

CSceneObject *CVisionSensor::_getInfoOfWhatNeedsToBeRendered_old(
    int entityID, bool detectAll, int rendAttrib, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
    bool overrideRenderableFlagsForNonCollections, std::vector<CSceneObject *> &toRender)
{
    CSceneObject *object = App::currentWorld->sceneObjects->getObjectFromHandle(entityID);
    CCollection *collection = nullptr;
    std::vector<int> transparentObjects;
    std::vector<double> transparentObjectsDist;
    C7Vector camTrInv(getCumulativeTransformation().getInverse());
    CSceneObject *viewBoxObject = nullptr;

    if (object == nullptr)
    {
        collection = App::currentWorld->collections->getObjectFromHandle(entityID);
        if (collection != nullptr)
        {
            bool overridePropertyFlag = collection->getOverridesObjectMainProperties();
            for (size_t i = 0; i < collection->getSceneObjectCountInCollection(); i++)
            {
                CSceneObject *it =
                    App::currentWorld->sceneObjects->getObjectFromHandle(collection->getSceneObjectHandleFromIndex(i));
                if (it != nullptr)
                {
                    if (((it->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_renderable) != 0) ||
                        overridePropertyFlag || (rendAttrib & sim_displayattribute_ignorerenderableflag))
                    { // supposed to be rendered
                        if (it->getObjectType() == sim_sceneobject_shape)
                        {
                            CShape *sh = (CShape *)it;
                            if (sh->getContainsTransparentComponent())
                            {
                                C7Vector obj(it->getCumulativeTransformation());
                                transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                 it->getTransparentObjectDistanceOffset());
                                transparentObjects.push_back(it->getObjectHandle());
                            }
                            else
                                toRender.push_back(it);
                        }
                        else
                        {
                            if (it->getObjectType() == sim_sceneobject_mirror)
                            {
                                CMirror *mir = (CMirror *)it;
                                if (mir->getContainsTransparentComponent())
                                {
                                    C7Vector obj(it->getCumulativeTransformation());
                                    transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                     it->getTransparentObjectDistanceOffset());
                                    transparentObjects.push_back(it->getObjectHandle());
                                }
                                else
                                    toRender.push_back(it);
                            }
                            else
                                toRender.push_back(it);
                        }
                        if (it->getParent() != nullptr)
                        { // We need this because the dummy that is the base of the skybox is not renderable!
                            if (it->getParent()->getObjectName_old() == IDSOGL_SKYBOX_DO_NOT_RENAME)
                                viewBoxObject = it->getParent();
                        }
                    }
                }
            }
        }
        else
        { // Here we want to detect all detectable objects maybe:
            if (detectAll)
            {
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                {
                    CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
                    if (it != nullptr)
                    {
                        if (((it->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_renderable) != 0) ||
                            overrideRenderableFlagsForNonCollections ||
                            (rendAttrib & sim_displayattribute_ignorerenderableflag))
                        { // supposed to be rendered
                            if (it->getObjectType() == sim_sceneobject_shape)
                            {
                                CShape *sh = (CShape *)it;
                                if (sh->getContainsTransparentComponent())
                                {
                                    C7Vector obj(it->getCumulativeTransformation());
                                    transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                     it->getTransparentObjectDistanceOffset());
                                    transparentObjects.push_back(it->getObjectHandle());
                                }
                                else
                                    toRender.push_back(it);
                            }
                            else
                            {
                                if (it->getObjectType() == sim_sceneobject_mirror)
                                {
                                    CMirror *mir = (CMirror *)it;
                                    if (mir->getContainsTransparentComponent())
                                    {
                                        C7Vector obj(it->getCumulativeTransformation());
                                        transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                         it->getTransparentObjectDistanceOffset());
                                        transparentObjects.push_back(it->getObjectHandle());
                                    }
                                    else
                                        toRender.push_back(it);
                                }
                                else
                                    toRender.push_back(it);
                            }
                            if (it->getParent() != nullptr)
                            { // We need this because the dummy that is the base of the skybox is not renderable!
                                if (it->getParent()->getObjectName_old() == IDSOGL_SKYBOX_DO_NOT_RENAME)
                                    viewBoxObject = it->getParent();
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
            if (((object->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_renderable) != 0) ||
                overrideRenderableFlagsForNonCollections || (rendAttrib & sim_displayattribute_ignorerenderableflag))
            {
                toRender.push_back(object);
                if (object->getParent() != nullptr)
                { // We need this because the dummy that is the base of the skybox is not renderable!
                    if (object->getParent()->getObjectName_old() == IDSOGL_SKYBOX_DO_NOT_RENAME)
                        viewBoxObject = object->getParent();
                }
            }
        }
        else
        { // we have a model here that we want to render. We render also non-renderable object. And only those currently
          // visible:
            std::vector<int> rootSel;
            rootSel.push_back(object->getObjectHandle());
            App::currentWorld->sceneObjects->addModelObjects(rootSel);
            for (int i = 0; i < int(rootSel.size()); i++)
            {
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(rootSel[i]);
                if (App::currentWorld->environment->getActiveLayers() & it->getVisibilityLayer())
                { // ok, currently visible
                    if (it->getObjectType() == sim_sceneobject_shape)
                    {
                        CShape *sh = (CShape *)it;
                        if (sh->getContainsTransparentComponent())
                        {
                            C7Vector obj(it->getCumulativeTransformation());
                            transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                             it->getTransparentObjectDistanceOffset());
                            transparentObjects.push_back(it->getObjectHandle());
                        }
                        else
                            toRender.push_back(it);
                    }
                    else
                    {
                        if (it->getObjectType() == sim_sceneobject_mirror)
                        {
                            CMirror *mir = (CMirror *)it;
                            if (mir->getContainsTransparentComponent())
                            {
                                C7Vector obj(it->getCumulativeTransformation());
                                transparentObjectsDist.push_back(-(camTrInv * obj).X(2) -
                                                                 it->getTransparentObjectDistanceOffset());
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

    tt::orderAscending(transparentObjectsDist, transparentObjects);
    for (int i = 0; i < int(transparentObjects.size()); i++)
        toRender.push_back(App::currentWorld->sceneObjects->getObjectFromHandle(transparentObjects[i]));

    return (viewBoxObject);
}

int CVisionSensor::_getActiveMirrors(int entityID, bool detectAll,
                                     bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                     bool overrideRenderableFlagsForNonCollections, int rendAttrib,
                                     std::vector<int> &activeMirrors)
{
    if (App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_mirror) == 0)
        return (0);
    if (App::currentWorld->mainSettings_old->mirrorsDisabled)
        return (0);
    if (_renderMode != sim_rendermode_opengl)
        return (0);

    CSceneObject *object = App::currentWorld->sceneObjects->getObjectFromHandle(entityID);
    CCollection *collection = nullptr;
    std::vector<CSceneObject *> toRender;
    if (object == nullptr)
    {
        collection = App::currentWorld->collections->getObjectFromHandle(entityID);
        if (collection != nullptr)
        {
            bool overridePropertyFlag = collection->getOverridesObjectMainProperties();
            for (size_t i = 0; i < collection->getSceneObjectCountInCollection(); i++)
            {
                CSceneObject *it =
                    App::currentWorld->sceneObjects->getObjectFromHandle(collection->getSceneObjectHandleFromIndex(i));
                if (it != nullptr)
                {
                    if (((it->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_renderable) != 0) ||
                        overridePropertyFlag || (rendAttrib & sim_displayattribute_ignorerenderableflag))
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
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
                {
                    CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
                    if (it != nullptr)
                    {
                        if (((it->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_renderable) != 0) ||
                            overrideRenderableFlagsForNonCollections ||
                            (rendAttrib & sim_displayattribute_ignorerenderableflag))
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
            if (((object->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_renderable) != 0) ||
                overrideRenderableFlagsForNonCollections || (rendAttrib & sim_displayattribute_ignorerenderableflag))
            {
                toRender.push_back(object);
            }
        }
        else
        { // we have a model here that we want to render. We render also non-renderable object. And only those currently
          // visible:
            std::vector<int> rootSel;
            rootSel.push_back(object->getObjectHandle());
            App::currentWorld->sceneObjects->addModelObjects(rootSel);
            for (int i = 0; i < int(rootSel.size()); i++)
            {
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(rootSel[i]);
                if (App::currentWorld->environment->getActiveLayers() & it->getVisibilityLayer())
                { // ok, currently visible
                    toRender.push_back(it);
                }
            }
        }
    }
    int retVal = 0;
    for (int i = 0; i < int(toRender.size()); i++)
    {
        CSceneObject *it = toRender[i];
        if (it->getObjectType() == sim_sceneobject_mirror)
        {
            CMirror *mi = (CMirror *)it;
            if (mi->getActive() && mi->getIsMirror())
            {
                activeMirrors.push_back(it->getObjectHandle());
                retVal++;
            }
        }
    }
    return (retVal);
}

void CVisionSensor::scaleObject(double scalingFactor)
{
    setClippingPlanes(_nearClippingPlane * scalingFactor, _farClippingPlane * scalingFactor);
    setOrthoViewSize(_orthoViewSize * scalingFactor);
    setVisionSensorSize(_visionSensorSize * scalingFactor);

    CSceneObject::scaleObject(scalingFactor);
}

void CVisionSensor::removeSceneDependencies()
{
    CSceneObject::removeSceneDependencies();
    _detectableEntityHandle = -1;
}

void CVisionSensor::addSpecializedObjectEventData(CCbor *ev)
{
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->openKeyMap(getObjectTypeInfo().c_str());
    ev->appendKeyBool("perspectiveMode", _perspective);
    ev->appendKeyDouble("nearClippingPlane", _nearClippingPlane);
    ev->appendKeyDouble("farClippingPlane", _farClippingPlane);
    ev->appendKeyDouble("orthoSize", _orthoViewSize);
    ev->openKeyMap("frustumVectors");
    ev->appendKeyDoubleArray("near", _volumeVectorNear.data, 3);
    ev->appendKeyDoubleArray("far", _volumeVectorFar.data, 3);
    ev->closeArrayOrMap(); // frustumVectors
#else
    color.addGenesisEventData(ev);
#endif
    ev->appendKeyDouble(propVisionSensor_size.name, _visionSensorSize);
    ev->appendKeyFloatArray(propVisionSensor_backgroundCol.name, _defaultBufferValues, 3);
    ev->appendKeyInt(propVisionSensor_renderMode.name, _renderMode);
    ev->appendKeyBool(propVisionSensor_backgroundSameAsEnv.name, _useSameBackgroundAsEnvironment);
    ev->appendKeyBool(propVisionSensor_explicitHandling.name, _explicitHandling);
    ev->appendKeyBool(propVisionSensor_useExtImage.name, _useExternalImage);
    ev->appendKeyBool(propVisionSensor_ignoreRgbInfo.name, _ignoreRGBInfo);
    ev->appendKeyBool(propVisionSensor_ignoreDepthInfo.name, _ignoreDepthInfo);
    ev->appendKeyBool(propVisionSensor_omitPacket1.name, !_computeImageBasicStats);
    ev->appendKeyBool(propVisionSensor_emitImageChangedEvent.name, _emitImageChangedEventEnabled);
    ev->appendKeyBool(propVisionSensor_emitDepthChangedEvent.name, _emitDepthChangedEventEnabled);
    _emitImageChangedEvent();
    _emitDepthChangedEvent();
    CViewableBase::addSpecializedObjectEventData(ev);
#if SIM_EVENT_PROTOCOL_VERSION == 2
    ev->closeArrayOrMap(); // visionSensor
#endif
}

CSceneObject *CVisionSensor::copyYourself()
{
    CVisionSensor *newVisionSensor = (CVisionSensor *)CSceneObject::copyYourself();

    newVisionSensor->_viewAngle = _viewAngle;
    newVisionSensor->_orthoViewSize = _orthoViewSize;
    newVisionSensor->_nearClippingPlane = _nearClippingPlane;
    newVisionSensor->_farClippingPlane = _farClippingPlane;
    newVisionSensor->_showFogIfAvailable = _showFogIfAvailable;
    newVisionSensor->_useLocalLights = _useLocalLights;

    newVisionSensor->_resolution[0] = _resolution[0];
    newVisionSensor->_resolution[1] = _resolution[1];
    for (int i = 0; i < 3; i++) // Important to do it before setting the sensor type
        newVisionSensor->_defaultBufferValues[i] = _defaultBufferValues[i];
    newVisionSensor->_perspective = _perspective;
    newVisionSensor->_volumeVectorNear = _volumeVectorNear;
    newVisionSensor->_volumeVectorFar = _volumeVectorFar;
    newVisionSensor->_visionSensorSize = _visionSensorSize;
    newVisionSensor->_detectableEntityHandle = _detectableEntityHandle;
    newVisionSensor->_useExternalImage = _useExternalImage;
    newVisionSensor->_useSameBackgroundAsEnvironment = _useSameBackgroundAsEnvironment;

    newVisionSensor->_explicitHandling = _explicitHandling;
    newVisionSensor->_showVolume = _showVolume;

    newVisionSensor->_ignoreRGBInfo = _ignoreRGBInfo;
    newVisionSensor->_ignoreDepthInfo = _ignoreDepthInfo;
    newVisionSensor->_computeImageBasicStats = _computeImageBasicStats;
    newVisionSensor->_renderMode = _renderMode;
    newVisionSensor->_attributesForRendering = _attributesForRendering;

    for (int i = 0; i < 3; i++)
    {
        newVisionSensor->sensorResult.sensorDataRed[i] = 0;
        newVisionSensor->sensorResult.sensorDataGreen[i] = 0;
        newVisionSensor->sensorResult.sensorDataBlue[i] = 0;
        newVisionSensor->sensorResult.sensorDataIntensity[i] = 0;
        newVisionSensor->sensorResult.sensorDataDepth[i] = 0.0;
    }
    newVisionSensor->sensorAuxiliaryResult.assign(sensorAuxiliaryResult.begin(), sensorAuxiliaryResult.end());
    newVisionSensor->sensorResult.sensorWasTriggered = false;
    newVisionSensor->sensorResult.sensorResultIsValid = false;

    color.copyYourselfInto(&newVisionSensor->color);
    newVisionSensor->_reserveBuffers(); // important!

    newVisionSensor->_initialValuesInitialized = _initialValuesInitialized;
    newVisionSensor->_initialExplicitHandling = _initialExplicitHandling;

    return (newVisionSensor);
}

void CVisionSensor::announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if (_detectableEntityHandle == object->getObjectHandle())
        _detectableEntityHandle = -1;
    CSceneObject::announceObjectWillBeErased(object, copyBuffer);
}

void CVisionSensor::announceCollectionWillBeErased(int groupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    if (_detectableEntityHandle == groupID)
        _detectableEntityHandle = -1;
    CSceneObject::announceCollectionWillBeErased(groupID, copyBuffer);
}
void CVisionSensor::announceCollisionWillBeErased(int collisionID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceCollisionWillBeErased(collisionID, copyBuffer);
}
void CVisionSensor::announceDistanceWillBeErased(int distanceID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceDistanceWillBeErased(distanceID, copyBuffer);
}
void CVisionSensor::announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer)
{ // copyBuffer is false by default (if true, we are 'talking' to objects
    // in the copyBuffer)
    CSceneObject::announceIkObjectWillBeErased(ikGroupID, copyBuffer);
}

void CVisionSensor::performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performObjectLoadingMapping(map, loadingAmodel);
    if (_detectableEntityHandle <= SIM_IDEND_SCENEOBJECT)
        _detectableEntityHandle = CWorld::getLoadingMapping(map, _detectableEntityHandle);
}
void CVisionSensor::performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollectionLoadingMapping(map, loadingAmodel);
    if (_detectableEntityHandle > SIM_IDEND_SCENEOBJECT)
        _detectableEntityHandle = CWorld::getLoadingMapping(map, _detectableEntityHandle);
}
void CVisionSensor::performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performCollisionLoadingMapping(map, loadingAmodel);
}
void CVisionSensor::performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performDistanceLoadingMapping(map, loadingAmodel);
}
void CVisionSensor::performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel)
{
    CSceneObject::performIkLoadingMapping(map, loadingAmodel);
}

void CVisionSensor::performTextureObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performTextureObjectLoadingMapping(map);
}
void CVisionSensor::performDynMaterialObjectLoadingMapping(const std::map<int, int> *map)
{
    CSceneObject::performDynMaterialObjectLoadingMapping(map);
}

void CVisionSensor::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    CSceneObject::initializeInitialValues(simulationAlreadyRunning);
    for (int i = 0; i < 3; i++)
    {
        sensorResult.sensorDataRed[i] = 0;
        sensorResult.sensorDataGreen[i] = 0;
        sensorResult.sensorDataBlue[i] = 0;
        sensorResult.sensorDataIntensity[i] = 0;
        sensorResult.sensorDataDepth[i] = 0.0;
    }
    sensorAuxiliaryResult.clear();
    sensorResult.sensorWasTriggered = false;
    sensorResult.sensorResultIsValid = false;
    sensorResult.calcTimeInMs = 0;
    _initialExplicitHandling = _explicitHandling;
}

void CVisionSensor::simulationAboutToStart()
{
    initializeInitialValues(false);
    CSceneObject::simulationAboutToStart();
}

void CVisionSensor::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
  // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        if (App::currentWorld->simulation->getResetSceneAtSimulationEnd() && ((getCumulativeModelProperty() & sim_modelproperty_not_reset) == 0))
            setExplicitHandling(_initialExplicitHandling);
    }
    CSceneObject::simulationEnded();
}

bool CVisionSensor::_computeDefaultReturnValuesAndApplyFilters()
{
    bool trigger = false;
    if (_inApplyFilterRoutine)
        return (trigger);
    _inApplyFilterRoutine = true;

    sensorAuxiliaryResult.clear();
    sensorResult.sensorResultIsValid = true;
    sensorResult.sensorWasTriggered = false;

    if (_computeImageBasicStats && (_renderMode != sim_rendermode_colorcoded))
    {
        double cumulRed = 0.0;
        double cumulGreen = 0.0;
        double cumulBlue = 0.0;
        double cumulIntensity = 0.0;
        float cumulDepth = 0.0;

        // Initialize the min/max values with first element:
        for (int i = 0; i < 2; i++)
        {
            sensorResult.sensorDataRed[i] = (double)_rgbBuffer[0];
            sensorResult.sensorDataGreen[i] = (double)_rgbBuffer[1];
            sensorResult.sensorDataBlue[i] = (double)_rgbBuffer[2];
            sensorResult.sensorDataIntensity[i] = ((double)(_rgbBuffer[0] + _rgbBuffer[1] + _rgbBuffer[2])) / 3.0;
            sensorResult.sensorDataDepth[i] = _depthBuffer[0];
        }

        int v = _resolution[0] * _resolution[1];
        for (int i = 0; i < v; i++)
        {
            double intens = ((double)(_rgbBuffer[3 * i + 0] + _rgbBuffer[3 * i + 1] + _rgbBuffer[3 * i + 2])) / 3.0;
            cumulRed += (double)_rgbBuffer[3 * i + 0];
            cumulGreen += (double)_rgbBuffer[3 * i + 1];
            cumulBlue += (double)_rgbBuffer[3 * i + 2];
            cumulDepth += (double)_depthBuffer[i];
            cumulIntensity += intens;
            // we actualize min/max values:
            // r = (x < y) ? x : y
            // Red
            if (_rgbBuffer[3 * i + 0] < sensorResult.sensorDataRed[0])
                sensorResult.sensorDataRed[0] = _rgbBuffer[3 * i + 0];
            if (_rgbBuffer[3 * i + 0] > sensorResult.sensorDataRed[1])
                sensorResult.sensorDataRed[1] = _rgbBuffer[3 * i + 0];
            // Green
            if (_rgbBuffer[3 * i + 1] < sensorResult.sensorDataGreen[0])
                sensorResult.sensorDataGreen[0] = _rgbBuffer[3 * i + 1];
            if (_rgbBuffer[3 * i + 1] > sensorResult.sensorDataGreen[1])
                sensorResult.sensorDataGreen[1] = _rgbBuffer[3 * i + 1];
            // Blue
            if (_rgbBuffer[3 * i + 2] < sensorResult.sensorDataBlue[0])
                sensorResult.sensorDataBlue[0] = _rgbBuffer[3 * i + 2];
            if (_rgbBuffer[3 * i + 2] > sensorResult.sensorDataBlue[1])
                sensorResult.sensorDataBlue[1] = _rgbBuffer[3 * i + 2];
            // Intensity
            if ((unsigned char)intens < sensorResult.sensorDataIntensity[0])
                sensorResult.sensorDataIntensity[0] = (unsigned char)intens;
            if ((unsigned char)intens > sensorResult.sensorDataIntensity[1])
                sensorResult.sensorDataIntensity[1] = (unsigned char)intens;
            // Depth
            if (_depthBuffer[i] < sensorResult.sensorDataDepth[0])
                sensorResult.sensorDataDepth[0] = _depthBuffer[i];
            if (_depthBuffer[i] > sensorResult.sensorDataDepth[1])
                sensorResult.sensorDataDepth[1] = _depthBuffer[i];
        }
        unsigned char averageRed = (unsigned char)(cumulRed / (double)v);
        unsigned char averageGreen = (unsigned char)(cumulGreen / (double)v);
        unsigned char averageBlue = (unsigned char)(cumulBlue / (double)v);
        unsigned char averageIntensity = (unsigned char)(cumulIntensity / (double)v);
        float averageDepth = cumulDepth / float(v);
        // We set-up average values:
        sensorResult.sensorDataRed[2] = averageRed;
        sensorResult.sensorDataGreen[2] = averageGreen;
        sensorResult.sensorDataBlue[2] = averageBlue;
        sensorResult.sensorDataIntensity[2] = averageIntensity;
        sensorResult.sensorDataDepth[2] = averageDepth;

        // We prepare the auxiliary values:
        std::vector<double> defaultResults;
        defaultResults.push_back(double(sensorResult.sensorDataIntensity[0]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataRed[0]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataGreen[0]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataBlue[0]) / 255.0);
        defaultResults.push_back((double)sensorResult.sensorDataDepth[0]);

        defaultResults.push_back(double(sensorResult.sensorDataIntensity[1]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataRed[1]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataGreen[1]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataBlue[1]) / 255.0);
        defaultResults.push_back((double)sensorResult.sensorDataDepth[1]);

        defaultResults.push_back(double(sensorResult.sensorDataIntensity[2]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataRed[2]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataGreen[2]) / 255.0);
        defaultResults.push_back(double(sensorResult.sensorDataBlue[2]) / 255.0);
        defaultResults.push_back((double)sensorResult.sensorDataDepth[2]);
        sensorAuxiliaryResult.push_back(defaultResults);
    }
    else
    { // We do not want to produce those values
        for (int i = 0; i < 3; i++)
        {
            sensorResult.sensorDataRed[i] = 0;
            sensorResult.sensorDataGreen[i] = 0;
            sensorResult.sensorDataBlue[i] = 0;
            sensorResult.sensorDataIntensity[i] = 0;
            sensorResult.sensorDataDepth[i] = 0.0;
        }
        if (_renderMode != sim_rendermode_colorcoded)
        {
            std::vector<double> defaultResults(0.0, 15);
            sensorAuxiliaryResult.push_back(defaultResults);
        }
        else
        {
            unsigned int r = (SIM_IDEND_SCENEOBJECT - SIM_IDSTART_SCENEOBJECT) + 1;
            unsigned char *visibleIds = new unsigned char[r];
            for (unsigned int i = 0; i < r; i++)
                visibleIds[i] = 0;
            int v = _resolution[0] * _resolution[1];
            for (int i = 0; i < v; i++)
            {
                unsigned int id = _rgbBuffer[3 * i + 0] + (_rgbBuffer[3 * i + 1] << 8) + (_rgbBuffer[3 * i + 2] << 16);
                if (id < r)
                    visibleIds[id] = 1;
            }
            std::vector<double> defaultResults;
            for (unsigned int i = 0; i < r; i++)
            {
                if (visibleIds[i] != 0)
                    defaultResults.push_back(double(i));
            }
            sensorAuxiliaryResult.push_back(defaultResults);
            delete[] visibleIds;
        }
    }

    std::vector<CScriptObject*> scripts;
    getAttachedScripts(scripts, -1, true);
    getAttachedScripts(scripts, -1, false);

    if (scripts.size() > 0)
    {
        CInterfaceStack *inStack = App::worldContainer->interfaceStackContainer->createStack();
        inStack->pushTableOntoStack();

        inStack->insertKeyInt32IntoStackTable("handle", getObjectHandle());
        int res[2] = {_resolution[0], _resolution[1]};
        inStack->insertKeyInt32ArrayIntoStackTable("resolution", res, 2);
        double clip[2] = {_nearClippingPlane, _farClippingPlane};
        inStack->insertKeyDoubleArrayIntoStackTable("clippingPlanes", clip, 2);

        inStack->insertKeyFloatIntoStackTable("viewAngle", getViewAngle());
        inStack->insertKeyFloatIntoStackTable("orthoSize", getOrthoViewSize());
        inStack->insertKeyBoolIntoStackTable("perspectiveOperation", getPerspective());

        for (size_t i = 0; i < scripts.size(); i++)
        {
            CScriptObject* script = scripts[i];
            if (script->hasSystemFunctionOrHook(sim_syscb_vision))
            {
                CInterfaceStack *outStack = App::worldContainer->interfaceStackContainer->createStack();
                if (VThread::isSimThread())
                { // we are in the main simulation thread. Call only scripts that live in the same thread
                    if (!script->getThreadedExecution_oldThreads())
                        script->systemCallScript(sim_syscb_vision, inStack, outStack);
                }
                else
                { // OLD: we are in the thread started by a threaded simulation script. Call only that script
                    if (script->getThreadedExecution_oldThreads())
                        script->systemCallScript(sim_syscb_vision, inStack, nullptr);
                }
                if (outStack->getStackSize() >= 1)
                {
                    outStack->moveStackItemToTop(0);
                    bool trig = false;
                    if (outStack->getStackMapBoolValue("trigger", trig))
                        trigger = trig;
                    CInterfaceStackObject *obj = outStack->getStackMapObject("packedPackets");
                    if ((obj != nullptr) && (obj->getObjectType() == sim_stackitem_table))
                    {
                        CInterfaceStackTable *table = (CInterfaceStackTable *)obj;
                        if (table->isTableArray())
                        {
                            for (size_t i = 0; i < table->getArraySize(); i++)
                            {
                                CInterfaceStackObject *obj2 = table->getArrayItemAtIndex(i);
                                if ((obj2 != nullptr) && (obj2->getObjectType() == sim_stackitem_string))
                                { // the packed values are float, for backward compatibility
                                    CInterfaceStackString *buff = (CInterfaceStackString *)obj2;
                                    std::vector<float> data;
                                    size_t l;
                                    buff->getValue(&l);
                                    data.assign((float *)buff->getValue(nullptr),
                                                ((float *)buff->getValue(nullptr)) + l / sizeof(float));
                                    std::vector<double> data2;
                                    data2.resize(data.size());
                                    for (size_t j = 0; j < data.size(); j++)
                                        data2[j] = (double)data[j];
                                    sensorAuxiliaryResult.push_back(data2);
                                }
                            }
                        }
                    }
                }
                App::worldContainer->interfaceStackContainer->destroyStack(outStack);
            }
        }
        App::worldContainer->interfaceStackContainer->destroyStack(inStack);

        if (trigger)
        {
            inStack = App::worldContainer->interfaceStackContainer->createStack();
            inStack->pushTableOntoStack();
            inStack->insertKeyInt32IntoStackTable("handle", getObjectHandle());

            inStack->pushTextOntoStack("packedPackets");
            inStack->pushTableOntoStack();
            for (size_t i = 0; i < sensorAuxiliaryResult.size(); i++)
            {
                inStack->pushInt32OntoStack(int(i + 1));
                if (sensorAuxiliaryResult[i].size() > 0)
                { // the packed data needs to be float for backward compatibility
                    std::vector<float> dat;
                    dat.resize(sensorAuxiliaryResult[i].size());
                    for (size_t j = 0; j < sensorAuxiliaryResult[i].size(); j++)
                        dat[j] = (float)sensorAuxiliaryResult[i][j];
                    inStack->pushBufferOntoStack((char *)dat.data(), dat.size() * sizeof(float));
                }
                else
                    inStack->pushBufferOntoStack("", 0);
                inStack->insertDataIntoStackTable();
            }
            inStack->insertDataIntoStackTable();

            for (size_t i = 0; i < scripts.size(); i++)
            {
                CScriptObject* script = scripts[i];
                if (script->hasSystemFunctionOrHook(sim_syscb_trigger))
                {
                    CInterfaceStack *outStack = App::worldContainer->interfaceStackContainer->createStack();
                    if (VThread::isSimThread())
                    { // we are in the main simulation thread. Call only scripts that live in the same thread
                        if (!script->getThreadedExecution_oldThreads())
                            script->systemCallScript(sim_syscb_trigger, inStack, outStack);
                    }
                    else
                    { // we are in the thread started by a threaded simulation script. Call only that script
                        if (script->getThreadedExecution_oldThreads())
                            script->systemCallScript(sim_syscb_trigger, inStack, nullptr);
                    }
                    if (outStack->getStackSize() >= 1)
                    {
                        outStack->moveStackItemToTop(0);
                        bool trig = false;
                        if (outStack->getStackMapBoolValue("trigger", trig))
                            trigger = trig;
                    }
                    App::worldContainer->interfaceStackContainer->destroyStack(outStack);
                }
            }
            App::worldContainer->interfaceStackContainer->destroyStack(inStack);
        }
    }

    _inApplyFilterRoutine = false;
    return (trigger);
}

void CVisionSensor::serialize(CSer &ar)
{
    CSceneObject::serialize(ar);
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("_cp");
            ar << _orthoViewSize << _viewAngle;
            ar << _nearClippingPlane << _farClippingPlane;
            ar.flush();

            ar.storeDataName("Res");
            ar << _resolution[0] << _resolution[1];
            ar.flush();

            ar.storeDataName("Dox");
            ar << _detectableEntityHandle;
            ar.flush();

            ar.storeDataName("Db2");
            ar << _defaultBufferValues[0] << _defaultBufferValues[1] << _defaultBufferValues[2];
            ar.flush();

            ar.storeDataName("_i2");
            ar << _visionSensorSize;
            ar.flush();

            ar.storeDataName("Rmd");
            ar << _renderMode;
            ar.flush();

            ar.storeDataName("Afr");
            ar << _attributesForRendering;
            ar.flush();

            ar.storeDataName("Va3");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _perspective);
            SIM_SET_CLEAR_BIT(nothing, 1, _explicitHandling);
            SIM_SET_CLEAR_BIT(nothing, 2, !_showFogIfAvailable);
            // 12/12/2011        SIM_SET_CLEAR_BIT(nothing,3,_detectAllDetectable);
            SIM_SET_CLEAR_BIT(nothing, 4, _showVolume);
            SIM_SET_CLEAR_BIT(nothing, 5, false); //_showVolumeWhenDetecting);
            SIM_SET_CLEAR_BIT(nothing, 6, _useLocalLights);
            SIM_SET_CLEAR_BIT(nothing, 7, _useSameBackgroundAsEnvironment);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Va2");
            nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _useExternalImage);
            SIM_SET_CLEAR_BIT(nothing, 1, _ignoreRGBInfo);
            SIM_SET_CLEAR_BIT(nothing, 2, _ignoreDepthInfo);
            // RESERVED SIM_SET_CLEAR_BIT(nothing,3,_povFocalBlurEnabled);
            SIM_SET_CLEAR_BIT(nothing, 4, !_computeImageBasicStats);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Cl1");
            ar.setCountingMode();
            color.serialize(ar, 0);
            if (ar.setWritingMode())
                color.serialize(ar, 0);

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool povFocalBlurEnabled_backwardCompatibility_3_2_2016 = false;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Ccp") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli, blo, blu;
                        ar >> bla >> bli >> blo >> blu;
                        _orthoViewSize = (double)bla;
                        _viewAngle = (double)bli;
                        _nearClippingPlane = (double)blo;
                        _farClippingPlane = (double)blu;
                    }

                    if (theName.compare("_cp") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _orthoViewSize >> _viewAngle;
                        ar >> _nearClippingPlane >> _farClippingPlane;
                    }

                    if (theName.compare("Res") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _resolution[0] >> _resolution[1];
                    }
                    if (theName.compare("Dox") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _detectableEntityHandle;
                    }
                    if (theName.compare("Db2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _defaultBufferValues[0] >> _defaultBufferValues[1] >> _defaultBufferValues[2];
                    }
                    if (theName.compare("Siz") == 0)
                    { // for backward compatibility
                        noHit = false;
                        ar >> byteQuantity;
                        float dum, bla;
                        ar >> dum >> bla >> bla;
                        _visionSensorSize = (double)dum;
                    }
                    if (theName.compare("Si2") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        float bla;
                        ar >> bla;
                        _visionSensorSize = (double)bla;
                    }

                    if (theName.compare("_i2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _visionSensorSize;
                    }

                    if (theName.compare("Rmd") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _renderMode;
                        if (_renderMode == sim_rendermode_extrendererwindowed)
                            _renderMode = sim_rendermode_extrenderer;
                        if (_renderMode == sim_rendermode_opengl3windowed)
                            _renderMode = sim_rendermode_opengl3;
                    }
                    if (theName.compare("Afr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _attributesForRendering;
                    }
                    if (theName == "Var")
                    { // keep a while for backward compatibility (2010/07/17)
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _perspective = SIM_IS_BIT_SET(nothing, 0);
                        _explicitHandling = SIM_IS_BIT_SET(nothing, 1);
                        bool hideDetectionVolume = SIM_IS_BIT_SET(nothing, 2);
                        _useSameBackgroundAsEnvironment = SIM_IS_BIT_SET(nothing, 7);
                        if (hideDetectionVolume)
                            _showVolume = false;
                    }
                    if (theName == "Va3")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _perspective = SIM_IS_BIT_SET(nothing, 0);
                        _explicitHandling = SIM_IS_BIT_SET(nothing, 1);
                        _showFogIfAvailable = !SIM_IS_BIT_SET(nothing, 2);
                        _showVolume = SIM_IS_BIT_SET(nothing, 4);
                        //_showVolumeWhenDetecting=SIM_IS_BIT_SET(nothing,5);
                        _useLocalLights = SIM_IS_BIT_SET(nothing, 6);
                        _useSameBackgroundAsEnvironment = SIM_IS_BIT_SET(nothing, 7);
                    }
                    if (theName == "Va2")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _useExternalImage = SIM_IS_BIT_SET(nothing, 0);
                        _ignoreRGBInfo = SIM_IS_BIT_SET(nothing, 1);
                        _ignoreDepthInfo = SIM_IS_BIT_SET(nothing, 2);
                        povFocalBlurEnabled_backwardCompatibility_3_2_2016 = SIM_IS_BIT_SET(nothing, 3);
                        _computeImageBasicStats = !SIM_IS_BIT_SET(nothing, 4);
                    }
                    if (theName.compare("Pv1") == 0)
                    { // Keep for backward compatibility (3/2/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float povFocalDistance, povAperture;
                        int povBlurSamples;
                        ar >> povFocalDistance >> povAperture;
                        ar >> povBlurSamples;
                        _extensionString = "povray {focalBlur {";
                        if (povFocalBlurEnabled_backwardCompatibility_3_2_2016)
                            _extensionString += "true} focalDist {";
                        else
                            _extensionString += "false} focalDist {";
                        _extensionString += utils::getSizeString(false, (double)povFocalDistance);
                        _extensionString += "} aperture {";
                        _extensionString += utils::getSizeString(false, (double)povAperture);
                        _extensionString += "} blurSamples {";
                        _extensionString += utils::getIntString(false, povBlurSamples);
                        _extensionString += "}}";
                    }
                    if (theName.compare("Cl1") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        color.serialize(ar, 0);
                    }
                    if (theName.compare("Cfr") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        delete _composedFilter;
                        _composedFilter = new CComposedFilter();
                        _composedFilter->serialize(ar);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            _reserveBuffers();

            if (ar.getSerializationVersionThatWroteThisFile() < 17)
                utils::scaleColorUp_(color.getColorsPtr()); // on 29/08/2013 we corrected all default lights. So we need
                                                            // to correct for that change
            computeBoundingBox();
            computeVolumeVectors();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            ar.xmlAddNode_float("objectSize", _visionSensorSize);

            ar.xmlAddNode_float("orthoViewSize", _orthoViewSize);

            ar.xmlAddNode_float("viewAngle", _viewAngle * 180.0 / piValue);

            ar.xmlAddNode_2float("clippingPlanes", _nearClippingPlane, _farClippingPlane);

            ar.xmlAddNode_ints("resolution", _resolution, 2);

            if (exhaustiveXml)
                ar.xmlAddNode_floats("defaultBufferValues", _defaultBufferValues, 3);
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(_defaultBufferValues[l] * 255.1);
                ar.xmlAddNode_ints("defaultBufferValues", rgb, 3);
            }

            ar.xmlAddNode_comment(
                " 'renderMode' tag: can be 'openGL', 'colorCoded', 'PovRay', 'externalRenderer' or 'openGL3'",
                exhaustiveXml);
            ar.xmlAddNode_enum("renderMode", _renderMode, sim_rendermode_opengl, "openGL", sim_rendermode_auxchannels,
                               "auxiliaryChannels", sim_rendermode_colorcoded, "colorCoded", sim_rendermode_povray,
                               "PovRay", sim_rendermode_extrenderer, "externalRenderer",
                               sim_rendermode_extrendererwindowed, "externalRendererWindowed", sim_rendermode_opengl3,
                               "openGL3", sim_rendermode_opengl3windowed, "openGL3Windowed");

            if (exhaustiveXml)
                ar.xmlAddNode_int("renderAttributes", _attributesForRendering);

            if (exhaustiveXml)
                ar.xmlAddNode_int("renderableEntity", _detectableEntityHandle);
            else
            {
                std::string str;
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(_detectableEntityHandle);
                if (it != nullptr)
                    str = it->getObjectName_old();
                else
                {
                    CCollection *coll = App::currentWorld->collections->getObjectFromHandle(_detectableEntityHandle);
                    if (coll != nullptr)
                        str = "@collection@" + coll->getCollectionName();
                }
                ar.xmlAddNode_comment(" 'renderableEntity' tag only provided for backward compatibility, use instead "
                                      "'renderableObjectAlias' tag",
                                      exhaustiveXml);
                ar.xmlAddNode_string("renderableEntity", str.c_str());
                if (it != nullptr)
                {
                    str = it->getObjectAlias() + "*";
                    str += std::to_string(it->getObjectHandle());
                }
                ar.xmlAddNode_string("renderableObjectAlias", str.c_str());
            }

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("perspectiveMode", _perspective);
            ar.xmlAddNode_bool("explicitHandling", _explicitHandling);
            ar.xmlAddNode_bool("showFog", _showFogIfAvailable);
            ar.xmlAddNode_bool("showVolume", _showVolume);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("useLocalLights", _useLocalLights);
            ar.xmlAddNode_bool("useExternalImage", _useExternalImage);
            ar.xmlAddNode_bool("ignoreRgbInfo", _ignoreRGBInfo);
            ar.xmlAddNode_bool("ignoreDepthInfo", _ignoreDepthInfo);
            ar.xmlAddNode_bool("computeBasicStats", _computeImageBasicStats);
            ar.xmlAddNode_bool("sameBackgroundAsEnvironment", _useSameBackgroundAsEnvironment);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                ar.xmlPushNewNode("objectColor");
                color.serialize(ar, 0);
                ar.xmlPopNode();
            }
            else
            {
                int rgb[3];
                for (size_t l = 0; l < 3; l++)
                    rgb[l] = int(color.getColorsPtr()[l] * 255.1);
                ar.xmlAddNode_ints("objectColor", rgb, 3);
            }
        }
        else
        {
            C3Vector x;
            if (ar.xmlGetNode_floats("size", x.data, 3, false))
                setVisionSensorSize(x(0));

            double s, s2;
            if (ar.xmlGetNode_float("objectSize", s, false))
                setVisionSensorSize(s);

            if (ar.xmlGetNode_float("orthoViewSize", s, exhaustiveXml))
                setOrthoViewSize(s);

            if (ar.xmlGetNode_float("viewAngle", s, exhaustiveXml))
                setViewAngle(s * piValue / 180.0);

            if (ar.xmlGetNode_2float("clippingPlanes", s, s2, exhaustiveXml))
                setClippingPlanes(s, s2);

            ar.xmlGetNode_ints("resolution", _resolution, 2, exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_floats("defaultBufferValues", _defaultBufferValues, 3, exhaustiveXml);
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("defaultBufferValues", rgb, 3, exhaustiveXml))
                {
                    _defaultBufferValues[0] = float(rgb[0]) / 255.0;
                    _defaultBufferValues[1] = float(rgb[1]) / 255.0;
                    _defaultBufferValues[2] = float(rgb[2]) / 255.0;
                }
            }

            ar.xmlGetNode_enum("renderMode", _renderMode, exhaustiveXml, "openGL", sim_rendermode_opengl,
                               "auxiliaryChannels", sim_rendermode_auxchannels, "colorCoded", sim_rendermode_colorcoded,
                               "PovRay", sim_rendermode_povray, "externalRenderer", sim_rendermode_extrenderer,
                               "externalRendererWindowed", sim_rendermode_extrendererwindowed, "openGL3",
                               sim_rendermode_opengl3, "openGL3Windowed", sim_rendermode_opengl3windowed);
            if (_renderMode == sim_rendermode_extrendererwindowed)
                _renderMode = sim_rendermode_extrenderer;
            if (_renderMode == sim_rendermode_opengl3windowed)
                _renderMode = sim_rendermode_opengl3;

            if (exhaustiveXml)
                ar.xmlGetNode_int("renderAttributes", _attributesForRendering, exhaustiveXml);

            if (exhaustiveXml)
                ar.xmlGetNode_int("renderableEntity", _detectableEntityHandle);
            else
            {
                ar.xmlGetNode_string("renderableObjectAlias", _detectableEntityLoadAlias, exhaustiveXml);
                ar.xmlGetNode_string("renderableEntity", _detectableEntityLoadName_old, exhaustiveXml);
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("perspectiveMode", _perspective, exhaustiveXml);
                ar.xmlGetNode_bool("explicitHandling", _explicitHandling, exhaustiveXml);
                ar.xmlGetNode_bool("showFog", _showFogIfAvailable, exhaustiveXml);
                ar.xmlGetNode_bool("showVolumeWhenNotDetecting", _showVolume, false);
                ar.xmlGetNode_bool("showVolume", _showVolume, exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("useLocalLights", _useLocalLights, exhaustiveXml);
                ar.xmlGetNode_bool("useExternalImage", _useExternalImage, exhaustiveXml);
                ar.xmlGetNode_bool("ignoreRgbInfo", _ignoreRGBInfo, exhaustiveXml);
                ar.xmlGetNode_bool("ignoreDepthInfo", _ignoreDepthInfo, exhaustiveXml);
                ar.xmlGetNode_bool("computeBasicStats", _computeImageBasicStats, exhaustiveXml);
                ar.xmlGetNode_bool("sameBackgroundAsEnvironment", _useSameBackgroundAsEnvironment, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (ar.xmlPushChildNode("color", false))
            { // for backward compatibility
                if (exhaustiveXml)
                {
                    if (ar.xmlPushChildNode("passive"))
                    {
                        color.serialize(ar, 0);
                        ar.xmlPopNode();
                    }
                }
                else
                {
                    int rgb[3];
                    if (ar.xmlGetNode_ints("passive", rgb, 3, exhaustiveXml))
                        color.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                       sim_colorcomponent_ambient_diffuse);
                }
                ar.xmlPopNode();
            }

            if (exhaustiveXml)
            {
                if (ar.xmlPushChildNode("objectColor", false))
                {
                    color.serialize(ar, 0);
                    ar.xmlPopNode();
                }
            }
            else
            {
                int rgb[3];
                if (ar.xmlGetNode_ints("objectColor", rgb, 3, false))
                    color.setColor(float(rgb[0]) / 255.1, float(rgb[1]) / 255.1, float(rgb[2]) / 255.1,
                                   sim_colorcomponent_ambient_diffuse);
            }

            _reserveBuffers();
            computeBoundingBox();
            computeVolumeVectors();
        }
    }
}

bool CVisionSensor::detectVisionSensorEntity_executedViaUiThread(
    int entityID, bool detectAll, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
    bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections)
{
    bool retVal = false;
#ifdef SIM_WITH_GUI
    TRACE_INTERNAL;
    if (VThread::isUiThread())
    { // we are in the UI thread. We execute the command now:
        retVal = detectEntity2(entityID, detectAll, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                               hideEdgesIfModel, overrideRenderableFlagsForNonCollections);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId = DETECT_VISION_SENSOR_ENTITY_UITHREADCMD;
        cmdIn.objectParams.push_back(this);
        cmdIn.intParams.push_back(entityID);
        cmdIn.boolParams.push_back(detectAll);
        cmdIn.boolParams.push_back(entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects);
        cmdIn.boolParams.push_back(hideEdgesIfModel);
        cmdIn.boolParams.push_back(overrideRenderableFlagsForNonCollections);
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
        retVal = cmdOut.boolParams[0];
    }
#endif
    return (retVal);
}

#ifdef SIM_WITH_GUI
void CVisionSensor::display(CViewableBase *renderingObject, int displayAttrib)
{
    displayVisionSensor(this, renderingObject, displayAttrib);
}

void CVisionSensor::createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(bool useStencilBuffer)
{
    TRACE_INTERNAL;
    if (VThread::isUiThread())
    { // we are in the UI thread. We execute the command now:
        createGlContextAndFboAndTextureObjectIfNeeded(useStencilBuffer);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId = CREATE_GL_CONTEXT_FBO_TEXTURE_IF_NEEDED_UITHREADCMD;
        cmdIn.objectParams.push_back(this);
        cmdIn.boolParams.push_back(useStencilBuffer);
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
    }
}

void CVisionSensor::createGlContextAndFboAndTextureObjectIfNeeded(bool useStencilBuffer)
{
    TRACE_INTERNAL;
    if ((_contextFboAndTexture != nullptr) &&
        (_contextFboAndTexture->frameBufferObject->getUsingStencilBuffer() != useStencilBuffer))
        _removeGlContextAndFboAndTextureObjectIfNeeded(); // if settings have changed (e.g. mirror was added), remove
                                                          // everything

    if (_contextFboAndTexture == nullptr)
    { // our objects are not yet there. Build them:
#ifdef USES_QGLWIDGET
        QGLWidget *otherWidgetToShareResourcesWith = nullptr;
#else
        QOpenGLWidget *otherWidgetToShareResourcesWith = nullptr;
#endif
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
            otherWidgetToShareResourcesWith = GuiApp::mainWindow->openglWidget;
#endif

        // By default, we use QT_WINDOW_HIDE_TP, since
        // QT_OFFSCREEN_TP causes problems on certain GPUs, e.g.:
        // Intel Graphics Media Accelerator 3150.
        // In Headless mode under Linux, we use a offscreen type by default,
        // because otherwise even hidden windows are visible somehow
        int offscreenContextType = COffscreenGlContext::QT_WINDOW_HIDE_TP;
#ifdef LIN_SIM
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow == nullptr) // headless mode
#endif
            offscreenContextType = COffscreenGlContext::QT_OFFSCREEN_TP;
#endif
        if (App::userSettings->offscreenContextType != -1)
        {
            if (App::userSettings->offscreenContextType == 0)
                offscreenContextType = COffscreenGlContext::QT_OFFSCREEN_TP;
            if (App::userSettings->offscreenContextType == 1)
                offscreenContextType = COffscreenGlContext::QT_WINDOW_SHOW_TP;
            if (App::userSettings->offscreenContextType == 2)
                offscreenContextType = COffscreenGlContext::QT_WINDOW_HIDE_TP;
        }

        // By default:
        // - we use Qt FBOs on Mac (the non-Qt FBOs always caused problems there)
        // - we use non-Qt FBOs on Windows and Linux (Qt FBOs on Linux cause a lot of problems, e.g.: NVIDIA Geforce
        // 9600M GS
#ifdef WIN_SIM
        bool nativeFbo = true;
#endif
#ifdef MAC_SIM
        bool nativeFbo = false;
#endif
#ifdef LIN_SIM
        bool nativeFbo = true;
#endif
        if (App::userSettings->fboType != -1)
        {
            if (App::userSettings->fboType == 0)
                nativeFbo = true;
            if (App::userSettings->fboType == 1)
                nativeFbo = false;
        }

        _contextFboAndTexture = new CVisionSensorGlStuff(_resolution[0], _resolution[1], offscreenContextType,
                                                         !nativeFbo, otherWidgetToShareResourcesWith, useStencilBuffer,
                                                         !App::userSettings->oglCompatibilityTweak1);
    }
}

void CVisionSensor::_removeGlContextAndFboAndTextureObjectIfNeeded()
{
    TRACE_INTERNAL;
    if (_contextFboAndTexture != nullptr)
    {
        if (VThread::isSimThread())
            _contextFboAndTexture->deleteLater(); // We are in the wrong thread to delete it here
        else
            delete _contextFboAndTexture;
        _contextFboAndTexture = nullptr;
    }
}

void CVisionSensor::_handleMirrors(const std::vector<int> &activeMirrors, int entityID, bool detectAll,
                                   bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                   bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections)
{
    if (activeMirrors.size() == 0)
        return;

    C7Vector camTr(getFullCumulativeTransformation());
    C7Vector camTri(camTr.getInverse());

    setFrustumCullingTemporarilyDisabled(true);
    // Prep stencil buffer:
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    int drawOk = 1;

    std::vector<int> allMirrors;
    std::vector<double> allMirrorDist;
    for (int mir = 0; mir < int(activeMirrors.size()); mir++)
    {
        CMirror *myMirror = App::currentWorld->sceneObjects->getMirrorFromHandle(activeMirrors[mir]);
        C7Vector mmtr(myMirror->getFullCumulativeTransformation());
        mmtr = camTri * mmtr;
        allMirrors.push_back(activeMirrors[mir]);
        allMirrorDist.push_back(mmtr.X(2));
    }
    tt::orderAscending(allMirrorDist, allMirrors);

    for (int mir = 0; mir < int(allMirrors.size()); mir++)
    {
        CMirror *myMirror = App::currentWorld->sceneObjects->getMirrorFromHandle(allMirrors[mir]);

        C7Vector mtr(myMirror->getFullCumulativeTransformation());
        C7Vector mtri(mtr.getInverse());
        C3Vector mtrN(mtr.Q.getMatrix().axis[2]);
        C4Vector mtrAxis = mtr.Q.getAngleAndAxis();
        C4Vector mtriAxis = mtri.Q.getAngleAndAxis();
        double d = (mtrN * mtr.X);
        C3Vector v0(+myMirror->getMirrorWidth() * 0.5, -myMirror->getMirrorHeight() * 0.5, 0.0);
        C3Vector v1(+myMirror->getMirrorWidth() * 0.5, +myMirror->getMirrorHeight() * 0.5, 0.0);
        C3Vector v2(-myMirror->getMirrorWidth() * 0.5, +myMirror->getMirrorHeight() * 0.5, 0.0);
        C3Vector v3(-myMirror->getMirrorWidth() * 0.5, -myMirror->getMirrorHeight() * 0.5, 0.0);
        v0 *= mtr;
        v1 *= mtr;
        v2 *= mtr;
        v3 *= mtr;

        C3Vector MirrCam(camTr.X - mtr.X);
        bool inFrontOfMirror = (((MirrCam * mtrN) > 0.0) && myMirror->getActive());

        glStencilFunc(GL_ALWAYS, drawOk, drawOk);  // we can draw everywhere
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // we draw drawOk where depth test passes
        glDepthMask(GL_FALSE);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glBegin(GL_QUADS);
        glVertex3dv(v0.data);
        glVertex3dv(v1.data);
        glVertex3dv(v2.data);
        glVertex3dv(v3.data);
        glEnd();
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Enable stencil masking:
        glStencilFunc(GL_EQUAL, drawOk, drawOk); // we draw only where stencil is drawOk
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        // Draw the mirror view:
        if (inFrontOfMirror)
        {
            glEnable(GL_CLIP_PLANE0);
            double cpv[4] = {-mtrN(0), -mtrN(1), -mtrN(2), d};
            glClipPlane(GL_CLIP_PLANE0, cpv);
            glPushMatrix();
            glTranslated(mtr.X(0), mtr.X(1), mtr.X(2));
            glRotated(mtrAxis(0) * radToDeg, mtrAxis(1), mtrAxis(2), mtrAxis(3));
            glScalef(1., 1., -1.);
            glTranslated(mtri.X(0), mtri.X(1), mtri.X(2));
            glRotated(mtriAxis(0) * radToDeg, mtriAxis(1), mtriAxis(2), mtriAxis(3));
            glFrontFace(GL_CW);
            CMirror::currentMirrorContentBeingRendered = myMirror->getObjectHandle();
            _drawObjects(entityID, detectAll, entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                         hideEdgesIfModel, overrideRenderableFlagsForNonCollections);
            CMirror::currentMirrorContentBeingRendered = -1;
            glFrontFace(GL_CCW);
            glPopMatrix();
            glDisable(GL_CLIP_PLANE0);
        }

        // Now draw the mirror overlay:
        glPushAttrib(0xffffffff);
        ogl::disableLighting_useWithCare(); // only temporarily
        glDepthFunc(GL_ALWAYS);
        if (inFrontOfMirror)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glColor4d(myMirror->mirrorColor[0], myMirror->mirrorColor[1], myMirror->mirrorColor[2],
                  1.0 - myMirror->getReflectance());
        glBegin(GL_QUADS);
        glVertex3dv(v0.data);
        glVertex3dv(v1.data);
        glVertex3dv(v2.data);
        glVertex3dv(v3.data);
        glEnd();
        glPopAttrib();
        ogl::enableLighting_useWithCare();
        glDepthFunc(GL_LEQUAL);
        drawOk++;
    }
    glDisable(GL_STENCIL_TEST);
    setFrustumCullingTemporarilyDisabled(false);
}

void CVisionSensor::lookAt(CSView *viewObject, int viewPos[2], int viewSize[2])
{ // viewPos and viewSize can be nullptr.
    TRACE_INTERNAL;
    int currentWinSize[2];
    int currentWinPos[2];
    if (viewObject != nullptr)
    {
        viewObject->getViewSize(currentWinSize);
        viewObject->getViewPosition(currentWinPos);
    }
    else
    {
        currentWinSize[0] = viewSize[0];
        currentWinSize[1] = viewSize[1];
        currentWinPos[0] = viewPos[0];
        currentWinPos[1] = viewPos[1];
    }

    ogl::setMaterialColor(ogl::colorBlack, ogl::colorBlack, ogl::colorBlack);
    glEnable(GL_SCISSOR_TEST);
    glViewport(currentWinPos[0], currentWinPos[1], currentWinSize[0], currentWinSize[1]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, currentWinSize[0], 0.0, currentWinSize[1], -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if ((_contextFboAndTexture != nullptr) || getApplyExternalRenderedImage())
    {
        double r0 = double(currentWinSize[1]) / double(currentWinSize[0]);
        double r1 = double(_resolution[1]) / double(_resolution[0]);
        int c0[2];
        int c1[2];
        if (r1 >= r0)
        {
            c0[1] = 0;
            c1[1] = currentWinSize[1];
            int d = int(double(currentWinSize[1]) / r1);
            c0[0] = (currentWinSize[0] - d) / 2;
            c1[0] = c0[0] + d;
        }
        else
        {
            c0[0] = 0;
            c1[0] = currentWinSize[0];
            int d = int(double(currentWinSize[0]) * r1);
            c0[1] = (currentWinSize[1] - d) / 2;
            c1[1] = c0[1] + d;
        }

        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::colorWhite);
        double texCorners[4] = {0.0, 0.0, 1.0, 1.0};

        if (getApplyExternalRenderedImage())
        {
            if (_rayTracingTextureName == (unsigned int)-1)
                _rayTracingTextureName = ogl::genTexture(); // glGenTextures(1,&_rayTracingTextureName);
            glBindTexture(GL_TEXTURE_2D, _rayTracingTextureName);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _resolution[0], _resolution[1], 0, GL_RGB, GL_UNSIGNED_BYTE, _rgbBuffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // keep to GL_LINEAR here!!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, _rayTracingTextureName);
            glColor3f(1.0, 1.0, 1.0);
        }
        else
        {
            _startTextureDisplay(_contextFboAndTexture->textureObject, false, 0, false, false);
            if (_rayTracingTextureName != (unsigned int)-1)
            {
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId = DESTROY_GL_TEXTURE_UITHREADCMD;
                cmdIn.uintParams.push_back(_rayTracingTextureName);
                GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
                _rayTracingTextureName = (unsigned int)-1;
            }
        }

        glBegin(GL_QUADS);
        glTexCoord2d(texCorners[0], texCorners[1]);
        glVertex3i(c0[0], c0[1], 0);
        glTexCoord2d(texCorners[0], texCorners[3]);
        glVertex3i(c0[0], c1[1], 0);
        glTexCoord2d(texCorners[2], texCorners[3]);
        glVertex3i(c1[0], c1[1], 0);
        glTexCoord2d(texCorners[2], texCorners[1]);
        glVertex3i(c1[0], c0[1], 0);
        glEnd();
        if (_rayTracingTextureName != (unsigned int)-1)
            glDisable(GL_TEXTURE_2D);
        else
            _endTextureDisplay();
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
}

CTextureObject *CVisionSensor::getTextureObject()
{
    if (_contextFboAndTexture != nullptr)
        return (_contextFboAndTexture->textureObject);
    return (nullptr);
}
#endif

int CVisionSensor::setBoolProperty(const char* ppName, bool pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::setBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_backgroundSameAsEnv.name)
        {
            retVal = 1;
            setUseEnvironmentBackgroundColor(pState);
        }
        else if (_pName == propVisionSensor_explicitHandling.name)
        {
            retVal = 1;
            setExplicitHandling(pState);
        }
        else if (_pName == propVisionSensor_useExtImage.name)
        {
            retVal = 1;
            setUseExternalImage(pState);
        }
        else if (_pName == propVisionSensor_ignoreRgbInfo.name)
        {
            retVal = 1;
            setIgnoreRGBInfo(pState);
        }
        else if (_pName == propVisionSensor_ignoreDepthInfo.name)
        {
            retVal = 1;
            setIgnoreDepthInfo(pState);
        }
        else if (_pName == propVisionSensor_omitPacket1.name)
        {
            retVal = 1;
            setComputeImageBasicStats(!pState);
        }
        else if (_pName == propVisionSensor_emitImageChangedEvent.name)
        {
            retVal = 1;
            setEmitImageChangedEvent(pState);
        }
        else if (_pName == propVisionSensor_emitDepthChangedEvent.name)
        {
            retVal = 1;
            setEmitDepthChangedEvent(pState);
        }
    }

    return retVal;
}

int CVisionSensor::getBoolProperty(const char* ppName, bool& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::getBoolProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_backgroundSameAsEnv.name)
        {
            retVal = 1;
            pState = _useSameBackgroundAsEnvironment;
        }
        else if (_pName == propVisionSensor_explicitHandling.name)
        {
            retVal = 1;
            pState = _explicitHandling;
        }
        else if (_pName == propVisionSensor_useExtImage.name)
        {
            retVal = 1;
            pState = _useExternalImage;
        }
        else if (_pName == propVisionSensor_ignoreRgbInfo.name)
        {
            retVal = 1;
            pState = _ignoreRGBInfo;
        }
        else if (_pName == propVisionSensor_ignoreDepthInfo.name)
        {
            retVal = 1;
            pState = _ignoreDepthInfo;
        }
        else if (_pName == propVisionSensor_omitPacket1.name)
        {
            retVal = 1;
            pState = !_computeImageBasicStats;
        }
        else if (_pName == propVisionSensor_emitImageChangedEvent.name)
        {
            retVal = 1;
            pState = _emitImageChangedEventEnabled;
        }
        else if (_pName == propVisionSensor_emitDepthChangedEvent.name)
        {
            retVal = 1;
            pState = _emitDepthChangedEventEnabled;
        }
    }

    return retVal;
}

int CVisionSensor::setIntProperty(const char* ppName, int pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::setIntProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_renderMode.name)
        {
            retVal = 1;
            setRenderMode(pState);
        }
    }

    return retVal;
}

int CVisionSensor::getIntProperty(const char* ppName, int& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::getIntProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_renderMode.name)
        {
            retVal = 1;
            pState = _renderMode;
        }
    }

    return retVal;
}

int CVisionSensor::setFloatProperty(const char* ppName, double pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::setFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = color.setFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_size.name)
        {
            setVisionSensorSize(pState);
            retVal = 1;
        }
    }

    return retVal;
}

int CVisionSensor::getFloatProperty(const char* ppName, double& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::getFloatProperty(pName, pState);
    if (retVal == -1)
        retVal = color.getFloatProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_size.name)
        {
            pState = _visionSensorSize;
            retVal = 1;
        }
    }

    return retVal;
}

int CVisionSensor::setBufferProperty(const char* ppName, const char* buffer, int bufferL)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::setBufferProperty(pName, buffer, bufferL);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_imageBuffer.name)
        {
            if (bufferL == 3 * _resolution[0] * _resolution[1])
            {
                writeImage((float*)buffer, 0);
                retVal = 1;
            }
            else
                retVal = 0;
        }
    }

    return retVal;
}

int CVisionSensor::getBufferProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    pState.clear();
    int retVal = CViewableBase::getBufferProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_imageBuffer.name)
        {
            pState.assign(_rgbBuffer, _rgbBuffer + 3 * _resolution[0] * _resolution[1]);
            retVal = 1;
        }
    }

    return retVal;
}

int CVisionSensor::setColorProperty(const char* ppName, const float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::setColorProperty(pName, pState);
    if (retVal == -1)
        retVal = color.setColorProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_backgroundCol.name)
        {
            retVal = 1;
            setDefaultBufferValues(pState);
        }
    }
    return retVal;
}

int CVisionSensor::getColorProperty(const char* ppName, float* pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::getColorProperty(pName, pState);
    if (retVal == -1)
        retVal = color.getColorProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_backgroundCol.name)
        {
            retVal = 1;
            pState[0] = _defaultBufferValues[0];
            pState[1] = _defaultBufferValues[1];
            pState[2] = _defaultBufferValues[2];
        }
    }
    return retVal;
}

int CVisionSensor::setVector3Property(const char* ppName, const C3Vector& pState)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::setVector3Property(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CVisionSensor::getVector3Property(const char* ppName, C3Vector& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CViewableBase::getVector3Property(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CVisionSensor::setVectorProperty(const char* ppName, const double* v, int vL)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    if (v == nullptr)
        vL = 0;
    int retVal = CViewableBase::setVectorProperty(pName, v, vL);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CVisionSensor::getVectorProperty(const char* ppName, std::vector<double>& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    pState.clear();
    int retVal = CViewableBase::getVectorProperty(pName, pState);
    if (retVal == -1)
    {
        if (_pName == propVisionSensor_depthBuffer.name)
        {
            retVal = 1;
            pState.assign(_depthBuffer, _depthBuffer + _resolution[0] * _resolution[1]);
        }
    }

    return retVal;
}

int CVisionSensor::setIntVectorProperty(const char* ppName, const int* v, int vL)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    if (v == nullptr)
        vL = 0;
    int retVal = CViewableBase::setIntVectorProperty(pName, v, vL);
    if (retVal == -1)
    {
    }


    return retVal;
}

int CVisionSensor::getIntVectorProperty(const char* ppName, std::vector<int>& pState) const
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    pState.clear();
    int retVal = CViewableBase::getIntVectorProperty(pName, pState);
    if (retVal == -1)
    {
    }

    return retVal;
}

int CVisionSensor::getPropertyName(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".visionSensor";
        retVal = color.getPropertyName(index, pName);
    }
    if (retVal == -1)
        retVal = CViewableBase::getPropertyName_vstatic(index, pName);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_visionSensor.size(); i++)
        {
            index--;
            if (index == -1)
            {
                pName = allProps_visionSensor[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CVisionSensor::getPropertyName_static(int& index, std::string& pName, std::string& appartenance)
{
    int retVal = CSceneObject::getPropertyName_bstatic(index, pName, appartenance);
    if (retVal == -1)
    {
        appartenance += ".visionSensor";
        retVal = CColorObject::getPropertyName_static(index, pName, 1 + 4 + 8, "");
    }
    if (retVal == -1)
        retVal = CViewableBase::getPropertyName_vstatic(index, pName);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_visionSensor.size(); i++)
        {
            index--;
            if (index == -1)
            {
                pName = allProps_visionSensor[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CVisionSensor::getPropertyInfo(const char* ppName, int& info)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo(pName, info);
    if (retVal == -1)
        retVal = color.getPropertyInfo(pName, info);
    if (retVal == -1)
        retVal = CViewableBase::getPropertyInfo_vstatic(pName, info);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_visionSensor.size(); i++)
        {
            if (strcmp(allProps_visionSensor[i].name, pName) == 0)
            {
                retVal = allProps_visionSensor[i].type;
                info = allProps_visionSensor[i].flags;
                break;
            }
        }
        if (retVal != -1)
        {
            if (_pName == propVisionSensor_imageBuffer.name)
            {
                if (3 * _resolution[0] * _resolution[1] > 1000)
                    retVal = retVal | sim_propertytype_largedata;
            }
            if (_pName == propVisionSensor_depthBuffer.name)
            {
                if (_resolution[0] * _resolution[1] > 1000)
                    retVal = retVal | sim_propertytype_largedata;
            }
        }
    }
    return retVal;
}

int CVisionSensor::getPropertyInfo_static(const char* ppName, int& info)
{
    std::string _pName(utils::getWithoutPrefix(utils::getWithoutPrefix(ppName, "object.").c_str(), "visionSensor."));
    const char* pName = _pName.c_str();
    int retVal = CSceneObject::getPropertyInfo_bstatic(pName, info);
    if (retVal == -1)
        retVal = CColorObject::getPropertyInfo_static(pName, info, 1 + 4 + 8, "");
    if (retVal == -1)
        retVal = CViewableBase::getPropertyInfo_vstatic(pName, info);
    if (retVal == -1)
    {
        for (size_t i = 0; i < allProps_visionSensor.size(); i++)
        {
            if (strcmp(allProps_visionSensor[i].name, pName) == 0)
            {
                retVal = allProps_visionSensor[i].type;
                info = allProps_visionSensor[i].flags;
                break;
            }
        }
    }
    return retVal;
}

