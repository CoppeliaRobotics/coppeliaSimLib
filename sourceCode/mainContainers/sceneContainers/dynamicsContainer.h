#pragma once

#include <simMath/3Vector.h>
#include <ser.h>
#include <colorObject.h>
#include <cbor.h>

struct SDynProperty {
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const char* shortInfoTxt;
    const char* infoTxt;
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propDyn_dynamicsEnabled,                       "dynamicsEnabled",                             sim_propertytype_bool,      0, -1, -1, -1, -1, -1, "Dynamics enabled", "") \
    FUNCX(propDyn_showContactPoints,                     "showContactPoints",                           sim_propertytype_bool,      0, -1, -1, -1, -1, -1, "Show contact points", "") \
    FUNCX(propDyn_dynamicsEngine,                        "dynamicsEngine",                              sim_propertytype_intarray,  0, -1, -1, -1, -1, -1, "Dynamics engine", "Selected dynamics engine index and version") \
    FUNCX(propDyn_dynamicsStepSize,                      "dynamicsStepSize",                            sim_propertytype_float,     0, -1, -1, -1, -1, -1, "Dynamics dt", "Dynamics step size") \
    FUNCX(propDyn_gravity,                               "gravity",                                     sim_propertytype_vector3,   0, -1, -1, -1, -1, -1, "Gravity", "") \
    FUNCX(propDyn_engineProperties,                      "engineProperties",                            sim_propertytype_string,    0, -1, -1, -1, -1, -1, "Engine properties", "Engine properties as JSON text") \
    FUNCX(propDyn_bulletSolver,                          "bullet_solver",                                sim_propertytype_int,       0, sim_bullet_global_constraintsolvertype, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_bulletIterations,                      "bullet_iterations",                            sim_propertytype_int,       0, sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_bulletComputeInertias,                 "bullet_computeInertias",                       sim_propertytype_bool,      0, sim_bullet_global_computeinertias, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_bulletInternalScalingFull,             "bullet_internalScalingFull",                   sim_propertytype_bool,      0, sim_bullet_global_fullinternalscaling, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_bulletInternalScalingScaling,          "bullet_internalScalingValue",                  sim_propertytype_float,     0, sim_bullet_global_internalscalingfactor, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_bulletCollMarginScaling,               "bullet_collisionMarginScaling",                sim_propertytype_float,     0, sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeQuickStepEnabled,                   "ode_quickStepEnabled",                         sim_propertytype_bool,      0, sim_ode_global_quickstep, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeQuickStepIterations,                "ode_quickStepIterations",                      sim_propertytype_int,       0, sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeComputeInertias,                    "ode_computeInertias",                          sim_propertytype_bool,      0, sim_ode_global_computeinertias, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeInternalScalingFull,                "ode_internalScalingFull",                      sim_propertytype_bool,      0, sim_ode_global_fullinternalscaling, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeInternalScalingScaling,             "ode_internalScalingValue",                     sim_propertytype_float,     0, sim_ode_global_internalscalingfactor, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeGlobalErp,                          "ode_globalErp",                                sim_propertytype_float,     0, sim_ode_global_erp, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_odeGlobalCfm,                          "ode_globalCfm",                                sim_propertytype_float,     0, sim_ode_global_cfm, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexComputeInertias,                 "vortex_computeInertias",                       sim_propertytype_bool,      0, sim_vortex_global_computeinertias, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexContactTolerance,                "vortex_contactTolerance",                      sim_propertytype_float,     0, sim_vortex_global_contacttolerance, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexAutoSleep,                       "vortex_autoSleep",                             sim_propertytype_bool,      0, sim_vortex_global_autosleep, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexMultithreading,                  "vortex_multithreading",                        sim_propertytype_bool,      0, sim_vortex_global_multithreading, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexConstraintsLinearCompliance,     "vortex_constraintsLinearCompliance",           sim_propertytype_float,     0, sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexConstraintsLinearDamping,        "vortex_constraintsLinearDamping",              sim_propertytype_float,     0, sim_vortex_global_constraintlineardamping, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexConstraintsLinearKineticLoss,    "vortex_constraintsLinearKineticLoss",          sim_propertytype_float,     0, sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexConstraintsAngularCompliance,    "vortex_constraintsAngularCompliance",          sim_propertytype_float,     0, sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexConstraintsAngularDamping,       "vortex_constraintsAngularDamping",             sim_propertytype_float,     0, sim_vortex_global_constraintangulardamping, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_vortexConstraintsAngularKineticLoss,   "vortex_constraintsAngularKineticLoss",         sim_propertytype_float,     0, sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_newtonIterations,                      "newton_iterations",                            sim_propertytype_int,       0, sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_newtonComputeInertias,                 "newton_computeInertias",                       sim_propertytype_bool,      0, sim_newton_global_computeinertias, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_newtonMultithreading,                  "newton_multithreading",                        sim_propertytype_bool,      0, sim_newton_global_multithreading, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_newtonExactSolver,                     "newton_exactSolver",                           sim_propertytype_bool,      0, sim_newton_global_exactsolver, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_newtonHighJointAccuracy,               "newton_highJointAccuracy",                     sim_propertytype_bool,      0, sim_newton_global_highjointaccuracy, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_newtonContactMergeTolerance,           "newton_contactMergeTolerance",                 sim_propertytype_float,     0, sim_newton_global_contactmergetolerance, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoIntegrator,                      "mujoco_integrator",                            sim_propertytype_int,       0, sim_mujoco_global_integrator, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoSolver,                          "mujoco_solver",                                sim_propertytype_int,       0, sim_mujoco_global_solver, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoIterations,                      "mujoco_iterations",                            sim_propertytype_int,       0, sim_mujoco_global_iterations, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoRebuildTrigger,                  "mujoco_rebuildTrigger",                        sim_propertytype_int,       0, sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoComputeInertias,                 "mujoco_computeInertias",                       sim_propertytype_bool,      0, sim_mujoco_global_computeinertias, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoNjMax,                           "mujoco_njmax",                                 sim_propertytype_int,       0, sim_mujoco_global_njmax, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoNconMax,                         "mujoco_nconmax",                               sim_propertytype_int,       0, sim_mujoco_global_nconmax, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoNstack,                          "mujoco_nstack",                                sim_propertytype_int,       0, sim_mujoco_global_nstack, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoCone,                            "mujoco_cone",                                  sim_propertytype_int,       0, sim_mujoco_global_cone, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoKinematicBodiesOverrideFlags,    "mujoco_kinematicBodiesOverrideFlags",          sim_propertytype_int,       0, sim_mujoco_global_overridekin, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoKinematicBodiesMass,             "mujoco_kinematicBodiesMass",                   sim_propertytype_float,     0, sim_mujoco_global_kinmass, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoKinematicBodiesInertia,          "mujoco_kinematicBodiesInertia",                sim_propertytype_float,     0, sim_mujoco_global_kininertia, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoBoundMass,                       "mujoco_boundMass",                             sim_propertytype_float,     0, sim_mujoco_global_boundmass, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoBoundInertia,                    "mujoco_boundInertia",                          sim_propertytype_float,     0, sim_mujoco_global_boundinertia, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoBalanceInertias,                 "mujoco_balanceInertias",                       sim_propertytype_bool,      0, sim_mujoco_global_balanceinertias, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoMultithreaded,                   "mujoco_multithreaded",                         sim_propertytype_bool,      0, sim_mujoco_global_multithreaded, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoMulticcd,                        "mujoco_multiccd",                              sim_propertytype_bool,      0, sim_mujoco_global_multiccd, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoContactParamsOverride,           "mujoco_contactParamsOverride",                 sim_propertytype_bool,      0, sim_mujoco_global_overridecontacts, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoContactParamsMargin,             "mujoco_contactParamsMargin",                   sim_propertytype_float,     0, sim_mujoco_global_overridemargin, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoContactParamsSolref,             "mujoco_contactParamsSolref",                   sim_propertytype_floatarray,   0, sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoContactParamsSolimp,             "mujoco_contactParamsSolimp",                   sim_propertytype_floatarray,    0, sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5, "", "") \
    FUNCX(propDyn_mujocoImpRatio,                        "mujoco_impratio",                              sim_propertytype_float,     0, sim_mujoco_global_impratio, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoWind,                            "mujoco_wind",                                  sim_propertytype_vector3,   0, sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1, "", "") \
    FUNCX(propDyn_mujocoDensity,                         "mujoco_density",                               sim_propertytype_float,     0, sim_mujoco_global_density, -1, -1, -1, -1, "", "") \
    FUNCX(propDyn_mujocoViscosity,                       "mujoco_viscosity",                             sim_propertytype_float,     0, sim_mujoco_global_viscosity, -1, -1, -1, -1, "", "")

#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) const SDynProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
const std::vector<SDynProperty> allProps_dyn = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

enum
{ /* Bullet global double params */
    simi_bullet_global_stepsize = 0,
    simi_bullet_global_internalscalingfactor,
    simi_bullet_global_collisionmarginfactor
};

enum
{ /* Bullet global int params */
    simi_bullet_global_constraintsolvingiterations = 0,
    simi_bullet_global_bitcoded,
    simi_bullet_global_constraintsolvertype
};

enum
{ /* Bullet global bit params */
    simi_bullet_global_fullinternalscaling = 1,
    simi_bullet_global_computeinertias = 2
};

enum
{ /* Ode global double params */
    simi_ode_global_stepsize = 0,
    simi_ode_global_internalscalingfactor,
    simi_ode_global_cfm,
    simi_ode_global_erp
};

enum
{ /* Ode global int params */
    simi_ode_global_constraintsolvingiterations = 0,
    simi_ode_global_bitcoded,
    simi_ode_global_randomseed
};

enum
{ /* Ode global bit params */
    simi_ode_global_fullinternalscaling = 1,
    simi_ode_global_quickstep = 2,
    simi_ode_global_computeinertias = 4
};

enum
{ /* Vortex global double params */
    simi_vortex_global_stepsize = 0,
    simi_vortex_global_internalscalingfactor,
    simi_vortex_global_contacttolerance,
    simi_vortex_global_constraintlinearcompliance,
    simi_vortex_global_constraintlineardamping,
    simi_vortex_global_constraintlinearkineticloss,
    simi_vortex_global_constraintangularcompliance,
    simi_vortex_global_constraintangulardamping,
    simi_vortex_global_constraintangularkineticloss
};

enum
{ /* Vortex global int params */
    simi_vortex_global_bitcoded = 0
};

enum
{ /* Vortex global bit params */
    simi_vortex_global_autosleep = 1,
    simi_vortex_global_multithreading = 2,
    simi_vortex_global_computeinertias = 16
};

enum
{ /* Newton global double params */
    simi_newton_global_stepsize = 0,
    simi_newton_global_contactmergetolerance
};

enum
{ /* Newton global int params */
    simi_newton_global_constraintsolvingiterations = 0,
    simi_newton_global_bitcoded
};

enum
{ /* Newton global bit params */
    simi_newton_global_multithreading = 1,
    simi_newton_global_exactsolver = 2,
    simi_newton_global_highjointaccuracy = 4,
    simi_newton_global_computeinertias = 8,
};

enum
{ /* Mujoco global double params */
    simi_mujoco_global_stepsize = 0,
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

enum
{ /* Mujoco global int params */
    simi_mujoco_global_bitcoded = 0,
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

enum
{ /* Mujoco global bit params */
    simi_mujoco_global_computeinertias = 1,
    simi_mujoco_global_multithreaded = 2,
    simi_mujoco_global_multiccd = 4,
    simi_mujoco_global_balanceinertias = 8,
    simi_mujoco_global_overridecontacts = 16
};

class CViewableBase;

class CDynamicsContainer
{
  public:
    CDynamicsContainer();
    virtual ~CDynamicsContainer();
    void serialize(CSer &ar);
    void simulationAboutToStart();
    void simulationEnded();

    void handleDynamics(double dt);
    bool getContactForce(int dynamicPass, int objectHandle, int index, int objectHandles[2], double *contactInfo) const;

    void addWorldIfNotThere();
    void removeWorld();
    bool isWorldThere() const;

    void appendGenesisData(CCbor *ev);

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

    void setDynamicEngineType(int t, int version);
    int getDynamicEngineType(int *version) const;
    bool setDesiredStepSize(double s);
    double getDesiredStepSize() const;
    double getEffectiveStepSize() const;
    bool setIterationCount(int c);
    int getIterationCount() const;
    bool getComputeInertias() const;
    void setDynamicsEnabled(bool e);
    bool getDynamicsEnabled() const;
    void setGravity(C3Vector gr);
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

    double getEngineFloatParam_old(int what, bool *ok, bool getDefault = false) const;
    int getEngineIntParam_old(int what, bool *ok, bool getDefault = false) const;
    bool getEngineBoolParam_old(int what, bool *ok, bool getDefault = false) const;
    bool setEngineFloatParam_old(int what, double v);
    bool setEngineIntParam_old(int what, int v);
    bool setEngineBoolParam_old(int what, bool v);

    void getBulletDefaultFloatParams(std::vector<double> &p, int defType = -1) const;
    void getBulletDefaultIntParams(std::vector<int> &p, int defType = -1) const;

    void getOdeDefaultFloatParams(std::vector<double> &p, int defType = -1) const;
    void getOdeDefaultIntParams(std::vector<int> &p, int defType = -1) const;

    void getVortexFloatParams(std::vector<double> &p, bool getDefault = false) const;
    void getVortexIntParams(std::vector<int> &p, bool getDefault = false) const;

    void getVortexDefaultFloatParams(std::vector<double> &p, int defType = -1) const;
    void getVortexDefaultIntParams(std::vector<int> &p, int defType = -1) const;

    void getNewtonFloatParams(std::vector<double> &p, bool getDefault = false) const;
    void getNewtonIntParams(std::vector<int> &p, bool getDefault = false) const;

    void getNewtonDefaultFloatParams(std::vector<double> &p, int defType = -1) const;
    void getNewtonDefaultIntParams(std::vector<int> &p, int defType = -1) const;

    void getMujocoDefaultFloatParams(std::vector<double> &p, int defType = -1) const;
    void getMujocoDefaultIntParams(std::vector<int> &p, int defType = -1) const;

    int setBoolProperty(const char* pName, bool pState, CCbor* eev = nullptr);
    int getBoolProperty(const char* pName, bool& pState, bool getDefaultValue = false) const;
    int setIntProperty(const char* pName, int pState, CCbor* eev = nullptr);
    int getIntProperty(const char* pName, int& pState, bool getDefaultValue = false) const;
    int setFloatProperty(const char* pName, double pState, CCbor* eev = nullptr);
    int getFloatProperty(const char* pName, double& pState, bool getDefaultValue = false) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setIntArray2Property(const char* pName, const int* pState, CCbor* eev = nullptr);
    int getIntArray2Property(const char* pName, int* pState, bool getDefaultValue = false) const;
    int setVector2Property(const char* pName, const double* pState, CCbor* eev = nullptr);
    int getVector2Property(const char* pName, double* pState, bool getDefaultValue = false) const;
    int setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev = nullptr);
    int getVector3Property(const char* pName, C3Vector& pState, bool getDefaultValue = false) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL, CCbor* eev = nullptr);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState, bool getDefaultValue = false) const;
    int setIntArrayProperty(const char* pName, const int* v, int vL);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const;
    int getPropertyName(int& index, std::string& pName) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    // Some helpers:
    bool getBoolPropertyValue(const char* pName, bool getDefaultValue = false) const;
    int getIntPropertyValue(const char* pName, bool getDefaultValue = false) const;
    double getFloatPropertyValue(const char* pName, bool getDefaultValue = false) const;

    CColorObject contactPointColor;

  protected:
    void _sendEngineString(CCbor* eev = nullptr);
    std::string _enumToProperty(int oldEnum, int type, int& indexWithArrays) const;
    bool _engineFloatsAreSimilar(const std::vector<double> &arr1, const std::vector<double> &arr2) const;
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

#ifdef SIM_WITH_GUI
  public:
    void renderYour3DStuff(CViewableBase *renderingObject, int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase *renderingObject, int displayAttrib);
#endif
};
