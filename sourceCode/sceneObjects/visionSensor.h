#pragma once

#include <viewableBase.h>
#include <sView.h>
#include <composedFilter.h>
#include <textureObject.h>
#ifdef SIM_WITH_GUI
#include <visionSensorGlStuff.h>
#endif

struct SHandlingResult
{
    bool sensorWasTriggered;
    bool sensorResultIsValid;
    unsigned char sensorDataRed[3];       // min, max, average
    unsigned char sensorDataGreen[3];     // min, max, average
    unsigned char sensorDataBlue[3];      // min, max, average
    unsigned char sensorDataIntensity[3]; // min, max, average
    float sensorDataDepth[3];             // min, max, average
    int calcTimeInMs;
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                         \
    FUNCX(propVisionSensor_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object meta information", "")             \
    FUNCX(propVisionSensor_size, "sensorSize", sim_propertytype_float, 0, "Size", "Sensor size")                                                                                  \
    FUNCX(propVisionSensor_backgroundCol, "backgroundColor", sim_propertytype_color, 0, "Background color", "")                                                                   \
    FUNCX(propVisionSensor_renderMode, "renderMode", sim_propertytype_int, 0, "Render mode", "")                                                                                  \
    FUNCX(propVisionSensor_backgroundSameAsEnv, "backgroundColorFromEnvironment", sim_propertytype_bool, 0, "Background color from environment", "")                              \
    FUNCX(propVisionSensor_explicitHandling, "explicitHandling", sim_propertytype_bool, 0, "Explicit handling", "")                                                               \
    FUNCX(propVisionSensor_useExtImage, "useExtImage", sim_propertytype_bool, 0, "External input", "")                                                                            \
    FUNCX(propVisionSensor_ignoreRgbInfo, "ignoreImageInfo", sim_propertytype_bool, 0, "Ignore RGB buffer", "Ignore RGB buffer (faster)")                                         \
    FUNCX(propVisionSensor_ignoreDepthInfo, "ignoreDepthInfo", sim_propertytype_bool, 0, "Ignore Depth buffer", "Ignore Depth buffer (faster)")                                   \
    FUNCX(propVisionSensor_omitPacket1, "omitPacket1", sim_propertytype_bool, 0, "Packet 1 is blank", "Omit packet 1 (faster)")                                                   \
    FUNCX(propVisionSensor_emitImageChangedEvent, "emitImageChangedEvent", sim_propertytype_bool, 0, "Emit image change event", "")                                               \
    FUNCX(propVisionSensor_emitDepthChangedEvent, "emitDepthChangedEvent", sim_propertytype_bool, 0, "Emit depth change event", "")                                               \
    FUNCX(propVisionSensor_imageBuffer, "imageBuffer", sim_propertytype_buffer, sim_propertyinfo_modelhashexclude, "RGB buffer", "")                                              \
    FUNCX(propVisionSensor_depthBuffer, "depthBuffer", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Depth buffer", "")         \
    FUNCX(propVisionSensor_packedDepthBuffer, "packedDepthBuffer", sim_propertytype_buffer, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Packed depth buffer", "")         \
    FUNCX(propVisionSensor_triggerState, "triggerState", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Trigger state", "")         \
    FUNCX(propVisionSensor_packet1, "packet1", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Data packet 1", "")         \
    FUNCX(propVisionSensor_packet2, "packet2", sim_propertytype_floatarray, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Data packet 2", "")         \
    FUNCX(propVisionSensor_povFocalBlur, "povray.focalBlur", sim_propertytype_bool, 0, "POV-Ray: focal blur", "Focal blur (with the POV-Ray renderer plugin)")                    \
    FUNCX(propVisionSensor_povBlurSamples, "povray.blurSamples", sim_propertytype_int, 0, "POV-Ray: blur samples", "Focal blur samples (with the POV-Ray renderer plugin)")       \
    FUNCX(propVisionSensor_povBlurDistance, "povray.blurDistance", sim_propertytype_float, 0, "POV-Ray: blur distance", "Focal blur distance (with the POV-Ray renderer plugin)") \
    FUNCX(propVisionSensor_povAperture, "povray.aperture", sim_propertytype_float, 0, "POV-Ray: aperture", "Aperture (with the POV-Ray renderer plugin)")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_visionSensor = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CVisionSensor : public CViewableBase
{
  public:
    CVisionSensor();
    virtual ~CVisionSensor();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;
    void setIsInScene(bool s) override;
    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setIntArray2Property(const char* pName, const int* pState) override;
    int getIntArray2Property(const char* pName, int* pState) const override;
    int setVector2Property(const char* pName, const double* pState) override;
    int getVector2Property(const char* pName, double* pState) const override;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL) override;
    int getBufferProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setVector3Property(const char* pName, const C3Vector& pState) override;
    int getVector3Property(const char* pName, C3Vector& pState) const override;
    int setFloatArrayProperty(const char* pName, const double* v, int vL) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int setIntArrayProperty(const char* pName, const int* v, int vL) override;
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    void commonInit();

    void setResolution(const int r[2]) override;

    void setVisionSensorSize(double s);
    double getVisionSensorSize() const;
    void setExplicitHandling(bool eh);
    bool getExplicitHandling() const;
    void resetSensor();
    bool handleSensor();
    bool checkSensor(int entityID, bool overrideRenderableFlagsForNonCollections);
    float* checkSensorEx(int entityID, bool imageBuffer,
                         bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, bool hideEdgesIfModel,
                         bool overrideRenderableFlagsForNonCollections);
    void setDepthBuffer(const float* img);

    void setIgnoreRGBInfo(bool ignore);
    bool getIgnoreRGBInfo() const;
    void setIgnoreDepthInfo(bool ignore);
    bool getIgnoreDepthInfo() const;

    void setRenderMode(int mode);
    int getRenderMode() const;

    void setEmitImageChangedEvent(bool e);
    void setEmitDepthChangedEvent(bool e);

    void setAttributesForRendering(int attr);
    int getAttributesForRendering() const;

    void setComputeImageBasicStats(bool c);
    bool getComputeImageBasicStats() const;

    unsigned char* getRgbBufferPointer();
    float* getDepthBufferPointer() const;

    void setDetectableEntityHandle(int entityHandle);
    int getDetectableEntityHandle() const;

    bool detectVisionSensorEntity_executedViaUiThread(
        int entityID, bool detectAll, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
        bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections);
    bool detectEntity(int entityID, bool detectAll, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                      bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections);
    bool detectEntity2(int entityID, bool detectAll, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                       bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections);
    bool renderForDetection(int entityID, bool detectAll,
                            bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, bool hideEdgesIfModel,
                            bool overrideRenderableFlagsForNonCollections, const std::vector<int>& activeMirrors);
    void setDefaultBufferValues(const float v[3]);
    void getDefaultBufferValues(float v[3]) const;

    void setUseExternalImage(bool u);
    bool getUseExternalImage() const;
    bool getInternalRendering() const;
    bool getApplyExternalRenderedImage() const;

    void setExtWindowSizeAndPos(int sizeX, int sizeY, int posX, int posY);
    void getExtWindowSizeAndPos(int& sizeX, int& sizeY, int& posX, int& posY) const;

    float* readPortionOfImage(int posX, int posY, int sizeX, int sizeY, int rgbGreyOrDepth) const;
    void writeImage(const float* buff, int rgbGreyOrDepth);
    unsigned char* readPortionOfCharImage(int posX, int posY, int sizeX, int sizeY, double cutoffRgba,
                                          int option) const;
    bool writePortionOfCharImage(const unsigned char* img, int posX, int posY, int sizeX, int sizeY, int option);

    void setUseEnvironmentBackgroundColor(bool s);
    bool getUseEnvironmentBackgroundColor() const;
    double getCalculationTime() const;
    std::string getDetectableEntityLoadAlias() const;
    std::string getDetectableEntityLoadName_old() const;

    CComposedFilter* getComposedFilter() const;
    void setComposedFilter(CComposedFilter* newFilter);
    CColorObject* getColor();

    SHandlingResult sensorResult;
    std::vector<std::vector<double>> sensorAuxiliaryResult; // e.g. vectors, etc. set by a filter or an extension module's filter

    bool setExternalImage_old(const float* img, bool imgIsGreyScale, bool noProcessing);
    bool setExternalCharImage_old(const unsigned char* img, bool imgIsGreyScale, bool noProcessing);

  protected:
    void _emitTriggerStateAndPacketChangeEvents(CCbor* thirdPartyEv = nullptr) const;
    void _emitImageChangedEvent(CCbor* thirdPartyEv = nullptr) const;
    void _emitDepthChangedEvent(CCbor* thirdPartyEv = nullptr) const;
    void _drawObjects(int entityID, bool detectAll, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                      bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections);
    int _getActiveMirrors(int entityID, bool detectAll,
                          bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                          bool overrideRenderableFlagsForNonCollections, int rendAttrib,
                          std::vector<int>& activeMirrors);

    void _reserveBuffers();
    void _clearBuffers();

    bool _computeDefaultReturnValuesAndApplyFilters();

    CSceneObject* _getInfoOfWhatNeedsToBeRendered(int entityID, bool detectAll, int rendAttrib,
                                                  bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                                  bool overrideRenderableFlagsForNonCollections,
                                                  std::vector<CSceneObject*>& toRender);
    CSceneObject* _getInfoOfWhatNeedsToBeRendered_old(
        int entityID, bool detectAll, int rendAttrib,
        bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
        bool overrideRenderableFlagsForNonCollections, std::vector<CSceneObject*>& toRender);

    bool _extRenderer_prepareView(int extRendererIndex);
    void _extRenderer_prepareLights();
    void _extRenderer_retrieveImage();

    unsigned char* _rgbBuffer;
    float* _depthBuffer;

    unsigned int _rayTracingTextureName;

    int _extWindowedViewSize[2];
    int _extWindowedViewPos[2];

    // Variables which need to be serialized & copied (don't forget the vars from the CViewableBase class!)
    CColorObject color;
    int _detectableEntityHandle;
    double _visionSensorSize;
    bool _explicitHandling;
    bool _useExternalImage;
    bool _useSameBackgroundAsEnvironment;
    float _defaultBufferValues[3];
    bool _ignoreRGBInfo;
    bool _ignoreDepthInfo;
    bool _computeImageBasicStats;
    int _renderMode; // 0=visible, 1=aux channels, 2=colorCodedID, 3=rayTracer, 4=rayTracer2, 5=extRenderer,
                     // 6=extRendererWindowed, 7=oculus, 8=oculusWindowed
    int _attributesForRendering;
    bool _inApplyFilterRoutine;

    CComposedFilter* _composedFilter;

    // Other variables:
    bool _initialExplicitHandling;
    std::string _detectableEntityLoadAlias;
    std::string _detectableEntityLoadName_old;
    bool _emitImageChangedEventEnabled;
    bool _emitDepthChangedEventEnabled;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
    void lookAt(CSView* viewObject, int viewPos[2] = nullptr, int viewSize[2] = nullptr);
    CTextureObject* getTextureObject();

    void createGlContextAndFboAndTextureObjectIfNeeded(bool useStencilBuffer);
    void createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(bool useStencilBuffer);

  protected:
    void _removeGlContextAndFboAndTextureObjectIfNeeded();
    void _handleMirrors(const std::vector<int>& activeMirrors, int entityID, bool detectAll,
                        bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, bool hideEdgesIfModel,
                        bool overrideRenderableFlagsForNonCollections);

    CVisionSensorGlStuff* _contextFboAndTexture;
#endif
};
