#pragma once

#include <_ikGroup_old.h>
#include <ser.h>
#include <sceneObject.h>
#include <jointObject.h>
#include <dummy.h>
#include <simMath/mXnMatrix.h>

class CIkGroup_old : public _CIkGroup_old
{
  public:
    CIkGroup_old();
    virtual ~CIkGroup_old();

    void buildOrUpdate_oldIk();
    void connect_oldIk();
    void remove_oldIk();

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();
    CIkGroup_old* copyYourself() const;
    void serialize(CSer& ar);
    void performIkGroupLoadingMapping(const std::map<int, int>* map, int opType);
    void performObjectLoadingMapping(const std::map<int, int>* map);
    void performIkGroupLoadingMapping(const std::map<int, int>* map);
    bool announceObjectWillBeErased(int objID, bool copyBuffer);
    bool announceIkGroupWillBeErased(int ikGroupID, bool copyBuffer);

    // Overridden from _CIkGroup_old:
    bool setObjectName(const char* newName, bool check) override;
    bool setDoOnFailOrSuccessOf(int groupID, bool check) override;
    bool setMaxIterations(int maxIter) override;
    bool setCalculationMethod(int theMethod) override;
    bool setDampingFactor(double theFactor) override;

    std::string getUniquePersistentIdString() const;
    int getIkPluginCounterpartHandle() const;
    double getCalculationTime() const;
    int getCalculationResult() const;
    const CMatrix* getLastJacobian() const;
    void resetCalculationResult();
    void setCalculationResult(int res, int calcTimeInMs);

    bool addIkElement(CIkElement_old* anElement);
    void removeIkElement(int ikElementHandle);

    void setAllInvolvedJointsToNewJointMode(int jointMode) const;

    int computeGroupIk(bool independentComputation);
    int getConfigForTipPose(int jointCnt, const int* jointHandles, double thresholdDist, int maxTimeInMs,
                            double* retConfig, const double* metric, int collisionPairCnt, const int* collisionPairs,
                            const int* jointOptions, const double* lowLimits, const double* ranges,
                            std::string& errorMsg);
    int checkIkGroup(int jointCnt, const int* jointHandles, double* jointValues, const int* jointOptions);
    int generateIkPath(int jointCnt, const int* jointHandles, int ptCnt, int collisionPairCnt,
                       const int* collisionPairs, const int* jointOptions, std::vector<double>& path);

    bool computeOnlyJacobian(int options);
    const double* getLastJacobianData(int matrixSize[2]);
    double* getLastManipulabilityValue(int matrixSize[2]);

  protected:
    // Overridden from _CIkGroup_old:
    void _addIkElement(CIkElement_old* anElement) override;
    void _removeIkElement(int ikElementHandle) override;

    double _getDeterminant(const CMatrix& m, const std::vector<int>* activeRows,
                           const std::vector<int>* activeColumns) const;

  private:
    // Overridden from _CIkGroup_old:
    void _setEnabled_send(bool e) const override;
    void _setMaxIterations_send(int it) const override;
    void _setCalculationMethod_send(int m) const override;
    void _setDampingFactor_send(double f) const override;
    void _setIgnoreMaxStepSizes_send(bool e) const override;
    void _setRestoreIfPositionNotReached_send(bool e) const override;
    void _setRestoreIfOrientationNotReached_send(bool e) const override;

    void _setLastJacobian(CMatrix* j);

    std::string _uniquePersistentIdString;
    int _ikPluginCounterpartHandle;
    int _calcTimeInMs;
    bool _initialValuesInitialized;
    bool _initialExplicitHandling;
    int _calculationResult;
    CMatrix* _lastJacobian;
};
