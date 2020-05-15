#pragma once

#include "ser.h"
#include "3Vector.h"

#define PROXSENSOR_MAX_FACE_NUMBER 1024 // Maximum faces of a disk or cone-type sensor
#define PROXSENSOR_MAX_SUBDIVISIONS 1024    // Maximum subdivisions of a cone-type sensor

enum { // IS SERIALIZED!!!!
    PYRAMID_TYPE_CONVEX_VOLUME=0,
    CYLINDER_TYPE_CONVEX_VOLUME,
    DISC_TYPE_CONVEX_VOLUME,
    CONE_TYPE_CONVEX_VOLUME,
    RAY_TYPE_CONVEX_VOLUME,
};

class CConvexVolume
{
public:

    CConvexVolume();
    virtual ~CConvexVolume();

    CConvexVolume* copyYourself();
    void scaleVolume(float scalingFactor);
    void scaleVolumeNonIsometrically(float x,float y,float z,float& xRet,float& yRet,float& zRet);
    void serialize(CSer& ar);
    bool getVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    void disableVolumeComputation(bool disableIt);

    void commonInit();
    C4X4Matrix getTheMatrix(const C3Vector& pt0,const C3Vector& pt1,const C3Vector& pt2,bool tri);
    void add3Values(std::vector<float>& vect,const C4X4Matrix& transf,float x,float y,float z);
    void addAPlane(std::vector<float>* volume,std::vector<float>* normals,float nL,const C4X4Matrix& m,bool inside);
    void computeVolumeEdges(std::vector<float>& edges);
    void getCloseAndFarVolumeEdges(std::vector<float>& allEdges,float distance,std::vector<float>& closeEdges,std::vector<float>& farEdges);
    void removeEdgesNotInsideVolume(std::vector<float>& edges,std::vector<float>& planes,bool invertSides);
    void generateSphereEdges(std::vector<float>& edges,float radius);

    void computeVolumes();

    void setVolumeType(int theType,int objectTypeTheVolumeIsFor,float pointSize);
    int getVolumeType();
    void setOffset(float theOffset,bool recomputeVolume=true);
    float getOffset() const;
    void setRange(float theRange,bool recomputeVolume=true);
    float getRange() const;
    void setXSize(float theXSize,bool recomputeVolume=true);
    float getXSize() const;
    void setYSize(float theYSize,bool recomputeVolume=true);
    float getYSize() const;
    void setXSizeFar(float theXSizeFar,bool recomputeVolume=true);
    float getXSizeFar() const;
    void setYSizeFar(float theYSizeFar,bool recomputeVolume=true);
    float getYSizeFar() const;
    void setRadius(float theRadius,bool recomputeVolume=true);
    float getRadius() const;
    void setRadiusFar(float theRadiusFar,bool recomputeVolume=true);
    float getRadiusFar() const;
    void setAngle(float theAngle,bool recomputeVolume=true);
    float getAngle() const;
    void setInsideAngleThing(float theAngleThing,bool recomputeVolume=true);
    float getInsideAngleThing() const;
    void setFaceNumber(int theFaceNumber,bool recomputeVolume=true);
    int getFaceNumber() const;
    void setFaceNumberFar(int theFaceNumberFar,bool recomputeVolume=true);
    int getFaceNumberFar() const;
    void setSubdivisions(int theSubdivisions,bool recomputeVolume=true);
    int getSubdivisions() const;
    void setSubdivisionsFar(int theSubdivisionsFar,bool recomputeVolume=true);
    int getSubdivisionsFar() const;
    void setSmallestDistanceEnabled(bool e,bool recomputeVolume=true);
    bool getSmallestDistanceEnabled() const;
    void setSmallestDistanceAllowed(float d,bool recomputeVolume=true);
    float getSmallestDistanceAllowed() const;
    void setDefaultVolumeParameters(int objectTypeTheVolumeIsFor,float pointSize);
    void solveInterferences();
    
    // Variables which need to be serialized & copied
    std::vector<float> planesInside;// Detection volume is formed by the inside volume minus the outside volume
    std::vector<float> planesOutside;
    std::vector<float> normalsInside;
    std::vector<float> normalsOutside;
    std::vector<float> volumeEdges;
    std::vector<float> nonDetectingVolumeEdges;

protected:
    bool _volumeComputationTemporarilyDisabled;

    // Variables which need to be serialized & copied
    float _smallestDistanceAllowed;
    bool _smallestDistanceEnabled;
    int _volumeType;
    float offset;
    float range;
    float xSize;
    float ySize;
    float xSizeFar;
    float ySizeFar;
    float radius;
    float radiusFar;
    float angle;
    float insideAngleThing;
    int faceNumber;
    int faceNumberFar;
    int subdivisions;
    int subdivisionsFar;
};
