#pragma once

#include <ser.h>
#include <cbor.h>
#include <simMath/3Vector.h>
#include <propertiesAndMethods.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) const SEngMaterialProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
DYNMATERIAL_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
const std::vector<SEngMaterialProperty> allProps_material = {DYNMATERIAL_PROPERTIES};
#undef FUNCX
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
    simi_mujoco_body_gap,
    simi_mujoco_body_adhesiongain,
    simi_mujoco_body_adhesionctrl,
    simi_mujoco_body_adhesionctrlrange1,
    simi_mujoco_body_adhesionctrlrange2,
    simi_mujoco_body_adhesionforcerange1,
    simi_mujoco_body_adhesionforcerange2,
    simi_mujoco_body_gravcomp,
};

enum
{ /* Mujoco body int params */
    simi_mujoco_body_condim = 0,
    simi_mujoco_body_priority,
    simi_mujoco_body_bitcoded,
    simi_mujoco_body_adhesionforcelimited,
};

enum
{ /* Mujoco body bit params */
    simi_mujoco_body_adhesion = 1
};

class CDynMaterialObject
{
  public:
    CDynMaterialObject();
    virtual ~CDynMaterialObject();

    void setObjectID(int newID);
    int getObjectID();
    void setObjectName(const char* newName);
    void setShapeHandleForEvents(int h);
    void generateDefaultMaterial(int defMatId);
    static std::string getDefaultMaterialName(int defMatId);

    std::string getObjectName();
    CDynMaterialObject* copyYourself();
    void serialize(CSer& ar);

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
    int setVector2Property(const char* pName, const double* pState, CCbor* eev = nullptr);
    int getVector2Property(const char* pName, double* pState) const;
    int setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev = nullptr);
    int getVector3Property(const char* pName, C3Vector* pState) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL, CCbor* eev = nullptr);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    int getPropertyName(int& index, std::string& pName, int excludeFlags) const;
    static int getPropertyName_static(int& index, std::string& pName, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);
    // Some helpers:
    bool getBoolPropertyValue(const char* pName) const;
    int getIntPropertyValue(const char* pName) const;
    double getFloatPropertyValue(const char* pName) const;

    // Engine properties
    // ---------------------
    double getEngineFloatParam_old(int what, bool* ok);
    int getEngineIntParam_old(int what, bool* ok);
    bool getEngineBoolParam_old(int what, bool* ok);
    bool setEngineFloatParam_old(int what, double v);
    bool setEngineIntParam_old(int what, int v);
    bool setEngineBoolParam_old(int what, bool v);

    void getVortexFloatParams(std::vector<double>& p);
    void getVortexIntParams(std::vector<int>& p);

    void getNewtonFloatParams(std::vector<double>& p);
    void getNewtonIntParams(std::vector<int>& p);
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
