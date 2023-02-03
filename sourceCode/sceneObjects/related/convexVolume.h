#pragma once

#include <ser.h>
#include <simMath/3Vector.h>

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
    void scaleVolume(double scalingFactor);
    void scaleVolumeNonIsometrically(double x,double y,double z,double& xRet,double& yRet,double& zRet);
    void serialize(CSer& ar);
    bool getVolumeBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    void disableVolumeComputation(bool disableIt);

    void commonInit();
    C4X4Matrix getTheMatrix(const C3Vector& pt0,const C3Vector& pt1,const C3Vector& pt2,bool tri);
    void add3Values(std::vector<double>& vect,const C4X4Matrix& transf,double x,double y,double z);
    void addAPlane(std::vector<double>* volume,std::vector<double>* normals,double nL,const C4X4Matrix& m,bool inside);
    void computeVolumeEdges(std::vector<double>& edges);
    void getCloseAndFarVolumeEdges(std::vector<double>& allEdges,double distance,std::vector<double>& closeEdges,std::vector<double>& farEdges);
    void removeEdgesNotInsideVolume(std::vector<double>& edges,std::vector<double>& planes,bool invertSides);
    void generateSphereEdges(std::vector<double>& edges,double radius);

    void computeVolumes();

    void setVolumeType(int theType,int objectTypeTheVolumeIsFor,double pointSize);
    int getVolumeType();
    void setOffset(double theOffset,bool recomputeVolume=true);
    double getOffset() const;
    void setRange(double theRange,bool recomputeVolume=true);
    double getRange() const;
    void setXSize(double theXSize,bool recomputeVolume=true);
    double getXSize() const;
    void setYSize(double theYSize,bool recomputeVolume=true);
    double getYSize() const;
    void setXSizeFar(double theXSizeFar,bool recomputeVolume=true);
    double getXSizeFar() const;
    void setYSizeFar(double theYSizeFar,bool recomputeVolume=true);
    double getYSizeFar() const;
    void setRadius(double theRadius,bool recomputeVolume=true);
    double getRadius() const;
    void setRadiusFar(double theRadiusFar,bool recomputeVolume=true);
    double getRadiusFar() const;
    void setAngle(double theAngle,bool recomputeVolume=true);
    double getAngle() const;
    void setInsideAngleThing(double theAngleThing,bool recomputeVolume=true);
    double getInsideAngleThing() const;
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
    void setSmallestDistanceAllowed(double d,bool recomputeVolume=true);
    double getSmallestDistanceAllowed() const;
    void setDefaultVolumeParameters(int objectTypeTheVolumeIsFor,double pointSize);
    void solveInterferences();
    
    // Variables which need to be serialized & copied
    std::vector<double> planesInside;// Detection volume is formed by the inside volume minus the outside volume
    std::vector<double> planesOutside;
    std::vector<double> normalsInside;
    std::vector<double> normalsOutside;
    std::vector<double> volumeEdges;
    std::vector<double> nonDetectingVolumeEdges;

protected:
    bool _volumeComputationTemporarilyDisabled;

    // Variables which need to be serialized & copied
    double _smallestDistanceAllowed;
    bool _smallestDistanceEnabled;
    int _volumeType;
    double offset;
    double range;
    double xSize;
    double ySize;
    double xSizeFar;
    double ySizeFar;
    double radius;
    double radiusFar;
    double angle;
    double insideAngleThing;
    int faceNumber;
    int faceNumberFar;
    int subdivisions;
    int subdivisionsFar;
};
