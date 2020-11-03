#pragma once

#include "sceneObject.h"
#include "pathCont.h"

class CPath : public CSceneObject  
{
public:

    CPath();
    virtual ~CPath();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    CSceneObject* copyYourself();
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
    void performObjectLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::vector<int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::vector<int>* map);
    void performDynMaterialObjectLoadingMapping(const std::vector<int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    bool getFullBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getMarkingBoundingBox(C3Vector& minV,C3Vector& maxV) const;
    bool getExportableMeshAtIndex(int index,std::vector<float>& vertices,std::vector<int>& indices) const;
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;

    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling();
    void resetPath();
    void handlePath(float deltaTime);

    void setShapingEnabled(bool isEnabled);
    bool getShapingEnabled();

    void setShapingFollowFullOrientation(bool full);
    bool getShapingFollowFullOrientation();
    void setShapingSectionClosed(bool closed);
    bool getShapingSectionClosed();
    void setShapingElementMaxLength(float l);
    float getShapingElementMaxLength();
    void setShapingType(int theType);
    int getShapingType();
    void setShapingThroughConvexHull(bool c);
    bool getShapingThroughConvexHull();
    CShape* getShape() const;

    void setShapingScaling(float s);
    float getShapingScaling() const;
    CColorObject* getShapingColor();

    // Variables which need to be copied and serialized:
    CPathCont* pathContainer;
    std::vector<float> shapingCoordinates;

    // Other variables:
    std::vector<float> _pathShapeVertices;
    std::vector<int> _pathShapeIndices;
    std::vector<float> _pathShapeNormals;

protected:
    void _generatePathShape();

    // Variables which need to be copied and serialized:
    CColorObject shapingColor;
    bool _explicitHandling;
    bool _shapingEnabled;

    bool _shapingFollowFullOrientation;
    bool _shapingSectionClosed;
    bool _shapingConvexHull;
    float _shapingElementMaxLength;
    int _shapingType;
    float _shapingScaling;

    // Other variables:
    unsigned short _pathModifID;

    bool _initialExplicitHandling;

#ifdef SIM_WITH_GUI
public:
    bool transformSelectedPathPoints(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,float prevPos[2],float pos[2],float screenHalfSizes[2],float halfSizes[2],bool perspective,int eventID);
#endif
};
