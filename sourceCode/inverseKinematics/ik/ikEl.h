
#pragma once

#include "ser.h"
#include "extIkSer.h"
#include "MyMath.h"

class CikEl  
{
public:
    CikEl();
    CikEl(int theTooltip);
    virtual ~CikEl();

    void initializeInitialValues(bool simulationIsRunning);
    void simulationEnded();
    CikEl* copyYourself();
    void serialize(CSer& ar);
    void serializeWExtIk(CExtIkSer& ar);
    void performCollectionLoadingMapping(std::vector<int>* map);
    bool announceCollectionWillBeErased(int groupID);
    void setObjectID(int newID);
    void setTooltip(int newTooltip);
    void setAlternativeBaseForConstraints(int b); 

    void commonInit();
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    void performObjectLoadingMapping(std::vector<int>* map);
    int getObjectID();
    int getTooltip();
    int getBase();
    bool getActive();
    void setActive(bool isActive);
    void setBase(int newBase);
    void setAllInvolvedJointsToPassiveMode();
    int getAlternativeBaseForConstraints(); 
    int getTarget();
    float getMinAngularPrecision();
    float getMinLinearPrecision();
    void setMinAngularPrecision(float prec);
    void setMinLinearPrecision(float prec);
    void setConstraints(int constr);
    int getConstraints();
    void setPositionWeight(float weight);
    float getPositionWeight();
    void setOrientationWeight(float weight);
    float getOrientationWeight();
    std::string getTipLoadName() const;
    std::string getBaseLoadName() const;
    std::string getAltBaseLoadName() const;

    void checkIfWithinTolerance(bool& position,bool& orientation,bool useTempValues);
    void prepareIkEquations(float interpolFact);
    void removeIkEquations();
    void getError(const C4X4Matrix& m1,const C4X4Matrix& m2,float err[2],bool xC,bool yC,bool zC,bool abC,bool gC);

    // various:
    CMatrix* matrix;
    CMatrix* matrix_correctJacobian;
    CMatrix* errorVector;
    std::vector<int>* rowJointIDs;
    std::vector<int>* rowJointStages;

private:
    // Variables which need to be serialized and copied:
    int objectID;
    int tooltip;
    int base;
    int alternativeBaseForConstraints;
    float minAngularPrecision;
    float minLinearPrecision;
    bool active;
    int constraints; // Bit-coded: use sim_x_constraint, etc. values!
                    // sim_avoidance_conjstraint is reserved for collections! Don't use it here!!
    float positionWeight;
    float orientationWeight;
    std::string _tipLoadName;
    std::string _baseLoadName;
    std::string _altBaseLoadName;
};
