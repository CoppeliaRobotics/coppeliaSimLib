#pragma once

#include <sceneObject.h>
#include <pathCont_old.h>

class CPath_old : public CSceneObject
{
  public:
    CPath_old();
    virtual ~CPath_old();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    bool isPotentiallyRenderable() const override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;

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
    void display(CViewableBase* renderingObject, int displayAttrib) override;
    bool transformSelectedPathPoints(const C4X4Matrix& cameraAbsConf, const C3Vector& clicked3DPoint, double prevPos[2],
                                     double pos[2], double screenHalfSizes[2], double halfSizes[2], bool perspective,
                                     int eventID);
#endif
};
