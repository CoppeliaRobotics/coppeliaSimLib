
#pragma once

#include "ser.h"
#include "ikEl.h"
#include "3DObject.h"
#include "jointObject.h"
#include "dummy.h"

class CikGroup  
{
public:
    CikGroup();
    virtual ~CikGroup();

    void initializeInitialValues(bool simulationIsRunning);
    void simulationAboutToStart();
    void simulationEnded();

    CikGroup* copyYourself();
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar);
    void performIkGroupLoadingMapping(std::vector<int>* map);
    void performCollectionLoadingMapping(std::vector<int>* map);
    bool announceCollectionWillBeErased(int groupID,bool copyBuffer);
    std::string getUniquePersistentIdString() const;

    bool addIkElement(CikEl* anElement);
    void setObjectID(int newID);

    void setRestoreIfPositionNotReached(bool active);
    void setRestoreIfOrientationNotReached(bool active);
    void setDoOnFailOrSuccessOf(int groupID);
    void setDoOnFail(bool onFail);
    void setDoOnPerformed(bool turnOn);

    void setAvoidanceWeight(float weight);
    float getAvoidanceWeight();

    void setJointTreshholdAngular(float treshhold);
    void setJointTreshholdLinear(float treshhold);

    void setAvoidanceRobot(int universalObject);
    int getAvoidanceRobot();
    void setAvoidanceObstacle(int universalObject);
    int getAvoidanceObstacle();
    void setAvoidanceThreshold(float t);
    float getAvoidanceThreshold();

    void setIgnoreMaxStepSizes(bool ignore);

    float getCalculationTime();

    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceIkGroupWillBeErased(int ikGroupID,bool copyBuffer);
    void performObjectLoadingMapping(std::vector<int>* map);

    CikEl* getIkElement(int ikElementID);
    CikEl* getIkElementWithTooltipID(int tooltipID);
    void removeIkElement(int elementID);

    void setExplicitHandling(bool explicitHandl);
    bool getExplicitHandling();

    void setAllInvolvedJointsToPassiveMode();

    int getObjectID();
    std::string getObjectName();
    void setObjectName(std::string newName);
    void setMaxIterations(int maxIter);
    int getMaxIterations();
    bool getActive();
    void setDlsFactor(float theFactor);
    float getDlsFactor();
    void setCalculationMethod(int theMethod);
    int getCalculationMethod();
    bool getRestoreIfPositionNotReached();
    bool getRestoreIfOrientationNotReached();

    int getDoOnFailOrSuccessOf();
    bool getDoOnFail();
    bool getDoOnPerformed();
    void setConstraints(int constr);
    int getConstraints();
    void setJointLimitWeight(float weight);
    float getJointLimitWeight();

    float getJointTreshholdAngular();
    float getJointTreshholdLinear();
    int computeGroupIk(bool forMotionPlanning);
    void getAllActiveJoints(std::vector<CJoint*>& jointList);
    void getTipAndTargetLists(std::vector<CDummy*>& tipList,std::vector<CDummy*>& targetList);

    bool getIgnoreMaxStepSizes();
    void resetCalculationResult();
    void setCalculationResult(int res,int calcTimeInMs);
    int getCalculationResult();
    void setCorrectJointLimits(bool c);
    bool getCorrectJointLimits();

    void setActive(bool isActive);

    float* getLastJacobianData(int matrixSize[2]);
    float* getLastManipulabilityValue(int matrixSize[2]);
    float getDeterminant(const CMatrix& m,const std::vector<int>* activeRows,const std::vector<int>* activeColumns);
    bool computeOnlyJacobian(int options);

    // Variables which need to be serialized and copied:
    std::vector<CikEl*> ikElements;

private:
    void getCollisionPartners(std::vector<C3DObject*>* collPartners,std::vector<float>* treshholds,CJoint* bottomJoint,std::vector<CJoint*>* allGroupJoints);
    void _resetTemporaryParameters();
    void _applyTemporaryParameters();

    int performOnePass(std::vector<CikEl*>* validElements,bool& limitOrAvoidanceNeedMoreCalculation,float interpolFact,bool forMotionPlanning);
    bool performOnePass_jacobianOnly(std::vector<CikEl*>* validElements,int options);

    // Variables which need to be serialized and copied:
    float avoidanceWeight;

    int _avoidanceRobotEntity;
    int _avoidanceObstacleEntity;
    float avoidanceThreshold;

    int _calcTimeInMs;

    bool _initialValuesInitialized;
    bool _initialExplicitHandling;

    int objectID;
    std::string objectName;
    std::string _uniquePersistentIdString;
    int maxIterations;
    bool active;
    bool _correctJointLimits;
    float dlsFactor;
    int calculationMethod;
    bool restoreIfPositionNotReached;
    bool restoreIfOrientationNotReached;
    int doOnFailOrSuccessOf; // group identifier which success/fail will be evaluated
    bool doOnFail;
    bool doOnPerformed;
    int constraints; // only sim_avoidance_constraint is valid!
    float jointLimitWeight;
    float jointTreshholdAngular;    // in radian
    float jointTreshholdLinear;     // in meter

    bool ignoreMaxStepSizes;
    int _calculationResult;

    CMatrix* _lastJacobian;

    bool _explicitHandling;
};
