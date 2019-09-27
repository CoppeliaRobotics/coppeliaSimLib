
#pragma once

#include "3DObject.h"
#include "convexVolume.h"

class CProxSensor : public C3DObject  
{
public:

    CProxSensor();
    CProxSensor(int theType);
    virtual ~CProxSensor();

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
    bool getSensingVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    void getSensingVolumeOBB(C4X4Matrix& m,C3Vector& halfSizes);
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    bool isPotentiallyCuttable() const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;

    // Various
    void setSensableObject(int objectID);
    int getSensableObject();

    bool handleSensor(bool exceptExplicitHandling,int& detectedObjectHandle,C3Vector& detectedNormalVector);
    void resetSensor(bool exceptExplicitHandling);
    int readSensor(C3Vector& detectPt,int& detectedObjectHandle,C3Vector& detectedNormalVector);

    void commonInit();
    bool getSensedData(C3Vector& pt);
    void setClosestObjectMode(bool closestObjMode);
    bool getClosestObjectMode();
    void setSize(float newSize);
    float getSize();

    void setAllowedNormal(float al);
    float getAllowedNormal() const;
    void setNormalCheck(bool check);
    bool getNormalCheck() const;
    bool getFrontFaceDetection() const;
    bool getBackFaceDetection() const;
    void setFrontFaceDetection(bool faceOn);
    void setBackFaceDetection(bool faceOn);
    void setShowDetectionVolume(bool show);

    void setSensorType(int theType);
    int getSensorType() const;

    void setExplicitHandling(bool setExplicit);
    bool getExplicitHandling() const;

    void setSensableType(int theType);
    int getSensableType() const;

    void setShowVolumeWhenNotDetecting(bool s);
    bool getShowVolumeWhenNotDetecting() const;
    void setShowVolumeWhenDetecting(bool s);
    bool getShowVolumeWhenDetecting() const;

//    void setCheckOcclusions(bool c);
//    bool getCheckOcclusions() const;

    void setHideDetectionRay(bool hide);
    bool getHideDetectionRay() const;

    void setRandomizedDetection(bool enable);
    bool getRandomizedDetection() const;
    void setRandomizedDetectionSampleCount(int c);
    int getRandomizedDetectionSampleCount() const;
    void setRandomizedDetectionCountForDetection(int c);
    int getRandomizedDetectionCountForDetection() const;

    void calculateFreshRandomizedRays();
    const std::vector<C3Vector>* getPointerToRandomizedRays() const;
    std::vector<float>* getPointerToRandomizedRayDetectionStates();

    float getCalculationTime() const;
    C3Vector getDetectedPoint() const;
    bool getIsDetectedPointValid() const;

    CVisualParam* getColor(int index);

    std::vector<float> cutEdges; // Only used for sensor cutting debugging

    CConvexVolume* convexVolume;

protected:

    // Variables which need to be serialized & copied
    CVisualParam passiveVolumeColor;
    CVisualParam activeVolumeColor;
    CVisualParam detectionRayColor;
    CVisualParam closestDistanceVolumeColor;
    float allowedNormal;
    float size;
    bool normalCheck;
//    bool _checkOcclusions;
    bool closestObjectMode;
    bool frontFaceDetection;
    bool backFaceDetection;
    bool explicitHandling;
    bool _showVolumeWhenNotDetecting;
    bool _showVolumeWhenDetecting;
    int sensorType;
    int _sensableType;
    bool displayNormals;
    int _sensableObject; // 3DObject ID or collection ID
    C3Vector _detectedPoint;
    bool _detectedPointValid;
    bool _sensorResultValid;
    int _detectedObjectHandle;
    C3Vector _detectedNormalVector;
    int _calcTimeInMs;

    bool _randomizedDetection;
    int _randomizedDetectionSampleCount;
    int _randomizedDetectionCountForDetection;

    std::vector<C3Vector> _randomizedVectors;
    std::vector<float> _randomizedVectorDetectionStates;

    bool _initialValuesInitialized;
    bool _initialExplicitHandling;

    bool _hideDetectionRay;
};
