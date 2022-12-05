#pragma once

#include "ser.h"

enum { /* Bullet body double params */
    simi_bullet_body_restitution=0,
    simi_bullet_body_oldfriction,
    simi_bullet_body_friction,
    simi_bullet_body_lineardamping,
    simi_bullet_body_angulardamping,
    simi_bullet_body_nondefaultcollisionmargingfactor,
    simi_bullet_body_nondefaultcollisionmargingfactorconvex
};

enum { /* Bullet body int params */
    simi_bullet_body_bitcoded=0
};

enum { /* Bullet body bit params */
    simi_bullet_body_sticky=1,
    simi_bullet_body_usenondefaultcollisionmargin=2,
    simi_bullet_body_usenondefaultcollisionmarginconvex=4,
    simi_bullet_body_autoshrinkconvex=8
};

enum { /* Ode body double params */
    simi_ode_body_friction=0,
    simi_ode_body_softerp,
    simi_ode_body_softcfm,
    simi_ode_body_lineardamping,
    simi_ode_body_angulardamping
};

enum { /* Ode body int params */
    simi_ode_body_maxcontacts=0
};

// enum { /* Ode body bit params */
//     /* if you add something here, search for obb11032016 */
// };

enum { /* Vortex body double params */
    simi_vortex_body_primlinearaxisfriction=0,
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

enum { /* Vortex body int params */
    simi_vortex_body_primlinearaxisfrictionmodel=0,
    simi_vortex_body_seclinearaxisfrictionmodel,
    simi_vortex_body_primangulararaxisfrictionmodel,
    simi_vortex_body_secmangulararaxisfrictionmodel,
    simi_vortex_body_normalmangulararaxisfrictionmodel,
    simi_vortex_body_bitcoded,
    simi_vortex_body_autosleepsteplivethreshold,
    simi_vortex_body_materialuniqueid
};

enum { /* Vortex body bit params */
    simi_vortex_body_pureshapesasconvex=1,
    simi_vortex_body_convexshapesasrandom=2,
    simi_vortex_body_randomshapesasterrain=4,
    simi_vortex_body_fastmoving=8,
    simi_vortex_body_autoslip=16,
    simi_vortex_body_seclinaxissameasprimlinaxis=32,
    simi_vortex_body_secangaxissameasprimangaxis=64,
    simi_vortex_body_normangaxissameasprimangaxis=128,
    simi_vortex_body_autoangulardamping=256
};

enum { /* Newton body double params */
    simi_newton_body_staticfriction=0,
    simi_newton_body_kineticfriction,
    simi_newton_body_restitution,
    simi_newton_body_lineardrag,
    simi_newton_body_angulardrag
};

enum { /* Newton body int params */
    simi_newton_body_bitcoded=0
};

enum { /* Newton body bit params */
    simi_newton_body_fastmoving=1
};

enum { /* Mujoco body double params */
    simi_mujoco_body_friction1=0,
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

enum { /* Mujoco body int params */
    simi_mujoco_body_condim=0,
    simi_mujoco_body_priority,
};

class CDynMaterialObject
{
public:
    CDynMaterialObject();
    virtual ~CDynMaterialObject();

    void setObjectID(int newID);
    int getObjectID();
    void setObjectName(const char* newName);
    void generateDefaultMaterial(int defMatId);
    static std::string getDefaultMaterialName(int defMatId);

    std::string getObjectName();
    CDynMaterialObject* copyYourself();
    void serialize(CSer& ar);

    static std::string getIndividualName();


    // Engine properties
    // ---------------------
    double getEngineFloatParam(int what,bool* ok);
    int getEngineIntParam(int what,bool* ok);
    bool getEngineBoolParam(int what,bool* ok);
    bool setEngineFloatParam(int what,double v);
    bool setEngineIntParam(int what,int v);
    bool setEngineBoolParam(int what,bool v);

    void getBulletFloatParams(std::vector<double>& p);
    void setBulletFloatParams(const std::vector<double>& p);
    void getBulletIntParams(std::vector<int>& p);
    void setBulletIntParams(const std::vector<int>& p);

    void getOdeFloatParams(std::vector<double>& p);
    void setOdeFloatParams(const std::vector<double>& p);
    void getOdeIntParams(std::vector<int>& p);
    void setOdeIntParams(const std::vector<int>& p);

    void getVortexFloatParams(std::vector<double>& p);
    void setVortexFloatParams(const std::vector<double>& p);
    void getVortexIntParams(std::vector<int>& p);
    void setVortexIntParams(const std::vector<int>& p);

    void getNewtonFloatParams(std::vector<double>& p);
    void setNewtonFloatParams(const std::vector<double>& p);
    void getNewtonIntParams(std::vector<int>& p);
    void setNewtonIntParams(const std::vector<int>& p);

    void getMujocoFloatParams(std::vector<double>& p);
    void setMujocoFloatParams(const std::vector<double>& p);
    void getMujocoIntParams(std::vector<int>& p);
    void setMujocoIntParams(const std::vector<int>& p);
    // ---------------------

    
protected:
    void _fixVortexInfVals();
    void _setDefaultParameters();
    static int _nextUniqueID;

    int _objectID;
    std::string _objectName;
    int _uniqueID;

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
