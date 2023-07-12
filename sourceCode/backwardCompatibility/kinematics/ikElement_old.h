#pragma once

#include <ser.h>
#include <simMath/mathFuncs.h>

class CIkElement_old
{
public:
    CIkElement_old();
    CIkElement_old(int theTooltip);
    virtual ~CIkElement_old();

    void buildOrUpdate_oldIk();
    void connect_oldIk();
    void remove_oldIk();

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationEnded();
    CIkElement_old* copyYourself() const;
    void serialize(CSer& ar);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int,int>* map);

    int getTargetHandle() const;
    int getIkPluginCounterpartHandle() const;
    void setIkGroupPluginCounterpartHandle(int h);

    int getObjectHandle() const;
    bool getEnabled() const;
    int getTipHandle()const ;
    int getBase() const;
    int getAlternativeBaseForConstraints() const;
    double getMinLinearPrecision() const;
    double getMinAngularPrecision() const;
    double getPositionWeight() const;
    double getOrientationWeight() const;
    int getConstraints() const;


    bool setMinLinearPrecision(double prec);
    bool setMinAngularPrecision(double prec);
    bool setPositionWeight(double weight);
    bool setOrientationWeight(double weight);
    bool setObjectHandle(int newHandle);
    bool setTipHandle(int newTipHandle);
    bool setEnabled(bool isEnabled);
    bool setBase(int newBase);
    bool setAlternativeBaseForConstraints(int b);
    bool setConstraints(int constr);

    std::string getTipLoadName() const;
    std::string getBaseLoadName() const;
    std::string getAltBaseLoadName() const;

    void setAllInvolvedJointsToIkPluginPositions() const;
    void setAllInvolvedJointsToNewJointMode(int jointMode) const;

private:
    void _commonInit();

    // Overridden from _CIkElement_old:
    void _setEnabled_send(bool e) const;
    void _setBase_send(int h) const;
    void _setAlternativeBaseForConstraints_send(int h) const;
    void _setMinLinearPrecision_send(double f) const;
    void _setMinAngularPrecision_send(double f) const;
    void _setPositionWeight_send(double f) const;
    void _setOrientationWeight_send(double f) const;
    void _setConstraints_send(int c) const;

    int _ikGroupPluginCounterpartHandle;
    int _ikElementPluginCounterpartHandle;

    std::string _tipLoadName;
    std::string _baseLoadName;
    std::string _altBaseLoadName;

    int _objectHandle;
    int _tipHandle;
    int _baseHandle;
    int _constraintBaseHandle;
    double _minLinearPrecision;
    double _minAngularPrecision;
    bool _enabled;
    int _constraints; // bit-coded: use sim_x_constraint, etc. values!
    double _positionWeight;
    double _orientationWeight;
};
