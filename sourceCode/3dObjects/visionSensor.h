
#pragma once

#include "viewableBase.h"
#include "sView.h"
#include "composedFilter.h"
#include "textureObject.h"
#ifdef SIM_WITH_OPENGL
#include "visionSensorGlStuff.h"
#endif

struct SHandlingResult
{
    bool sensorWasTriggered;
    bool sensorResultIsValid;
    unsigned char sensorDataRed[3]; // min, max, average
    unsigned char sensorDataGreen[3]; // min, max, average
    unsigned char sensorDataBlue[3]; // min, max, average
    unsigned char sensorDataIntensity[3]; // min, max, average
    float sensorDataDepth[3]; // min, max, average
    int calcTimeInMs;
};

class CVisionSensor : public CViewableBase  
{
public:
    CVisionSensor();
    virtual ~CVisionSensor();

    // Following functions are inherited from 3DObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    C3DObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void announceGcsObjectWillBeErased(int gcsObjectID,bool copyBuffer);
    void performObjectLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(std::vector<int>* map,bool loadingAmodel);
    void performGcsLoadingMapping(std::vector<int>* map);
    void performTextureObjectLoadingMapping(std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(std::vector<int>* map);
    void bufferMainDisplayStateVariables();
    void bufferedMainDisplayStateVariablesToDisplay();
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationIsRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    bool isPotentiallyCuttable() const;

    void commonInit();

    void getRealResolution(int r[2]);
    void setDesiredResolution(int r[2]);
    void getDesiredResolution(int r[2]);

    void setSize(const C3Vector& s);
    C3Vector getSize();
    void setPerspectiveOperation(bool p);
    bool getPerspectiveOperation();
    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling();
    void setShowVolumeWhenNotDetecting(bool s);
    bool getShowVolumeWhenNotDetecting();
    void setShowVolumeWhenDetecting(bool s);
    bool getShowVolumeWhenDetecting();
    void resetSensor();
    bool handleSensor();
    bool checkSensor(int entityID,bool overrideRenderableFlagsForNonCollections);
    float* checkSensorEx(int entityID,bool imageBuffer,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    bool setExternalImage(const float* img,bool imgIsGreyScale);
    bool setExternalCharImage(const unsigned char* img,bool imgIsGreyScale);

    void setIgnoreRGBInfo(bool ignore);
    bool getIgnoreRGBInfo();
    void setIgnoreDepthInfo(bool ignore);
    bool getIgnoreDepthInfo();

    void setRenderMode(int mode);
    int getRenderMode();

    void setAttributesForRendering(int attr);
    int getAttributesForRendering();

    void setComputeImageBasicStats(bool c);
    bool getComputeImageBasicStats();

    unsigned char* getRgbBufferPointer();
    float* getDepthBufferPointer();

    void getSensingVolumeCorners(C3Vector& sizeAndPosClose,C3Vector& sizeAndPosFar) const;
    void setDetectableEntityID(int entityID);
    int getDetectableEntityID();

    void detectVisionSensorEntity_executedViaUiThread(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    bool detectEntity(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    void detectEntity2(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    void renderForDetection(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections,const std::vector<int>& activeMirrors);
    void setDefaultBufferValues(const float v[3]);
    void getDefaultBufferValues(float v[3]);

    void setUseExternalImage(bool u);
    bool getUseExternalImage();
    bool getInternalRendering();
    bool getApplyExternalRenderedImage();

    void setExtWindowSizeAndPos(int sizeX,int sizeY,int posX,int posY);
    void getExtWindowSizeAndPos(int& sizeX,int& sizeY,int& posX,int& posY);

    float* readPortionOfImage(int posX,int posY,int sizeX,int sizeY,int rgbGreyOrDepth);
    unsigned char* readPortionOfCharImage(int posX,int posY,int sizeX,int sizeY,float cutoffRgba,bool imgIsGreyScale);

    void setUseEnvironmentBackgroundColor(bool s);
    bool getUseEnvironmentBackgroundColor();
    float getCalculationTime();

    CComposedFilter* getComposedFilter();
    void setComposedFilter(CComposedFilter* newFilter);
    CVisualParam* getColor(bool colorWhenActive);

    SHandlingResult sensorResult;
    std::vector<std::vector<float> > sensorAuxiliaryResult; // e.g. vectors, etc. set by a filter or an extension module's filter

protected:
    void _drawObjects(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    int _getActiveMirrors(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,int rendAttrib,std::vector<int>& activeMirrors);

    void _reserveBuffers();
    void _clearBuffers();

    bool _computeDefaultReturnValuesAndApplyFilters();

    C3DObject* _getInfoOfWhatNeedsToBeRendered(int entityID,bool detectAll,int rendAttrib,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,std::vector<C3DObject*>& toRender);

    bool _extRenderer_prepareView(int extRendererIndex);
    void _extRenderer_prepareLights();
    void _extRenderer_prepareMirrors();
    void _extRenderer_retrieveImage();

    unsigned char* _rgbBuffer;
    float* _depthBuffer;

    unsigned int _rayTracingTextureName;

    int _extWindowedViewSize[2];
    int _extWindowedViewPos[2];

    // Variables which need to be serialized & copied (don't forget the vars from the CViewableBase class!)
    CVisualParam color;
    CVisualParam activeColor;
    int _resolutionX;
    int _resolutionY;
    int _desiredResolution[2];
    int _detectableEntityID;
    C3Vector _size;
    bool _perspectiveOperation;
    bool _explicitHandling;
    bool _showVolumeWhenNotDetecting;
    bool _showVolumeWhenDetecting;
    bool _useExternalImage;
    bool _useSameBackgroundAsEnvironment;
    float _defaultBufferValues[3];
    bool _ignoreRGBInfo;
    bool _ignoreDepthInfo;
    bool _computeImageBasicStats;
    int _renderMode; // 0=visible, 1=aux channels, 2=colorCodedID, 3=rayTracer, 4=rayTracer2, 5=extRenderer, 6=extRendererWindowed, 7=oculus, 8=oculusWindowed
    int _attributesForRendering;


    CComposedFilter* _composedFilter;

    // Other variables:
    bool _initialValuesInitialized;
    bool _initialExplicitHandling;

#ifdef SIM_WITH_OPENGL
public:
    // Following function is inherited from 3DObject
    void lookAt(CSView* viewObject,int viewPos[2]=nullptr,int viewSize[2]=nullptr);
    CTextureObject* getTextureObject();

    void createGlContextAndFboAndTextureObjectIfNeeded(bool useStencilBuffer);
    void createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(bool useStencilBuffer);
protected:
    void _removeGlContextAndFboAndTextureObjectIfNeeded();
    void _handleMirrors(const std::vector<int>& activeMirrors,int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);

    CVisionSensorGlStuff* _contextFboAndTexture;
#endif
};
