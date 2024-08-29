#pragma once

#include <ser.h>
#include <cbor.h>
#include <simMath/3Vector.h>

struct SEngMaterialProperty {
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propMaterial_engineProperties,            "engineProperties",         sim_propertytype_string, 0, -1, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletRestitution,           "bulletRestitution",        sim_propertytype_float, 0, sim_bullet_body_restitution, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletFriction0,             "bulletFrictionOld",        sim_propertytype_float, 0, sim_bullet_body_oldfriction, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletFriction,              "bulletFriction",           sim_propertytype_float, 0, sim_bullet_body_friction, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletLinearDamping,         "bulletLinearDamping",      sim_propertytype_float, 0, sim_bullet_body_lineardamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletAngularDamping,        "bulletAngularDamping",     sim_propertytype_float, 0, sim_bullet_body_angulardamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactor,        "bulletCustomCollisionMargin",           sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactor, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginFactorConvex,  "bulletCustomCollisionMarginConvex",     sim_propertytype_float, 0, sim_bullet_body_nondefaultcollisionmargingfactorconvex, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletSticky,                "bulletStickyContact",             sim_propertytype_bool,  0, sim_bullet_body_sticky, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletNonDefaultCollisionMargin,              "bulletCustomCollisionMarginEnabled",                 sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmargin, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletNonDefaultCollisionMarginConvex,        "bulletCustomCollisionMarginConvexEnabled",           sim_propertytype_bool, 0, sim_bullet_body_usenondefaultcollisionmarginconvex, -1, -1, -1, -1) \
    FUNCX(propMaterial_bulletAutoShrinkConvex,       "bulletAutoShrinkConvexMeshes",   sim_propertytype_bool,  0, sim_bullet_body_autoshrinkconvex, -1, -1, -1, -1) \
    FUNCX(propMaterial_odeFriction,                 "odeFriction",              sim_propertytype_float, 0, sim_ode_body_friction, -1, -1, -1, -1) \
    FUNCX(propMaterial_odeSoftErp,                  "odeSoftErp",               sim_propertytype_float, 0, sim_ode_body_softerp, -1, -1, -1, -1) \
    FUNCX(propMaterial_odeSoftCfm,                  "odeSoftCfm",               sim_propertytype_float, 0, sim_ode_body_softcfm, -1, -1, -1, -1) \
    FUNCX(propMaterial_odeLinearDamping,            "odeLinearDamping",         sim_propertytype_float, 0, sim_ode_body_lineardamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_odeAngularDamping,           "odeAngularDamping",        sim_propertytype_float, 0, sim_ode_body_angulardamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_odeMaxContacts,              "odeMaxContacts",           sim_propertytype_int,   0, sim_ode_body_maxcontacts, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFriction,  "vortexLinearPrimaryAxisFrictionCoeff",              sim_propertytype_float, 0, sim_vortex_body_primlinearaxisfriction, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFriction,   "vortexLinearSecondaryAxisFrictionCoeff",               sim_propertytype_float, 0, sim_vortex_body_seclinearaxisfriction,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFriction, "vortexAngularPrimaryAxisFrictionCoeff",             sim_propertytype_float, 0, sim_vortex_body_primangularaxisfriction,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFriction,  "vortexAngularSecondaryAxisFrictionCoeff",              sim_propertytype_float, 0, sim_vortex_body_secangularaxisfriction,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexNormalAngularAxisFriction,"vortexAngularNormalAxisFrictionCoeff",          sim_propertytype_float, 0, sim_vortex_body_normalangularaxisfriction,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale,  "vortexLinearPrimaryAxisStaticFrictionScale",              sim_propertytype_float, 0, sim_vortex_body_primlinearaxisstaticfrictionscale, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale,   "vortexLinearSecondaryAxisStaticFrictionScale",               sim_propertytype_float, 0, sim_vortex_body_seclinearaxisstaticfrictionscale,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale, "vortexAngularPrimaryAxisStaticFrictionScale",             sim_propertytype_float, 0, sim_vortex_body_primangularaxisstaticfrictionscale,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale,  "vortexAngularSecondaryAxisStaticFrictionScale",              sim_propertytype_float, 0, sim_vortex_body_secangularaxisstaticfrictionscale,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexNormalAngularAxisStaticFrictionScale,"vortexAngularNormalAxisStaticFrictionScale",          sim_propertytype_float, 0, sim_vortex_body_normalangularaxisstaticfrictionscale,  -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexCompliance,            "vortexCompliance",         sim_propertytype_float, 0, sim_vortex_body_compliance, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexDamping,               "vortexDamping",            sim_propertytype_float, 0, sim_vortex_body_damping, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexRestitution,           "vortexRestitution",        sim_propertytype_float, 0, sim_vortex_body_restitution, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexRestitutionThreshold,  "vortexRestitutionThreshold", sim_propertytype_float, 0, sim_vortex_body_restitutionthreshold, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAdhesiveForce,         "vortexAdhesiveForce",        sim_propertytype_float, 0, sim_vortex_body_adhesiveforce, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexLinearVelocityDamping, "vortexLinearVelDamping", sim_propertytype_float, 0, sim_vortex_body_linearvelocitydamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAngularVelocityDamping,"vortexAngularVelDamping", sim_propertytype_float, 0, sim_vortex_body_angularvelocitydamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlide,"vortexLinearPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslide, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlide,"vortexLinearSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslide, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlide,"vortexAngularPrimaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslide, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlide,"vortexAngularSecondaryAxisSlide", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslide, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexNormalAngularAxisSlide,"vortexAngularNormalAxisSlide", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslide, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryLinearAxisSlip,"vortexLinearPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primlinearaxisslip, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSlip,"vortexLinearSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_seclinearaxisslip, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryAngularAxisSlip,"vortexAngularPrimaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_primangularaxisslip, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSlip,"vortexAngularSecondaryAxisSlip", sim_propertytype_float, 0, sim_vortex_body_secangularaxisslip, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexNormalAngularAxisSlip,"vortexAngularNormalAxisSlip", sim_propertytype_float, 0, sim_vortex_body_normalangularaxisslip, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoSleepLinearSpeedThreshold,"vortexAutoSleepThresholdLinearSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearspeedthreshold, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoSleepLinearAccelerationThreshold,"vortexAutoSleepThresholdLinearAccel", sim_propertytype_float, 0, sim_vortex_body_autosleeplinearaccelthreshold, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoSleepAngularSpeedThreshold,"vortexAutoSleepThresholdAngularSpeed", sim_propertytype_float, 0, sim_vortex_body_autosleepangularspeedthreshold, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoSleepAngularAccelerationThreshold,"vortexAutoSleepThresholdAngularAccel", sim_propertytype_float, 0, sim_vortex_body_autosleepangularaccelthreshold, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSkinThickness,"vortexSkinThickness", sim_propertytype_float, 0, sim_vortex_body_skinthickness, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoAngularDampingTensionRatio,"vortexAutoAngularDampingTensionRatio", sim_propertytype_float, 0, sim_vortex_body_autoangulardampingtensionratio, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryAxisVector,"vortexLinearPrimaryAxisOrient", sim_propertytype_vector3, 0, sim_vortex_body_primaxisvectorx, sim_vortex_body_primaxisvectory, sim_vortex_body_primaxisvectorz, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryLinearAxisFrictionModel,              "vortexLinearPrimaryAxisFrictionModel",           sim_propertytype_int,   0, sim_vortex_body_primlinearaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryLinearAxisFrictionModel,            "vortexLinearSecondaryAxisFrictionModel",         sim_propertytype_int,   0, sim_vortex_body_seclinearaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimaryAngularAxisFrictionModel,             "vortexAngularPrimaryAxisFrictionModel",          sim_propertytype_int,   0, sim_vortex_body_primangulararaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryAngularAxisFrictionModel,           "vortexAngularSecondaryAxisFrictionModel",        sim_propertytype_int,   0, sim_vortex_body_secangularaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexNormalAngularAxisFrictionModel,              "vortexAngularNormalAxisFrictionModel",           sim_propertytype_int,   0, sim_vortex_body_normalangularaxisfrictionmodel, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoSleepStepLiveThreshold,                  "vortexAutoSleepThresholdSteps",                  sim_propertytype_int,   0, sim_vortex_body_autosleepsteplivethreshold, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexMaterialUniqueId,                            "vortexMaterialUniqueId",                         sim_propertytype_int,   0, sim_vortex_body_materialuniqueid, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexPrimitiveShapesAsConvex,                     "vortexPureShapeAsConvex",                       sim_propertytype_bool,  0, sim_vortex_body_pureshapesasconvex, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexConvexShapesAsRandom,                        "vortexConvexShapeAsMesh",                       sim_propertytype_bool,  0, sim_vortex_body_convexshapesasrandom, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexRandomShapesAsTerrain,                       "vortexRandomShapeAsTerrain",                     sim_propertytype_bool,  0, sim_vortex_body_randomshapesasterrain, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexFastMoving,                                  "vortexFastMoving",                               sim_propertytype_bool,  0, sim_vortex_body_fastmoving, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoSlip,                                    "vortexAutoSlip",                                 sim_propertytype_bool,  0, sim_vortex_body_autoslip, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis,  "vortexLinearSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool,  0, sim_vortex_body_seclinaxissameasprimlinaxis, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis,"vortexAngularSecondaryAxisFollowPrimaryAxis", sim_propertytype_bool,  0, sim_vortex_body_secangaxissameasprimangaxis, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis,   "vortexAngularNormalAxisFollowPrimaryAxis", sim_propertytype_bool,  0, sim_vortex_body_normangaxissameasprimangaxis, -1, -1, -1, -1) \
    FUNCX(propMaterial_vortexAutoAngularDamping,                          "vortexAutoAngularDampingEnabled", sim_propertytype_bool,  0, sim_vortex_body_autoangulardamping, -1, -1, -1, -1) \
    FUNCX(propMaterial_newtonStaticFriction,                "newtonStaticFriction",         sim_propertytype_float, 0,      sim_newton_body_staticfriction, -1, -1, -1, -1) \
    FUNCX(propMaterial_newtonKineticFriction,               "newtonKineticFriction",        sim_propertytype_float, 0,      sim_newton_body_kineticfriction, -1, -1, -1, -1) \
    FUNCX(propMaterial_newtonRestitution,                   "newtonRestitution",            sim_propertytype_float, 0,      sim_newton_body_restitution, -1, -1, -1, -1) \
    FUNCX(propMaterial_newtonLinearDrag,                    "newtonLinearDrag",             sim_propertytype_float, 0,      sim_newton_body_lineardrag, -1, -1, -1, -1) \
    FUNCX(propMaterial_newtonAngularDrag,                   "newtonAngularDrag",            sim_propertytype_float, 0,      sim_newton_body_angulardrag, -1, -1, -1, -1) \
    FUNCX(propMaterial_newtonFastMoving,                    "newtonFastMoving",             sim_propertytype_bool,  0,      sim_newton_body_fastmoving, -1, -1, -1, -1) \
    FUNCX(propMaterial_mujocoFriction,                      "mujocoFriction",               sim_propertytype_vector3, 0,    sim_mujoco_body_friction1, sim_mujoco_body_friction2, sim_mujoco_body_friction3, -1, -1) \
    FUNCX(propMaterial_mujocoSolref,                        "mujocoSolref",                 sim_propertytype_vector, 0,     sim_mujoco_body_solref1, sim_mujoco_body_solref2, -1, -1, -1) \
    FUNCX(propMaterial_mujocoSolimp,                        "mujocoSolimp",                 sim_propertytype_vector, 0,     sim_mujoco_body_solimp1, sim_mujoco_body_solimp2, sim_mujoco_body_solimp3, sim_mujoco_body_solimp4, sim_mujoco_body_solimp5) \
    FUNCX(propMaterial_mujocoSolmix,                        "mujocoSolmix",                 sim_propertytype_float, 0,      sim_mujoco_body_solmix, -1, -1, -1, -1) \
    FUNCX(propMaterial_mujocoMargin,                        "mujocoMargin",                 sim_propertytype_float, 0,      sim_mujoco_body_margin, -1, -1, -1, -1) \
    FUNCX(propMaterial_mujocoCondim,                        "mujocoCondim",                 sim_propertytype_int, 0,        sim_mujoco_body_condim, -1, -1, -1, -1) \
    FUNCX(propMaterial_mujocoPriority,                      "mujocoPriority",               sim_propertytype_int, 0,        sim_mujoco_body_priority, -1, -1, -1, -1) \


#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4) const SEngMaterialProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4) name,
const std::vector<SEngMaterialProperty> allProps_material = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

enum
{ /* Bullet body double params */
    simi_bullet_body_restitution = 0,
    simi_bullet_body_oldfriction,
    simi_bullet_body_friction,
    simi_bullet_body_lineardamping,
    simi_bullet_body_angulardamping,
    simi_bullet_body_nondefaultcollisionmargingfactor,
    simi_bullet_body_nondefaultcollisionmargingfactorconvex
};

enum
{ /* Bullet body int params */
    simi_bullet_body_bitcoded = 0
};

enum
{ /* Bullet body bit params */
    simi_bullet_body_sticky = 1,
    simi_bullet_body_usenondefaultcollisionmargin = 2,
    simi_bullet_body_usenondefaultcollisionmarginconvex = 4,
    simi_bullet_body_autoshrinkconvex = 8
};

enum
{ /* Ode body double params */
    simi_ode_body_friction = 0,
    simi_ode_body_softerp,
    simi_ode_body_softcfm,
    simi_ode_body_lineardamping,
    simi_ode_body_angulardamping
};

enum
{ /* Ode body int params */
    simi_ode_body_maxcontacts = 0
};

// enum { /* Ode body bit params */
//     /* if you add something here, search for obb11032016 */
// };

enum
{ /* Vortex body double params */
    simi_vortex_body_primlinearaxisfriction = 0,
    simi_vortex_body_seclinearaxisfriction,
    simi_vortex_body_primangularaxisfriction,
    simi_vortex_body_secangularaxisfriction,
    simi_vortex_body_normalangularaxisfriction,
    simi_vortex_body_primlinearaxisstaticfrictionscale,
    simi_vortex_body_seclinearaxisstaticfrictionscale,
    simi_vortex_body_primangularaxisstaticfrictionscale,
    simi_vortex_body_secangularaxisstaticfrictionscale,
    simi_vortex_body_normalangularaxisstaticfrictionscale,
    simi_vortex_body_compliance,
    simi_vortex_body_damping,
    simi_vortex_body_restitution,
    simi_vortex_body_restitutionthreshold,
    simi_vortex_body_adhesiveforce,
    simi_vortex_body_linearvelocitydamping,
    simi_vortex_body_angularvelocitydamping,
    simi_vortex_body_primlinearaxisslide,
    simi_vortex_body_seclinearaxisslide,
    simi_vortex_body_primangularaxisslide,
    simi_vortex_body_secangularaxisslide,
    simi_vortex_body_normalangularaxisslide,
    simi_vortex_body_primlinearaxisslip,
    simi_vortex_body_seclinearaxisslip,
    simi_vortex_body_primangularaxisslip,
    simi_vortex_body_secangularaxisslip,
    simi_vortex_body_normalangularaxisslip,
    simi_vortex_body_autosleeplinearspeedthreshold,
    simi_vortex_body_autosleeplinearaccelthreshold,
    simi_vortex_body_autosleepangularspeedthreshold,
    simi_vortex_body_autosleepangularaccelthreshold,
    simi_vortex_body_skinthickness,
    simi_vortex_body_autoangulardampingtensionratio,
    simi_vortex_body_primaxisvectorx,
    simi_vortex_body_primaxisvectory,
    simi_vortex_body_primaxisvectorz
};

enum
{ /* Vortex body int params */
    simi_vortex_body_primlinearaxisfrictionmodel = 0,
    simi_vortex_body_seclinearaxisfrictionmodel,
    simi_vortex_body_primangulararaxisfrictionmodel,
    simi_vortex_body_secmangulararaxisfrictionmodel,
    simi_vortex_body_normalmangulararaxisfrictionmodel,
    simi_vortex_body_bitcoded,
    simi_vortex_body_autosleepsteplivethreshold,
    simi_vortex_body_materialuniqueid
};

enum
{ /* Vortex body bit params */
    simi_vortex_body_pureshapesasconvex = 1,
    simi_vortex_body_convexshapesasrandom = 2,
    simi_vortex_body_randomshapesasterrain = 4,
    simi_vortex_body_fastmoving = 8,
    simi_vortex_body_autoslip = 16,
    simi_vortex_body_seclinaxissameasprimlinaxis = 32,
    simi_vortex_body_secangaxissameasprimangaxis = 64,
    simi_vortex_body_normangaxissameasprimangaxis = 128,
    simi_vortex_body_autoangulardamping = 256
};

enum
{ /* Newton body double params */
    simi_newton_body_staticfriction = 0,
    simi_newton_body_kineticfriction,
    simi_newton_body_restitution,
    simi_newton_body_lineardrag,
    simi_newton_body_angulardrag
};

enum
{ /* Newton body int params */
    simi_newton_body_bitcoded = 0
};

enum
{ /* Newton body bit params */
    simi_newton_body_fastmoving = 1
};

enum
{ /* Mujoco body double params */
    simi_mujoco_body_friction1 = 0,
    simi_mujoco_body_friction2,
    simi_mujoco_body_friction3,
    simi_mujoco_body_solref1,
    simi_mujoco_body_solref2,
    simi_mujoco_body_solimp1,
    simi_mujoco_body_solimp2,
    simi_mujoco_body_solimp3,
    simi_mujoco_body_solimp4,
    simi_mujoco_body_solimp5,
    simi_mujoco_body_solmix,
    simi_mujoco_body_margin,
};

enum
{ /* Mujoco body int params */
    simi_mujoco_body_condim = 0,
    simi_mujoco_body_priority,
};

class CDynMaterialObject
{
  public:
    CDynMaterialObject();
    virtual ~CDynMaterialObject();

    void setObjectID(int newID);
    int getObjectID();
    void setObjectName(const char *newName);
    void setShapeHandleForEvents(int h);
    void generateDefaultMaterial(int defMatId);
    static std::string getDefaultMaterialName(int defMatId);

    std::string getObjectName();
    CDynMaterialObject *copyYourself();
    void serialize(CSer &ar);

    static std::string getIndividualName();

    void sendEngineString(CCbor* eev = nullptr);

    int setBoolProperty(const char* pName, bool pState, CCbor* eev = nullptr);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState, CCbor* eev = nullptr);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState, CCbor* eev = nullptr);
    int getFloatProperty(const char* pName, double& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev = nullptr);
    int getVector3Property(const char* pName, C3Vector* pState) const;
    int setVectorProperty(const char* pName, const double* v, int vL, CCbor* eev = nullptr);
    int getVectorProperty(const char* pName, std::vector<double>& pState) const;
    int getPropertyName(int& index, std::string& pName) const;
    static int getPropertyName_static(int& index, std::string& pName);
    int getPropertyInfo(const char* pName, int& info, int& size) const;
    static int getPropertyInfo_static(const char* pName, int& info, int& size);

    // Engine properties
    // ---------------------
    double getEngineFloatParam(int what, bool *ok);
    int getEngineIntParam(int what, bool *ok);
    bool getEngineBoolParam(int what, bool *ok);
    bool setEngineFloatParam(int what, double v);
    bool setEngineIntParam(int what, int v);
    bool setEngineBoolParam(int what, bool v);

    void getBulletFloatParams(std::vector<double> &p);
    void setBulletFloatParams(const std::vector<double> &p);
    void getBulletIntParams(std::vector<int> &p);
    void setBulletIntParams(const std::vector<int> &p);

    void getOdeFloatParams(std::vector<double> &p);
    void setOdeFloatParams(const std::vector<double> &p);
    void getOdeIntParams(std::vector<int> &p);
    void setOdeIntParams(const std::vector<int> &p);

    void getVortexFloatParams(std::vector<double> &p);
    void setVortexFloatParams(const std::vector<double> &p);
    void getVortexIntParams(std::vector<int> &p);
    void setVortexIntParams(const std::vector<int> &p);

    void getNewtonFloatParams(std::vector<double> &p);
    void setNewtonFloatParams(const std::vector<double> &p);
    void getNewtonIntParams(std::vector<int> &p);
    void setNewtonIntParams(const std::vector<int> &p);

    void getMujocoFloatParams(std::vector<double> &p);
    void setMujocoFloatParams(const std::vector<double> &p);
    void getMujocoIntParams(std::vector<int> &p);
    void setMujocoIntParams(const std::vector<int> &p);
    // ---------------------

  protected:
    std::string _enumToProperty(int oldEnum, int type, int& indexWithArrays) const;
    void _fixVortexInfVals();
    void _setDefaultParameters();
    static int _nextUniqueID;

    int _objectID;
    std::string _objectName;
    int _uniqueID;
    int _shapeHandleForEvents;
    bool _sendAlsoAllEngineProperties;

    // Engine properties
    // ---------------------
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
    // ---------------------
};
