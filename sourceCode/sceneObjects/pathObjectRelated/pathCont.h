
#pragma once

#include "simplePathPoint.h"
#include "bezierPathPoint.h"
#include "colorObject.h"

class CPath;

class CPathCont  
{
public:
    CPathCont();
    virtual ~CPathCont(); 

    void serialize(CSer& ar);
    CPathCont* copyYourself();
    void initializeInitialValues(bool simulationIsRunning);
    void simulationEnded();


    // Simple path and bezier path point routines:
    void removeAllSimplePathPoints();                               // Triggers an actualizePath() call
    void addSimplePathPoint(CSimplePathPoint* it);                  // Triggers an actualizePath() call
    void insertSimplePathPoint(CSimplePathPoint* it,int position);  // Triggers an actualizePath() call
    void removeSimplePathPoint(int position);                       // Triggers an actualizePath() call
    void rollPathPoints(bool forward);                              // Triggers an actualizePath() call

    CSimplePathPoint* getSimplePathPoint(int position);
    CBezierPathPoint* getBezierPathPoint(int position);
    int getSimplePathPointCount();
    int getBezierPathPointCount();
    void copyPointsToClipboard();

    void enableActualization(bool enable);
    void actualizePath();

    void setAttributes(int attrib);                                 // Triggers an actualizePath() call
    int getAttributes();

    // Visual aspect:
    void setLineSize(int size);
    int getLineSize();
    void setSquareSize(float size);
    float getSquareSize();

    void setPosition(double p);
    double getPosition();

    double getPosition_linearComponentOnly();
    double getPosition_angularComponentOnly();


    void setNominalVelocity(float v);
    float getNominalVelocity();
    void setTargetNominalVelocity(float v);
    float getTargetNominalVelocity();
    void setAngleVarToDistanceCoeff(float f);               // Triggers an actualizePath call!
    float getAngleVarToDistanceCoeff();
    void setOnSpotDistanceToDistanceCoeff(float f);             // Triggers an actualizePath call!
    float getOnSpotDistanceToDistanceCoeff();

    void setMaxAcceleration(float maxAccel);
    float getMaxAcceleration();

//****************************

    void scaleObject(float scalingFactor);
    void scaleObjectNonIsometrically(float x,float y,float z);
    void resetPath(CPath* it);
    void handlePath(CPath* it,float deltaTime);
    void handlePath_keepObjectUnchanged(float deltaTime,double& pos_,float& nomVel_,float& targetNomVel_,float maxAccel,bool invertVelocity,bool infiniteAccel); // so that the path can be handled by simFollowPath functions or such!
    float getBezierVirtualPathLength();
    float getBezierNormalPathLength();
    float getBezierAngularPathLength();
    bool getConfigurationOnBezierCurveClosestTo(C3Vector& pt,C7Vector& conf);
    
    bool getPositionOnPathClosestTo(const C3Vector& pt,float& distOnPath);

    unsigned short getPathModifID();

    bool getPointOnBezierCurveAtNormalDistance(float& l,int& index0,float& t);
    C7Vector _getInterpolatedBezierCurvePoint(int index0,float t);
    void _getInterpolatedBezierCurveData(int index0,float t,int& auxFlags,float auxChannels[4]);

    bool getTransformationOnBezierCurveAtNormalizedVirtualDistance(float l,C7Vector& tr);
    bool getAuxDataOnBezierCurveAtNormalizedVirtualDistance(float l,int& auxFlags,float auxChannels[4]);

    bool invertSimplePathPointOrder(const std::vector<int>& selectedPoints);

    void setAvpTurningCircleRadiusForHalfVelocity(float radius);
    float getAvpTurningCircleRadiusForHalfVelocity();
    void setAvpRelativeVelocityAtRotationAxisChange(float relVel);
    float getAvpRelativeVelocityAtRotationAxisChange();
    void setAvpRelativeAcceleration(float relAccel);
    float getAvpRelativeAcceleration();

    void setPathLengthCalculationMethod(int m);                             // Triggers an actualizePath()!
    int getPathLengthCalculationMethod();


    CColorObject _lineColor;

protected:
    int _removeDoubleBezierPoints(float linTol,float angTol);               // Called from actualizePath()-routine
    void _computeAutomaticOrientation(const std::vector<CPathPoint*>& pc);  // Called from actualizePath()-routine
    void _recomputeBezierPathElementLengths();                              // Called from actualizePath()-routine
    void _recomputeBezierPathMaxVelocities();                               // Called from actualizePath()-routine

    bool _getNextBezierPathPointIndex(int currentIndex,int& nextIndex);
    bool _getPointOnBezierCurveAtVirtualDistance(float& l,int& index0,float& t);

    bool _getBezierPointsForVirtualDistance(double& l,int& index0,int& index1,bool forwardDirection);


    void _handleAttachedDummies(CPath* it);

    C3Vector _getPointOnBezierCubic(const C3Vector& ptBefore,const C3Vector& ptMiddle,const C3Vector& ptAfter,C3Vector& dir,float t);
    C4Vector _getOrientationOnBezierCubic(const C4Vector& orBefore,const C4Vector& orMiddle,const C4Vector& orAfter,float t);
    void _getDistinctConsecutivePoints(const std::vector<CPathPoint*>& ptCont,std::vector<C4X4Matrix>& tr,std::vector<std::vector<int> >& ptIndices);
    void _applyDistinctConsecutivePoints(const std::vector<CPathPoint*>& ptCont,std::vector<C4X4Matrix>& tr,std::vector<std::vector<int> >& ptIndices);
    void _recomputeBezierPoints();
    void _removeAllBezierPathPoints();
    CBezierPathPoint* _addBezierPathPoint(const C7Vector& transf,float maxRelAbsVelocity,float onSpotDistance,unsigned short auxFlags,const float auxChannels[4]);

    std::vector<CSimplePathPoint*> _simplePathPoints;
    std::vector<CBezierPathPoint*> _bezierPathPoints;

//***********************************

    // Variables needed for the reset procedure of the path. Do not serialize
    bool _initialValuesInitialized;
    float _initialNominalVelocity;
    float _initialPosition;
    float _initialTargetNominalVelocity;

    // Path state and other:
    double _position; // This one is double!!!!
    float _nominalVelocity;
    float _targetNominalVelocity;

    float _maxAcceleration;
    int _pathLengthCalculationMethod;

    // Variables for the automatic velocity profile calculation:
    float _avp_turningCircleRadiusForHalfVelocity;
    float _avp_relativeVelocityAtRotationAxisChange;
    float _avp_relativeAcceleration;
//***********************************

    float _angleVarToDistanceCoeff;
    float _onSpotDistanceToDistanceCoeff;

    int _attributes;
    int _lineSize;
    float _squareSize;


    bool _actualizationEnabled;

    // following is calculated:
    C3Vector _startPosition;
    unsigned short _pathModifID;
    int _lifeID;

#ifdef SIM_WITH_OPENGL
public:
    void render(bool pathEditMode,int displayAttrib,bool pathIsOnlySelectedObject,int objectID);

protected:
    void _draw(std::vector<CPathPoint*>& ptCont,bool pathEditMode,bool isPath,bool showLine,bool stripplePts,bool showSquare,bool showOrientation,int lineSize,float squareSize,bool pathIsOnlySelectedObject,int objectID);
#endif
};
