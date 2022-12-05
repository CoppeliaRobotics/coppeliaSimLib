#pragma once

#include "simplePathPoint_old.h"
#include "bezierPathPoint_old.h"
#include "colorObject.h"

class CPath_old;

class CPathCont_old
{
public:
    CPathCont_old();
    virtual ~CPathCont_old();

    void serialize(CSer& ar);
    CPathCont_old* copyYourself();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationEnded();


    // Simple path and bezier path point routines:
    void removeAllSimplePathPoints();                               // Triggers an actualizePath() call
    void addSimplePathPoint(CSimplePathPoint_old* it);                  // Triggers an actualizePath() call
    void insertSimplePathPoint(CSimplePathPoint_old* it,int position);  // Triggers an actualizePath() call
    void removeSimplePathPoint(int position);                       // Triggers an actualizePath() call
    void rollPathPoints(bool forward);                              // Triggers an actualizePath() call

    CSimplePathPoint_old* getSimplePathPoint(int position);
    CBezierPathPoint_old* getBezierPathPoint(int position);
    int getSimplePathPointCount();
    int getBezierPathPointCount();
    void copyPointsToClipboard();
    void createEquivalent(int pathHandle);

    void enableActualization(bool enable);
    void actualizePath();

    void setAttributes(int attrib);                                 // Triggers an actualizePath() call
    int getAttributes();

    // Visual aspect:
    void setLineSize(int size);
    int getLineSize();
    void setSquareSize(double size);
    double getSquareSize();

    void setPosition(double p);
    double getPosition();

    double getPosition_linearComponentOnly();
    double getPosition_angularComponentOnly();


    void setNominalVelocity(double v);
    double getNominalVelocity();
    void setTargetNominalVelocity(double v);
    double getTargetNominalVelocity();
    void setAngleVarToDistanceCoeff(double f);               // Triggers an actualizePath call!
    double getAngleVarToDistanceCoeff();
    void setOnSpotDistanceToDistanceCoeff(double f);             // Triggers an actualizePath call!
    double getOnSpotDistanceToDistanceCoeff();

    void setMaxAcceleration(double maxAccel);
    double getMaxAcceleration();

//****************************

    void scaleObject(double scalingFactor);
    void scaleObjectNonIsometrically(double x,double y,double z);
    void resetPath(CPath_old* it);
    void handlePath(CPath_old* it,double deltaTime);
    void handlePath_keepObjectUnchanged(double deltaTime,double& pos_,double& nomVel_,double& targetNomVel_,double maxAccel,bool invertVelocity,bool infiniteAccel); // so that the path can be handled by simFollowPath functions or such!
    double getBezierVirtualPathLength();
    double getBezierNormalPathLength();
    double getBezierAngularPathLength();
    bool getConfigurationOnBezierCurveClosestTo(C3Vector& pt,C7Vector& conf);
    
    bool getPositionOnPathClosestTo(const C3Vector& pt,double& distOnPath);

    unsigned short getPathModifID();

    bool getPointOnBezierCurveAtNormalDistance(double& l,int& index0,double& t);
    C7Vector _getInterpolatedBezierCurvePoint(int index0,double t);
    void _getInterpolatedBezierCurveData(int index0,double t,int& auxFlags,double auxChannels[4]);

    bool getTransformationOnBezierCurveAtNormalizedVirtualDistance(double l,C7Vector& tr);
    bool getAuxDataOnBezierCurveAtNormalizedVirtualDistance(double l,int& auxFlags,double auxChannels[4]);

    bool invertSimplePathPointOrder(const std::vector<int>& selectedPoints);

    void setAvpTurningCircleRadiusForHalfVelocity(double radius);
    double getAvpTurningCircleRadiusForHalfVelocity();
    void setAvpRelativeVelocityAtRotationAxisChange(double relVel);
    double getAvpRelativeVelocityAtRotationAxisChange();
    void setAvpRelativeAcceleration(double relAccel);
    double getAvpRelativeAcceleration();

    void setPathLengthCalculationMethod(int m);                             // Triggers an actualizePath()!
    int getPathLengthCalculationMethod();


    CColorObject _lineColor;

protected:
    int _removeDoubleBezierPoints(double linTol,double angTol);               // Called from actualizePath()-routine
    void _computeAutomaticOrientation(const std::vector<CPathPoint_old*>& pc);  // Called from actualizePath()-routine
    void _recomputeBezierPathElementLengths();                              // Called from actualizePath()-routine
    void _recomputeBezierPathMaxVelocities();                               // Called from actualizePath()-routine

    bool _getNextBezierPathPointIndex(int currentIndex,int& nextIndex);
    bool _getPointOnBezierCurveAtVirtualDistance(double& l,int& index0,double& t);

    bool _getBezierPointsForVirtualDistance(double& l,int& index0,int& index1,bool forwardDirection);


    void _handleAttachedDummies(CPath_old* it);

    C3Vector _getPointOnBezierCubic(const C3Vector& ptBefore,const C3Vector& ptMiddle,const C3Vector& ptAfter,C3Vector& dir,double t);
    C4Vector _getOrientationOnBezierCubic(const C4Vector& orBefore,const C4Vector& orMiddle,const C4Vector& orAfter,double t);
    void _getDistinctConsecutivePoints(const std::vector<CPathPoint_old*>& ptCont,std::vector<C4X4Matrix>& tr,std::vector<std::vector<int> >& ptIndices);
    void _applyDistinctConsecutivePoints(const std::vector<CPathPoint_old*>& ptCont,std::vector<C4X4Matrix>& tr,std::vector<std::vector<int> >& ptIndices);
    void _recomputeBezierPoints();
    void _removeAllBezierPathPoints();
    CBezierPathPoint_old* _addBezierPathPoint(const C7Vector& transf,double maxRelAbsVelocity,double onSpotDistance,unsigned short auxFlags,const double auxChannels[4]);

    std::vector<CSimplePathPoint_old*> _simplePathPoints;
    std::vector<CBezierPathPoint_old*> _bezierPathPoints;

//***********************************

    // Variables needed for the reset procedure of the path. Do not serialize
    bool _initialValuesInitialized;
    double _initialNominalVelocity;
    double _initialPosition;
    double _initialTargetNominalVelocity;

    // Path state and other:
    double _position;
    double _nominalVelocity;
    double _targetNominalVelocity;

    double _maxAcceleration;
    int _pathLengthCalculationMethod;

    // Variables for the automatic velocity profile calculation:
    double _avp_turningCircleRadiusForHalfVelocity;
    double _avp_relativeVelocityAtRotationAxisChange;
    double _avp_relativeAcceleration;
//***********************************

    double _angleVarToDistanceCoeff;
    double _onSpotDistanceToDistanceCoeff;

    int _attributes;
    int _lineSize;
    double _squareSize;


    bool _actualizationEnabled;

    // following is calculated:
    C3Vector _startPosition;
    unsigned short _pathModifID;
    int _lifeID;

#ifdef SIM_WITH_OPENGL
public:
    void render(bool pathEditMode,int displayAttrib,bool pathIsOnlySelectedObject,int objectID);

protected:
    void _draw(std::vector<CPathPoint_old*>& ptCont,bool pathEditMode,bool isPath,bool showLine,bool stripplePts,bool showSquare,bool showOrientation,int lineSize,double squareSize,bool pathIsOnlySelectedObject,int objectID);
#endif
};
