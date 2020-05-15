#pragma once

#include "_ikGroup_.h"
#include "ser.h"
#include "sceneObject.h"
#include "jointObject.h"
#include "dummy.h"

class CIkGroup : public _CIkGroup_
{
public:
    CIkGroup();
    virtual ~CIkGroup();

    // Overridden from CSyncObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();
    void removeSynchronizationObject();

    void initializeInitialValues(bool simulationIsRunning);
    void simulationAboutToStart();
    void simulationEnded();
    CIkGroup* copyYourself() const;
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar) const;
    void performObjectLoadingMapping(const std::vector<int>* map);
    void performIkGroupLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    bool announceIkGroupWillBeErased(int ikGroupID,bool copyBuffer);

    // Overridden from _CIkGroup_:
    bool setObjectName(const char* newName,bool check);
    bool setDoOnFailOrSuccessOf(int groupID,bool check);
    bool setMaxIterations(int maxIter);
    bool setCalculationMethod(int theMethod);
    bool setDampingFactor(float theFactor);

    std::string getUniquePersistentIdString() const;
    int getIkPluginCounterpartHandle() const;
    float getCalculationTime() const;
    int getCalculationResult() const;
    const CMatrix* getLastJacobian() const;
    void resetCalculationResult();
    void setCalculationResult(int res,int calcTimeInMs);

    bool addIkElement(CIkElement* anElement);
    void removeIkElement(int ikElementHandle);

    void setAllInvolvedJointsToNewJointMode(int jointMode) const;


    // OLD_IK_FUNC:
    int computeGroupIk(bool forMotionPlanning);
    bool computeOnlyJacobian(int options);
    float* getLastJacobianData(int matrixSize[2]);
    float* getLastManipulabilityValue(int matrixSize[2]);

protected:
    // Overridden from _CIkGroup_:
    void _addIkElement(CIkElement* anElement);
    void _removeIkElement(int ikElementHandle);

private:
    // Overridden from _CIkGroup_:
    void _setEnabled_send(bool e) const;
    void _setMaxIterations_send(int it) const;
    void _setCalculationMethod_send(int m) const;
    void _setDampingFactor_send(float f) const;
    void _setIgnoreMaxStepSizes_send(bool e) const;
    void _setRestoreIfPositionNotReached_send(bool e) const;
    void _setRestoreIfOrientationNotReached_send(bool e) const;

    void _setLastJacobian(CMatrix* j);

    // OLD_IK_FUNC:
    float _getDeterminant(const CMatrix& m,const std::vector<int>* activeRows,const std::vector<int>* activeColumns) const;
    void _resetTemporaryParameters();
    void _applyTemporaryParameters();
    int performOnePass(std::vector<CIkElement*>* validElements,bool& limitOrAvoidanceNeedMoreCalculation,float interpolFact,bool forMotionPlanning);
    bool performOnePass_jacobianOnly(std::vector<CIkElement*>* validElements,int options);

    std::string _uniquePersistentIdString;
    int _ikPluginCounterpartHandle;
    int _calcTimeInMs;
    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
    int _calculationResult;
    CMatrix* _lastJacobian;
};
