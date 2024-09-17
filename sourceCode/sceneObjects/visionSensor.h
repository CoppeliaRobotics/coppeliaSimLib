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
#define DEFINE_PROPERTIES \
    FUNCX(propVisionSensor_size,                    "size",                                     sim_propertytype_float,   0) \
    FUNCX(propVisionSensor_backgroundCol,           "backgroundColor",                          sim_propertytype_color,   0) \
    FUNCX(propVisionSensor_renderMode,              "renderMode",                               sim_propertytype_int,     0) \
    FUNCX(propVisionSensor_backgroundSameAsEnv,     "backgroundColorFromEnvironment",           sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_explicitHandling,        "explicitHandling",                         sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_useExtImage,             "useExtImage",                              sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_ignoreRgbInfo,           "ignoreImageInfo",                          sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_ignoreDepthInfo,         "ignoreDepthInfo",                          sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_omitPacket1,             "omitPacket1",                              sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_emitImageChangedEvent,   "emitImageChangedEvent",                    sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_emitDepthChangedEvent,   "emitDepthChangedEvent",                    sim_propertytype_bool,    0) \
    FUNCX(propVisionSensor_imageBuffer,             "imageBuffer",                              sim_propertytype_buffer,  0) \
    FUNCX(propVisionSensor_depthBuffer,             "depthBuffer",                              sim_propertytype_vector,  sim_propertyinfo_notwritable) \

#define FUNCX(name, str, v1, v2) const SProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<SProperty> allProps_visionSensor = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CVisionSensor : public CViewableBase
{
  public:
    CVisionSensor();
    virtual ~CVisionSensor();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor *ev);
    CSceneObject *copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void serialize(CSer &ar);
    void announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer);
    void announceCollectionWillBeErased(int groupID, bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int, int> *map);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int> *map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    void setIsInScene(bool s);
    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(const char* pName, std::string& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setVectorProperty(const char* pName, const double* v, int vL);
    int getVectorProperty(const char* pName, std::vector<double>& pState) const;
    int setIntVectorProperty(const char* pName, const int* v, int vL);
    int getIntVectorProperty(const char* pName, std::vector<int>& pState) const;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance);
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info);
    static int getPropertyInfo_static(const char* pName, int& info);

    void commonInit();

    void setResolution(const int r[2]); // override

    void setVisionSensorSize(double s);
    double getVisionSensorSize() const;
    void setExplicitHandling(bool eh);
    bool getExplicitHandling() const;
    void resetSensor();
    bool handleSensor();
    bool checkSensor(int entityID, bool overrideRenderableFlagsForNonCollections);
    float *checkSensorEx(int entityID, bool imageBuffer,
                         bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, bool hideEdgesIfModel,
                         bool overrideRenderableFlagsForNonCollections);
    void setDepthBuffer(const float *img);

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

    unsigned char *getRgbBufferPointer();
    float *getDepthBufferPointer() const;

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
                            bool overrideRenderableFlagsForNonCollections, const std::vector<int> &activeMirrors);
    void setDefaultBufferValues(const float v[3]);
    void getDefaultBufferValues(float v[3]) const;

    void setUseExternalImage(bool u);
    bool getUseExternalImage() const;
    bool getInternalRendering() const;
    bool getApplyExternalRenderedImage() const;

    void setExtWindowSizeAndPos(int sizeX, int sizeY, int posX, int posY);
    void getExtWindowSizeAndPos(int &sizeX, int &sizeY, int &posX, int &posY) const;

    float *readPortionOfImage(int posX, int posY, int sizeX, int sizeY, int rgbGreyOrDepth) const;
    void writeImage(const float *buff, int rgbGreyOrDepth);
    unsigned char *readPortionOfCharImage(int posX, int posY, int sizeX, int sizeY, double cutoffRgba,
                                          int option) const;
    bool writePortionOfCharImage(const unsigned char *img, int posX, int posY, int sizeX, int sizeY, int option);

    void setUseEnvironmentBackgroundColor(bool s);
    bool getUseEnvironmentBackgroundColor() const;
    double getCalculationTime() const;
    std::string getDetectableEntityLoadAlias() const;
    std::string getDetectableEntityLoadName_old() const;

    CComposedFilter *getComposedFilter() const;
    void setComposedFilter(CComposedFilter *newFilter);
    CColorObject *getColor();

    SHandlingResult sensorResult;
    std::vector<std::vector<double>>
        sensorAuxiliaryResult; // e.g. vectors, etc. set by a filter or an extension module's filter

    bool setExternalImage_old(const float *img, bool imgIsGreyScale, bool noProcessing);
    bool setExternalCharImage_old(const unsigned char *img, bool imgIsGreyScale, bool noProcessing);

  protected:
    void _emitImageChangedEvent() const;
    void _emitDepthChangedEvent() const;
    void _drawObjects(int entityID, bool detectAll, bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                      bool hideEdgesIfModel, bool overrideRenderableFlagsForNonCollections);
    int _getActiveMirrors(int entityID, bool detectAll,
                          bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                          bool overrideRenderableFlagsForNonCollections, int rendAttrib,
                          std::vector<int> &activeMirrors);

    void _reserveBuffers();
    void _clearBuffers();

    bool _computeDefaultReturnValuesAndApplyFilters();

    CSceneObject *_getInfoOfWhatNeedsToBeRendered(int entityID, bool detectAll, int rendAttrib,
                                                  bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
                                                  bool overrideRenderableFlagsForNonCollections,
                                                  std::vector<CSceneObject *> &toRender);
    CSceneObject *_getInfoOfWhatNeedsToBeRendered_old(
        int entityID, bool detectAll, int rendAttrib,
        bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,
        bool overrideRenderableFlagsForNonCollections, std::vector<CSceneObject *> &toRender);

    bool _extRenderer_prepareView(int extRendererIndex);
    void _extRenderer_prepareLights();
    void _extRenderer_retrieveImage();

    unsigned char *_rgbBuffer;
    float *_depthBuffer;

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

    CComposedFilter *_composedFilter;

    // Other variables:
    bool _initialExplicitHandling;
    std::string _detectableEntityLoadAlias;
    std::string _detectableEntityLoadName_old;
    bool _emitImageChangedEventEnabled;
    bool _emitDepthChangedEventEnabled;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase *renderingObject, int displayAttrib);
    void lookAt(CSView *viewObject, int viewPos[2] = nullptr, int viewSize[2] = nullptr);
    CTextureObject *getTextureObject();

    void createGlContextAndFboAndTextureObjectIfNeeded(bool useStencilBuffer);
    void createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(bool useStencilBuffer);

  protected:
    void _removeGlContextAndFboAndTextureObjectIfNeeded();
    void _handleMirrors(const std::vector<int> &activeMirrors, int entityID, bool detectAll,
                        bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects, bool hideEdgesIfModel,
                        bool overrideRenderableFlagsForNonCollections);

    CVisionSensorGlStuff *_contextFboAndTexture;
#endif
};
