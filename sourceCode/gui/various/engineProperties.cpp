#include "engineProperties.h"
#include "app.h"
#include "vMessageBox.h"
#include "annJson.h"
#include "ttUtil.h"

CEngineProperties::CEngineProperties()
{
}

CEngineProperties::~CEngineProperties()
{
}

void CEngineProperties::editObjectProperties(int objectHandle)
{
    CSceneObject* object=App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

    QJsonObject jmain;
    CAnnJson annJson(&jmain);
    std::string title("Dynamic engine global properties");
    int engine=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);

    if (object!=nullptr)
    {
        if (object->getObjectType()==sim_object_shape_type)
        {
            _writeShape(engine,objectHandle,annJson);
            for (size_t i=sim_physics_bullet;i<=sim_physics_mujoco;i++)
            {
                if (i!=engine)
                    _writeShape(i,objectHandle,annJson);
            }
            title="Dynamic engine properties for shape ";
        }
        if (object->getObjectType()==sim_object_joint_type)
        {
            _writeJoint(engine,objectHandle,annJson);
            for (size_t i=sim_physics_bullet;i<=sim_physics_mujoco;i++)
            {
                if (i!=engine)
                    _writeJoint(i,objectHandle,annJson);
            }
            title="Dynamic engine properties for joint ";
        }
        title+=object->getObjectAlias_printPath();
    }
    else
    {
        _writeGlobal(engine,annJson);
        for (size_t i=sim_physics_bullet;i<=sim_physics_mujoco;i++)
        {
            if (i!=engine)
                _writeGlobal(i,annJson);
        }
    }

    std::string initText(annJson.getAnnotatedString());

    //printf("%s:\n%s\n",title.c_str(),initText.c_str());
    QJsonObject obj;
    annJson.setMainObject(&obj);
    std::string modifiedText(initText);
    while(true)
    {
        if (true)
        {
            std::string xml("<editor title=\"");
            xml+=title;
            xml+="\" lang=\"json\" line-numbers=\"true\"/>";
            modifiedText=App::mainWindow->codeEditorContainer->openModalTextEditor(modifiedText.c_str(),xml.c_str(),nullptr);
            //printf("Modified text:\n%s\n",modifiedText.c_str());
        }

        std::string modifiedText_noComments=annJson.stripComments(modifiedText.c_str());

        QJsonParseError parseError;
        QJsonDocument doc=QJsonDocument::fromJson(modifiedText_noComments.c_str(),&parseError);

        if ( (!doc.isNull())&&doc.isObject() )
        {
            obj=doc.object();
            break;
        }

        std::string msg("Invalid JSON data:\n\n");
        msg+=parseError.errorString().toStdString()+"\nat line ";
        msg+=std::to_string(CTTUtil::lineCountAtOffset(modifiedText_noComments.c_str(),parseError.offset))+"\n\nDiscard changes?";
        if (VMESSAGEBOX_REPLY_CANCEL!=App::uiThread->messageBox_warning(App::mainWindow,title.c_str(),msg.c_str(),VMESSAGEBOX_OK_CANCEL,VMESSAGEBOX_REPLY_CANCEL))
            return;
    }
    std::string allErrors;

    if (object!=nullptr)
    {
        if (object->getObjectType()==sim_object_shape_type)
        {
            for (size_t i=sim_physics_bullet;i<=sim_physics_mujoco;i++)
                _readShape(i,objectHandle,annJson,&allErrors);
        }
        if (object->getObjectType()==sim_object_joint_type)
        {
            for (size_t i=sim_physics_bullet;i<=sim_physics_mujoco;i++)
                _readJoint(i,objectHandle,annJson,&allErrors);
        }
    }
    else
    {
        for (size_t i=sim_physics_bullet;i<=sim_physics_mujoco;i++)
            _readGlobal(i,annJson,&allErrors);
    }

    if (allErrors.size()>0)
    {
        allErrors=std::string("The JSON parser found following error(s):\n")+allErrors;
        App::logMsg(sim_verbosity_scripterrors,allErrors.c_str());
    }
}

void CEngineProperties::_writeJoint(int engine,int jointHandle,CAnnJson& annJson)
{
    CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);

    if (engine==sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(jbullet,"normalCFM",joint->getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr));
        annJson.addJson(jbullet,"stopERP",joint->getEngineFloatParam(sim_bullet_joint_stoperp,nullptr));
        annJson.addJson(jbullet,"stopCFM",joint->getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr));
        annJson.addJson(annJson.getMainObject()[0],"bullet",jbullet);
    }

    if (engine==sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(jode,"normalCFM",joint->getEngineFloatParam(sim_ode_joint_normalcfm,nullptr));
        annJson.addJson(jode,"stopERP",joint->getEngineFloatParam(sim_ode_joint_stoperp,nullptr));
        annJson.addJson(jode,"stopCFM",joint->getEngineFloatParam(sim_ode_joint_stopcfm,nullptr));
        annJson.addJson(jode,"bounce",joint->getEngineFloatParam(sim_ode_joint_bounce,nullptr));
        annJson.addJson(jode,"fudgeFactor",joint->getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr));
        annJson.addJson(annJson.getMainObject()[0],"ode",jode);
    }

    if (engine==sim_physics_newton)
    {
        QJsonObject jnewton;
        QJsonObject jnewtonDependency;
        int h=joint->getEngineIntParam(sim_newton_joint_dependentobjectid,nullptr);
        std::string nameAndPath("");
        if (h>=0)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
            nameAndPath=obj->getObjectAlias_shortPath();
        }
        annJson.addJson(jnewtonDependency,"dependentJoint",nameAndPath.c_str(),"specify the full, unique path");
        annJson.addJson(jnewtonDependency,"multiplicationFactor",joint->getEngineFloatParam(sim_newton_joint_dependencyfactor,nullptr));
        annJson.addJson(jnewtonDependency,"offset",joint->getEngineFloatParam(sim_newton_joint_dependencyoffset,nullptr));
        annJson.addJson(jnewton,"jointDependency",jnewtonDependency);
        annJson.addJson(annJson.getMainObject()[0],"newton",jnewton);
    }

    if (engine==sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        QJsonObject jmujocoLimits;
        double v[5];
        v[0]=joint->getEngineFloatParam(sim_mujoco_joint_solreflimit1,nullptr);
        v[1]=joint->getEngineFloatParam(sim_mujoco_joint_solreflimit2,nullptr);
        annJson.addJson(jmujocoLimits,"solref",v,2);
        for (size_t j=0;j<5;j++)
            v[j]=joint->getEngineFloatParam(sim_mujoco_joint_solimplimit1+j,nullptr);
        annJson.addJson(jmujocoLimits,"solimp",v,5);
        annJson.addJson(jmujoco,"limits",jmujocoLimits);
        QJsonObject jmujocoFriction;
        annJson.addJson(jmujocoFriction,"frictionloss",joint->getEngineFloatParam(sim_mujoco_joint_frictionloss,nullptr));
        v[0]=joint->getEngineFloatParam(sim_mujoco_joint_solreffriction1,nullptr);
        v[1]=joint->getEngineFloatParam(sim_mujoco_joint_solreffriction2,nullptr);
        annJson.addJson(jmujocoFriction,"solref",v,2);
        for (size_t j=0;j<5;j++)
            v[j]=joint->getEngineFloatParam(sim_mujoco_joint_solimpfriction1+j,nullptr);
        annJson.addJson(jmujocoFriction,"solimp",v,5);
        annJson.addJson(jmujoco,"friction",jmujocoFriction);
        QJsonObject jmujocoSpring;
        annJson.addJson(jmujocoSpring,"stiffness",joint->getEngineFloatParam(sim_mujoco_joint_stiffness,nullptr));
        annJson.addJson(jmujocoSpring,"damping",joint->getEngineFloatParam(sim_mujoco_joint_damping,nullptr));
        annJson.addJson(jmujocoSpring,"springref",joint->getEngineFloatParam(sim_mujoco_joint_springref,nullptr));
        v[0]=joint->getEngineFloatParam(sim_mujoco_joint_springdamper1,nullptr);
        v[1]=joint->getEngineFloatParam(sim_mujoco_joint_springdamper2,nullptr);
        annJson.addJson(jmujocoSpring,"springdamper",v,2);
        annJson.addJson(jmujoco,"spring",jmujocoSpring);
        annJson.addJson(jmujoco,"armature",joint->getEngineFloatParam(sim_mujoco_joint_armature,nullptr));
        annJson.addJson(jmujoco,"margin",joint->getEngineFloatParam(sim_mujoco_joint_margin,nullptr));
        QJsonObject jmujocoDependency;
        int h=joint->getEngineIntParam(sim_mujoco_joint_dependentobjectid,nullptr);
        std::string nameAndPath;
        if (h>=0)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
            nameAndPath=obj->getObjectAlias_shortPath();
        }
        annJson.addJson(jmujocoDependency,"dependentJoint",nameAndPath.c_str(),"specify the full, unique path");
        for (size_t j=0;j<5;j++)
            v[j]=joint->getEngineFloatParam(sim_mujoco_joint_polycoef1+j,nullptr);
        annJson.addJson(jmujocoDependency,"polycoef",v,5);
        annJson.addJson(jmujoco,"jointDependency",jmujocoDependency);
        annJson.addJson(annJson.getMainObject()[0],"mujoco",jmujoco);
    }

    if (engine==sim_physics_vortex)
    {
        QJsonObject jvortex;
        QJsonObject jvortexAxisFriction;
        annJson.addJson(jvortexAxisFriction,"enabled",joint->getEngineBoolParam(sim_vortex_joint_motorfrictionenabled,nullptr));
        annJson.addJson(jvortexAxisFriction,"proportional",joint->getEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,nullptr));
        annJson.addJson(jvortexAxisFriction,"coeff",joint->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,nullptr));
        annJson.addJson(jvortexAxisFriction,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,nullptr));
        annJson.addJson(jvortexAxisFriction,"loss",joint->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,nullptr));
        annJson.addJson(jvortex,"axisFriction",jvortexAxisFriction);
        QJsonObject jvortexAxisLimits;
        annJson.addJson(jvortexAxisLimits,"lowerRestitution",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,nullptr));
        annJson.addJson(jvortexAxisLimits,"upperRestitution",joint->getEngineFloatParam(sim_vortex_joint_upperlimitrestitution,nullptr));
        annJson.addJson(jvortexAxisLimits,"lowerStiffness",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,nullptr));
        annJson.addJson(jvortexAxisLimits,"upperStiffness",joint->getEngineFloatParam(sim_vortex_joint_upperlimitstiffness,nullptr));
        annJson.addJson(jvortexAxisLimits,"lowerDamping",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitdamping,nullptr));
        annJson.addJson(jvortexAxisLimits,"upperDamping",joint->getEngineFloatParam(sim_vortex_joint_upperlimitdamping,nullptr));
        annJson.addJson(jvortexAxisLimits,"lowerMaxForce",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,nullptr));
        annJson.addJson(jvortexAxisLimits,"upperMaxForce",joint->getEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,nullptr));
        annJson.addJson(jvortex,"axisLimits",jvortexAxisLimits);
        QJsonObject jvortexDependency;
        int h=joint->getEngineIntParam(sim_vortex_joint_dependentobjectid,nullptr);
        std::string nameAndPath;
        if (h>=0)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
            nameAndPath=obj->getObjectAlias_shortPath();
        }
        annJson.addJson(jvortexDependency,"dependentJoint",nameAndPath.c_str(),"specify the full, unique path");
        annJson.addJson(jvortexDependency,"multiplicationFactor",joint->getEngineFloatParam(sim_vortex_joint_dependencyfactor,nullptr));
        annJson.addJson(jvortexDependency,"offset",joint->getEngineFloatParam(sim_vortex_joint_dependencyoffset,nullptr));
        annJson.addJson(jvortex,"jointDependency",jvortexDependency);

        int vval;
        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
        bool P0_relaxation_enabled=((vval&1)!=0);
        bool P1_relaxation_enabled=((vval&2)!=0);
        bool P2_relaxation_enabled=((vval&4)!=0);
        bool A0_relaxation_enabled=((vval&8)!=0);
        bool A1_relaxation_enabled=((vval&16)!=0);
        bool A2_relaxation_enabled=((vval&32)!=0);
        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
        bool P0_friction_enabled=((vval&1)!=0);
        bool P1_friction_enabled=((vval&2)!=0);
        bool P2_friction_enabled=((vval&4)!=0);
        bool A0_friction_enabled=((vval&8)!=0);
        bool A1_friction_enabled=((vval&16)!=0);
        bool A2_friction_enabled=((vval&32)!=0);
        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
        bool P0_friction_proportional=((vval&1)!=0);
        bool P1_friction_proportional=((vval&2)!=0);
        bool P2_friction_proportional=((vval&4)!=0);
        bool A0_friction_proportional=((vval&8)!=0);
        bool A1_friction_proportional=((vval&16)!=0);
        bool A2_friction_proportional=((vval&32)!=0);

        QJsonObject jvortexXaxisPosition;
        QJsonObject jvortexXaxisPositionRelax;
        annJson.addJson(jvortexXaxisPositionRelax,"enabled",P0_relaxation_enabled);
        annJson.addJson(jvortexXaxisPositionRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_p0stiffness,nullptr));
        annJson.addJson(jvortexXaxisPositionRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_p0damping,nullptr));
        annJson.addJson(jvortexXaxisPositionRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_p0loss,nullptr));
        annJson.addJson(jvortexXaxisPosition,"relaxation",jvortexXaxisPositionRelax);
        QJsonObject jvortexXaxisPositionFric;
        annJson.addJson(jvortexXaxisPositionFric,"enabled",P0_friction_enabled);
        annJson.addJson(jvortexXaxisPositionFric,"proportional",P0_friction_proportional);
        annJson.addJson(jvortexXaxisPositionFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_p0frictioncoeff,nullptr));
        annJson.addJson(jvortexXaxisPositionFric,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,nullptr));
        annJson.addJson(jvortexXaxisPositionFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_p0frictionloss,nullptr));
        annJson.addJson(jvortexXaxisPosition,"friction",jvortexXaxisPositionFric);
        annJson.addJson(jvortex,"x-axisPosition",jvortexXaxisPosition);

        QJsonObject jvortexYaxisPosition;
        QJsonObject jvortexYaxisPositionRelax;
        annJson.addJson(jvortexYaxisPositionRelax,"enabled",P1_relaxation_enabled);
        annJson.addJson(jvortexYaxisPositionRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_p1stiffness,nullptr));
        annJson.addJson(jvortexYaxisPositionRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_p1damping,nullptr));
        annJson.addJson(jvortexYaxisPositionRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_p1loss,nullptr));
        annJson.addJson(jvortexYaxisPosition,"relaxation",jvortexYaxisPositionRelax);
        QJsonObject jvortexYaxisPositionFric;
        annJson.addJson(jvortexYaxisPositionFric,"enabled",P1_friction_enabled);
        annJson.addJson(jvortexYaxisPositionFric,"proportional",P1_friction_proportional);
        annJson.addJson(jvortexYaxisPositionFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_p1frictioncoeff,nullptr));
        annJson.addJson(jvortexYaxisPositionFric,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,nullptr));
        annJson.addJson(jvortexYaxisPositionFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_p1frictionloss,nullptr));
        annJson.addJson(jvortexYaxisPosition,"friction",jvortexYaxisPositionFric);
        annJson.addJson(jvortex,"y-axisPosition",jvortexYaxisPosition);

        QJsonObject jvortexZaxisPosition;
        QJsonObject jvortexZaxisPositionRelax;
        annJson.addJson(jvortexZaxisPositionRelax,"enabled",P2_relaxation_enabled);
        annJson.addJson(jvortexZaxisPositionRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_p2stiffness,nullptr));
        annJson.addJson(jvortexZaxisPositionRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_p2damping,nullptr));
        annJson.addJson(jvortexZaxisPositionRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_p2loss,nullptr));
        annJson.addJson(jvortexZaxisPosition,"relaxation",jvortexZaxisPositionRelax);
        QJsonObject jvortexZaxisPositionFric;
        annJson.addJson(jvortexZaxisPositionFric,"enabled",P2_friction_enabled);
        annJson.addJson(jvortexZaxisPositionFric,"proportional",P2_friction_proportional);
        annJson.addJson(jvortexZaxisPositionFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_p2frictioncoeff,nullptr));
        annJson.addJson(jvortexZaxisPositionFric,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,nullptr));
        annJson.addJson(jvortexZaxisPositionFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_p2frictionloss,nullptr));
        annJson.addJson(jvortexZaxisPosition,"friction",jvortexZaxisPositionFric);
        annJson.addJson(jvortex,"z-axisPosition",jvortexZaxisPosition);

        QJsonObject jvortexXaxisOrientation;
        QJsonObject jvortexXaxisOrientationRelax;
        annJson.addJson(jvortexXaxisOrientationRelax,"enabled",A0_relaxation_enabled);
        annJson.addJson(jvortexXaxisOrientationRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_a0stiffness,nullptr));
        annJson.addJson(jvortexXaxisOrientationRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_a0damping,nullptr));
        annJson.addJson(jvortexXaxisOrientationRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_a0loss,nullptr));
        annJson.addJson(jvortexXaxisOrientation,"relaxation",jvortexXaxisOrientationRelax);
        QJsonObject jvortexXaxisOrientationFric;
        annJson.addJson(jvortexXaxisOrientationFric,"enabled",A0_friction_enabled);
        annJson.addJson(jvortexXaxisOrientationFric,"proportional",A0_friction_proportional);
        annJson.addJson(jvortexXaxisOrientationFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_a0frictioncoeff,nullptr));
        annJson.addJson(jvortexXaxisOrientationFric,"maxTorque",joint->getEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,nullptr));
        annJson.addJson(jvortexXaxisOrientationFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_a0frictionloss,nullptr));
        annJson.addJson(jvortexXaxisOrientation,"friction",jvortexXaxisOrientationFric);
        annJson.addJson(jvortex,"x-axisOrientation",jvortexXaxisOrientation);

        QJsonObject jvortexYaxisOrientation;
        QJsonObject jvortexYaxisOrientationRelax;
        annJson.addJson(jvortexYaxisOrientationRelax,"enabled",A1_relaxation_enabled);
        annJson.addJson(jvortexYaxisOrientationRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_a1stiffness,nullptr));
        annJson.addJson(jvortexYaxisOrientationRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_a1damping,nullptr));
        annJson.addJson(jvortexYaxisOrientationRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_a1loss,nullptr));
        annJson.addJson(jvortexYaxisOrientation,"relaxation",jvortexYaxisOrientationRelax);
        QJsonObject jvortexYaxisOrientationFric;
        annJson.addJson(jvortexYaxisOrientationFric,"enabled",A1_friction_enabled);
        annJson.addJson(jvortexYaxisOrientationFric,"proportional",A1_friction_proportional);
        annJson.addJson(jvortexYaxisOrientationFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_a1frictioncoeff,nullptr));
        annJson.addJson(jvortexYaxisOrientationFric,"maxTorque",joint->getEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,nullptr));
        annJson.addJson(jvortexYaxisOrientationFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_a1frictionloss,nullptr));
        annJson.addJson(jvortexYaxisOrientation,"friction",jvortexYaxisOrientationFric);
        annJson.addJson(jvortex,"y-axisOrientation",jvortexYaxisOrientation);

        QJsonObject jvortexZaxisOrientation;
        QJsonObject jvortexZaxisOrientationRelax;
        annJson.addJson(jvortexZaxisOrientationRelax,"enabled",A2_relaxation_enabled);
        annJson.addJson(jvortexZaxisOrientationRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_a2stiffness,nullptr));
        annJson.addJson(jvortexZaxisOrientationRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_a2damping,nullptr));
        annJson.addJson(jvortexZaxisOrientationRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_a2loss,nullptr));
        annJson.addJson(jvortexZaxisOrientation,"relaxation",jvortexZaxisOrientationRelax);
        QJsonObject jvortexZaxisOrientationFric;
        annJson.addJson(jvortexZaxisOrientationFric,"enabled",A2_friction_enabled);
        annJson.addJson(jvortexZaxisOrientationFric,"proportional",A2_friction_proportional);
        annJson.addJson(jvortexZaxisOrientationFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_a2frictioncoeff,nullptr));
        annJson.addJson(jvortexZaxisOrientationFric,"maxTorque",joint->getEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,nullptr));
        annJson.addJson(jvortexZaxisOrientationFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_a2frictionloss,nullptr));
        annJson.addJson(jvortexZaxisOrientation,"friction",jvortexZaxisOrientationFric);
        annJson.addJson(jvortex,"z-axisOrientation",jvortexZaxisOrientation);

        annJson.addJson(annJson.getMainObject()[0],"vortex",jvortex);
    }
}

void CEngineProperties::_readJoint(int engine,int jointHandle,CAnnJson& annJson,std::string* allErrors)
{
    CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
    QJsonValue val;

    if (engine==sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"bullet",QJsonValue::Object,val,allErrors))
        {
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet,"normalCFM",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_bullet_joint_normalcfm,val.toDouble());
            if (annJson.getValue(bullet,"stopERP",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_bullet_joint_stoperp,val.toDouble());
            if (annJson.getValue(bullet,"stopCFM",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_bullet_joint_stopcfm,val.toDouble());
        }
    }

    if (engine==sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"ode",QJsonValue::Object,val,allErrors))
        {
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode,"normalCFM",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_ode_joint_normalcfm,val.toDouble());
            if (annJson.getValue(ode,"stopERP",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_ode_joint_stoperp,val.toDouble());
            if (annJson.getValue(ode,"stopCFM",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_ode_joint_stopcfm,val.toDouble());
            if (annJson.getValue(ode,"bounce",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_ode_joint_bounce,val.toDouble());
            if (annJson.getValue(ode,"fudgeFactor",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_ode_joint_fudgefactor,val.toDouble());
        }
    }

    if (engine==sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"newton",QJsonValue::Object,val,allErrors))
        {
            QJsonObject newton(val.toObject());
            if (annJson.getValue(newton,"jointDependency",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"dependentJoint",QJsonValue::String,val,allErrors))
                {
                    int h=-1;
                    std::string str(val.toString().toStdString());
                    if (str.size()>0)
                    {
                        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,str.c_str(),0,nullptr);
                        if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_joint_type)&&(obj->getObjectHandle()!=joint->getObjectHandle()) )
                        {
                            h=obj->getObjectHandle();
                            joint->setEngineIntParam(sim_newton_joint_dependentobjectid,h);
                        }
                        else
                        {
                            if (allErrors->size()>0)
                                allErrors[0]+="\n";
                            allErrors[0]+="Key 'dependentJoint' does not point to a valid joint object and will be ignored.";
                        }
                    }
                    else
                        joint->setEngineIntParam(sim_newton_joint_dependentobjectid,h);
                }
                if (annJson.getValue(sub,"multiplicationFactor",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_newton_joint_dependencyfactor,val.toDouble());
                if (annJson.getValue(sub,"offset",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_newton_joint_dependencyoffset,val.toDouble());
            }
        }
    }

    if (engine==sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"mujoco",QJsonValue::Object,val,allErrors))
        {
            QJsonObject mujoco(val.toObject());
            double w[5];
            if (annJson.getValue(mujoco,"limits",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"solref",w,2,allErrors))
                {
                    joint->setEngineFloatParam(sim_mujoco_joint_solreflimit1,w[0]);
                    joint->setEngineFloatParam(sim_mujoco_joint_solreflimit2,w[1]);
                }
                if (annJson.getValue(sub,"solimp",w,5,allErrors))
                {
                    for (size_t j=0;j<5;j++)
                        joint->setEngineFloatParam(sim_mujoco_joint_solimplimit1+j,w[j]);
                }
            }
            if (annJson.getValue(mujoco,"friction",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"frictionloss",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_mujoco_joint_frictionloss,val.toDouble());
                if (annJson.getValue(sub,"solref",w,2,allErrors))
                {
                    joint->setEngineFloatParam(sim_mujoco_joint_solreffriction1,w[0]);
                    joint->setEngineFloatParam(sim_mujoco_joint_solreffriction2,w[1]);
                }
                if (annJson.getValue(sub,"solimp",w,5,allErrors))
                {
                    for (size_t j=0;j<5;j++)
                        joint->setEngineFloatParam(sim_mujoco_joint_solimpfriction1+j,w[j]);
                }
            }
            if (annJson.getValue(mujoco,"spring",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"stiffness",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_mujoco_joint_stiffness,val.toDouble());
                if (annJson.getValue(sub,"damping",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_mujoco_joint_damping,val.toDouble());
                if (annJson.getValue(sub,"springref",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_mujoco_joint_springref,val.toDouble());
                if (annJson.getValue(sub,"springdamper",w,2,allErrors))
                {
                    joint->setEngineFloatParam(sim_mujoco_joint_springdamper1,w[0]);
                    joint->setEngineFloatParam(sim_mujoco_joint_springdamper2,w[1]);
                }
            }
            if (annJson.getValue(mujoco,"armature",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_mujoco_joint_armature,val.toDouble());
            if (annJson.getValue(mujoco,"margin",QJsonValue::Double,val,allErrors))
                joint->setEngineFloatParam(sim_mujoco_joint_margin,val.toDouble());
            if (annJson.getValue(mujoco,"jointDependency",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"dependentJoint",QJsonValue::String,val,allErrors))
                {
                    int h=-1;
                    std::string str(val.toString().toStdString());
                    if (str.size()>0)
                    {
                        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,str.c_str(),0,nullptr);
                        if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_joint_type)&&(obj->getObjectHandle()!=joint->getObjectHandle()) )
                        {
                            h=obj->getObjectHandle();
                            joint->setEngineIntParam(sim_mujoco_joint_dependentobjectid,h);
                        }
                        else
                        {
                            if (allErrors->size()>0)
                                allErrors[0]+="\n";
                            allErrors[0]+="Key 'dependentJoint' does not point to a valid joint object and will be ignored.";
                        }
                    }
                    else
                        joint->setEngineIntParam(sim_mujoco_joint_dependentobjectid,h);
                }
                if (annJson.getValue(sub,"polycoef",w,5,allErrors))
                {
                    for (size_t j=0;j<5;j++)
                        joint->setEngineFloatParam(sim_mujoco_joint_polycoef1+j,w[j]);
                }
            }
        }
    }

    if (engine==sim_physics_vortex)
    {
        int vval;
        if (annJson.getValue(annJson.getMainObject()[0],"vortex",QJsonValue::Object,val,allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex,"axisFriction",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,allErrors))
                    joint->setEngineBoolParam(sim_vortex_joint_motorfrictionenabled,val.toBool());
                if (annJson.getValue(sub,"proportional",QJsonValue::Bool,val,allErrors))
                    joint->setEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,val.toBool());
                if (annJson.getValue(sub,"coeff",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,val.toDouble());
                if (annJson.getValue(sub,"maxForce",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,val.toDouble());
                if (annJson.getValue(sub,"loss",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,val.toDouble());
            }
            if (annJson.getValue(vortex,"axisLimits",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"lowerRestitution",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,val.toDouble());
                if (annJson.getValue(sub,"upperRestitution",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_upperlimitrestitution,val.toDouble());
                if (annJson.getValue(sub,"lowerStiffness",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,val.toDouble());
                if (annJson.getValue(sub,"upperStiffness",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_upperlimitstiffness,val.toDouble());
                if (annJson.getValue(sub,"lowerDamping",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_lowerlimitdamping,val.toDouble());
                if (annJson.getValue(sub,"upperDamping",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_upperlimitdamping,val.toDouble());
                if (annJson.getValue(sub,"lowerMaxForce",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,val.toDouble());
                if (annJson.getValue(sub,"upperMaxForce",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,val.toDouble());
            }
            if (annJson.getValue(vortex,"jointDependency",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"dependentJoint",QJsonValue::String,val,allErrors))
                {
                    int h=-1;
                    std::string str(val.toString().toStdString());
                    if (str.size()>0)
                    {
                        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,str.c_str(),0,nullptr);
                        if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_joint_type)&&(obj->getObjectHandle()!=joint->getObjectHandle()) )
                        {
                            h=obj->getObjectHandle();
                            joint->setEngineIntParam(sim_vortex_joint_dependentobjectid,h);
                        }
                        else
                        {
                            if (allErrors->size()>0)
                                allErrors[0]+="\n";
                            allErrors[0]+="Key 'dependentJoint' does not point to a valid joint object and will be ignored.";
                        }
                    }
                    else
                        joint->setEngineIntParam(sim_vortex_joint_dependentobjectid,h);
                }
                if (annJson.getValue(sub,"multiplicationFactor",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_dependencyfactor,val.toDouble());
                if (annJson.getValue(sub,"offset",QJsonValue::Double,val,allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_dependencyoffset,val.toDouble());
            }
            if (annJson.getValue(vortex,"x-axisPosition",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                        vval=(vval|1)-1;
                        if (val.toBool())
                            vval|=1;
                        joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p0stiffness,val.toDouble());
                    if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p0damping,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p0loss,val.toDouble());
                }
                if (annJson.getValue(sub,"friction",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                        vval=(vval|1)-1;
                        if (val.toBool())
                            vval|=1;
                        joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                        vval=(vval|1)-1;
                        if (val.toBool())
                            vval|=1;
                        joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                    }
                    if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p0frictioncoeff,val.toDouble());
                    if (annJson.getValue(sub2,"maxForce",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p0frictionloss,val.toDouble());
                }
            }
            if (annJson.getValue(vortex,"y-axisPosition",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                        vval=(vval|2)-2;
                        if (val.toBool())
                            vval|=2;
                        joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p1stiffness,val.toDouble());
                    if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p1damping,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p1loss,val.toDouble());
                }
                if (annJson.getValue(sub,"friction",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                        vval=(vval|2)-2;
                        if (val.toBool())
                            vval|=2;
                        joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                        vval=(vval|2)-2;
                        if (val.toBool())
                            vval|=2;
                        joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                    }
                    if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p1frictioncoeff,val.toDouble());
                    if (annJson.getValue(sub2,"maxForce",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p1frictionloss,val.toDouble());
                }
            }
            if (annJson.getValue(vortex,"z-axisPosition",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                        vval=(vval|4)-4;
                        if (val.toBool())
                            vval|=4;
                        joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p2stiffness,val.toDouble());
                    if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p2damping,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p2loss,val.toDouble());
                }
                if (annJson.getValue(sub,"friction",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                        vval=(vval|4)-4;
                        if (val.toBool())
                            vval|=4;
                        joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                        vval=(vval|4)-4;
                        if (val.toBool())
                            vval|=4;
                        joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                    }
                    if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p2frictioncoeff,val.toDouble());
                    if (annJson.getValue(sub2,"maxForce",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_p2frictionloss,val.toDouble());
                }
            }
            if (annJson.getValue(vortex,"x-axisOrientation",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                        vval=(vval|8)-8;
                        if (val.toBool())
                            vval|=8;
                        joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a0stiffness,val.toDouble());
                    if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a0damping,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a0loss,val.toDouble());
                }
                if (annJson.getValue(sub,"friction",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                        vval=(vval|8)-8;
                        if (val.toBool())
                            vval|=8;
                        joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                        vval=(vval|8)-8;
                        if (val.toBool())
                            vval|=8;
                        joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                    }
                    if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a0frictioncoeff,val.toDouble());
                    if (annJson.getValue(sub2,"maxTorque",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a0frictionloss,val.toDouble());
                }
            }
            if (annJson.getValue(vortex,"y-axisOrientation",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                        vval=(vval|16)-16;
                        if (val.toBool())
                            vval|=16;
                        joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a1stiffness,val.toDouble());
                    if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a1damping,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a1loss,val.toDouble());
                }
                if (annJson.getValue(sub,"friction",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                        vval=(vval|16)-16;
                        if (val.toBool())
                            vval|=16;
                        joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                        vval=(vval|16)-16;
                        if (val.toBool())
                            vval|=16;
                        joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                    }
                    if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a1frictioncoeff,val.toDouble());
                    if (annJson.getValue(sub2,"maxTorque",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a1frictionloss,val.toDouble());
                }
            }
            if (annJson.getValue(vortex,"z-axisOrientation",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                        vval=(vval|32)-32;
                        if (val.toBool())
                            vval|=32;
                        joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a2stiffness,val.toDouble());
                    if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a2damping,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a2loss,val.toDouble());
                }
                if (annJson.getValue(sub,"friction",QJsonValue::Object,val,allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                        vval=(vval|32)-32;
                        if (val.toBool())
                            vval|=32;
                        joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                    }
                    if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,allErrors))
                    {
                        vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                        vval=(vval|32)-32;
                        if (val.toBool())
                            vval|=32;
                        joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                    }
                    if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a2frictioncoeff,val.toDouble());
                    if (annJson.getValue(sub2,"maxTorque",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,val.toDouble());
                    if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,allErrors))
                        joint->setEngineFloatParam(sim_vortex_joint_a2frictionloss,val.toDouble());
                }
            }
        }
    }
}

void CEngineProperties::_writeShape(int engine,int shapeHandle,CAnnJson& annJson)
{
    CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    CDynMaterialObject* mat=shape->getDynMaterial();

    if (engine==sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(jbullet,"stickyContact_v278",mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr),"only Bullet V2.78");
        annJson.addJson(jbullet,"friction_v278",mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr),"only Bullet V2.78");
        annJson.addJson(jbullet,"friction",mat->getEngineFloatParam(sim_bullet_body_friction,nullptr),"only Bullet V2.83 and later");
        annJson.addJson(jbullet,"restitution",mat->getEngineFloatParam(sim_bullet_body_restitution,nullptr));
        annJson.addJson(jbullet,"linearDamping",mat->getEngineFloatParam(sim_bullet_body_lineardamping,nullptr));
        annJson.addJson(jbullet,"angularDamping",mat->getEngineFloatParam(sim_bullet_body_angulardamping,nullptr));
        QJsonObject jbulletCollMargin;
        annJson.addJson(jbulletCollMargin,"enabled",mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,nullptr));
        annJson.addJson(jbulletCollMargin,"margin",mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,nullptr));
        annJson.addJson(jbullet,"customCollisionMargin",jbulletCollMargin);
        QJsonObject jbulletCollMarginConvex;
        annJson.addJson(jbulletCollMarginConvex,"enabled",mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,nullptr));
        annJson.addJson(jbulletCollMarginConvex,"margin",mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,nullptr));
        annJson.addJson(jbullet,"customCollisionMargin_convex",jbulletCollMarginConvex);
        annJson.addJson(jbullet,"autoShrinkConvexMeshes",mat->getEngineBoolParam(sim_bullet_body_autoshrinkconvex,nullptr));
        annJson.addJson(annJson.getMainObject()[0],"bullet",jbullet);
    }

    if (engine==sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(jode,"friction",mat->getEngineFloatParam(sim_ode_body_friction,nullptr));
        annJson.addJson(jode,"softERP",mat->getEngineFloatParam(sim_ode_body_softerp,nullptr));
        annJson.addJson(jode,"softCFM",mat->getEngineFloatParam(sim_ode_body_softcfm,nullptr));
        annJson.addJson(jode,"linearDamping",mat->getEngineFloatParam(sim_ode_body_lineardamping,nullptr));
        annJson.addJson(jode,"angularDamping",mat->getEngineFloatParam(sim_ode_body_angulardamping,nullptr));
        annJson.addJson(jode,"maxContacts",mat->getEngineIntParam(sim_ode_body_maxcontacts,nullptr));
        annJson.addJson(annJson.getMainObject()[0],"ode",jode);
    }

    if (engine==sim_physics_newton)
    {
        QJsonObject jnewton;
        annJson.addJson(jnewton,"staticFriction",mat->getEngineFloatParam(sim_newton_body_staticfriction,nullptr));
        annJson.addJson(jnewton,"kineticFriction",mat->getEngineFloatParam(sim_newton_body_kineticfriction,nullptr));
        annJson.addJson(jnewton,"restitution",mat->getEngineFloatParam(sim_newton_body_restitution,nullptr));
        annJson.addJson(jnewton,"linearDrag",mat->getEngineFloatParam(sim_newton_body_lineardrag,nullptr));
        annJson.addJson(jnewton,"angularDrag",mat->getEngineFloatParam(sim_newton_body_angulardrag,nullptr));
        annJson.addJson(jnewton,"fastMoving",mat->getEngineBoolParam(sim_newton_body_fastmoving,nullptr));
        annJson.addJson(annJson.getMainObject()[0],"newton",jnewton);
    }

    if (engine==sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        double v[5];
        v[0]=mat->getEngineFloatParam(sim_mujoco_body_friction1,nullptr);
        v[1]=mat->getEngineFloatParam(sim_mujoco_body_friction2,nullptr);
        v[2]=mat->getEngineFloatParam(sim_mujoco_body_friction3,nullptr);
        annJson.addJson(jmujoco,"friction",v,3);
        v[0]=mat->getEngineFloatParam(sim_mujoco_body_solref1,nullptr);
        v[1]=mat->getEngineFloatParam(sim_mujoco_body_solref2,nullptr);
        annJson.addJson(jmujoco,"solref",v,2);
        v[0]=mat->getEngineFloatParam(sim_mujoco_body_solimp1,nullptr);
        v[1]=mat->getEngineFloatParam(sim_mujoco_body_solimp2,nullptr);
        v[2]=mat->getEngineFloatParam(sim_mujoco_body_solimp3,nullptr);
        v[3]=mat->getEngineFloatParam(sim_mujoco_body_solimp4,nullptr);
        v[4]=mat->getEngineFloatParam(sim_mujoco_body_solimp5,nullptr);
        annJson.addJson(jmujoco,"solimp",v,5);
        annJson.addJson(jmujoco,"condim",mat->getEngineIntParam(sim_mujoco_body_condim,nullptr));
        annJson.addJson(jmujoco,"solmix",mat->getEngineFloatParam(sim_mujoco_body_solmix,nullptr));
        annJson.addJson(annJson.getMainObject()[0],"mujoco",jmujoco);
    }

    if (engine==sim_physics_vortex)
    {
        QJsonObject jvortex;
        annJson.addJson(jvortex,"restitution",mat->getEngineFloatParam(sim_vortex_body_restitution,nullptr));
        annJson.addJson(jvortex,"restitutionThreshold",mat->getEngineFloatParam(sim_vortex_body_restitutionthreshold,nullptr));
        annJson.addJson(jvortex,"compliance",mat->getEngineFloatParam(sim_vortex_body_compliance,nullptr));
        annJson.addJson(jvortex,"damping",mat->getEngineFloatParam(sim_vortex_body_damping,nullptr));
        annJson.addJson(jvortex,"adhesiveForce",mat->getEngineFloatParam(sim_vortex_body_adhesiveforce,nullptr));
        annJson.addJson(jvortex,"linearVelocityDamping",mat->getEngineFloatParam(sim_vortex_body_linearvelocitydamping,nullptr));
        annJson.addJson(jvortex,"angularVelocityDamping",mat->getEngineFloatParam(sim_vortex_body_angularvelocitydamping,nullptr));
        QJsonObject jvortexAutoAngularDamping;
        annJson.addJson(jvortexAutoAngularDamping,"enabled",mat->getEngineBoolParam(sim_vortex_body_autoangulardamping,nullptr));
        annJson.addJson(jvortexAutoAngularDamping,"tensionRatio",mat->getEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio,nullptr));
        annJson.addJson(jvortex,"autoAngularDamping",jvortexAutoAngularDamping);
        annJson.addJson(jvortex,"skinThickness",mat->getEngineFloatParam(sim_vortex_body_skinthickness,nullptr));
        annJson.addJson(jvortex,"autoSlip",mat->getEngineBoolParam(sim_vortex_body_autoslip,nullptr));
        annJson.addJson(jvortex,"fastMoving",mat->getEngineBoolParam(sim_vortex_body_fastmoving,nullptr));
        annJson.addJson(jvortex,"pureShapesAsConvex",mat->getEngineBoolParam(sim_vortex_body_pureshapesasconvex,nullptr));
        annJson.addJson(jvortex,"convexShapesAsVxTriangleMeshBvTree",mat->getEngineBoolParam(sim_vortex_body_convexshapesasrandom,nullptr));
        annJson.addJson(jvortex,"randomShapesAsVxTriangleMeshUvGrid",mat->getEngineBoolParam(sim_vortex_body_randomshapesasterrain,nullptr));
        QJsonObject jvortexautoSleep;
        annJson.addJson(jvortexautoSleep,"linearSpeed",mat->getEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold,nullptr));
        annJson.addJson(jvortexautoSleep,"linearAccel",mat->getEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold,nullptr));
        annJson.addJson(jvortexautoSleep,"angularSpeed",mat->getEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold,nullptr));
        annJson.addJson(jvortexautoSleep,"angularAccel",mat->getEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold,nullptr));
        annJson.addJson(jvortexautoSleep,"steps",mat->getEngineIntParam(sim_vortex_body_autosleepsteplivethreshold,nullptr));
        annJson.addJson(jvortex,"autoSleepThreshold",jvortexautoSleep);
        std::string fricModelInfo("0=box, 1=scaled box, 2=prop low, 3=prop high, 4=scaled box fast, 5=neutral, 6=none");
        QJsonObject jvortexlinPrimAxis;
        double v[3];
        v[0]=mat->getEngineFloatParam(sim_vortex_body_primaxisvectorx,nullptr);
        v[1]=mat->getEngineFloatParam(sim_vortex_body_primaxisvectory,nullptr);
        v[2]=mat->getEngineFloatParam(sim_vortex_body_primaxisvectorz,nullptr);
        annJson.addJson(jvortexlinPrimAxis,"axisOrientation",v,3);
        annJson.addJson(jvortexlinPrimAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,nullptr),fricModelInfo.c_str());
        annJson.addJson(jvortexlinPrimAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisfriction,nullptr));
        annJson.addJson(jvortexlinPrimAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,nullptr));
        annJson.addJson(jvortexlinPrimAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslip,nullptr));
        annJson.addJson(jvortexlinPrimAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslide,nullptr));
        annJson.addJson(jvortex,"linearPrimaryAxis",jvortexlinPrimAxis);
        QJsonObject jvortexlinSecAxis;
        annJson.addJson(jvortexlinSecAxis,"sameValuesAsPrimaryAxis",mat->getEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,nullptr));
        annJson.addJson(jvortexlinSecAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,nullptr),fricModelInfo.c_str());
        annJson.addJson(jvortexlinSecAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisfriction,nullptr));
        annJson.addJson(jvortexlinSecAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,nullptr));
        annJson.addJson(jvortexlinSecAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisslip,nullptr));
        annJson.addJson(jvortexlinSecAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisslide,nullptr));
        annJson.addJson(jvortex,"linearSecondaryAxis",jvortexlinSecAxis);
        QJsonObject jvortexangPrimAxis;
        annJson.addJson(jvortexangPrimAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,nullptr),fricModelInfo.c_str());
        annJson.addJson(jvortexangPrimAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_primangularaxisfriction,nullptr));
        annJson.addJson(jvortexangPrimAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,nullptr));
        annJson.addJson(jvortexangPrimAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_primangularaxisslip,nullptr));
        annJson.addJson(jvortexangPrimAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_primangularaxisslide,nullptr));
        annJson.addJson(jvortex,"angularPrimaryAxis",jvortexangPrimAxis);
        QJsonObject jvortexangSecAxis;
        annJson.addJson(jvortexangSecAxis,"sameValuesAsPrimaryAxis",mat->getEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,nullptr));
        annJson.addJson(jvortexangSecAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,nullptr),fricModelInfo.c_str());
        annJson.addJson(jvortexangSecAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_secangularaxisfriction,nullptr));
        annJson.addJson(jvortexangSecAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,nullptr));
        annJson.addJson(jvortexangSecAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_secangularaxisslip,nullptr));
        annJson.addJson(jvortexangSecAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_secangularaxisslide,nullptr));
        annJson.addJson(jvortex,"angularSecondaryAxis",jvortexangSecAxis);
        QJsonObject jvortexangNormAxis;
        annJson.addJson(jvortexangNormAxis,"sameValuesAsPrimaryAxis",mat->getEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,nullptr));
        annJson.addJson(jvortexangNormAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,nullptr),fricModelInfo.c_str());
        annJson.addJson(jvortexangNormAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisfriction,nullptr));
        annJson.addJson(jvortexangNormAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,nullptr));
        annJson.addJson(jvortexangNormAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisslip,nullptr));
        annJson.addJson(jvortexangNormAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisslide,nullptr));
        annJson.addJson(jvortex,"angularNormalAxis",jvortexangNormAxis);
        annJson.addJson(annJson.getMainObject()[0],"vortex",jvortex);
    }
}

void CEngineProperties::_readShape(int engine,int shapeHandle,CAnnJson& annJson,std::string* allErrors)
{
    CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    CDynMaterialObject* mat=shape->getDynMaterial();
    QJsonValue val;

    if (engine==sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"bullet",QJsonValue::Object,val,allErrors))
        {
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet,"stickyContact_v278",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_bullet_body_sticky,val.toBool());
            if (annJson.getValue(bullet,"friction_v278",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_bullet_body_oldfriction,val.toDouble());
            if (annJson.getValue(bullet,"friction",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_bullet_body_friction,val.toDouble());
            if (annJson.getValue(bullet,"restitution",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_bullet_body_restitution,val.toDouble());
            if (annJson.getValue(bullet,"linearDamping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_bullet_body_lineardamping,val.toDouble());
            if (annJson.getValue(bullet,"angularDamping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_bullet_body_angulardamping,val.toDouble());
            if (annJson.getValue(bullet,"customCollisionMargin",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,allErrors))
                    mat->setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,val.toBool());
                if (annJson.getValue(sub,"margin",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,val.toDouble());
            }
            if (annJson.getValue(bullet,"customCollisionMargin_convex",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,allErrors))
                    mat->setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,val.toBool());
                if (annJson.getValue(sub,"margin",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,val.toDouble());
            }
            if (annJson.getValue(bullet,"autoShrinkConvexMeshes",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_bullet_body_autoshrinkconvex,val.toBool());
        }
    }

    if (engine==sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"ode",QJsonValue::Object,val,allErrors))
        {
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode,"friction",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_ode_body_friction,val.toDouble());
            if (annJson.getValue(ode,"softERP",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_ode_body_softerp,val.toDouble());
            if (annJson.getValue(ode,"softCFM",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_ode_body_softcfm,val.toDouble());
            if (annJson.getValue(ode,"linearDamping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_ode_body_lineardamping,val.toDouble());
            if (annJson.getValue(ode,"angularDamping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_ode_body_angulardamping,val.toDouble());
            if (annJson.getValue(ode,"maxContacts",QJsonValue::Double,val,allErrors))
                mat->setEngineIntParam(sim_ode_body_maxcontacts,val.toInt());
        }
    }

    if (engine==sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"newton",QJsonValue::Object,val,allErrors))
        {
            QJsonObject newton(val.toObject());
            if (annJson.getValue(newton,"staticFriction",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_newton_body_staticfriction,val.toDouble());
            if (annJson.getValue(newton,"kineticFriction",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_newton_body_kineticfriction,val.toDouble());
            if (annJson.getValue(newton,"restitution",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_newton_body_restitution,val.toDouble());
            if (annJson.getValue(newton,"linearDrag",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_newton_body_lineardrag,val.toDouble());
            if (annJson.getValue(newton,"angularDrag",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_newton_body_angulardrag,val.toDouble());
            if (annJson.getValue(newton,"fastMoving",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_newton_body_fastmoving,val.toBool());
        }
    }

    if (engine==sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"mujoco",QJsonValue::Object,val,allErrors))
        {
            QJsonObject mujoco(val.toObject());
            double v[5];
            if (annJson.getValue(mujoco,"friction",v,3,allErrors))
            {
                mat->setEngineFloatParam(sim_mujoco_body_friction1,v[0]);
                mat->setEngineFloatParam(sim_mujoco_body_friction2,v[1]);
                mat->setEngineFloatParam(sim_mujoco_body_friction3,v[2]);
            }
            if (annJson.getValue(mujoco,"solref",v,2,allErrors))
            {
                mat->setEngineFloatParam(sim_mujoco_body_solref1,v[0]);
                mat->setEngineFloatParam(sim_mujoco_body_solref2,v[1]);
            }
            if (annJson.getValue(mujoco,"solimp",v,5,allErrors))
            {
                mat->setEngineFloatParam(sim_mujoco_body_solimp1,v[0]);
                mat->setEngineFloatParam(sim_mujoco_body_solimp2,v[1]);
                mat->setEngineFloatParam(sim_mujoco_body_solimp3,v[2]);
                mat->setEngineFloatParam(sim_mujoco_body_solimp4,v[3]);
                mat->setEngineFloatParam(sim_mujoco_body_solimp5,v[4]);
            }
            if (annJson.getValue(mujoco,"condim",QJsonValue::Double,val,allErrors))
                mat->setEngineIntParam(sim_mujoco_body_condim,val.toInt());
            if (annJson.getValue(mujoco,"solmix",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_mujoco_body_solmix,val.toDouble());
        }
    }

    if (engine==sim_physics_vortex)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"vortex",QJsonValue::Object,val,allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex,"restitution",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_restitution,val.toDouble());
            if (annJson.getValue(vortex,"restitutionThreshold",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_restitutionthreshold,val.toDouble());
            if (annJson.getValue(vortex,"compliance",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_compliance,val.toDouble());
            if (annJson.getValue(vortex,"damping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_damping,val.toDouble());
            if (annJson.getValue(vortex,"adhesiveForce",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_adhesiveforce,val.toDouble());
            if (annJson.getValue(vortex,"linearVelocityDamping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_linearvelocitydamping,val.toDouble());
            if (annJson.getValue(vortex,"angularVelocityDamping",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_angularvelocitydamping,val.toDouble());
            if (annJson.getValue(vortex,"autoAngularDamping",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,allErrors))
                    mat->setEngineBoolParam(sim_vortex_body_autoangulardamping,val.toBool());
                if (annJson.getValue(sub,"tensionRatio",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio,val.toDouble());
            }
            if (annJson.getValue(vortex,"skinThickness",QJsonValue::Double,val,allErrors))
                mat->setEngineFloatParam(sim_vortex_body_skinthickness,val.toDouble());
            if (annJson.getValue(vortex,"autoSlip",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_vortex_body_autoslip,val.toBool());
            if (annJson.getValue(vortex,"fastMoving",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_vortex_body_fastmoving,val.toBool());
            if (annJson.getValue(vortex,"pureShapesAsConvex",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_vortex_body_pureshapesasconvex,val.toBool());
            if (annJson.getValue(vortex,"convexShapesAsVxTriangleMeshBvTree",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_vortex_body_convexshapesasrandom,val.toBool());
            if (annJson.getValue(vortex,"randomShapesAsVxTriangleMeshUvGrid",QJsonValue::Bool,val,allErrors))
                mat->setEngineBoolParam(sim_vortex_body_randomshapesasterrain,val.toBool());
            if (annJson.getValue(vortex,"autoSleepThreshold",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"linearSpeed",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold,val.toDouble());
                if (annJson.getValue(sub,"linearAccel",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold,val.toDouble());
                if (annJson.getValue(sub,"angularSpeed",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold,val.toDouble());
                if (annJson.getValue(sub,"angularAccel",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold,val.toDouble());
                if (annJson.getValue(sub,"steps",QJsonValue::Double,val,allErrors))
                    mat->setEngineIntParam(sim_vortex_body_autosleepsteplivethreshold,val.toInt());
            }
            if (annJson.getValue(vortex,"linearPrimaryAxis",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                double v[3];
                if (annJson.getValue(sub,"axisOrientation",v,3,allErrors))
                {
                    mat->setEngineFloatParam(sim_vortex_body_primaxisvectorx,v[0]);
                    mat->setEngineFloatParam(sim_vortex_body_primaxisvectory,v[1]);
                    mat->setEngineFloatParam(sim_vortex_body_primaxisvectorz,v[2]);
                }
                if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,allErrors))
                    mat->setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,val.toInt());
                if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,val.toDouble());
                if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,val.toDouble());
                if (annJson.getValue(sub,"slip",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primlinearaxisslip,val.toDouble());
                if (annJson.getValue(sub,"slide",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primlinearaxisslide,val.toDouble());
            }
            if (annJson.getValue(vortex,"linearSecondaryAxis",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"sameValuesAsPrimaryAxis",QJsonValue::Bool,val,allErrors))
                    mat->setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,val.toInt());
                if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,allErrors))
                    mat->setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,val.toInt());
                if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,val.toDouble());
                if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,val.toDouble());
                if (annJson.getValue(sub,"slip",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslip,val.toDouble());
                if (annJson.getValue(sub,"slide",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslide,val.toDouble());
            }
            if (annJson.getValue(vortex,"angularPrimaryAxis",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,allErrors))
                    mat->setEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,val.toInt());
                if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primangularaxisfriction,val.toDouble());
                if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,val.toDouble());
                if (annJson.getValue(sub,"slip",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primangularaxisslip,val.toDouble());
                if (annJson.getValue(sub,"slide",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_primangularaxisslide,val.toDouble());
            }
            if (annJson.getValue(vortex,"angularSecondaryAxis",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"sameValuesAsPrimaryAxis",QJsonValue::Bool,val,allErrors))
                    mat->setEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,val.toInt());
                if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,allErrors))
                    mat->setEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,val.toInt());
                if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_secangularaxisfriction,val.toDouble());
                if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,val.toDouble());
                if (annJson.getValue(sub,"slip",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_secangularaxisslip,val.toDouble());
                if (annJson.getValue(sub,"slide",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_secangularaxisslide,val.toDouble());
            }
            if (annJson.getValue(vortex,"angularNormalAxis",QJsonValue::Object,val,allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub,"sameValuesAsPrimaryAxis",QJsonValue::Bool,val,allErrors))
                    mat->setEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,val.toInt());
                if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,allErrors))
                    mat->setEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,val.toInt());
                if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_normalangularaxisfriction,val.toDouble());
                if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,val.toDouble());
                if (annJson.getValue(sub,"slip",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslip,val.toDouble());
                if (annJson.getValue(sub,"slide",QJsonValue::Double,val,allErrors))
                    mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslide,val.toDouble());
            }
        }
    }
}

void CEngineProperties::_writeGlobal(int engine,CAnnJson& annJson)
{
    if (engine==sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(annJson.getMainObject()[0],"bullet",jbullet);
    }

    if (engine==sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(annJson.getMainObject()[0],"ode",jode);
    }

    if (engine==sim_physics_newton)
    {
        QJsonObject jnewton;
        annJson.addJson(annJson.getMainObject()[0],"newton",jnewton);
    }

    if (engine==sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        annJson.addJson(annJson.getMainObject()[0],"mujoco",jmujoco);
    }

    if (engine==sim_physics_vortex)
    {
        QJsonObject jvortex;
        annJson.addJson(annJson.getMainObject()[0],"vortex",jvortex);
    }
}

void CEngineProperties::_readGlobal(int engine,CAnnJson& annJson,std::string* allErrors)
{
    QJsonValue val;

    if (engine==sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"bullet",QJsonValue::Object,val,allErrors))
        {
            QJsonObject bullet(val.toObject());
        }
    }

    if (engine==sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"ode",QJsonValue::Object,val,allErrors))
        {
            QJsonObject ode(val.toObject());
        }
    }

    if (engine==sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"newton",QJsonValue::Object,val,allErrors))
        {
            QJsonObject newton(val.toObject());
        }
    }

    if (engine==sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0],"mujoco",QJsonValue::Object,val,allErrors))
        {
            QJsonObject mujoco(val.toObject());
        }
    }

    if (engine==sim_physics_vortex)
    {
        int vval;
        if (annJson.getValue(annJson.getMainObject()[0],"vortex",QJsonValue::Object,val,allErrors))
        {
            QJsonObject vortex(val.toObject());
        }
    }
}

