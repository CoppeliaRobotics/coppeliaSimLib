
#pragma once

#include "vrepMainHeader.h"
#include "ser.h"

class CConstraintSolverObject
{
public:
    CConstraintSolverObject();
    virtual ~CConstraintSolverObject();

    void initializeInitialValues(bool simulationIsRunning);
    void simulationAboutToStart();
    void simulationEnded();

    void setObjectID(int newID);
    int getObjectID();
    void setObjectName(const char* newName);
    std::string getObjectName();


    void performObjectLoadingMapping(std::vector<int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);

    CConstraintSolverObject* copyYourself();
    void serialize(CSer& ar);

    void setMaximumIterations(int maxIt);
    int getMaximumIterations();
    void setInterpolation(float interpol);
    float getInterpolation();
    void setGeneralDamping(float damping);
    float getGeneralDamping();
    void setMaxAngleVariation(float maxVar);
    float getMaxAngleVariation();
    void setMaxLinearVariation(float maxVar);
    float getMaxLinearVariation();
    void setLoopClosurePositionTolerance(float tol);
    float getLoopClosurePositionTolerance();
    void setLoopClosureOrientationTolerance(float tol);
    float getLoopClosureOrientationTolerance();
    void setBase3DObject(int objID);
    int getBase3DObject();
    void setExplicitHandling(bool explHandl);
    bool getExplicitHandling();
    void selectAllObjectsInMechanism();
    float getCalculationTime();

    bool computeGcs();

private:
    int _objectID;
    std::string _objectName;
    int _base3DObject;
    bool _explicitHandling;
    int _maxIterations;
    float _interpolation;
    float _generalDamping;
    float _maxAngleVariation;
    float _maxLinearVariation;
    float _loopClosurePositionTolerance;
    float _loopClosureOrientationTolerance;

    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
};
