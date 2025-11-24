#pragma once

#include <simMath/3Vector.h>
#include <ser.h>
#include <colorObject.h>
#include <cbor.h>

struct SDynProperty
{
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const char* shortInfoTxt;
    const char* infoTxt;
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                                                                                                                         \
    FUNCX(propDyn_dynamicsEnabled, "dynamicsEnabled", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "Dynamics enabled", "description": ""})", "")                                                                                                                                                                       \
    FUNCX(propDyn_showContactPoints, "showContactPoints", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "Show contact points", "description": ""})", "")                                                                                                                                                                \
    FUNCX(propDyn_dynamicsEngine, "dynamicsEngine", sim_propertytype_intarray, 0, -1, -1, -1, -1, -1, R"({"label": "Dynamics engine", "description": "Selected dynamics engine index and version"})", "")                                                                                                                            \
    FUNCX(propDyn_dynamicsStepSize, "dynamicsStepSize", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "Dynamics dt", "description": "Dynamics step size"})", "")                                                                                                                                                       \
    FUNCX(propDyn_gravity, "gravity", sim_propertytype_vector3, 0, -1, -1, -1, -1, -1, R"({"label": "Gravity", "description": ""})", "")                                                                                                                                                                                             \
    FUNCX(propDyn_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, R"({"label": "Engine properties", "description": "Engine properties as JSON text"})", "")                                                                                                    \
    FUNCX(propDyn_bulletSolver, "bullet.solver", sim_propertytype_int, 0, sim_bullet_global_constraintsolvertype, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                         \
    FUNCX(propDyn_bulletIterations, "bullet.iterations", sim_propertytype_int, 0, sim_bullet_global_constraintsolvingiterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                          \
    FUNCX(propDyn_bulletComputeInertias, "bullet.computeInertias", sim_propertytype_bool, 0, sim_bullet_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                           \
    FUNCX(propDyn_bulletInternalScalingFull, "bullet.internalScalingFull", sim_propertytype_bool, 0, sim_bullet_global_fullinternalscaling, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                               \
    FUNCX(propDyn_bulletInternalScalingScaling, "bullet.internalScalingValue", sim_propertytype_float, 0, sim_bullet_global_internalscalingfactor, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                        \
    FUNCX(propDyn_bulletCollMarginScaling, "bullet.collisionMarginScaling", sim_propertytype_float, 0, sim_bullet_global_collisionmarginfactor, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                           \
    FUNCX(propDyn_odeQuickStepEnabled, "ode.quickStepEnabled", sim_propertytype_bool, 0, sim_ode_global_quickstep, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                        \
    FUNCX(propDyn_odeQuickStepIterations, "ode.quickStepIterations", sim_propertytype_int, 0, sim_ode_global_constraintsolvingiterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                 \
    FUNCX(propDyn_odeComputeInertias, "ode.computeInertias", sim_propertytype_bool, 0, sim_ode_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                    \
    FUNCX(propDyn_odeInternalScalingFull, "ode.internalScalingFull", sim_propertytype_bool, 0, sim_ode_global_fullinternalscaling, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                        \
    FUNCX(propDyn_odeInternalScalingScaling, "ode.internalScalingValue", sim_propertytype_float, 0, sim_ode_global_internalscalingfactor, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                 \
    FUNCX(propDyn_odeGlobalErp, "ode.globalErp", sim_propertytype_float, 0, sim_ode_global_erp, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                           \
    FUNCX(propDyn_odeGlobalCfm, "ode.globalCfm", sim_propertytype_float, 0, sim_ode_global_cfm, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                           \
    FUNCX(propDyn_vortexComputeInertias, "vortex.computeInertias", sim_propertytype_bool, 0, sim_vortex_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                           \
    FUNCX(propDyn_vortexContactTolerance, "vortex.contactTolerance", sim_propertytype_float, 0, sim_vortex_global_contacttolerance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                       \
    FUNCX(propDyn_vortexAutoSleep, "vortex.autoSleep", sim_propertytype_bool, 0, sim_vortex_global_autosleep, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                             \
    FUNCX(propDyn_vortexMultithreading, "vortex.multithreading", sim_propertytype_bool, 0, sim_vortex_global_multithreading, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                              \
    FUNCX(propDyn_vortexConstraintsLinearCompliance, "vortex.constraintsLinearCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintlinearcompliance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                       \
    FUNCX(propDyn_vortexConstraintsLinearDamping, "vortex.constraintsLinearDamping", sim_propertytype_float, 0, sim_vortex_global_constraintlineardamping, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                \
    FUNCX(propDyn_vortexConstraintsLinearKineticLoss, "vortex.constraintsLinearKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintlinearkineticloss, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                    \
    FUNCX(propDyn_vortexConstraintsAngularCompliance, "vortex.constraintsAngularCompliance", sim_propertytype_float, 0, sim_vortex_global_constraintangularcompliance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                    \
    FUNCX(propDyn_vortexConstraintsAngularDamping, "vortex.constraintsAngularDamping", sim_propertytype_float, 0, sim_vortex_global_constraintangulardamping, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                             \
    FUNCX(propDyn_vortexConstraintsAngularKineticLoss, "vortex.constraintsAngularKineticLoss", sim_propertytype_float, 0, sim_vortex_global_constraintangularkineticloss, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                 \
    FUNCX(propDyn_newtonIterations, "newton.iterations", sim_propertytype_int, 0, sim_newton_global_constraintsolvingiterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                          \
    FUNCX(propDyn_newtonComputeInertias, "newton.computeInertias", sim_propertytype_bool, 0, sim_newton_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                           \
    FUNCX(propDyn_newtonMultithreading, "newton.multithreading", sim_propertytype_bool, 0, sim_newton_global_multithreading, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                              \
    FUNCX(propDyn_newtonExactSolver, "newton.exactSolver", sim_propertytype_bool, 0, sim_newton_global_exactsolver, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                       \
    FUNCX(propDyn_newtonHighJointAccuracy, "newton.highJointAccuracy", sim_propertytype_bool, 0, sim_newton_global_highjointaccuracy, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                     \
    FUNCX(propDyn_newtonContactMergeTolerance, "newton.contactMergeTolerance", sim_propertytype_float, 0, sim_newton_global_contactmergetolerance, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                        \
    FUNCX(propDyn_mujocoIntegrator, "mujoco.integrator", sim_propertytype_int, 0, sim_mujoco_global_integrator, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                           \
    FUNCX(propDyn_mujocoSolver, "mujoco.solver", sim_propertytype_int, 0, sim_mujoco_global_solver, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                       \
    FUNCX(propDyn_mujocoIterations, "mujoco.iterations", sim_propertytype_int, 0, sim_mujoco_global_iterations, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                           \
    FUNCX(propDyn_mujocoRebuildTrigger, "mujoco.rebuildTrigger", sim_propertytype_int, 0, sim_mujoco_global_rebuildtrigger, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                               \
    FUNCX(propDyn_mujocoComputeInertias, "mujoco.computeInertias", sim_propertytype_bool, 0, sim_mujoco_global_computeinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                           \
    FUNCX(propDyn_mujocoMbMemory, "mujoco.mbmemory", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                         \
    FUNCX(propDyn_mujocoNjMax, "mujoco.njmax", sim_propertytype_int, 0, sim_mujoco_global_njmax, -1, -1, -1, -1, R"({"label": "", "description": "Deprecated. See mujoco.mbmemory instead."})", "")                                                                                                                                  \
    FUNCX(propDyn_mujocoNconMax, "mujoco.nconmax", sim_propertytype_int, 0, sim_mujoco_global_nconmax, -1, -1, -1, -1, R"({"label": "", "description": "Deprecated. See mujoco.mbmemory instead."})", "")                                                                                                                            \
    FUNCX(propDyn_mujocoNstack, "mujoco.nstack", sim_propertytype_int, 0, sim_mujoco_global_nstack, -1, -1, -1, -1, R"({"label": "", "description": "Deprecated. See mujoco.mbmemory instead."})", "")                                                                                                                               \
    FUNCX(propDyn_mujocoCone, "mujoco.cone", sim_propertytype_int, 0, sim_mujoco_global_cone, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                             \
    FUNCX(propDyn_mujocoKinematicBodiesOverrideFlags, "mujoco.kinematicBodiesOverrideFlags", sim_propertytype_int, 0, sim_mujoco_global_overridekin, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                      \
    FUNCX(propDyn_mujocoKinematicBodiesMass, "mujoco.kinematicBodiesMass", sim_propertytype_float, 0, sim_mujoco_global_kinmass, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                          \
    FUNCX(propDyn_mujocoKinematicBodiesInertia, "mujoco.kinematicBodiesInertia", sim_propertytype_float, 0, sim_mujoco_global_kininertia, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                 \
    FUNCX(propDyn_mujocoBoundMass, "mujoco.boundMass", sim_propertytype_float, 0, sim_mujoco_global_boundmass, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                            \
    FUNCX(propDyn_mujocoBoundInertia, "mujoco.boundInertia", sim_propertytype_float, 0, sim_mujoco_global_boundinertia, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                   \
    FUNCX(propDyn_mujocoBalanceInertias, "mujoco.balanceInertias", sim_propertytype_bool, 0, sim_mujoco_global_balanceinertias, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                           \
    FUNCX(propDyn_mujocoMultithreaded, "mujoco.multithreaded", sim_propertytype_bool, 0, sim_mujoco_global_multithreaded, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                 \
    FUNCX(propDyn_mujocoMulticcd, "mujoco.multiccd", sim_propertytype_bool, 0, sim_mujoco_global_multiccd, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                \
    FUNCX(propDyn_mujocoContactParamsOverride, "mujoco.contactParamsOverride", sim_propertytype_bool, 0, sim_mujoco_global_overridecontacts, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                              \
    FUNCX(propDyn_mujocoContactParamsMargin, "mujoco.contactParamsMargin", sim_propertytype_float, 0, sim_mujoco_global_overridemargin, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                   \
    FUNCX(propDyn_mujocoContactParamsSolref, "mujoco.contactParamsSolref", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolref1, sim_mujoco_global_overridesolref2, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                              \
    FUNCX(propDyn_mujocoContactParamsSolimp, "mujoco.contactParamsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_global_overridesolimp1, sim_mujoco_global_overridesolimp2, sim_mujoco_global_overridesolimp3, sim_mujoco_global_overridesolimp4, sim_mujoco_global_overridesolimp5, R"({"label": "", "description": ""})", "") \
    FUNCX(propDyn_mujocoImpRatio, "mujoco.impratio", sim_propertytype_float, 0, sim_mujoco_global_impratio, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                               \
    FUNCX(propDyn_mujocoWind, "mujoco.wind", sim_propertytype_vector3, 0, sim_mujoco_global_wind1, sim_mujoco_global_wind2, sim_mujoco_global_wind3, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                              \
    FUNCX(propDyn_mujocoDensity, "mujoco.density", sim_propertytype_float, 0, sim_mujoco_global_density, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                  \
    FUNCX(propDyn_mujocoViscosity, "mujoco.viscosity", sim_propertytype_float, 0, sim_mujoco_global_viscosity, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                            \
    FUNCX(propDyn_mujocoJacobian, "mujoco.jacobian", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                         \
    FUNCX(propDyn_mujocoTolerance, "mujoco.tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                     \
    FUNCX(propDyn_mujocoLs_iterations, "mujoco.ls_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                               \
    FUNCX(propDyn_mujocoLs_tolerance, "mujoco.ls_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                               \
    FUNCX(propDyn_mujocoNoslip_iterations, "mujoco.noslip_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                       \
    FUNCX(propDyn_mujocoNoslip_tolerance, "mujoco.noslip_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                       \
    FUNCX(propDyn_mujocoCcd_iterations, "mujoco.ccd_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                             \
    FUNCX(propDyn_mujocoCcd_tolerance, "mujoco.ccd_tolerance", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                             \
    FUNCX(propDyn_mujocoSdf_iterations, "mujoco.sdf_iterations", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                             \
    FUNCX(propDyn_mujocoSdf_initpoints, "mujoco.sdf_initpoints", sim_propertytype_int, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                             \
    FUNCX(propDyn_mujocoEqualityEnable, "mujoco.equalityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                            \
    FUNCX(propDyn_mujocoFrictionlossEnable, "mujoco.frictionlossEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                    \
    FUNCX(propDyn_mujocoLimitEnable, "mujoco.limitEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                  \
    FUNCX(propDyn_mujocoContactEnable, "mujoco.contactEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                              \
    FUNCX(propDyn_mujocoPassiveEnable, "mujoco.passiveEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                              \
    FUNCX(propDyn_mujocoGravityEnable, "mujoco.gravityEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                              \
    FUNCX(propDyn_mujocoWarmstartEnable, "mujoco.warmstartEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                          \
    FUNCX(propDyn_mujocoActuationEnable, "mujoco.actuationEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                          \
    FUNCX(propDyn_mujocoRefsafeEnable, "mujoco.refsafeEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                              \
    FUNCX(propDyn_mujocoSensorEnable, "mujoco.sensorEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                \
    FUNCX(propDyn_mujocoMidphaseEnable, "mujoco.midphaseEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                            \
    FUNCX(propDyn_mujocoEulerdampEnable, "mujoco.eulerdampEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                          \
    FUNCX(propDyn_mujocoAutoresetEnable, "mujoco.autoresetEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                          \
    FUNCX(propDyn_mujocoEnergyEnable, "mujoco.energyEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                \
    FUNCX(propDyn_mujocoInvdiscreteEnable, "mujoco.invdiscreteEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                      \
    FUNCX(propDyn_mujocoNativeccdEnable, "mujoco.nativeccdEnable", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                          \
    FUNCX(propDyn_mujocoAlignfree, "mujoco.alignfree", sim_propertytype_bool, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                                                      \
    FUNCX(propDyn_mujocoKinematicWeldSolref, "mujoco.kinematicWeldSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                            \
    FUNCX(propDyn_mujocoKinematicWeldSolimp, "mujoco.kinematicWeldSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")                                                                                                                                                            \
    FUNCX(propDyn_mujocoKinematicWeldTorqueScale, "mujoco.kinematicWeldTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1, R"({"label": "", "description": ""})", "")

#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) const SDynProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
const std::vector<SDynProperty> allProps_dyn = {DEFINE_PROPERTIES};
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
    simi_mujoco_global_tolerance,
    simi_mujoco_global_ls_tolerance,
    simi_mujoco_global_noslip_tolerance,
    simi_mujoco_global_ccd_tolerance,
    simi_mujoco_global_kinematicweldsolref1,
    simi_mujoco_global_kinematicweldsolref2,
    simi_mujoco_global_kinematicweldsolimp1,
    simi_mujoco_global_kinematicweldsolimp2,
    simi_mujoco_global_kinematicweldsolimp3,
    simi_mujoco_global_kinematicweldsolimp4,
    simi_mujoco_global_kinematicweldsolimp5,
    simi_mujoco_global_kinematicweldtorquescale,
};

enum
{ /* Mujoco global int params */
    simi_mujoco_global_bitcoded = 0,
    simi_mujoco_global_iterations,
    simi_mujoco_global_integrator,
    simi_mujoco_global_solver,
    simi_mujoco_global_njmax,   // deprecated
    simi_mujoco_global_nconmax, // deprecated
    simi_mujoco_global_cone,
    simi_mujoco_global_overridekin,
    simi_mujoco_global_nstack, // deprecated
    simi_mujoco_global_rebuildtrigger,
    simi_mujoco_global_mbmemory,
    simi_mujoco_global_jacobian,
    simi_mujoco_global_ls_iterations,
    simi_mujoco_global_noslip_iterations,
    simi_mujoco_global_ccd_iterations,
    simi_mujoco_global_sdf_iterations,
    simi_mujoco_global_sdf_initpoints,
    simi_mujoco_global_bitcoded2,
};

enum
{ /* Mujoco global bit params */
    simi_mujoco_global_computeinertias = 0x01,
    simi_mujoco_global_multithreaded = 0x02,
    simi_mujoco_global_multiccd = 0x04,
    simi_mujoco_global_balanceinertias = 0x08,
    simi_mujoco_global_overridecontacts = 0x10,
    /* from here, simi_mujoco_global_bitcoded2 */
    simi_mujoco_global_equality = 0x01,
    simi_mujoco_global_frictionloss = 0x02,
    simi_mujoco_global_limit = 0x04,
    simi_mujoco_global_contact = 0x08,
    simi_mujoco_global_passive = 0x10,
    simi_mujoco_global_gravity = 0x20,
    simi_mujoco_global_warmstart = 0x40,
    simi_mujoco_global_actuation = 0x80,
    simi_mujoco_global_refsafe = 0x100,
    simi_mujoco_global_sensor = 0x200,
    simi_mujoco_global_midphase = 0x400,
    simi_mujoco_global_eulerdamp = 0x800,
    simi_mujoco_global_autoreset = 0x1000,
    simi_mujoco_global_energy = 0x2000,
    simi_mujoco_global_invdiscrete = 0x4000,
    simi_mujoco_global_nativeccd = 0x8000,
    simi_mujoco_global_alignfree = 0x10000,
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

    void handleDynamics(double dt);
    bool getContactForce(int dynamicPass, int objectHandle, int index, int objectHandles[2], double* contactInfo) const;

    void addWorldIfNotThere();
    void removeWorld();
    bool isWorldThere() const;

    void appendGenesisData(CCbor* ev);

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
    int getDynamicEngineType(int* version) const;
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

    double getEngineFloatParam_old(int what, bool* ok, bool getDefault = false) const;
    int getEngineIntParam_old(int what, bool* ok, bool getDefault = false) const;
    bool getEngineBoolParam_old(int what, bool* ok, bool getDefault = false) const;
    bool setEngineFloatParam_old(int what, double v);
    bool setEngineIntParam_old(int what, int v);
    bool setEngineBoolParam_old(int what, bool v);

    void getBulletDefaultFloatParams(std::vector<double>& p, int defType = -1) const;
    void getBulletDefaultIntParams(std::vector<int>& p, int defType = -1) const;

    void getOdeDefaultFloatParams(std::vector<double>& p, int defType = -1) const;
    void getOdeDefaultIntParams(std::vector<int>& p, int defType = -1) const;

    void getVortexFloatParams(std::vector<double>& p, bool getDefault = false) const;
    void getVortexIntParams(std::vector<int>& p, bool getDefault = false) const;

    void getVortexDefaultFloatParams(std::vector<double>& p, int defType = -1) const;
    void getVortexDefaultIntParams(std::vector<int>& p, int defType = -1) const;

    void getNewtonFloatParams(std::vector<double>& p, bool getDefault = false) const;
    void getNewtonIntParams(std::vector<int>& p, bool getDefault = false) const;

    void getNewtonDefaultFloatParams(std::vector<double>& p, int defType = -1) const;
    void getNewtonDefaultIntParams(std::vector<int>& p, int defType = -1) const;

    void getMujocoDefaultFloatParams(std::vector<double>& p, int defType = -1) const;
    void getMujocoDefaultIntParams(std::vector<int>& p, int defType = -1) const;

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
    int getPropertyName(int& index, std::string& pName, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    // Some helpers:
    bool getBoolPropertyValue(const char* pName, bool getDefaultValue = false) const;
    int getIntPropertyValue(const char* pName, bool getDefaultValue = false) const;
    double getFloatPropertyValue(const char* pName, bool getDefaultValue = false) const;

    CColorObject contactPointColor;

  protected:
    void _sendEngineString(CCbor* eev = nullptr);
    std::string _enumToProperty(int oldEnum, int type, int& indexWithArrays) const;
    bool _engineFloatsAreSimilar(const std::vector<double>& arr1, const std::vector<double>& arr2) const;
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
    void renderYour3DStuff(CViewableBase* renderingObject, int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject, int displayAttrib);
#endif
};
