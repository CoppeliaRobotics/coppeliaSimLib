
#pragma once

#include "3DObject.h"
#include "viewableBase.h"
#include "global.h"
#include "light.h"
#include "sView.h"

const float ORTHO_CAMERA_NEAR_CLIPPING_PLANE=0.0001f;
const float ORTHO_CAMERA_FAR_CLIPPING_PLANE=300.0f;

enum {  DEPTHPASS=0,
        RENDERPASS,
        PICKPASS,
        COLORCODEDPICKPASS
};

class CCamera : public CViewableBase  
{
public:
    CCamera();
    virtual ~CCamera();

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
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationIsRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    void bufferMainDisplayStateVariables();
    void bufferedMainDisplayStateVariablesToDisplay();

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    bool isPotentiallyCuttable() const;

    void handleTrackingAndHeadAlwaysUp();
    void commonInit();
    int getViewOrientation() const;
    void setViewOrientation(int ori,bool setPositionAlso=false);
    void setCameraSize(float size);
    float getCameraSize() const;
    void setAllowPicking(bool a);
    bool getAllowPicking() const;
    void setTrackedObjectID(int trackedObjID);
    int getTrackedObjectID() const;
    void setUseParentObjectAsManipulationProxy(bool useParent);
    bool getUseParentObjectAsManipulationProxy() const;
    void setCameraManipulationModePermissions(int p); // bit coded: own x, own y, own z, full rotation, tilting, never tilting
    int getCameraManipulationModePermissions() const; // bit coded: own x, own y, own z, full rotation, tilting, never tilting

    void shiftCameraInCameraManipulationMode(const C3Vector& newLocalPos);
    void rotateCameraInCameraManipulationMode(const C7Vector& newLocalConf);
    void tiltCameraInCameraManipulationMode(float tiltAmount);

    void frameSceneOrSelectedObjects(float windowWidthByHeight,bool forPerspectiveProjection,std::vector<int>* selectedObjects,bool useSystemSelection,bool includeModelObjects,float scalingFactor,CSView* optionalView);
    CVisualParam* getColor(bool secondPart);

protected:
    // Variables which need to be serialized & copied (don't forget the vars from the CViewableBase class!)
    float cameraSize;
    int _renderMode;
    bool _useParentObjectAsManipulationProxy;
    bool _allowPicking;
    bool _renderModeDuringSimulation;
    bool _renderModeDuringRecording;
    int _cameraManipulationModePermissions;
    CVisualParam colorPart1; // Part1 color
    CVisualParam colorPart2; // Part2 color

    // Various
    int trackedObjectIdentifier_NeverDirectlyTouch;
    int hitForMouseUpProcessing_minus2MeansIgnore;
    int _attributesForRendering;
    unsigned int _textureNameForExtGeneratedView;
    
    static unsigned int selectBuff[SELECTION_BUFFER_SIZE*4];    // For picking

    bool _initialValuesInitialized;

#ifdef SIM_WITH_GUI
public:
    void lookIn(int windowSize[2],CSView* subView,bool drawText=false,bool passiveSubView=true);
    void setAttributesForRendering(int attr);
    int getAttributesForRendering() const;
    void setRenderMode(int mode,bool duringSimulation,bool duringRecording);
    int getRenderMode(bool* duringSimulation,bool* duringRecording) const;
    bool getInternalRendering() const;

protected:
    void _drawObjects(int renderingMode,int pass,int currentWinSize[2],CSView* subView,bool mirrored);
    void _drawOverlay(bool passiveView,bool drawText,bool displ_ref,int windowSize[2],CSView* subView);

    int getSingleHit(int hits,unsigned int selectBuff[],bool ignoreDepthBuffer,int& hitThatIgnoresTheSelectableFlag);
    int getSeveralHits(int hits,unsigned int selectBuff[],std::vector<int>& hitList);
    int handleHits(int hits,unsigned int selectBuff[]);
    void handleMouseUpHit(int hitId);

    void performDepthPerception(CSView* subView,bool isPerspective);
    void _handleBannerClick(int bannerID);
    C3DObject* _getInfoOfWhatNeedsToBeRendered(std::vector<C3DObject*>& toRender);

    void _handleMirrors(int renderingMode,bool noSelection,int pass,int navigationMode,int currentWinSize[2],CSView* subView);

    bool _extRenderer_prepareView(int extRendererIndex,int resolution[2],bool perspective);
    void _extRenderer_prepareLights();
    void _extRenderer_prepareMirrors();
    void _extRenderer_retrieveImage(char* rgbBuffer);
#endif
};
