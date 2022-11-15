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
    void scaleVolume(floatDouble scalingFactor);
    void scaleVolumeNonIsometrically(floatDouble x,floatDouble y,floatDouble z,floatDouble& xRet,floatDouble& yRet,floatDouble& zRet);
    void serialize(CSer& ar);
    bool getVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    void disableVolumeComputation(bool disableIt);

    void commonInit();
    C4X4Matrix getTheMatrix(const C3Vector& pt0,const C3Vector& pt1,const C3Vector& pt2,bool tri);
    void add3Values(std::vector<floatDouble>& vect,const C4X4Matrix& transf,floatDouble x,floatDouble y,floatDouble z);
    void addAPlane(std::vector<floatDouble>* volume,std::vector<floatDouble>* normals,floatDouble nL,const C4X4Matrix& m,bool inside);
    void computeVolumeEdges(std::vector<floatDouble>& edges);
    void getCloseAndFarVolumeEdges(std::vector<floatDouble>& allEdges,floatDouble distance,std::vector<floatDouble>& closeEdges,std::vector<floatDouble>& farEdges);
    void removeEdgesNotInsideVolume(std::vector<floatDouble>& edges,std::vector<floatDouble>& planes,bool invertSides);
    void generateSphereEdges(std::vector<floatDouble>& edges,floatDouble radius);

    void computeVolumes();

    void setVolumeType(int theType,int objectTypeTheVolumeIsFor,floatDouble pointSize);
    int getVolumeType();
    void setOffset(floatDouble theOffset,bool recomputeVolume=true);
    floatDouble getOffset() const;
    void setRange(floatDouble theRange,bool recomputeVolume=true);
    floatDouble getRange() const;
    void setXSize(floatDouble theXSize,bool recomputeVolume=true);
    floatDouble getXSize() const;
    void setYSize(floatDouble theYSize,bool recomputeVolume=true);
    floatDouble getYSize() const;
    void setXSizeFar(floatDouble theXSizeFar,bool recomputeVolume=true);
    floatDouble getXSizeFar() const;
    void setYSizeFar(floatDouble theYSizeFar,bool recomputeVolume=true);
    floatDouble getYSizeFar() const;
    void setRadius(floatDouble theRadius,bool recomputeVolume=true);
    floatDouble getRadius() const;
    void setRadiusFar(floatDouble theRadiusFar,bool recomputeVolume=true);
    floatDouble getRadiusFar() const;
    void setAngle(floatDouble theAngle,bool recomputeVolume=true);
    floatDouble getAngle() const;
    void setInsideAngleThing(floatDouble theAngleThing,bool recomputeVolume=true);
    floatDouble getInsideAngleThing() const;
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
    void setSmallestDistanceAllowed(floatDouble d,bool recomputeVolume=true);
    floatDouble getSmallestDistanceAllowed() const;
    void setDefaultVolumeParameters(int objectTypeTheVolumeIsFor,floatDouble pointSize);
    void solveInterferences();
    
    // Variables which need to be serialized & copied
    std::vector<floatDouble> planesInside;// Detection volume is formed by the inside volume minus the outside volume
    std::vector<floatDouble> planesOutside;
    std::vector<floatDouble> normalsInside;
    std::vector<floatDouble> normalsOutside;
    std::vector<floatDouble> volumeEdges;
    std::vector<floatDouble> nonDetectingVolumeEdges;

protected:
    bool _volumeComputationTemporarilyDisabled;

    // Variables which need to be serialized & copied
    floatDouble _smallestDistanceAllowed;
    bool _smallestDistanceEnabled;
    int _volumeType;
    floatDouble offset;
    floatDouble range;
    floatDouble xSize;
    floatDouble ySize;
    floatDouble xSizeFar;
    floatDouble ySizeFar;
    floatDouble radius;
    floatDouble radiusFar;
    floatDouble angle;
    floatDouble insideAngleThing;
    int faceNumber;
    int faceNumberFar;
    int subdivisions;
    int subdivisionsFar;
};
