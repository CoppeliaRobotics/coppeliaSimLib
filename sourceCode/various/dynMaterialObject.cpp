#include <simInternal.h>
#include <dynMaterialObject.h>
#include <app.h>
#include <tt.h>
#include <boost/format.hpp>

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
        0.71); // simi_ode_body_friction (box on 26.8deg inclined plane slightly slides (same mat. for box and plane)
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
    _mujocoFloatParams.push_back(1.f);    // simi_mujoco_body_friction1
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

    _mujocoIntParams.push_back(3); // simi_mujoco_body_condim
    int mujocoBitCoded = 0;        // not used for now
    _mujocoIntParams.push_back(mujocoBitCoded);
    _mujocoIntParams.push_back(0); // simi_mujoco_body_priority
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

void CDynMaterialObject::setObjectName(const char *newName)
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

double CDynMaterialObject::getEngineFloatParam(int what, bool *ok)
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
    prop = _enumToProperty(what, sim_propertytype_vector, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> v;
        if (getVectorProperty(prop.c_str(), v) > 0)
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
/*
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_bullet_body_float_start) && (what < sim_bullet_body_float_end))
    {
        int w = what - sim_bullet_body_restitution + simi_bullet_body_restitution;
        return (_bulletFloatParams[w]);
    }
    if ((what > sim_ode_body_float_start) && (what < sim_ode_body_float_end))
    {
        int w = what - sim_ode_body_friction + simi_ode_body_friction;
        return (_odeFloatParams[w]);
    }
    if ((what > sim_vortex_body_float_start) && (what < sim_vortex_body_float_end))
    {
        int w = what - sim_vortex_body_primlinearaxisfriction + simi_vortex_body_primlinearaxisfriction;
        return (_vortexFloatParams[w]);
    }
    if ((what > sim_newton_body_float_start) && (what < sim_newton_body_float_end))
    {
        int w = what - sim_newton_body_staticfriction + simi_newton_body_staticfriction;
        return (_newtonFloatParams[w]);
    }
    if ((what > sim_mujoco_body_float_start) && (what < sim_mujoco_body_float_end))
    {
        int w = what - sim_mujoco_body_friction1 + simi_mujoco_body_friction1;
        return (_mujocoFloatParams[w]);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0.0);
    */
}

int CDynMaterialObject::getEngineIntParam(int what, bool *ok)
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
    prop = _enumToProperty(what, sim_propertytype_intvector, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<int> v;
        if (getIntVectorProperty(prop.c_str(), v) > 0)
            return v[indexWithArrays];
    }
    if (ok != nullptr)
        ok[0] = false;
    return 0;
/*
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_bullet_body_int_start) && (what < sim_bullet_body_int_end))
    {
        int w = what - sim_bullet_body_bitcoded + simi_bullet_body_bitcoded;
        return (_bulletIntParams[w]);
    }
    if ((what > sim_ode_body_int_start) && (what < sim_ode_body_int_end))
    {
        int w = what - sim_ode_body_maxcontacts + simi_ode_body_maxcontacts;
        return (_odeIntParams[w]);
    }
    if ((what > sim_vortex_body_int_start) && (what < sim_vortex_body_int_end))
    {
        int w = what - sim_vortex_body_primlinearaxisfrictionmodel + simi_vortex_body_primlinearaxisfrictionmodel;
        return (_vortexIntParams[w]);
    }
    if ((what > sim_newton_body_int_start) && (what < sim_newton_body_int_end))
    {
        int w = what - sim_newton_body_bitcoded + simi_newton_body_bitcoded;
        return (_newtonIntParams[w]);
    }
    if ((what > sim_mujoco_body_int_start) && (what < sim_mujoco_body_int_end))
    {
        int w = what - sim_mujoco_body_condim + simi_mujoco_body_condim;
        return (_mujocoIntParams[w]);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0);
    */
}

bool CDynMaterialObject::getEngineBoolParam(int what, bool *ok)
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
    /*
    if (ok != nullptr)
        ok[0] = true;
    if ((what > sim_bullet_body_bool_start) && (what < sim_bullet_body_bool_end))
    {
        int b = 1;
        int w = (what - sim_bullet_body_sticky);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        return ((_bulletIntParams[simi_bullet_body_bitcoded] & b) != 0);
    }
    if ((what > sim_ode_body_bool_start) && (what < sim_ode_body_bool_end))
    {
        // no bool params for now
        // search for obb11032016
        if (ok != nullptr)
            ok[0] = false;
        return (0);
    }
    if ((what > sim_vortex_body_bool_start) && (what < sim_vortex_body_bool_end))
    {
        int b = 1;
        int w = (what - sim_vortex_body_pureshapesasconvex);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        return ((_vortexIntParams[simi_vortex_body_bitcoded] & b) != 0);
    }
    if ((what > sim_newton_body_bool_start) && (what < sim_newton_body_bool_end))
    {
        int b = 1;
        int w = (what - sim_newton_body_fastmoving);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        return ((_newtonIntParams[simi_newton_body_bitcoded] & b) != 0);
    }
    if ((what > sim_mujoco_body_bool_start) && (what < sim_mujoco_body_bool_end))
    {
        // no bool params for now
        if (ok != nullptr)
            ok[0] = false;
        return (0);
    }
    if (ok != nullptr)
        ok[0] = false;
    return (0);
    */
}

bool CDynMaterialObject::setEngineFloatParam(int what, double v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_float, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setFloatProperty(prop.c_str(), v) > 0)
            return true;
    }
    prop = _enumToProperty(what, sim_propertytype_vector, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<double> w;
        if (getVectorProperty(prop.c_str(), w) > 0)
        {
            w[indexWithArrays] = v;
            if (setVectorProperty(prop.c_str(), w.data(), indexWithArrays + 1) > 0)
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
    /*
    if ((what > sim_bullet_body_float_start) && (what < sim_bullet_body_float_end))
    {
        int w = what - sim_bullet_body_restitution + simi_bullet_body_restitution;
        std::vector<double> fp;
        getBulletFloatParams(fp);
        fp[w] = v;
        setBulletFloatParams(fp);
        return (true);
    }
    if ((what > sim_ode_body_float_start) && (what < sim_ode_body_float_end))
    {
        int w = what - sim_ode_body_friction + simi_ode_body_friction;
        std::vector<double> fp;
        getOdeFloatParams(fp);
        fp[w] = v;
        setOdeFloatParams(fp);
        return (true);
    }
    if ((what > sim_vortex_body_float_start) && (what < sim_vortex_body_float_end))
    {
        int w = what - sim_vortex_body_primlinearaxisfriction + simi_vortex_body_primlinearaxisfriction;
        std::vector<double> fp;
        getVortexFloatParams(fp);
        fp[w] = v;
        setVortexFloatParams(fp);
        return (true);
    }
    if ((what > sim_newton_body_float_start) && (what < sim_newton_body_float_end))
    {
        int w = what - sim_newton_body_staticfriction + simi_newton_body_staticfriction;
        std::vector<double> fp;
        getNewtonFloatParams(fp);
        fp[w] = v;
        setNewtonFloatParams(fp);
        return (true);
    }
    if ((what > sim_mujoco_body_float_start) && (what < sim_mujoco_body_float_end))
    {
        int w = what - sim_mujoco_body_friction1 + simi_mujoco_body_friction1;
        std::vector<double> fp;
        getMujocoFloatParams(fp);
        fp[w] = v;
        setMujocoFloatParams(fp);
        return (true);
    }
    return (false);
    */
}

bool CDynMaterialObject::setEngineIntParam(int what, int v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_int, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setIntProperty(prop.c_str(), v) > 0)
            return true;
    }
    prop = _enumToProperty(what, sim_propertytype_intvector, indexWithArrays);
    if (prop.size() > 0)
    {
        std::vector<int> w;
        if (getIntVectorProperty(prop.c_str(), w) > 0)
        {
            w[indexWithArrays] = v;
            if (setIntVectorProperty(prop.c_str(), w.data(), indexWithArrays + 1) > 0)
                return true;
        }
    }
    return false;
    /*
    if ((what > sim_bullet_body_int_start) && (what < sim_bullet_body_int_end))
    {
        int w = what - sim_bullet_body_bitcoded + simi_bullet_body_bitcoded;
        std::vector<int> ip;
        getBulletIntParams(ip);
        ip[w] = v;
        setBulletIntParams(ip);
        return (true);
    }
    if ((what > sim_ode_body_int_start) && (what < sim_ode_body_int_end))
    {
        int w = what - sim_ode_body_maxcontacts + simi_ode_body_maxcontacts;
        std::vector<int> ip;
        getOdeIntParams(ip);
        ip[w] = v;
        setOdeIntParams(ip);
        return (true);
    }
    if ((what > sim_vortex_body_int_start) && (what < sim_vortex_body_int_end))
    {
        int w = what - sim_vortex_body_primlinearaxisfrictionmodel + simi_vortex_body_primlinearaxisfrictionmodel;
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[w] = v;
        setVortexIntParams(ip);
        return (true);
    }
    if ((what > sim_newton_body_int_start) && (what < sim_newton_body_int_end))
    {
        int w = what - sim_newton_body_bitcoded + simi_newton_body_bitcoded;
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[w] = v;
        setNewtonIntParams(ip);
        return (true);
    }
    if ((what > sim_mujoco_body_int_start) && (what < sim_mujoco_body_int_end))
    {
        int w = what - sim_mujoco_body_condim + simi_mujoco_body_condim;
        std::vector<int> ip;
        getMujocoIntParams(ip);
        ip[w] = v;
        setMujocoIntParams(ip);
        return (true);
    }
    return (false);
    */
}

bool CDynMaterialObject::setEngineBoolParam(int what, bool v)
{
    int indexWithArrays;
    std::string prop = _enumToProperty(what, sim_propertytype_bool, indexWithArrays);
    if (prop.size() > 0)
    {
        if (setBoolProperty(prop.c_str(), v) > 0)
            return true;
    }
    return false;
    /*
    if ((what > sim_bullet_body_bool_start) && (what < sim_bullet_body_bool_end))
    {
        int b = 1;
        int w = (what - sim_bullet_body_sticky);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        _bulletIntParams[simi_bullet_body_bitcoded] |= b;
        if (!v)
            _bulletIntParams[simi_bullet_body_bitcoded] -= b;
        return (true);
    }
    if ((what > sim_ode_body_bool_start) && (what < sim_ode_body_bool_end))
    {
        // no bool params yet
        // search for obb11032016
        return (false);
    }
    if ((what > sim_vortex_body_bool_start) && (what < sim_vortex_body_bool_end))
    {
        int b = 1;
        int w = (what - sim_vortex_body_pureshapesasconvex);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        _vortexIntParams[simi_vortex_body_bitcoded] |= b;
        if (!v)
            _vortexIntParams[simi_vortex_body_bitcoded] -= b;
        return (true);
    }
    if ((what > sim_newton_body_bool_start) && (what < sim_newton_body_bool_end))
    {
        int b = 1;
        int w = (what - sim_newton_body_fastmoving);
        while (w > 0)
        {
            b *= 2;
            w--;
        }
        _newtonIntParams[simi_newton_body_bitcoded] |= b;
        if (!v)
            _newtonIntParams[simi_newton_body_bitcoded] -= b;
        return (true);
    }
    if ((what > sim_mujoco_body_bool_start) && (what < sim_mujoco_body_bool_end))
    {
        // no bool params yet
        return (false);
    }
    return (false);
    */
}

void CDynMaterialObject::getBulletFloatParams(std::vector<double> &p)
{
    p.assign(_bulletFloatParams.begin(), _bulletFloatParams.end());
}

void CDynMaterialObject::setBulletFloatParams(const std::vector<double> &p)
{
    for (size_t i = 0; i < p.size(); i++)
        _bulletFloatParams[i] = p[i];

    _bulletFloatParams[simi_bullet_body_restitution] =
        tt::getLimitedFloat(0.0, 10.0, _bulletFloatParams[simi_bullet_body_restitution]); // restitution
    _bulletFloatParams[simi_bullet_body_oldfriction] =
        tt::getLimitedFloat(0.0, 1000.0, _bulletFloatParams[simi_bullet_body_oldfriction]); // old friction
    _bulletFloatParams[simi_bullet_body_friction] =
        tt::getLimitedFloat(0.0, 1000.0, _bulletFloatParams[simi_bullet_body_friction]); // new friction
    _bulletFloatParams[simi_bullet_body_lineardamping] =
        tt::getLimitedFloat(0.0, 10.0, _bulletFloatParams[simi_bullet_body_lineardamping]); // lin. damp.
    _bulletFloatParams[simi_bullet_body_angulardamping] =
        tt::getLimitedFloat(0.0, 10.0, _bulletFloatParams[simi_bullet_body_angulardamping]); // ang. damp.
    _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor] = tt::getLimitedFloat(
        0.0001, 1000.0,
        _bulletFloatParams
            [simi_bullet_body_nondefaultcollisionmargingfactor]); // _bulletNonDefaultCollisionMarginFactor
    _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex] = tt::getLimitedFloat(
        0.0001, 1000.0,
        _bulletFloatParams
            [simi_bullet_body_nondefaultcollisionmargingfactorconvex]); // _bulletNonDefaultCollisionMarginFactor_forConvexAndNonPureShape
}

void CDynMaterialObject::getBulletIntParams(std::vector<int> &p)
{
    p.assign(_bulletIntParams.begin(), _bulletIntParams.end());
}

void CDynMaterialObject::setBulletIntParams(const std::vector<int> &p)
{
    for (size_t i = 0; i < p.size(); i++)
        _bulletIntParams[i] = p[i];
}

void CDynMaterialObject::getOdeFloatParams(std::vector<double> &p)
{
    p.assign(_odeFloatParams.begin(), _odeFloatParams.end());
}

void CDynMaterialObject::setOdeFloatParams(const std::vector<double> &p)
{
    for (size_t i = 0; i < p.size(); i++)
        _odeFloatParams[i] = p[i];

    _odeFloatParams[simi_ode_body_friction] =
        tt::getLimitedFloat(0.0, 1000.0, _odeFloatParams[simi_ode_body_friction]); // friction
    _odeFloatParams[simi_ode_body_softerp] =
        tt::getLimitedFloat(0.0, 1.0, _odeFloatParams[simi_ode_body_softerp]); // soft ERP
    _odeFloatParams[simi_ode_body_softcfm] =
        tt::getLimitedFloat(0.0, 1000000.0, _odeFloatParams[simi_ode_body_softcfm]); // soft CFM
    _odeFloatParams[simi_ode_body_lineardamping] =
        tt::getLimitedFloat(0.0, 10.0, _odeFloatParams[simi_ode_body_lineardamping]); // lin. damp.
    _odeFloatParams[simi_ode_body_angulardamping] =
        tt::getLimitedFloat(0.0, 10.0, _odeFloatParams[simi_ode_body_angulardamping]); // ang. damp.
}

void CDynMaterialObject::getOdeIntParams(std::vector<int> &p)
{
    p.assign(_odeIntParams.begin(), _odeIntParams.end());
}

void CDynMaterialObject::setOdeIntParams(const std::vector<int> &p)
{
    for (size_t i = 0; i < p.size(); i++)
        _odeIntParams[i] = p[i];
    _odeIntParams[0] = tt::getLimitedInt(-64, 64, _odeIntParams[simi_ode_body_maxcontacts]); // max contacts
}

void CDynMaterialObject::getVortexFloatParams(std::vector<double> &p)
{
    p.assign(_vortexFloatParams.begin(), _vortexFloatParams.end());
}

void CDynMaterialObject::setVortexFloatParams(const std::vector<double> &p)
{
    int l = int(p.size());
    if (l > int(_vortexFloatParams.size()))
        l = int(_vortexFloatParams.size()); // should normally never happen!
    for (int i = 0; i < l; i++)
        _vortexFloatParams[i] = p[i];

    _vortexFloatParams[simi_vortex_body_skinthickness] =
        tt::getLimitedFloat(0.0, 10.0, _vortexFloatParams[simi_vortex_body_skinthickness]); // skin thickness
    _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio] = tt::getLimitedFloat(
        0.0, 1000.0,
        _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio]); // auto angular damping tension ratio
    // make sure the vector for the primary axis is normalized:
    C3Vector v(_vortexFloatParams[simi_vortex_body_primaxisvectorx],
               _vortexFloatParams[simi_vortex_body_primaxisvectory],
               _vortexFloatParams[simi_vortex_body_primaxisvectorz]);
    if (v.getLength() < 0.01)
        v = C3Vector::unitZVector;
    v.normalize();
    _vortexFloatParams[simi_vortex_body_primaxisvectorx] = v(0);
    _vortexFloatParams[simi_vortex_body_primaxisvectory] = v(1);
    _vortexFloatParams[simi_vortex_body_primaxisvectorz] = v(2);
}

void CDynMaterialObject::getVortexIntParams(std::vector<int> &p)
{
    _vortexIntParams[7] = _uniqueID;
    p.assign(_vortexIntParams.begin(), _vortexIntParams.end());
}

void CDynMaterialObject::setVortexIntParams(const std::vector<int> &p)
{
    int l = int(p.size());
    if (l > int(_vortexIntParams.size()))
        l = int(_vortexIntParams.size()); // should normally never happen!
    for (int i = 0; i < l; i++)
        _vortexIntParams[i] = p[i];

    _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold] =
        tt::getLimitedInt(1, 10000, _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold]); // auto-sleep steps
    _vortexIntParams[simi_vortex_body_materialuniqueid] = _uniqueID;
}

void CDynMaterialObject::getNewtonFloatParams(std::vector<double> &p)
{
    p.assign(_newtonFloatParams.begin(), _newtonFloatParams.end());
}

void CDynMaterialObject::setNewtonFloatParams(const std::vector<double> &p)
{
    int l = int(p.size());
    if (l > int(_newtonFloatParams.size()))
        l = int(_newtonFloatParams.size()); // should normally never happen!
    for (int i = 0; i < l; i++)
        _newtonFloatParams[i] = p[i];
    _newtonFloatParams[simi_newton_body_staticfriction] =
        tt::getLimitedFloat(0.0, 2.0, _newtonFloatParams[simi_newton_body_staticfriction]); // static friction
    _newtonFloatParams[simi_newton_body_kineticfriction] =
        tt::getLimitedFloat(0.0, _newtonFloatParams[simi_newton_body_staticfriction],
                            _newtonFloatParams[simi_newton_body_kineticfriction]); // kin. friction
    _newtonFloatParams[simi_newton_body_restitution] =
        tt::getLimitedFloat(0.0, 1.0, _newtonFloatParams[simi_newton_body_restitution]); // restitution
    _newtonFloatParams[simi_newton_body_lineardrag] =
        tt::getLimitedFloat(0.0, 1.0, _newtonFloatParams[simi_newton_body_lineardrag]); // lin. drag
    _newtonFloatParams[simi_newton_body_angulardrag] =
        tt::getLimitedFloat(0.0, 1.0, _newtonFloatParams[simi_newton_body_angulardrag]); // ang. drag
}

void CDynMaterialObject::getNewtonIntParams(std::vector<int> &p)
{
    p.assign(_newtonIntParams.begin(), _newtonIntParams.end());
}

void CDynMaterialObject::setNewtonIntParams(const std::vector<int> &p)
{
    int l = int(p.size());
    if (l > int(_newtonIntParams.size()))
        l = int(_newtonIntParams.size()); // should normally never happen!
    for (int i = 0; i < l; i++)
        _newtonIntParams[i] = p[i];
}

void CDynMaterialObject::getMujocoFloatParams(std::vector<double> &p)
{
    p.assign(_mujocoFloatParams.begin(), _mujocoFloatParams.end());
}

void CDynMaterialObject::setMujocoFloatParams(const std::vector<double> &p)
{
    for (size_t i = 0; i < p.size(); i++)
        _mujocoFloatParams[i] = p[i];
}

void CDynMaterialObject::getMujocoIntParams(std::vector<int> &p)
{
    p.assign(_mujocoIntParams.begin(), _mujocoIntParams.end());
}

void CDynMaterialObject::setMujocoIntParams(const std::vector<int> &p)
{
    for (size_t i = 0; i < p.size(); i++)
        _mujocoIntParams[i] = p[i];
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
        setEngineFloatParam(sim_bullet_body_oldfriction, 1.0);
        setEngineFloatParam(sim_bullet_body_friction, 1.0);
        setEngineFloatParam(sim_ode_body_friction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 1.0);
        setEngineFloatParam(sim_newton_body_staticfriction, 2.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 2.0);
    }
    if (defMatId == sim_dynmat_lowfriction)
    {
        _objectName = "lowFrictionMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 0.41);
        setEngineFloatParam(sim_bullet_body_friction, 0.20);
        setEngineFloatParam(sim_ode_body_friction, 0.41);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 0.17);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 0.17);
        setEngineFloatParam(sim_newton_body_staticfriction, 0.17);
        setEngineFloatParam(sim_newton_body_kineticfriction, 0.17);
    }
    if (defMatId == sim_dynmat_nofriction)
    {
        _objectName = "noFrictionMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 0.0);
        setEngineFloatParam(sim_bullet_body_friction, 0.0);
        setEngineFloatParam(sim_ode_body_friction, 0.0);
        setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel, sim_vortex_bodyfrictionmodel_none);
        setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel, sim_vortex_bodyfrictionmodel_none);
        setEngineFloatParam(sim_newton_body_staticfriction, 0.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 0.0);
    }
    if (defMatId == sim_dynmat_reststackgrasp)
    {
        _objectName = "restStackGraspMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 1.0);
        setEngineFloatParam(sim_bullet_body_friction, 0.5);
        setEngineFloatParam(sim_ode_body_friction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_skinthickness, 0.002);
        setEngineBoolParam(sim_vortex_body_autoangulardamping, true);
        setEngineFloatParam(sim_newton_body_staticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_restitution, 0.0);
    }
    if (defMatId == sim_dynmat_foot)
    {
        _objectName = "footMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 1.0);
        setEngineFloatParam(sim_bullet_body_friction, 0.5);
        setEngineFloatParam(sim_ode_body_friction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_skinthickness, 0.002);
        setEngineFloatParam(sim_newton_body_staticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_restitution, 0.0);
    }
    if (defMatId == sim_dynmat_wheel)
    {
        _objectName = "wheelMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 1.0);
        setEngineFloatParam(sim_bullet_body_friction, 1.0);
        setEngineFloatParam(sim_ode_body_friction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_skinthickness, 0.0);
        setEngineFloatParam(sim_newton_body_staticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_restitution, 0.0);
    }
    if (defMatId == sim_dynmat_gripper)
    {
        _objectName = "gripperMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 1.0);
        setEngineFloatParam(sim_bullet_body_friction, 0.5);
        setEngineFloatParam(sim_ode_body_friction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_skinthickness, 0.002);
        setEngineFloatParam(sim_newton_body_staticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_restitution, 0.0);
    }
    if (defMatId == sim_dynmat_floor)
    {
        _objectName = "floorMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction, 1.0);
        setEngineFloatParam(sim_bullet_body_friction, 1.0);
        setEngineFloatParam(sim_ode_body_friction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, 1.0);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip, 0.0);
        setEngineFloatParam(sim_vortex_body_skinthickness, 0.002);
        setEngineFloatParam(sim_newton_body_staticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_kineticfriction, 1.0);
        setEngineFloatParam(sim_newton_body_restitution, 0.0);
    }
}

CDynMaterialObject *CDynMaterialObject::copyYourself()
{
    CDynMaterialObject *newObj = new CDynMaterialObject();
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

void CDynMaterialObject::serialize(CSer &ar)
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
            ar.xmlAddNode_float("restitution", getEngineFloatParam(sim_bullet_body_restitution, nullptr));
            ar.xmlAddNode_float("friction", getEngineFloatParam(sim_bullet_body_friction, nullptr));
            ar.xmlAddNode_float("oldfriction", getEngineFloatParam(sim_bullet_body_oldfriction, nullptr));
            ar.xmlAddNode_float("lineardamping", getEngineFloatParam(sim_bullet_body_lineardamping, nullptr));
            ar.xmlAddNode_float("angulardamping", getEngineFloatParam(sim_bullet_body_angulardamping, nullptr));
            ar.xmlAddNode_float("nondefaultcollisionmargingfactor",
                                getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor, nullptr));
            ar.xmlAddNode_float("nondefaultcollisionmargingfactorconvex",
                                getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex, nullptr));

            ar.xmlAddNode_bool("sticky", getEngineBoolParam(sim_bullet_body_sticky, nullptr));
            ar.xmlAddNode_bool("usenondefaultcollisionmargin",
                               getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin, nullptr));
            ar.xmlAddNode_bool("usenondefaultcollisionmarginconvex",
                               getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex, nullptr));
            ar.xmlAddNode_bool("autoshrinkconvex", getEngineBoolParam(sim_bullet_body_autoshrinkconvex, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("ode");
            ar.xmlAddNode_float("friction", getEngineFloatParam(sim_ode_body_friction, nullptr));
            ar.xmlAddNode_float("softerp", getEngineFloatParam(sim_ode_body_softerp, nullptr));
            ar.xmlAddNode_float("softcfm", getEngineFloatParam(sim_ode_body_softcfm, nullptr));
            ar.xmlAddNode_float("lineardamping", getEngineFloatParam(sim_ode_body_lineardamping, nullptr));
            ar.xmlAddNode_float("angulardamping", getEngineFloatParam(sim_ode_body_angulardamping, nullptr));

            ar.xmlAddNode_int("maxcontacts", getEngineIntParam(sim_ode_body_maxcontacts, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("vortex");
            ar.xmlAddNode_float("primlinearaxisfriction",
                                getEngineFloatParam(sim_vortex_body_primlinearaxisfriction, nullptr));
            ar.xmlAddNode_float("seclinearaxisfriction",
                                getEngineFloatParam(sim_vortex_body_seclinearaxisfriction, nullptr));
            ar.xmlAddNode_float("primangularaxisfriction",
                                getEngineFloatParam(sim_vortex_body_primangularaxisfriction, nullptr));
            ar.xmlAddNode_float("secangularaxisfriction",
                                getEngineFloatParam(sim_vortex_body_secangularaxisfriction, nullptr));
            ar.xmlAddNode_float("normalangularaxisfriction",
                                getEngineFloatParam(sim_vortex_body_normalangularaxisfriction, nullptr));
            ar.xmlAddNode_float("primlinearaxisstaticfrictionscale",
                                getEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale, nullptr));
            ar.xmlAddNode_float("seclinearaxisstaticfrictionscale",
                                getEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale, nullptr));
            ar.xmlAddNode_float("primangularaxisstaticfrictionscale",
                                getEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale, nullptr));
            ar.xmlAddNode_float("secangularaxisstaticfrictionscale",
                                getEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale, nullptr));
            ar.xmlAddNode_float("normalangularaxisstaticfrictionscale",
                                getEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale, nullptr));
            ar.xmlAddNode_float("compliance", getEngineFloatParam(sim_vortex_body_compliance, nullptr));
            ar.xmlAddNode_float("damping", getEngineFloatParam(sim_vortex_body_damping, nullptr));
            ar.xmlAddNode_float("restitution", getEngineFloatParam(sim_vortex_body_restitution, nullptr));
            ar.xmlAddNode_float("restitutionthreshold",
                                getEngineFloatParam(sim_vortex_body_restitutionthreshold, nullptr));
            ar.xmlAddNode_float("adhesiveforce", getEngineFloatParam(sim_vortex_body_adhesiveforce, nullptr));
            ar.xmlAddNode_float("linearvelocitydamping",
                                getEngineFloatParam(sim_vortex_body_linearvelocitydamping, nullptr));
            ar.xmlAddNode_float("angularvelocitydamping",
                                getEngineFloatParam(sim_vortex_body_angularvelocitydamping, nullptr));
            ar.xmlAddNode_float("primlinearaxisslide",
                                getEngineFloatParam(sim_vortex_body_primlinearaxisslide, nullptr));
            ar.xmlAddNode_float("seclinearaxisslide", getEngineFloatParam(sim_vortex_body_seclinearaxisslide, nullptr));
            ar.xmlAddNode_float("primangularaxisslide",
                                getEngineFloatParam(sim_vortex_body_primangularaxisslide, nullptr));
            ar.xmlAddNode_float("secangularaxisslide",
                                getEngineFloatParam(sim_vortex_body_secangularaxisslide, nullptr));
            ar.xmlAddNode_float("normalangularaxisslide",
                                getEngineFloatParam(sim_vortex_body_normalangularaxisslide, nullptr));
            ar.xmlAddNode_float("primlinearaxisslip", getEngineFloatParam(sim_vortex_body_primlinearaxisslip, nullptr));
            ar.xmlAddNode_float("seclinearaxisslip", getEngineFloatParam(sim_vortex_body_seclinearaxisslip, nullptr));
            ar.xmlAddNode_float("primangularaxisslip",
                                getEngineFloatParam(sim_vortex_body_primangularaxisslip, nullptr));
            ar.xmlAddNode_float("secangularaxisslip", getEngineFloatParam(sim_vortex_body_secangularaxisslip, nullptr));
            ar.xmlAddNode_float("normalangularaxisslip",
                                getEngineFloatParam(sim_vortex_body_normalangularaxisslip, nullptr));
            ar.xmlAddNode_float("autosleeplinearspeedthreshold",
                                getEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold, nullptr));
            ar.xmlAddNode_float("autosleeplinearaccelthreshold",
                                getEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold, nullptr));
            ar.xmlAddNode_float("autosleepangularspeedthreshold",
                                getEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold, nullptr));
            ar.xmlAddNode_float("autosleepangularaccelthreshold",
                                getEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold, nullptr));
            ar.xmlAddNode_float("skinthickness", getEngineFloatParam(sim_vortex_body_skinthickness, nullptr));
            ar.xmlAddNode_float("autoangulardampingtensionratio",
                                getEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio, nullptr));
            ar.xmlAddNode_float("primaxisvectorx", getEngineFloatParam(sim_vortex_body_primaxisvectorx, nullptr));
            ar.xmlAddNode_float("primaxisvectory", getEngineFloatParam(sim_vortex_body_primaxisvectory, nullptr));
            ar.xmlAddNode_float("primaxisvectorz", getEngineFloatParam(sim_vortex_body_primaxisvectorz, nullptr));

            ar.xmlAddNode_int("primlinearaxisfrictionmodel",
                              getEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel, nullptr));
            ar.xmlAddNode_int("seclinearaxisfrictionmodel",
                              getEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel, nullptr));
            ar.xmlAddNode_int("primangulararaxisfrictionmodel",
                              getEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel, nullptr));
            ar.xmlAddNode_int("secmangulararaxisfrictionmodel",
                              getEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel, nullptr));
            ar.xmlAddNode_int("normalmangulararaxisfrictionmodel",
                              getEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel, nullptr));
            ar.xmlAddNode_int("autosleepsteplivethreshold",
                              getEngineIntParam(sim_vortex_body_autosleepsteplivethreshold, nullptr));
            ar.xmlAddNode_int("materialuniqueid", getEngineIntParam(sim_vortex_body_materialuniqueid, nullptr));

            ar.xmlAddNode_bool("pureshapesasconvex", getEngineBoolParam(sim_vortex_body_pureshapesasconvex, nullptr));
            ar.xmlAddNode_bool("convexshapesasrandom",
                               getEngineBoolParam(sim_vortex_body_convexshapesasrandom, nullptr));
            ar.xmlAddNode_bool("randomshapesasterrain",
                               getEngineBoolParam(sim_vortex_body_randomshapesasterrain, nullptr));
            ar.xmlAddNode_bool("fastmoving", getEngineBoolParam(sim_vortex_body_fastmoving, nullptr));
            ar.xmlAddNode_bool("autoslip", getEngineBoolParam(sim_vortex_body_autoslip, nullptr));
            ar.xmlAddNode_bool("seclinaxissameasprimlinaxis",
                               getEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis, nullptr));
            ar.xmlAddNode_bool("secangaxissameasprimangaxis",
                               getEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis, nullptr));
            ar.xmlAddNode_bool("normangaxissameasprimangaxis",
                               getEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis, nullptr));
            ar.xmlAddNode_bool("autoangulardamping", getEngineBoolParam(sim_vortex_body_autoangulardamping, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("newton");
            ar.xmlAddNode_float("staticfriction", getEngineFloatParam(sim_newton_body_staticfriction, nullptr));
            ar.xmlAddNode_float("kineticfriction", getEngineFloatParam(sim_newton_body_kineticfriction, nullptr));
            ar.xmlAddNode_float("restitution", getEngineFloatParam(sim_newton_body_restitution, nullptr));
            ar.xmlAddNode_float("lineardrag", getEngineFloatParam(sim_newton_body_lineardrag, nullptr));
            ar.xmlAddNode_float("angulardrag", getEngineFloatParam(sim_newton_body_angulardrag, nullptr));
            ar.xmlAddNode_bool("fastmoving", getEngineBoolParam(sim_newton_body_fastmoving, nullptr));
            ar.xmlPopNode();

            ar.xmlPushNewNode("mujoco");
            ar.xmlAddNode_3float("friction", getEngineFloatParam(sim_mujoco_body_friction1, nullptr),
                                 getEngineFloatParam(sim_mujoco_body_friction2, nullptr),
                                 getEngineFloatParam(sim_mujoco_body_friction3, nullptr));
            ar.xmlAddNode_2float("solref", getEngineFloatParam(sim_mujoco_body_solref1, nullptr),
                                 getEngineFloatParam(sim_mujoco_body_solref2, nullptr));
            double si[5];
            si[0] = getEngineFloatParam(sim_mujoco_body_solimp1, nullptr);
            si[1] = getEngineFloatParam(sim_mujoco_body_solimp2, nullptr);
            si[2] = getEngineFloatParam(sim_mujoco_body_solimp3, nullptr);
            si[3] = getEngineFloatParam(sim_mujoco_body_solimp4, nullptr);
            si[4] = getEngineFloatParam(sim_mujoco_body_solimp5, nullptr);
            ar.xmlAddNode_floats("solimp", si, 5);
            ar.xmlAddNode_float("solmix", getEngineFloatParam(sim_mujoco_body_solmix, nullptr));
            ar.xmlAddNode_float("margin", getEngineFloatParam(sim_mujoco_body_margin, nullptr));
            ar.xmlAddNode_int("condim", getEngineIntParam(sim_mujoco_body_condim, nullptr));
            ar.xmlAddNode_int("priority", getEngineIntParam(sim_mujoco_body_priority, nullptr));
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
                        setEngineFloatParam(sim_bullet_body_restitution, v);
                    if (ar.xmlGetNode_float("friction", v, exhaustiveXml))
                        setEngineFloatParam(sim_bullet_body_friction, v);
                    if (ar.xmlGetNode_float("oldfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_bullet_body_oldfriction, v);
                    if (ar.xmlGetNode_float("lineardamping", v, exhaustiveXml))
                        setEngineFloatParam(sim_bullet_body_lineardamping, v);
                    if (ar.xmlGetNode_float("angulardamping", v, exhaustiveXml))
                        setEngineFloatParam(sim_bullet_body_angulardamping, v);
                    if (ar.xmlGetNode_float("nondefaultcollisionmargingfactor", v, exhaustiveXml))
                        setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor, v);
                    if (ar.xmlGetNode_float("nondefaultcollisionmargingfactorconvex", v, exhaustiveXml))
                        setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex, v);

                    if (ar.xmlGetNode_bool("sticky", vb, exhaustiveXml))
                        setEngineBoolParam(sim_bullet_body_sticky, vb);
                    if (ar.xmlGetNode_bool("usenondefaultcollisionmargin", vb, exhaustiveXml))
                        setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin, vb);
                    if (ar.xmlGetNode_bool("usenondefaultcollisionmarginconvex", vb, exhaustiveXml))
                        setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex, vb);
                    if (ar.xmlGetNode_bool("autoshrinkconvex", vb, exhaustiveXml))
                        setEngineBoolParam(sim_bullet_body_autoshrinkconvex, vb);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("ode", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("friction", v, exhaustiveXml))
                        setEngineFloatParam(sim_ode_body_friction, v);
                    if (ar.xmlGetNode_float("softerp", v, exhaustiveXml))
                        setEngineFloatParam(sim_ode_body_softerp, v);
                    if (ar.xmlGetNode_float("softcfm", v, exhaustiveXml))
                        setEngineFloatParam(sim_ode_body_softcfm, v);
                    if (ar.xmlGetNode_float("lineardamping", v, exhaustiveXml))
                        setEngineFloatParam(sim_ode_body_lineardamping, v);
                    if (ar.xmlGetNode_float("angulardamping", v, exhaustiveXml))
                        setEngineFloatParam(sim_ode_body_angulardamping, v);

                    if (ar.xmlGetNode_int("maxcontacts", vi, exhaustiveXml))
                        setEngineIntParam(sim_ode_body_maxcontacts, vi);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("vortex", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("primlinearaxisfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction, v);
                    if (ar.xmlGetNode_float("seclinearaxisfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction, v);
                    if (ar.xmlGetNode_float("primangularaxisfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primangularaxisfriction, v);
                    if (ar.xmlGetNode_float("secangularaxisfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_secangularaxisfriction, v);
                    if (ar.xmlGetNode_float("normalangularaxisfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_normalangularaxisfriction, v);
                    if (ar.xmlGetNode_float("primlinearaxisstaticfrictionscale", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale, v);
                    if (ar.xmlGetNode_float("seclinearaxisstaticfrictionscale", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale, v);
                    if (ar.xmlGetNode_float("primangularaxisstaticfrictionscale", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale, v);
                    if (ar.xmlGetNode_float("secangularaxisstaticfrictionscale", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale, v);
                    if (ar.xmlGetNode_float("normalangularaxisstaticfrictionscale", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale, v);
                    if (ar.xmlGetNode_float("compliance", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_compliance, v);
                    if (ar.xmlGetNode_float("damping", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_damping, v);
                    if (ar.xmlGetNode_float("restitution", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_restitution, v);
                    if (ar.xmlGetNode_float("restitutionthreshold", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_restitutionthreshold, v);
                    if (ar.xmlGetNode_float("adhesiveforce", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_adhesiveforce, v);
                    if (ar.xmlGetNode_float("linearvelocitydamping", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_linearvelocitydamping, v);
                    if (ar.xmlGetNode_float("angularvelocitydamping", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_angularvelocitydamping, v);
                    if (ar.xmlGetNode_float("primlinearaxisslide", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primlinearaxisslide, v);
                    if (ar.xmlGetNode_float("seclinearaxisslide", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_seclinearaxisslide, v);
                    if (ar.xmlGetNode_float("primangularaxisslide", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primangularaxisslide, v);
                    if (ar.xmlGetNode_float("secangularaxisslide", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_secangularaxisslide, v);
                    if (ar.xmlGetNode_float("normalangularaxisslide", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_normalangularaxisslide, v);
                    if (ar.xmlGetNode_float("primlinearaxisslip", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primlinearaxisslip, v);
                    if (ar.xmlGetNode_float("seclinearaxisslip", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_seclinearaxisslip, v);
                    if (ar.xmlGetNode_float("primangularaxisslip", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primangularaxisslip, v);
                    if (ar.xmlGetNode_float("secangularaxisslip", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_secangularaxisslip, v);
                    if (ar.xmlGetNode_float("normalangularaxisslip", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_normalangularaxisslip, v);
                    if (ar.xmlGetNode_float("autosleeplinearspeedthreshold", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold, v);
                    if (ar.xmlGetNode_float("autosleeplinearaccelthreshold", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold, v);
                    if (ar.xmlGetNode_float("autosleepangularspeedthreshold", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold, v);
                    if (ar.xmlGetNode_float("autosleepangularaccelthreshold", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold, v);
                    if (ar.xmlGetNode_float("skinthickness", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_skinthickness, v);
                    if (ar.xmlGetNode_float("autoangulardampingtensionratio", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio, v);
                    if (ar.xmlGetNode_float("primaxisvectorx", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primaxisvectorx, v);
                    if (ar.xmlGetNode_float("primaxisvectory", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primaxisvectory, v);
                    if (ar.xmlGetNode_float("primaxisvectorz", v, exhaustiveXml))
                        setEngineFloatParam(sim_vortex_body_primaxisvectorz, v);

                    if (ar.xmlGetNode_int("primlinearaxisfrictionmodel", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel, vi);
                    if (ar.xmlGetNode_int("seclinearaxisfrictionmodel", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel, vi);
                    if (ar.xmlGetNode_int("primangulararaxisfrictionmodel", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel, vi);
                    if (ar.xmlGetNode_int("secmangulararaxisfrictionmodel", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel, vi);
                    if (ar.xmlGetNode_int("normalmangulararaxisfrictionmodel", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel, vi);
                    if (ar.xmlGetNode_int("autosleepsteplivethreshold", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_autosleepsteplivethreshold, vi);
                    if (ar.xmlGetNode_int("materialuniqueid", vi, exhaustiveXml))
                        setEngineIntParam(sim_vortex_body_materialuniqueid, vi);

                    if (ar.xmlGetNode_bool("pureshapesasconvex", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_pureshapesasconvex, vb);
                    if (ar.xmlGetNode_bool("convexshapesasrandom", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_convexshapesasrandom, vb);
                    if (ar.xmlGetNode_bool("randomshapesasterrain", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_randomshapesasterrain, vb);
                    if (ar.xmlGetNode_bool("fastmoving", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_fastmoving, vb);
                    if (ar.xmlGetNode_bool("autoslip", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_autoslip, vb);
                    if (ar.xmlGetNode_bool("seclinaxissameasprimlinaxis", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis, vb);
                    if (ar.xmlGetNode_bool("secangaxissameasprimangaxis", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis, vb);
                    if (ar.xmlGetNode_bool("normangaxissameasprimangaxis", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis, vb);
                    if (ar.xmlGetNode_bool("autoangulardamping", vb, exhaustiveXml))
                        setEngineBoolParam(sim_vortex_body_autoangulardamping, vb);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("newton", exhaustiveXml))
                {
                    if (ar.xmlGetNode_float("staticfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_newton_body_staticfriction, v);
                    if (ar.xmlGetNode_float("kineticfriction", v, exhaustiveXml))
                        setEngineFloatParam(sim_newton_body_kineticfriction, v);
                    if (ar.xmlGetNode_float("restitution", v, exhaustiveXml))
                        setEngineFloatParam(sim_newton_body_restitution, v);
                    if (ar.xmlGetNode_float("lineardrag", v, exhaustiveXml))
                        setEngineFloatParam(sim_newton_body_lineardrag, v);
                    if (ar.xmlGetNode_float("angulardrag", v, exhaustiveXml))
                        setEngineFloatParam(sim_newton_body_angulardrag, v);

                    if (ar.xmlGetNode_bool("fastmoving", vb, exhaustiveXml))
                        setEngineBoolParam(sim_newton_body_fastmoving, vb);
                    ar.xmlPopNode();
                }

                if (ar.xmlPushChildNode("mujoco", exhaustiveXml))
                {
                    double vv[5];
                    if (ar.xmlGetNode_floats("friction", vv, 3, exhaustiveXml))
                    {
                        setEngineFloatParam(sim_mujoco_body_friction1, vv[0]);
                        setEngineFloatParam(sim_mujoco_body_friction2, vv[1]);
                        setEngineFloatParam(sim_mujoco_body_friction3, vv[2]);
                    }
                    if (ar.xmlGetNode_floats("solref", vv, 2, exhaustiveXml))
                    {
                        setEngineFloatParam(sim_mujoco_body_solref1, vv[0]);
                        setEngineFloatParam(sim_mujoco_body_solref2, vv[1]);
                    }
                    if (ar.xmlGetNode_floats("solimp", vv, 5, exhaustiveXml))
                    {
                        setEngineFloatParam(sim_mujoco_body_solimp1, vv[0]);
                        setEngineFloatParam(sim_mujoco_body_solimp2, vv[1]);
                        setEngineFloatParam(sim_mujoco_body_solimp3, vv[2]);
                        setEngineFloatParam(sim_mujoco_body_solimp4, vv[3]);
                        setEngineFloatParam(sim_mujoco_body_solimp5, vv[4]);
                    }
                    if (ar.xmlGetNode_float("solmix", v, exhaustiveXml))
                        setEngineFloatParam(sim_mujoco_body_solmix, v);
                    if (ar.xmlGetNode_float("margin", v, exhaustiveXml))
                        setEngineFloatParam(sim_mujoco_body_margin, v);
                    if (ar.xmlGetNode_int("condim", vi, exhaustiveXml))
                        setEngineIntParam(sim_mujoco_body_condim, vi);
                    if (ar.xmlGetNode_int("priority", vi, exhaustiveXml))
                        setEngineIntParam(sim_mujoco_body_priority, vi);
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

std::string CDynMaterialObject::_enumToProperty(int oldEnum, int type, int& indexWithArrays)
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
                    if ( (j > 0) || (allProps_material[i].oldEnums[j + 1] != -1) )
                        indexWithArrays = int(j);
                    else
                        indexWithArrays = -1;
                    retVal =  allProps_material[i].name;
                }
                break;
            }
        }
        if (retVal.size() > 0)
            break;
    }
    return retVal;
}

int CDynMaterialObject::setBoolProperty(const char* pName, bool pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;
    N = propMaterial_bulletSticky.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_bulletIntParams[simi_bullet_body_bitcoded] | simi_bullet_body_sticky) - (1 - pState) * simi_bullet_body_sticky;
        if ( (nv != _bulletIntParams[simi_bullet_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletIntParams[simi_bullet_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyBool(N.c_str(), _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_sticky);
            }
        }
    }

    N = propMaterial_bulletNonDefaultCollisionMargin.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_bulletIntParams[simi_bullet_body_bitcoded] | simi_bullet_body_usenondefaultcollisionmargin) - (1 - pState) * simi_bullet_body_usenondefaultcollisionmargin;
        if ( (nv != _bulletIntParams[simi_bullet_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletIntParams[simi_bullet_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_usenondefaultcollisionmargin);
            }
        }
    }

    N = propMaterial_bulletNonDefaultCollisionMarginConvex.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_bulletIntParams[simi_bullet_body_bitcoded] | simi_bullet_body_usenondefaultcollisionmarginconvex) - (1 - pState) * simi_bullet_body_usenondefaultcollisionmarginconvex;
        if ( (nv != _bulletIntParams[simi_bullet_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletIntParams[simi_bullet_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_usenondefaultcollisionmarginconvex);
            }
        }
    }

    N = propMaterial_bulletAutoShrinkConvex.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_bulletIntParams[simi_bullet_body_bitcoded] | simi_bullet_body_autoshrinkconvex) - (1 - pState) * simi_bullet_body_autoshrinkconvex;
        if ( (nv != _bulletIntParams[simi_bullet_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletIntParams[simi_bullet_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _bulletIntParams[simi_bullet_body_bitcoded] & simi_bullet_body_autoshrinkconvex);
            }
        }
    }

    N = propMaterial_vortexPrimitiveShapesAsConvex.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_pureshapesasconvex) - (1 - pState) * simi_vortex_body_pureshapesasconvex;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_pureshapesasconvex);
            }
        }
    }

    N = propMaterial_vortexConvexShapesAsRandom.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_convexshapesasrandom) - (1 - pState) * simi_vortex_body_convexshapesasrandom;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_convexshapesasrandom);
            }
        }
    }

    N = propMaterial_vortexRandomShapesAsTerrain.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_randomshapesasterrain) - (1 - pState) * simi_vortex_body_randomshapesasterrain;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_randomshapesasterrain);
            }
        }
    }

    N = propMaterial_vortexFastMoving.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_fastmoving) - (1 - pState) * simi_vortex_body_fastmoving;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_fastmoving);
            }
        }
    }

    N = propMaterial_vortexAutoSlip.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_autoslip) - (1 - pState) * simi_vortex_body_autoslip;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_autoslip);
            }
        }
    }

    N = propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_seclinaxissameasprimlinaxis) - (1 - pState) * simi_vortex_body_seclinaxissameasprimlinaxis;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_seclinaxissameasprimlinaxis);
            }
        }
    }

    N = propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_secangaxissameasprimangaxis) - (1 - pState) * simi_vortex_body_secangaxissameasprimangaxis;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_secangaxissameasprimangaxis);
            }
        }
    }

    N = propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_normangaxissameasprimangaxis) - (1 - pState) * simi_vortex_body_normangaxissameasprimangaxis;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_normangaxissameasprimangaxis);
            }
        }
    }

    N = propMaterial_vortexAutoAngularDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_vortexIntParams[simi_vortex_body_bitcoded] | simi_vortex_body_autoangulardamping) - (1 - pState) * simi_vortex_body_autoangulardamping;
        if ( (nv != _vortexIntParams[simi_vortex_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _vortexIntParams[simi_vortex_body_bitcoded] & simi_vortex_body_autoangulardamping);
            }
        }
    }

    N = propMaterial_newtonFastMoving.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        int nv = (_newtonIntParams[simi_newton_body_bitcoded] | simi_newton_body_fastmoving) - (1 - pState) * simi_newton_body_fastmoving;
        if ( (nv != _newtonIntParams[simi_newton_body_bitcoded]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _newtonIntParams[simi_newton_body_bitcoded] = nv;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);
                ev->appendKeyBool(N.c_str(), _newtonIntParams[simi_newton_body_bitcoded] & simi_newton_body_fastmoving);
            }
        }
    }

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getBoolProperty(const char* pName, bool& pState)
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

    return retVal;
}

int CDynMaterialObject::setIntProperty(const char* pName, int pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    N = propMaterial_odeMaxContacts.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _odeIntParams[simi_ode_body_maxcontacts]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _odeIntParams[simi_ode_body_maxcontacts] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _odeIntParams[simi_ode_body_maxcontacts]);
            }
        }
    }

    N = propMaterial_vortexPrimaryLinearAxisFrictionModel.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel]);
            }
        }
    }

    N = propMaterial_vortexSecopndaryLinearAxisFrictionModel.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel]);
            }
        }
    }

    N = propMaterial_vortexPrimaryAngularAxisFrictionModel.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_primangulararaxisfrictionmodel]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_primangulararaxisfrictionmodel] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_primangulararaxisfrictionmodel]);
            }
        }
    }

    N = propMaterial_vortexSecondaryAngularAxisFrictionModel.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_secmangulararaxisfrictionmodel]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_secmangulararaxisfrictionmodel] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_secmangulararaxisfrictionmodel]);
            }
        }
    }

    N = propMaterial_vortexNormalAngularAxisFrictionModel.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_normalmangulararaxisfrictionmodel]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_normalmangulararaxisfrictionmodel] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_normalmangulararaxisfrictionmodel]);
            }
        }
    }

    N = propMaterial_vortexAutoSleepStepLiveThreshold.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold]);
            }
        }
    }

    N = propMaterial_vortexMaterialUniqueId.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexIntParams[simi_vortex_body_materialuniqueid]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexIntParams[simi_vortex_body_materialuniqueid] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _vortexIntParams[simi_vortex_body_materialuniqueid]);
            }
        }
    }

    N = propMaterial_mujocoCondim.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _mujocoIntParams[simi_mujoco_body_condim]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _mujocoIntParams[simi_mujoco_body_condim] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _mujocoIntParams[simi_mujoco_body_condim]);
            }
        }
    }

    N = propMaterial_mujocoPriority.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _mujocoIntParams[simi_mujoco_body_priority]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _mujocoIntParams[simi_mujoco_body_priority] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyInt(N.c_str(), _mujocoIntParams[simi_mujoco_body_priority]);
            }
        }
    }

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getIntProperty(const char* pName, int& pState)
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
    else if (strcmp(pName, propMaterial_vortexSecopndaryLinearAxisFrictionModel.name) == 0)
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
        pState = _vortexIntParams[simi_mujoco_body_condim];
    }
    else if (strcmp(pName, propMaterial_mujocoPriority.name) == 0)
    {
        retVal = 1;
        pState = _vortexIntParams[simi_mujoco_body_priority];
    }

    return retVal;
}

int CDynMaterialObject::setFloatProperty(const char* pName, double pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    N = propMaterial_bulletRestitution.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_restitution]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_restitution] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_restitution]);
            }
        }
    }

    N = propMaterial_bulletFriction0.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_oldfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_oldfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_oldfriction]);
            }
        }
    }

    N = propMaterial_bulletFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_friction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_friction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_friction]);
            }
        }
    }

    N = propMaterial_bulletLinearDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_lineardamping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_lineardamping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_lineardamping]);
            }
        }
    }

    N = propMaterial_bulletAngularDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_angulardamping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_angulardamping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_angulardamping]);
            }
        }
    }

    N = propMaterial_bulletNonDefaultCollisionMarginFactor.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor]);
            }
        }
    }

    N = propMaterial_bulletNonDefaultCollisionMarginFactorConvex.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex]);
            }
        }
    }

    N = propMaterial_odeFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _odeFloatParams[simi_ode_body_friction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _odeFloatParams[simi_ode_body_friction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _odeFloatParams[simi_ode_body_friction]);
            }
        }
    }

    N = propMaterial_odeSoftErp.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _odeFloatParams[simi_ode_body_softerp]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _odeFloatParams[simi_ode_body_softerp] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _odeFloatParams[simi_ode_body_softerp]);
            }
        }
    }

    N = propMaterial_odeSoftCfm.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _odeFloatParams[simi_ode_body_softcfm]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _odeFloatParams[simi_ode_body_softcfm] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _odeFloatParams[simi_ode_body_softcfm]);
            }
        }
    }

    N = propMaterial_odeLinearDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _odeFloatParams[simi_ode_body_lineardamping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _odeFloatParams[simi_ode_body_lineardamping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _odeFloatParams[simi_ode_body_lineardamping]);
            }
        }
    }

    N = propMaterial_odeAngularDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _odeFloatParams[simi_ode_body_angulardamping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _odeFloatParams[simi_ode_body_angulardamping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _odeFloatParams[simi_ode_body_angulardamping]);
            }
        }
    }

    N = propMaterial_vortexPrimaryLinearAxisFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primlinearaxisfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primlinearaxisfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primlinearaxisfriction]);
            }
        }
    }

    N = propMaterial_vortexSecondaryLinearAxisFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_seclinearaxisfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_seclinearaxisfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_seclinearaxisfriction]);
            }
        }
    }

    N = propMaterial_vortexPrimaryAngularAxisFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primangularaxisfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primangularaxisfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primangularaxisfriction]);
            }
        }
    }

    N = propMaterial_vortexSecondaryAngularAxisFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_secangularaxisfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_secangularaxisfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_secangularaxisfriction]);
            }
        }
    }

    N = propMaterial_vortexNormalAngularAxisFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_normalangularaxisfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_normalangularaxisfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_normalangularaxisfriction]);
            }
        }
    }

    N = propMaterial_vortexPrimaryLinearAxisStaticFrictionScale.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primlinearaxisstaticfrictionscale]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primlinearaxisstaticfrictionscale] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primlinearaxisstaticfrictionscale]);
            }
        }
    }

    N = propMaterial_vortexSecondaryLinearAxisStaticFrictionScale.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_seclinearaxisstaticfrictionscale]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_seclinearaxisstaticfrictionscale] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_seclinearaxisstaticfrictionscale]);
            }
        }
    }

    N = propMaterial_vortexPrimaryAngularAxisStaticFrictionScale.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primangularaxisstaticfrictionscale]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primangularaxisstaticfrictionscale] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primangularaxisstaticfrictionscale]);
            }
        }
    }

    N = propMaterial_vortexSecondaryAngularAxisStaticFrictionScale.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_secangularaxisstaticfrictionscale]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_secangularaxisstaticfrictionscale] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_secangularaxisstaticfrictionscale]);
            }
        }
    }

    N = propMaterial_vortexNormalAngularAxisStaticFrictionScale.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_normalangularaxisstaticfrictionscale]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_normalangularaxisstaticfrictionscale] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_normalangularaxisstaticfrictionscale]);
            }
        }
    }

    N = propMaterial_vortexCompliance.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_compliance]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_compliance] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_compliance]);
            }
        }
    }

    N = propMaterial_vortexDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_damping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_damping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_damping]);
            }
        }
    }

    N = propMaterial_vortexRestitution.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_restitution]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_restitution] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_restitution]);
            }
        }
    }

    N = propMaterial_vortexRestitutionThreshold.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_restitutionthreshold]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_restitutionthreshold] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_restitutionthreshold]);
            }
        }
    }

    N = propMaterial_vortexAdhesiveForce.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_adhesiveforce]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_adhesiveforce] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_adhesiveforce]);
            }
        }
    }

    N = propMaterial_vortexLinearVelocityDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_linearvelocitydamping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_linearvelocitydamping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_linearvelocitydamping]);
            }
        }
    }

    N = propMaterial_vortexAngularVelocityDamping.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_angularvelocitydamping]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_angularvelocitydamping] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_angularvelocitydamping]);
            }
        }
    }

    N = propMaterial_vortexPrimaryLinearAxisSlide.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primlinearaxisslide]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primlinearaxisslide] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primlinearaxisslide]);
            }
        }
    }

    N = propMaterial_vortexSecondaryLinearAxisSlide.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_seclinearaxisslide]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_seclinearaxisslide] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_seclinearaxisslide]);
            }
        }
    }

    N = propMaterial_vortexPrimaryAngularAxisSlide.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primangularaxisslide]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primangularaxisslide] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primangularaxisslide]);
            }
        }
    }

    N = propMaterial_vortexSecondaryAngularAxisSlide.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_secangularaxisslide]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_secangularaxisslide] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_secangularaxisslide]);
            }
        }
    }

    N = propMaterial_vortexNormalAngularAxisSlide.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_normalangularaxisslide]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_normalangularaxisslide] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_normalangularaxisslide]);
            }
        }
    }

    N = propMaterial_vortexPrimaryLinearAxisSlip.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primlinearaxisslip]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primlinearaxisslip] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primlinearaxisslip]);
            }
        }
    }

    N = propMaterial_vortexSecondaryLinearAxisSlip.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_seclinearaxisslip]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_seclinearaxisslip] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_seclinearaxisslip]);
            }
        }
    }

    N = propMaterial_vortexPrimaryAngularAxisSlip.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_primangularaxisslip]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_primangularaxisslip] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_primangularaxisslip]);
            }
        }
    }

    N = propMaterial_vortexSecondaryAngularAxisSlip.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_secangularaxisslip]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_secangularaxisslip] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_secangularaxisslip]);
            }
        }
    }

    N = propMaterial_vortexNormalAngularAxisSlip.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_normalangularaxisslip]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_normalangularaxisslip] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_normalangularaxisslip]);
            }
        }
    }

    N = propMaterial_vortexAutoSleepLinearSpeedThreshold.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_autosleeplinearspeedthreshold]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_autosleeplinearspeedthreshold] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_autosleeplinearspeedthreshold]);
            }
        }
    }

    N = propMaterial_vortexAutoSleepLinearAccelerationThreshold.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_autosleeplinearaccelthreshold]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_autosleeplinearaccelthreshold] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_autosleeplinearaccelthreshold]);
            }
        }
    }

    N = propMaterial_vortexAutoSleepAngularSpeedThreshold.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_autosleepangularspeedthreshold]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_autosleepangularspeedthreshold] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_autosleepangularspeedthreshold]);
            }
        }
    }

    N = propMaterial_vortexAutoSleepAngularAccelerationThreshold.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_autosleepangularaccelthreshold]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_autosleepangularaccelthreshold] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_autosleepangularaccelthreshold]);
            }
        }
    }

    N = propMaterial_vortexSkinThickness.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_skinthickness]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_skinthickness] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_skinthickness]);
            }
        }
    }

    N = propMaterial_vortexAutoAngularDampingTensionRatio.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio]);
            }
        }
    }

    N = propMaterial_newtonStaticFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _newtonFloatParams[simi_newton_body_staticfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _newtonFloatParams[simi_newton_body_staticfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _newtonFloatParams[simi_newton_body_staticfriction]);
            }
        }
    }

    N = propMaterial_newtonKineticFriction.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _newtonFloatParams[simi_newton_body_kineticfriction]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _newtonFloatParams[simi_newton_body_kineticfriction] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _newtonFloatParams[simi_newton_body_kineticfriction]);
            }
        }
    }

    N = propMaterial_newtonRestitution.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _newtonFloatParams[simi_newton_body_restitution]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _newtonFloatParams[simi_newton_body_restitution] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _newtonFloatParams[simi_newton_body_restitution]);
            }
        }
    }

    N = propMaterial_newtonLinearDrag.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _newtonFloatParams[simi_newton_body_lineardrag]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _newtonFloatParams[simi_newton_body_lineardrag] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _newtonFloatParams[simi_newton_body_lineardrag]);
            }
        }
    }

    N = propMaterial_newtonAngularDrag.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _newtonFloatParams[simi_newton_body_angulardrag]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _newtonFloatParams[simi_newton_body_angulardrag] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _newtonFloatParams[simi_newton_body_angulardrag]);
            }
        }
    }

    N = propMaterial_mujocoSolmix.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _mujocoFloatParams[simi_mujoco_body_solmix]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _mujocoFloatParams[simi_mujoco_body_solmix] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _mujocoFloatParams[simi_mujoco_body_solmix]);
            }
        }
    }

    N = propMaterial_mujocoMargin.name;
    if ( (pName == nullptr)  || (N == pName) )
    {
        retVal = 1;
        if ( (pState != _mujocoFloatParams[simi_mujoco_body_margin]) ||(pName == nullptr) )
        {
            if (pName != nullptr)
                _mujocoFloatParams[simi_mujoco_body_margin] = pState;
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDouble(N.c_str(), _mujocoFloatParams[simi_mujoco_body_margin]);
            }
        }
    }

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getFloatProperty(const char* pName, double& pState)
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

    return retVal;
}

int CDynMaterialObject::setVector3Property(const char* pName, const C3Vector* pState, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    N = propMaterial_vortexPrimaryAxisVector.name;
    if ( (pName == nullptr) || (N == pName) )
    {
        retVal = 1;
        C3Vector current(_vortexFloatParams.data() + simi_vortex_body_primaxisvectorx);
        if ( (pName == nullptr) || (current != pState[0]) )
        {
            if (pName != nullptr)
            {
                current = pState[0];
                current.normalize();
                current.getData(_vortexFloatParams.data() + simi_vortex_body_primaxisvectorx);
            }
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDoubleArray(N.c_str(), _vortexFloatParams.data() + simi_vortex_body_primaxisvectorx, 3);
            }
        }
    }

    N = propMaterial_mujocoFriction.name;
    if ( (pName == nullptr) || (N == pName) )
    {
        retVal = 1;
        C3Vector current(_mujocoFloatParams.data() + simi_mujoco_body_friction1);
        if ( (pName == nullptr) || (current != pState[0]) )
        {
            if (pName != nullptr)
            {
                current = pState[0];
                current.normalize();
                current.getData(_mujocoFloatParams.data() + simi_mujoco_body_friction1);
            }
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDoubleArray(N.c_str(), _mujocoFloatParams.data() + simi_mujoco_body_friction1, 3);
            }
        }
    }

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getVector3Property(const char* pName, C3Vector* pState)
{
    int retVal = -1;

    if (strcmp(pName, propMaterial_vortexPrimaryAxisVector.name) == 0)
    {
        retVal = 1;
        pState->setData(_vortexFloatParams.data() + simi_vortex_body_primaxisvectorx);
    }
    else if (strcmp(pName, propMaterial_mujocoFriction.name) == 0)
    {
        retVal = 1;
        pState->setData(_mujocoFloatParams.data() + simi_mujoco_body_friction1);
    }

    return retVal;
}

int CDynMaterialObject::setVectorProperty(const char* pName, const double* v, int vL, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;
    std::string N;
    CCbor* ev = nullptr;
    if (eev != nullptr)
        ev = eev;

    N = propMaterial_mujocoSolref.name;
    if ( (pName == nullptr) || (N == pName) )
    {
        retVal = 1;
        if ( (pName == nullptr) || (_mujocoFloatParams[simi_mujoco_body_solref1 + 0] != v[0]) || ((vL > 1) && (_mujocoFloatParams[simi_mujoco_body_solref1 + 1] != v[1])) )
        {
            if (pName != nullptr)
            {
                _mujocoFloatParams[simi_mujoco_body_solref1 + 0] = v[0];
                if (vL > 1)
                    _mujocoFloatParams[simi_mujoco_body_solref1 + 1] = v[1];
            }
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDoubleArray(N.c_str(), _mujocoFloatParams.data() + simi_mujoco_body_solref1, 2);
            }
        }
    }

    N = propMaterial_mujocoSolimp.name;
    if ( (pName == nullptr) || (N == pName) )
    {
        retVal = 1;
        if ( (pName == nullptr) || (_mujocoFloatParams[simi_mujoco_body_solimp1 + 0] != v[0]) || ((vL > 1) && (_mujocoFloatParams[simi_mujoco_body_solimp1 + 1] != v[1])) || ((vL > 2) && (_mujocoFloatParams[simi_mujoco_body_solimp1 + 2] != v[2])) || ((vL > 3) && (_mujocoFloatParams[simi_mujoco_body_solimp1 + 3] != v[3])) || ((vL > 4) && (_mujocoFloatParams[simi_mujoco_body_solimp1 + 4] != v[4])) )
        {
            if (pName != nullptr)
            {
                _mujocoFloatParams[simi_mujoco_body_solimp1 + 0] = v[0];
                if (vL > 1)
                    _mujocoFloatParams[simi_mujoco_body_solimp1 + 1] = v[1];
                if (vL > 2)
                    _mujocoFloatParams[simi_mujoco_body_solimp1 + 2] = v[2];
                if (vL > 3)
                    _mujocoFloatParams[simi_mujoco_body_solimp1 + 3] = v[3];
                if (vL > 4)
                    _mujocoFloatParams[simi_mujoco_body_solimp1 + 4] = v[4];
            }
            if (_shapeHandleForEvents != -1)
            {
                if (ev == nullptr)
                    ev = App::worldContainer->createObjectChangedEvent(_shapeHandleForEvents, "mat", false);

                ev->appendKeyDoubleArray(N.c_str(), _mujocoFloatParams.data() + simi_mujoco_body_solimp1, 5);
            }
        }
    }

    if ( (ev != nullptr) && (eev == nullptr) )
        App::worldContainer->pushEvent();
    return retVal;
}

int CDynMaterialObject::getVectorProperty(const char* pName, std::vector<double>& pState)
{
    int retVal = -1;

    if (strcmp(pName, propMaterial_mujocoSolref.name) == 0)
    {
        retVal = 1;
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solref1 + 0]);
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solref1 + 1]);
    }
    else if (strcmp(pName, propMaterial_mujocoSolimp.name) == 0)
    {
        retVal = 1;
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solimp1 + 0]);
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solimp1 + 1]);
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solimp1 + 2]);
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solimp1 + 3]);
        pState.push_back(_mujocoFloatParams[simi_mujoco_body_solimp1 + 4]);
    }

    return retVal;
}

int CDynMaterialObject::setIntVectorProperty(const char* pName, const int* v, int vL, CCbor* eev/* = nullptr*/)
{
    int retVal = -1;

    return retVal;
}

int CDynMaterialObject::getIntVectorProperty(const char* pName, std::vector<int>& pState)
{
    int retVal = -1;

    return retVal;
}

int CDynMaterialObject::getPropertyName(int& index, std::string& pName)
{
    return getPropertyName_static(index, pName);
}

int CDynMaterialObject::getPropertyName_static(int& index, std::string& pName)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_material.size(); i++)
    {
        index--;
        if (index == -1)
        {
            pName = allProps_material[i].name;
            retVal = 1;
            break;
        }
    }
    return retVal;
}

int CDynMaterialObject::getPropertyInfo(const char* pName, int& info, int& size)
{
    return getPropertyInfo_static(pName, info, size);
}

int CDynMaterialObject::getPropertyInfo_static(const char* pName, int& info, int& size)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_material.size(); i++)
    {
        if (strcmp(allProps_material[i].name, pName) == 0)
        {
            retVal = allProps_material[i].type;
            info = allProps_material[i].flags;
            size = 0;
            break;
        }
    }
    return retVal;
}

