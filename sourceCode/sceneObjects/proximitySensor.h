#pragma once

#include <sceneObject.h>
#include <convexVolume.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propProximitySensor_size,                    "size",                                     sim_propertytype_float,     0) \

#define FUNCX(name, str, v1, v2) const SProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<SProperty> allProps_proximitySensor = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CProxSensor : public CSceneObject
{
  public:
    CProxSensor();
    CProxSensor(int theType);
    virtual ~CProxSensor();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor *ev) const;
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
    bool getSensingVolumeBoundingBox(C3Vector &minV, C3Vector &maxV) const;
    void getSensingVolumeOBB(C7Vector &tr, C3Vector &halfSizes);
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState);
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance);
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, int& size);
    static int getPropertyInfo_static(const char* pName, int& info, int& size);

    // Various
    void setSensableObject(int objectID);
    int getSensableObject();

    bool handleSensor(bool exceptExplicitHandling, int &detectedObjectHandle, C3Vector &detectedNormalVector);
    void resetSensor(bool exceptExplicitHandling);
    int readSensor(C3Vector &detectPt, int &detectedObjectHandle, C3Vector &detectedNormalVector);

    void commonInit();
    bool getSensedData(C3Vector &pt);
    void setClosestObjectMode(bool closestObjMode);
    bool getClosestObjectMode();
    void setProxSensorSize(double newSize);
    double getProxSensorSize();

    void setAllowedNormal(double al);
    double getAllowedNormal() const;
    void setNormalCheck(bool check);
    bool getNormalCheck() const;
    bool getFrontFaceDetection() const;
    bool getBackFaceDetection() const;
    void setFrontFaceDetection(bool faceOn);
    void setBackFaceDetection(bool faceOn);
    void setShowDetectionVolume(bool show);

    int getSensorType() const;

    void setExplicitHandling(bool setExplicit);
    bool getExplicitHandling() const;

    void setSensableType(int theType);
    int getSensableType() const;

    void setShowVolume(bool s);
    bool getShowVolume() const;

    void setHideDetectionRay(bool hide);
    bool getHideDetectionRay() const;

    void setRandomizedDetection(bool enable);
    bool getRandomizedDetection() const;
    void setRandomizedDetectionSampleCount(int c);
    int getRandomizedDetectionSampleCount() const;
    void setRandomizedDetectionCountForDetection(int c);
    int getRandomizedDetectionCountForDetection() const;

    void calculateFreshRandomizedRays();
    const std::vector<C3Vector> *getPointerToRandomizedRays() const;
    std::vector<double> *getPointerToRandomizedRayDetectionStates();

    double getCalculationTime() const;
    C3Vector getDetectedPoint() const;
    bool getIsDetectedPointValid() const;
    std::string getSensableObjectLoadAlias() const;
    std::string getSensableObjectLoadName_old() const;

    CColorObject *getColor(int index);

    std::vector<double> cutEdges; // Only used for sensor cutting debugging

    CConvexVolume *convexVolume;

  protected:
    // Variables which need to be serialized & copied
    CColorObject volumeColor;
    CColorObject detectionRayColor;
    double allowedNormal;
    double _proxSensorSize;
    bool normalCheck;
    bool closestObjectMode;
    bool frontFaceDetection;
    bool backFaceDetection;
    bool explicitHandling;
    bool _showVolume;
    int sensorType;
    int _sensableType;
    bool displayNormals;
    int _sensableObject; // scene object handle or collection handle
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
    std::vector<double> _randomizedVectorDetectionStates;

    bool _initialExplicitHandling;

    bool _hideDetectionRay;
    std::string _sensableObjectLoadAlias;
    std::string _sensableObjectLoadName_old;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase *renderingObject, int displayAttrib);
#endif
};
