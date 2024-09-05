#pragma once

#include <sceneObject.h>

struct SJointProperty {
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propJoint_length,                    "length",                                     sim_propertytype_float,     0, -1, -1, -1, -1, -1) \
    FUNCX(propJoint_diameter,                  "diameter",                                   sim_propertytype_float,     0, -1, -1, -1, -1, -1) \
    FUNCX(propJoint_engineProperties,          "engineProperties",                           sim_propertytype_string,    0, -1, -1, -1, -1, -1) \
    FUNCX(propJoint_bulletStopErp,             "bulletStopErp",                              sim_propertytype_float,     0, sim_bullet_joint_stoperp, -1, -1, -1, -1) \
    FUNCX(propJoint_bulletStopCfm,             "bulletStopCfm",                              sim_propertytype_float,     0, sim_bullet_joint_stopcfm, -1, -1, -1, -1) \
    FUNCX(propJoint_bulletNormalCfm,           "bulletNormalCfm",                            sim_propertytype_float,     0, sim_bullet_joint_normalcfm, -1, -1, -1, -1) \
    FUNCX(propJoint_bulletPosPid,              "bulletPosPid",                               sim_propertytype_vector,    0, sim_bullet_joint_pospid1, sim_bullet_joint_pospid2, sim_bullet_joint_pospid3, -1, -1) \
    FUNCX(propJoint_odeStopErp,                "odeStopErp",                                 sim_propertytype_float,     0, sim_ode_joint_stoperp, -1, -1, -1, -1) \
    FUNCX(propJoint_odeStopCfm,                "odeStopCfm",                                 sim_propertytype_float,     0, sim_ode_joint_stopcfm, -1, -1, -1, -1) \
    FUNCX(propJoint_odeNormalCfm,              "odeNormalCfm",                               sim_propertytype_float,     0, sim_ode_joint_normalcfm, -1, -1, -1, -1) \
    FUNCX(propJoint_odeBounce,                 "odeBounce",                                  sim_propertytype_float,     0, sim_ode_joint_bounce, -1, -1, -1, -1) \
    FUNCX(propJoint_odeFudgeFactor,            "odeFudge",                                   sim_propertytype_float,     0, sim_ode_joint_fudgefactor, -1, -1, -1, -1) \
    FUNCX(propJoint_odePosPid,                 "odePosPid",                                  sim_propertytype_vector,    0, sim_ode_joint_pospid1, sim_ode_joint_pospid2, sim_ode_joint_pospid3, -1, -1) \
    FUNCX(propJoint_vortexLowerLimitDamping,   "vortexAxisLimitsLowerDamping",                     sim_propertytype_float,     0, sim_vortex_joint_lowerlimitdamping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexUpperLimitDamping,   "vortexAxisLimitsUpperDamping",                     sim_propertytype_float,     0, sim_vortex_joint_upperlimitdamping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexLowerLimitStiffness, "vortexAxisLimitsLowerStiffness",                   sim_propertytype_float,     0, sim_vortex_joint_lowerlimitstiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexUpperLimitStiffness, "vortexAxisLimitsUpperStiffness",                   sim_propertytype_float,     0, sim_vortex_joint_upperlimitstiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexLowerLimitRestitution, "vortexAxisLimitsLowerRestitution",               sim_propertytype_float,     0, sim_vortex_joint_lowerlimitrestitution, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexUpperLimitRestitution, "vortexAxisLimitsUpperRestitution",               sim_propertytype_float,     0, sim_vortex_joint_upperlimitrestitution, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexLowerLimitMaxForce,  "vortexAxisLimitsLowerMaxForce",                    sim_propertytype_float,     0, sim_vortex_joint_lowerlimitmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexUpperLimitMaxForce,  "vortexAxisLimitsUpperMaxForce",                    sim_propertytype_float,     0, sim_vortex_joint_upperlimitmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexAxisFrictionEnabled,     "vortexAxisFrictionEnabled",                        sim_propertytype_bool,      0, sim_vortex_joint_motorfrictionenabled, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexAxisFrictionProportional,"vortexAxisFrictionProportional",                   sim_propertytype_bool,      0, sim_vortex_joint_proportionalmotorfriction, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexAxisFrictionCoeff,       "vortexAxisFrictionValue",                          sim_propertytype_float,     0, sim_vortex_joint_motorconstraintfrictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexAxisFrictionMaxForce,    "vortexAxisFrictionMaxForce",                       sim_propertytype_float,     0, sim_vortex_joint_motorconstraintfrictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexAxisFrictionLoss,        "vortexAxisFrictionLoss",                           sim_propertytype_float,     0, sim_vortex_joint_motorconstraintfrictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexRelaxationEnabledBits,    "vortexRelaxationEnabledBits",                  sim_propertytype_int,      0, sim_vortex_joint_relaxationenabledbc, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexFrictionEnabledBits,      "vortexFrictionEnabledBits",                    sim_propertytype_int,      0, sim_vortex_joint_frictionenabledbc, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexFrictionProportionalBits, "vortexFrictionProportionalBits",               sim_propertytype_int,      0, sim_vortex_joint_frictionproportionalbc, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisPosRelaxationStiffness,  "vortexXAxisPosRelaxationStiffness",                sim_propertytype_float,     0, sim_vortex_joint_p0stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisPosRelaxationDamping,    "vortexXAxisPosRelaxationDamping",                  sim_propertytype_float,     0, sim_vortex_joint_p0damping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisPosRelaxationLoss,       "vortexXAxisPosRelaxationLoss",                     sim_propertytype_float,     0, sim_vortex_joint_p0loss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisPosFrictionCoeff,        "vortexXAxisPosFrictionCoeff",                      sim_propertytype_float,     0, sim_vortex_joint_p0frictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisPosFrictionMaxForce,     "vortexXAxisPosFrictionMaxForce",                   sim_propertytype_float,     0, sim_vortex_joint_p0frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisPosFrictionLoss,         "vortexXAxisPosFrictionLoss",                       sim_propertytype_float,     0, sim_vortex_joint_p0frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisPosRelaxationStiffness, "vortexYAxisPosRelaxationStiffness",                sim_propertytype_float,     0, sim_vortex_joint_p1stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisPosRelaxationDamping,   "vortexYAxisPosRelaxationDamping",                  sim_propertytype_float,     0, sim_vortex_joint_p1damping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisPosRelaxationLoss,      "vortexYAxisPosRelaxationLoss",                     sim_propertytype_float,     0, sim_vortex_joint_p1loss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisPosFrictionCoeff,       "vortexYAxisPosFrictionCoeff",                      sim_propertytype_float,     0, sim_vortex_joint_p1frictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisPosFrictionMaxForce,    "vortexYAxisPosFrictionMaxForce",                   sim_propertytype_float,     0, sim_vortex_joint_p1frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisPosFrictionLoss,        "vortexYAxisPosFrictionLoss",                       sim_propertytype_float,     0, sim_vortex_joint_p1frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisPosRelaxationStiffness, "vortexZAxisPosRelaxationStiffness",                sim_propertytype_float,     0, sim_vortex_joint_p2stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisPosRelaxationDamping,   "vortexZAxisPosRelaxationDamping",                  sim_propertytype_float,     0, sim_vortex_joint_p2damping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisPosRelaxationLoss,      "vortexZAxisPosRelaxationLoss",                     sim_propertytype_float,     0, sim_vortex_joint_p2loss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisPosFrictionCoeff,       "vortexZAxisPosFrictionCoeff",                      sim_propertytype_float,     0, sim_vortex_joint_p2frictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisPosFrictionMaxForce,    "vortexZAxisPosFrictionMaxForce",                   sim_propertytype_float,     0, sim_vortex_joint_p2frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisPosFrictionLoss,        "vortexZAxisPosFrictionLoss",                       sim_propertytype_float,     0, sim_vortex_joint_p2frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisOrientRelaxStiffness,"vortexXAxisOrientRelaxationStiffness",            sim_propertytype_float,     0, sim_vortex_joint_a0stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisOrientRelaxDamping, "vortexXAxisOrientRelaxationDamping",               sim_propertytype_float,     0, sim_vortex_joint_a0damping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisOrientRelaxLoss,    "vortexXAxisOrientRelaxationLoss",                  sim_propertytype_float,     0, sim_vortex_joint_a0loss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisOrientFrictionCoeff,"vortexXAxisOrientFrictionCoeff",                   sim_propertytype_float,     0, sim_vortex_joint_a0frictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisOrientFrictionMaxTorque,"vortexXAxisOrientFrictionMaxTorque",           sim_propertytype_float,     0, sim_vortex_joint_a0frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexXAxisOrientFrictionLoss, "vortexXAxisOrientFrictionLoss",                    sim_propertytype_float,     0, sim_vortex_joint_a0frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisOrientRelaxStiffness,"vortexYAxisOrientRelaxationStiffness",            sim_propertytype_float,     0, sim_vortex_joint_a1stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisOrientRelaxDamping, "vortexYAxisOrientRelaxationDamping",               sim_propertytype_float,     0, sim_vortex_joint_a1damping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisOrientRelaxLoss,    "vortexYAxisOrientRelaxationLoss",                  sim_propertytype_float,     0, sim_vortex_joint_a1loss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisOrientFrictionCoeff,"vortexYAxisOrientFrictionCoeff",                   sim_propertytype_float,     0, sim_vortex_joint_a1frictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisOrientFrictionMaxTorque,"vortexYAxisOrientFrictionMaxTorque",           sim_propertytype_float,     0, sim_vortex_joint_a1frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexYAxisOrientFrictionLoss, "vortexYAxisOrientFrictionLoss",                    sim_propertytype_float,     0, sim_vortex_joint_a1frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisOrientRelaxStiffness,"vortexZAxisOrientRelaxationStiffness",            sim_propertytype_float,     0, sim_vortex_joint_a2stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisOrientRelaxDamping, "vortexZAxisOrientRelaxationDamping",               sim_propertytype_float,     0, sim_vortex_joint_a2damping, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisOrientRelaxLoss,    "vortexZAxisOrientRelaxationLoss",                  sim_propertytype_float,     0, sim_vortex_joint_a2loss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisOrientFrictionCoeff,"vortexZAxisOrientFrictionCoeff",                   sim_propertytype_float,     0, sim_vortex_joint_a2frictioncoeff, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisOrientFrictionMaxTorque,"vortexZAxisOrientFrictionMaxTorque",           sim_propertytype_float,     0, sim_vortex_joint_a2frictionmaxforce, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexZAxisOrientFrictionLoss, "vortexZAxisOrientFrictionLoss",                    sim_propertytype_float,     0, sim_vortex_joint_a2frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_vortexPosPid,                  "vortexPosPid",                                     sim_propertytype_vector,    0, sim_vortex_joint_pospid1, sim_vortex_joint_pospid2, sim_vortex_joint_pospid3, -1, -1) \
    FUNCX(propJoint_newtonPosPid,                  "newtonPosPid",                                     sim_propertytype_vector,    0, sim_newton_joint_pospid1, sim_newton_joint_pospid2, sim_newton_joint_pospid3, -1, -1) \
    FUNCX(propJoint_mujocoArmature,                "mujocoArmature",                                   sim_propertytype_float,     0, sim_mujoco_joint_armature, -1, -1, -1, -1) \
    FUNCX(propJoint_mujocoMargin,                  "mujocoMargin",                                     sim_propertytype_float,     0, sim_mujoco_joint_margin, -1, -1, -1, -1) \
    FUNCX(propJoint_mujocoFrictionLoss,            "mujocoFrictionLoss",                               sim_propertytype_float,     0, sim_mujoco_joint_frictionloss, -1, -1, -1, -1) \
    FUNCX(propJoint_mujocoSpringStiffness,         "mujocoSpringStiffness",                            sim_propertytype_float,     0, sim_mujoco_joint_stiffness, -1, -1, -1, -1) \
    FUNCX(propJoint_mujocoSpringDamping,           "mujocoSpringDamping",                              sim_propertytype_float,     0, sim_mujoco_joint_damping, -1, -1, -1, -1) \
    FUNCX(propJoint_mujocoSpringRef,               "mujocoSpringRef",                                  sim_propertytype_float,     0, sim_mujoco_joint_springref, -1, -1, -1, -1) \
    FUNCX(propJoint_mujocoPosPid,                  "mujocoPosPid",                                     sim_propertytype_vector,    0, sim_mujoco_joint_pospid1, sim_mujoco_joint_pospid2, sim_mujoco_joint_pospid3, -1, -1) \
    FUNCX(propJoint_mujocoLimitsSolRef,            "mujocoLimitsSolref",                               sim_propertytype_vector,    0, sim_mujoco_joint_solreflimit1, sim_mujoco_joint_solreflimit2, -1, -1, -1) \
    FUNCX(propJoint_mujocoLimitsSolImp,            "mujocoLimitsSolimp",                               sim_propertytype_vector,    0, sim_mujoco_joint_solimplimit1, sim_mujoco_joint_solimplimit2, sim_mujoco_joint_solimplimit3, sim_mujoco_joint_solimplimit4, sim_mujoco_joint_solimplimit5) \
    FUNCX(propJoint_mujocoFrictionSolRef,          "mujocoFrictionSolref",                             sim_propertytype_vector,    0, sim_mujoco_joint_solreffriction1, sim_mujoco_joint_solreffriction2, -1, -1, -1) \
    FUNCX(propJoint_mujocoFrictionSolImp,          "mujocoFrictionSolimp",                             sim_propertytype_vector,    0, sim_mujoco_joint_solimpfriction1, sim_mujoco_joint_solimpfriction2, sim_mujoco_joint_solimpfriction3, sim_mujoco_joint_solimpfriction4, sim_mujoco_joint_solimpfriction5) \
    FUNCX(propJoint_mujocoSpringDamper,            "mujocoSpringSpringDamper",                         sim_propertytype_vector,    0, sim_mujoco_joint_springdamper1, sim_mujoco_joint_springdamper2, -1, -1, -1) \
    FUNCX(propJoint_mujocoDependencyPolyCoef,      "mujocoDependencyPolyCoeff",                        sim_propertytype_vector,    0, sim_mujoco_joint_polycoef1, sim_mujoco_joint_polycoef2, sim_mujoco_joint_polycoef3, sim_mujoco_joint_polycoef4, sim_mujoco_joint_polycoef5) \

#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4) const SJointProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4) name,
const std::vector<SJointProperty> allProps_joint = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------


enum
{ /* Bullet joint double params */
    simi_bullet_joint_stoperp = 0,
    simi_bullet_joint_stopcfm,
    simi_bullet_joint_normalcfm,
    simi_bullet_joint_free1,
    simi_bullet_joint_free2,
    simi_bullet_joint_pospid1,
    simi_bullet_joint_pospid2,
    simi_bullet_joint_pospid3,
};

enum
{ /* Ode joint double params */
    simi_ode_joint_stoperp = 0,
    simi_ode_joint_stopcfm,
    simi_ode_joint_bounce,
    simi_ode_joint_fudgefactor,
    simi_ode_joint_normalcfm,
    simi_ode_joint_pospid1,
    simi_ode_joint_pospid2,
    simi_ode_joint_pospid3,
};

enum
{ /* Vortex joint double params */
    simi_vortex_joint_lowerlimitdamping = 0,
    simi_vortex_joint_upperlimitdamping,
    simi_vortex_joint_lowerlimitstiffness,
    simi_vortex_joint_upperlimitstiffness,
    simi_vortex_joint_lowerlimitrestitution,
    simi_vortex_joint_upperlimitrestitution,
    simi_vortex_joint_lowerlimitmaxforce,
    simi_vortex_joint_upperlimitmaxforce,
    simi_vortex_joint_motorconstraintfrictioncoeff,
    simi_vortex_joint_motorconstraintfrictionmaxforce,
    simi_vortex_joint_motorconstraintfrictionloss,
    simi_vortex_joint_p0loss,
    simi_vortex_joint_p0stiffness,
    simi_vortex_joint_p0damping,
    simi_vortex_joint_p0frictioncoeff,
    simi_vortex_joint_p0frictionmaxforce,
    simi_vortex_joint_p0frictionloss,
    simi_vortex_joint_p1loss,
    simi_vortex_joint_p1stiffness,
    simi_vortex_joint_p1damping,
    simi_vortex_joint_p1frictioncoeff,
    simi_vortex_joint_p1frictionmaxforce,
    simi_vortex_joint_p1frictionloss,
    simi_vortex_joint_p2loss,
    simi_vortex_joint_p2stiffness,
    simi_vortex_joint_p2damping,
    simi_vortex_joint_p2frictioncoeff,
    simi_vortex_joint_p2frictionmaxforce,
    simi_vortex_joint_p2frictionloss,
    simi_vortex_joint_a0loss,
    simi_vortex_joint_a0stiffness,
    simi_vortex_joint_a0damping,
    simi_vortex_joint_a0frictioncoeff,
    simi_vortex_joint_a0frictionmaxforce,
    simi_vortex_joint_a0frictionloss,
    simi_vortex_joint_a1loss,
    simi_vortex_joint_a1stiffness,
    simi_vortex_joint_a1damping,
    simi_vortex_joint_a1frictioncoeff,
    simi_vortex_joint_a1frictionmaxforce,
    simi_vortex_joint_a1frictionloss,
    simi_vortex_joint_a2loss,
    simi_vortex_joint_a2stiffness,
    simi_vortex_joint_a2damping,
    simi_vortex_joint_a2frictioncoeff,
    simi_vortex_joint_a2frictionmaxforce,
    simi_vortex_joint_a2frictionloss,
    simi_vortex_joint_dependencyfactor, // deprecated
    simi_vortex_joint_dependencyoffset, // deprecated
    simi_vortex_joint_free1,
    simi_vortex_joint_free2,
    simi_vortex_joint_pospid1,
    simi_vortex_joint_pospid2,
    simi_vortex_joint_pospid3,
};

enum
{ /* Vortex joint int params */
    simi_vortex_joint_bitcoded = 0,
    simi_vortex_joint_relaxationenabledbc,
    simi_vortex_joint_frictionenabledbc,
    simi_vortex_joint_frictionproportionalbc,
    simi_vortex_joint_objectid,         // deprecated
    simi_vortex_joint_dependentobjectid // deprecated
};

enum
{ /* Vortex joint bit params */
    simi_vortex_joint_motorfrictionenabled = 1,
    simi_vortex_joint_proportionalmotorfriction = 2
};

enum
{                                           /* Newton joint double params */
    simi_newton_joint_dependencyfactor = 0, // deprecated
    simi_newton_joint_dependencyoffset,     // deprecated
    simi_newton_joint_pospid1,
    simi_newton_joint_pospid2,
    simi_newton_joint_pospid3,
};

enum
{                                       /* Newton joint int params */
    simi_newton_joint_objectid = 0,     // deprecated
    simi_newton_joint_dependentobjectid // deprecated
};

enum
{ /* Mujoco joint double params */
    simi_mujoco_joint_solreflimit1 = 0,
    simi_mujoco_joint_solreflimit2,
    simi_mujoco_joint_solimplimit1,
    simi_mujoco_joint_solimplimit2,
    simi_mujoco_joint_solimplimit3,
    simi_mujoco_joint_solimplimit4,
    simi_mujoco_joint_solimplimit5,
    simi_mujoco_joint_frictionloss,
    simi_mujoco_joint_solreffriction1,
    simi_mujoco_joint_solreffriction2,
    simi_mujoco_joint_solimpfriction1,
    simi_mujoco_joint_solimpfriction2,
    simi_mujoco_joint_solimpfriction3,
    simi_mujoco_joint_solimpfriction4,
    simi_mujoco_joint_solimpfriction5,
    simi_mujoco_joint_stiffness,
    simi_mujoco_joint_damping,
    simi_mujoco_joint_springref,
    simi_mujoco_joint_springdamper1,
    simi_mujoco_joint_springdamper2,
    simi_mujoco_joint_armature,
    simi_mujoco_joint_margin,
    simi_mujoco_joint_polycoef1, // deprecated
    simi_mujoco_joint_polycoef2, // deprecated
    simi_mujoco_joint_polycoef3,
    simi_mujoco_joint_polycoef4,
    simi_mujoco_joint_polycoef5,
    simi_mujoco_joint_pospid1,
    simi_mujoco_joint_pospid2,
    simi_mujoco_joint_pospid3,
};

enum
{                                       /* Mujoco joint int params */
    simi_mujoco_joint_objectid = 0,     // deprecated
    simi_mujoco_joint_dependentobjectid // deprecated
};

class CJoint : public CSceneObject
{
  public:
    CJoint(); // default, use only during serialization!
    CJoint(int jointType);
    virtual ~CJoint();

    void buildOrUpdate_oldIk();
    void connect_oldIk();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor *ev);
    CSceneObject *copyYourself();
    void removeSceneDependencies();
    void scaleObject(double scalingFactor);
    void serialize(CSer &ar);
    void announceCollectionWillBeErased(int groupID, bool copyBuffer);
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer);
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer);
    void announceObjectWillBeErased(const CSceneObject *object, bool copyBuffer);
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer);
    void performObjectLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performIkLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollectionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performCollisionLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performDistanceLoadingMapping(const std::map<int, int> *map, bool loadingAmodel);
    void performTextureObjectLoadingMapping(const std::map<int, int> *map);
    void performDynMaterialObjectLoadingMapping(const std::map<int, int> *map);
    void simulationAboutToStart();
    void simulationEnded();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void computeBoundingBox();
    void setIsInScene(bool s);

    int setBoolProperty(const char* pName, bool pState, CCbor* eev = nullptr);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState, CCbor* eev = nullptr);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState, CCbor* eev = nullptr);
    int getFloatProperty(const char* pName, double& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setVectorProperty(const char* pName, const double* v, int vL, CCbor* eev = nullptr);
    int getVectorProperty(const char* pName, std::vector<double>& pState) const;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance) const;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, int& size) const;
    static int getPropertyInfo_static(const char* pName, int& info, int& size);
    // Some helpers:
    bool getBoolPropertyValue(const char* pName) const;
    int getIntPropertyValue(const char* pName) const;
    double getFloatPropertyValue(const char* pName) const;

    // Overridden from CSceneObject:
    virtual C7Vector getIntrinsicTransformation(bool includeDynErrorComponent, bool *available = nullptr) const;
    virtual C7Vector getFullLocalTransformation() const;

    std::string getObjectTypeInfo() const;
    std::string getObjectTypeInfoExtended() const;

    double getLength() const;
    double getDiameter() const;
    double getPosition() const;
    double getTargetVelocity() const;
    bool getMotorLock() const;
    double getTargetForce(bool signedValue) const;
    int getDynCtrlMode() const;
    int getDynVelCtrlType() const;
    int getDynPosCtrlType() const;
    double getEngineFloatParam_old(int what, bool *ok) const;
    int getEngineIntParam_old(int what, bool *ok) const;
    bool getEngineBoolParam_old(int what, bool *ok) const;

    void getVortexFloatParams(std::vector<double> &p) const;
    void getVortexIntParams(std::vector<int> &p) const;
    void getNewtonFloatParams(std::vector<double> &p) const;
    void getNewtonIntParams(std::vector<int> &p) const;
    int getVortexDependentJointId() const;
    int getNewtonDependentJointId() const;
    int getMujocoDependentJointId() const;
    void getMaxVelAccelJerk(double maxVelAccelJerk[3]) const;
    double getScrewLead() const;
    int getJointType() const;
    C4Vector getSphericalTransformation() const;
    bool getIsCyclic() const;
    double getPositionMin() const;
    double getPositionRange() const;
    int getJointMode() const;
    int getDependencyMasterJointHandle() const;
    double getDependencyJointMult() const;
    double getDependencyJointOffset() const;
    void getPid(double &p_param, double &i_param, double &d_param, int engine = -1) const;
    void getKc(double &k_param, double &c_param) const;
    double getTargetPosition() const;
    CColorObject *getColor(bool part2);

    double getMeasuredJointVelocity() const;
    std::string getDependencyJointLoadAlias() const;
    int getJointCallbackCallOrder_backwardCompatibility() const;
    void setDirectDependentJoints(const std::vector<CJoint *> &joints);

    void setPositionMin(double min);
    void setPositionRange(double range);
    void setIsCyclic(bool isCyclic);
    void setLength(double l);
    void setDiameter(double d);
    void setMaxVelAccelJerk(const double maxVelAccelJerk[3]);
    bool setScrewLead(double lead);
    void setDependencyMasterJointHandle(int depJointID);
    void setDependencyJointMult(double coeff);
    void setDependencyJointOffset(double off);
    void setVelocity(double vel, const CJoint *masterJoint = nullptr);
    void setPosition(double pos, const CJoint *masterJoint = nullptr, bool setDirect = false);
    void setSphericalTransformation(const C4Vector &tr);
    void setJointMode(int theMode);

    void setIntrinsicTransformationError(const C7Vector &tr);

    void setTargetVelocity(double v);
    void setTargetPosition(double pos);
    void setKc(double k_param, double c_param);
    void setTargetForce(double f, bool isSigned);
    void setDynCtrlMode(int mode);
    void setDynVelCtrlType(int mode);
    void setDynPosCtrlType(int mode);

    bool setJointMode_noDynMotorTargetPosCorrection(int newMode);

    void setMotorLock(bool e);

    void measureJointVelocity(double simTime);

    void getDynamicJointErrors(double &linear, double &angular) const;
    void getDynamicJointErrorsFull(C3Vector &linear, C3Vector &angular) const;

    void handleMotion();
    int handleDynJoint(int flags, const int intVals[3], double currentPosVelAccel[3], double effort, double dynStepSize,
                       double errorV, double velAndForce[2]);

    void setDynamicMotorReflectedPosition_useOnlyFromDynamicPart(double rfp, double simTime);

    void addCumulativeForceOrTorque(double forceOrTorque, int countForAverage);
    bool getDynamicForceOrTorque(double &forceOrTorque, bool dynamicStepValue) const;
    void setForceOrTorqueNotValid();

    void setKinematicMotionType(int t, bool reset, double initVel = 0.0);
    int getKinematicMotionType() const;

    bool setEngineFloatParam_old(int what, double v);
    bool setEngineIntParam_old(int what, int v);
    bool setEngineBoolParam_old(int what, bool v);

    void copyEnginePropertiesTo(CJoint *target);

    // DEPRECATED:
    bool getHybridFunctionality_old() const;
    double getIKWeight_old() const;
    double getMaxStepSize_old() const;
    std::string getDependencyJointLoadName_old() const;
    void setIKWeight_old(double newWeight);
    void setMaxStepSize_old(double stepS);
    void setHybridFunctionality_old(bool h);
    void setExplicitHandling_DEPRECATED(bool explicitHandl);
    bool getExplicitHandling_DEPRECATED();
    void resetJoint_DEPRECATED();
    void handleJoint_DEPRECATED(double deltaTime);
    void setUnlimitedAcceleration_DEPRECATED(bool unlimited);
    bool getUnlimitedAcceleration_DEPRECATED();
    void setInvertTargetVelocityAtLimits_DEPRECATED(bool invert);
    bool getInvertTargetVelocityAtLimits_DEPRECATED();
    void setMaxAcceleration_DEPRECATED(double maxAccel);
    double getMaxAcceleration_DEPRECATED();
    void setVelocity_DEPRECATED(double vel);
    double getVelocity_DEPRECATED();
    void setPid_old(double p_param, double i_param, double d_param);

  protected:
    void _sendEngineString(CCbor* eev = nullptr);
    std::string _enumToProperty(int oldEnum, int type, int& indexWithArrays) const;
    void updateSelfAsSlave();
    void _fixVortexInfVals();

    void _commonInit();
    void _sendDependencyChange() const;

    void _setPositionIntervalMin_sendOldIk(double min) const;
    void _setPositionIntervalRange_sendOldIk(double range) const;
    void _setPositionIsCyclic_sendOldIk(bool isCyclic) const;
    void _setScrewPitch_sendOldIk(double pitch) const;
    void _setDependencyJointHandle_sendOldIk(int depJointID) const;
    void _setDependencyJointMult_sendOldIk(double coeff) const;
    void _setDependencyJointOffset_sendOldIk(double off) const;
    void _setIkWeight_sendOldIk(double newWeight) const;
    void _setMaxStepSize_sendOldIk(double stepS) const;
    void _setPosition_sendOldIk(double pos) const;
    void _setSphericalTransformation_sendOldIk(const C4Vector &tr) const;
    void _setJointMode_sendOldIk(int theMode) const;

    std::vector<CJoint *> _directDependentJoints;

    std::string _dependencyJointLoadAlias;

    double _initialPosition;
    double _initialTargetPosition;
    double _initialTargetVelocity;
    C4Vector _initialSphericalJointTransformation;
    int _initialJointMode;

    int _initialDynCtrlMode;
    int _initialDynVelocityCtrlType;
    int _initialDynPositionCtrlType;
    bool _initialDynCtrl_lockAtVelZero;
    double _initialDynCtrl_force;
    double _initialDynCtrl_kc[3];

    bool _initialHybridOperation;

    double _initialMaxVelAccelJerk[3];

    double _velCalc_vel;
    double _velCalc_prevPos;
    double _velCalc_prevSimTime;
    bool _velCalc_prevPosValid;

    double _dynPosCtrl_currentVelAccel[2];
    double _dynVelCtrl_currentVelAccel[2];
    double _dynCtrl_previousVelForce[2];

    double _cumulatedForceOrTorque;
    double _cumulativeForceOrTorqueTmp;
    bool _averageForceOrTorqueValid;
    int _kinematicMotionType; // 0=none, 1=pos ctrl, 2=vel ctrl, bit4 (16): reset motion
    double _kinematicMotionInitVel;

    int _jointType;
    double _length;
    double _diameter;
    C4Vector _sphericalTransf; // spherical joints don't have a range anymore since 22.10.2022
    bool _isCyclic;
    double _screwLead; // distance along the screw's axis for one complete rotation of the screw
    double _posMin;
    double _posRange;
    double _pos;
    double _targetVel; // signed. Should be same sign as _targetForce (however _targetVel has precedence when conflict)
    double _targetPos;
    int _jointMode;
    int _dependencyMasterJointHandle;
    double _dependencyJointMult;
    double _dependencyJointOffset;

    double _maxVelAccelJerk[3];

    CColorObject _color;
    CColorObject _color_removeSoon;

    bool _motorLock;     // deprecated, should not be used anymore
    double _targetForce; // signed. Should be same sign as _targetVel (however _targetVel has precedence when conflict)
    double _dynCtrl_pid_cumulErr;

    double _dynCtrl_kc[2];
    int _dynCtrlMode;
    int _dynPositionCtrlType; // built-in velocity mode + pos PID (0) or Ruckig (1)
    int _dynVelocityCtrlType; // built-in velocity mode (0) or Ruckig (1)

    bool _jointHasHybridFunctionality;

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

    C7Vector _intrinsicTransformationError; // from physics engine

    // Following is the force/torque acquired during a single dyn. calculation step:
    double _lastForceOrTorque_dynStep;
    bool _lastForceOrTorqueValid_dynStep;

    // DEPRECATED:
    double _maxStepSize_old;
    double _ikWeight_old;
    std::string _dependencyJointLoadName_old;
    double _jointPositionForMotionHandling_DEPRECATED;
    double _velocity_DEPRECATED;
    bool _explicitHandling_DEPRECATED;
    bool _unlimitedAcceleration_DEPRECATED;
    bool _invertTargetVelocityAtLimits_DEPRECATED;
    double _maxAcceleration_DEPRECATED;
    int _jointCallbackCallOrder_backwardCompatibility;
    double _initialVelocity_DEPRECATED;
    bool _initialExplicitHandling_DEPRECATED;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase *renderingObject, int displayAttrib);
#endif
};
