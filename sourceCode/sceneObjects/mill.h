#pragma once

#include <sceneObject.h>
#include <convexVolume.h>

class CMill : public CSceneObject  
{
public:

    CMill();
    CMill(int theType);
    virtual ~CMill();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void scaleObjectNonIsometrically(double x,double y,double z);
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
    bool getMillingVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;

    // Various
    void setMillableObject(int objectID);
    int getMillableObject() const;

    int handleMill(bool exceptExplicitHandling,double& milledSurface,double& milledVolume,bool justForInitialization);
    void resetMill(bool exceptExplicitHandling);

    void commonInit();
    bool getMilledSurface(double& surf) const;
    bool getMilledVolume(double& vol) const;
    bool getMilledCount(int& milledCount) const;


    void setSize(double newSize);
    double getSize() const;


    void setMillType(int theType);
    int getMillType() const;

    void setExplicitHandling(bool setExplicit);
    bool getExplicitHandling() const;

    double getCalculationTime() const;
    bool getMillDataIsValid() const;

    CColorObject* getColor(bool getActiveColor);

    // Variables which need to be serialized & copied
    CConvexVolume* convexVolume;

protected:

    // Variables which need to be serialized & copied
    CColorObject activeVolumeColor;
    CColorObject passiveVolumeColor;
    double _size;
    bool _explicitHandling;
    int _millType;
    int _millableObject; // scene object handle or collection handle
    bool _millDataValid;
    double _milledSurface;
    double _milledVolume;
    int _milledObjectCount;
    int _calcTimeInMs;

    bool _initialExplicitHandling;
};
