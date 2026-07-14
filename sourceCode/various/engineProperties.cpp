#include <engineProperties.h>
#include <app.h>
#include <annJson.h>
#include <tt.h>
#include <utils.h>
#ifdef SIM_WITH_GUI
#include <vMessageBox.h>
#include <guiApp.h>
#endif

CEngineProperties::CEngineProperties()
{
}

CEngineProperties::~CEngineProperties()
{
}

#ifdef SIM_WITH_GUI
void CEngineProperties::editObjectProperties(int objectHandle) const
{
    std::string title;
    std::string initText(getObjectProperties(objectHandle, &title, false));

    // printf("%s:\n%s\n",title.c_str(),initText.c_str());
    std::string modifiedText(initText);
    std::string err;
    while (true)
    {
        err.clear();
        std::string xml("<editor title=\"");
        xml += title;
        xml += "\" lang=\"json\" line-numbers=\"true\" ";
        if (App::userSettings->scriptEditorFont.compare("") != 0) // defaults are decided in the code editor plugin
        {
            xml += "font=\"";
            xml += App::userSettings->scriptEditorFont + "\" ";
        }
        int fontSize = 12;
#ifdef MAC_SIM
        fontSize = 16; // bigger fonts here
#endif
        if (App::userSettings->scriptEditorFontSize != -1)
            fontSize = App::userSettings->scriptEditorFontSize;
        xml += "font-size=\"";
        xml += std::to_string(fontSize) + "\" ";
        xml += "/>";
        modifiedText = GuiApp::mainWindow->codeEditorContainer->openModalTextEditor(modifiedText.c_str(), xml.c_str(), nullptr, false);
        // printf("Modified text:\n%s\n",modifiedText.c_str());

        int parseErrorLine;
        if (setObjectProperties(objectHandle, modifiedText.c_str(), &err, &parseErrorLine))
            break;

        std::string msg("Invalid JSON data:\n\n");
        msg += err + "\nat line ";
        msg += std::to_string(parseErrorLine) + "\n\nDiscard changes?";
        if (VMESSAGEBOX_REPLY_CANCEL != GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, title.c_str(), msg.c_str(), VMESSAGEBOX_OK_CANCEL, VMESSAGEBOX_REPLY_CANCEL))
            return;
    }

    if (err.size() > 0)
    {
        err = std::string("The JSON parser found following error(s):\n") + err;
        App::logMsg(sim_verbosity_scripterrors, err.c_str());
        GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, title.c_str(), err.c_str(), VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
    }
}
#endif

std::string CEngineProperties::getObjectProperties(int objectHandle, std::string* title /*= nullptr*/, bool stripComments /*= true*/) const
{
    CSceneObject* object = App::scene->sceneObjects->getObjectFromHandle(objectHandle);

    QJsonObject jmain;
    CAnnJson annJson(&jmain);
    if (title != nullptr)
        title[0] = "Dynamic engine global properties";
    int engine = App::scene->dynamicsContainer->getDynamicEngineType(nullptr);

    if (object != nullptr)
    {
        if (object->getObjectType() == sim_sceneobject_shape)
        {
            _writeShape(engine, objectHandle, annJson);
            for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
            {
                if (i != engine)
                    _writeShape((int)i, objectHandle, annJson);
            }
            if (title != nullptr)
                title[0] = "Dynamic engine properties for shape ";
        }
        if (object->getObjectType() == sim_sceneobject_joint)
        {
            _writeJoint(engine, objectHandle, annJson);
            for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
            {
                if (i != engine)
                    _writeJoint((int)i, objectHandle, annJson);
            }
            if (title != nullptr)
                title[0] = "Dynamic engine properties for joint ";
        }
        if (object->getObjectType() == sim_sceneobject_dummy)
        {
            _writeDummy(engine, objectHandle, annJson);
            for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
            {
                if (i != engine)
                    _writeDummy((int)i, objectHandle, annJson);
            }
            if (title != nullptr)
                title[0] = "Dynamic engine properties for dummy ";
        }
        if (title != nullptr)
            title[0] += object->getObjectAlias_printPath();
    }
    else
    {
        _writeGlobal(engine, annJson);
        for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
        {
            if (i != engine)
                _writeGlobal((int)i, annJson);
        }
    }

    std::string str(annJson.getAnnotatedString());
    if (stripComments)
        str = annJson.stripComments(str.c_str());

    return str;
}

bool CEngineProperties::setObjectProperties(int objectHandle, const char* prop, std::string* errorString /*= nullptr*/, int* parseErrorLine /*= nullptr*/) const
{
    bool retVal = false; // means parse error, no property at all set
    CSceneObject* object = App::scene->sceneObjects->getObjectFromHandle(objectHandle);

    QJsonObject jmain;
    CAnnJson annJson(&jmain);

    std::string modifiedText_noComments(annJson.stripComments(prop));

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(modifiedText_noComments.c_str(), &parseError);

    if ((!doc.isNull()) && doc.isObject())
    {
        retVal = true;
        jmain = doc.object();
        if (object != nullptr)
        {
            if (object->getObjectType() == sim_sceneobject_shape)
            {
                for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
                    _readShape((int)i, objectHandle, annJson, errorString);
            }
            if (object->getObjectType() == sim_sceneobject_joint)
            {
                for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
                    _readJoint((int)i, objectHandle, annJson, errorString);
            }
            if (object->getObjectType() == sim_sceneobject_dummy)
            {
                for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
                    _readDummy((int)i, objectHandle, annJson, errorString);
            }
        }
        else
        {
            for (size_t i = sim_physics_bullet; i <= sim_physics_mujoco; i++)
                _readGlobal((int)i, annJson, errorString);
        }
    }
    else
    {
        if (errorString != nullptr)
            errorString[0] = parseError.errorString().toStdString();
        if (parseErrorLine != nullptr)
            parseErrorLine[0] = utils::lineCountAtOffset(modifiedText_noComments.c_str(), parseError.offset);
    }

    return retVal;
}

void CEngineProperties::_writeJoint(int engine, int jointHandle, CAnnJson& annJson) const
{
    CJoint* joint = App::scene->sceneObjects->getJointFromHandle(jointHandle);

    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(jbullet, "normalCfm", joint->getFloatPropertyValue(prop(PropJoint::bulletNormalCfm).name));
        annJson.addJson(jbullet, "stopErp", joint->getFloatPropertyValue(prop(PropJoint::bulletStopErp).name));
        annJson.addJson(jbullet, "stopCfm", joint->getFloatPropertyValue(prop(PropJoint::bulletStopCfm).name));
        std::vector<double> v;
        joint->getFloatArrayProperty(prop(PropJoint::bulletPosPid).name, v);
        annJson.addJson(jbullet, "posPid", v.data(), 3);
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(jode, "normalCfm", joint->getFloatPropertyValue(prop(PropJoint::odeNormalCfm).name));
        annJson.addJson(jode, "stopErp", joint->getFloatPropertyValue(prop(PropJoint::odeStopErp).name));
        annJson.addJson(jode, "stopCfm", joint->getFloatPropertyValue(prop(PropJoint::odeStopCfm).name));
        annJson.addJson(jode, "bounce", joint->getFloatPropertyValue(prop(PropJoint::odeBounce).name));
        annJson.addJson(jode, "fudge", joint->getFloatPropertyValue(prop(PropJoint::odeFudgeFactor).name));
        std::vector<double> v;
        joint->getFloatArrayProperty(prop(PropJoint::odePosPid).name, v);
        annJson.addJson(jode, "posPid", v.data(), 3);
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        std::vector<double> v;
        joint->getFloatArrayProperty(prop(PropJoint::newtonPosPid).name, v);
        annJson.addJson(jnewton, "posPid", v.data(), 3);
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        annJson.addJson(jmujoco, "armature", joint->getFloatPropertyValue(prop(PropJoint::mujocoArmature).name));
        annJson.addJson(jmujoco, "margin", joint->getFloatPropertyValue(prop(PropJoint::mujocoMargin).name));
        QJsonObject jmujocoLimits;
        std::vector<double> v;
        joint->getFloatArrayProperty(prop(PropJoint::mujocoLimitsSolRef).name, v);
        annJson.addJson(jmujocoLimits, "solref", v.data(), 2);
        joint->getFloatArrayProperty(prop(PropJoint::mujocoLimitsSolImp).name, v);
        annJson.addJson(jmujocoLimits, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "limits", jmujocoLimits);
        QJsonObject jmujocoFriction;
        annJson.addJson(jmujocoFriction, "loss", joint->getFloatPropertyValue(prop(PropJoint::mujocoFrictionLoss).name));
        joint->getFloatArrayProperty(prop(PropJoint::mujocoFrictionSolRef).name, v);
        annJson.addJson(jmujocoFriction, "solref", v.data(), 2);
        joint->getFloatArrayProperty(prop(PropJoint::mujocoFrictionSolImp).name, v);
        annJson.addJson(jmujocoFriction, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "friction", jmujocoFriction);
        QJsonObject jmujocoSpring;
        annJson.addJson(jmujocoSpring, "stiffness", joint->getFloatPropertyValue(prop(PropJoint::mujocoSpringStiffness).name));
        annJson.addJson(jmujocoSpring, "damping", joint->getFloatPropertyValue(prop(PropJoint::mujocoSpringDamping).name));
        annJson.addJson(jmujocoSpring, "ref", joint->getFloatPropertyValue(prop(PropJoint::mujocoSpringRef).name));
        joint->getFloatArrayProperty(prop(PropJoint::mujocoSpringDamper).name, v);
        annJson.addJson(jmujocoSpring, "springDamper", v.data(), 2);
        annJson.addJson(jmujoco, "spring", jmujocoSpring);
        QJsonObject jmujocoDependency;
        joint->getFloatArrayProperty(prop(PropJoint::mujocoDependencyPolyCoef).name, v);
        annJson.addJson(jmujocoDependency, "polyCoeff", v.data(), 5);
        annJson.addJson(jmujoco, "dependency", jmujocoDependency);
        joint->getFloatArrayProperty(prop(PropJoint::mujocoPosPid).name, v);
        annJson.addJson(jmujoco, "posPid", v.data(), 3);
        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        QJsonObject jvortexAxisFriction;
        annJson.addJson(jvortexAxisFriction, "enabled",
                        joint->getBoolPropertyValue(prop(PropJoint::vortexAxisFrictionEnabled).name));
        annJson.addJson(jvortexAxisFriction, "proportional",
                        joint->getBoolPropertyValue(prop(PropJoint::vortexAxisFrictionProportional).name));
        annJson.addJson(jvortexAxisFriction, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexAxisFrictionCoeff).name));
        annJson.addJson(jvortexAxisFriction, "maxForce",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexAxisFrictionMaxForce).name));
        annJson.addJson(jvortexAxisFriction, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexAxisFrictionLoss).name));
        annJson.addJson(jvortex, "axisFriction", jvortexAxisFriction);
        QJsonObject jvortexAxisLimits;
        annJson.addJson(jvortexAxisLimits, "lowerRestitution",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexLowerLimitRestitution).name));
        annJson.addJson(jvortexAxisLimits, "upperRestitution",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexUpperLimitRestitution).name));
        annJson.addJson(jvortexAxisLimits, "lowerStiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexLowerLimitStiffness).name));
        annJson.addJson(jvortexAxisLimits, "upperStiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexUpperLimitStiffness).name));
        annJson.addJson(jvortexAxisLimits, "lowerDamping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexLowerLimitDamping).name));
        annJson.addJson(jvortexAxisLimits, "upperDamping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexUpperLimitDamping).name));
        annJson.addJson(jvortexAxisLimits, "lowerMaxForce",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexLowerLimitMaxForce).name));
        annJson.addJson(jvortexAxisLimits, "upperMaxForce",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexUpperLimitMaxForce).name));
        annJson.addJson(jvortex, "axisLimits", jvortexAxisLimits);

        int vval;
        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
        bool P0_relaxation_enabled = ((vval & 1) != 0);
        bool P1_relaxation_enabled = ((vval & 2) != 0);
        bool P2_relaxation_enabled = ((vval & 4) != 0);
        bool A0_relaxation_enabled = ((vval & 8) != 0);
        bool A1_relaxation_enabled = ((vval & 16) != 0);
        bool A2_relaxation_enabled = ((vval & 32) != 0);
        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
        bool P0_friction_enabled = ((vval & 1) != 0);
        bool P1_friction_enabled = ((vval & 2) != 0);
        bool P2_friction_enabled = ((vval & 4) != 0);
        bool A0_friction_enabled = ((vval & 8) != 0);
        bool A1_friction_enabled = ((vval & 16) != 0);
        bool A2_friction_enabled = ((vval & 32) != 0);
        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
        bool P0_friction_proportional = ((vval & 1) != 0);
        bool P1_friction_proportional = ((vval & 2) != 0);
        bool P2_friction_proportional = ((vval & 4) != 0);
        bool A0_friction_proportional = ((vval & 8) != 0);
        bool A1_friction_proportional = ((vval & 16) != 0);
        bool A2_friction_proportional = ((vval & 32) != 0);

        QJsonObject jvortexXaxisPosition;
        QJsonObject jvortexXaxisPositionRelax;
        annJson.addJson(jvortexXaxisPositionRelax, "enabled", P0_relaxation_enabled);
        annJson.addJson(jvortexXaxisPositionRelax, "stiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisPosRelaxationStiffness).name));
        annJson.addJson(jvortexXaxisPositionRelax, "damping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisPosRelaxationDamping).name));
        annJson.addJson(jvortexXaxisPositionRelax, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisPosRelaxationLoss).name));
        annJson.addJson(jvortexXaxisPosition, "relaxation", jvortexXaxisPositionRelax);
        QJsonObject jvortexXaxisPositionFric;
        annJson.addJson(jvortexXaxisPositionFric, "enabled", P0_friction_enabled);
        annJson.addJson(jvortexXaxisPositionFric, "proportional", P0_friction_proportional);
        annJson.addJson(jvortexXaxisPositionFric, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisPosFrictionCoeff).name));
        annJson.addJson(jvortexXaxisPositionFric, "maxForce",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisPosFrictionMaxForce).name));
        annJson.addJson(jvortexXaxisPositionFric, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisPosFrictionLoss).name));
        annJson.addJson(jvortexXaxisPosition, "friction", jvortexXaxisPositionFric);
        annJson.addJson(jvortex, "xAxisPos", jvortexXaxisPosition);

        QJsonObject jvortexYaxisPosition;
        QJsonObject jvortexYaxisPositionRelax;
        annJson.addJson(jvortexYaxisPositionRelax, "enabled", P1_relaxation_enabled);
        annJson.addJson(jvortexYaxisPositionRelax, "stiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisPosRelaxationStiffness).name));
        annJson.addJson(jvortexYaxisPositionRelax, "damping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisPosRelaxationDamping).name));
        annJson.addJson(jvortexYaxisPositionRelax, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisPosRelaxationLoss).name));
        annJson.addJson(jvortexYaxisPosition, "relaxation", jvortexYaxisPositionRelax);
        QJsonObject jvortexYaxisPositionFric;
        annJson.addJson(jvortexYaxisPositionFric, "enabled", P1_friction_enabled);
        annJson.addJson(jvortexYaxisPositionFric, "proportional", P1_friction_proportional);
        annJson.addJson(jvortexYaxisPositionFric, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisPosFrictionCoeff).name));
        annJson.addJson(jvortexYaxisPositionFric, "maxForce",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisPosFrictionMaxForce).name));
        annJson.addJson(jvortexYaxisPositionFric, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisPosFrictionLoss).name));
        annJson.addJson(jvortexYaxisPosition, "friction", jvortexYaxisPositionFric);
        annJson.addJson(jvortex, "yAxisPos", jvortexYaxisPosition);

        QJsonObject jvortexZaxisPosition;
        QJsonObject jvortexZaxisPositionRelax;
        annJson.addJson(jvortexZaxisPositionRelax, "enabled", P2_relaxation_enabled);
        annJson.addJson(jvortexZaxisPositionRelax, "stiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisPosRelaxationStiffness).name));
        annJson.addJson(jvortexZaxisPositionRelax, "damping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisPosRelaxationDamping).name));
        annJson.addJson(jvortexZaxisPositionRelax, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisPosRelaxationLoss).name));
        annJson.addJson(jvortexZaxisPosition, "relaxation", jvortexZaxisPositionRelax);
        QJsonObject jvortexZaxisPositionFric;
        annJson.addJson(jvortexZaxisPositionFric, "enabled", P2_friction_enabled);
        annJson.addJson(jvortexZaxisPositionFric, "proportional", P2_friction_proportional);
        annJson.addJson(jvortexZaxisPositionFric, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisPosFrictionCoeff).name));
        annJson.addJson(jvortexZaxisPositionFric, "maxForce",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisPosFrictionMaxForce).name));
        annJson.addJson(jvortexZaxisPositionFric, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisPosFrictionLoss).name));
        annJson.addJson(jvortexZaxisPosition, "friction", jvortexZaxisPositionFric);
        annJson.addJson(jvortex, "zAxisPos", jvortexZaxisPosition);

        QJsonObject jvortexXaxisOrientation;
        QJsonObject jvortexXaxisOrientationRelax;
        annJson.addJson(jvortexXaxisOrientationRelax, "enabled", A0_relaxation_enabled);
        annJson.addJson(jvortexXaxisOrientationRelax, "stiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisOrientRelaxStiffness).name));
        annJson.addJson(jvortexXaxisOrientationRelax, "damping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisOrientRelaxDamping).name));
        annJson.addJson(jvortexXaxisOrientationRelax, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisOrientRelaxLoss).name));
        annJson.addJson(jvortexXaxisOrientation, "relaxation", jvortexXaxisOrientationRelax);
        QJsonObject jvortexXaxisOrientationFric;
        annJson.addJson(jvortexXaxisOrientationFric, "enabled", A0_friction_enabled);
        annJson.addJson(jvortexXaxisOrientationFric, "proportional", A0_friction_proportional);
        annJson.addJson(jvortexXaxisOrientationFric, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisOrientFrictionCoeff).name));
        annJson.addJson(jvortexXaxisOrientationFric, "maxTorque",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisOrientFrictionMaxTorque).name));
        annJson.addJson(jvortexXaxisOrientationFric, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexXAxisOrientFrictionLoss).name));
        annJson.addJson(jvortexXaxisOrientation, "friction", jvortexXaxisOrientationFric);
        annJson.addJson(jvortex, "xAxisOrient", jvortexXaxisOrientation);

        QJsonObject jvortexYaxisOrientation;
        QJsonObject jvortexYaxisOrientationRelax;
        annJson.addJson(jvortexYaxisOrientationRelax, "enabled", A1_relaxation_enabled);
        annJson.addJson(jvortexYaxisOrientationRelax, "stiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisOrientRelaxStiffness).name));
        annJson.addJson(jvortexYaxisOrientationRelax, "damping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisOrientRelaxDamping).name));
        annJson.addJson(jvortexYaxisOrientationRelax, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisOrientRelaxLoss).name));
        annJson.addJson(jvortexYaxisOrientation, "relaxation", jvortexYaxisOrientationRelax);
        QJsonObject jvortexYaxisOrientationFric;
        annJson.addJson(jvortexYaxisOrientationFric, "enabled", A1_friction_enabled);
        annJson.addJson(jvortexYaxisOrientationFric, "proportional", A1_friction_proportional);
        annJson.addJson(jvortexYaxisOrientationFric, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisOrientFrictionCoeff).name));
        annJson.addJson(jvortexYaxisOrientationFric, "maxTorque",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisOrientFrictionMaxTorque).name));
        annJson.addJson(jvortexYaxisOrientationFric, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexYAxisOrientFrictionLoss).name));
        annJson.addJson(jvortexYaxisOrientation, "friction", jvortexYaxisOrientationFric);
        annJson.addJson(jvortex, "yAxisOrient", jvortexYaxisOrientation);

        QJsonObject jvortexZaxisOrientation;
        QJsonObject jvortexZaxisOrientationRelax;
        annJson.addJson(jvortexZaxisOrientationRelax, "enabled", A2_relaxation_enabled);
        annJson.addJson(jvortexZaxisOrientationRelax, "stiffness",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisOrientRelaxStiffness).name));
        annJson.addJson(jvortexZaxisOrientationRelax, "damping",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisOrientRelaxDamping).name));
        annJson.addJson(jvortexZaxisOrientationRelax, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisOrientRelaxLoss).name));
        annJson.addJson(jvortexZaxisOrientation, "relaxation", jvortexZaxisOrientationRelax);
        QJsonObject jvortexZaxisOrientationFric;
        annJson.addJson(jvortexZaxisOrientationFric, "enabled", A2_friction_enabled);
        annJson.addJson(jvortexZaxisOrientationFric, "proportional", A2_friction_proportional);
        annJson.addJson(jvortexZaxisOrientationFric, "value",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisOrientFrictionCoeff).name));
        annJson.addJson(jvortexZaxisOrientationFric, "maxTorque",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisOrientFrictionMaxTorque).name));
        annJson.addJson(jvortexZaxisOrientationFric, "loss",
                        joint->getFloatPropertyValue(prop(PropJoint::vortexZAxisOrientFrictionLoss).name));
        annJson.addJson(jvortexZaxisOrientation, "friction", jvortexZaxisOrientationFric);
        annJson.addJson(jvortex, "zAxisOrient", jvortexZaxisOrientation);

        std::vector<double> v;
        joint->getFloatArrayProperty(prop(PropJoint::vortexPosPid).name, v);
        annJson.addJson(jvortex, "posPid", v.data(), 3);

        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readJoint(int engine, int jointHandle, CAnnJson& annJson, std::string* allErrors) const
{
    CJoint* joint = App::scene->sceneObjects->getJointFromHandle(jointHandle);
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
            double w[3];
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet, "normalCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::bulletNormalCfm).name, val.toDouble());
            if (annJson.getValue(bullet, "stopErp", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::bulletStopErp).name, val.toDouble());
            if (annJson.getValue(bullet, "stopCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::bulletStopCfm).name, val.toDouble());
            if (annJson.getValue(bullet, "posPid", w, 3, allErrors))
            {
                std::vector<double> ww(w, w + 3);
                joint->setFloatArrayProperty(prop(PropJoint::bulletPosPid).name, ww);
            }
        }
    }

    if (engine == sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "ode", QJsonValue::Object, val, allErrors))
        {
            double w[3];
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode, "normalCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::odeNormalCfm).name, val.toDouble());
            if (annJson.getValue(ode, "stopErp", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::odeStopErp).name, val.toDouble());
            if (annJson.getValue(ode, "stopCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::odeStopCfm).name, val.toDouble());
            if (annJson.getValue(ode, "bounce", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::odeBounce).name, val.toDouble());
            if (annJson.getValue(ode, "fudge", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::odeFudgeFactor).name, val.toDouble());
            if (annJson.getValue(ode, "posPid", w, 3, allErrors))
            {
                std::vector<double> ww(w, w + 3);
                joint->setFloatArrayProperty(prop(PropJoint::odePosPid).name, ww);
            }
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
            QJsonObject newton(val.toObject());
            double w[3];
            if (annJson.getValue(newton, "posPid", w, 3, allErrors))
            {
                std::vector<double> ww(w, w + 3);
                joint->setFloatArrayProperty(prop(PropJoint::newtonPosPid).name, ww);
            }
        }
    }

    if (engine == sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "mujoco", QJsonValue::Object, val, allErrors))
        {
            QJsonObject mujoco(val.toObject());
            if (annJson.getValue(mujoco, "armature", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::mujocoArmature).name, val.toDouble());
            if (annJson.getValue(mujoco, "margin", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(prop(PropJoint::mujocoMargin).name, val.toDouble());
            double w[5];
            if (annJson.getValue(mujoco, "limits", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    joint->setFloatArrayProperty(prop(PropJoint::mujocoLimitsSolRef).name, ww);
                }
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    joint->setFloatArrayProperty(prop(PropJoint::mujocoLimitsSolImp).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "friction", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "loss", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::mujocoFrictionLoss).name, val.toDouble());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    joint->setFloatArrayProperty(prop(PropJoint::mujocoFrictionSolRef).name, ww);
                }
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    joint->setFloatArrayProperty(prop(PropJoint::mujocoFrictionSolImp).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "spring", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "stiffness", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::mujocoSpringStiffness).name, val.toDouble());
                if (annJson.getValue(sub, "damping", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::mujocoSpringDamping).name, val.toDouble());
                if (annJson.getValue(sub, "ref", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::mujocoSpringRef).name, val.toDouble());
                if (annJson.getValue(sub, "springDamper", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    joint->setFloatArrayProperty(prop(PropJoint::mujocoSpringDamper).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "dependency", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "polyCoeff", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    joint->setFloatArrayProperty(prop(PropJoint::mujocoDependencyPolyCoef).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "posPid", w, 3, allErrors))
            {
                std::vector<double> ww(w, w + 3);
                joint->setFloatArrayProperty(prop(PropJoint::mujocoPosPid).name, ww);
            }
        }
    }

    if (engine == sim_physics_vortex)
    {
        int vval;
        if (annJson.getValue(annJson.getMainObject()[0], "vortex", QJsonValue::Object, val, allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex, "axisFriction", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    joint->setBoolProperty(prop(PropJoint::vortexAxisFrictionEnabled).name, val.toBool());
                if (annJson.getValue(sub, "proportional", QJsonValue::Bool, val, allErrors))
                    joint->setBoolProperty(prop(PropJoint::vortexAxisFrictionProportional).name, val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexAxisFrictionCoeff).name, val.toDouble());
                if (annJson.getValue(sub, "maxForce", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexAxisFrictionMaxForce).name, val.toDouble());
                if (annJson.getValue(sub, "loss", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexAxisFrictionLoss).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "axisLimits", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "lowerRestitution", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexLowerLimitRestitution).name, val.toDouble());
                if (annJson.getValue(sub, "upperRestitution", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexUpperLimitRestitution).name, val.toDouble());
                if (annJson.getValue(sub, "lowerStiffness", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexLowerLimitStiffness).name, val.toDouble());
                if (annJson.getValue(sub, "upperStiffness", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexUpperLimitStiffness).name, val.toDouble());
                if (annJson.getValue(sub, "lowerDamping", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexLowerLimitDamping).name, val.toDouble());
                if (annJson.getValue(sub, "upperDamping", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexUpperLimitDamping).name, val.toDouble());
                if (annJson.getValue(sub, "lowerMaxForce", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexLowerLimitMaxForce).name, val.toDouble());
                if (annJson.getValue(sub, "upperMaxForce", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(prop(PropJoint::vortexUpperLimitMaxForce).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "xAxisPos", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
                        vval = (vval | 1) - 1;
                        if (val.toBool())
                            vval |= 1;
                        joint->setIntProperty(prop(PropJoint::vortexRelaxationEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisPosRelaxationStiffness).name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisPosRelaxationDamping).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisPosRelaxationLoss).name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
                        vval = (vval | 1) - 1;
                        if (val.toBool())
                            vval |= 1;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
                        vval = (vval | 1) - 1;
                        if (val.toBool())
                            vval |= 1;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionProportionalBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisPosFrictionCoeff).name, val.toDouble());
                    if (annJson.getValue(sub2, "maxForce", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisPosFrictionMaxForce).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisPosFrictionLoss).name, val.toDouble());
                }
            }
            if (annJson.getValue(vortex, "yAxisPos", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
                        vval = (vval | 2) - 2;
                        if (val.toBool())
                            vval |= 2;
                        joint->setIntProperty(prop(PropJoint::vortexRelaxationEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisPosRelaxationStiffness).name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisPosRelaxationDamping).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisPosRelaxationLoss).name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
                        vval = (vval | 2) - 2;
                        if (val.toBool())
                            vval |= 2;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
                        vval = (vval | 2) - 2;
                        if (val.toBool())
                            vval |= 2;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionProportionalBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisPosFrictionCoeff).name, val.toDouble());
                    if (annJson.getValue(sub2, "maxForce", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisPosFrictionMaxForce).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisPosFrictionLoss).name, val.toDouble());
                }
            }
            if (annJson.getValue(vortex, "zAxisPos", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
                        vval = (vval | 4) - 4;
                        if (val.toBool())
                            vval |= 4;
                        joint->setIntProperty(prop(PropJoint::vortexRelaxationEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisPosRelaxationStiffness).name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisPosRelaxationDamping).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisPosRelaxationLoss).name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
                        vval = (vval | 4) - 4;
                        if (val.toBool())
                            vval |= 4;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
                        vval = (vval | 4) - 4;
                        if (val.toBool())
                            vval |= 4;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionProportionalBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisPosFrictionCoeff).name, val.toDouble());
                    if (annJson.getValue(sub2, "maxForce", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisPosFrictionMaxForce).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisPosFrictionLoss).name, val.toDouble());
                }
            }
            if (annJson.getValue(vortex, "xAxisOrient", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
                        vval = (vval | 8) - 8;
                        if (val.toBool())
                            vval |= 8;
                        joint->setIntProperty(prop(PropJoint::vortexRelaxationEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisOrientRelaxStiffness).name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisOrientRelaxDamping).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisOrientRelaxLoss).name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
                        vval = (vval | 8) - 8;
                        if (val.toBool())
                            vval |= 8;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
                        vval = (vval | 8) - 8;
                        if (val.toBool())
                            vval |= 8;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionProportionalBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisOrientFrictionCoeff).name, val.toDouble());
                    if (annJson.getValue(sub2, "maxTorque", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisOrientFrictionMaxTorque).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexXAxisOrientFrictionLoss).name, val.toDouble());
                }
            }
            if (annJson.getValue(vortex, "yAxisOrient", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
                        vval = (vval | 16) - 16;
                        if (val.toBool())
                            vval |= 16;
                        joint->setIntProperty(prop(PropJoint::vortexRelaxationEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisOrientRelaxStiffness).name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisOrientRelaxDamping).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisOrientRelaxLoss).name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
                        vval = (vval | 16) - 16;
                        if (val.toBool())
                            vval |= 16;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
                        vval = (vval | 16) - 16;
                        if (val.toBool())
                            vval |= 16;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionProportionalBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisOrientFrictionCoeff).name, val.toDouble());
                    if (annJson.getValue(sub2, "maxTorque", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisOrientFrictionMaxTorque).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexYAxisOrientFrictionLoss).name, val.toDouble());
                }
            }
            if (annJson.getValue(vortex, "zAxisOrient", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexRelaxationEnabledBits).name);
                        vval = (vval | 32) - 32;
                        if (val.toBool())
                            vval |= 32;
                        joint->setIntProperty(prop(PropJoint::vortexRelaxationEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisOrientRelaxStiffness).name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisOrientRelaxDamping).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisOrientRelaxLoss).name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionEnabledBits).name);
                        vval = (vval | 32) - 32;
                        if (val.toBool())
                            vval |= 32;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionEnabledBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(prop(PropJoint::vortexFrictionProportionalBits).name);
                        vval = (vval | 32) - 32;
                        if (val.toBool())
                            vval |= 32;
                        joint->setIntProperty(prop(PropJoint::vortexFrictionProportionalBits).name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisOrientFrictionCoeff).name, val.toDouble());
                    if (annJson.getValue(sub2, "maxTorque", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisOrientFrictionMaxTorque).name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(prop(PropJoint::vortexZAxisOrientFrictionLoss).name, val.toDouble());
                }
            }
            double w[3];
            if (annJson.getValue(vortex, "posPid", w, 3, allErrors))
            {
                std::vector<double> ww(w, w + 3);
                joint->setFloatArrayProperty(prop(PropJoint::vortexPosPid).name, ww);
            }
        }
    }
}

void CEngineProperties::_writeShape(int engine, int shapeHandle, CAnnJson& annJson) const
{
    CShape* shape = App::scene->sceneObjects->getShapeFromHandle(shapeHandle);
    CDynMaterialObject* mat = shape->getDynMaterial();
    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(jbullet, "stickyContact", mat->getBoolPropertyValue(prop(PropShape::bulletSticky).name),
                        "only Bullet V2.78");
        annJson.addJson(jbullet, "frictionOld", mat->getFloatPropertyValue(prop(PropShape::bulletFriction0).name),
                        "only Bullet V2.78");
        annJson.addJson(jbullet, "friction", mat->getFloatPropertyValue(prop(PropShape::bulletFriction).name),
                        "only Bullet V2.83 and later");
        annJson.addJson(jbullet, "restitution", mat->getFloatPropertyValue(prop(PropShape::bulletRestitution).name));
        annJson.addJson(jbullet, "linearDamping", mat->getFloatPropertyValue(prop(PropShape::bulletLinearDamping).name));
        annJson.addJson(jbullet, "angularDamping", mat->getFloatPropertyValue(prop(PropShape::bulletAngularDamping).name));
        QJsonObject jbulletCollMargin;
        annJson.addJson(jbulletCollMargin, "enabled",
                        mat->getBoolPropertyValue(prop(PropShape::bulletNonDefaultCollisionMargin).name));
        annJson.addJson(jbulletCollMargin, "value",
                        mat->getFloatPropertyValue(prop(PropShape::bulletNonDefaultCollisionMarginFactor).name));
        annJson.addJson(jbullet, "customCollisionMargin", jbulletCollMargin);
        QJsonObject jbulletCollMarginConvex;
        annJson.addJson(jbulletCollMarginConvex, "enabled",
                        mat->getBoolPropertyValue(prop(PropShape::bulletNonDefaultCollisionMarginConvex).name));
        annJson.addJson(jbulletCollMarginConvex, "value",
                        mat->getFloatPropertyValue(prop(PropShape::bulletNonDefaultCollisionMarginFactorConvex).name));
        annJson.addJson(jbullet, "customCollisionMarginConvex", jbulletCollMarginConvex);
        annJson.addJson(jbullet, "autoShrinkConvexMeshes",
                        mat->getBoolPropertyValue(prop(PropShape::bulletAutoShrinkConvex).name));
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(jode, "friction", mat->getFloatPropertyValue(prop(PropShape::odeFriction).name));
        annJson.addJson(jode, "softErp", mat->getFloatPropertyValue(prop(PropShape::odeSoftErp).name));
        annJson.addJson(jode, "softCfm", mat->getFloatPropertyValue(prop(PropShape::odeSoftCfm).name));
        annJson.addJson(jode, "linearDamping", mat->getFloatPropertyValue(prop(PropShape::odeLinearDamping).name));
        annJson.addJson(jode, "angularDamping", mat->getFloatPropertyValue(prop(PropShape::odeAngularDamping).name));
        annJson.addJson(jode, "maxContacts", mat->getIntPropertyValue(prop(PropShape::odeMaxContacts).name));
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        annJson.addJson(jnewton, "staticFriction", mat->getFloatPropertyValue(prop(PropShape::newtonStaticFriction).name));
        annJson.addJson(jnewton, "kineticFriction", mat->getFloatPropertyValue(prop(PropShape::newtonKineticFriction).name));
        annJson.addJson(jnewton, "restitution", mat->getFloatPropertyValue(prop(PropShape::newtonRestitution).name));
        annJson.addJson(jnewton, "linearDrag", mat->getFloatPropertyValue(prop(PropShape::newtonLinearDrag).name));
        annJson.addJson(jnewton, "angularDrag", mat->getFloatPropertyValue(prop(PropShape::newtonAngularDrag).name));
        annJson.addJson(jnewton, "fastMoving", mat->getBoolPropertyValue(prop(PropShape::newtonFastMoving).name));
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        std::vector<double> v;
        mat->getFloatArrayProperty(prop(PropShape::mujocoFriction).name, v);
        annJson.addJson(jmujoco, "friction", v.data(), 3);
        mat->getFloatArrayProperty(prop(PropShape::mujocoSolref).name, v);
        annJson.addJson(jmujoco, "solref", v.data(), 2);
        mat->getFloatArrayProperty(prop(PropShape::mujocoSolimp).name, v);
        annJson.addJson(jmujoco, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "condim", mat->getIntPropertyValue(prop(PropShape::mujocoCondim).name));
        annJson.addJson(jmujoco, "solmix", mat->getFloatPropertyValue(prop(PropShape::mujocoSolmix).name));
        annJson.addJson(jmujoco, "margin", mat->getFloatPropertyValue(prop(PropShape::mujocoMargin).name));
        annJson.addJson(jmujoco, "gap", mat->getFloatPropertyValue(prop(PropShape::mujocoGap).name));
        annJson.addJson(jmujoco, "gravcomp", mat->getFloatPropertyValue(prop(PropShape::mujocoGravcomp).name));
        annJson.addJson(jmujoco, "priority", mat->getIntPropertyValue(prop(PropShape::mujocoPriority).name));
        annJson.addJson(jmujoco, "adhesion", mat->getBoolPropertyValue(prop(PropShape::mujocoAdhesion).name));
        annJson.addJson(jmujoco, "adhesiongain", mat->getFloatPropertyValue(prop(PropShape::mujocoAdhesionGain).name));
        annJson.addJson(jmujoco, "adhesionctrl", mat->getFloatPropertyValue(prop(PropShape::mujocoAdhesionCtrl).name));
        mat->getFloatArrayProperty(prop(PropShape::mujocoAdhesionCtrlrange).name, v);
        annJson.addJson(jmujoco, "adhesionctrlrange", v.data(), 2);
        mat->getFloatArrayProperty(prop(PropShape::mujocoAdhesionForcerange).name, v);
        annJson.addJson(jmujoco, "adhesionforcerange", v.data(), 2);
        annJson.addJson(jmujoco, "adhesionforcelimited", mat->getIntPropertyValue(prop(PropShape::mujocoAdhesionForcelimited).name));
        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        annJson.addJson(jvortex, "restitution", mat->getFloatPropertyValue(prop(PropShape::vortexRestitution).name));
        annJson.addJson(jvortex, "restitutionThreshold",
                        mat->getFloatPropertyValue(prop(PropShape::vortexRestitutionThreshold).name));
        annJson.addJson(jvortex, "compliance", mat->getFloatPropertyValue(prop(PropShape::vortexCompliance).name));
        annJson.addJson(jvortex, "damping", mat->getFloatPropertyValue(prop(PropShape::vortexDamping).name));
        annJson.addJson(jvortex, "adhesiveForce", mat->getFloatPropertyValue(prop(PropShape::vortexAdhesiveForce).name));
        annJson.addJson(jvortex, "linearVelDamping",
                        mat->getFloatPropertyValue(prop(PropShape::vortexLinearVelocityDamping).name));
        annJson.addJson(jvortex, "angularVelDamping",
                        mat->getFloatPropertyValue(prop(PropShape::vortexAngularVelocityDamping).name));
        QJsonObject jvortexAutoAngularDamping;
        annJson.addJson(jvortexAutoAngularDamping, "enabled",
                        mat->getBoolPropertyValue(prop(PropShape::vortexAutoAngularDamping).name));
        annJson.addJson(jvortexAutoAngularDamping, "tensionRatio",
                        mat->getFloatPropertyValue(prop(PropShape::vortexAutoAngularDampingTensionRatio).name));
        annJson.addJson(jvortex, "autoAngularDamping", jvortexAutoAngularDamping);
        annJson.addJson(jvortex, "skinThickness", mat->getFloatPropertyValue(prop(PropShape::vortexSkinThickness).name));
        annJson.addJson(jvortex, "autoSlip", mat->getBoolPropertyValue(prop(PropShape::vortexAutoSlip).name));
        annJson.addJson(jvortex, "fastMoving", mat->getBoolPropertyValue(prop(PropShape::vortexFastMoving).name));
        annJson.addJson(jvortex, "primitiveAsConvex",
                        mat->getBoolPropertyValue(prop(PropShape::vortexPrimitiveShapesAsConvex).name));
        annJson.addJson(jvortex, "convexAsRandom",
                        mat->getBoolPropertyValue(prop(PropShape::vortexConvexShapesAsRandom).name));
        annJson.addJson(jvortex, "randomAsTerrain",
                        mat->getBoolPropertyValue(prop(PropShape::vortexRandomShapesAsTerrain).name));
        QJsonObject jvortexautoSleep;
        annJson.addJson(jvortexautoSleep, "linearSpeed",
                        mat->getFloatPropertyValue(prop(PropShape::vortexAutoSleepLinearSpeedThreshold).name));
        annJson.addJson(jvortexautoSleep, "linearAccel",
                        mat->getFloatPropertyValue(prop(PropShape::vortexAutoSleepLinearAccelerationThreshold).name));
        annJson.addJson(jvortexautoSleep, "angularSpeed",
                        mat->getFloatPropertyValue(prop(PropShape::vortexAutoSleepAngularSpeedThreshold).name));
        annJson.addJson(jvortexautoSleep, "angularAccel",
                        mat->getFloatPropertyValue(prop(PropShape::vortexAutoSleepAngularAccelerationThreshold).name));
        annJson.addJson(jvortexautoSleep, "steps",
                        mat->getIntPropertyValue(prop(PropShape::vortexAutoSleepStepLiveThreshold).name));
        annJson.addJson(jvortex, "autoSleepThreshold", jvortexautoSleep);
        std::string fricModelInfo("0=box, 1=scaled box, 2=prop low, 3=prop high, 4=scaled box fast, 5=neutral, 6=none");
        QJsonObject jvortexlinPrimAxis;
        C3Vector v;
        mat->getVector3Property(prop(PropShape::vortexPrimaryAxisVector).name, &v);
        annJson.addJson(jvortexlinPrimAxis, "value", v.data, 3);
        annJson.addJson(jvortexlinPrimAxis, "frictionModel",
                        mat->getIntPropertyValue(prop(PropShape::vortexPrimaryLinearAxisFrictionModel).name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexlinPrimAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryLinearAxisFriction).name));
        annJson.addJson(jvortexlinPrimAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryLinearAxisStaticFrictionScale).name));
        annJson.addJson(jvortexlinPrimAxis, "slip",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryLinearAxisSlip).name));
        annJson.addJson(jvortexlinPrimAxis, "slide",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryLinearAxisSlide).name));
        annJson.addJson(jvortex, "linearPrimaryAxis", jvortexlinPrimAxis);
        QJsonObject jvortexlinSecAxis;
        annJson.addJson(jvortexlinSecAxis, "followPrimaryAxis",
                        mat->getBoolPropertyValue(prop(PropShape::vortexSecondaryLinearAxisSameAsPrimaryLinearAxis).name));
        annJson.addJson(jvortexlinSecAxis, "frictionModel",
                        mat->getIntPropertyValue(prop(PropShape::vortexSecondaryLinearAxisFrictionModel).name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexlinSecAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryLinearAxisFriction).name));
        annJson.addJson(jvortexlinSecAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryLinearAxisStaticFrictionScale).name));
        annJson.addJson(jvortexlinSecAxis, "slip",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryLinearAxisSlip).name));
        annJson.addJson(jvortexlinSecAxis, "slide",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryLinearAxisSlide).name));
        annJson.addJson(jvortex, "linearSecondaryAxis", jvortexlinSecAxis);
        QJsonObject jvortexangPrimAxis;
        annJson.addJson(jvortexangPrimAxis, "frictionModel",
                        mat->getIntPropertyValue(prop(PropShape::vortexPrimaryAngularAxisFrictionModel).name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexangPrimAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryAngularAxisFriction).name));
        annJson.addJson(jvortexangPrimAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryAngularAxisStaticFrictionScale).name));
        annJson.addJson(jvortexangPrimAxis, "slip",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryAngularAxisSlip).name));
        annJson.addJson(jvortexangPrimAxis, "slide",
                        mat->getFloatPropertyValue(prop(PropShape::vortexPrimaryAngularAxisSlide).name));
        annJson.addJson(jvortex, "angularPrimaryAxis", jvortexangPrimAxis);
        QJsonObject jvortexangSecAxis;
        annJson.addJson(jvortexangSecAxis, "followPrimaryAxis",
                        mat->getBoolPropertyValue(prop(PropShape::vortexSecondaryAngularAxisSameAsPrimaryAngularAxis).name));
        annJson.addJson(jvortexangSecAxis, "frictionModel",
                        mat->getIntPropertyValue(prop(PropShape::vortexSecondaryAngularAxisFrictionModel).name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexangSecAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryAngularAxisFriction).name));
        annJson.addJson(jvortexangSecAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryAngularAxisStaticFrictionScale).name));
        annJson.addJson(jvortexangSecAxis, "slip",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryAngularAxisSlip).name));
        annJson.addJson(jvortexangSecAxis, "slide",
                        mat->getFloatPropertyValue(prop(PropShape::vortexSecondaryAngularAxisSlide).name));
        annJson.addJson(jvortex, "angularSecondaryAxis", jvortexangSecAxis);
        QJsonObject jvortexangNormAxis;
        annJson.addJson(jvortexangNormAxis, "followPrimaryAxis",
                        mat->getBoolPropertyValue(prop(PropShape::vortexNormalAngularAxisSameAsPrimaryAngularAxis).name));
        annJson.addJson(jvortexangNormAxis, "frictionModel",
                        mat->getIntPropertyValue(prop(PropShape::vortexNormalAngularAxisFrictionModel).name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexangNormAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(prop(PropShape::vortexNormalAngularAxisFriction).name));
        annJson.addJson(jvortexangNormAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(prop(PropShape::vortexNormalAngularAxisStaticFrictionScale).name));
        annJson.addJson(jvortexangNormAxis, "slip",
                        mat->getFloatPropertyValue(prop(PropShape::vortexNormalAngularAxisSlip).name));
        annJson.addJson(jvortexangNormAxis, "slide",
                        mat->getFloatPropertyValue(prop(PropShape::vortexNormalAngularAxisSlide).name));
        annJson.addJson(jvortex, "angularNormalAxis", jvortexangNormAxis);
        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readShape(int engine, int shapeHandle, CAnnJson& annJson, std::string* allErrors) const
{
    CShape* shape = App::scene->sceneObjects->getShapeFromHandle(shapeHandle);
    CDynMaterialObject* mat = shape->getDynMaterial();
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet, "stickyContact", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::bulletSticky).name, val.toBool());
            if (annJson.getValue(bullet, "frictionOld", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::bulletFriction0).name, val.toDouble());
            if (annJson.getValue(bullet, "friction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::bulletFriction).name, val.toDouble());
            if (annJson.getValue(bullet, "restitution", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::bulletRestitution).name, val.toDouble());
            if (annJson.getValue(bullet, "linearDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::bulletLinearDamping).name, val.toDouble());
            if (annJson.getValue(bullet, "angularDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::bulletAngularDamping).name, val.toDouble());
            if (annJson.getValue(bullet, "customCollisionMargin", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(prop(PropShape::bulletNonDefaultCollisionMargin).name, val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::bulletNonDefaultCollisionMarginFactor).name, val.toDouble());
            }
            if (annJson.getValue(bullet, "customCollisionMarginConvex", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(prop(PropShape::bulletNonDefaultCollisionMarginConvex).name, val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::bulletNonDefaultCollisionMarginFactorConvex).name, val.toDouble());
            }
            if (annJson.getValue(bullet, "autoShrinkConvexMeshes", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::bulletAutoShrinkConvex).name, val.toBool());
        }
    }

    if (engine == sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "ode", QJsonValue::Object, val, allErrors))
        {
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode, "friction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::odeFriction).name, val.toDouble());
            if (annJson.getValue(ode, "softErp", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::odeSoftErp).name, val.toDouble());
            if (annJson.getValue(ode, "softCfm", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::odeSoftCfm).name, val.toDouble());
            if (annJson.getValue(ode, "linearDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::odeLinearDamping).name, val.toDouble());
            if (annJson.getValue(ode, "angularDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::odeAngularDamping).name, val.toDouble());
            if (annJson.getValue(ode, "maxContacts", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(prop(PropShape::odeMaxContacts).name, val.toInt());
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
            QJsonObject newton(val.toObject());
            if (annJson.getValue(newton, "staticFriction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::newtonStaticFriction).name, val.toDouble());
            if (annJson.getValue(newton, "kineticFriction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::newtonKineticFriction).name, val.toDouble());
            if (annJson.getValue(newton, "restitution", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::newtonRestitution).name, val.toDouble());
            if (annJson.getValue(newton, "linearDrag", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::newtonLinearDrag).name, val.toDouble());
            if (annJson.getValue(newton, "angularDrag", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::newtonAngularDrag).name, val.toDouble());
            if (annJson.getValue(newton, "fastMoving", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::newtonFastMoving).name, val.toBool());
        }
    }

    if (engine == sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "mujoco", QJsonValue::Object, val, allErrors))
        {
            QJsonObject mujoco(val.toObject());
            double v[5];
            if (annJson.getValue(mujoco, "friction", v, 3, allErrors))
            {
                std::vector<double> vv(v, v + 3);
                mat->setFloatArrayProperty(prop(PropShape::mujocoFriction).name, vv);
            }
            if (annJson.getValue(mujoco, "solref", v, 2, allErrors))
            {
                std::vector<double> vv(v, v + 2);
                mat->setFloatArrayProperty(prop(PropShape::mujocoSolref).name, vv);
            }
            if (annJson.getValue(mujoco, "solimp", v, 5, allErrors))
            {
                std::vector<double> vv(v, v + 5);
                mat->setFloatArrayProperty(prop(PropShape::mujocoSolimp).name, vv);
            }
            if (annJson.getValue(mujoco, "condim", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(prop(PropShape::mujocoCondim).name, val.toInt());
            if (annJson.getValue(mujoco, "solmix", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::mujocoSolmix).name, val.toDouble());
            if (annJson.getValue(mujoco, "margin", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::mujocoMargin).name, val.toDouble());
            if (annJson.getValue(mujoco, "gap", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::mujocoGap).name, val.toDouble());
            if (annJson.getValue(mujoco, "gravcomp", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::mujocoGravcomp).name, val.toDouble());
            if (annJson.getValue(mujoco, "priority", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(prop(PropShape::mujocoPriority).name, val.toInt());
            if (annJson.getValue(mujoco, "adhesion", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::mujocoAdhesion).name, val.toBool());
            if (annJson.getValue(mujoco, "adhesiongain", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::mujocoAdhesionGain).name, val.toDouble());
            if (annJson.getValue(mujoco, "adhesionctrl", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::mujocoAdhesionCtrl).name, val.toDouble());
            if (annJson.getValue(mujoco, "adhesionctrlrange", v, 2, allErrors))
            {
                std::vector<double> vv(v, v + 2);
                mat->setFloatArrayProperty(prop(PropShape::mujocoAdhesionCtrlrange).name, vv);
            }
            if (annJson.getValue(mujoco, "adhesionforcerange", v, 2, allErrors))
            {
                std::vector<double> vv(v, v + 2);
                mat->setFloatArrayProperty(prop(PropShape::mujocoAdhesionForcerange).name, vv);
            }
            if (annJson.getValue(mujoco, "adhesionforcelimited", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(prop(PropShape::mujocoAdhesionForcelimited).name, val.toInt());
        }
    }

    if (engine == sim_physics_vortex)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "vortex", QJsonValue::Object, val, allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex, "restitution", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexRestitution).name, val.toDouble());
            if (annJson.getValue(vortex, "restitutionThreshold", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexRestitutionThreshold).name, val.toDouble());
            if (annJson.getValue(vortex, "compliance", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexCompliance).name, val.toDouble());
            if (annJson.getValue(vortex, "damping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexDamping).name, val.toDouble());
            if (annJson.getValue(vortex, "adhesiveForce", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexAdhesiveForce).name, val.toDouble());
            if (annJson.getValue(vortex, "linearVelDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexLinearVelocityDamping).name, val.toDouble());
            if (annJson.getValue(vortex, "angularVelDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexAngularVelocityDamping).name, val.toDouble());
            if (annJson.getValue(vortex, "autoAngularDamping", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(prop(PropShape::vortexAutoAngularDamping).name, val.toBool());
                if (annJson.getValue(sub, "tensionRatio", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexAutoAngularDampingTensionRatio).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "skinThickness", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(prop(PropShape::vortexSkinThickness).name, val.toDouble());
            if (annJson.getValue(vortex, "autoSlip", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::vortexAutoSlip).name, val.toBool());
            if (annJson.getValue(vortex, "fastMoving", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::vortexFastMoving).name, val.toBool());
            if (annJson.getValue(vortex, "primitiveAsConvex", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::vortexPrimitiveShapesAsConvex).name, val.toBool());
            if (annJson.getValue(vortex, "convexAsRandom", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::vortexConvexShapesAsRandom).name, val.toBool());
            if (annJson.getValue(vortex, "randomAsTerrain", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(prop(PropShape::vortexRandomShapesAsTerrain).name, val.toBool());
            if (annJson.getValue(vortex, "autoSleepThreshold", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "linearSpeed", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexAutoSleepLinearSpeedThreshold).name, val.toDouble());
                if (annJson.getValue(sub, "linearAccel", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexAutoSleepLinearAccelerationThreshold).name, val.toDouble());
                if (annJson.getValue(sub, "angularSpeed", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexAutoSleepAngularSpeedThreshold).name, val.toDouble());
                if (annJson.getValue(sub, "angularAccel", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexAutoSleepAngularAccelerationThreshold).name, val.toDouble());
                if (annJson.getValue(sub, "steps", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(prop(PropShape::vortexAutoSleepStepLiveThreshold).name, val.toInt());
            }
            if (annJson.getValue(vortex, "linearPrimaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                double v[3];
                if (annJson.getValue(sub, "value", v, 3, allErrors))
                {
                    C3Vector v3(v);
                    mat->setVector3Property(prop(PropShape::vortexPrimaryAxisVector).name, &v3);
                }
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(prop(PropShape::vortexPrimaryLinearAxisFrictionModel).name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryLinearAxisFriction).name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryLinearAxisStaticFrictionScale).name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryLinearAxisSlip).name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryLinearAxisSlide).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "linearSecondaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "followPrimaryAxis", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(prop(PropShape::vortexSecondaryLinearAxisSameAsPrimaryLinearAxis).name, val.toInt());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(prop(PropShape::vortexSecondaryLinearAxisFrictionModel).name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryLinearAxisFriction).name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryLinearAxisStaticFrictionScale).name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryLinearAxisSlip).name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryLinearAxisSlide).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "angularPrimaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(prop(PropShape::vortexPrimaryAngularAxisFrictionModel).name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryAngularAxisFriction).name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryAngularAxisStaticFrictionScale).name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryAngularAxisSlip).name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexPrimaryAngularAxisSlide).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "angularSecondaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "followPrimaryAxis", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(prop(PropShape::vortexSecondaryAngularAxisSameAsPrimaryAngularAxis).name, val.toInt());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(prop(PropShape::vortexSecondaryAngularAxisFrictionModel).name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryAngularAxisFriction).name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryAngularAxisStaticFrictionScale).name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryAngularAxisSlip).name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexSecondaryAngularAxisSlide).name, val.toDouble());
            }
            if (annJson.getValue(vortex, "angularNormalAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "followPrimaryAxis", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(prop(PropShape::vortexNormalAngularAxisSameAsPrimaryAngularAxis).name, val.toInt());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(prop(PropShape::vortexNormalAngularAxisFrictionModel).name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexNormalAngularAxisFriction).name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexNormalAngularAxisStaticFrictionScale).name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexNormalAngularAxisSlip).name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(prop(PropShape::vortexNormalAngularAxisSlide).name, val.toDouble());
            }
        }
    }
}

void CEngineProperties::_writeGlobal(int engine, CAnnJson& annJson) const
{
    std::string comment;
    double fv;
    int iv;
    bool bv;
    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        iv = _getGlobalIntParam(prop(PropScene::bulletSolver).name, comment,
                                "0=sequential impulse, 1=NNCG, 2=dantzig, 3=projected Gauss-Seidel");
        annJson.addJson(jbullet, "solver", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::bulletIterations).name, comment);
        annJson.addJson(jbullet, "iterations", iv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::bulletComputeInertias).name, comment);
        annJson.addJson(jbullet, "computeInertias", bv, comment.c_str());
        QJsonObject jbulletInternalScaling;
        bv = _getGlobalBoolParam(prop(PropScene::bulletInternalScalingFull).name, comment);
        annJson.addJson(jbulletInternalScaling, "full", bv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::bulletInternalScalingScaling).name, comment);
        annJson.addJson(jbulletInternalScaling, "value", fv, comment.c_str());
        annJson.addJson(jbullet, "internalScaling", jbulletInternalScaling);
        fv = _getGlobalFloatParam(prop(PropScene::bulletCollMarginScaling).name, comment);
        annJson.addJson(jbullet, "collisionMarginScaling", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        QJsonObject jodeQuickstep;
        bv = _getGlobalBoolParam(prop(PropScene::odeQuickStepEnabled).name, comment);
        annJson.addJson(jodeQuickstep, "enabled", bv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::odeQuickStepIterations).name, comment);
        annJson.addJson(jodeQuickstep, "iterations", iv, comment.c_str());
        annJson.addJson(jode, "quickStep", jodeQuickstep);
        bv = _getGlobalBoolParam(prop(PropScene::odeComputeInertias).name, comment);
        annJson.addJson(jode, "computeInertias", bv, comment.c_str());
        QJsonObject jodeInternalScaling;
        bv = _getGlobalBoolParam(prop(PropScene::odeInternalScalingFull).name, comment);
        annJson.addJson(jodeInternalScaling, "full", bv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::odeInternalScalingScaling).name, comment);
        annJson.addJson(jodeInternalScaling, "value", fv, comment.c_str());
        annJson.addJson(jode, "internalScaling", jodeInternalScaling);
        fv = _getGlobalFloatParam(prop(PropScene::odeGlobalErp).name, comment);
        annJson.addJson(jode, "globalErp", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::odeGlobalCfm).name, comment);
        annJson.addJson(jode, "globalCfm", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        iv = _getGlobalIntParam(prop(PropScene::newtonIterations).name, comment);
        annJson.addJson(jnewton, "iterations", iv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::newtonComputeInertias).name, comment);
        annJson.addJson(jnewton, "computeInertias", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::newtonMultithreading).name, comment);
        annJson.addJson(jnewton, "multithreading", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::newtonExactSolver).name, comment);
        annJson.addJson(jnewton, "exactSolver", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::newtonHighJointAccuracy).name, comment);
        annJson.addJson(jnewton, "highJointAccuracy", bv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::newtonContactMergeTolerance).name, comment);
        annJson.addJson(jnewton, "contactMergeTolerance", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        iv = _getGlobalIntParam(prop(PropScene::mujocoIntegrator).name, comment, "0=euler, 1=rk4, 2=implicit, 3=implicitfast");
        annJson.addJson(jmujoco, "integrator", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoSolver).name, comment, "0=pgs, 1=cg, 2=newton");
        annJson.addJson(jmujoco, "solver", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoJacobian).name, comment, "-1=auto 0=dense, 1=sparse");
        annJson.addJson(jmujoco, "jacobian", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoIterations).name, comment);
        annJson.addJson(jmujoco, "iterations", iv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoTolerance).name, comment);
        annJson.addJson(jmujoco, "tolerance", fv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoLs_iterations).name, comment);
        annJson.addJson(jmujoco, "ls_iterations", iv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoLs_tolerance).name, comment);
        annJson.addJson(jmujoco, "ls_tolerance", fv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoNoslip_iterations).name, comment);
        annJson.addJson(jmujoco, "noslip_iterations", iv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoNoslip_tolerance).name, comment);
        annJson.addJson(jmujoco, "noslip_tolerance", fv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoCcd_iterations).name, comment);
        annJson.addJson(jmujoco, "ccd_iterations", iv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoCcd_tolerance).name, comment);
        annJson.addJson(jmujoco, "ccd_tolerance", fv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoSdf_iterations).name, comment);
        annJson.addJson(jmujoco, "sdf_iterations", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoSdf_initpoints).name, comment);
        annJson.addJson(jmujoco, "sdf_initpoints", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoRebuildTrigger).name, comment,
                                "bit coded: bit0=object added, bit1=object removed, bit2=hierarchy changed, "
                                "bit3=dynamic refresh flag found,  bit4=xml injections, bit5=CoppeliaSim particles");
        annJson.addJson(jmujoco, "rebuildTrigger", iv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoComputeInertias).name, comment);
        annJson.addJson(jmujoco, "computeInertias", bv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoMbMemory).name, comment);
        annJson.addJson(jmujoco, "mbMemory", iv, comment.c_str());
        iv = _getGlobalIntParam(prop(PropScene::mujocoCone).name, comment, "0=pyramidal, 1=elliptic");
        annJson.addJson(jmujoco, "cone", iv, comment.c_str());
        QJsonObject jmujocoKinematic;
        iv = _getGlobalIntParam(prop(PropScene::mujocoKinematicBodiesOverrideFlags).name, comment,
                                "0=do not override, 1=all disabled, 2=all enabled");
        annJson.addJson(jmujocoKinematic, "overrideFlags", iv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoKinematicBodiesMass).name, comment);
        annJson.addJson(jmujocoKinematic, "mass", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoKinematicBodiesInertia).name, comment);
        annJson.addJson(jmujocoKinematic, "inertia", fv, comment.c_str());
        annJson.addJson(jmujoco, "kinematicBodies", jmujocoKinematic);
        fv = _getGlobalFloatParam(prop(PropScene::mujocoBoundMass).name, comment);
        annJson.addJson(jmujoco, "boundMass", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoBoundInertia).name, comment);
        annJson.addJson(jmujoco, "boundInertia", fv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoBalanceInertias).name, comment);
        annJson.addJson(jmujoco, "balanceInertias", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoMultithreaded).name, comment);
        annJson.addJson(jmujoco, "multithreaded", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoMulticcd).name, comment);
        annJson.addJson(jmujoco, "multiccd", bv, comment.c_str());
        QJsonObject jmujocoContacts;
        bv = _getGlobalBoolParam(prop(PropScene::mujocoContactParamsOverride).name, comment);
        annJson.addJson(jmujocoContacts, "override", bv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoContactParamsMargin).name, comment);
        annJson.addJson(jmujocoContacts, "margin", fv, comment.c_str());
        double w[5];
        _getGlobalFloatParams(prop(PropScene::mujocoContactParamsSolref).name, w, comment);
        annJson.addJson(jmujocoContacts, "solref", w, 2, comment.c_str());
        _getGlobalFloatParams(prop(PropScene::mujocoContactParamsSolimp).name, w, comment);
        annJson.addJson(jmujocoContacts, "solimp", w, 5, comment.c_str());
        annJson.addJson(jmujoco, "contactParams", jmujocoContacts);
        QJsonObject jmujocoKinematicWeld;
        fv = _getGlobalFloatParam(prop(PropScene::mujocoKinematicWeldTorqueScale).name, comment);
        annJson.addJson(jmujocoKinematicWeld, "torquescale", fv, comment.c_str());
        _getGlobalFloatParams(prop(PropScene::mujocoKinematicWeldSolref).name, w, comment);
        annJson.addJson(jmujocoKinematicWeld, "solref", w, 2, comment.c_str());
        _getGlobalFloatParams(prop(PropScene::mujocoKinematicWeldSolimp).name, w, comment);
        annJson.addJson(jmujocoKinematicWeld, "solimp", w, 5, comment.c_str());
        annJson.addJson(jmujoco, "kinematicWeld", jmujocoKinematicWeld);
        fv = _getGlobalFloatParam(prop(PropScene::mujocoImpRatio).name, comment);
        annJson.addJson(jmujoco, "impratio", fv, comment.c_str());
        _getGlobalFloatParams(prop(PropScene::mujocoWind).name, w, comment);
        annJson.addJson(jmujoco, "wind", w, 3, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoDensity).name, comment);
        annJson.addJson(jmujoco, "density", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::mujocoViscosity).name, comment);
        annJson.addJson(jmujoco, "viscosity", fv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoEqualityEnable).name, comment);
        annJson.addJson(jmujoco, "equalityEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoFrictionlossEnable).name, comment);
        annJson.addJson(jmujoco, "frictionlossEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoLimitEnable).name, comment);
        annJson.addJson(jmujoco, "limitEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoContactEnable).name, comment);
        annJson.addJson(jmujoco, "contactEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoPassiveEnable).name, comment);
        annJson.addJson(jmujoco, "passiveEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoGravityEnable).name, comment);
        annJson.addJson(jmujoco, "gravityEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoWarmstartEnable).name, comment);
        annJson.addJson(jmujoco, "warmstartEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoActuationEnable).name, comment);
        annJson.addJson(jmujoco, "actuationEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoRefsafeEnable).name, comment);
        annJson.addJson(jmujoco, "refsafeEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoSensorEnable).name, comment);
        annJson.addJson(jmujoco, "sensorEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoMidphaseEnable).name, comment);
        annJson.addJson(jmujoco, "midphaseEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoEulerdampEnable).name, comment);
        annJson.addJson(jmujoco, "eulerdampEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoAutoresetEnable).name, comment);
        annJson.addJson(jmujoco, "autoresetEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoEnergyEnable).name, comment);
        annJson.addJson(jmujoco, "energyEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoInvdiscreteEnable).name, comment);
        annJson.addJson(jmujoco, "invdiscreteEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoNativeccdEnable).name, comment);
        annJson.addJson(jmujoco, "nativeccdEnable", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::mujocoAlignfree).name, comment);
        annJson.addJson(jmujoco, "alignfree", bv, comment.c_str());

        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        bv = _getGlobalBoolParam(prop(PropScene::vortexComputeInertias).name, comment);
        annJson.addJson(jvortex, "computeInertias", bv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::vortexContactTolerance).name, comment);
        annJson.addJson(jvortex, "contactTolerance", fv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::vortexAutoSleep).name, comment);
        annJson.addJson(jvortex, "autoSleep", bv, comment.c_str());
        bv = _getGlobalBoolParam(prop(PropScene::vortexMultithreading).name, comment);
        annJson.addJson(jvortex, "multithreading", bv, comment.c_str());
        QJsonObject jvortexConstr;
        fv = _getGlobalFloatParam(prop(PropScene::vortexConstraintsLinearCompliance).name, comment);
        annJson.addJson(jvortexConstr, "linearCompliance", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::vortexConstraintsLinearDamping).name, comment);
        annJson.addJson(jvortexConstr, "linearDamping", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::vortexConstraintsLinearKineticLoss).name, comment);
        annJson.addJson(jvortexConstr, "linearKineticLoss", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::vortexConstraintsAngularCompliance).name, comment);
        annJson.addJson(jvortexConstr, "angularCompliance", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::vortexConstraintsAngularDamping).name, comment);
        annJson.addJson(jvortexConstr, "angularDamping", fv, comment.c_str());
        fv = _getGlobalFloatParam(prop(PropScene::vortexConstraintsAngularKineticLoss).name, comment);
        annJson.addJson(jvortexConstr, "angularKineticLoss", fv, comment.c_str());
        annJson.addJson(jvortex, "constraints", jvortexConstr);
        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readGlobal(int engine, CAnnJson& annJson, std::string* allErrors) const
{
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet, "solver", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::bulletSolver).name,
                                                                     val.toInt());
            if (annJson.getValue(bullet, "iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::bulletIterations).name,
                                                                     val.toInt());
            if (annJson.getValue(bullet, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::bulletComputeInertias).name,
                                                                      val.toBool());
            if (annJson.getValue(bullet, "internalScaling", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "full", QJsonValue::Bool, val, allErrors))
                    App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::bulletInternalScalingFull).name,
                                                                          val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::bulletInternalScalingScaling).name,
                                                                           val.toDouble());
            }
            if (annJson.getValue(bullet, "collisionMarginScaling", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::bulletCollMarginScaling).name,
                                                                       val.toDouble());
        }
    }

    if (engine == sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "ode", QJsonValue::Object, val, allErrors))
        {
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode, "quickStep", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::odeQuickStepEnabled).name, val.toBool());
                if (annJson.getValue(sub, "iterations", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setIntProperty(prop(PropScene::odeQuickStepIterations).name, val.toInt());
            }
            if (annJson.getValue(ode, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::odeComputeInertias).name, val.toBool());
            if (annJson.getValue(ode, "internalScaling", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "full", QJsonValue::Bool, val, allErrors))
                    App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::odeInternalScalingFull).name,
                                                                          val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::odeInternalScalingScaling).name,
                                                                           val.toDouble());
            }
            if (annJson.getValue(ode, "globalErp", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::odeGlobalErp).name, val.toDouble());
            if (annJson.getValue(ode, "globalCfm", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::odeGlobalCfm).name, val.toDouble());
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
            QJsonObject newton(val.toObject());
            if (annJson.getValue(newton, "iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::newtonIterations).name,
                                                                     val.toInt());
            if (annJson.getValue(newton, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::newtonComputeInertias).name,
                                                                      val.toBool());
            if (annJson.getValue(newton, "multithreading", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::newtonMultithreading).name,
                                                                      val.toBool());
            if (annJson.getValue(newton, "exactSolver", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::newtonExactSolver).name, val.toBool());
            if (annJson.getValue(newton, "highJointAccuracy", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::newtonHighJointAccuracy).name,
                                                                      val.toBool());
            if (annJson.getValue(newton, "contactMergeTolerance", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::newtonContactMergeTolerance).name,
                                                                       val.toDouble());
        }
    }

    if (engine == sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "mujoco", QJsonValue::Object, val, allErrors))
        {
            double w[5];
            QJsonObject mujoco(val.toObject());
            if (annJson.getValue(mujoco, "integrator", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoIntegrator).name, val.toInt());
            if (annJson.getValue(mujoco, "solver", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoSolver).name, val.toInt());
            if (annJson.getValue(mujoco, "jacobian", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoJacobian).name, val.toInt());
            if (annJson.getValue(mujoco, "iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoIterations).name, val.toInt());
            if (annJson.getValue(mujoco, "tolerance", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoTolerance).name, val.toDouble());
            if (annJson.getValue(mujoco, "ls_iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoLs_iterations).name, val.toInt());
            if (annJson.getValue(mujoco, "ls_tolerance", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoLs_tolerance).name, val.toDouble());
            if (annJson.getValue(mujoco, "noslip_iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoNoslip_iterations).name, val.toInt());
            if (annJson.getValue(mujoco, "noslip_tolerance", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoNoslip_tolerance).name, val.toDouble());
            if (annJson.getValue(mujoco, "ccd_iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoCcd_iterations).name, val.toInt());
            if (annJson.getValue(mujoco, "ccd_tolerance", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoCcd_tolerance).name, val.toDouble());
            if (annJson.getValue(mujoco, "sdf_iterations", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoSdf_iterations).name, val.toInt());
            if (annJson.getValue(mujoco, "sdf_initpoints", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoSdf_initpoints).name, val.toInt());
            if (annJson.getValue(mujoco, "rebuildTrigger", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoRebuildTrigger).name, val.toInt());
            if (annJson.getValue(mujoco, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoComputeInertias).name, val.toBool());
            if (annJson.getValue(mujoco, "mbMemory", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoMbMemory).name, val.toInt());
            if (annJson.getValue(mujoco, "cone", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoCone).name, val.toInt());
            if (annJson.getValue(mujoco, "kinematicBodies", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "overrideFlags", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setIntProperty(prop(PropScene::mujocoKinematicBodiesOverrideFlags).name, val.toInt());
                if (annJson.getValue(sub, "mass", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoKinematicBodiesMass).name,
                                                                           val.toDouble());
                if (annJson.getValue(sub, "inertia", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoKinematicBodiesInertia).name,
                                                                           val.toDouble());
            }
            if (annJson.getValue(mujoco, "boundMass", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoBoundMass).name, val.toDouble());
            if (annJson.getValue(mujoco, "boundInertia", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoBoundInertia).name,
                                                                       val.toDouble());
            if (annJson.getValue(mujoco, "balanceInertias", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoBalanceInertias).name,
                                                                      val.toBool());
            if (annJson.getValue(mujoco, "multithreaded", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoMultithreaded).name, val.toBool());
            if (annJson.getValue(mujoco, "multiccd", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoMulticcd).name, val.toBool());
            if (annJson.getValue(mujoco, "contactParams", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "override", QJsonValue::Bool, val, allErrors))
                    App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoContactParamsOverride).name, val.toBool());
                if (annJson.getValue(sub, "margin", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoContactParamsMargin).name, val.toDouble());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    App::scene->dynamicsContainer->setFloatArrayProperty(prop(PropScene::mujocoContactParamsSolref).name, ww);
                }
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    App::scene->dynamicsContainer->setFloatArrayProperty(prop(PropScene::mujocoContactParamsSolimp).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "kinematicWeld", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "torquescale", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoKinematicWeldTorqueScale).name, val.toDouble());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    App::scene->dynamicsContainer->setFloatArrayProperty(prop(PropScene::mujocoKinematicWeldSolref).name, ww);
                }
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    App::scene->dynamicsContainer->setFloatArrayProperty(prop(PropScene::mujocoKinematicWeldSolimp).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "impratio", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoImpRatio).name, val.toDouble());
            if (annJson.getValue(mujoco, "wind", w, 3, allErrors))
            {
                C3Vector w3(w);
                App::scene->dynamicsContainer->setVector3Property(prop(PropScene::mujocoWind).name, &w3);
            }
            if (annJson.getValue(mujoco, "density", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoDensity).name, val.toDouble());
            if (annJson.getValue(mujoco, "viscosity", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::mujocoViscosity).name, val.toDouble());
            if (annJson.getValue(mujoco, "equalityEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoEqualityEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "frictionlossEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoFrictionlossEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "limitEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoLimitEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "contactEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoContactEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "passiveEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoPassiveEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "gravityEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoGravityEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "warmstartEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoWarmstartEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "actuationEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoActuationEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "refsafeEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoRefsafeEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "sensorEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoSensorEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "midphaseEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoMidphaseEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "eulerdampEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoEulerdampEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "autoresetEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoAutoresetEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "energyEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoEnergyEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "invdiscreteEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoInvdiscreteEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "nativeccdEnable", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoNativeccdEnable).name, val.toBool());
            if (annJson.getValue(mujoco, "alignfree", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::mujocoAlignfree).name, val.toBool());
        }
    }

    if (engine == sim_physics_vortex)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "vortex", QJsonValue::Object, val, allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::vortexComputeInertias).name,
                                                                      val.toBool());
            if (annJson.getValue(vortex, "contactTolerance", QJsonValue::Double, val, allErrors))
                App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexContactTolerance).name,
                                                                       val.toDouble());
            if (annJson.getValue(vortex, "autoSleep", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::vortexAutoSleep).name, val.toBool());
            if (annJson.getValue(vortex, "multithreading", QJsonValue::Bool, val, allErrors))
                App::scene->dynamicsContainer->setBoolProperty(prop(PropScene::vortexMultithreading).name,
                                                                      val.toBool());
            if (annJson.getValue(vortex, "constraints", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "linearCompliance", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexConstraintsLinearCompliance).name, val.toDouble());
                if (annJson.getValue(sub, "linearDamping", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexConstraintsLinearDamping).name, val.toDouble());
                if (annJson.getValue(sub, "linearKineticLoss", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexConstraintsLinearKineticLoss).name, val.toDouble());
                if (annJson.getValue(sub, "angularCompliance", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexConstraintsAngularCompliance).name, val.toDouble());
                if (annJson.getValue(sub, "angularDamping", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexConstraintsAngularDamping).name, val.toDouble());
                if (annJson.getValue(sub, "angularKineticLoss", QJsonValue::Double, val, allErrors))
                    App::scene->dynamicsContainer->setFloatProperty(prop(PropScene::vortexConstraintsAngularKineticLoss).name, val.toDouble());
            }
        }
    }
}

double CEngineProperties::_getGlobalFloatParam(const char* item, std::string& comment,
                                               const char* additionalComment /*=nullptr*/) const
{
    double retVal = App::scene->dynamicsContainer->getFloatPropertyValue(item, false);
    double def = App::scene->dynamicsContainer->getFloatPropertyValue(item, true);
    bool similar = true;
    if ((retVal == 0.0) || (def == 0.0))
        similar = (retVal == def);
    else
        similar = fabs((def - retVal) / def) < 0.01;
    comment.clear();
    if (!similar)
    {
        comment = "DEFAULT: ";
        comment += utils::getDoubleString(false, def, 3, 5, 0.0001, 9999);
        if (additionalComment != nullptr)
        {
            comment += " (";
            comment += additionalComment;
            comment += ")";
        }
    }
    else
    {
        if (additionalComment != nullptr)
            comment = additionalComment;
    }
    return retVal;
}

int CEngineProperties::_getGlobalIntParam(const char* item, std::string& comment,
                                          const char* additionalComment /*=nullptr*/) const
{
    int retVal = App::scene->dynamicsContainer->getIntPropertyValue(item, false);
    int def = App::scene->dynamicsContainer->getIntPropertyValue(item, true);
    comment.clear();
    if (retVal != def)
    {
        comment = "DEFAULT: ";
        comment += std::to_string(def);
        if (additionalComment != nullptr)
        {
            comment += " (";
            comment += additionalComment;
            comment += ")";
        }
    }
    else
    {
        if (additionalComment != nullptr)
            comment = additionalComment;
    }
    return retVal;
}

bool CEngineProperties::_getGlobalBoolParam(const char* item, std::string& comment,
                                            const char* additionalComment /*=nullptr*/) const
{
    bool retVal = App::scene->dynamicsContainer->getBoolPropertyValue(item, false);
    bool def = App::scene->dynamicsContainer->getBoolPropertyValue(item, true);
    comment.clear();
    if (retVal != def)
    {
        comment = "DEFAULT: ";
        if (def)
            comment += "true";
        else
            comment += "false";
        if (additionalComment != nullptr)
        {
            comment += " (";
            comment += additionalComment;
            comment += ")";
        }
    }
    else
    {
        if (additionalComment != nullptr)
            comment = additionalComment;
    }
    return retVal;
}

void CEngineProperties::_getGlobalFloatParams(const char* item, double* w, std::string& comment, const char* additionalComment /*=nullptr*/) const
{
    std::vector<double> p;
    std::vector<double> p0;
    if (App::scene->dynamicsContainer->getFloatArrayProperty(item, p, false) == 1)
        App::scene->dynamicsContainer->getFloatArrayProperty(item, p0, true);
    else
    {

        C3Vector v;
        C3Vector v0;
        App::scene->dynamicsContainer->getVector3Property(item, v, false);
        App::scene->dynamicsContainer->getVector3Property(item, v0, true);
        for (size_t i = 0; i < 3; i++)
        {
            p.push_back(v(i));
            p0.push_back(v0(i));
        }
    }

    bool similar = true;
    for (size_t i = 0; i < p0.size(); i++)
    {
        w[i] = p[i];
        if ((p[i] == 0.0) || (p0[i] == 0.0))
        {
            if (p[i] != p0[i])
                similar = false;
        }
        else
        {
            if (fabs((p0[i] - p[i]) / p0[i]) > 0.01)
                similar = false;
        }
    }
    comment.clear();
    if (!similar)
    {
        comment = "DEFAULT: [";
        for (size_t i = 0; i < p0.size(); i++)
        {
            if (i != 0)
                comment += ",";
            comment += utils::getDoubleString(false, p0[i], 3, 5, 0.0001, 9999);
        }
        comment += "]";
        if (additionalComment != nullptr)
        {
            comment += " (";
            comment += additionalComment;
            comment += ")";
        }
    }
    else
    {
        if (additionalComment != nullptr)
            comment = additionalComment;
    }
}

void CEngineProperties::_writeDummy(int engine, int dummyHandle, CAnnJson& annJson) const
{
    CDummy* dummy = App::scene->sceneObjects->getDummyFromHandle(dummyHandle);

    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        QJsonObject jmujocoLimits;
        annJson.addJson(jmujocoLimits, "enabled", dummy->getBoolPropertyValue(prop(PropDummy::mujocoLimitsEnabled).name));
        std::vector<double> v;
        dummy->getFloatArrayProperty(prop(PropDummy::mujocoLimitsRange).name, v);
        annJson.addJson(jmujocoLimits, "range", v.data(), 2);
        dummy->getFloatArrayProperty(prop(PropDummy::mujocoLimitsSolref).name, v);
        annJson.addJson(jmujocoLimits, "solref", v.data(), 2);
        dummy->getFloatArrayProperty(prop(PropDummy::mujocoLimitsSolimp).name, v);
        annJson.addJson(jmujocoLimits, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "limits", jmujocoLimits);

        QJsonObject jmujocoOverlapConstr;
        dummy->getFloatArrayProperty(prop(PropDummy::mujocoOverlapConstrSolref).name, v);
        annJson.addJson(jmujocoOverlapConstr, "solref", v.data(), 2);
        dummy->getFloatArrayProperty(prop(PropDummy::mujocoOverlapConstrSolimp).name, v);
        annJson.addJson(jmujocoOverlapConstr, "solimp", v.data(), 5);
        annJson.addJson(jmujocoOverlapConstr, "torquescale", dummy->getFloatPropertyValue(prop(PropDummy::mujocoOverlapConstrTorqueScale).name));
        annJson.addJson(jmujoco, "overlapConstraint", jmujocoOverlapConstr);

        QJsonObject jmujocoSpring;
        annJson.addJson(jmujocoSpring, "stiffness", dummy->getFloatPropertyValue(prop(PropDummy::mujocoSpringStiffness).name));
        annJson.addJson(jmujocoSpring, "damping", dummy->getFloatPropertyValue(prop(PropDummy::mujocoSpringDamping).name));
        annJson.addJson(jmujocoSpring, "length", dummy->getFloatPropertyValue(prop(PropDummy::mujocoSpringLength).name));
        annJson.addJson(jmujoco, "spring", jmujocoSpring);
        annJson.addJson(jmujoco, "margin", dummy->getFloatPropertyValue(prop(PropDummy::mujocoMargin).name));
        QJsonObject jmujocoDependency;
        int h = dummy->getIntPropertyValue(prop(PropDummy::mujocoJointProxyHandle).name);
        std::string nameAndPath;
        if (h >= 0)
        {
            CJoint* obj = App::scene->sceneObjects->getJointFromHandle(h);
            if (obj != nullptr)
                nameAndPath = obj->getObjectAlias_shortPath();
        }
        annJson.addJson(jmujocoDependency, "name", nameAndPath.c_str(), "specify the full, unique path");
        annJson.addJson(jmujoco, "jointProxy", jmujocoDependency);
        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readDummy(int engine, int dummyHandle, CAnnJson& annJson, std::string* allErrors) const
{
    CDummy* dummy = App::scene->sceneObjects->getDummyFromHandle(dummyHandle);
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
        }
    }

    if (engine == sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "ode", QJsonValue::Object, val, allErrors))
        {
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
        }
    }

    if (engine == sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "mujoco", QJsonValue::Object, val, allErrors))
        {
            QJsonObject mujoco(val.toObject());
            double w[5];
            if (annJson.getValue(mujoco, "limits", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    dummy->setBoolProperty(prop(PropDummy::mujocoLimitsEnabled).name, val.toBool());
                if (annJson.getValue(sub, "range", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    dummy->setFloatArrayProperty(prop(PropDummy::mujocoLimitsRange).name, ww);
                }
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    dummy->setFloatArrayProperty(prop(PropDummy::mujocoLimitsSolref).name, ww);
                }
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    dummy->setFloatArrayProperty(prop(PropDummy::mujocoLimitsSolimp).name, ww);
                }
            }
            if (annJson.getValue(mujoco, "overlapConstraint", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                {
                    std::vector<double> ww(w, w + 2);
                    dummy->setFloatArrayProperty(prop(PropDummy::mujocoOverlapConstrSolref).name, ww);
                }
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                {
                    std::vector<double> ww(w, w + 5);
                    dummy->setFloatArrayProperty(prop(PropDummy::mujocoOverlapConstrSolimp).name, ww);
                }
                if (annJson.getValue(sub, "torquescale", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(prop(PropDummy::mujocoOverlapConstrTorqueScale).name, val.toDouble());
            }
            if (annJson.getValue(mujoco, "spring", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "stiffness", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(prop(PropDummy::mujocoSpringStiffness).name, val.toDouble());
                if (annJson.getValue(sub, "damping", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(prop(PropDummy::mujocoSpringDamping).name, val.toDouble());
                if (annJson.getValue(sub, "length", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(prop(PropDummy::mujocoSpringLength).name, val.toDouble());
            }
            if (annJson.getValue(mujoco, "margin", QJsonValue::Double, val, allErrors))
                dummy->setFloatProperty(prop(PropDummy::mujocoMargin).name, val.toDouble());
            if (annJson.getValue(mujoco, "jointProxy", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "name", QJsonValue::String, val, allErrors))
                {
                    int h = -1;
                    std::string str(val.toString().toStdString());
                    if (str.size() > 0)
                    {
                        CSceneObject* obj = App::scene->sceneObjects->getObjectFromPath(nullptr, str.c_str(), 0);
                        if ((obj != nullptr) && (obj->getObjectType() == sim_sceneobject_joint))
                        {
                            h = obj->getObjectHandle();
                            dummy->setIntProperty(prop(PropDummy::mujocoJointProxyHandle).name, h);
                        }
                        else
                        {
                            if (allErrors->size() > 0)
                                allErrors[0] += "\n";
                            allErrors[0] +=
                                "Key 'jointProxy/name' does not point to a valid joint object and will be ignored.";
                        }
                    }
                    else
                        dummy->setIntProperty(prop(PropDummy::mujocoJointProxyHandle).name, h);
                }
            }
        }
    }

    if (engine == sim_physics_vortex)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "vortex", QJsonValue::Object, val, allErrors))
        {
        }
    }
}
