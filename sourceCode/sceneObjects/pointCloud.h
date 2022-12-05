#pragma once

#include "ser.h"
#include "sceneObject.h"
#include "3Vector.h"
#include "7Vector.h"

class CDummy;
class COctree;

class CPointCloud : public CSceneObject
{
public:

    CPointCloud();
    virtual ~CPointCloud();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void scaleObjectNonIsometrically(double x,double y,double z);
    void serialize(CSer& ar);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
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
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);

    void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);

    // Various functions
    void setCellSize(double theNewSize);
    double getCellSize() const;
    void setMaxPointCountPerCell(int cnt);
    int getMaxPointCountPerCell() const;
    void insertPoints(const double* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,const unsigned char* optionalColors3,bool colorsAreIndividual);
    void insertShape(CShape* shape);
    void insertOctree(const COctree* octree);
    void insertDummy(const CDummy* dummy);
    void insertPointCloud(const CPointCloud* pointCloud);
    void insertObjects(const std::vector<int>& sel);
    void insertObject(const CSceneObject* obj);

    int removePoints(const double* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,double distanceTolerance);
    void subtractOctree(const COctree* octree);
    void subtractDummy(const CDummy* dummy,double distanceTolerance);
    void subtractPointCloud(const CPointCloud* pointCloud,double distanceTolerance);
    void subtractOctree(const void* octree2Info,const C7Vector& octree2Tr);
    void subtractObjects(const std::vector<int>& sel);
    void subtractObject(const CSceneObject* obj,double distanceTolerance);

    int intersectPoints(const double* pts,int ptsCnt,bool ptsAreRelativeToPointCloud,double distanceTolerance);

    void clear();
    bool getShowOctree() const;
    void setShowOctree(bool show);
    double getAveragePointCountInCell();
    int getPointSize() const;
    void setPointSize(int s);
    double getBuildResolution() const;
    void setBuildResolution(double r);
    double getRemovalDistanceTolerance() const;
    void setRemovalDistanceTolerance(double t);
    double getInsertionDistanceTolerance() const;
    void setInsertionDistanceTolerance(double t);
    bool getUseRandomColors() const;
    void setUseRandomColors(bool r);
    bool getColorIsEmissive() const;
    void setColorIsEmissive(bool e);
    bool getSaveCalculationStructure() const;
    void setSaveCalculationStructure(bool s);
    bool getDoNotUseCalculationStructure() const;
    void setDoNotUseCalculationStructure(bool s);
    double getPointDisplayRatio() const;
    void setPointDisplayRatio(double r);
    const std::vector<double>* getPoints() const;
    std::vector<double>* getPoints();
    const void* getPointCloudInfo() const;
    void* getPointCloudInfo();
    void getTransfAndHalfSizeOfBoundingBox(C7Vector& tr,C3Vector& hs) const;

    CColorObject* getColor();
    std::vector<double>* getColors();
    std::vector<double>* getDisplayPoints();
    std::vector<double>* getDisplayColors();

protected:
    void _updatePointCloudEvent() const;
    void _readPositionsAndColorsAndSetDimensions();
    void _getCharRGB3Colors(const std::vector<double>& floatRGBA,std::vector<unsigned char>& charRGB);

    // Variables which need to be serialized & copied
    CColorObject color;
    double _cellSize;
    int _maxPointCountPerCell;
    void* _pointCloudInfo;
    std::vector<double> _points;
    std::vector<double> _colors;
    std::vector<double> _displayPoints;
    std::vector<double> _displayColors;
    std::vector<unsigned char> _displayColorsByte;
    bool _showOctreeStructure;
    bool _useRandomColors;
    bool _saveCalculationStructure;
    int _pointSize;
    int _nonEmptyCells;
    double _buildResolution;
    double _removalDistanceTolerance;
    double _insertionDistanceTolerance;
    double _pointDisplayRatio;
    bool _doNotUseOctreeStructure;
    bool _colorIsEmissive;
};
