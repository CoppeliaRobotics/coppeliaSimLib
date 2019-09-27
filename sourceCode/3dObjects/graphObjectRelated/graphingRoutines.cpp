
#include "vrepMainHeader.h"
#include "graphingRoutines.h"
#include "v_rep_internal.h"
#include "app.h"
#include "tt.h"
#include "gV.h"

#include "v_repStrings.h"

CGraphingRoutines::CGraphingRoutines()
{
}

CGraphingRoutines::~CGraphingRoutines()
{
}

std::string CGraphingRoutines::getDataUnit(CGraphData* it)
{
    int dType=it->getDataType();
    if ( (dType==GRAPH_3DOBJECT_ALL_X_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_Y_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_Z_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_X_REL)||
        (dType==GRAPH_3DOBJECT_ALL_Y_REL)||
        (dType==GRAPH_3DOBJECT_ALL_Z_REL)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_X_ABS)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_Y_ABS)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_Z_ABS)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_X_REL)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_Y_REL)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_Z_REL)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_X_REL_TO_SENSOR)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_Y_REL_TO_SENSOR)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_Z_REL_TO_SENSOR)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_DIST)||
        (dType==GRAPH_DISTANCE_DIST)||
        (dType==GRAPH_DISTANCE_SEG1_X_ABS)||
        (dType==GRAPH_DISTANCE_SEG1_Y_ABS)||
        (dType==GRAPH_DISTANCE_SEG1_Z_ABS)||
        (dType==GRAPH_DISTANCE_SEG2_X_ABS)||
        (dType==GRAPH_DISTANCE_SEG2_Y_ABS)||
        (dType==GRAPH_DISTANCE_SEG2_Z_ABS)||
        (dType==GRAPH_DISTANCE_SEG1_X_REL)||
        (dType==GRAPH_DISTANCE_SEG1_Y_REL)||
        (dType==GRAPH_DISTANCE_SEG1_Z_REL)||
        (dType==GRAPH_DISTANCE_SEG2_X_REL)||
        (dType==GRAPH_DISTANCE_SEG2_Y_REL)||
        (dType==GRAPH_DISTANCE_SEG2_Z_REL) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getSizeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getSizeUnitStr()+"*"+gv::getTimeUnitStr());
    }
    
    if ( (dType==GRAPH_3DOBJECT_ALL_XVEL_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_YVEL_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_ZVEL_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_VEL_ABS) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStrShort()+"^2");
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getSizeUnitStr());
    }


    if (dType==GRAPH_3DOBJECT_ALL_ANGULAR_VEL_ABS)
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStrShort()+"^2");
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getAngleUnitStr());
    }


    if ( (dType==GRAPH_3DOBJECT_ALL_ALPHA_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_BETA_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_GAMMA_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_ALPHA_REL)||
        (dType==GRAPH_3DOBJECT_ALL_BETA_REL)||
        (dType==GRAPH_3DOBJECT_ALL_GAMMA_REL)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_AZIMUTH)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_ELEVATION) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getAngleUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getAngleUnitStr()+"*"+gv::getTimeUnitStr());
    }
    if ( (dType>GRAPH_3DOBJECT_JOINT_SUBSTART)&&(dType<GRAPH_3DOBJECT_JOINT_SUBEND) )
    {
        if (dType==GRAPH_3DOBJECT_JOINT_POSITION)
        {
            CJoint* act=App::ct->objCont->getJoint(it->getDataObjectID());
            if (act!=nullptr)
            {
                if (act->getJointType()==sim_joint_prismatic_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getSizeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getSizeUnitStr()+"*"+gv::getTimeUnitStr());
                }
                if (act->getJointType()==sim_joint_revolute_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getAngleUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getAngleUnitStr()+"*"+gv::getTimeUnitStr());
                }
                if (act->getJointType()==sim_joint_spherical_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getNullUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getNullUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getNullUnitStr()+"*"+gv::getTimeUnitStr());
                }
            }
        }
        if (dType==GRAPH_3DOBJECT_JOINT_VELOCITY)
        {
            CJoint* act=App::ct->objCont->getJoint(it->getDataObjectID());
            if (act!=nullptr)
            {
                if (act->getJointType()==sim_joint_prismatic_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStrShort()+"^2");
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getSizeUnitStr());
                }
                if (act->getJointType()==sim_joint_revolute_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStrShort()+"^2");
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getAngleUnitStr());
                }
                if (act->getJointType()==sim_joint_spherical_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getNullUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getNullUnitStr()+"/"+gv::getTimeUnitStrShort()+"^2");
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getNullUnitStr());
                }
            }
        }
        if (dType==GRAPH_3DOBJECT_JOINT_DYN_POSITION_ERROR)
        {
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                return(gv::getSizeUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                return(gv::getSizeUnitStr()+"*"+gv::getTimeUnitStr());
        }
        if (dType==GRAPH_3DOBJECT_JOINT_DYN_ORIENTATION_ERROR)
        {
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                return(gv::getAngleUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                return(gv::getAngleUnitStr()+"*"+gv::getTimeUnitStr());
        }
        if (dType==GRAPH_3DOBJECT_JOINT_DYN_FORCE_OR_TORQUE)
        { // New since 1/6/2011:
            CJoint* act=App::ct->objCont->getJoint(it->getDataObjectID());
            if (act!=nullptr)
            {
                if (act->getJointType()==sim_joint_prismatic_subtype)
                {
                    std::string retV;
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"/"+gv::getTimeUnitStrShort()+"^2";
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"/"+gv::getTimeUnitStrShort()+"^3";
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"/"+gv::getTimeUnitStrShort();
                    return(retV);
                }
                if (act->getJointType()==sim_joint_revolute_subtype)
                {
                    std::string retV;
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"^2/"+gv::getTimeUnitStrShort()+"^2";
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"^2/"+gv::getTimeUnitStrShort()+"^3";
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"^2/"+gv::getTimeUnitStrShort();
                    return(retV);
                }
                if (act->getJointType()==sim_joint_spherical_subtype)
                {
                    if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                        return(gv::getNullUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                        return(gv::getNullUnitStr()+"/"+gv::getTimeUnitStr());
                    if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                        return(gv::getNullUnitStr()+"*"+gv::getTimeUnitStr());
                }
            }
        }
    }

    if ( (dType>GRAPH_3DOBJECT_FORCESENSOR_SUBSTART)&&(dType<GRAPH_3DOBJECT_FORCESENSOR_SUBEND) )
    {
        if ( (dType==GRAPH_3DOBJECT_FORCESENSOR_FORCE_X)||(dType==GRAPH_3DOBJECT_FORCESENSOR_FORCE_Y)||(dType==GRAPH_3DOBJECT_FORCESENSOR_FORCE_Z)||(dType==GRAPH_3DOBJECT_FORCESENSOR_FORCE_LENGTH) )
        {
            std::string retV;
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"/"+gv::getTimeUnitStrShort()+"^2";
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"/"+gv::getTimeUnitStrShort()+"^3";
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"/"+gv::getTimeUnitStrShort();
            return(retV);
        }
        if ( (dType==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_X)||(dType==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_Y)||(dType==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_Z)||(dType==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_LENGTH) )
        {
            std::string retV;
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"^2/"+gv::getTimeUnitStrShort()+"^2";
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"^2/"+gv::getTimeUnitStrShort()+"^3";
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                retV=gv::getMassUnitStrShort()+"*"+gv::getSizeUnitStrShort()+"^2/"+gv::getTimeUnitStrShort();
            return(retV);
        }
        if (dType==GRAPH_3DOBJECT_FORCESENSOR_POSITION_ERROR)
        {
            std::string retV;
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                retV=gv::getSizeUnitStr();
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                retV=gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr();
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                retV=gv::getSizeUnitStr()+"*"+gv::getTimeUnitStr();
            return(retV);
        }
        if (dType==GRAPH_3DOBJECT_FORCESENSOR_ORIENTATION_ERROR)
        {
            std::string retV;
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                retV=gv::getAngleUnitStr();
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                retV=gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr();
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                retV=gv::getAngleUnitStr()+"*"+gv::getTimeUnitStr();
            return(retV);
        }
        if (dType==GRAPH_3DOBJECT_FORCESENSOR_BROKEN)
        {
            std::string retV;
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                retV=gv::getBoolStateUnitStr();
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                retV=gv::getBoolStateUnitStr()+"/"+gv::getTimeUnitStr();
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                retV=gv::getBoolStateUnitStr()+"*"+gv::getTimeUnitStr();
            return(retV);
        }
    }
    if ((dType==GRAPH_3DOBJECT_PATH_POSITION)||(dType==GRAPH_3DOBJECT_PATH_POSITION_LINEAR_COMPONENT))
    { // We have to check if this is a path:
        CPath* path=App::ct->objCont->getPath(it->getDataObjectID());
        if (path!=nullptr)
        {
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                return(gv::getSizeUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                return(gv::getSizeUnitStr()+"/"+gv::getTimeUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                return(gv::getSizeUnitStr()+"*"+gv::getTimeUnitStr());
        }
    }
    if (dType==GRAPH_3DOBJECT_PATH_POSITION_ANGULAR_COMPONENT)
    { // We have to check if this is a path:
        CPath* path=App::ct->objCont->getPath(it->getDataObjectID());
        if (path!=nullptr)
        {
            if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
                return(gv::getAngleUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
                return(gv::getAngleUnitStr()+"/"+gv::getTimeUnitStr());
            if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
                return(gv::getAngleUnitStr()+"*"+gv::getTimeUnitStr());
        }
    }
    if ( (dType==GRAPH_NOOBJECT_TIME)||
        (dType==GRAPH_COLLISION_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_COLLISION_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_DISTANCE_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_DISTANCE_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_PROXSENSOR_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_3DOBJECT_MILL_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_MILL_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_VISIONSENSOR_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_IK_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_IK_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_GCS_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_DYNAMICS_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_WORK_THREADS_EXECUTION_TIME)||
        (dType==GRAPH_NOOBJECT_RENDERING_TIME)||
        (dType==GRAPH_NOOBJECT_GCS_OVERALL_CALCULATION_TIME)||
        (dType==GRAPH_NOOBJECT_CHILDSCRIPT_EXECUTION_TIME)||
        (dType==GRAPH_NOOBJECT_SIMULATIONPASS_EXECUTION_TIME) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return("-");//gv::getTimeUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getTimeUnitStr()+"*"+gv::getTimeUnitStr());
    }
    if (dType==GRAPH_NOOBJECT_COLLISION_QUANTITY)
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getCountUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getCountUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getCountUnitStr()+"*"+gv::getTimeUnitStr());
    }
    if ( (dType==GRAPH_COLLISION_BOOLEAN)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_DETECTION_STATE)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_DETECTION_STATE) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getBoolStateUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getBoolStateUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getBoolStateUnitStr()+"*"+gv::getTimeUnitStr());
    }
    if ( (dType==GRAPH_3DOBJECT_VISIONSENSOR_MIN_RED)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MIN_GREEN)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MIN_BLUE)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MIN_INTENSITY)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MIN_DEPTH)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MAX_RED)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MAX_GREEN)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MAX_BLUE)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MAX_INTENSITY)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_MAX_DEPTH)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_AVG_RED)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_AVG_GREEN)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_AVG_BLUE)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_AVG_INTENSITY)||
        (dType==GRAPH_3DOBJECT_VISIONSENSOR_AVG_DEPTH) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getFloatUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getFloatUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getFloatUnitStr()+"*"+gv::getTimeUnitStr());
    }
    if (dType==GRAPH_NOOBJECT_USER_DEFINED)
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getUserUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getUserUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getUserUnitStr()+"*"+gv::getTimeUnitStr());
            // was this before:     return(gv::getUserUnitStr());
    }

    if ( (dType==GRAPH_3DOBJECT_MILL_CUT_SURFACE)||
        (dType==GRAPH_NOOBJECT_MILL_OVERALL_CUT_SURFACE) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getSurfaceUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getSurfaceUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getSurfaceUnitStr()+"*"+gv::getTimeUnitStr());
    }
    if ( (dType==GRAPH_3DOBJECT_MILL_CUT_VOLUME)||
        (dType==GRAPH_NOOBJECT_MILL_OVERALL_CUT_VOLUME) )
    {
        if ((it->getDerivativeIntegralAndCumulative()==DATA_STREAM_ORIGINAL)||(it->getDerivativeIntegralAndCumulative()==DATA_STREAM_CUMULATIVE))
            return(gv::getVolumeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_DERIVATIVE)
            return(gv::getVolumeUnitStr()+"/"+gv::getTimeUnitStr());
        if (it->getDerivativeIntegralAndCumulative()==DATA_STREAM_INTEGRAL)
            return(gv::getVolumeUnitStr()+"*"+gv::getTimeUnitStr());
    }
    return(gv::getNullUnitStr());
}

void CGraphingRoutines::adjustDataToUserMetric(const CGraphData* it,float& v,int derivativeIntegralOrCumulative)
{
    int dType=it->getDataType();

    if (dType==GRAPH_3DOBJECT_ALL_ANGULAR_VEL_ABS)
    {
        v*=gv::radToUser;
    }

    if ( (dType==GRAPH_3DOBJECT_ALL_ALPHA_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_BETA_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_GAMMA_ABS)||
        (dType==GRAPH_3DOBJECT_ALL_ALPHA_REL)||
        (dType==GRAPH_3DOBJECT_ALL_BETA_REL)||
        (dType==GRAPH_3DOBJECT_ALL_GAMMA_REL)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_AZIMUTH)||
        (dType==GRAPH_3DOBJECT_PROXSENSOR_ELEVATION) )
    {
            v*=gv::radToUser;
    }
    if ( (dType>GRAPH_3DOBJECT_JOINT_SUBSTART)&&(dType<GRAPH_3DOBJECT_JOINT_SUBEND) )
    {
        CJoint* act=App::ct->objCont->getJoint(it->getDataObjectID());
        if (dType==GRAPH_3DOBJECT_JOINT_POSITION)
        {   // We have to check if this is a joint and what type:
            if (act!=nullptr)
            {
                if (act->getJointType()==sim_joint_revolute_subtype)
                    v*=gv::radToUser;
            }
        }
        if (dType==GRAPH_3DOBJECT_JOINT_VELOCITY)
        {   // We have to check if this is a joint and what type:
            if (act!=nullptr)
            {
                if (act->getJointType()==sim_joint_revolute_subtype)
                    v*=gv::radToUser;
            }
        }
        if (dType==GRAPH_3DOBJECT_JOINT_DYN_ORIENTATION_ERROR)
            v*=gv::radToUser;
    }
    if ( (dType>GRAPH_3DOBJECT_FORCESENSOR_SUBSTART)&&(dType<GRAPH_3DOBJECT_FORCESENSOR_SUBEND) )
    {
        if (dType==GRAPH_3DOBJECT_FORCESENSOR_ORIENTATION_ERROR)
            v*=gv::radToUser;
    }
    if (dType==GRAPH_3DOBJECT_PATH_POSITION_ANGULAR_COMPONENT)
    { // We have to check if this is a path:
        CPath* path=App::ct->objCont->getPath(it->getDataObjectID());
        if (path!=nullptr)
        {
                v*=gv::radToUser;
        }
    }
}

void CGraphingRoutines::getCyclicAndRangeValues(const CGraphData* it,bool& cyclic,float& range)
{
    int dType=it->getDataType();
    if ((dType==GRAPH_3DOBJECT_ALL_ALPHA_ABS)||(dType==GRAPH_3DOBJECT_ALL_GAMMA_ABS))
    {
        cyclic=true;
        range=0.0; // means: piValTimes2;
        return;
    }
    if (dType==GRAPH_3DOBJECT_ALL_BETA_ABS)
    {
        cyclic=true;
        range=piValue_f;
        return;
    }
    if (dType==GRAPH_3DOBJECT_JOINT_POSITION)
    { // We have to check if this is a joint and is cyclic:
        CJoint* act=App::ct->objCont->getJoint(it->getDataObjectID());
        if (act!=nullptr)
        {
            if (act->getPositionIsCyclic())
            {
                cyclic=true;
                range=0.0; // means: piValTimes2;
                return;
            }
        }
    }
    if (dType==GRAPH_3DOBJECT_PATH_POSITION)
    { // We have to check if this is a path and is cyclic:
        CPath* path=App::ct->objCont->getPath(it->getDataObjectID());
        if (path!=nullptr)
        {
            if ((path->pathContainer->getAttributes()&sim_pathproperty_closed_path)!=0)
            {
                cyclic=true;
                range=path->pathContainer->getBezierVirtualPathLength();
                return;
            }
        }
    }
    if (dType==GRAPH_3DOBJECT_PATH_POSITION_LINEAR_COMPONENT)
    { // We have to check if this is a path and is cyclic:
        CPath* path=App::ct->objCont->getPath(it->getDataObjectID());
        if (path!=nullptr)
        {
            if ((path->pathContainer->getAttributes()&sim_pathproperty_closed_path)!=0)
            {
                cyclic=true;
                range=path->pathContainer->getBezierNormalPathLength();
                return;
            }
        }
    }
    if (dType==GRAPH_3DOBJECT_PATH_POSITION_ANGULAR_COMPONENT)
    { // We have to check if this is a path and is cyclic:
        CPath* path=App::ct->objCont->getPath(it->getDataObjectID());
        if (path!=nullptr)
        {
            if ((path->pathContainer->getAttributes()&sim_pathproperty_closed_path)!=0)
            {
                cyclic=true;
                range=path->pathContainer->getBezierAngularPathLength();
                return;
            }
        }
    }
    cyclic=false;
}

bool CGraphingRoutines::getDataName(int dataIndex,std::string& dataName)
{ // If return value is true, dataName contains the text for the dataIndex (used by graphs)
    dataName=IDS_ERROR;
    if (dataIndex==GRAPH_COLLISION_BOOLEAN)
        dataName=IDS_COLLISION__COLLIDING_STATE;
    if (dataIndex==GRAPH_NOOBJECT_COLLISION_QUANTITY)
        dataName=IDS_COLLISION__NUMBER_OF_COLLISIONS;
    if (dataIndex==GRAPH_COLLISION_CALCULATION_TIME)
        dataName=IDS_COLLISION_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_COLLISION_OVERALL_CALCULATION_TIME)
        dataName=IDS_COLLISION_OVERALL_CALCULATION_TIME;
    if (dataIndex==GRAPH_DISTANCE_DIST)
        dataName=IDS_DISTANCE__SEGMENT_LENGTH;
    if (dataIndex==GRAPH_DISTANCE_SEG1_X_ABS)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_1_ABSOLUTE_X_POSITION;
    if (dataIndex==GRAPH_DISTANCE_SEG1_Y_ABS)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_1_ABSOLUTE_Y_POSITION;
    if (dataIndex==GRAPH_DISTANCE_SEG1_Z_ABS)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_1_ABSOLUTE_Z_POSITION;
    if (dataIndex==GRAPH_DISTANCE_SEG2_X_ABS)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_2_ABSOLUTE_X_POSITION;
    if (dataIndex==GRAPH_DISTANCE_SEG2_Y_ABS)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_2_ABSOLUTE_Y_POSITION;
    if (dataIndex==GRAPH_DISTANCE_SEG2_Z_ABS)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_2_ABSOLUTE_Z_POSITION;
    if (dataIndex==GRAPH_DISTANCE_SEG1_X_REL)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_1_X_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_DISTANCE_SEG1_Y_REL)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_1_Y_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_DISTANCE_SEG1_Z_REL)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_1_Z_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_DISTANCE_SEG2_X_REL)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_2_X_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_DISTANCE_SEG2_Y_REL)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_2_Y_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_DISTANCE_SEG2_Z_REL)
        dataName=IDS_DISTANCE__SEGMENT_SIDE_2_Z_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_DISTANCE_CALCULATION_TIME)
        dataName=IDS_DISTANCE_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_DISTANCE_OVERALL_CALCULATION_TIME)
        dataName=IDS_DISTANCE_OVERALL_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_TIME)
        dataName=IDS_SIMULATION__TIME;
    if (dataIndex==GRAPH_NOOBJECT_USER_DEFINED)
        dataName=IDS_VARIOUS__USER_DEFINED;
    if (dataIndex==GRAPH_3DOBJECT_ALL_X_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_X_POSITION;
    if (dataIndex==GRAPH_3DOBJECT_ALL_Y_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_Y_POSITION;
    if (dataIndex==GRAPH_3DOBJECT_ALL_Z_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_Z_POSITION;
    if (dataIndex==GRAPH_3DOBJECT_ALL_X_REL)
        dataName=IDS_OBJECT__X_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_ALL_Y_REL)
        dataName=IDS_OBJECT__Y_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_ALL_Z_REL)
        dataName=IDS_OBJECT__Z_POSITION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_ALL_ALPHA_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_ALPHA_ORIENTATION;
    if (dataIndex==GRAPH_3DOBJECT_ALL_BETA_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_BETA_ORIENTATION;
    if (dataIndex==GRAPH_3DOBJECT_ALL_GAMMA_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_GAMMA_ORIENTATION;


    if (dataIndex==GRAPH_3DOBJECT_ALL_ALPHA_REL)
        dataName=IDS_OBJECT__ALPHA_ORIENTATION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_ALL_BETA_REL)
        dataName=IDS_OBJECT__BETA_ORIENTATION_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_ALL_GAMMA_REL)
        dataName=IDS_OBJECT__GAMMA_ORIENTATION_RELATIVE_TO_GRAPH;



    if (dataIndex==GRAPH_3DOBJECT_JOINT_POSITION)
        dataName=IDS_JOINT__POSITION;
    if (dataIndex==GRAPH_3DOBJECT_JOINT_VELOCITY)
        dataName=IDS_JOINT__VELOCITY;
    if (dataIndex==GRAPH_3DOBJECT_JOINT_DYN_POSITION_ERROR)
        dataName=IDS_JOINT__DYN_POSITION_ERROR;
    if (dataIndex==GRAPH_3DOBJECT_JOINT_DYN_ORIENTATION_ERROR)
        dataName=IDS_JOINT__DYN_ORIENTATION_ERROR;
    if (dataIndex==GRAPH_3DOBJECT_JOINT_DYN_FORCE_OR_TORQUE)
        dataName=IDS_JOINT__FORCE_OR_TORQUE;
    

    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_X)
        dataName=IDS_FORCE_SENSOR__FORCE_X;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_Y)
        dataName=IDS_FORCE_SENSOR__FORCE_Y;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_Z)
        dataName=IDS_FORCE_SENSOR__FORCE_Z;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_LENGTH)
        dataName=IDS_FORCE_SENSOR__FORCE;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_X)
        dataName=IDS_FORCE_SENSOR__TORQUE_X;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_Y)
        dataName=IDS_FORCE_SENSOR__TORQUE_Y;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_Z)
        dataName=IDS_FORCE_SENSOR__TORQUE_Z;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_LENGTH)
        dataName=IDS_FORCE_SENSOR__TORQUE;

    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_POSITION_ERROR)
        dataName=IDS_FORCE_SENSOR__POSITION_ERROR;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_ORIENTATION_ERROR)
        dataName=IDS_FORCE_SENSOR__ORIENTATION_ERROR;
    if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_BROKEN)
        dataName=IDS_FORCE_SENSOR__BROKEN_STATE;


    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_REL_TO_SENSOR)
        dataName=IDS_PROXSENSOR__DETECTION_POSITION_X_RELATIVE_TO_SENSOR;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_REL_TO_SENSOR)
        dataName=IDS_PROXSENSOR__DETECTION_POSITION_Y_RELATIVE_TO_SENSOR;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_REL_TO_SENSOR)
        dataName=IDS_PROXSENSOR__DETECTION_POSITION_Z_RELATIVE_TO_SENSOR;

    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_ABS)
        dataName=IDS_PROXSENSOR__ABSOLUTE_DETECTION_POSITION_X;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_ABS)
        dataName=IDS_PROXSENSOR__ABSOLUTE_DETECTION_POSITION_Y;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_ABS)
        dataName=IDS_PROXSENSOR__ABSOLUTE_DETECTION_POSITION_Z;


    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_REL)
        dataName=IDS_PROXSENSOR__DETECTION_POSITION_X_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_REL)
        dataName=IDS_PROXSENSOR__DETECTION_POSITION_Y_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_REL)
        dataName=IDS_PROXSENSOR__DETECTION_POSITION_Z_RELATIVE_TO_GRAPH;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_DIST)
        dataName=IDS_PROXSENSOR__DETECTION_DISTANCE;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_AZIMUTH)
        dataName=IDS_PROXSENSOR__DETECTION_AZIMUTH;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_ELEVATION)
        dataName=IDS_PROXSENSOR__DETECTION_ELEVATION;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_DETECTION_STATE)
        dataName=IDS_PROXSENSOR__DETECTION_STATE;
    if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_CALCULATION_TIME)
        dataName=IDS_PROXSENSOR_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_PROXSENSOR_OVERALL_CALCULATION_TIME)
        dataName=IDS_PROXSENSOR_OVERALL_CALCULATION_TIME;

    if (dataIndex==GRAPH_3DOBJECT_MILL_CUT_SURFACE)
        dataName=IDS_MILL_CUT_SURFACE;
    if (dataIndex==GRAPH_3DOBJECT_MILL_CUT_VOLUME)
        dataName=IDS_MILL_CUT_VOLUME;
    if (dataIndex==GRAPH_3DOBJECT_MILL_CALCULATION_TIME)
        dataName=IDS_MILL_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_SURFACE)
        dataName=IDS_MILL_OVERALL_CUT_SURFACE;
    if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_VOLUME)
        dataName=IDS_MILL_OVERALL_CUT_VOLUME;
    if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CALCULATION_TIME)
        dataName=IDS_MILL_OVERALL_CALCULATION_TIME;

    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_RED)
        dataName=IDS_VISIONSENS_GRAPH_MIN_RED;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_GREEN)
        dataName=IDS_VISIONSENS_GRAPH_MIN_GREEN;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_BLUE)
        dataName=IDS_VISIONSENS_GRAPH_MIN_BLUE;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_INTENSITY)
        dataName=IDS_VISIONSENS_GRAPH_MIN_INTENSITY;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_DEPTH)
        dataName=IDS_VISIONSENS_GRAPH_MIN_DEPTH;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_RED)
        dataName=IDS_VISIONSENS_GRAPH_MAX_RED;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_GREEN)
        dataName=IDS_VISIONSENS_GRAPH_MAX_GREEN;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_BLUE)
        dataName=IDS_VISIONSENS_GRAPH_MAX_BLUE;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_INTENSITY)
        dataName=IDS_VISIONSENS_GRAPH_MAX_INTENSITY;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_DEPTH)
        dataName=IDS_VISIONSENS_GRAPH_MAX_DEPTH;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_RED)
        dataName=IDS_VISIONSENS_GRAPH_AVG_RED;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_GREEN)
        dataName=IDS_VISIONSENS_GRAPH_AVG_GREEN;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_BLUE)
        dataName=IDS_VISIONSENS_GRAPH_AVG_BLUE;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_INTENSITY)
        dataName=IDS_VISIONSENS_GRAPH_AVG_INTENSITY;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_DEPTH)
        dataName=IDS_VISIONSENS_GRAPH_AVG_DEPTH;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_DETECTION_STATE)
        dataName=IDS_VISIONSENS_GRAPH_DETECTION_STATE;
    if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_CALCULATION_TIME)
        dataName=IDS_VISIONSENS_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_VISIONSENSOR_OVERALL_CALCULATION_TIME)
        dataName=IDS_VISIONSENS_OVERALL_CALCULATION_TIME;
    if (dataIndex==GRAPH_3DOBJECT_PATH_POSITION)
        dataName=IDS_PATH_GRAPH_POSITION;
    if (dataIndex==GRAPH_3DOBJECT_PATH_POSITION_LINEAR_COMPONENT)
        dataName=IDS_PATH_GRAPH_POSITION_LINEAR_COMPONENT;
    if (dataIndex==GRAPH_3DOBJECT_PATH_POSITION_ANGULAR_COMPONENT)
        dataName=IDS_PATH_GRAPH_POSITION_ANGULAR_COMPONENT;
    if (dataIndex==GRAPH_IK_CALCULATION_TIME)
        dataName=IDS_IK_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_IK_OVERALL_CALCULATION_TIME)
        dataName=IDS_IK_OVERALL_CALCULATION_TIME;
    if (dataIndex==GRAPH_GCS_CALCULATION_TIME)
        dataName=IDS_GCS_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_DYNAMICS_OVERALL_CALCULATION_TIME)
        dataName=IDS_DYNAMICS_OVERALL_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_WORK_THREADS_EXECUTION_TIME)
        dataName=IDS_WORK_THREADS_EXECUTION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_RENDERING_TIME)
        dataName=IDS_RENDERING_TIME;
    if (dataIndex==GRAPH_NOOBJECT_GCS_OVERALL_CALCULATION_TIME)
        dataName=IDS_GCS_OVERALL_CALCULATION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_CHILDSCRIPT_EXECUTION_TIME)
        dataName=IDS_CHILD_SCRIPT_EXECUTION_TIME;
    if (dataIndex==GRAPH_NOOBJECT_SIMULATIONPASS_EXECUTION_TIME)
        dataName=IDS_SIMULATION_PASS_EXECUTION_TIME;

    if (dataIndex==GRAPH_3DOBJECT_ALL_XVEL_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_X_VELOCITY;
    if (dataIndex==GRAPH_3DOBJECT_ALL_YVEL_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_Y_VELOCITY;
    if (dataIndex==GRAPH_3DOBJECT_ALL_ZVEL_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_Z_VELOCITY;
    if (dataIndex==GRAPH_3DOBJECT_ALL_VEL_ABS)
        dataName=IDS_OBJECT__ABSOLUTE_VELOCITY;
    if (dataIndex==GRAPH_3DOBJECT_ALL_ANGULAR_VEL_ABS)
        dataName=IDS_OBJECT__ANGULAR_VELOCITY;

    return(dataName!=std::string(IDS_ERROR));
}

bool CGraphingRoutines::getDataValue(int dataIndex,int objectID,float& value,const C7Vector* graphCTM)
{   // If return value is true, value contains the asked value (used by graphs)
    // If return value is false, the value can't be found here. If it can't be
    // found anywhere, use the default value!
    // graphCTM is nullptr by default

    if ( (dataIndex>GRAPH_NOOBJECT_START)&&(dataIndex<GRAPH_NOOBJECT_END) )
    {
        if (dataIndex==GRAPH_NOOBJECT_IK_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getIkCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_DYNAMICS_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getDynamicsCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_GCS_OVERALL_CALCULATION_TIME)
        {
            value=0; // deprecated
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_CHILDSCRIPT_EXECUTION_TIME)
        {
            value=App::ct->calcInfo->getChildScriptExecutionTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_RENDERING_TIME)
        {
            value=App::ct->calcInfo->getRenderingDuration();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_WORK_THREADS_EXECUTION_TIME)
        {
            value=0.0; // for backward compatibility
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_SIMULATIONPASS_EXECUTION_TIME)
        {
            value=App::ct->calcInfo->getSimulationPassExecutionTime()+App::ct->calcInfo->getRenderingDuration(); // added second part on 3/4/2013
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_COLLISION_QUANTITY)
        {
            int q=0;
            for (int i=0;i<int(App::ct->collisions->collisionObjects.size());i++)
            {
                CRegCollision* aColl=App::ct->collisions->collisionObjects[i];
                if (aColl!=nullptr)
                    if (aColl->getCollisionResult())
                        q++;
            }
            value=(float)q;
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_COLLISION_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getCollisionCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_DISTANCE_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getDistanceCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_PROXSENSOR_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getProximitySensorCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_VISIONSENSOR_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getVisionSensorCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CALCULATION_TIME)
        {
            value=App::ct->calcInfo->getMillingCalculationTime();
            return(true);
        }
        if ( (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_SURFACE)||
            (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_VOLUME) )
        {
            bool validData=false;
            float totV=0.0f;
            for (int i=0;i<int(App::ct->objCont->millList.size());i++)
            {
                float v;
                CMill* it=(CMill*)App::ct->objCont->getObjectFromHandle(App::ct->objCont->millList[i]);
                if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_VOLUME)
                {
                    if (it->getMilledVolume(v))
                    {
                        validData=true;
                        totV+=v;
                    }
                }
                else
                {
                    if (it->getMilledSurface(v))
                    {
                        validData=true;
                        totV+=v;
                    }
                }
            }
            if (validData)
                value=totV;
            return(validData);
        }

        if (dataIndex==GRAPH_NOOBJECT_USER_DEFINED)
            return(false); // We want a default value
        if (dataIndex==GRAPH_NOOBJECT_TIME)
        { // We handle this case in CGraph::addNextPoint!!!! (this is an exception!)        
            return(true);
        }
        return(false);
    }


    if ( (dataIndex>GRAPH_IK_START)&&(dataIndex<GRAPH_IK_END) )
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(objectID);
        if (it==nullptr)
            return(false);
        if (dataIndex==GRAPH_IK_CALCULATION_TIME)
        {
            value=it->getCalculationTime();
            return(true);
        }
        return(false);
    }

    if ( (dataIndex>GRAPH_GCS_START)&&(dataIndex<GRAPH_GCS_END) )
    {
        CConstraintSolverObject* it=App::ct->constraintSolver->getObject(objectID);
        if (it==nullptr)
            return(false);
        if (dataIndex==GRAPH_GCS_CALCULATION_TIME)
        {
            value=it->getCalculationTime();
            return(true);
        }
        return(false);
    }

    if ( (dataIndex>GRAPH_COLLISION_START)&&(dataIndex<GRAPH_COLLISION_END) )
    {
        CRegCollision* it=App::ct->collisions->getObject(objectID);
        if (it==nullptr)
            return(false);
        if (dataIndex==GRAPH_COLLISION_BOOLEAN)
        {
            if (!it->isCollisionResultValid())
                return(false);
            if (it->getCollisionResult())
                value=1.0f;
            else
                value=0.0f;
            return(true);
        }
        if (dataIndex==GRAPH_COLLISION_CALCULATION_TIME)
        {
            if (!it->isCollisionResultValid())
                return(false);
            value=it->getCalculationTime();
            return(true);
        }
        return(false);
    }
    if ( (dataIndex>GRAPH_DISTANCE_START)&&(dataIndex<GRAPH_DISTANCE_END) )
    {
        CRegDist* it=App::ct->distances->getObject(objectID);
        if (it==nullptr)
            return(false);
        float dist[7];
        bool validResult=it->getDistanceResult(dist);
        if (dataIndex==GRAPH_DISTANCE_DIST)
        {
            value=dist[6];
            return(validResult);
        }
        if (dataIndex==GRAPH_DISTANCE_CALCULATION_TIME)
        {
            value=it->getCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_DISTANCE_SEG1_X_ABS)
        {
            value=dist[0];
            return(validResult);
        }
        if (dataIndex==GRAPH_DISTANCE_SEG1_Y_ABS)
        {
            value=dist[1];
            return(validResult);
        }
        if (dataIndex==GRAPH_DISTANCE_SEG1_Z_ABS)
        {
            value=dist[2];
            return(validResult);
        }
        if (dataIndex==GRAPH_DISTANCE_SEG2_X_ABS)
        {
            value=dist[3];
            return(validResult);
        }
        if (dataIndex==GRAPH_DISTANCE_SEG2_Y_ABS)
        {
            value=dist[4];
            return(validResult);
        }
        if (dataIndex==GRAPH_DISTANCE_SEG2_Z_ABS)
        {
            value=dist[5];
            return(validResult);
        }
        if ( (dataIndex==GRAPH_DISTANCE_SEG1_X_REL)||
            (dataIndex==GRAPH_DISTANCE_SEG1_Y_REL)||
            (dataIndex==GRAPH_DISTANCE_SEG1_Z_REL)||
            (dataIndex==GRAPH_DISTANCE_SEG2_X_REL)||
            (dataIndex==GRAPH_DISTANCE_SEG2_Y_REL)||
            (dataIndex==GRAPH_DISTANCE_SEG2_Z_REL) )
        {
            if (!validResult)
                return(validResult);
            if (graphCTM==nullptr)
                return(false);
            C7Vector graphInv(graphCTM->getInverse());
            C3Vector d0(dist);
            C3Vector d1(dist+3);
            d0*=graphInv;
            d1*=graphInv;
            d0.copyTo(dist);
            d1.copyTo(dist+3);
            if (dataIndex==GRAPH_DISTANCE_SEG1_X_REL)
                value=dist[0];
            if (dataIndex==GRAPH_DISTANCE_SEG1_Y_REL)
                value=dist[1];
            if (dataIndex==GRAPH_DISTANCE_SEG1_Z_REL)
                value=dist[2];
            if (dataIndex==GRAPH_DISTANCE_SEG2_X_REL)
                value=dist[3];
            if (dataIndex==GRAPH_DISTANCE_SEG2_Y_REL)
                value=dist[4];
            if (dataIndex==GRAPH_DISTANCE_SEG2_Z_REL)
                value=dist[5];
            return(validResult);
        }
        return(false);
    }



    if ( (dataIndex>GRAPH_3DOBJECT_JOINT_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_JOINT_SUBEND) )
    {
        CJoint* it=App::ct->objCont->getJoint(objectID);
        if (it==nullptr)
            return(false);
        if (dataIndex==GRAPH_3DOBJECT_JOINT_POSITION)
        {
            value=it->getPosition();
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_JOINT_VELOCITY)
        {
            value=it->getMeasuredJointVelocity();
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_JOINT_DYN_POSITION_ERROR)
        {
            float dummyVal;
            it->getDynamicJointErrors(value,dummyVal);
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_JOINT_DYN_ORIENTATION_ERROR)
        {
            float dummyVal;
            it->getDynamicJointErrors(dummyVal,value);
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_JOINT_DYN_FORCE_OR_TORQUE)
        { // New since 1/6/2011
            float dummyVal;
            if (it->getDynamicForceOrTorque(dummyVal,false))
            {
                value=dummyVal;
                return(true);
            }
            return(false);
        }
        return(false);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_FORCESENSOR_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_FORCESENSOR_SUBEND) )
    {
        CForceSensor* it=App::ct->objCont->getForceSensor(objectID);
        if (it==nullptr)
            return(false);
        C3Vector forces;
        C3Vector torques;
        bool validForces=it->getDynamicForces(forces,false);
        bool validTorques=it->getDynamicTorques(torques,false);
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_X)&&validForces)
        {
            value=forces(0);
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_Y)&&validForces)
        {
            value=forces(1);
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_Z)&&validForces)
        {
            value=forces(2);
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_FORCE_LENGTH)&&validForces)
        {
            value=forces.getLength();
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_X)&&validTorques)
        {
            value=torques(0);
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_Y)&&validTorques)
        {
            value=torques(1);
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_Z)&&validTorques)
        {
            value=torques(2);
            return(true);
        }
        if ((dataIndex==GRAPH_3DOBJECT_FORCESENSOR_TORQUE_LENGTH)&&validTorques)
        {
            value=torques.getLength();
            return(true);
        }

        if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_POSITION_ERROR)
        {
            value=it->getDynamicPositionError();
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_ORIENTATION_ERROR)
        {
            value=it->getDynamicOrientationError();
            return(true);
        }

        if (dataIndex==GRAPH_3DOBJECT_FORCESENSOR_BROKEN)
        {
            value=0.0f;
            if (it->getForceSensorIsBroken())
                value=1.0f;
            return(true);
        }

        return(false);
    }


    if ( (dataIndex>GRAPH_3DOBJECT_PATH_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_PATH_SUBEND) )
    {
        CPath* it=App::ct->objCont->getPath(objectID);
        if (it==nullptr)
            return(false);
        if (dataIndex==GRAPH_3DOBJECT_PATH_POSITION)
        {
            value=float(it->pathContainer->getPosition());
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_PATH_POSITION_LINEAR_COMPONENT)
        {
            value=float(it->pathContainer->getPosition_linearComponentOnly());
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_PATH_POSITION_ANGULAR_COMPONENT)
        {
            value=float(it->pathContainer->getPosition_angularComponentOnly());
            return(true);
        }
        return(false);
    }

    if ( (dataIndex>GRAPH_3DOBJECT_PROXSENSOR_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_PROXSENSOR_SUBEND) )
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
        if (it==nullptr)
            return(false);
        if (it->getObjectType()!=sim_object_proximitysensor_type)
            return(false);
        CProxSensor* sens=(CProxSensor*)it;
        if ( (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_REL_TO_SENSOR)||
            (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_REL_TO_SENSOR)||
            (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_REL_TO_SENSOR) )
        {
            C3Vector pt;
            if (sens->getSensedData(pt))
            {
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_REL_TO_SENSOR)
                    value=pt(0);
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_REL_TO_SENSOR)
                    value=pt(1);
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_REL_TO_SENSOR)
                    value=pt(2);
                return(true);
            }
            else
                return(false); // didn't sense anything
        }
        if ( (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_REL)||
            (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_REL)||
            (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_REL) )
        {
            if (graphCTM==nullptr)
                return(false);
            C3Vector pt;
            if (sens->getSensedData(pt))
            {
                C7Vector sensCTM(sens->getCumulativeTransformationPart1());
                C7Vector graphInv(graphCTM->getInverse());
                sensCTM=graphInv*sensCTM;
                pt=sensCTM*pt;
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_REL)
                    value=pt(0);
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_REL)
                    value=pt(1);
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_REL)
                    value=pt(2);
                return(true);
            }
            else
                return(false); // didn't sense anything
        }
        if ( (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_ABS)||
            (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_ABS)||
            (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_ABS) )
        {
            C3Vector pt;
            if (sens->getSensedData(pt))
            {
                C4X4Matrix sensCTM(sens->getCumulativeTransformationPart1().getMatrix());
                pt=sensCTM*pt;
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_X_ABS)
                    value=pt(0);
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Y_ABS)
                    value=pt(1);
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_Z_ABS)
                    value=pt(2);
                return(true);
            }
            else
                return(false); // didn't sense anything
        }
        if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_DIST)
        {
            C3Vector pt;
            if (sens->getSensedData(pt))
            {
                value=pt.getLength();
                return(true);
            }
            else
            {
                return(false); // didn't sense anything
            }
        }
        if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_CALCULATION_TIME)
        {
            value=sens->getCalculationTime();
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_DETECTION_STATE)
        {
            C3Vector pt;
            if (sens->getSensedData(pt))
                value=1.0f;
            else
                value=0.0f;
            return(true);
        }
        if ( (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_AZIMUTH)||(dataIndex==GRAPH_3DOBJECT_PROXSENSOR_ELEVATION) )
        {
            C3Vector pt;
            if (sens->getSensedData(pt))
            {
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_ELEVATION)
                {
                    float r=pt.getLength();
                    float el=CMath::robustAsin(pt(1)/r);
                    value=el;
                }
                if (dataIndex==GRAPH_3DOBJECT_PROXSENSOR_AZIMUTH)
                {
                    float az;
                    if (pt(2)!=0.0f)
                        az=(float)atan(-pt(0)/pt(2));
                    else
                    {
                        if (pt(0)>=0.0f) 
                            az=piValD2_f;
                        else 
                            az=-piValD2_f;
                    }
                    value=az;
                }
                return(true);
            }
            else
                return(false); // didn't sense anything
        }
        return(false);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_MILL_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_MILL_SUBEND) )
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
        if (it==nullptr)
            return(false);
        if (it->getObjectType()!=sim_object_mill_type)
            return(false);
        CMill* mill=(CMill*)it;
        if (dataIndex==GRAPH_3DOBJECT_MILL_CUT_SURFACE)
        {
            if (mill->getMilledSurface(value))
                return(true);
            else
                return(false); // not valid data
        }
        if (dataIndex==GRAPH_3DOBJECT_MILL_CUT_VOLUME)
        {
            if (mill->getMilledVolume(value))
                return(true);
            else
                return(false); // not valid data
        }
        if (dataIndex==GRAPH_3DOBJECT_MILL_CALCULATION_TIME)
        {
            float dummy;
            if (mill->getMilledSurface(dummy))
            {
                value=mill->getCalculationTime();
                return(true);
            }
            else
                return(false); // not valid data
        }
        return(false);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_VISIONSENSOR_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_VISIONSENSOR_SUBEND) )
    {
        CVisionSensor* it=App::ct->objCont->getVisionSensor(objectID);
        if (it==nullptr)
            return(false);
        SHandlingResult* r=&it->sensorResult;
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_RED)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataRed[0])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_GREEN)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataGreen[0])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_BLUE)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataBlue[0])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_INTENSITY)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataIntensity[0])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MIN_DEPTH)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=r->sensorDataDepth[0];
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_RED)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataRed[1])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_GREEN)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataGreen[1])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_BLUE)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataBlue[1])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_INTENSITY)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataIntensity[1])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_MAX_DEPTH)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=r->sensorDataDepth[1];
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_RED)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataRed[2])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_GREEN)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataGreen[2])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_BLUE)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataBlue[2])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_INTENSITY)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=float(r->sensorDataIntensity[2])/255.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_AVG_DEPTH)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=r->sensorDataDepth[2];
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_DETECTION_STATE)
        {
            if (!r->sensorResultIsValid)
                return(false);
            if (r->sensorWasTriggered)
                value=1.0f;
            else
                value=0.0f;
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_VISIONSENSOR_CALCULATION_TIME)
        {
            if (!r->sensorResultIsValid)
                return(false);
            value=it->getCalculationTime();
            return(true);
        }
        return(false);
    }


    // DO FOLLOWING ONLY AFTER HAVING HANDLED ALL 3DOBJECT SUB_CATEGORIES (e.g. Joints, paths, sensors, etc.)
    if ( (dataIndex>GRAPH_3DOBJECT_START)&&(dataIndex<GRAPH_3DOBJECT_END) )
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
        if (it==nullptr)
            return(false);
        if ( (dataIndex==GRAPH_3DOBJECT_ALL_X_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_Y_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_Z_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_ALPHA_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_BETA_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_GAMMA_ABS) )
        {
            C7Vector itCTM(it->getCumulativeTransformationPart1());
            C3Vector euler(itCTM.Q.getEulerAngles());
            if (dataIndex==GRAPH_3DOBJECT_ALL_X_ABS)
                value=itCTM.X(0);
            if (dataIndex==GRAPH_3DOBJECT_ALL_Y_ABS)
                value=itCTM.X(1);
            if (dataIndex==GRAPH_3DOBJECT_ALL_Z_ABS)
                value=itCTM.X(2);
            if (dataIndex==GRAPH_3DOBJECT_ALL_ALPHA_ABS)
                value=euler(0);
            if (dataIndex==GRAPH_3DOBJECT_ALL_BETA_ABS)
                value=euler(1);
            if (dataIndex==GRAPH_3DOBJECT_ALL_GAMMA_ABS)
                value=euler(2);
            return(true);
        }
        if ( (dataIndex==GRAPH_3DOBJECT_ALL_X_REL)||
            (dataIndex==GRAPH_3DOBJECT_ALL_Y_REL)||
            (dataIndex==GRAPH_3DOBJECT_ALL_Z_REL)||
            (dataIndex==GRAPH_3DOBJECT_ALL_ALPHA_REL)||
            (dataIndex==GRAPH_3DOBJECT_ALL_BETA_REL)||
            (dataIndex==GRAPH_3DOBJECT_ALL_GAMMA_REL) )
        {
            C7Vector itCTM(it->getCumulativeTransformationPart1());
            if (graphCTM==nullptr)
                return(false);
            C7Vector graphInv(graphCTM->getInverse());
            itCTM=graphInv*itCTM;
            C3Vector euler(itCTM.Q.getEulerAngles());
            if (dataIndex==GRAPH_3DOBJECT_ALL_X_REL)
                value=itCTM.X(0);
            if (dataIndex==GRAPH_3DOBJECT_ALL_Y_REL)
                value=itCTM.X(1);
            if (dataIndex==GRAPH_3DOBJECT_ALL_Z_REL)
                value=itCTM.X(2);
            if (dataIndex==GRAPH_3DOBJECT_ALL_ALPHA_REL)
                value=euler(0);
            if (dataIndex==GRAPH_3DOBJECT_ALL_BETA_REL)
                value=euler(1);
            if (dataIndex==GRAPH_3DOBJECT_ALL_GAMMA_REL)
                value=euler(2);
            return(true);
        }
        if ( (dataIndex==GRAPH_3DOBJECT_ALL_XVEL_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_YVEL_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_ZVEL_ABS)||
            (dataIndex==GRAPH_3DOBJECT_ALL_VEL_ABS) )
        {
            C3Vector v(it->getMeasuredLinearVelocity());
            if (dataIndex==GRAPH_3DOBJECT_ALL_XVEL_ABS)
                value=v(0);
            if (dataIndex==GRAPH_3DOBJECT_ALL_YVEL_ABS)
                value=v(1);
            if (dataIndex==GRAPH_3DOBJECT_ALL_ZVEL_ABS)
                value=v(2);
            if (dataIndex==GRAPH_3DOBJECT_ALL_VEL_ABS)
                value=v.getLength();
            return(true);
        }
        if (dataIndex==GRAPH_3DOBJECT_ALL_ANGULAR_VEL_ABS)
        {
            value=it->getMeasuredAngularVelocity();
            return(true);
        }
        return(false);
    }

    return(false);
}
bool CGraphingRoutines::getGraphObjectName(int dataIndex,int objectID,std::string& objName)
{ // Returns the name of an object being graphed (name of 3DObject, sensor, collision partn., etc)
    // objName is valid only if return value is true
    if ( (dataIndex>GRAPH_IK_START)&&(dataIndex<GRAPH_IK_END) )
    {
        CikGroup* it=App::ct->ikGroups->getIkGroup(objectID);
        if (it==nullptr)
            return(false);
        objName=it->getObjectName();
        return(true);
    }
    if ( (dataIndex>GRAPH_GCS_START)&&(dataIndex<GRAPH_GCS_END) )
    {
        CConstraintSolverObject* it=App::ct->constraintSolver->getObject(objectID);
        if (it==nullptr)
            return(false);
        objName=it->getObjectName();
        return(true);
    }
    if ( (dataIndex>GRAPH_COLLISION_START)&&(dataIndex<GRAPH_COLLISION_END) )
    {
        CRegCollision* it=App::ct->collisions->getObject(objectID);
        if (it==nullptr)
            return(false);
        objName=it->getObjectName();
        return(true);
    }
    if ( (dataIndex>GRAPH_DISTANCE_START)&&(dataIndex<GRAPH_DISTANCE_END) )
    {
        CRegDist* it=App::ct->distances->getObject(objectID);
        if (it==nullptr)
            return(false);
        objName=it->getObjectName();
        return(true);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_START)&&(dataIndex<GRAPH_3DOBJECT_END) )
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objectID);
        if (it==nullptr)
            return(false);
        objName=it->getObjectName();
        return(true);
    }
    if ((dataIndex==GRAPH_NOOBJECT_IK_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_DYNAMICS_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_GCS_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_PROXSENSOR_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_VISIONSENSOR_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_COLLISION_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CALCULATION_TIME)||
        (dataIndex==GRAPH_NOOBJECT_DISTANCE_OVERALL_CALCULATION_TIME) )
    {
        objName=IDS_OVERALL_CALCULATION_TIME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_SURFACE)
    {
        objName=IDS_OVERALL_CUT_SURFACE;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_MILL_OVERALL_CUT_VOLUME)
    {
        objName=IDS_OVERALL_CUT_VOLUME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_CHILDSCRIPT_EXECUTION_TIME)
    {
        objName=IDS_EXECUTION_TIME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_WORK_THREADS_EXECUTION_TIME)
    {
        objName=IDS_EXECUTION_TIME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_RENDERING_TIME)
    {
        objName=IDS_EXECUTION_TIME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_SIMULATIONPASS_EXECUTION_TIME)
    {
        objName=IDS_EXECUTION_TIME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_COLLISION_QUANTITY)
    {
        objName=IDS_OVERALL_COLLISION_COUNT;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_TIME)
    {
        objName=IDS_SIMULATION_TIME;
        return(true);
    }
    if (dataIndex==GRAPH_NOOBJECT_USER_DEFINED)
    {
        objName=IDS_USER_DATA;
        return(true);
    }
    return(false);
}

bool CGraphingRoutines::getObjectsFromGraphCategory(int index,int dataIndex,int& objectID)
{   // Returns the objectID and auxDat for all objects concerned by the category 'dataIndex'
    // Loop through this function beginning with index=0 and incrementing it as long
    // as the return value is true
    if (index<0)
        return(false);


    if ( (dataIndex>GRAPH_NOOBJECT_START)&&(dataIndex<GRAPH_NOOBJECT_END) )
    {
        objectID=-1;
        if (index==0)
            return(true);
        return(false);  
    }
    if ( (dataIndex>GRAPH_IK_START)&&(dataIndex<GRAPH_IK_END) )
    {
        if (index>=int(App::ct->ikGroups->ikGroups.size()))
            return(false);
        CikGroup* it=App::ct->ikGroups->ikGroups[index];
        if (it==nullptr)
            return(false);
        objectID=it->getObjectID();
        return(true);
    }
    if ( (dataIndex>GRAPH_GCS_START)&&(dataIndex<GRAPH_GCS_END) )
    {
        if (index>=int(App::ct->constraintSolver->allGcsObjects.size()))
            return(false);
        CConstraintSolverObject* it=App::ct->constraintSolver->allGcsObjects[index];
        if (it==nullptr)
            return(false);
        objectID=it->getObjectID();
        return(true);
    }
    if ( (dataIndex>GRAPH_COLLISION_START)&&(dataIndex<GRAPH_COLLISION_END) )
    {
        if (index>=int(App::ct->collisions->collisionObjects.size()))
            return(false);
        CRegCollision* it=App::ct->collisions->collisionObjects[index];
        if (it==nullptr)
            return(false);
        objectID=it->getObjectID();
        return(true);
    }
    if ( (dataIndex>GRAPH_DISTANCE_START)&&(dataIndex<GRAPH_DISTANCE_END) )
    {
        if (index>=int(App::ct->distances->distanceObjects.size()))
            return(false);
        CRegDist* it=App::ct->distances->distanceObjects[index];
        if (it==nullptr)
            return(false);
        objectID=it->getObjectID();
        return(true);
    }


    // HERE WE HAVE ALL 3DOBJECT SUB-CATEGORIES (IMPORTANT TO PUT THEM BEFORE 3DOBJECT IN GENERAL!)
    if ( (dataIndex>GRAPH_3DOBJECT_JOINT_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_JOINT_SUBEND) )
    {
        if (index>=int(App::ct->objCont->jointList.size()))
            return(false);
        objectID=App::ct->objCont->jointList[index];
        return(true);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_PATH_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_PATH_SUBEND) )
    {
        if (index>=int(App::ct->objCont->pathList.size()))
            return(false);
        objectID=App::ct->objCont->pathList[index];
        return(true);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_PROXSENSOR_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_PROXSENSOR_SUBEND) )
    {
        if (index>=int(App::ct->objCont->proximitySensorList.size()))
            return(false);
        objectID=App::ct->objCont->proximitySensorList[index];
        return(true);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_MILL_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_MILL_SUBEND) )
    {
        if (index>=int(App::ct->objCont->millList.size()))
            return(false);
        objectID=App::ct->objCont->millList[index];
        return(true);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_FORCESENSOR_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_FORCESENSOR_SUBEND) )
    {
        if (index>=int(App::ct->objCont->forceSensorList.size()))
            return(false);
        objectID=App::ct->objCont->forceSensorList[index];
        return(true);
    }
    if ( (dataIndex>GRAPH_3DOBJECT_VISIONSENSOR_SUBSTART)&&(dataIndex<GRAPH_3DOBJECT_VISIONSENSOR_SUBEND) )
    {
        if (index>=int(App::ct->objCont->visionSensorList.size()))
            return(false);
        objectID=App::ct->objCont->visionSensorList[index];
        return(true);
    }

    // FOLLOWING HAS TO COME AFTER ALL OTHER 3DOBJECT SUB-CATEGORIES (e.g. GRAPH_#DOBJECT_JOINT_SUBSTART/SUBEND, etc.)
    if ( (dataIndex>GRAPH_3DOBJECT_START)&&(dataIndex<GRAPH_3DOBJECT_END) )
    {
        if (index>=int(App::ct->objCont->objectList.size()))
            return(false);
        objectID=App::ct->objCont->objectList[index];
        return(true);
    }
    return(false);
}

bool CGraphingRoutines::loopThroughAllAndGetDataName(int dataIndex,std::string& dataName)
{ // If return value is true, dataName contains the text for the dataIndex (used by graphs)
    if (CGraphingRoutines::getDataName(dataIndex,dataName))
        return(true);
    return(false);
}

bool CGraphingRoutines::loopThroughAllAndGetDataValue(int dataIndex,int objectID,float& value,const C7Vector* graphCTM)
{   // If return value is true, value contains the asked value (used by graphs)
    // If return value is false, the value couldn't be found and you have to use
    // the default value!
    // graphCTM is nullptr by default
    if (CGraphingRoutines::getDataValue(dataIndex,objectID,value,graphCTM))
        return(true);
    return(false);
}

bool CGraphingRoutines::loopThroughAllAndGetGraphObjectName(int dataIndex,int objectID,std::string& objName)
{ // Returns the name of an object being graphed (name of 3DObject, sensor, collision partn., etc)
    // objName is valid only if return value is true
    if (CGraphingRoutines::getGraphObjectName(dataIndex,objectID,objName))
        return(true);
    return(false);
}
bool CGraphingRoutines::loopThroughAllAndGetObjectsFromGraphCategory(int index,int dataIndex,int& objectID)
{   // Returns the objectID and auxDat for all objects concerned by the category 'dataIndex'
    // Loop through this function beginning with index=0 and incrementing it as long
    // as the return value is true
    if (CGraphingRoutines::getObjectsFromGraphCategory(index,dataIndex,objectID))
        return(true);
    return(false);
}

void CGraphingRoutines::resetAllGraphs(bool exceptExplicitHandling)
{
    for (int i=0;i<int(App::ct->objCont->graphList.size());i++)
    {
        CGraph* it=App::ct->objCont->getGraph(App::ct->objCont->graphList[i]);
        if (it!=nullptr)
        {
            if ( (!it->getExplicitHandling())||(!exceptExplicitHandling) )
                it->resetGraph();
        }
    }
}

void CGraphingRoutines::handleAllGraphs(bool exceptExplicitHandling,float time)
{
    for (int i=0;i<int(App::ct->objCont->graphList.size());i++)
    {
        CGraph* it=App::ct->objCont->getGraph(App::ct->objCont->graphList[i]);
        if (it!=nullptr)
        {
            if ( (!it->getExplicitHandling())||(!exceptExplicitHandling) )
                it->addNextPoint(time);
        }
    }
}

