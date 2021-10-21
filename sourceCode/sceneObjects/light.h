
#pragma once

#include "sceneObject.h"

class CLight : public CSceneObject
{
public:
    CLight();
    CLight(int theType);
    virtual ~CLight();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void pushCreationEvent(CInterfaceStackTable* ev=nullptr) const;
    CSceneObject* copyYourself();
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
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    void setLightActive(bool active);
    bool getLightActive();
    float getAttenuationFactor(short type);
    void setAttenuationFactor(float value,short type);
    void setLightSize(float size);
    float getLightSize();
    void setSpotExponent(int e);
    int getSpotExponent();
    void setSpotCutoffAngle(float co);
    float getSpotCutoffAngle();
    int getLightType();

    void setLightIsLocal(bool l);
    bool getLightIsLocal();

    static void setMaxAvailableOglLights(int c);
    static int getMaxAvailableOglLights();

    CColorObject* getColor(bool getLightColor);

protected:

    void _setDefaultColors();
    void _commonInit();

    // Variables which need to be serialized & copied
    CColorObject objectColor;
    CColorObject lightColor;
    bool lightActive;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    int _spotExponent;
    float _spotCutoffAngle;
    float _lightSize;
    int _lightType;
    bool _lightIsLocal;

    // Variables which do not need to be serialized
    bool _initialLightActive;
    static int _maximumOpenGlLights;
};
