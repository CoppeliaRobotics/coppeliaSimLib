
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "dynMaterialObject.h"
#include "app.h"
#include "tt.h"
#include <boost/format.hpp>

int CDynMaterialObject::_nextUniqueID=0;

CDynMaterialObject::CDynMaterialObject()
{
    _setDefaultParameters();
}

void CDynMaterialObject::_setDefaultParameters()
{
    _objectID=SIM_IDSTART_DYNMATERIAL_OLD;
    _objectName="usr_";

    _uniqueID=_nextUniqueID++;
    _bulletFloatParams.clear();
    _bulletIntParams.clear();
    _odeFloatParams.clear();
    _odeIntParams.clear();
    _vortexFloatParams.clear();
    _vortexIntParams.clear();
    _newtonFloatParams.clear();
    _newtonIntParams.clear();

    // Bullet parameters:
    // ----------------------------------------------------
    _bulletFloatParams.push_back(0.0f); // simi_bullet_body_restitution
    _bulletFloatParams.push_back(0.71f); // simi_bullet_body_oldfriction
    _bulletFloatParams.push_back(0.71f); // simi_bullet_body_friction
    _bulletFloatParams.push_back(0.0f); // simi_bullet_body_lineardamping
    _bulletFloatParams.push_back(0.0f); // simi_bullet_body_angulardamping
    _bulletFloatParams.push_back(0.1f); // simi_bullet_body_nondefaultcollisionmargingfactor
    _bulletFloatParams.push_back(0.002f); // simi_bullet_body_nondefaultcollisionmargingfactorconvex
    _bulletFloatParams.push_back(0.0f); // free
    // BULLET_SHAPE_FLOAT_PARAM_CNT_CURRENT=8

    int bulletBitCoded=simi_bullet_body_usenondefaultcollisionmarginconvex;
    // above is bit-coded: 1=_bulletStickyContact, 2=_bulletNonDefaultCollisionMargin, 4=_bulletNonDefaultCollisionMargin_forConvexAndNonPureShape, 8=_bulletAutoShrinkConvexMesh
    _bulletIntParams.push_back(bulletBitCoded); // simi_bullet_body_bitcoded
    _bulletIntParams.push_back(0); // free
    // BULLET_SHAPE_INT_PARAM_CNT_CURRENT=2
    // ----------------------------------------------------

    // ODE parameters:
    // ----------------------------------------------------
    _odeFloatParams.push_back(0.71f); // simi_ode_body_friction (box on 26.8deg inclined plane slightly slides (same mat. for box and plane)
    _odeFloatParams.push_back(0.2f); // simi_ode_body_softerp
    _odeFloatParams.push_back(0.0f); // simi_ode_body_softcfm
    _odeFloatParams.push_back(0.0f); // simi_ode_body_lineardamping
    _odeFloatParams.push_back(0.0f); // simi_ode_body_angulardamping
    _odeFloatParams.push_back(0.0f); // free
    // ODE_SHAPE_FLOAT_PARAM_CNT_CURRENT=6


    _odeIntParams.push_back(64); // simi_ode_body_maxcontacts
    int odeBitCoded=0; // not used for now
    _odeIntParams.push_back(odeBitCoded);
    _odeIntParams.push_back(0); // free
    // ODE_SHAPE_INT_PARAM_CNT_CURRENT=3
    // ----------------------------------------------------

    // Vortex parameters:
    // ----------------------------------------------------
    _vortexFloatParams.push_back(0.5f); // simi_vortex_body_primlinearaxisfriction
    _vortexFloatParams.push_back(0.5f); // simi_vortex_body_seclinearaxisfriction
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primangularaxisfriction
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_secangularaxisfriction
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_normalangularaxisfriction

    _vortexFloatParams.push_back(1.1f); // simi_vortex_body_primlinearaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.1f); // simi_vortex_body_seclinearaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.0f); // simi_vortex_body_primangularaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.0f); // simi_vortex_body_secangularaxisstaticfrictionscale
    _vortexFloatParams.push_back(1.0f); // simi_vortex_body_normalangularaxisstaticfrictionscale

    _vortexFloatParams.push_back(0.00000001f); // simi_vortex_body_compliance
    _vortexFloatParams.push_back(10000000.0f); // simi_vortex_body_damping (was 0.0f)
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_restitution
    _vortexFloatParams.push_back(0.5f); // simi_vortex_body_restitutionthreshold
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_adhesiveforce
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_linearvelocitydamping
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_angularvelocitydamping

    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primlinearaxisslide
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_seclinearaxisslide
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primangularaxisslide
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_secangularaxisslide
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_normalangularaxisslide

    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primlinearaxisslip
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_seclinearaxisslip
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primangularaxisslip
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_secangularaxisslip
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_normalangularaxisslip

    _vortexFloatParams.push_back(0.14f); // simi_vortex_body_autosleeplinearspeedthreshold
    _vortexFloatParams.push_back(0.045f); // simi_vortex_body_autosleeplinearaccelthreshold
    _vortexFloatParams.push_back(0.03f); // simi_vortex_body_autosleepangularspeedthreshold
    _vortexFloatParams.push_back(0.045f); // simi_vortex_body_autosleepangularaccelthreshold

    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_skinthickness (was 0.002 before)
    _vortexFloatParams.push_back(0.01f); // simi_vortex_body_autoangulardampingtensionratio
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primaxisvectorx
    _vortexFloatParams.push_back(0.0f); // simi_vortex_body_primaxisvectory
    _vortexFloatParams.push_back(1.0f); // simi_vortex_body_primaxisvectorz --> by default, primary axis aligned with Z axis (if possible)
    // VORTEX_SHAPE_FLOAT_PARAM_CNT_CURRENT=36

    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_scaledboxfast); // simi_vortex_body_primlinearaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_scaledboxfast); // simi_vortex_body_seclinearaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_none); // simi_vortex_body_primangulararaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_none); // simi_vortex_body_secmangulararaxisfrictionmodel
    _vortexIntParams.push_back(sim_vortex_bodyfrictionmodel_none); // simi_vortex_body_normalmangulararaxisfrictionmodel

    _vortexIntParams.push_back(simi_vortex_body_fastmoving+simi_vortex_body_seclinaxissameasprimlinaxis+simi_vortex_body_secangaxissameasprimangaxis+simi_vortex_body_normangaxissameasprimangaxis); // simi_vortex_body_bitcoded
                                                // bit coded: 0set=treat pure shapes as convex shapes, 1set=treat convex shapes as random shapes, 2set=threat random shapes as terrain, 3set=fast moving, 4set=auto-slip,
                                                // 5set=sec. lin. axis uses same values as prim. lin. axis, 6set=sec. ang. axis uses same values as prim. ang. axis, 7set=norm. ang. axis uses same values as prim. ang. axis (bits 5-7 are not taken into account on the plugin side: they just serve as setting memory)
                                                // 8set=auto angular damping is on
    _vortexIntParams.push_back(10); // simi_vortex_body_autosleepsteplivethreshold
    _vortexIntParams.push_back(_uniqueID); // simi_vortex_body_materialuniqueid: the material unique ID. The ID is redefined in each session
    // VORTEX_SHAPE_INT_PARAM_CNT_CURRENT=8
    // ----------------------------------------------------

    // Newton parameters:
    // ----------------------------------------------------
    _newtonFloatParams.push_back(0.5f); // simi_newton_body_staticfriction
    _newtonFloatParams.push_back(0.5f); // simi_newton_body_kineticfriction
    _newtonFloatParams.push_back(0.0f); // simi_newton_body_restitution
    _newtonFloatParams.push_back(0.0f); // simi_newton_body_lineardrag
    _newtonFloatParams.push_back(0.0f); // simi_newton_body_angulardrag
    // NEWTON_SHAPE_FLOAT_PARAM_CNT_CURRENT=5

    int newtonBitCoded=0;
    // newtonBitCoded|=simi_newton_body_fastmoving; // for fast-moving objects
    _newtonIntParams.push_back(newtonBitCoded); // simi_newton_body_bitcoded
    // NEWTON_SHAPE_INT_PARAM_CNT_CURRENT=1
    // ----------------------------------------------------
}

CDynMaterialObject::~CDynMaterialObject()
{
}

void CDynMaterialObject::setObjectID(int newID)
{
    _objectID=newID;
}

int CDynMaterialObject::getObjectID()
{
    return(_objectID);
}

void CDynMaterialObject::setObjectName(const char* newName)
{
    _objectName=newName;
}

std::string CDynMaterialObject::getObjectName()
{
    return(getIndividualName());
}

std::string CDynMaterialObject::getIndividualName()
{
    return("<individual (i.e. not shared)>");
}

float CDynMaterialObject::getEngineFloatParam(int what,bool* ok)
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_body_float_start)&&(what<sim_bullet_body_float_end))
    {
        int w=what-sim_bullet_body_restitution+simi_bullet_body_restitution;
        return(_bulletFloatParams[w]);
    }
    if ((what>sim_ode_body_float_start)&&(what<sim_ode_body_float_end))
    {
        int w=what-sim_ode_body_friction+simi_ode_body_friction;
        return(_odeFloatParams[w]);
    }
    if ((what>sim_vortex_body_float_start)&&(what<sim_vortex_body_float_end))
    {
        int w=what-sim_vortex_body_primlinearaxisfriction+simi_vortex_body_primlinearaxisfriction;
        return(_vortexFloatParams[w]);
    }
    if ((what>sim_newton_body_float_start)&&(what<sim_newton_body_float_end))
    {
        int w=what-sim_newton_body_staticfriction+simi_newton_body_staticfriction;
        return(_newtonFloatParams[w]);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0.0);
}

int CDynMaterialObject::getEngineIntParam(int what,bool* ok)
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_body_int_start)&&(what<sim_bullet_body_int_end))
    {
        int w=what-sim_bullet_body_bitcoded+simi_bullet_body_bitcoded;
        return(_bulletIntParams[w]);
    }
    if ((what>sim_ode_body_int_start)&&(what<sim_ode_body_int_end))
    {
        int w=what-sim_ode_body_maxcontacts+simi_ode_body_maxcontacts;
        return(_odeIntParams[w]);
    }
    if ((what>sim_vortex_body_int_start)&&(what<sim_vortex_body_int_end))
    {
        int w=what-sim_vortex_body_primlinearaxisfrictionmodel+simi_vortex_body_primlinearaxisfrictionmodel;
        return(_vortexIntParams[w]);
    }
    if ((what>sim_newton_body_int_start)&&(what<sim_newton_body_int_end))
    {
        int w=what-sim_newton_body_bitcoded+simi_newton_body_bitcoded;
        return(_newtonIntParams[w]);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0);
}

bool CDynMaterialObject::getEngineBoolParam(int what,bool* ok)
{
    if (ok!=nullptr)
        ok[0]=true;
    if ((what>sim_bullet_body_bool_start)&&(what<sim_bullet_body_bool_end))
    {
        int b=1;
        int w=(what-sim_bullet_body_sticky);
        while (w>0) {b*=2; w--;}
        return((_bulletIntParams[simi_bullet_body_bitcoded]&b)!=0);
    }
    if ((what>sim_ode_body_bool_start)&&(what<sim_ode_body_bool_end))
    {
        // no bool params for now
        // search for obb11032016
        if (ok!=nullptr)
            ok[0]=false;
        return(0);
    }
    if ((what>sim_vortex_body_bool_start)&&(what<sim_vortex_body_bool_end))
    {
        int b=1;
        int w=(what-sim_vortex_body_pureshapesasconvex);
        while (w>0) {b*=2; w--;}
        return((_vortexIntParams[simi_vortex_body_bitcoded]&b)!=0);
    }
    if ((what>sim_newton_body_bool_start)&&(what<sim_newton_body_bool_end))
    {
        int b=1;
        int w=(what-sim_newton_body_fastmoving);
        while (w>0) {b*=2; w--;}
        return((_newtonIntParams[simi_newton_body_bitcoded]&b)!=0);
    }
    if (ok!=nullptr)
        ok[0]=false;
    return(0);
}

bool CDynMaterialObject::setEngineFloatParam(int what,float v)
{
    if ((what>sim_bullet_body_float_start)&&(what<sim_bullet_body_float_end))
    {
        int w=what-sim_bullet_body_restitution+simi_bullet_body_restitution;
        std::vector<float> fp;
        getBulletFloatParams(fp);
        fp[w]=v;
        setBulletFloatParams(fp);
        return(true);
    }
    if ((what>sim_ode_body_float_start)&&(what<sim_ode_body_float_end))
    {
        int w=what-sim_ode_body_friction+simi_ode_body_friction;
        std::vector<float> fp;
        getOdeFloatParams(fp);
        fp[w]=v;
        setOdeFloatParams(fp);
        return(true);
    }
    if ((what>sim_vortex_body_float_start)&&(what<sim_vortex_body_float_end))
    {
        int w=what-sim_vortex_body_primlinearaxisfriction+simi_vortex_body_primlinearaxisfriction;
        std::vector<float> fp;
        getVortexFloatParams(fp);
        fp[w]=v;
        setVortexFloatParams(fp);
        return(true);
    }
    if ((what>sim_newton_body_float_start)&&(what<sim_newton_body_float_end))
    {
        int w=what-sim_newton_body_staticfriction+simi_newton_body_staticfriction;
        std::vector<float> fp;
        getNewtonFloatParams(fp);
        fp[w]=v;
        setNewtonFloatParams(fp);
        return(true);
    }
    return(false);
}

bool CDynMaterialObject::setEngineIntParam(int what,int v)
{
    if ((what>sim_bullet_body_int_start)&&(what<sim_bullet_body_int_end))
    {
        int w=what-sim_bullet_body_bitcoded+simi_bullet_body_bitcoded;
        std::vector<int> ip;
        getBulletIntParams(ip);
        ip[w]=v;
        setBulletIntParams(ip);
        return(true);
    }
    if ((what>sim_ode_body_int_start)&&(what<sim_ode_body_int_end))
    {
        int w=what-sim_ode_body_maxcontacts+simi_ode_body_maxcontacts;
        std::vector<int> ip;
        getOdeIntParams(ip);
        ip[w]=v;
        setOdeIntParams(ip);
        return(true);
    }
    if ((what>sim_vortex_body_int_start)&&(what<sim_vortex_body_int_end))
    {
        int w=what-sim_vortex_body_primlinearaxisfrictionmodel+simi_vortex_body_primlinearaxisfrictionmodel;
        std::vector<int> ip;
        getVortexIntParams(ip);
        ip[w]=v;
        setVortexIntParams(ip);
        return(true);
    }
    if ((what>sim_newton_body_int_start)&&(what<sim_newton_body_int_end))
    {
        int w=what-sim_newton_body_bitcoded+simi_newton_body_bitcoded;
        std::vector<int> ip;
        getNewtonIntParams(ip);
        ip[w]=v;
        setNewtonIntParams(ip);
        return(true);
    }
    return(false);
}

bool CDynMaterialObject::setEngineBoolParam(int what,bool v)
{
    if ((what>sim_bullet_body_bool_start)&&(what<sim_bullet_body_bool_end))
    {
        int b=1;
        int w=(what-sim_bullet_body_sticky);
        while (w>0) {b*=2; w--;}
        _bulletIntParams[simi_bullet_body_bitcoded]|=b;
        if (!v)
            _bulletIntParams[simi_bullet_body_bitcoded]-=b;
        return(true);
    }
    if ((what>sim_ode_body_bool_start)&&(what<sim_ode_body_bool_end))
    {
        // no bool params yet
        // search for obb11032016
        return(false);
    }
    if ((what>sim_vortex_body_bool_start)&&(what<sim_vortex_body_bool_end))
    {
        int b=1;
        int w=(what-sim_vortex_body_pureshapesasconvex);
        while (w>0) {b*=2; w--;}
        _vortexIntParams[simi_vortex_body_bitcoded]|=b;
        if (!v)
            _vortexIntParams[simi_vortex_body_bitcoded]-=b;
        return(true);
    }
    if ((what>sim_newton_body_bool_start)&&(what<sim_newton_body_bool_end))
    {
        int b=1;
        int w=(what-sim_newton_body_fastmoving);
        while (w>0) {b*=2; w--;}
        _newtonIntParams[simi_newton_body_bitcoded]|=b;
        if (!v)
            _newtonIntParams[simi_newton_body_bitcoded]-=b;
        return(true);
    }
    return(false);
}

void CDynMaterialObject::getBulletFloatParams(std::vector<float>& p)
{
    p.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
}

void CDynMaterialObject::setBulletFloatParams(const std::vector<float>& p)
{
    for (size_t i=0;i<p.size();i++)
        _bulletFloatParams[i]=p[i];

    _bulletFloatParams[simi_bullet_body_restitution]=tt::getLimitedFloat(0.0f,10.0f,_bulletFloatParams[simi_bullet_body_restitution]); // restitution
    _bulletFloatParams[simi_bullet_body_oldfriction]=tt::getLimitedFloat(0.0f,1000.0f,_bulletFloatParams[simi_bullet_body_oldfriction]); // old friction
    _bulletFloatParams[simi_bullet_body_friction]=tt::getLimitedFloat(0.0f,1000.0f,_bulletFloatParams[simi_bullet_body_friction]); // new friction
    _bulletFloatParams[simi_bullet_body_lineardamping]=tt::getLimitedFloat(0.0f,10.0f,_bulletFloatParams[simi_bullet_body_lineardamping]); // lin. damp.
    _bulletFloatParams[simi_bullet_body_angulardamping]=tt::getLimitedFloat(0.0f,10.0f,_bulletFloatParams[simi_bullet_body_angulardamping]); // ang. damp.
    _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor]=tt::getLimitedFloat(0.0001f,1000.0f,_bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor]); // _bulletNonDefaultCollisionMarginFactor
    _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex]=tt::getLimitedFloat(0.0001f,1000.0f,_bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex]); // _bulletNonDefaultCollisionMarginFactor_forConvexAndNonPureShape
}

void CDynMaterialObject::getBulletIntParams(std::vector<int>& p)
{
    p.assign(_bulletIntParams.begin(),_bulletIntParams.end());
}

void CDynMaterialObject::setBulletIntParams(const std::vector<int>& p)
{
    for (size_t i=0;i<p.size();i++)
        _bulletIntParams[i]=p[i];
}


void CDynMaterialObject::getOdeFloatParams(std::vector<float>& p)
{
    p.assign(_odeFloatParams.begin(),_odeFloatParams.end());
}

void CDynMaterialObject::setOdeFloatParams(const std::vector<float>& p)
{
    for (size_t i=0;i<p.size();i++)
        _odeFloatParams[i]=p[i];

    _odeFloatParams[simi_ode_body_friction]=tt::getLimitedFloat(0.0f,1000.0f,_odeFloatParams[simi_ode_body_friction]); // friction
    _odeFloatParams[simi_ode_body_softerp]=tt::getLimitedFloat(0.0f,1.0f,_odeFloatParams[simi_ode_body_softerp]); // soft ERP
    _odeFloatParams[simi_ode_body_softcfm]=tt::getLimitedFloat(0.0f,1000000.0f,_odeFloatParams[simi_ode_body_softcfm]); // soft CFM
    _odeFloatParams[simi_ode_body_lineardamping]=tt::getLimitedFloat(0.0f,10.0f,_odeFloatParams[simi_ode_body_lineardamping]); // lin. damp.
    _odeFloatParams[simi_ode_body_angulardamping]=tt::getLimitedFloat(0.0f,10.0f,_odeFloatParams[simi_ode_body_angulardamping]); // ang. damp.
}

void CDynMaterialObject::getOdeIntParams(std::vector<int>& p)
{
    p.assign(_odeIntParams.begin(),_odeIntParams.end());
}

void CDynMaterialObject::setOdeIntParams(const std::vector<int>& p)
{
    for (size_t i=0;i<p.size();i++)
        _odeIntParams[i]=p[i];
    _odeIntParams[0]=tt::getLimitedInt(-64,64,_odeIntParams[simi_ode_body_maxcontacts]); // max contacts
}


void CDynMaterialObject::getVortexFloatParams(std::vector<float>& p)
{
    p.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
}

void CDynMaterialObject::setVortexFloatParams(const std::vector<float>& p)
{
    int l=int(p.size());
    if (l>int(_vortexFloatParams.size()))
        l=int(_vortexFloatParams.size()); // should normally never happen!
    for (int i=0;i<l;i++)
        _vortexFloatParams[i]=p[i];

    _vortexFloatParams[simi_vortex_body_skinthickness]=tt::getLimitedFloat(0.0f,10.0f,_vortexFloatParams[simi_vortex_body_skinthickness]); // skin thickness
    _vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio]=tt::getLimitedFloat(0.0f,1000.0f,_vortexFloatParams[simi_vortex_body_autoangulardampingtensionratio]); // auto angular damping tension ratio
    // make sure the vector for the primary axis is normalized:
    C3Vector v(_vortexFloatParams[simi_vortex_body_primaxisvectorx],_vortexFloatParams[simi_vortex_body_primaxisvectory],_vortexFloatParams[simi_vortex_body_primaxisvectorz]);
    if (v.getLength()<0.01f)
        v=C3Vector::unitZVector;
    v.normalize();
    _vortexFloatParams[simi_vortex_body_primaxisvectorx]=v(0);
    _vortexFloatParams[simi_vortex_body_primaxisvectory]=v(1);
    _vortexFloatParams[simi_vortex_body_primaxisvectorz]=v(2);
}

void CDynMaterialObject::getVortexIntParams(std::vector<int>& p)
{
    _vortexIntParams[7]=_uniqueID;
    p.assign(_vortexIntParams.begin(),_vortexIntParams.end());
}

void CDynMaterialObject::setVortexIntParams(const std::vector<int>& p)
{
    int l=int(p.size());
    if (l>int(_vortexIntParams.size()))
        l=int(_vortexIntParams.size()); // should normally never happen!
    for (int i=0;i<l;i++)
        _vortexIntParams[i]=p[i];

    _vortexIntParams[simi_vortex_body_autosleepsteplivethreshold]=tt::getLimitedInt(1,10000,_vortexIntParams[simi_vortex_body_autosleepsteplivethreshold]); // auto-sleep steps
    _vortexIntParams[simi_vortex_body_materialuniqueid]=_uniqueID;
}

void CDynMaterialObject::getNewtonFloatParams(std::vector<float>& p)
{
    p.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
}

void CDynMaterialObject::setNewtonFloatParams(const std::vector<float>& p)
{
    int l=int(p.size());
    if (l>int(_newtonFloatParams.size()))
        l=int(_newtonFloatParams.size()); // should normally never happen!
    for (int i=0;i<l;i++)
        _newtonFloatParams[i]=p[i];
    _newtonFloatParams[simi_newton_body_staticfriction]=tt::getLimitedFloat(0.0f,2.0f,_newtonFloatParams[simi_newton_body_staticfriction]); // static friction
    _newtonFloatParams[simi_newton_body_kineticfriction]=tt::getLimitedFloat(0.0f,_newtonFloatParams[simi_newton_body_staticfriction],_newtonFloatParams[simi_newton_body_kineticfriction]); // kin. friction
    _newtonFloatParams[simi_newton_body_restitution]=tt::getLimitedFloat(0.0f,1.0f,_newtonFloatParams[simi_newton_body_restitution]); // restitution
    _newtonFloatParams[simi_newton_body_lineardrag]=tt::getLimitedFloat(0.0f,1.0f,_newtonFloatParams[simi_newton_body_lineardrag]); // lin. drag
    _newtonFloatParams[simi_newton_body_angulardrag]=tt::getLimitedFloat(0.0f,1.0f,_newtonFloatParams[simi_newton_body_angulardrag]); // ang. drag
}

void CDynMaterialObject::getNewtonIntParams(std::vector<int>& p)
{
    p.assign(_newtonIntParams.begin(),_newtonIntParams.end());
}

void CDynMaterialObject::setNewtonIntParams(const std::vector<int>& p)
{
    int l=int(p.size());
    if (l>int(_newtonIntParams.size()))
        l=int(_newtonIntParams.size()); // should normally never happen!
    for (int i=0;i<l;i++)
        _newtonIntParams[i]=p[i];
}

std::string CDynMaterialObject::getDefaultMaterialName(int defMatId)
{
    if (defMatId==sim_dynmat_default)
        return("defaultMaterial");
    if (defMatId==sim_dynmat_highfriction)
        return("highFrictionMaterial");
    if (defMatId==sim_dynmat_lowfriction)
        return("lowFrictionMaterial");
    if (defMatId==sim_dynmat_nofriction)
        return("noFrictionMaterial");
    if (defMatId==sim_dynmat_reststackgrasp)
        return("restStackGraspMaterial");
    if (defMatId==sim_dynmat_foot)
        return("footMaterial");
    if (defMatId==sim_dynmat_wheel)
        return("wheelMaterial");
    if (defMatId==sim_dynmat_gripper)
        return("gripperMaterial");
    if (defMatId==sim_dynmat_floor)
        return("floorMaterial");
    return("ERROR");
}
void CDynMaterialObject::generateDefaultMaterial(int defMatId)
{
    _setDefaultParameters();
    if (defMatId==sim_dynmat_default)
        _objectName="defaultMaterial";
    if (defMatId==sim_dynmat_highfriction)
    {
        _objectName="highFrictionMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
        setEngineFloatParam(sim_bullet_body_friction,1.0f);
        setEngineFloatParam(sim_ode_body_friction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,1.0f);
        setEngineFloatParam(sim_newton_body_staticfriction,2.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,2.0f);
    }
    if (defMatId==sim_dynmat_lowfriction)
    {
        _objectName="lowFrictionMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,0.41f);
        setEngineFloatParam(sim_bullet_body_friction,0.20f);
        setEngineFloatParam(sim_ode_body_friction,0.41f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,0.17f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,0.17f);
        setEngineFloatParam(sim_newton_body_staticfriction,0.17f);
        setEngineFloatParam(sim_newton_body_kineticfriction,0.17f);
    }
    if (defMatId==sim_dynmat_nofriction)
    {
        _objectName="noFrictionMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,0.0f);
        setEngineFloatParam(sim_bullet_body_friction,0.0f);
        setEngineFloatParam(sim_ode_body_friction,0.0f);
        setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,sim_vortex_bodyfrictionmodel_none);
        setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,sim_vortex_bodyfrictionmodel_none);
        setEngineFloatParam(sim_newton_body_staticfriction,0.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,0.0f);
    }
    if (defMatId==sim_dynmat_reststackgrasp)
    {
        _objectName="restStackGraspMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
        setEngineFloatParam(sim_bullet_body_friction,0.5f);
        setEngineFloatParam(sim_ode_body_friction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_skinthickness,0.002f);
        setEngineBoolParam(sim_vortex_body_autoangulardamping,true);
        setEngineFloatParam(sim_newton_body_staticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_restitution,0.0f);
    }
    if (defMatId==sim_dynmat_foot)
    {
        _objectName="footMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
        setEngineFloatParam(sim_bullet_body_friction,0.5f);
        setEngineFloatParam(sim_ode_body_friction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_skinthickness,0.002f);
        setEngineFloatParam(sim_newton_body_staticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_restitution,0.0f);
    }
    if (defMatId==sim_dynmat_wheel)
    {
        _objectName="wheelMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
        setEngineFloatParam(sim_bullet_body_friction,1.0f);
        setEngineFloatParam(sim_ode_body_friction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_skinthickness,0.0f);
        setEngineFloatParam(sim_newton_body_staticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_restitution,0.0f);
    }
    if (defMatId==sim_dynmat_gripper)
    {
        _objectName="gripperMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
        setEngineFloatParam(sim_bullet_body_friction,0.5f);
        setEngineFloatParam(sim_ode_body_friction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_skinthickness,0.002f);
        setEngineFloatParam(sim_newton_body_staticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_restitution,0.0f);
    }
    if (defMatId==sim_dynmat_floor)
    {
        _objectName="floorMaterial";
        setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
        setEngineFloatParam(sim_bullet_body_friction,1.0f);
        setEngineFloatParam(sim_ode_body_friction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,1.0f);
        setEngineFloatParam(sim_vortex_body_primlinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_seclinearaxisslip,0.0f);
        setEngineFloatParam(sim_vortex_body_skinthickness,0.002f);
        setEngineFloatParam(sim_newton_body_staticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_kineticfriction,1.0f);
        setEngineFloatParam(sim_newton_body_restitution,0.0f);
    }
/*
    setObjectName("bulletMaterial_sticky_special"); // Bullet sticky contact
    setEngineFloatParam(sim_bullet_body_oldfriction,1.0f);
    setEngineFloatParam(sim_bullet_body_friction,0.25f);
    setEngineBoolParam(sim_bullet_body_sticky,true);

    setObjectName("vortexMaterial_axisXWithoutFriction");
    setEngineFloatParam(sim_vortex_body_primaxisvectorx,1.0f);
    setEngineFloatParam(sim_vortex_body_primaxisvectory,0.0f);
    setEngineFloatParam(sim_vortex_body_primaxisvectorz,0.0f);
    setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,false);
    setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,sim_vortex_bodyfrictionmodel_none);

    setObjectName("vortexMaterial_axisYWithoutFriction");
    setEngineFloatParam(sim_vortex_body_primaxisvectorx,0.0f);
    setEngineFloatParam(sim_vortex_body_primaxisvectory,1.0f);
    setEngineFloatParam(sim_vortex_body_primaxisvectorz,0.0f);
    setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,false);
    setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,sim_vortex_bodyfrictionmodel_none);

    setObjectName("vortexMaterial_axisZWithoutFriction");
    setEngineFloatParam(sim_vortex_body_primaxisvectorx,0.0f);
    setEngineFloatParam(sim_vortex_body_primaxisvectory,0.0f);
    setEngineFloatParam(sim_vortex_body_primaxisvectorz,1.0f);
    setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,false);
    setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,sim_vortex_bodyfrictionmodel_none);

    setObjectName("vortexMaterial_pureShapesAsVxConvexMesh");
    setEngineBoolParam(sim_vortex_body_pureshapesasconvex,true);

    setObjectName("vortexMaterial_convexShapesAsVxTriangleMeshBVTree");
    setEngineBoolParam(sim_vortex_body_convexshapesasrandom,true);

    setObjectName("vortexMaterial_randomShapesAsVxTriangleMeshUVGrid");
    setEngineBoolParam(sim_vortex_body_randomshapesasterrain,true);
*/
}

CDynMaterialObject* CDynMaterialObject::copyYourself()
{
    CDynMaterialObject* newObj=new CDynMaterialObject();
    newObj->_objectID=_objectID;
    newObj->_objectName=_objectName;
    newObj->_bulletFloatParams.assign(_bulletFloatParams.begin(),_bulletFloatParams.end());
    newObj->_bulletIntParams.assign(_bulletIntParams.begin(),_bulletIntParams.end());

    newObj->_odeFloatParams.assign(_odeFloatParams.begin(),_odeFloatParams.end());
    newObj->_odeIntParams.assign(_odeIntParams.begin(),_odeIntParams.end());

    newObj->_vortexFloatParams.assign(_vortexFloatParams.begin(),_vortexFloatParams.end());
    newObj->_vortexIntParams.assign(_vortexIntParams.begin(),_vortexIntParams.end());
    newObj->_vortexIntParams[7]=newObj->_uniqueID;

    newObj->_newtonFloatParams.assign(_newtonFloatParams.begin(),_newtonFloatParams.end());
    newObj->_newtonIntParams.assign(_newtonIntParams.begin(),_newtonIntParams.end());

    return(newObj);
}

void CDynMaterialObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Ipa");
            ar << _objectID;
            ar.flush();

            ar.storeDataName("Gon");
            ar << _objectName;
            ar.flush();

            ar.storeDataName("Bcv");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,true); // for backward compatibility (29/10/2016)
            ar << dummy;
            ar.flush();

            ar.storeDataName("Bul"); // keep for file write backw. compat. (09/03/2016)
            // ar << _bulletRestitution << _bulletFriction << _bulletLinearDamping << _bulletAngularDamping << _bulletNonDefaultCollisionMarginFactor << _bulletNonDefaultCollisionMarginFactor_forConvexAndNonPureShape;
            ar << _bulletFloatParams[simi_bullet_body_restitution] << _bulletFloatParams[simi_bullet_body_oldfriction] << _bulletFloatParams[simi_bullet_body_lineardamping] << _bulletFloatParams[simi_bullet_body_angulardamping] << _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor] << _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex];
            ar.flush();

            ar.storeDataName("Od2"); // keep for file write backw. compat. (09/03/2016)
            // ar << _odeMaxContacts << _odeFriction << _odeSoftERP << _odeSoftCFM << _odeLinearDamping << _odeAngularDamping;
            ar << _odeIntParams[simi_ode_body_maxcontacts] << _odeFloatParams[simi_ode_body_friction] << _odeFloatParams[simi_ode_body_softerp] << _odeFloatParams[simi_ode_body_softcfm] << _odeFloatParams[simi_ode_body_lineardamping] << _odeFloatParams[simi_ode_body_angulardamping];
            ar.flush();

            ar.storeDataName("Vo4"); // vortex params:
            ar << int(_vortexFloatParams.size()) << int(_vortexIntParams.size());
            for (int i=0;i<int(_vortexFloatParams.size());i++)
                ar << _vortexFloatParams[i];
            for (int i=0;i<int(_vortexIntParams.size());i++)
                ar << _vortexIntParams[i];
            ar.flush();

            ar.storeDataName("Nw1"); // newton params:
            ar << int(_newtonFloatParams.size()) << int(_newtonIntParams.size());
            for (int i=0;i<int(_newtonFloatParams.size());i++)
                ar << _newtonFloatParams[i];
            for (int i=0;i<int(_newtonIntParams.size());i++)
                ar << _newtonIntParams[i];
            ar.flush();

            ar.storeDataName("Var"); // keep for file write backw. compat. (09/03/2016)
            unsigned char nothing=0;
            if (_bulletIntParams[simi_bullet_body_bitcoded]&simi_bullet_body_usenondefaultcollisionmargin)
                nothing|=1;
            if (_bulletIntParams[simi_bullet_body_bitcoded]&simi_bullet_body_sticky)
                nothing|=2;
            if (_bulletIntParams[simi_bullet_body_bitcoded]&simi_bullet_body_autoshrinkconvex)
                nothing|=4;
            if (_bulletIntParams[simi_bullet_body_bitcoded]&simi_bullet_body_usenondefaultcollisionmarginconvex)
                nothing|=8;
            ar << nothing;
            ar.flush();

            ar.storeDataName("BuN"); // Bullet params, keep after "Bul" and "Var"
            ar << int(_bulletFloatParams.size()) << int(_bulletIntParams.size());
            for (int i=0;i<int(_bulletFloatParams.size());i++)
                ar << _bulletFloatParams[i];
            for (int i=0;i<int(_bulletIntParams.size());i++)
                ar << _bulletIntParams[i];
            ar.flush();

            ar.storeDataName("OdN"); // Ode params, keep after "Od2"
            ar << int(_odeFloatParams.size()) << int(_odeIntParams.size());
            for (int i=0;i<int(_odeFloatParams.size());i++)
                ar << _odeFloatParams[i];
            for (int i=0;i<int(_odeIntParams.size());i++)
                ar << _odeIntParams[i];
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            bool vortexDataLoaded=false;
            bool newtonDataLoaded=false;
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Ipa")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectID;
                    }
                    if (theName.compare("Gon")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _objectName;
                    }
                    if (theName.compare("Bcv")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        // _isIndividualTag=SIM_IS_BIT_SET(dummy,0);
                    }
                    if (theName.compare("Bul")==0)
                    { // keep for backw. compat. (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _bulletRestitution >> _bulletFriction >> _bulletLinearDamping >> _bulletAngularDamping >> _bulletNonDefaultCollisionMarginFactor >> _bulletNonDefaultCollisionMarginFactor_forConvexAndNonPureShape;
                        float fric;
                        ar >> _bulletFloatParams[simi_bullet_body_restitution] >> fric >> _bulletFloatParams[simi_bullet_body_lineardamping] >> _bulletFloatParams[simi_bullet_body_angulardamping] >> _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactor] >> _bulletFloatParams[simi_bullet_body_nondefaultcollisionmargingfactorconvex];
                        _bulletFloatParams[simi_bullet_body_oldfriction]=fric;
                        _bulletFloatParams[simi_bullet_body_friction]=tt::getLimitedFloat(0.0f,1.0f,fric); // sticky contacts have disappeared for the new Bullet, now everything is "sticky", so make sure it is not too sticky!
                    }
                    if (theName.compare("Ode")==0)
                    { // for backward compatibility (13/8/2015)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _odeMaxContacts >> _odeFriction >> _odeSoftERP >> _odeSoftCFM >> _odeLinearDamping >> _odeAngularDamping;
                        ar >> _odeIntParams[simi_ode_body_maxcontacts] >> _odeFloatParams[simi_ode_body_friction] >> _odeFloatParams[simi_ode_body_softerp] >> _odeFloatParams[simi_ode_body_softcfm] >> _odeFloatParams[simi_ode_body_lineardamping] >> _odeFloatParams[simi_ode_body_angulardamping];
                        _odeIntParams[0]=64;
                    }
                    if (theName.compare("Od2")==0)
                    { // keep for backw. compat. (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        // ar >> _odeMaxContacts >> _odeFriction >> _odeSoftERP >> _odeSoftCFM >> _odeLinearDamping >> _odeAngularDamping;
                        ar >> _odeIntParams[0] >> _odeFloatParams[0] >> _odeFloatParams[1] >> _odeFloatParams[2] >> _odeFloatParams[3] >> _odeFloatParams[4];
                    }
                    if (theName.compare("BuN")==0)
                    { // Bullet params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_bulletFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_bulletIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _bulletFloatParams already!
                            ar >> vf;
                            _bulletFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _bulletIntParams already!
                            ar >> vi;
                            _bulletIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("OdN")==0)
                    { // Ode params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_odeFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_odeIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _odeFloatParams already!
                            ar >> vf;
                            _odeFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _odeIntParams already!
                            ar >> vi;
                            _odeIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                    }
                    if (theName.compare("Vo4")==0)
                    { // vortex params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_vortexFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_vortexIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _vortexFloatParams already!
                            ar >> vf;
                            _vortexFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _vortexIntParams already!
                            ar >> vi;
                            _vortexIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                            // was there by mistake until 5/8/2015: _vortexIntParams[i]=vi;
                        }
                        vortexDataLoaded=true;
                    }
                    if (theName.compare("Nw1")==0)
                    { // newton params:
                        noHit=false;
                        ar >> byteQuantity;
                        int cnt1,cnt2;
                        ar >> cnt1 >> cnt2;

                        int cnt1_b=SIM_MIN(int(_newtonFloatParams.size()),cnt1);
                        int cnt2_b=SIM_MIN(int(_newtonIntParams.size()),cnt2);

                        float vf;
                        int vi;
                        for (int i=0;i<cnt1_b;i++)
                        { // new versions will always have same or more items in _newtonFloatParams already!
                            ar >> vf;
                            _newtonFloatParams[i]=vf;
                        }
                        for (int i=0;i<cnt1-cnt1_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vf;
                        }
                        for (int i=0;i<cnt2_b;i++)
                        { // new versions will always have same or more items in _newtonIntParams already!
                            ar >> vi;
                            _newtonIntParams[i]=vi;
                        }
                        for (int i=0;i<cnt2-cnt2_b;i++)
                        { // this serialization version is newer than what we know. Discard the unrecognized data:
                            ar >> vi;
                        }
                        newtonDataLoaded=true;
                    }
                    if (theName=="Var")
                    { // keep for backw. compat. (09/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;

                        if (nothing&1)
                            _bulletIntParams[simi_bullet_body_bitcoded]|=simi_bullet_body_usenondefaultcollisionmargin;
                        if (nothing&2)
                            _bulletIntParams[simi_bullet_body_bitcoded]|=simi_bullet_body_sticky;
                        if (nothing&4)
                            _bulletIntParams[simi_bullet_body_bitcoded]|=simi_bullet_body_autoshrinkconvex;
                        if (nothing&8)
                            _bulletIntParams[simi_bullet_body_bitcoded]|=simi_bullet_body_usenondefaultcollisionmarginconvex;
                        // Following is done after everything was loaded:
                        //  if (_bulletIntParams[simi_bullet_body_bitcoded]&simi_bullet_body_sticky)
                        //  _bulletFloatParams[simi_bullet_body_friction]=0.25f; // sticky contacts have disappeared for the new Bullet, we need to adjust for that
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (!vortexDataLoaded)
            { // keep for backward compatibility (16/10/2013)
                // Try to guess the friction we would need for Vortex:
                float averageFriction=0.0f;
                if (_bulletFloatParams[simi_bullet_body_oldfriction]>1.0f)
                    averageFriction+=1.0f;
                else
                    averageFriction+=_bulletFloatParams[simi_bullet_body_oldfriction];
                if (_odeFloatParams[simi_ode_body_friction]>1.0f)
                    averageFriction+=1.0f;
                else
                    averageFriction+=_odeFloatParams[simi_ode_body_friction];
                averageFriction*=0.5f;
                if (averageFriction<0.01f)
                {
                    _vortexIntParams[simi_vortex_body_primlinearaxisfrictionmodel]=sim_vortex_bodyfrictionmodel_none;
                    _vortexIntParams[simi_vortex_body_seclinearaxisfrictionmodel]=sim_vortex_bodyfrictionmodel_none;
                }
                else
                {
                    _vortexFloatParams[simi_vortex_body_primlinearaxisfriction]=averageFriction;
                    _vortexFloatParams[simi_vortex_body_seclinearaxisfriction]=averageFriction;
                }
            }
            if (!newtonDataLoaded)
            { // keep for backward compatibility (5/8/2015)
                // Try to guess the friction we would need for Newton:
                float averageFriction=0.0f;
                if (_bulletFloatParams[simi_bullet_body_oldfriction]>1.0f)
                    averageFriction+=1.0f;
                else
                    averageFriction+=_bulletFloatParams[simi_bullet_body_oldfriction];
                if (_odeFloatParams[simi_ode_body_friction]>1.0f)
                    averageFriction+=1.0f;
                else
                    averageFriction+=_odeFloatParams[simi_ode_body_friction];
                averageFriction*=0.5f;
                _newtonFloatParams[simi_newton_body_staticfriction]=averageFriction;
                _newtonFloatParams[simi_newton_body_kineticfriction]=averageFriction;
            }
        }
    }
}
