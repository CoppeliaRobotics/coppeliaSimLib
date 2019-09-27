
#pragma once

#include "3DObject.h"

class CMirror : public C3DObject
{
public:
    CMirror();
    virtual ~CMirror();

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

    void setMirrorWidth(float w);
    float getMirrorWidth();
    void setMirrorHeight(float h);
    float getMirrorHeight();
    void setReflectance(float r);
    float getReflectance();

    void setActive(bool a);
    bool getActive();
    void setIsMirror(bool m);
    bool getIsMirror();

    void setClippingObjectOrCollection(int co);
    int getClippingObjectOrCollection();

    bool getContainsTransparentComponent() const;
    CVisualParam* getClipPlaneColor();

    // Variables which need to be serialized & copied
    float mirrorColor[3];
    static int currentMirrorContentBeingRendered;

protected:
    void _commonInit();

    // Variables which need to be serialized & copied
    CVisualParam clipPlaneColor;
    float _mirrorWidth;
    float _mirrorHeight;
    float _mirrorReflectance;
    bool _active;
    bool _isMirror;
    int _clippingObjectOrCollection;

    // Variables which do not need to be serialized
    bool _initialValuesInitialized;
    bool _initialMirrorActive;
};
