#pragma once

#include "sceneObject.h"

class CMirror : public CSceneObject
{
public:
    CMirror();
    virtual ~CMirror();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(floatDouble scalingFactor);
    void scaleObjectNonIsometrically(floatDouble x,floatDouble y,floatDouble z);
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

    void setMirrorWidth(floatDouble w);
    floatDouble getMirrorWidth();
    void setMirrorHeight(floatDouble h);
    floatDouble getMirrorHeight();
    void setReflectance(floatDouble r);
    floatDouble getReflectance();

    void setActive(bool a);
    bool getActive();
    void setIsMirror(bool m);
    bool getIsMirror();

    void setClippingObjectOrCollection(int co);
    int getClippingObjectOrCollection();

    bool getContainsTransparentComponent() const;
    CColorObject* getClipPlaneColor();

    // Variables which need to be serialized & copied
    floatDouble mirrorColor[3];
    static int currentMirrorContentBeingRendered;

protected:
    void _commonInit();

    // Variables which need to be serialized & copied
    CColorObject clipPlaneColor;
    floatDouble _mirrorWidth;
    floatDouble _mirrorHeight;
    floatDouble _mirrorReflectance;
    bool _active;
    bool _isMirror;
    int _clippingObjectOrCollection;

    // Variables which do not need to be serialized
    bool _initialMirrorActive;
};
