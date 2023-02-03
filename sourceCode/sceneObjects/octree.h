#pragma once

#include <ser.h>
#include <sceneObject.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>

class CDummy;
class CPointCloud;

class COctree : public CSceneObject
{
public:

    COctree();
    virtual ~COctree();

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
    void setCellSizeForDisplay(double theNewSizeForDisplay);
    double getCellSizeForDisplay() const;

    void insertPoints(const double* pts,int ptsCnt,bool ptsAreRelativeToOctree,const unsigned char* optionalColors3,bool colorsAreIndividual,const unsigned int* optionalTags,unsigned int theTagWhenOptionalTagsIsNull);
    void insertShape(CShape* shape,unsigned int theTag);
    void insertOctree(const COctree* octree,unsigned int theTag);
    void insertDummy(const CDummy* dummy,unsigned int theTag);
    void insertPointCloud(const CPointCloud* pointCloud,unsigned int theTag);
    void insertOctree(const void* octree2Info,const C7Vector& octree2Tr,unsigned int theTag);
    void insertObjects(const std::vector<int>& sel);
    void insertObject(const CSceneObject* obj,unsigned int theTag);

    void subtractPoints(const double* pts,int ptsCnt,bool ptsAreRelativeToOctree);
    void subtractShape(CShape* shape);
    void subtractOctree(const COctree* octree);
    void subtractDummy(const CDummy* dummy);
    void subtractPointCloud(const CPointCloud* pointCloud);
    void subtractOctree(const void* octree2Info,const C7Vector& octree2Tr);
    void subtractObjects(const std::vector<int>& sel);
    void subtractObject(const CSceneObject* obj);

    void clear();
    bool getShowOctree() const;
    void setShowOctree(bool show);
    bool getUseRandomColors() const;
    void setUseRandomColors(bool r);
    bool getColorIsEmissive() const;
    void setColorIsEmissive(bool e);
    bool getUsePointsInsteadOfCubes() const;
    void setUsePointsInsteadOfCubes(bool r);
    bool getSaveCalculationStructure() const;
    void setSaveCalculationStructure(bool s);
    int getPointSize() const;
    void setPointSize(int s);
    const std::vector<double>* getCubePositions() const;
    std::vector<double>* getCubePositions();
    const void* getOctreeInfo() const;
    void* getOctreeInfo();
    void getTransfAndHalfSizeOfBoundingBox(C7Vector& tr,C3Vector& hs) const;
    CColorObject* getColor();

    void setVertexBufferId(int id);
    int getVertexBufferId() const;
    void setNormalBufferId(int id);
    int getNormalBufferId() const;
    float* getCubeVertices();
    float* getColors();

protected:
    void _updateOctreeEvent() const;
    void _readPositionsAndColorsAndSetDimensions();

    // Variables which need to be serialized & copied
    CColorObject color;
    double _cellSize;
    int _pointSize;
    void* _octreeInfo;
    std::vector<double> _voxelPositions;
    std::vector<float> _colors;
    std::vector<unsigned char> _colorsByte;
    bool _showOctreeStructure;
    bool _useRandomColors;
    bool _usePointsInsteadOfCubes;
    bool _saveCalculationStructure;
    bool _colorIsEmissive;

    // following only for display:
    float _cubeVertices[24*3];
    double _cellSizeForDisplay;
    int _vertexBufferId;
    int _normalBufferId;
};
