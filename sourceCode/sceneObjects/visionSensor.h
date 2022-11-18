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
    floatFloat sensorDataDepth[3]; // min, max, average
    int calcTimeInMs;
};

class CVisionSensor : public CViewableBase  
{
public:
    CVisionSensor();
    virtual ~CVisionSensor();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void serialize(CSer& ar);
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
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

    void commonInit();

    void setResolution(const int r[2]); // override

    void setVisionSensorSize(float s);
    float getVisionSensorSize() const;
    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling() const;
    void resetSensor();
    bool handleSensor();
    bool checkSensor(int entityID,bool overrideRenderableFlagsForNonCollections);
    floatDouble* checkSensorEx(int entityID,bool imageBuffer,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    void setDepthBuffer(const floatFloat* img);

    void setIgnoreRGBInfo(bool ignore);
    bool getIgnoreRGBInfo() const;
    void setIgnoreDepthInfo(bool ignore);
    bool getIgnoreDepthInfo() const;

    void setRenderMode(int mode);
    int getRenderMode() const;

    void setAttributesForRendering(int attr);
    int getAttributesForRendering() const;

    void setComputeImageBasicStats(bool c);
    bool getComputeImageBasicStats() const;

    unsigned char* getRgbBufferPointer();
    floatFloat* getDepthBufferPointer() const;

    void setDetectableEntityHandle(int entityHandle);
    int getDetectableEntityHandle() const;

    void detectVisionSensorEntity_executedViaUiThread(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    bool detectEntity(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    void detectEntity2(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    void renderForDetection(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections,const std::vector<int>& activeMirrors);
    void setDefaultBufferValues(const float v[3]);
    void getDefaultBufferValues(float v[3]) const;

    void setUseExternalImage(bool u);
    bool getUseExternalImage() const;
    bool getInternalRendering() const;
    bool getApplyExternalRenderedImage() const;

    void setExtWindowSizeAndPos(int sizeX,int sizeY,int posX,int posY);
    void getExtWindowSizeAndPos(int& sizeX,int& sizeY,int& posX,int& posY) const;

    floatDouble* readPortionOfImage(int posX,int posY,int sizeX,int sizeY,int rgbGreyOrDepth) const;
    unsigned char* readPortionOfCharImage(int posX,int posY,int sizeX,int sizeY,floatDouble cutoffRgba,int option) const;
    bool writePortionOfCharImage(const unsigned char* img,int posX,int posY,int sizeX,int sizeY,int option);

    void setUseEnvironmentBackgroundColor(bool s);
    bool getUseEnvironmentBackgroundColor() const;
    float getCalculationTime() const;
    std::string getDetectableEntityLoadAlias() const;
    std::string getDetectableEntityLoadName_old() const;

    CComposedFilter* getComposedFilter() const;
    void setComposedFilter(CComposedFilter* newFilter);
    CColorObject* getColor();

    SHandlingResult sensorResult;
    std::vector<std::vector<floatDouble>> sensorAuxiliaryResult; // e.g. vectors, etc. set by a filter or an extension module's filter

    bool setExternalImage_old(const floatFloat* img,bool imgIsGreyScale,bool noProcessing);
    bool setExternalCharImage_old(const unsigned char* img,bool imgIsGreyScale,bool noProcessing);

protected:
    void _drawObjects(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool hideEdgesIfModel,bool overrideRenderableFlagsForNonCollections);
    int _getActiveMirrors(int entityID,bool detectAll,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,int rendAttrib,std::vector<int>& activeMirrors);

    void _reserveBuffers();
    void _clearBuffers();

    bool _computeDefaultReturnValuesAndApplyFilters();

    CSceneObject* _getInfoOfWhatNeedsToBeRendered(int entityID,bool detectAll,int rendAttrib,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,std::vector<CSceneObject*>& toRender);
    CSceneObject* _getInfoOfWhatNeedsToBeRendered_old(int entityID,bool detectAll,int rendAttrib,bool entityIsModelAndRenderAllVisibleModelAlsoNonRenderableObjects,bool overrideRenderableFlagsForNonCollections,std::vector<CSceneObject*>& toRender);

    bool _extRenderer_prepareView(int extRendererIndex);
    void _extRenderer_prepareLights();
    void _extRenderer_prepareMirrors();
    void _extRenderer_retrieveImage();

    unsigned char* _rgbBuffer;
    floatFloat* _depthBuffer;

    unsigned int _rayTracingTextureName;

    int _extWindowedViewSize[2];
    int _extWindowedViewPos[2];

    // Variables which need to be serialized & copied (don't forget the vars from the CViewableBase class!)
    CColorObject color;
    int _detectableEntityHandle;
    float _visionSensorSize;
    bool _explicitHandling;
    bool _useExternalImage;
    bool _useSameBackgroundAsEnvironment;
    float _defaultBufferValues[3];
    bool _ignoreRGBInfo;
    bool _ignoreDepthInfo;
    bool _computeImageBasicStats;
    int _renderMode; // 0=visible, 1=aux channels, 2=colorCodedID, 3=rayTracer, 4=rayTracer2, 5=extRenderer, 6=extRendererWindowed, 7=oculus, 8=oculusWindowed
    int _attributesForRendering;
    bool _inApplyFilterRoutine;


    CComposedFilter* _composedFilter;

    // Other variables:
    bool _initialExplicitHandling;
    std::string _detectableEntityLoadAlias;
    std::string _detectableEntityLoadName_old;

#ifdef SIM_WITH_OPENGL
public:
    // Following function is inherited from CSceneObject
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
