#pragma once

#include "3Vector.h"
#include "ser.h"
#include "colorObject.h"

enum { /* Bullet global double params */
    simi_bullet_global_stepsize=0,
    simi_bullet_global_internalscalingfactor,
    simi_bullet_global_collisionmarginfactor
};

enum { /* Bullet global int params */
    simi_bullet_global_constraintsolvingiterations=0,
    simi_bullet_global_bitcoded,
    simi_bullet_global_constraintsolvertype
};

enum { /* Bullet global bit params */
    simi_bullet_global_fullinternalscaling=1,
    simi_bullet_global_computeinertias=2
};

enum { /* Ode global double params */
    simi_ode_global_stepsize=0,
    simi_ode_global_internalscalingfactor,
    simi_ode_global_cfm,
    simi_ode_global_erp
};

enum { /* Ode global int params */
    simi_ode_global_constraintsolvingiterations=0,
    simi_ode_global_bitcoded,
    simi_ode_global_randomseed
};

enum { /* Ode global bit params */
    simi_ode_global_fullinternalscaling=1,
    simi_ode_global_quickstep=2,
    simi_ode_global_computeinertias=4
};

enum { /* Vortex global double params */
    simi_vortex_global_stepsize=0,
    simi_vortex_global_internalscalingfactor,
    simi_vortex_global_contacttolerance,
    simi_vortex_global_constraintlinearcompliance,
    simi_vortex_global_constraintlineardamping,
    simi_vortex_global_constraintlinearkineticloss,
    simi_vortex_global_constraintangularcompliance,
    simi_vortex_global_constraintangulardamping,
    simi_vortex_global_constraintangularkineticloss
};

enum { /* Vortex global int params */
    simi_vortex_global_bitcoded=0
};

enum { /* Vortex global bit params */
    simi_vortex_global_autosleep=1,
    simi_vortex_global_multithreading=2,
    simi_vortex_global_computeinertias=16
};

enum { /* Newton global double params */
    simi_newton_global_stepsize=0,
    simi_newton_global_contactmergetolerance
};

enum { /* Newton global int params */
    simi_newton_global_constraintsolvingiterations=0,
    simi_newton_global_bitcoded
};

enum { /* Newton global bit params */
    simi_newton_global_multithreading=1,
    simi_newton_global_exactsolver=2,
    simi_newton_global_highjointaccuracy=4,
    simi_newton_global_computeinertias=8,
};

enum { /* Mujoco global double params */
    simi_mujoco_global_stepsize=0,
    simi_mujoco_global_impratio,
    simi_mujoco_global_wind1,
    simi_mujoco_global_wind2,
    simi_mujoco_global_wind3,
    simi_mujoco_global_density,
    simi_mujoco_global_viscosity,
    simi_mujoco_global_boundmass,
    simi_mujoco_global_boundinertia,
    simi_mujoco_global_overridemargin,
    simi_mujoco_global_overridesolref1,
    simi_mujoco_global_overridesolref2,
    simi_mujoco_global_overridesolimp1,
    simi_mujoco_global_overridesolimp2,
    simi_mujoco_global_overridesolimp3,
    simi_mujoco_global_overridesolimp4,
    simi_mujoco_global_overridesolimp5,
    simi_mujoco_global_kinmass,
    simi_mujoco_global_kininertia,
};

enum { /* Mujoco global int params */
    simi_mujoco_global_bitcoded=0,
    simi_mujoco_global_iterations,
    simi_mujoco_global_integrator,
    simi_mujoco_global_solver,
    simi_mujoco_global_njmax,
    simi_mujoco_global_nconmax,
    simi_mujoco_global_cone,
    simi_mujoco_global_overridekin,
    simi_mujoco_global_nstack,
    simi_mujoco_global_rebuildtrigger,
};

enum { /* Mujoco global bit params */
    simi_mujoco_global_computeinertias=1,
    simi_mujoco_global_multithreaded=2,
    simi_mujoco_global_multiccd=4,
    simi_mujoco_global_balanceinertias=8,
    simi_mujoco_global_overridecontacts=16
};

class CViewableBase;

class CDynamicsContainer 
{
public:
    CDynamicsContainer();
    virtual ~CDynamicsContainer();
    void serialize(CSer& ar);
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib);

    void handleDynamics(double dt);
    bool getContactForce(int dynamicPass,int objectHandle,int index,int objectHandles[2],double* contactInfo) const;

    void addWorldIfNotThere();
    void removeWorld();
    bool isWorldThere() const;

    void markForWarningDisplay_pureSpheroidNotSupported();
    void markForWarningDisplay_pureConeNotSupported();
    void markForWarningDisplay_containsNonPureNonConvexShapes();
    void markForWarningDisplay_containsStaticShapesOnDynamicConstruction();
    void markForWarningDisplay_pureHollowShapeNotSupported();
    void markForWarningDisplay_physicsEngineNotSupported();
    void markForWarningDisplay_vortexPluginIsDemo();

    void displayWarningsIfNeeded();

    void checkIfEngineSettingsAreDefault();
    bool getSettingsAreDefault() const;


    void setDynamicEngineType(int t,int version);
    int getDynamicEngineType(int* version) const;
    bool setDesiredStepSize(double s);
    double getDesiredStepSize() const;
    double getEffectiveStepSize() const;
    bool setIterationCount(int c);
    int getIterationCount() const;
    bool getComputeInertias() const;
    void setDynamicsEnabled(bool e);
    bool getDynamicsEnabled() const;
    void setGravity(const C3Vector& gr);
    C3Vector getGravity() const;

    double getPositionScalingFactorDyn() const;
    double getLinearVelocityScalingFactorDyn() const;
    double getMassScalingFactorDyn() const;
    double getMasslessInertiaScalingFactorDyn() const;
    double getForceScalingFactorDyn() const;
    double getTorqueScalingFactorDyn() const;
    double getGravityScalingFactorDyn() const;

    void setDisplayContactPoints(bool d);
    bool getDisplayContactPoints() const;

    void setTempDisabledWarnings(int mask);
    int getTempDisabledWarnings() const;

    bool getCurrentlyInDynamicsCalculations() const;

    double getEngineFloatParam(int what,bool* ok,bool getDefault=false) const;
    int getEngineIntParam(int what,bool* ok,bool getDefault=false) const;
    bool getEngineBoolParam(int what,bool* ok,bool getDefault=false) const;
    bool setEngineFloatParam(int what,double v);
    bool setEngineIntParam(int what,int v);
    bool setEngineBoolParam(int what,bool v);

    void getBulletFloatParams(std::vector<double>& p,bool getDefault=false) const;
    void setBulletFloatParams(const std::vector<double>& p);
    void getBulletIntParams(std::vector<int>& p,bool getDefault=false) const;
    void setBulletIntParams(const std::vector<int>& p);

    void getBulletDefaultFloatParams(std::vector<double>& p,int defType=-1) const;
    void getBulletDefaultIntParams(std::vector<int>& p,int defType=-1) const;

    void getOdeFloatParams(std::vector<double>& p,bool getDefault=false) const;
    void setOdeFloatParams(const std::vector<double>& p);
    void getOdeIntParams(std::vector<int>& p,bool getDefault=false) const;
    void setOdeIntParams(const std::vector<int>& p);

    void getOdeDefaultFloatParams(std::vector<double>& p,int defType=-1) const;
    void getOdeDefaultIntParams(std::vector<int>& p,int defType=-1) const;

    void getVortexFloatParams(std::vector<double>& p,bool getDefault=false) const;
    void setVortexFloatParams(const std::vector<double>& p);
    void getVortexIntParams(std::vector<int>& p,bool getDefault=false) const;
    void setVortexIntParams(const std::vector<int>& p);

    void getVortexDefaultFloatParams(std::vector<double>& p,int defType=-1) const;
    void getVortexDefaultIntParams(std::vector<int>& p,int defType=-1) const;

    void getNewtonFloatParams(std::vector<double>& p,bool getDefault=false) const;
    void setNewtonFloatParams(const std::vector<double>& p);
    void getNewtonIntParams(std::vector<int>& p,bool getDefault=false) const;
    void setNewtonIntParams(const std::vector<int>& p);

    void getNewtonDefaultFloatParams(std::vector<double>& p,int defType=-1) const;
    void getNewtonDefaultIntParams(std::vector<int>& p,int defType=-1) const;

    void getMujocoFloatParams(std::vector<double>& p,bool getDefault=false) const;
    void setMujocoFloatParams(const std::vector<double>& p);
    void getMujocoIntParams(std::vector<int>& p,bool getDefault=false) const;
    void setMujocoIntParams(const std::vector<int>& p);

    void getMujocoDefaultFloatParams(std::vector<double>& p,int defType=-1) const;
    void getMujocoDefaultIntParams(std::vector<int>& p,int defType=-1) const;

    CColorObject contactPointColor;

protected:
    bool _engineFloatsAreSimilar(const std::vector<double>& arr1,const std::vector<double>& arr2) const;
    void _fixVortexInfVals();
    void _resetWarningFlags();

    unsigned char _pureSpheroidNotSupportedMark;
    unsigned char _pureConeNotSupportedMark;
    unsigned char _pureHollowShapeNotSupportedMark;
    unsigned char _physicsEngineNotSupportedWarning;

    unsigned char _containsNonPureNonConvexShapes;
    unsigned char _containsStaticShapesOnDynamicConstruction;
    unsigned char _vortexPluginIsDemoWarning;
    unsigned char _stringsNotSupportedWarning;

    int _tempDisabledWarnings; // bits in the same order as above messages

    bool _currentlyInDynamicsCalculations;

    // To serialize:
    bool _dynamicsEnabled;
    int _dynamicEngineToUse;
    int _dynamicEngineVersionToUse;
    double _stepSize;
    C3Vector _gravity;
    bool _displayContactPoints;

    bool _engineSettingsAreDefault;

    std::vector<double> _bulletFloatParams;
    std::vector<int> _bulletIntParams;

    std::vector<double> _odeFloatParams;
    std::vector<int> _odeIntParams;

    std::vector<double> _vortexFloatParams;
    std::vector<int> _vortexIntParams;

    std::vector<double> _newtonFloatParams;
    std::vector<int> _newtonIntParams;

    std::vector<double> _mujocoFloatParams;
    std::vector<int> _mujocoIntParams;
};
