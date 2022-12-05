#pragma once

#include "sceneObject.h"
#include "pathCont_old.h"

class CPath_old : public CSceneObject  
{
public:

    CPath_old();
    virtual ~CPath_old();

    // Following functions are inherited from CSceneObject
    void display(CViewableBase* renderingObject,int displayAttrib);
    void addSpecializedObjectEventData(CInterfaceStackTable* data) const;
    CSceneObject* copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void scaleObjectNonIsometrically(double x,double y,double z);
    void serialize(CSer& ar);
    void announceObjectWillBeErased(const CSceneObject* object,bool copyBuffer);
    void announceCollectionWillBeErased(int groupID,bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID,bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID,bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int,int>* map,bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int,int>* map);
    void performDynMaterialObjectLoadingMapping(const std::map<int,int>* map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    bool isPotentiallyCollidable() const;
    bool isPotentiallyMeasurable() const;
    bool isPotentiallyDetectable() const;
    bool isPotentiallyRenderable() const;
    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;

    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling();
    void resetPath();
    void handlePath(double deltaTime);

    void setShapingEnabled(bool isEnabled);
    bool getShapingEnabled();

    void setShapingFollowFullOrientation(bool full);
    bool getShapingFollowFullOrientation();
    void setShapingSectionClosed(bool closed);
    bool getShapingSectionClosed();
    void setShapingElementMaxLength(double l);
    double getShapingElementMaxLength();
    void setShapingType(int theType);
    int getShapingType();
    void setShapingThroughConvexHull(bool c);
    bool getShapingThroughConvexHull();
    CShape* getShape() const;

    void setShapingScaling(double s);
    double getShapingScaling() const;
    CColorObject* getShapingColor();

    // Variables which need to be copied and serialized:
    CPathCont_old* pathContainer;
    std::vector<double> shapingCoordinates;

    // Other variables:
    std::vector<double> _pathShapeVertices;
    std::vector<int> _pathShapeIndices;
    std::vector<double> _pathShapeNormals;

protected:
    void _generatePathShape();

    // Variables which need to be copied and serialized:
    CColorObject shapingColor;
    bool _explicitHandling;
    bool _shapingEnabled;

    bool _shapingFollowFullOrientation;
    bool _shapingSectionClosed;
    bool _shapingConvexHull;
    double _shapingElementMaxLength;
    int _shapingType;
    double _shapingScaling;

    // Other variables:
    unsigned short _pathModifID;

    bool _initialExplicitHandling;

#ifdef SIM_WITH_GUI
public:
    bool transformSelectedPathPoints(const C4X4Matrix& cameraAbsConf,const C3Vector& clicked3DPoint,double prevPos[2],double pos[2],double screenHalfSizes[2],double halfSizes[2],bool perspective,int eventID);
#endif
};
