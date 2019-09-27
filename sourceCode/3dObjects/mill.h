
#pragma once

#include "3DObject.h"
#include "convexVolume.h"

class CMill : public C3DObject  
{
public:

    CMill();
    CMill(int theType);
    virtual ~CMill();

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
    bool getMillingVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    bool isPotentiallyCuttable() const;

    // Various
    void setMillableObject(int objectID);
    int getMillableObject() const;

    int handleMill(bool exceptExplicitHandling,float& milledSurface,float& milledVolume,bool justForInitialization);
    void resetMill(bool exceptExplicitHandling);

    void commonInit();
    bool getMilledSurface(float& surf) const;
    bool getMilledVolume(float& vol) const;
    bool getMilledCount(int& milledCount) const;


    void setSize(float newSize);
    float getSize() const;


    void setMillType(int theType);
    int getMillType() const;

    void setExplicitHandling(bool setExplicit);
    bool getExplicitHandling() const;

    float getCalculationTime() const;
    bool getMillDataIsValid() const;

    CVisualParam* getColor(bool getActiveColor);

    // Variables which need to be serialized & copied
    CConvexVolume* convexVolume;

protected:

    // Variables which need to be serialized & copied
    CVisualParam activeVolumeColor;
    CVisualParam passiveVolumeColor;
    float _size;
    bool _explicitHandling;
    int _millType;
    int _millableObject; // 3DObject ID or collection ID
    bool _millDataValid;
    float _milledSurface;
    float _milledVolume;
    int _milledObjectCount;
    int _calcTimeInMs;

    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
};
