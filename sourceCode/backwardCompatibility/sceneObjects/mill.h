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
    bool getMillingVolumeBoundingBox(C3Vector& minV, C3Vector& maxV) const;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;

    // Various
    void setMillableObject(int objectID);
    int getMillableObject() const;

    int handleMill(bool exceptExplicitHandling, double& milledSurface, double& milledVolume,
                   bool justForInitialization);
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

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
