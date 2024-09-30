#pragma once

#include <sceneObject.h>
#include <viewableBase.h>
#include <global.h>
#include <light.h>
#include <sView.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propCamera_size,                    "cameraSize",                               sim_propertytype_float,     0, "Size", "Camera size") \
    FUNCX(propCamera_parentAsManipProxy,      "parentAsManipulationProxy",                sim_propertytype_bool,      0, "Parent as proxy", "Use parent as manipulation proxy") \
    FUNCX(propCamera_translationEnabled,      "translationEnabled",                       sim_propertytype_bool,      0, "Translation enabled", "") \
    FUNCX(propCamera_rotationEnabled,         "rotationEnabled",                          sim_propertytype_bool,      0, "Rotation enabled", "") \
    FUNCX(propCamera_trackedObjectHandle,     "trackedObjectHandle",                      sim_propertytype_int,       0, "Tracked object", "Tracked scene object handle") \

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_camera = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

const double ORTHO_CAMERA_NEAR_CLIPPING_PLANE = 0.0001;
const double ORTHO_CAMERA_FAR_CLIPPING_PLANE = 300.0;

enum
{
    DEPTHPASS = 0,
    RENDERPASS,
    PICKPASS,
    COLORCODEDPICKPASS
};

class CCamera : public CViewableBase
{
  public:
    CCamera();
    virtual ~CCamera();

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
    void setIsInScene(bool s);

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setIntVector2Property(const char* pName, const int* pState);
    int getIntVector2Property(const char* pName, int* pState) const;
    int setVector2Property(const char* pName, const double* pState);
    int getVector2Property(const char* pName, double* pState) const;
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
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt);
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    void handleCameraTracking();
    void commonInit();
    int getPerspectiveOperation_old() const;
    int getViewOrientation() const;
    void setViewOrientation(int ori, bool setPositionAlso = false);
    void setCameraSize(double size);
    double getCameraSize() const;
    void setAllowPicking(bool a);
    bool getAllowPicking() const;
    void setTrackedObjectHandle(int trackedObjHandle);
    int getTrackedObjectHandle() const;
    void setUseParentObjectAsManipulationProxy(bool useParent);
    bool getUseParentObjectAsManipulationProxy() const;

    void setAllowTranslation(bool allow);
    bool getAllowTranslation() const;
    void setAllowRotation(bool allow);
    bool getAllowRotation() const;

    bool getIsMainCamera();
    std::string getTrackedObjectLoadAlias() const;
    std::string getTrackedObjectLoadName_old() const;

    void shiftCameraInCameraManipulationMode(const C3Vector &newLocalPos);
    void rotateCameraInCameraManipulationMode(const C7Vector &newLocalConf);

    void frameSceneOrSelectedObjects(double windowWidthByHeight, bool forPerspectiveProjection,
                                     std::vector<int> *selectedObjects, bool useSystemSelection,
                                     bool includeModelObjects, double scalingFactor, CSView *optionalView);
    CColorObject *getColor(bool secondPart);

  protected:
    // Variables which need to be serialized & copied (don't forget the vars from the CViewableBase class!)
    double _cameraSize;
    int _renderMode;
    bool _viewModeSet_old; // with old cameras, the view mode (perspect./ortho) is decided by the view, not the camera.
                           // With old cameras, this is false
    bool _useParentObjectAsManipulationProxy;
    bool _allowPicking;
    bool _renderModeDuringSimulation;
    bool _renderModeDuringRecording;
    bool _allowTranslation;
    bool _allowRotation;
    CColorObject _color;
    CColorObject _color_removeSoon;

    // Various
    int _trackedObjectHandle;
    int hitForMouseUpProcessing_minus2MeansIgnore;
    int _attributesForRendering;
    unsigned int _textureNameForExtGeneratedView;
    bool _isMainCamera;
    std::string _trackedObjectLoadAlias;
    std::string _trackedObjectLoadName_old;

    static unsigned int selectBuff[SELECTION_BUFFER_SIZE * 4]; // For picking

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase *renderingObject, int displayAttrib);
    void lookIn(int windowSize[2], CSView *subView, bool drawText = false, bool passiveSubView = true);
    void setAttributesForRendering(int attr);
    int getAttributesForRendering() const;
    void setRenderMode(int mode, bool duringSimulation, bool duringRecording);
    int getRenderMode(bool *duringSimulation, bool *duringRecording) const;
    bool getInternalRendering() const;

  protected:
    void _drawObjects(int renderingMode, int pass, int currentWinSize[2], CSView *subView, bool mirrored);
    void _drawOverlay(bool passiveView, bool drawText, bool displ_ref, int windowSize[2], CSView *subView);

    int getSingleHit(int hits, unsigned int selectBuff[], bool ignoreDepthBuffer, int &hitThatIgnoresTheSelectableFlag);
    int getSeveralHits(int hits, unsigned int selectBuff[], std::vector<int> &hitList);
    int handleHits(int hits, unsigned int selectBuff[]);
    void handleMouseUpHit(int hitId);

    void performDepthPerception(CSView *subView, bool isPerspective);
    void _handleBannerClick(int bannerID);
    CSceneObject *_getInfoOfWhatNeedsToBeRendered(std::vector<CSceneObject *> &toRender);

    void _handleMirrors(int renderingMode, bool noSelection, int pass, int navigationMode, int currentWinSize[2],
                        CSView *subView);

    bool _extRenderer_prepareView(int extRendererIndex, int resolution[2], bool perspective);
    void _extRenderer_prepareLights();
    void _extRenderer_retrieveImage(char *rgbBuffer);
#endif
};
