#include <simInternal.h>
#include <dynMaterialObject.h>
#include <app.h>
#include <tt.h>
#include <boost/format.hpp>
#include <engineProperties.h>
#include <utils.h>

int CDynMaterialObject::_nextUniqueID = 0;

CDynMaterialObject::CDynMaterialObject()
{
    _setDefaultParameters();
}

void CDynMaterialObject::_setDefaultParameters()
{
    _objectID = SIM_IDSTART_DYNMATERIAL_old;
    _objectName = "usr_";
    _shapeHandleForEvents = -1;
    _sendAlsoAllEngineProperties = true;

    _uniqueID = _nextUniqueID++;
    _bulletFloatParams.clear();
    _bulletIntParams.clear();
    _odeFloatParams.clear();
    _odeIntParams.clear();
    _vortexFloatParams.clear();
    _vortexIntParams.clear();
    _newtonFloatParams.clear();
    _newtonIntParams.clear();
    _mujocoFloatParams.clear();
    _mujocoIntParams.clear();

    // Bullet parameters:
    // ----------------------------------------------------
    _bulletFloatParams.push_back(0.0);   // simi_bullet_body_restitution
    _bulletFloatParams.push_back(0.71);  // simi_bullet_body_oldfriction
    _bulletFloatParams.push_back(0.71);  // simi_bullet_body_friction
    _bulletFloatParams.push_back(0.0);   // simi_bullet_body_lineardamping
    _bulletFloatParams.push_back(0.0);   // simi_bullet_body_angulardamping
    _bulletFloatParams.push_back(0.1);   // simi_bullet_body_nondefaultcollisionmargingfactor
    _bulletFloatParams.push_back(0.002); // simi_bullet_body_nondefaultcollisionmargingfactorconvex
    _bulletFloatParams.push_back(0.0);   // free

    int bulletBitCoded = simi_bullet_body_usenondefaultcollisionmarginconvex;
    // above is bit-coded: 1=_bulletStickyContact, 2=_bulletNonDefaultCollisionMargin,
    // 4=_bulletNonDefaultCollisionMargin_forConvexAndNonPureShape, 8=_bulletAutoShrinkConvexMesh
    _bulletIntParams.push_back(bulletBitCoded); // simi_bullet_body_bitcoded
    _bulletIntParams.push_back(0);              // free
    // ----------------------------------------------------

    // ODE parameters:
    // ----------------------------------------------------
    _odeFloatParams.push_back(
        0.71);                      // simi_ode_body_friction (box on 26.8deg inclined plane slightly slides (same mat. for box and plane)
    _odeFloatParams.push_back(0.2); // simi_ode_body_softerp
    _odeFloatParams.push_back(0.0); // simi_ode_body_softcfm
    _odeFloatParams.push_back(0.0); // simi_ode_body_lineardamping
    _odeFloatParams.push_back(0.0); // simi_ode_body_angulardamping
    _odeFloatParams.push_back(0.0); // free

    _odeIntParams.push_back(64); // simi_ode_body_maxcontacts
    int odeBitCoded = 0;         // not used for now
    _odeIntParams.push_back(odeBitCoded);
    _odeIntParams.push_back(0); // free
    // ----------------------------------------------------

    // Vortex parameters:
    // ----------------------------------------------------
    _vortexFloatParams.push_back(0.5);        // simi_vortex_body_primlinearaxisfriction
    _vortexFloatParams.push_back(0.5);        // simi_vortex_body_seclinearaxisfriction
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primangularaxisfriction
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_secangularaxisfriction
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_normalangularaxisfriction
    _vortexFloatParams.push_back(1.1);        // simi_vortex_body_primlinearaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.1);        // simi_vortex_body_seclinearaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.0);        // simi_vortex_body_primangularaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.0);        // simi_vortex_body_secangularaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.0);        // simi_vortex_body_normalangularaxisstaticfrictionscale
    _vortexFloatParams.push_back(0.00000001); // simi_vortex_body_compliance
    _vortexFloatParams.push_back(10000000.0); // simi_vortex_body_damping (was 0.0)
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_restitution
    _vortexFloatParams.push_back(0.5);        // simi_vortex_body_restitutionthreshold
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_adhesiveforce
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_linearvelocitydamping
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_angularvelocitydamping
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primlinearaxisslide
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_seclinearaxisslide
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primangularaxisslide
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_secangularaxisslide
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_normalangularaxisslide
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primlinearaxisslip
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_seclinearaxisslip
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primangularaxisslip
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_secangularaxisslip
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_normalangularaxisslip
    _vortexFloatParams.push_back(0.14);       // simi_vortex_body_autosleeplinearspeedthreshold
    _vortexFloatParams.push_back(0.045);      // simi_vortex_body_autosleeplinearaccelthreshold
    _vortexFloatParams.push_back(0.03);       // simi_vortex_body_autosleepangularspeedthreshold
    _vortexFloatParams.push_back(0.045);      // simi_vortex_body_autosleepangularaccelthreshold
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_skinthickness (was 0.002 before)
    _vortexFloatParams.push_back(0.01);       // simi_vortex_body_autoangulardampingtensionratio
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primaxisvectorx
    _vortexFloatParams.push_back(0.0);        // simi_vortex_body_primaxisvectory
    _vortexFloatParams.push_back(
        1.0); // simi_vortex_body_primaxisvectorz --> by default, primary axis aligned with Z axis (if possible)

    _vortexIntParams.push_back(
        sim_vortex_bodyfrictionmodel_scaledboxfast); // simi_vortex_body_primlinearaxisfrictionmodel
    _vortexIntParams.push_back(
        sim_vortex_bodyfrictionmodel_scaledboxfast);               // simi_vortex_body_seclinearaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_none); // simi_vortex_body_primangulararaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_none); // simi_vortex_body_secmangulararaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_none); // simi_vortex_body_normalmangulararaxisfrictionmodel
    _vortexIntParams.push_back(
        simi_vortex_body_fastmoving + simi_vortex_body_seclinaxissameasprimlinaxis +
        simi_vortex_body_secangaxissameasprimangaxis +
        simi_vortex_body_normangaxissameasprimangaxis); // simi_vortex_body_bitcoded
                                                        // bit coded: 0set=treat pure shapes as convex shapes,
                                                        // 1set=treat convex shapes as random shapes, 2set=threat random
                                                        // shapes as terrain, 3set=fast moving, 4set=auto-slip,
                                                        // 5set=sec. lin. axis uses same values as prim. lin. axis,
                                                        // 6set=sec. ang. axis uses same values as prim. ang. axis,
                                                        // 7set=norm. ang. axis uses same values as prim. ang. axis
                                                        // (bits 5-7 are not taken into account on the plugin side: they
                                                        // just serve as setting memory) 8set=auto angular damping is on
    _vortexIntParams.push_back(10);                     // simi_vortex_body_autosleepsteplivethreshold
    _vortexIntParams.push_back(
        _uniqueID); // simi_vortex_body_materialuniqueid: the material unique ID. The ID is redefined in each session
    // ----------------------------------------------------

    // Newton parameters:
    // ----------------------------------------------------
    _newtonFloatParams.push_back(0.5); // simi_newton_body_staticfriction
    _newtonFloatParams.push_back(0.5); // simi_newton_body_kineticfriction
    _newtonFloatParams.push_back(0.0); // simi_newton_body_restitution
    _newtonFloatParams.push_back(0.0); // simi_newton_body_lineardrag
    _newtonFloatParams.push_back(0.0); // simi_newton_body_angulardrag

    int newtonBitCoded = 0;
    // newtonBitCoded|=simi_newton_body_fastmoving; // for fast-moving objects
    _newtonIntParams.push_back(newtonBitCoded); // simi_newton_body_bitcoded
    // ----------------------------------------------------

    // Mujoco parameters:
    // ----------------------------------------------------
    _mujocoFloatParams.push_back(1.0);    // simi_mujoco_body_friction1
    _mujocoFloatParams.push_back(0.005);  // simi_mujoco_body_friction2
    _mujocoFloatParams.push_back(0.0001); // simi_mujoco_body_friction3
    _mujocoFloatParams.push_back(0.02);   // simi_mujoco_body_solref1
    _mujocoFloatParams.push_back(1.0);    // simi_mujoco_body_solref2
    _mujocoFloatParams.push_back(0.9);    // simi_mujoco_body_solimp1
    _mujocoFloatParams.push_back(0.95);   // simi_mujoco_body_solimp2
    _mujocoFloatParams.push_back(0.001);  // simi_mujoco_body_solimp3
    _mujocoFloatParams.push_back(0.05);   // simi_mujoco_body_solimp4
    _mujocoFloatParams.push_back(2.0);    // simi_mujoco_body_solimp5
    _mujocoFloatParams.push_back(1.0);    // simi_mujoco_body_solmix
    _mujocoFloatParams.push_back(0.0);    // simi_mujoco_body_margin
    _mujocoFloatParams.push_back(0.0);    // simi_mujoco_body_gap
    _mujocoFloatParams.push_back(1.0);    // simi_mujoco_body_adhesiongain
    _mujocoFloatParams.push_back(0.0);    // simi_mujoco_body_adhesionctrl
    _mujocoFloatParams.push_back(0.0);    // simi_mujoco_body_adhesionctrlrange1
    _mujocoFloatParams.push_back(1000.0); // simi_mujoco_body_adhesionctrlrange2
    _mujocoFloatParams.push_back(0.0);    // simi_mujoco_body_adhesionforcerange1
    _mujocoFloatParams.push_back(1000.0); // simi_mujoco_body_adhesionforcerange2
    _mujocoFloatParams.push_back(0.0);    // simi_mujoco_body_gravcomp

    _mujocoIntParams.push_back(3); // simi_mujoco_body_condim
    _mujocoIntParams.push_back(0); // simi_mujoco_body_priority
    int mujocoBitCoded = 0;        // bit0: simi_mujoco_body_adhesion
    _mujocoIntParams.push_back(mujocoBitCoded);
    _mujocoIntParams.push_back(-1); // simi_mujoco_body_adhesionforcelimited to auto
    // ----------------------------------------------------
}

CDynMaterialObject::~CDynMaterialObject()
{
}

void CDynMaterialObject::setObjectID(int newID)
{
    _objectID = newID;
}

int CDynMaterialObject::getObjectID()
{
    return (_objectID);
}

void CDynMaterialObject::setObjectName(const char* newName)
{
    _objectName = newName;
}

void CDynMaterialObject::setShapeHandleForEvents(int h)
{
    _shapeHandleForEvents = h;
}

std::string CDynMaterialObject::getObjectName()
{
    return (getIndividualName());
}

std::string CDynMaterialObject::getIndividualName()
{
    return ("<individual (i.e. not shared)>");
}

double CDynMaterialObject::getEngineFloatParam_old(int what, bool* ok)
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        double v;
        if (getFloatProperty(prop.c_str(), v) > 0)
            return v;
    }
    prop = _enumToProperty(what, sim_propertytype_floatarray, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> v;
        if (getFloatArrayProperty(prop.c_str(), v) > 0)
            return v[indexWithArrays];
    }
    prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
    if (prop.size() > 0)
    {
        C3Vector v;
        if (getVector3Property(prop.c_str(), &v) > 0)
            return v(indexWithArrays);
    }
    if (ok != nullptr)
        ok[0] = false;
    return 0.0;
}

int CDynMaterialObject::getEngineIntParam_old(int what, bool* ok)
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        int v;
        if (getIntProperty(prop.c_str(), v) > 0)
            return v;
    }
    if (ok != nullptr)
        ok[0] = false;
    return 0;
}

bool CDynMaterialObject::getEngineBoolParam_old(int what, bool* ok)
{
    if (ok != nullptr)
        ok[0] = true;
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        bool v;
        if (getBoolProperty(prop.c_str(), v) > 0)
            return v;
    }
    if (ok != nullptr)
        ok[0] = false;
    return false;
}

bool CDynMaterialObject::setEngineFloatParam_old(int what, double v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setFloatProperty(prop.c_str(), v) > 0)
            return true;
    }
    prop = _enumToProperty(what, sim_propertytype_floatarray, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> w;
        if (getFloatArrayProperty(prop.c_str(), w) > 0)
        {
            w[indexWithArrays] = v;
            if (setFloatArrayProperty(prop.c_str(), w.data(), indexWithArrays + 1) > 0)
                return true;
        }
    }
    prop = _enumToProperty(what, sim_propertytype_vector3, indexWithArrays);
    if (prop.size() > 0)
    {
        C3Vector w;
        if (getVector3Property(prop.c_str(), &w) > 0)
        {
            w(indexWithArrays) = v;
            if (setVector3Property(prop.c_str(), &w) > 0)
                return true;
        }
    }
    return false;
}

bool CDynMaterialObject::setEngineIntParam_old(int what, int v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setIntProperty(prop.c_str(), v) > 0)
            return true;
    }
    return false;
}

bool CDynMaterialObject::setEngineBoolParam_old(int what, bool v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setBoolProperty(prop.c_str(), v) > 0)
            return true;
    }
    return false;
}

void CDynMaterialObject::getVortexFloatParams(std::vector<double>& p)
{
    p.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
}

void CDynMaterialObject::getVortexIntParams(std::vector<int>& p)
{
    _vortexIntParams[7] = _uniqueID;
    p.assign(_vortexIntParams.begin(), _vortexIntParams.end());
}

void CDynMaterialObject::getNewtonFloatParams(std::vector<double>& p)
{
    p.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
}

void CDynMaterialObject::getNewtonIntParams(std::vector<int>& p)
{
    p.assign(_newtonIntParams.begin(), _newtonIntParams.end());
}

std::string CDynMaterialObject::getDefaultMaterialName(int defMatId)
{
    if (defMatId == sim_dynmat_default)
        return ("defaultMaterial");
    if (defMatId == sim_dynmat_highfriction)
        return ("highFrictionMaterial");
    if (defMatId == sim_dynmat_lowfriction)
        return ("lowFrictionMaterial");
    if (defMatId == sim_dynmat_nofriction)
        return ("noFrictionMaterial");
    if (defMatId == sim_dynmat_reststackgrasp)
        return ("restStackGraspMaterial");
    if (defMatId == sim_dynmat_foot)
        return ("footMaterial");
    if (defMatId == sim_dynmat_wheel)
        return ("wheelMaterial");
    if (defMatId == sim_dynmat_gripper)
        return ("gripperMaterial");
    if (defMatId == sim_dynmat_floor)
        return ("floorMaterial");
    return ("ERROR");
}
void CDynMaterialObject::generateDefaultMaterial(int defMatId)
{
    _setDefaultParameters();
    if (defMatId == sim_dynmat_default)
        _objectName = "defaultMaterial";
    if (defMatId == sim_dynmat_highfriction)
    {
        _objectName = "highFrictionMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 1.0;
        _bulletFloatParams[simi_bullet_body_friction] = 1.0;
        _odeFloatParams[simi_ode_body_friction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_staticfriction] = 2.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 2.0;
    }
    if (defMatId == sim_dynmat_lowfriction)
    {
        _objectName = "lowFrictionMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 0.41;
        _bulletFloatParams[simi_bullet_body_friction] = 0.20;
        _odeFloatParams[simi_ode_body_friction] = 0.41;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 0.17;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 0.17;
        _newtonFloatParams[simi_newton_body_staticfriction] = 0.17;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 0.17;
    }
    if (defMatId == sim_dynmat_nofriction)
    {
        _objectName = "noFrictionMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 0.0;
        _bulletFloatParams[simi_bullet_body_friction] = 0.0;
        _odeFloatParams[simi_ode_body_friction] = 0.0;
        _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel] = sim_vortex_bodyfrictionmodel_none;
        _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel] = sim_vortex_bodyfrictionmodel_none;
        _newtonFloatParams[simi_newton_body_staticfriction] = 0.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 0.0;
    }
    if (defMatId == sim_dynmat_reststackgrasp)
    {
        _objectName = "restStackGraspMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 1.0;
        _bulletFloatParams[simi_bullet_body_friction] = 0.5;
        _odeFloatParams[simi_ode_body_friction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_skinthickness] = 0.002;
        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_autoangulardamping;
        _newtonFloatParams[simi_newton_body_staticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_restitution] = 0.0;
    }
    if (defMatId == sim_dynmat_foot)
    {
        _objectName = "footMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 1.0;
        _bulletFloatParams[simi_bullet_body_friction] = 0.5;
        _odeFloatParams[simi_ode_body_friction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_skinthickness] = 0.002;
        _newtonFloatParams[simi_newton_body_staticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_restitution] = 0.0;
    }
    if (defMatId == sim_dynmat_wheel)
    {
        _objectName = "wheelMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 1.0;
        _bulletFloatParams[simi_bullet_body_friction] = 1.0;
        _odeFloatParams[simi_ode_body_friction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_skinthickness] = 0.0;
        _newtonFloatParams[simi_newton_body_staticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_restitution] = 0.0;
    }
    if (defMatId == sim_dynmat_gripper)
    {
        _objectName = "gripperMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 1.0;
        _bulletFloatParams[simi_bullet_body_friction] = 0.5;
        _odeFloatParams[simi_ode_body_friction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_skinthickness] = 0.002;
        _newtonFloatParams[simi_newton_body_staticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_restitution] = 0.0;
    }
    if (defMatId == sim_dynmat_floor)
    {
        _objectName = "floorMaterial";
        _bulletFloatParams[simi_bullet_body_oldfriction] = 1.0;
        _bulletFloatParams[simi_bullet_body_friction] = 1.0;
        _odeFloatParams[simi_ode_body_friction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = 1.0;
        _vortexFloatParams[simi_vortex_body_primlinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_seclinearaxisslip] = 0.0;
        _vortexFloatParams[simi_vortex_body_skinthickness] = 0.002;
        _newtonFloatParams[simi_newton_body_staticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_kineticfriction] = 1.0;
        _newtonFloatParams[simi_newton_body_restitution] = 0.0;
    }
}

CDynMaterialObject* CDynMaterialObject::copyYourself()
{
    CDynMaterialObject* newObj = new CDynMaterialObject();
    newObj->_objectID = _objectID;
    newObj->_objectName = _objectName;

    newObj->_bulletFloatParams.assign(_bulletFloatParams.begin(), _bulletFloatParams.end());
    newObj->_bulletIntParams.assign(_bulletIntParams.begin(), _bulletIntParams.end());

    newObj->_odeFloatParams.assign(_odeFloatParams.begin(), _odeFloatParams.end());
    newObj->_odeIntParams.assign(_odeIntParams.begin(), _odeIntParams.end());

    newObj->_vortexFloatParams.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
    newObj->_vortexIntParams.assign(_vortexIntParams.begin(), _vortexIntParams.end());
    newObj->_vortexIntParams[7] = newObj->_uniqueID;

    newObj->_newtonFloatParams.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
    newObj->_newtonIntParams.assign(_newtonIntParams.begin(), _newtonIntParams.end());

    newObj->_mujocoFloatParams.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
    newObj->_mujocoIntParams.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());

    return (newObj);
}

void CDynMaterialObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Ipa");
            ar << _objectID;
            ar.flush();

            ar.storeDataName("Gon");
            ar << _objectName;
            ar.flush();

            ar.storeDataName("Bcv");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, true); // for backward compatibility (29/10/2016)
            ar << dummy;
            ar.flush();

            ar.storeDataName("_o4"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (int i = 0; i < int(_vortexFloatParams.size()); i++)
                ar << _vortexFloatParams[i];
            for (int i = 0; i < int(_vortexIntParams.size()); i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("_w1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (int i = 0; i < int(_newtonFloatParams.size()); i++)
                ar << _newtonFloatParams[i];
            for (int i = 0; i < int(_newtonIntParams.size()); i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("Var"); // keep for file write backw. compat. (09/03/2016)
            unsigned char nothing = 0;
            if (_bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_usenondefaultcollisionmargin)
                nothing |= 1;
            if (_bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_sticky)
                nothing |= 2;
            if (_bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_autoshrinkconvex)
                nothing |= 4;
            if (_bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_usenondefaultcollisionmarginconvex)
                nothing |= 8;
            ar << nothing;
            ar.flush();

            ar.storeDataName("_uN"); // Bullet params, keep after "Bul" and "Var"
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (int i = 0; i < int(_bulletFloatParams.size()); i++)
                ar << _bulletFloatParams[i];
            for (int i = 0; i < int(_bulletIntParams.size()); i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("_dN"); // Ode params, keep after "Od2"
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (int i = 0; i < int(_odeFloatParams.size()); i++)
                ar << _odeFloatParams[i];
            for (int i = 0; i < int(_odeIntParams.size()); i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName("_j1"); // mujoco params:
            ar << int(_mujocoFloatParams.size()) << int(_mujocoIntParams.size());
            for (int i = 0; i < int(_mujocoFloatParams.size()); i++)
                ar << _mujocoFloatParams[i];
            for (int i = 0; i < int(_mujocoIntParams.size()); i++)
                ar << _mujocoIntParams[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            bool vortexDataLoaded = false;
            bool newtonDataLoaded = false;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Ipa") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectID;
                    }
                    if (theName.compare("Gon") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectName;
                    }
                    if (theName.compare("Bcv") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                    }
                    if (theName.compare("Bul") == 0)
                    { // keep for backw. compat. (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        float fric, bla, bli, blo, blu, ble;
                        ar >> bla >> fric >> bli >> blo >> blu >> ble;
                        _bulletFloatParams[simi_bullet_body_restitution] = (double)bla;
                        _bulletFloatParams[simi_bullet_body_lineardamping] = (double)bli;
                        _bulletFloatParams[simi_bullet_body_angulardamping] = (double)blo;
                        _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor] = (double)blu;
                        _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex] = (double)ble;
                        _bulletFloatParams[simi_bullet_body_oldfriction] = fric;
                        _bulletFloatParams[simi_bullet_body_friction] = tt::getLimitedFloat(
                            0.0, 1.0, fric); // sticky contacts have disappeared for the new Bullet, now everything is
                                             // "sticky", so make sure it is not too sticky!
                    }
                    if (theName.compare("Ode") == 0)
                    { // for backward compatibility (13/8/2015)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _odeIntParams[simi_ode_body_maxcontacts];
                        float bla, bli, blo, blu, ble;
                        ar >> bla >> bli >> blo >> blu >> ble;
                        _odeFloatParams[simi_ode_body_friction] = (double)bla;
                        _odeFloatParams[simi_ode_body_softerp] = (double)bli;
                        _odeFloatParams[simi_ode_body_softcfm] = (double)blo;
                        _odeFloatParams[simi_ode_body_lineardamping] = (double)blu;
                        _odeFloatParams[simi_ode_body_angulardamping] = (double)ble;
                        ;
                        _odeIntParams[0] = 64;
                    }
                    if (theName.compare("Od2") == 0)
                    { // keep for backw. compat. (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _odeIntParams[0];
                        float bla;
                        for (size_t i = 0; i < 5; i++)
                        {
                            ar >> bla;
                            _odeFloatParams[i] = (double)bla;
                        }
                    }
                    if (theName.compare("BuN") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_bulletFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_bulletIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_uN") == 0)
                    { // Bullet params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_bulletFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_bulletIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("OdN") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_odeFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_odeIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_dN") == 0)
                    { // Ode params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_odeFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_odeIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("Vo4") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_vortexFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_vortexIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                            // was there by mistake until 5/8/2015: _vortexIntParams[i]=vi;
                        }
                        vortexDataLoaded = true;
                    }

                    if (theName.compare("_o4") == 0)
                    { // vortex params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_vortexFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_vortexIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                            // was there by mistake until 5/8/2015: _vortexIntParams[i]=vi;
                        }
                        vortexDataLoaded = true;
                    }

                    if (theName.compare("Nw1") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_newtonFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_newtonIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        newtonDataLoaded = true;
                    }

                    if (theName.compare("_w1") == 0)
                    { // newton params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_newtonFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_newtonIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        newtonDataLoaded = true;
                    }

                    if (theName.compare("Mj1") == 0)
                    { // for backward comp. (flt->dbl)
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_mujocoFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_mujocoIntParams.size()), cnt2);

                        float vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i] = (double)vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName.compare("_j1") == 0)
                    { // mujoco params:
                        noHit = false;
                        ar >> byteQuantity;
                        int cnt1, cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b = std::min<int>(int(_mujocoFloatParams.size()), cnt1);
                        int cnt2_b = std::min<int>(int(_mujocoIntParams.size()), cnt2);

                        double vf;
                        int vi;
                        for (int i = 0; i < cnt1_b; i++)
                        { // new versions will always have same or more items in _mujocoFloatParams already!
                            ar >> vf;
                            _mujocoFloatParams[i] = vf;
                        }
                        for (int i = 0; i < cnt1 - cnt1_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i = 0; i < cnt2_b; i++)
                        { // new versions will always have same or more items in _mujocoIntParams already!
                            ar >> vi;
                            _mujocoIntParams[i] = vi;
                        }
                        for (int i = 0; i < cnt2 - cnt2_b; i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }

                    if (theName == "Var")
                    { // keep for backw. compat. (09/03/2016)
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;

                        if (nothing & 1)
                            _bulletIntParams[simi_bullet_body_bitcoded] |=
                                simi_bullet_body_usenondefaultcollisionmargin;
                        if (nothing & 2)
                            _bulletIntParams[simi_bullet_body_bitcoded] |= simi_bullet_body_sticky;
                        if (nothing & 4)
                            _bulletIntParams[simi_bullet_body_bitcoded] |= simi_bullet_body_autoshrinkconvex;
                        if (nothing & 8)
                            _bulletIntParams[simi_bullet_body_bitcoded] |=
                                simi_bullet_body_usenondefaultcollisionmarginconvex;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (!vortexDataLoaded)
            { // keep for backward compatibility (16/10/2013)
                // Try to guess the friction we would need for Vortex:
                double averageFriction = 0.0;
                if (_bulletFloatParams[simi_bullet_body_oldfriction] > 1.0)
                    averageFriction += 1.0;
                else
                    averageFriction += _bulletFloatParams[simi_bullet_body_oldfriction];
                if (_odeFloatParams[simi_ode_body_friction] > 1.0)
                    averageFriction += 1.0;
                else
                    averageFriction += _odeFloatParams[simi_ode_body_friction];
                averageFriction *= 0.5;
                if (averageFriction < 0.01)
                {
                    _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel] = sim_vortex_bodyfrictionmodel_none;
                    _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel] = sim_vortex_bodyfrictionmodel_none;
                }
                else
                {
                    _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = averageFriction;
                    _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = averageFriction;
                }
            }
            if (!newtonDataLoaded)
            { // keep for backward compatibility (5/8/2015)
                // Try to guess the friction we would need for Newton:
                double averageFriction = 0.0;
                if (_bulletFloatParams[simi_bullet_body_oldfriction] > 1.0)
                    averageFriction += 1.0;
                else
                    averageFriction += _bulletFloatParams[simi_bullet_body_oldfriction];
                if (_odeFloatParams[simi_ode_body_friction] > 1.0)
                    averageFriction += 1.0;
                else
                    averageFriction += _odeFloatParams[simi_ode_body_friction];
                averageFriction *= 0.5;
                _newtonFloatParams[simi_newton_body_staticfriction] = averageFriction;
                _newtonFloatParams[simi_newton_body_kineticfriction] = averageFriction;
            }
            _fixVortexInfVals();
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            if (exhaustiveXml)
            {
                ar.xmlAddNode_int("id", _objectID);
                ar.xmlAddNode_string("name", _objectName.c_str());
            }

            ar.xmlPushNewNode("engines");
            ar.xmlPushNewNode("bullet");
            ar.xmlAddNode_float("restitution", _bulletFloatParams[simi_bullet_body_restitution]);
            ar.xmlAddNode_float("friction", _bulletFloatParams[simi_bullet_body_friction]);
            ar.xmlAddNode_float("oldfriction", _bulletFloatParams[simi_bullet_body_oldfriction]);
            ar.xmlAddNode_float("lineardamping", _bulletFloatParams[simi_bullet_body_lineardamping]);
            ar.xmlAddNode_float("angulardamping", _bulletFloatParams[simi_bullet_body_angulardamping]);
            ar.xmlAddNode_float("nondefaultcollisionmargingfactor",
                                _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor]);
            ar.xmlAddNode_float("nondefaultcollisionmargingfactorconvex",
                                _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex]);

            ar.xmlAddNode_bool("sticky", getBoolPropertyValue(propMaterial_bulletSticky.name));
            ar.xmlAddNode_bool("usenondefaultcollisionmargin", getBoolPropertyValue(propMaterial_bulletNonDefaultCollisionMargin.name));
            ar.xmlAddNode_bool("usenondefaultcollisionmarginconvex", getBoolPropertyValue(propMaterial_bulletNonDefaultCollisionMarginConvex.name));
            ar.xmlAddNode_bool("autoshrinkconvex", getBoolPropertyValue(propMaterial_bulletAutoShrinkConvex.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_float("friction", _odeFloatParams[simi_ode_body_friction]);
            ar.xmlAddNode_float("softerp", _odeFloatParams[simi_ode_body_softerp]);
            ar.xmlAddNode_float("softcfm", _odeFloatParams[simi_ode_body_softcfm]);
            ar.xmlAddNode_float("lineardamping", _odeFloatParams[simi_ode_body_lineardamping]);
            ar.xmlAddNode_float("angulardamping", _odeFloatParams[simi_ode_body_angulardamping]);

            ar.xmlAddNode_int("maxcontacts", _odeIntParams[simi_ode_body_maxcontacts]);
            ar.xmlPopNode();

            ar.xmlPushNewNode("vortex");
            ar.xmlAddNode_float("primlinearaxisfriction",
                                _vortexFloatParams[simi_vortex_body_primlinearaxisfriction]);
            ar.xmlAddNode_float("seclinearaxisfriction",
                                _vortexFloatParams[simi_vortex_body_seclinearaxisfriction]);
            ar.xmlAddNode_float("primangularaxisfriction",
                                _vortexFloatParams[simi_vortex_body_primangularaxisfriction]);
            ar.xmlAddNode_float("secangularaxisfriction",
                                _vortexFloatParams[simi_vortex_body_secangularaxisfriction]);
            ar.xmlAddNode_float("normalangularaxisfriction",
                                _vortexFloatParams[simi_vortex_body_normalangularaxisfriction]);
            ar.xmlAddNode_float("primlinearaxisstaticfrictionscale",
                                _vortexFloatParams[simi_vortex_body_primlinearaxisstaticfrictionscale]);
            ar.xmlAddNode_float("seclinearaxisstaticfrictionscale",
                                _vortexFloatParams[simi_vortex_body_seclinearaxisstaticfrictionscale]);
            ar.xmlAddNode_float("primangularaxisstaticfrictionscale",
                                _vortexFloatParams[simi_vortex_body_primangularaxisstaticfrictionscale]);
            ar.xmlAddNode_float("secangularaxisstaticfrictionscale",
                                _vortexFloatParams[simi_vortex_body_secangularaxisstaticfrictionscale]);
            ar.xmlAddNode_float("normalangularaxisstaticfrictionscale",
                                _vortexFloatParams[simi_vortex_body_normalangularaxisstaticfrictionscale]);
            ar.xmlAddNode_float("compliance", _vortexFloatParams[simi_vortex_body_compliance]);
            ar.xmlAddNode_float("damping", _vortexFloatParams[simi_vortex_body_damping]);
            ar.xmlAddNode_float("restitution", _vortexFloatParams[simi_vortex_body_restitution]);
            ar.xmlAddNode_float("restitutionthreshold",
                                _vortexFloatParams[simi_vortex_body_restitutionthreshold]);
            ar.xmlAddNode_float("adhesiveforce", _vortexFloatParams[simi_vortex_body_adhesiveforce]);
            ar.xmlAddNode_float("linearvelocitydamping",
                                _vortexFloatParams[simi_vortex_body_linearvelocitydamping]);
            ar.xmlAddNode_float("angularvelocitydamping",
                                _vortexFloatParams[simi_vortex_body_angularvelocitydamping]);
            ar.xmlAddNode_float("primlinearaxisslide",
                                _vortexFloatParams[simi_vortex_body_primlinearaxisslide]);
            ar.xmlAddNode_float("seclinearaxisslide", _vortexFloatParams[simi_vortex_body_seclinearaxisslide]);
            ar.xmlAddNode_float("primangularaxisslide",
                                _vortexFloatParams[simi_vortex_body_primangularaxisslide]);
            ar.xmlAddNode_float("secangularaxisslide",
                                _vortexFloatParams[simi_vortex_body_secangularaxisslide]);
            ar.xmlAddNode_float("normalangularaxisslide",
                                _vortexFloatParams[simi_vortex_body_normalangularaxisslide]);
            ar.xmlAddNode_float("primlinearaxisslip", _vortexFloatParams[simi_vortex_body_primlinearaxisslip]);
            ar.xmlAddNode_float("seclinearaxisslip", _vortexFloatParams[simi_vortex_body_seclinearaxisslip]);
            ar.xmlAddNode_float("primangularaxisslip",
                                _vortexFloatParams[simi_vortex_body_primangularaxisslip]);
            ar.xmlAddNode_float("secangularaxisslip", _vortexFloatParams[simi_vortex_body_secangularaxisslip]);
            ar.xmlAddNode_float("normalangularaxisslip",
                                _vortexFloatParams[simi_vortex_body_normalangularaxisslip]);
            ar.xmlAddNode_float("autosleeplinearspeedthreshold",
                                _vortexFloatParams[simi_vortex_body_autosleeplinearspeedthreshold]);
            ar.xmlAddNode_float("autosleeplinearaccelthreshold",
                                _vortexFloatParams[simi_vortex_body_autosleeplinearaccelthreshold]);
            ar.xmlAddNode_float("autosleepangularspeedthreshold",
                                _vortexFloatParams[simi_vortex_body_autosleepangularspeedthreshold]);
            ar.xmlAddNode_float("autosleepangularaccelthreshold",
                                _vortexFloatParams[simi_vortex_body_autosleepangularaccelthreshold]);
            ar.xmlAddNode_float("skinthickness", _vortexFloatParams[simi_vortex_body_skinthickness]);
            ar.xmlAddNode_float("autoangulardampingtensionratio",
                                _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio]);
            ar.xmlAddNode_float("primaxisvectorx", _vortexFloatParams[simi_vortex_body_primaxisvectorx]);
            ar.xmlAddNode_float("primaxisvectory", _vortexFloatParams[simi_vortex_body_primaxisvectory]);
            ar.xmlAddNode_float("primaxisvectorz", _vortexFloatParams[simi_vortex_body_primaxisvectorz]);

            ar.xmlAddNode_int("primlinearaxisfrictionmodel",
                              _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel]);
            ar.xmlAddNode_int("seclinearaxisfrictionmodel",
                              _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel]);
            ar.xmlAddNode_int("primangulararaxisfrictionmodel",
                              _vortexIntParams[simi_vortex_body_primangulararaxisfrictionmodel]);
            ar.xmlAddNode_int("secmangulararaxisfrictionmodel",
                              _vortexIntParams[simi_vortex_body_secmangulararaxisfrictionmodel]);
            ar.xmlAddNode_int("normalmangulararaxisfrictionmodel",
                              _vortexIntParams[simi_vortex_body_normalmangulararaxisfrictionmodel]);
            ar.xmlAddNode_int("autosleepsteplivethreshold",
                              _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold]);
            ar.xmlAddNode_int("materialuniqueid", _vortexIntParams[simi_vortex_body_materialuniqueid]);

            ar.xmlAddNode_bool("pureshapesasconvex", getBoolPropertyValue(propMaterial_vortexPrimitiveShapesAsConvex.name));
            ar.xmlAddNode_bool("convexshapesasrandom", getBoolPropertyValue(propMaterial_vortexConvexShapesAsRandom.name));
            ar.xmlAddNode_bool("randomshapesasterrain", getBoolPropertyValue(propMaterial_vortexRandomShapesAsTerrain.name));
            ar.xmlAddNode_bool("fastmoving", getBoolPropertyValue(propMaterial_vortexFastMoving.name));
            ar.xmlAddNode_bool("autoslip", getBoolPropertyValue(propMaterial_vortexAutoSlip.name));
            ar.xmlAddNode_bool("seclinaxissameasprimlinaxis", getBoolPropertyValue(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name));
            ar.xmlAddNode_bool("secangaxissameasprimangaxis", getBoolPropertyValue(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name));
            ar.xmlAddNode_bool("normangaxissameasprimangaxis", getBoolPropertyValue(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name));
            ar.xmlAddNode_bool("autoangulardamping", getBoolPropertyValue(propMaterial_vortexAutoAngularDamping.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("newton");
            ar.xmlAddNode_float("staticfriction", _newtonFloatParams[simi_newton_body_staticfriction]);
            ar.xmlAddNode_float("kineticfriction", _newtonFloatParams[simi_newton_body_kineticfriction]);
            ar.xmlAddNode_float("restitution", _newtonFloatParams[simi_newton_body_restitution]);
            ar.xmlAddNode_float("lineardrag", _newtonFloatParams[simi_newton_body_lineardrag]);
            ar.xmlAddNode_float("angulardrag", _newtonFloatParams[simi_newton_body_angulardrag]);
            ar.xmlAddNode_bool("fastmoving", getBoolPropertyValue(propMaterial_newtonFastMoving.name));
            ar.xmlPopNode();

            ar.xmlPushNewNode("mujoco");
            ar.xmlAddNode_3float("friction", _mujocoFloatParams[simi_mujoco_body_friction1],
                                 _mujocoFloatParams[simi_mujoco_body_friction2],
                                 _mujocoFloatParams[simi_mujoco_body_friction3]);
            ar.xmlAddNode_2float("solref", _mujocoFloatParams[simi_mujoco_body_solref1],
                                 _mujocoFloatParams[simi_mujoco_body_solref2]);
            double si[5];
            si[0] = _mujocoFloatParams[simi_mujoco_body_solimp1];
            si[1] = _mujocoFloatParams[simi_mujoco_body_solimp2];
            si[2] = _mujocoFloatParams[simi_mujoco_body_solimp3];
            si[3] = _mujocoFloatParams[simi_mujoco_body_solimp4];
            si[4] = _mujocoFloatParams[simi_mujoco_body_solimp5];
            ar.xmlAddNode_floats("solimp", si, 5);
            ar.xmlAddNode_float("solmix", _mujocoFloatParams[simi_mujoco_body_solmix]);
            ar.xmlAddNode_float("margin", _mujocoFloatParams[simi_mujoco_body_margin]);
            ar.xmlAddNode_float("gap", _mujocoFloatParams[simi_mujoco_body_gap]);
            ar.xmlAddNode_float("gravcomp", _mujocoFloatParams[simi_mujoco_body_gravcomp]);
            ar.xmlAddNode_int("condim", _mujocoIntParams[simi_mujoco_body_condim]);
            ar.xmlAddNode_int("priority", _mujocoIntParams[simi_mujoco_body_priority]);
            ar.xmlAddNode_bool("adhesion", getBoolPropertyValue(propMaterial_mujocoAdhesion.name));
            ar.xmlAddNode_float("adhesiongain", _mujocoFloatParams[simi_mujoco_body_adhesiongain]);
            si[0] = _mujocoFloatParams[simi_mujoco_body_adhesionctrlrange1];
            si[1] = _mujocoFloatParams[simi_mujoco_body_adhesionctrlrange2];
            ar.xmlAddNode_floats("adhesionctrlrange", si, 2);
            si[0] = _mujocoFloatParams[simi_mujoco_body_adhesionforcerange1];
            si[1] = _mujocoFloatParams[simi_mujoco_body_adhesionforcerange2];
            ar.xmlAddNode_floats("adhesionforcerange", si, 2);
            ar.xmlAddNode_int("adhesionforcelimited", _mujocoIntParams[simi_mujoco_body_adhesionforcelimited]);
            ar.xmlPopNode();

            ar.xmlPopNode();
        }
        else
        {
            if (exhaustiveXml)
            {
                ar.xmlGetNode_int("id", _objectID);
                ar.xmlGetNode_string("name", _objectName);
            }

            double v;
            int vi;
            bool vb;
            if (ar.xmlPushChildNode("engines", exhaustiveXml))
            {
                if (ar.xmlPushChildNode("bullet", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("restitution", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_restitution] = v;
                    if (ar.xmlGetNode_float("friction", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_friction] = v;
                    if (ar.xmlGetNode_float("oldfriction", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_oldfriction] = v;
                    if (ar.xmlGetNode_float("lineardamping", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_lineardamping] = v;
                    if (ar.xmlGetNode_float("angulardamping", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_angulardamping] = v;
                    if (ar.xmlGetNode_float("nondefaultcollisionmargingfactor", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor] = v;
                    if (ar.xmlGetNode_float("nondefaultcollisionmargingfactorconvex", v, exhaustiveXml))
                        _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex] = v;

                    if (ar.xmlGetNode_bool("sticky", vb, exhaustiveXml))
                    {
                        _bulletIntParams[simi_bullet_body_bitcoded] |= simi_bullet_body_sticky;
                        if (!vb)
                            _bulletIntParams[simi_bullet_body_bitcoded] -= simi_bullet_body_sticky;
                    }
                    if (ar.xmlGetNode_bool("usenondefaultcollisionmargin", vb, exhaustiveXml))
                    {
                        _bulletIntParams[simi_bullet_body_bitcoded] |= simi_bullet_body_usenondefaultcollisionmargin;
                        if (!vb)
                            _bulletIntParams[simi_bullet_body_bitcoded] -= simi_bullet_body_usenondefaultcollisionmargin;
                    }
                    if (ar.xmlGetNode_bool("usenondefaultcollisionmarginconvex", vb, exhaustiveXml))
                    {
                        _bulletIntParams[simi_bullet_body_bitcoded] |= simi_bullet_body_usenondefaultcollisionmarginconvex;
                        if (!vb)
                            _bulletIntParams[simi_bullet_body_bitcoded] -= simi_bullet_body_usenondefaultcollisionmarginconvex;
                    }
                    if (ar.xmlGetNode_bool("autoshrinkconvex", vb, exhaustiveXml))
                    {
                        _bulletIntParams[simi_bullet_body_bitcoded] |= simi_bullet_body_autoshrinkconvex;
                        if (!vb)
                            _bulletIntParams[simi_bullet_body_bitcoded] -= simi_bullet_body_autoshrinkconvex;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("ode", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("friction", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_body_friction] = v;
                    if (ar.xmlGetNode_float("softerp", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_body_softerp] = v;
                    if (ar.xmlGetNode_float("softcfm", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_body_softcfm] = v;
                    if (ar.xmlGetNode_float("lineardamping", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_body_lineardamping] = v;
                    if (ar.xmlGetNode_float("angulardamping", v, exhaustiveXml))
                        _odeFloatParams[simi_ode_body_angulardamping] = v;

                    if (ar.xmlGetNode_int("maxcontacts", vi, exhaustiveXml))
                        _odeIntParams[simi_ode_body_maxcontacts] = vi;
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("vortex", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("primlinearaxisfriction", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = v;
                    if (ar.xmlGetNode_float("seclinearaxisfriction", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = v;
                    if (ar.xmlGetNode_float("primangularaxisfriction", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primangularaxisfriction] = v;
                    if (ar.xmlGetNode_float("secangularaxisfriction", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_secangularaxisfriction] = v;
                    if (ar.xmlGetNode_float("normalangularaxisfriction", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_normalangularaxisfriction] = v;
                    if (ar.xmlGetNode_float("primlinearaxisstaticfrictionscale", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primlinearaxisstaticfrictionscale] = v;
                    if (ar.xmlGetNode_float("seclinearaxisstaticfrictionscale", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_seclinearaxisstaticfrictionscale] = v;
                    if (ar.xmlGetNode_float("primangularaxisstaticfrictionscale", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primangularaxisstaticfrictionscale] = v;
                    if (ar.xmlGetNode_float("secangularaxisstaticfrictionscale", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_secangularaxisstaticfrictionscale] = v;
                    if (ar.xmlGetNode_float("normalangularaxisstaticfrictionscale", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_normalangularaxisstaticfrictionscale] = v;
                    if (ar.xmlGetNode_float("compliance", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_compliance] = v;
                    if (ar.xmlGetNode_float("damping", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_damping] = v;
                    if (ar.xmlGetNode_float("restitution", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_restitution] = v;
                    if (ar.xmlGetNode_float("restitutionthreshold", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_restitutionthreshold] = v;
                    if (ar.xmlGetNode_float("adhesiveforce", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_adhesiveforce] = v;
                    if (ar.xmlGetNode_float("linearvelocitydamping", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_linearvelocitydamping] = v;
                    if (ar.xmlGetNode_float("angularvelocitydamping", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_angularvelocitydamping] = v;
                    if (ar.xmlGetNode_float("primlinearaxisslide", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primlinearaxisslide] = v;
                    if (ar.xmlGetNode_float("seclinearaxisslide", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_seclinearaxisslide] = v;
                    if (ar.xmlGetNode_float("primangularaxisslide", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primangularaxisslide] = v;
                    if (ar.xmlGetNode_float("secangularaxisslide", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_secangularaxisslide] = v;
                    if (ar.xmlGetNode_float("normalangularaxisslide", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_normalangularaxisslide] = v;
                    if (ar.xmlGetNode_float("primlinearaxisslip", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primlinearaxisslip] = v;
                    if (ar.xmlGetNode_float("seclinearaxisslip", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_seclinearaxisslip] = v;
                    if (ar.xmlGetNode_float("primangularaxisslip", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primangularaxisslip] = v;
                    if (ar.xmlGetNode_float("secangularaxisslip", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_secangularaxisslip] = v;
                    if (ar.xmlGetNode_float("normalangularaxisslip", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_normalangularaxisslip] = v;
                    if (ar.xmlGetNode_float("autosleeplinearspeedthreshold", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_autosleeplinearspeedthreshold] = v;
                    if (ar.xmlGetNode_float("autosleeplinearaccelthreshold", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_autosleeplinearaccelthreshold] = v;
                    if (ar.xmlGetNode_float("autosleepangularspeedthreshold", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_autosleepangularspeedthreshold] = v;
                    if (ar.xmlGetNode_float("autosleepangularaccelthreshold", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_autosleepangularaccelthreshold] = v;
                    if (ar.xmlGetNode_float("skinthickness", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_skinthickness] = v;
                    if (ar.xmlGetNode_float("autoangulardampingtensionratio", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio] = v;
                    if (ar.xmlGetNode_float("primaxisvectorx", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primaxisvectorx] = v;
                    if (ar.xmlGetNode_float("primaxisvectory", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primaxisvectory] = v;
                    if (ar.xmlGetNode_float("primaxisvectorz", v, exhaustiveXml))
                        _vortexFloatParams[simi_vortex_body_primaxisvectorz] = v;

                    if (ar.xmlGetNode_int("primlinearaxisfrictionmodel", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel] = vi;
                    if (ar.xmlGetNode_int("seclinearaxisfrictionmodel", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel] = vi;
                    if (ar.xmlGetNode_int("primangulararaxisfrictionmodel", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_primangulararaxisfrictionmodel] = vi;
                    if (ar.xmlGetNode_int("secmangulararaxisfrictionmodel", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_secmangulararaxisfrictionmodel] = vi;
                    if (ar.xmlGetNode_int("normalmangulararaxisfrictionmodel", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_normalmangulararaxisfrictionmodel] = vi;
                    if (ar.xmlGetNode_int("autosleepsteplivethreshold", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold] = vi;
                    if (ar.xmlGetNode_int("materialuniqueid", vi, exhaustiveXml))
                        _vortexIntParams[simi_vortex_body_materialuniqueid] = vi;

                    if (ar.xmlGetNode_bool("pureshapesasconvex", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_pureshapesasconvex;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_pureshapesasconvex;
                    }
                    if (ar.xmlGetNode_bool("convexshapesasrandom", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_convexshapesasrandom;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_convexshapesasrandom;
                    }
                    if (ar.xmlGetNode_bool("randomshapesasterrain", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_randomshapesasterrain;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_randomshapesasterrain;
                    }
                    if (ar.xmlGetNode_bool("fastmoving", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_fastmoving;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_fastmoving;
                    }
                    if (ar.xmlGetNode_bool("autoslip", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_autoslip;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_autoslip;
                    }
                    if (ar.xmlGetNode_bool("seclinaxissameasprimlinaxis", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_seclinaxissameasprimlinaxis;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_seclinaxissameasprimlinaxis;
                    }
                    if (ar.xmlGetNode_bool("secangaxissameasprimangaxis", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_secangaxissameasprimangaxis;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_secangaxissameasprimangaxis;
                    }
                    if (ar.xmlGetNode_bool("normangaxissameasprimangaxis", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_normangaxissameasprimangaxis;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_normangaxissameasprimangaxis;
                    }
                    if (ar.xmlGetNode_bool("autoangulardamping", vb, exhaustiveXml))
                    {
                        _vortexIntParams[simi_vortex_body_bitcoded] |= simi_vortex_body_autoangulardamping;
                        if (!vb)
                            _vortexIntParams[simi_vortex_body_bitcoded] -= simi_vortex_body_autoangulardamping;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("newton", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("staticfriction", v, exhaustiveXml))
                        _newtonFloatParams[simi_newton_body_staticfriction] = v;
                    if (ar.xmlGetNode_float("kineticfriction", v, exhaustiveXml))
                        _newtonFloatParams[simi_newton_body_kineticfriction] = v;
                    if (ar.xmlGetNode_float("restitution", v, exhaustiveXml))
                        _newtonFloatParams[simi_newton_body_restitution] = v;
                    if (ar.xmlGetNode_float("lineardrag", v, exhaustiveXml))
                        _newtonFloatParams[simi_newton_body_lineardrag] = v;
                    if (ar.xmlGetNode_float("angulardrag", v, exhaustiveXml))
                        _newtonFloatParams[simi_newton_body_angulardrag] = v;

                    if (ar.xmlGetNode_bool("fastmoving", vb, exhaustiveXml))
                    {
                        _newtonIntParams[simi_newton_body_bitcoded] |= simi_newton_body_fastmoving;
                        if (!vb)
                            _newtonIntParams[simi_newton_body_bitcoded] -= simi_newton_body_fastmoving;
                    }
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("mujoco", exhaustiveXml))
                {
                    double vv[5];
                    if (ar.xmlGetNode_floats("friction", vv, 3, exhaustiveXml))
                    {
                        _mujocoFloatParams[simi_mujoco_body_friction1] = vv[0];
                        _mujocoFloatParams[simi_mujoco_body_friction2] = vv[1];
                        _mujocoFloatParams[simi_mujoco_body_friction3] = vv[2];
                    }
                    if (ar.xmlGetNode_floats("solref", vv, 2, exhaustiveXml))
                    {
                        _mujocoFloatParams[simi_mujoco_body_solref1] = vv[0];
                        _mujocoFloatParams[simi_mujoco_body_solref2] = vv[1];
                    }
                    if (ar.xmlGetNode_floats("solimp", vv, 5, exhaustiveXml))
                    {
                        _mujocoFloatParams[simi_mujoco_body_solimp1] = vv[0];
                        _mujocoFloatParams[simi_mujoco_body_solimp2] = vv[1];
                        _mujocoFloatParams[simi_mujoco_body_solimp3] = vv[2];
                        _mujocoFloatParams[simi_mujoco_body_solimp4] = vv[3];
                        _mujocoFloatParams[simi_mujoco_body_solimp5] = vv[4];
                    }
                    if (ar.xmlGetNode_float("solmix", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_body_solmix] = v;
                    if (ar.xmlGetNode_float("margin", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_body_margin] = v;
                    if (ar.xmlGetNode_float("gap", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_body_gap] = v;
                    if (ar.xmlGetNode_float("gravcomp", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_body_gravcomp] = v;
                    if (ar.xmlGetNode_int("condim", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_body_condim] = vi;
                    if (ar.xmlGetNode_int("priority", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_body_priority] = vi;
                    if (ar.xmlGetNode_bool("adhesion", vb, exhaustiveXml))
                    {
                        _mujocoIntParams[simi_mujoco_body_bitcoded] |= simi_mujoco_body_adhesion;
                        if (!vb)
                            _mujocoIntParams[simi_mujoco_body_bitcoded] -= simi_mujoco_body_adhesion;
                    }
                    if (ar.xmlGetNode_float("adhesiongain", v, exhaustiveXml))
                        _mujocoFloatParams[simi_mujoco_body_adhesiongain] = v;
                    if (ar.xmlGetNode_floats("adhesionctrlrange", vv, 2, exhaustiveXml))
                    {
                        _mujocoFloatParams[simi_mujoco_body_adhesionctrlrange1] = vv[0];
                        _mujocoFloatParams[simi_mujoco_body_adhesionctrlrange2] = vv[1];
                    }
                    if (ar.xmlGetNode_floats("adhesionforcerange", vv, 2, exhaustiveXml))
                    {
                        _mujocoFloatParams[simi_mujoco_body_adhesionforcerange1] = vv[0];
                        _mujocoFloatParams[simi_mujoco_body_adhesionforcerange2] = vv[1];
                    }
                    if (ar.xmlGetNode_int("adhesionforcelimited", vi, exhaustiveXml))
                        _mujocoIntParams[simi_mujoco_body_adhesionforcelimited] = vi;

                    ar.xmlPopNode();
                }

                ar.xmlPopNode();
            }
            _fixVortexInfVals();
        }
    }
}

void CDynMaterialObject::_fixVortexInfVals()
{ // to fix a past complication (i.e. neg. val. of unsigned would be inf)
    for (size_t i = 0; i < 33; i++)
    {
        if (_vortexFloatParams[i] < 0.0)
            _vortexFloatParams[i] = DBL_MAX;
    }
    // values at index 33 and later are signed
}

std::string CDynMaterialObject::_enumToProperty(int oldEnum, int type, int& indexWithArrays) const
{
    std::string retVal;
    for (size_t i = 0; i < allProps_material.size(); i++)
    {
        for (size_t j = 0; j < 5; j++)
        {
            int en = allProps_material[i].oldEnums[j];
            if (en == -1)
                break;
            else if (en == oldEnum)
            {
                if (type == allProps_material[i].type)
                {
                    if ((j > 0) || (allProps_material[i].oldEnums[j + 1] != -1))
                        indexWithArrays = int(j);
                    else
                        indexWithArrays = -1;
                    retVal = allProps_material[i].name;
                }
                break;
            }
        }
        if (retVal.size() > 0)
            break;
    }
    return retVal;
}

int CDynMaterialObject::setBoolProperty(const char* pName, bool pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndexBitCoded, int simiIndex) {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            int nv = (arr[simiIndexBitCoded] | simiIndex) - (1 - pState) * simiIndex;
            if ((nv != arr[simiIndexBitCoded]) || (pName == nullptr))
            {
                if (pName != nullptr)
                    arr[simiIndexBitCoded] = nv;
                if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propertyName.c_str(), true);
                    ev->appendKeyBool(propertyName.c_str(), arr[simiIndexBitCoded] & simiIndex);
                    if (pName != nullptr)
                        sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propMaterial_bulletSticky.name, _bulletIntParams, simi_bullet_body_bitcoded, simi_bullet_body_sticky);
    handleProp(propMaterial_bulletNonDefaultCollisionMargin.name, _bulletIntParams, simi_bullet_body_bitcoded, simi_bullet_body_usenondefaultcollisionmargin);
    handleProp(propMaterial_bulletNonDefaultCollisionMarginConvex.name, _bulletIntParams, simi_bullet_body_bitcoded, simi_bullet_body_usenondefaultcollisionmarginconvex);
    handleProp(propMaterial_bulletAutoShrinkConvex.name, _bulletIntParams, simi_bullet_body_bitcoded, simi_bullet_body_autoshrinkconvex);
    handleProp(propMaterial_vortexPrimitiveShapesAsConvex.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_pureshapesasconvex);
    handleProp(propMaterial_vortexConvexShapesAsRandom.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_convexshapesasrandom);
    handleProp(propMaterial_vortexRandomShapesAsTerrain.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_randomshapesasterrain);
    handleProp(propMaterial_vortexFastMoving.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_fastmoving);
    handleProp(propMaterial_vortexAutoSlip.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_autoslip);
    handleProp(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_seclinaxissameasprimlinaxis);
    handleProp(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_secangaxissameasprimangaxis);
    handleProp(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_normangaxissameasprimangaxis);
    handleProp(propMaterial_vortexAutoAngularDamping.name, _vortexIntParams, simi_vortex_body_bitcoded, simi_vortex_body_autoangulardamping);
    handleProp(propMaterial_newtonFastMoving.name, _newtonIntParams, simi_newton_body_bitcoded, simi_newton_body_fastmoving);
    handleProp(propMaterial_mujocoAdhesion.name, _mujocoIntParams, simi_mujoco_body_bitcoded, simi_mujoco_body_adhesion);

    if ((ev != nullptr) && (eev == nullptr))
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getBoolProperty(const char* pName, bool& pState) const
{
    int retVal = -1;

    if (strcmp(pName, propMaterial_bulletSticky.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_sticky;
    }
    else if (strcmp(pName, propMaterial_bulletNonDefaultCollisionMargin.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_usenondefaultcollisionmargin;
    }
    else if (strcmp(pName, propMaterial_bulletNonDefaultCollisionMarginConvex.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_usenondefaultcollisionmarginconvex;
    }
    else if (strcmp(pName, propMaterial_bulletAutoShrinkConvex.name) == 0)
    {
        retVal = 1;
        pState = _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_autoshrinkconvex;
    }
    else if (strcmp(pName, propMaterial_vortexPrimitiveShapesAsConvex.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_pureshapesasconvex;
    }
    else if (strcmp(pName, propMaterial_vortexConvexShapesAsRandom.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_convexshapesasrandom;
    }
    else if (strcmp(pName, propMaterial_vortexRandomShapesAsTerrain.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_randomshapesasterrain;
    }
    else if (strcmp(pName, propMaterial_vortexFastMoving.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_fastmoving;
    }
    else if (strcmp(pName, propMaterial_vortexAutoSlip.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_autoslip;
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_seclinaxissameasprimlinaxis;
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_secangaxissameasprimangaxis;
    }
    else if (strcmp(pName, propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_normangaxissameasprimangaxis;
    }
    else if (strcmp(pName, propMaterial_vortexAutoAngularDamping.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_autoangulardamping;
    }
    else if (strcmp(pName, propMaterial_newtonFastMoving.name) == 0)
    {
        retVal = 1;
        pState = _newtonIntParams[simi_newton_body_bitcoded] & simi_newton_body_fastmoving;
    }
    else if (strcmp(pName, propMaterial_mujocoAdhesion.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_body_bitcoded] & simi_mujoco_body_adhesion;
    }

    return retVal;
}

int CDynMaterialObject::setIntProperty(const char* pName, int pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    auto handleProp = [&](const std::string& propertyName, std::vector<int>& arr, int simiIndex) {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            if ((pState != arr[simiIndex]) || (pName == nullptr))
            {
                if (pName != nullptr)
                    arr[simiIndex] = pState;
                if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propertyName.c_str(), true);
                    ev->appendKeyInt(propertyName.c_str(), arr[simiIndex]);
                    if (pName != nullptr)
                        sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propMaterial_odeMaxContacts.name, _odeIntParams, simi_ode_body_maxcontacts);
    handleProp(propMaterial_vortexPrimaryLinearAxisFrictionModel.name, _vortexIntParams, simi_vortex_body_primlinearaxisfrictionmodel);
    handleProp(propMaterial_vortexSecondaryLinearAxisFrictionModel.name, _vortexIntParams, simi_vortex_body_seclinearaxisfrictionmodel);
    handleProp(propMaterial_vortexPrimaryAngularAxisFrictionModel.name, _vortexIntParams, simi_vortex_body_primangulararaxisfrictionmodel);
    handleProp(propMaterial_vortexSecondaryAngularAxisFrictionModel.name, _vortexIntParams, simi_vortex_body_secmangulararaxisfrictionmodel);
    handleProp(propMaterial_vortexNormalAngularAxisFrictionModel.name, _vortexIntParams, simi_vortex_body_normalmangulararaxisfrictionmodel);
    handleProp(propMaterial_vortexAutoSleepStepLiveThreshold.name, _vortexIntParams, simi_vortex_body_autosleepsteplivethreshold);
    handleProp(propMaterial_vortexMaterialUniqueId.name, _vortexIntParams, simi_vortex_body_materialuniqueid);
    handleProp(propMaterial_mujocoCondim.name, _mujocoIntParams, simi_mujoco_body_condim);
    handleProp(propMaterial_mujocoPriority.name, _mujocoIntParams, simi_mujoco_body_priority);
    handleProp(propMaterial_mujocoAdhesionForcelimited.name, _mujocoIntParams, simi_mujoco_body_adhesionforcelimited);

    if ((ev != nullptr) && (eev == nullptr))
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getIntProperty(const char* pName, int& pState) const
{
    int retVal = -1;

    if (strcmp(pName, propMaterial_odeMaxContacts.name) == 0)
    {
        retVal = 1;
        pState = _odeIntParams[simi_ode_body_maxcontacts];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryLinearAxisFrictionModel.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryLinearAxisFrictionModel.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryAngularAxisFrictionModel.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_primangulararaxisfrictionmodel];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryAngularAxisFrictionModel.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_secmangulararaxisfrictionmodel];
    }
    else if (strcmp(pName, propMaterial_vortexNormalAngularAxisFrictionModel.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_normalmangulararaxisfrictionmodel];
    }
    else if (strcmp(pName, propMaterial_vortexAutoSleepStepLiveThreshold.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold];
    }
    else if (strcmp(pName, propMaterial_vortexMaterialUniqueId.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_vortex_body_materialuniqueid];
    }
    else if (strcmp(pName, propMaterial_mujocoCondim.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_body_condim];
    }
    else if (strcmp(pName, propMaterial_mujocoPriority.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_body_priority];
    }
    else if (strcmp(pName, propMaterial_mujocoAdhesionForcelimited.name) == 0)
    {
        retVal = 1;
        pState = _mujocoIntParams[simi_mujoco_body_adhesionforcelimited];
    }

    return retVal;
}

int CDynMaterialObject::setFloatProperty(const char* pName, double pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex) {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            if ((pState != arr[simiIndex]) || (pName == nullptr))
            {
                if (pName != nullptr)
                    arr[simiIndex] = pState;
                if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propertyName.c_str(), true);
                    ev->appendKeyDouble(propertyName.c_str(), arr[simiIndex]);
                    if (pName != nullptr)
                        sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propMaterial_bulletRestitution.name, _bulletFloatParams, simi_bullet_body_restitution);
    handleProp(propMaterial_bulletFriction0.name, _bulletFloatParams, simi_bullet_body_oldfriction);
    handleProp(propMaterial_bulletFriction.name, _bulletFloatParams, simi_bullet_body_friction);
    handleProp(propMaterial_bulletLinearDamping.name, _bulletFloatParams, simi_bullet_body_lineardamping);
    handleProp(propMaterial_bulletAngularDamping.name, _bulletFloatParams, simi_bullet_body_angulardamping);
    handleProp(propMaterial_bulletNonDefaultCollisionMarginFactor.name, _bulletFloatParams, simi_bullet_body_nondefaultcollisionmargingfactor);
    handleProp(propMaterial_bulletNonDefaultCollisionMarginFactorConvex.name, _bulletFloatParams, simi_bullet_body_nondefaultcollisionmargingfactorconvex);
    handleProp(propMaterial_odeFriction.name, _odeFloatParams, simi_ode_body_friction);
    handleProp(propMaterial_odeSoftErp.name, _odeFloatParams, simi_ode_body_softerp);
    handleProp(propMaterial_odeSoftCfm.name, _odeFloatParams, simi_ode_body_softcfm);
    handleProp(propMaterial_odeLinearDamping.name, _odeFloatParams, simi_ode_body_lineardamping);
    handleProp(propMaterial_odeAngularDamping.name, _odeFloatParams, simi_ode_body_angulardamping);
    handleProp(propMaterial_vortexPrimaryLinearAxisFriction.name, _vortexFloatParams, simi_vortex_body_primlinearaxisfriction);
    handleProp(propMaterial_vortexSecondaryLinearAxisFriction.name, _vortexFloatParams, simi_vortex_body_seclinearaxisfriction);
    handleProp(propMaterial_vortexPrimaryAngularAxisFriction.name, _vortexFloatParams, simi_vortex_body_primangularaxisfriction);
    handleProp(propMaterial_vortexSecondaryAngularAxisFriction.name, _vortexFloatParams, simi_vortex_body_secangularaxisfriction);
    handleProp(propMaterial_vortexNormalAngularAxisFriction.name, _vortexFloatParams, simi_vortex_body_normalangularaxisfriction);
    handleProp(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale.name, _vortexFloatParams, simi_vortex_body_primlinearaxisstaticfrictionscale);
    handleProp(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale.name, _vortexFloatParams, simi_vortex_body_seclinearaxisstaticfrictionscale);
    handleProp(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale.name, _vortexFloatParams, simi_vortex_body_primangularaxisstaticfrictionscale);
    handleProp(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale.name, _vortexFloatParams, simi_vortex_body_secangularaxisstaticfrictionscale);
    handleProp(propMaterial_vortexNormalAngularAxisStaticFrictionScale.name, _vortexFloatParams, simi_vortex_body_normalangularaxisstaticfrictionscale);
    handleProp(propMaterial_vortexCompliance.name, _vortexFloatParams, simi_vortex_body_compliance);
    handleProp(propMaterial_vortexDamping.name, _vortexFloatParams, simi_vortex_body_damping);
    handleProp(propMaterial_vortexRestitution.name, _vortexFloatParams, simi_vortex_body_restitution);
    handleProp(propMaterial_vortexRestitutionThreshold.name, _vortexFloatParams, simi_vortex_body_restitutionthreshold);
    handleProp(propMaterial_vortexAdhesiveForce.name, _vortexFloatParams, simi_vortex_body_adhesiveforce);
    handleProp(propMaterial_vortexLinearVelocityDamping.name, _vortexFloatParams, simi_vortex_body_linearvelocitydamping);
    handleProp(propMaterial_vortexAngularVelocityDamping.name, _vortexFloatParams, simi_vortex_body_angularvelocitydamping);
    handleProp(propMaterial_vortexPrimaryLinearAxisSlide.name, _vortexFloatParams, simi_vortex_body_primlinearaxisslide);
    handleProp(propMaterial_vortexSecondaryLinearAxisSlide.name, _vortexFloatParams, simi_vortex_body_seclinearaxisslide);
    handleProp(propMaterial_vortexPrimaryAngularAxisSlide.name, _vortexFloatParams, simi_vortex_body_primangularaxisslide);
    handleProp(propMaterial_vortexSecondaryAngularAxisSlide.name, _vortexFloatParams, simi_vortex_body_secangularaxisslide);
    handleProp(propMaterial_vortexNormalAngularAxisSlide.name, _vortexFloatParams, simi_vortex_body_normalangularaxisslide);
    handleProp(propMaterial_vortexPrimaryLinearAxisSlip.name, _vortexFloatParams, simi_vortex_body_primlinearaxisslip);
    handleProp(propMaterial_vortexSecondaryLinearAxisSlip.name, _vortexFloatParams, simi_vortex_body_seclinearaxisslip);
    handleProp(propMaterial_vortexPrimaryAngularAxisSlip.name, _vortexFloatParams, simi_vortex_body_primangularaxisslip);
    handleProp(propMaterial_vortexSecondaryAngularAxisSlip.name, _vortexFloatParams, simi_vortex_body_secangularaxisslip);
    handleProp(propMaterial_vortexNormalAngularAxisSlip.name, _vortexFloatParams, simi_vortex_body_normalangularaxisslip);
    handleProp(propMaterial_vortexAutoSleepLinearSpeedThreshold.name, _vortexFloatParams, simi_vortex_body_autosleeplinearspeedthreshold);
    handleProp(propMaterial_vortexAutoSleepLinearAccelerationThreshold.name, _vortexFloatParams, simi_vortex_body_autosleeplinearaccelthreshold);
    handleProp(propMaterial_vortexAutoSleepAngularSpeedThreshold.name, _vortexFloatParams, simi_vortex_body_autosleepangularspeedthreshold);
    handleProp(propMaterial_vortexAutoSleepAngularAccelerationThreshold.name, _vortexFloatParams, simi_vortex_body_autosleepangularaccelthreshold);
    handleProp(propMaterial_vortexSkinThickness.name, _vortexFloatParams, simi_vortex_body_skinthickness);
    handleProp(propMaterial_vortexAutoAngularDampingTensionRatio.name, _vortexFloatParams, simi_vortex_body_autoangulardampingtensionratio);
    handleProp(propMaterial_newtonStaticFriction.name, _newtonFloatParams, simi_newton_body_staticfriction);
    handleProp(propMaterial_newtonKineticFriction.name, _newtonFloatParams, simi_newton_body_kineticfriction);
    handleProp(propMaterial_newtonRestitution.name, _newtonFloatParams, simi_newton_body_restitution);
    handleProp(propMaterial_newtonLinearDrag.name, _newtonFloatParams, simi_newton_body_lineardrag);
    handleProp(propMaterial_newtonAngularDrag.name, _newtonFloatParams, simi_newton_body_angulardrag);
    handleProp(propMaterial_mujocoSolmix.name, _mujocoFloatParams, simi_mujoco_body_solmix);
    handleProp(propMaterial_mujocoMargin.name, _mujocoFloatParams, simi_mujoco_body_margin);
    handleProp(propMaterial_mujocoGap.name, _mujocoFloatParams, simi_mujoco_body_gap);
    handleProp(propMaterial_mujocoAdhesionGain.name, _mujocoFloatParams, simi_mujoco_body_adhesiongain);
    handleProp(propMaterial_mujocoAdhesionCtrl.name, _mujocoFloatParams, simi_mujoco_body_adhesionctrl);
    handleProp(propMaterial_mujocoGravcomp.name, _mujocoFloatParams, simi_mujoco_body_gravcomp);

    if ((ev != nullptr) && (eev == nullptr))
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getFloatProperty(const char* pName, double& pState) const
{
    int retVal = -1;

    if (strcmp(pName, propMaterial_bulletRestitution.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_restitution];
    }
    else if (strcmp(pName, propMaterial_bulletFriction0.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_oldfriction];
    }
    else if (strcmp(pName, propMaterial_bulletFriction.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_friction];
    }
    else if (strcmp(pName, propMaterial_bulletLinearDamping.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_lineardamping];
    }
    else if (strcmp(pName, propMaterial_bulletAngularDamping.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_angulardamping];
    }
    else if (strcmp(pName, propMaterial_bulletNonDefaultCollisionMarginFactor.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor];
    }
    else if (strcmp(pName, propMaterial_bulletNonDefaultCollisionMarginFactorConvex.name) == 0)
    {
        retVal = 1;
        pState = _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex];
    }
    else if (strcmp(pName, propMaterial_odeFriction.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_body_friction];
    }
    else if (strcmp(pName, propMaterial_odeSoftErp.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_body_softerp];
    }
    else if (strcmp(pName, propMaterial_odeSoftCfm.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_body_softcfm];
    }
    else if (strcmp(pName, propMaterial_odeLinearDamping.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_body_lineardamping];
    }
    else if (strcmp(pName, propMaterial_odeAngularDamping.name) == 0)
    {
        retVal = 1;
        pState = _odeFloatParams[simi_ode_body_angulardamping];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryLinearAxisFriction.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primlinearaxisfriction];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryLinearAxisFriction.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_seclinearaxisfriction];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryAngularAxisFriction.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primangularaxisfriction];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryAngularAxisFriction.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_secangularaxisfriction];
    }
    else if (strcmp(pName, propMaterial_vortexNormalAngularAxisFriction.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_normalangularaxisfriction];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryLinearAxisStaticFrictionScale.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primlinearaxisstaticfrictionscale];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryLinearAxisStaticFrictionScale.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_seclinearaxisstaticfrictionscale];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryAngularAxisStaticFrictionScale.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primangularaxisstaticfrictionscale];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryAngularAxisStaticFrictionScale.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_secangularaxisstaticfrictionscale];
    }
    else if (strcmp(pName, propMaterial_vortexNormalAngularAxisStaticFrictionScale.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_normalangularaxisstaticfrictionscale];
    }
    else if (strcmp(pName, propMaterial_vortexCompliance.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_compliance];
    }
    else if (strcmp(pName, propMaterial_vortexDamping.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_damping];
    }
    else if (strcmp(pName, propMaterial_vortexRestitution.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_restitution];
    }
    else if (strcmp(pName, propMaterial_vortexRestitutionThreshold.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_restitutionthreshold];
    }
    else if (strcmp(pName, propMaterial_vortexAdhesiveForce.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_adhesiveforce];
    }
    else if (strcmp(pName, propMaterial_vortexLinearVelocityDamping.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_linearvelocitydamping];
    }
    else if (strcmp(pName, propMaterial_vortexAngularVelocityDamping.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_angularvelocitydamping];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryLinearAxisSlide.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primlinearaxisslide];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryLinearAxisSlide.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_seclinearaxisslide];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryAngularAxisSlide.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primangularaxisslide];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryAngularAxisSlide.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_secangularaxisslide];
    }
    else if (strcmp(pName, propMaterial_vortexNormalAngularAxisSlide.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_normalangularaxisslide];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryLinearAxisSlip.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primlinearaxisslip];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryLinearAxisSlip.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_seclinearaxisslip];
    }
    else if (strcmp(pName, propMaterial_vortexPrimaryAngularAxisSlip.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_primangularaxisslip];
    }
    else if (strcmp(pName, propMaterial_vortexSecondaryAngularAxisSlip.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_secangularaxisslip];
    }
    else if (strcmp(pName, propMaterial_vortexNormalAngularAxisSlip.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_normalangularaxisslip];
    }
    else if (strcmp(pName, propMaterial_vortexAutoSleepLinearSpeedThreshold.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_autosleeplinearspeedthreshold];
    }
    else if (strcmp(pName, propMaterial_vortexAutoSleepLinearAccelerationThreshold.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_autosleeplinearaccelthreshold];
    }
    else if (strcmp(pName, propMaterial_vortexAutoSleepAngularSpeedThreshold.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_autosleepangularspeedthreshold];
    }
    else if (strcmp(pName, propMaterial_vortexAutoSleepAngularAccelerationThreshold.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_autosleepangularaccelthreshold];
    }
    else if (strcmp(pName, propMaterial_vortexSkinThickness.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_skinthickness];
    }
    else if (strcmp(pName, propMaterial_vortexAutoAngularDampingTensionRatio.name) == 0)
    {
        retVal = 1;
        pState = _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio];
    }
    else if (strcmp(pName, propMaterial_newtonStaticFriction.name) == 0)
    {
        retVal = 1;
        pState = _newtonFloatParams[simi_newton_body_staticfriction];
    }
    else if (strcmp(pName, propMaterial_newtonKineticFriction.name) == 0)
    {
        retVal = 1;
        pState = _newtonFloatParams[simi_newton_body_kineticfriction];
    }
    else if (strcmp(pName, propMaterial_newtonRestitution.name) == 0)
    {
        retVal = 1;
        pState = _newtonFloatParams[simi_newton_body_restitution];
    }
    else if (strcmp(pName, propMaterial_newtonLinearDrag.name) == 0)
    {
        retVal = 1;
        pState = _newtonFloatParams[simi_newton_body_lineardrag];
    }
    else if (strcmp(pName, propMaterial_newtonAngularDrag.name) == 0)
    {
        retVal = 1;
        pState = _newtonFloatParams[simi_newton_body_angulardrag];
    }
    else if (strcmp(pName, propMaterial_mujocoSolmix.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_body_solmix];
    }
    else if (strcmp(pName, propMaterial_mujocoMargin.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_body_margin];
    }
    else if (strcmp(pName, propMaterial_mujocoGap.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_body_gap];
    }
    else if (strcmp(pName, propMaterial_mujocoAdhesionGain.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_body_adhesiongain];
    }
    else if (strcmp(pName, propMaterial_mujocoAdhesionCtrl.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_body_adhesionctrl];
    }
    else if (strcmp(pName, propMaterial_mujocoGravcomp.name) == 0)
    {
        retVal = 1;
        pState = _mujocoFloatParams[simi_mujoco_body_gravcomp];
    }

    return retVal;
}

void CDynMaterialObject::sendEngineString(CCbor* eev /*= nullptr*/)
{
    if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
    {
        CCbor* ev = nullptr;
        if (eev != nullptr)
            ev = eev;
        CEngineProperties prop;
        std::string current(prop.getObjectProperties(_shapeHandleForEvents));
        if (ev == nullptr)
            ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propMaterial_engineProperties.name, true);
        ev->appendKeyText(propMaterial_engineProperties.name, current.c_str());
        if ((ev != nullptr) && (eev == nullptr))
            App::worldContainer->pushEvent();
    }
}

int CDynMaterialObject::setStringProperty(const char* pName, const char* pState)
{
    int retVal = -1;
    if (strcmp(pName, propMaterial_engineProperties.name) == 0)
    {
        retVal = 0;
        CEngineProperties prop;
        std::string current(prop.getObjectProperties(_shapeHandleForEvents));
        if (prop.setObjectProperties(_shapeHandleForEvents, pState))
        {
            retVal = 1;
            std::string current2(prop.getObjectProperties(_shapeHandleForEvents));
            if (current != current2)
                sendEngineString();
        }
    }
    return retVal;
}

int CDynMaterialObject::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = -1;
    if (strcmp(pName, propMaterial_engineProperties.name) == 0)
    {
        retVal = 1;
        CEngineProperties prop;
        pState = prop.getObjectProperties(_shapeHandleForEvents);
    }
    return retVal;
}

int CDynMaterialObject::setVector2Property(const char* pName, const double* pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1) {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            bool pa = false;
            if (pState != nullptr)
            {
                for (size_t i = 0; i < 2; i++)
                    pa = pa || (arr[simiIndex1 + i] != pState[i]);
            }
            if ((pName == nullptr) || pa)
            {
                if (pName != nullptr)
                {
                    for (size_t i = 0; i < 2; i++)
                        arr[simiIndex1 + i] = pState[i];
                }
                if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propertyName.c_str(), true);
                    ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 2);
                    if (pName != nullptr)
                        sendEngineString(ev);
                }
            }
        }
    };

    if ((ev != nullptr) && (eev == nullptr))
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getVector2Property(const char* pName, double* pState) const
{
    int retVal = -1;

    return retVal;
}

int CDynMaterialObject::setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1) {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            bool pa = false;
            if (pState != nullptr)
            {
                for (size_t i = 0; i < 3; i++)
                    pa = pa || (arr[simiIndex1 + i] != pState->data[i]);
            }
            if ((pName == nullptr) || pa)
            {
                if (pName != nullptr)
                {
                    for (size_t i = 0; i < 3; i++)
                        arr[simiIndex1 + i] = pState->data[i];
                }
                if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propertyName.c_str(), true);
                    ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, 3);
                    if (pName != nullptr)
                        sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propMaterial_vortexPrimaryAxisVector.name, _vortexFloatParams, simi_vortex_body_primaxisvectorx);

    if ((ev != nullptr) && (eev == nullptr))
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getVector3Property(const char* pName, C3Vector* pState) const
{
    int retVal = -1;

    if (strcmp(pName, propMaterial_vortexPrimaryAxisVector.name) == 0)
    {
        retVal = 1;
        pState->setData(_vortexFloatParams.data() + simi_vortex_body_primaxisvectorx);
    }

    return retVal;
}

int CDynMaterialObject::setFloatArrayProperty(const char* pName, const double* v, int vL, CCbor* eev /* = nullptr*/)
{
    int retVal = -1;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    auto handleProp = [&](const std::string& propertyName, std::vector<double>& arr, int simiIndex1, size_t n) {
        if ((pName == nullptr) || (propertyName == pName))
        {
            retVal = 1;
            bool pa = false;
            for (size_t i = 0; i < n; i++)
                pa = pa || ((vL > i) && (arr[simiIndex1 + i] != v[i]));
            if ((pName == nullptr) || pa)
            {
                if (pName != nullptr)
                {
                    for (size_t i = 0; i < n; i++)
                    {
                        if (vL > i)
                            arr[simiIndex1 + i] = v[i];
                    }
                }
                if ((_shapeHandleForEvents != -1) && App::worldContainer->getEventsEnabled())
                {
                    if (ev == nullptr)
                        ev = App::worldContainer->createSceneObjectChangedEvent(_shapeHandleForEvents, false, propertyName.c_str(), true);
                    ev->appendKeyDoubleArray(propertyName.c_str(), arr.data() + simiIndex1, n);
                    if (pName != nullptr)
                        sendEngineString(ev);
                }
            }
        }
    };

    handleProp(propMaterial_mujocoFriction.name, _mujocoFloatParams, simi_mujoco_body_friction1, 3);
    handleProp(propMaterial_mujocoSolref.name, _mujocoFloatParams, simi_mujoco_body_solref1, 2);
    handleProp(propMaterial_mujocoSolimp.name, _mujocoFloatParams, simi_mujoco_body_solimp1, 5);
    handleProp(propMaterial_mujocoAdhesionCtrlrange.name, _mujocoFloatParams, simi_mujoco_body_adhesionctrlrange1, 2);
    handleProp(propMaterial_mujocoAdhesionForcerange.name, _mujocoFloatParams, simi_mujoco_body_adhesionforcerange1, 2);

    if ((ev != nullptr) && (eev == nullptr))
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    int retVal = -1;
    pState.clear();

    auto handleProp = [&](const std::vector<double>& arr, int simiIndex1, size_t n) {
        retVal = 1;
        for (size_t i = 0; i < n; i++)
            pState.push_back(arr[simiIndex1 + i]);
    };

    if (strcmp(pName, propMaterial_mujocoSolref.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_body_solref1, 2);
    else if (strcmp(pName, propMaterial_mujocoSolimp.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_body_solimp1, 5);
    else if (strcmp(pName, propMaterial_mujocoFriction.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_body_friction1, 3);
    else if (strcmp(pName, propMaterial_mujocoAdhesionCtrlrange.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_body_adhesionctrlrange1, 2);
    else if (strcmp(pName, propMaterial_mujocoAdhesionForcerange.name) == 0)
        handleProp(_mujocoFloatParams, simi_mujoco_body_adhesionforcerange1, 2);

    return retVal;
}

int CDynMaterialObject::getPropertyName(int& index, std::string& pName) const
{
    return getPropertyName_static(index, pName);
}

int CDynMaterialObject::getPropertyName_static(int& index, std::string& pName)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_material.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_material[i].name, pName.c_str()))
        {
            index--;
            if (index == -1)
            {
                pName = allProps_material[i].name;
                retVal = 1;
                break;
            }
        }
    }
    return retVal;
}

int CDynMaterialObject::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    return getPropertyInfo_static(pName, info, infoTxt);
}

int CDynMaterialObject::getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_material.size(); i++)
    {
        if (strcmp(allProps_material[i].name, pName) == 0)
        {
            retVal = allProps_material[i].type;
            info = allProps_material[i].flags;
            if ((infoTxt == "") && (strcmp(allProps_material[i].infoTxt, "") != 0))
                infoTxt = allProps_material[i].infoTxt;
            else
                infoTxt = allProps_material[i].shortInfoTxt;
            break;
        }
    }
    return retVal;
}

// Some helpers:
bool CDynMaterialObject::getBoolPropertyValue(const char* pName) const
{
    bool retVal = false;
    getBoolProperty(pName, retVal);
    return retVal;
}

int CDynMaterialObject::getIntPropertyValue(const char* pName) const
{
    int retVal = 0;
    getIntProperty(pName, retVal);
    return retVal;
}

double CDynMaterialObject::getFloatPropertyValue(const char* pName) const
{
    double retVal = 0.0;
    getFloatProperty(pName, retVal);
    return retVal;
}
