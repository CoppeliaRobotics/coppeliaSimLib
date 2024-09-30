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
    CSceneObject *object = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

    QJsonObject jmain;
    CAnnJson annJson(&jmain);
    if (title != nullptr)
        title[0] = "Dynamic engine global properties";
    int engine = App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);

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
    CSceneObject *object = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);

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

void CEngineProperties::_writeJoint(int engine, int jointHandle, CAnnJson &annJson) const
{
    CJoint *joint = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);

    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(jbullet, "normalCfm", joint->getFloatPropertyValue(propJoint_bulletNormalCfm.name));
        annJson.addJson(jbullet, "stopErp", joint->getFloatPropertyValue(propJoint_bulletStopErp.name));
        annJson.addJson(jbullet, "stopCfm", joint->getFloatPropertyValue(propJoint_bulletStopCfm.name));
        C3Vector v;
        joint->getVector3Property(propJoint_bulletPosPid.name, v);
        annJson.addJson(jbullet, "posPid", v.data, 3);
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(jode, "normalCfm", joint->getFloatPropertyValue(propJoint_odeNormalCfm.name));
        annJson.addJson(jode, "stopErp", joint->getFloatPropertyValue(propJoint_odeStopErp.name));
        annJson.addJson(jode, "stopCfm", joint->getFloatPropertyValue(propJoint_odeStopCfm.name));
        annJson.addJson(jode, "bounce", joint->getFloatPropertyValue(propJoint_odeBounce.name));
        annJson.addJson(jode, "fudge", joint->getFloatPropertyValue(propJoint_odeFudgeFactor.name));
        C3Vector v;
        joint->getVector3Property(propJoint_odePosPid.name, v);
        annJson.addJson(jode, "posPid", v.data, 3);
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        C3Vector v;
        joint->getVector3Property(propJoint_newtonPosPid.name, v);
        annJson.addJson(jnewton, "posPid", v.data, 3);
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        annJson.addJson(jmujoco, "armature", joint->getFloatPropertyValue(propJoint_mujocoArmature.name));
        annJson.addJson(jmujoco, "margin", joint->getFloatPropertyValue(propJoint_mujocoMargin.name));
        QJsonObject jmujocoLimits;
        double vec[2];
        joint->getVector2Property(propJoint_mujocoLimitsSolRef.name, vec);
        annJson.addJson(jmujocoLimits, "solref", vec, 2);
        std::vector<double> v;
        joint->getVectorProperty(propJoint_mujocoLimitsSolImp.name, v);
        annJson.addJson(jmujocoLimits, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "limits", jmujocoLimits);
        QJsonObject jmujocoFriction;
        annJson.addJson(jmujocoFriction, "loss", joint->getFloatPropertyValue(propJoint_mujocoFrictionLoss.name));
        joint->getVector2Property(propJoint_mujocoFrictionSolRef.name, vec);
        annJson.addJson(jmujocoFriction, "solref", vec, 2);
        joint->getVectorProperty(propJoint_mujocoFrictionSolImp.name, v);
        annJson.addJson(jmujocoFriction, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "friction", jmujocoFriction);
        QJsonObject jmujocoSpring;
        annJson.addJson(jmujocoSpring, "stiffness", joint->getFloatPropertyValue(propJoint_mujocoSpringStiffness.name));
        annJson.addJson(jmujocoSpring, "damping", joint->getFloatPropertyValue(propJoint_mujocoSpringDamping.name));
        annJson.addJson(jmujocoSpring, "ref", joint->getFloatPropertyValue(propJoint_mujocoSpringRef.name));
        joint->getVector2Property(propJoint_mujocoSpringDamper.name, vec);
        annJson.addJson(jmujocoSpring, "springDamper", vec, 2);
        annJson.addJson(jmujoco, "spring", jmujocoSpring);
        QJsonObject jmujocoDependency;
        joint->getVectorProperty(propJoint_mujocoDependencyPolyCoef.name, v);
        annJson.addJson(jmujocoDependency, "polyCoeff", v.data(), 5);
        annJson.addJson(jmujoco, "dependency", jmujocoDependency);
        C3Vector vec3;
        joint->getVector3Property(propJoint_mujocoPosPid.name, vec3);
        annJson.addJson(jmujoco, "posPid", vec3.data, 3);
        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        QJsonObject jvortexAxisFriction;
        annJson.addJson(jvortexAxisFriction, "enabled",
                        joint->getBoolPropertyValue(propJoint_vortexAxisFrictionEnabled.name));
        annJson.addJson(jvortexAxisFriction, "proportional",
                        joint->getBoolPropertyValue(propJoint_vortexAxisFrictionProportional.name));
        annJson.addJson(jvortexAxisFriction, "value",
                        joint->getFloatPropertyValue(propJoint_vortexAxisFrictionCoeff.name));
        annJson.addJson(jvortexAxisFriction, "maxForce",
                        joint->getFloatPropertyValue(propJoint_vortexAxisFrictionMaxForce.name));
        annJson.addJson(jvortexAxisFriction, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexAxisFrictionLoss.name));
        annJson.addJson(jvortex, "axisFriction", jvortexAxisFriction);
        QJsonObject jvortexAxisLimits;
        annJson.addJson(jvortexAxisLimits, "lowerRestitution",
                        joint->getFloatPropertyValue(propJoint_vortexLowerLimitRestitution.name));
        annJson.addJson(jvortexAxisLimits, "upperRestitution",
                        joint->getFloatPropertyValue(propJoint_vortexUpperLimitRestitution.name));
        annJson.addJson(jvortexAxisLimits, "lowerStiffness",
                        joint->getFloatPropertyValue(propJoint_vortexLowerLimitStiffness.name));
        annJson.addJson(jvortexAxisLimits, "upperStiffness",
                        joint->getFloatPropertyValue(propJoint_vortexUpperLimitStiffness.name));
        annJson.addJson(jvortexAxisLimits, "lowerDamping",
                        joint->getFloatPropertyValue(propJoint_vortexLowerLimitDamping.name));
        annJson.addJson(jvortexAxisLimits, "upperDamping",
                        joint->getFloatPropertyValue(propJoint_vortexUpperLimitDamping.name));
        annJson.addJson(jvortexAxisLimits, "lowerMaxForce",
                        joint->getFloatPropertyValue(propJoint_vortexLowerLimitMaxForce.name));
        annJson.addJson(jvortexAxisLimits, "upperMaxForce",
                        joint->getFloatPropertyValue(propJoint_vortexUpperLimitMaxForce.name));
        annJson.addJson(jvortex, "axisLimits", jvortexAxisLimits);

        int vval;
        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
        bool P0_relaxation_enabled = ((vval & 1) != 0);
        bool P1_relaxation_enabled = ((vval & 2) != 0);
        bool P2_relaxation_enabled = ((vval & 4) != 0);
        bool A0_relaxation_enabled = ((vval & 8) != 0);
        bool A1_relaxation_enabled = ((vval & 16) != 0);
        bool A2_relaxation_enabled = ((vval & 32) != 0);
        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
        bool P0_friction_enabled = ((vval & 1) != 0);
        bool P1_friction_enabled = ((vval & 2) != 0);
        bool P2_friction_enabled = ((vval & 4) != 0);
        bool A0_friction_enabled = ((vval & 8) != 0);
        bool A1_friction_enabled = ((vval & 16) != 0);
        bool A2_friction_enabled = ((vval & 32) != 0);
        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
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
                        joint->getFloatPropertyValue(propJoint_vortexXAxisPosRelaxationStiffness.name));
        annJson.addJson(jvortexXaxisPositionRelax, "damping",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisPosRelaxationDamping.name));
        annJson.addJson(jvortexXaxisPositionRelax, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisPosRelaxationLoss.name));
        annJson.addJson(jvortexXaxisPosition, "relaxation", jvortexXaxisPositionRelax);
        QJsonObject jvortexXaxisPositionFric;
        annJson.addJson(jvortexXaxisPositionFric, "enabled", P0_friction_enabled);
        annJson.addJson(jvortexXaxisPositionFric, "proportional", P0_friction_proportional);
        annJson.addJson(jvortexXaxisPositionFric, "value",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisPosFrictionCoeff.name));
        annJson.addJson(jvortexXaxisPositionFric, "maxForce",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisPosFrictionMaxForce.name));
        annJson.addJson(jvortexXaxisPositionFric, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisPosFrictionLoss.name));
        annJson.addJson(jvortexXaxisPosition, "friction", jvortexXaxisPositionFric);
        annJson.addJson(jvortex, "xAxisPos", jvortexXaxisPosition);

        QJsonObject jvortexYaxisPosition;
        QJsonObject jvortexYaxisPositionRelax;
        annJson.addJson(jvortexYaxisPositionRelax, "enabled", P1_relaxation_enabled);
        annJson.addJson(jvortexYaxisPositionRelax, "stiffness",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisPosRelaxationStiffness.name));
        annJson.addJson(jvortexYaxisPositionRelax, "damping",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisPosRelaxationDamping.name));
        annJson.addJson(jvortexYaxisPositionRelax, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisPosRelaxationLoss.name));
        annJson.addJson(jvortexYaxisPosition, "relaxation", jvortexYaxisPositionRelax);
        QJsonObject jvortexYaxisPositionFric;
        annJson.addJson(jvortexYaxisPositionFric, "enabled", P1_friction_enabled);
        annJson.addJson(jvortexYaxisPositionFric, "proportional", P1_friction_proportional);
        annJson.addJson(jvortexYaxisPositionFric, "value",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisPosFrictionCoeff.name));
        annJson.addJson(jvortexYaxisPositionFric, "maxForce",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisPosFrictionMaxForce.name));
        annJson.addJson(jvortexYaxisPositionFric, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisPosFrictionLoss.name));
        annJson.addJson(jvortexYaxisPosition, "friction", jvortexYaxisPositionFric);
        annJson.addJson(jvortex, "yAxisPos", jvortexYaxisPosition);

        QJsonObject jvortexZaxisPosition;
        QJsonObject jvortexZaxisPositionRelax;
        annJson.addJson(jvortexZaxisPositionRelax, "enabled", P2_relaxation_enabled);
        annJson.addJson(jvortexZaxisPositionRelax, "stiffness",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisPosRelaxationStiffness.name));
        annJson.addJson(jvortexZaxisPositionRelax, "damping",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisPosRelaxationDamping.name));
        annJson.addJson(jvortexZaxisPositionRelax, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisPosRelaxationLoss.name));
        annJson.addJson(jvortexZaxisPosition, "relaxation", jvortexZaxisPositionRelax);
        QJsonObject jvortexZaxisPositionFric;
        annJson.addJson(jvortexZaxisPositionFric, "enabled", P2_friction_enabled);
        annJson.addJson(jvortexZaxisPositionFric, "proportional", P2_friction_proportional);
        annJson.addJson(jvortexZaxisPositionFric, "value",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisPosFrictionCoeff.name));
        annJson.addJson(jvortexZaxisPositionFric, "maxForce",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisPosFrictionMaxForce.name));
        annJson.addJson(jvortexZaxisPositionFric, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisPosFrictionLoss.name));
        annJson.addJson(jvortexZaxisPosition, "friction", jvortexZaxisPositionFric);
        annJson.addJson(jvortex, "zAxisPos", jvortexZaxisPosition);

        QJsonObject jvortexXaxisOrientation;
        QJsonObject jvortexXaxisOrientationRelax;
        annJson.addJson(jvortexXaxisOrientationRelax, "enabled", A0_relaxation_enabled);
        annJson.addJson(jvortexXaxisOrientationRelax, "stiffness",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisOrientRelaxStiffness.name));
        annJson.addJson(jvortexXaxisOrientationRelax, "damping",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisOrientRelaxDamping.name));
        annJson.addJson(jvortexXaxisOrientationRelax, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisOrientRelaxLoss.name));
        annJson.addJson(jvortexXaxisOrientation, "relaxation", jvortexXaxisOrientationRelax);
        QJsonObject jvortexXaxisOrientationFric;
        annJson.addJson(jvortexXaxisOrientationFric, "enabled", A0_friction_enabled);
        annJson.addJson(jvortexXaxisOrientationFric, "proportional", A0_friction_proportional);
        annJson.addJson(jvortexXaxisOrientationFric, "value",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisOrientFrictionCoeff.name));
        annJson.addJson(jvortexXaxisOrientationFric, "maxTorque",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisOrientFrictionMaxTorque.name));
        annJson.addJson(jvortexXaxisOrientationFric, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexXAxisOrientFrictionLoss.name));
        annJson.addJson(jvortexXaxisOrientation, "friction", jvortexXaxisOrientationFric);
        annJson.addJson(jvortex, "xAxisOrient", jvortexXaxisOrientation);

        QJsonObject jvortexYaxisOrientation;
        QJsonObject jvortexYaxisOrientationRelax;
        annJson.addJson(jvortexYaxisOrientationRelax, "enabled", A1_relaxation_enabled);
        annJson.addJson(jvortexYaxisOrientationRelax, "stiffness",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisOrientRelaxStiffness.name));
        annJson.addJson(jvortexYaxisOrientationRelax, "damping",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisOrientRelaxDamping.name));
        annJson.addJson(jvortexYaxisOrientationRelax, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisOrientRelaxLoss.name));
        annJson.addJson(jvortexYaxisOrientation, "relaxation", jvortexYaxisOrientationRelax);
        QJsonObject jvortexYaxisOrientationFric;
        annJson.addJson(jvortexYaxisOrientationFric, "enabled", A1_friction_enabled);
        annJson.addJson(jvortexYaxisOrientationFric, "proportional", A1_friction_proportional);
        annJson.addJson(jvortexYaxisOrientationFric, "value",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisOrientFrictionCoeff.name));
        annJson.addJson(jvortexYaxisOrientationFric, "maxTorque",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisOrientFrictionMaxTorque.name));
        annJson.addJson(jvortexYaxisOrientationFric, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexYAxisOrientFrictionLoss.name));
        annJson.addJson(jvortexYaxisOrientation, "friction", jvortexYaxisOrientationFric);
        annJson.addJson(jvortex, "yAxisOrient", jvortexYaxisOrientation);

        QJsonObject jvortexZaxisOrientation;
        QJsonObject jvortexZaxisOrientationRelax;
        annJson.addJson(jvortexZaxisOrientationRelax, "enabled", A2_relaxation_enabled);
        annJson.addJson(jvortexZaxisOrientationRelax, "stiffness",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisOrientRelaxStiffness.name));
        annJson.addJson(jvortexZaxisOrientationRelax, "damping",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisOrientRelaxDamping.name));
        annJson.addJson(jvortexZaxisOrientationRelax, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisOrientRelaxLoss.name));
        annJson.addJson(jvortexZaxisOrientation, "relaxation", jvortexZaxisOrientationRelax);
        QJsonObject jvortexZaxisOrientationFric;
        annJson.addJson(jvortexZaxisOrientationFric, "enabled", A2_friction_enabled);
        annJson.addJson(jvortexZaxisOrientationFric, "proportional", A2_friction_proportional);
        annJson.addJson(jvortexZaxisOrientationFric, "value",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisOrientFrictionCoeff.name));
        annJson.addJson(jvortexZaxisOrientationFric, "maxTorque",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisOrientFrictionMaxTorque.name));
        annJson.addJson(jvortexZaxisOrientationFric, "loss",
                        joint->getFloatPropertyValue(propJoint_vortexZAxisOrientFrictionLoss.name));
        annJson.addJson(jvortexZaxisOrientation, "friction", jvortexZaxisOrientationFric);
        annJson.addJson(jvortex, "zAxisOrient", jvortexZaxisOrientation);

        C3Vector v;
        joint->getVector3Property(propJoint_vortexPosPid.name, v);
        annJson.addJson(jvortex, "posPid", v.data, 3);

        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readJoint(int engine, int jointHandle, CAnnJson &annJson, std::string *allErrors) const
{
    CJoint *joint = App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
            double w[3];
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet, "normalCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_bulletNormalCfm.name, val.toDouble());
            if (annJson.getValue(bullet, "stopErp", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_bulletStopErp.name, val.toDouble());
            if (annJson.getValue(bullet, "stopCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_bulletStopCfm.name, val.toDouble());
            if (annJson.getValue(bullet, "posPid", w, 3, allErrors))
                joint->setVector3Property(propJoint_bulletPosPid.name, C3Vector(w));
        }
    }

    if (engine == sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "ode", QJsonValue::Object, val, allErrors))
        {
            double w[3];
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode, "normalCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_odeNormalCfm.name, val.toDouble());
            if (annJson.getValue(ode, "stopErp", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_odeStopErp.name, val.toDouble());
            if (annJson.getValue(ode, "stopCfm", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_odeStopCfm.name, val.toDouble());
            if (annJson.getValue(ode, "bounce", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_odeBounce.name, val.toDouble());
            if (annJson.getValue(ode, "fudge", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_odeFudgeFactor.name, val.toDouble());
            if (annJson.getValue(ode, "posPid", w, 3, allErrors))
                joint->setVector3Property(propJoint_odePosPid.name, C3Vector(w));
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
            QJsonObject newton(val.toObject());
            double w[3];
            if (annJson.getValue(newton, "posPid", w, 3, allErrors))
                joint->setVector3Property(propJoint_newtonPosPid.name, C3Vector(w));
        }
    }

    if (engine == sim_physics_mujoco)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "mujoco", QJsonValue::Object, val, allErrors))
        {
            QJsonObject mujoco(val.toObject());
            if (annJson.getValue(mujoco, "armature", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_mujocoArmature.name, val.toDouble());
            if (annJson.getValue(mujoco, "margin", QJsonValue::Double, val, allErrors))
                joint->setFloatProperty(propJoint_mujocoMargin.name, val.toDouble());
            double w[5];
            if (annJson.getValue(mujoco, "limits", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                    joint->setVector2Property(propJoint_mujocoLimitsSolRef.name, w);
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                    joint->setVectorProperty(propJoint_mujocoLimitsSolImp.name, w, 5);
            }
            if (annJson.getValue(mujoco, "friction", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "loss", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_mujocoFrictionLoss.name, val.toDouble());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                    joint->setVector2Property(propJoint_mujocoFrictionSolRef.name, w);
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                    joint->setVectorProperty(propJoint_mujocoFrictionSolImp.name, w, 5);
            }
            if (annJson.getValue(mujoco, "spring", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "stiffness", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_mujocoSpringStiffness.name, val.toDouble());
                if (annJson.getValue(sub, "damping", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_mujocoSpringDamping.name, val.toDouble());
                if (annJson.getValue(sub, "ref", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_mujocoSpringRef.name, val.toDouble());
                if (annJson.getValue(sub, "springDamper", w, 2, allErrors))
                    joint->setVector2Property(propJoint_mujocoSpringDamper.name, w);
            }
            if (annJson.getValue(mujoco, "dependency", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "polyCoeff", w, 5, allErrors))
                    joint->setVectorProperty(propJoint_mujocoDependencyPolyCoef.name, w, 5);
            }
            if (annJson.getValue(mujoco, "posPid", w, 3, allErrors))
                joint->setVector3Property(propJoint_mujocoPosPid.name, C3Vector(w));
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
                    joint->setBoolProperty(propJoint_vortexAxisFrictionEnabled.name, val.toBool());
                if (annJson.getValue(sub, "proportional", QJsonValue::Bool, val, allErrors))
                    joint->setBoolProperty(propJoint_vortexAxisFrictionProportional.name, val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexAxisFrictionCoeff.name, val.toDouble());
                if (annJson.getValue(sub, "maxForce", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexAxisFrictionMaxForce.name, val.toDouble());
                if (annJson.getValue(sub, "loss", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexAxisFrictionLoss.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "axisLimits", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "lowerRestitution", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexLowerLimitRestitution.name, val.toDouble());
                if (annJson.getValue(sub, "upperRestitution", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexUpperLimitRestitution.name, val.toDouble());
                if (annJson.getValue(sub, "lowerStiffness", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexLowerLimitStiffness.name, val.toDouble());
                if (annJson.getValue(sub, "upperStiffness", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexUpperLimitStiffness.name, val.toDouble());
                if (annJson.getValue(sub, "lowerDamping", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexLowerLimitDamping.name, val.toDouble());
                if (annJson.getValue(sub, "upperDamping", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexUpperLimitDamping.name, val.toDouble());
                if (annJson.getValue(sub, "lowerMaxForce", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexLowerLimitMaxForce.name, val.toDouble());
                if (annJson.getValue(sub, "upperMaxForce", QJsonValue::Double, val, allErrors))
                    joint->setFloatProperty(propJoint_vortexUpperLimitMaxForce.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "xAxisPos", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "relaxation", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
                        vval = (vval | 1) - 1;
                        if (val.toBool())
                            vval |= 1;
                        joint->setIntProperty(propJoint_vortexRelaxationEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisPosRelaxationStiffness.name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisPosRelaxationDamping.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisPosRelaxationLoss.name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
                        vval = (vval | 1) - 1;
                        if (val.toBool())
                            vval |= 1;
                        joint->setIntProperty(propJoint_vortexFrictionEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
                        vval = (vval | 1) - 1;
                        if (val.toBool())
                            vval |= 1;
                        joint->setIntProperty(propJoint_vortexFrictionProportionalBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisPosFrictionCoeff.name, val.toDouble());
                    if (annJson.getValue(sub2, "maxForce", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisPosFrictionMaxForce.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisPosFrictionLoss.name, val.toDouble());
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
                        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
                        vval = (vval | 2) - 2;
                        if (val.toBool())
                            vval |= 2;
                        joint->setIntProperty(propJoint_vortexRelaxationEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisPosRelaxationStiffness.name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisPosRelaxationDamping.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisPosRelaxationLoss.name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
                        vval = (vval | 2) - 2;
                        if (val.toBool())
                            vval |= 2;
                        joint->setIntProperty(propJoint_vortexFrictionEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
                        vval = (vval | 2) - 2;
                        if (val.toBool())
                            vval |= 2;
                        joint->setIntProperty(propJoint_vortexFrictionProportionalBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisPosFrictionCoeff.name, val.toDouble());
                    if (annJson.getValue(sub2, "maxForce", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisPosFrictionMaxForce.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisPosFrictionLoss.name, val.toDouble());
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
                        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
                        vval = (vval | 4) - 4;
                        if (val.toBool())
                            vval |= 4;
                        joint->setIntProperty(propJoint_vortexRelaxationEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisPosRelaxationStiffness.name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisPosRelaxationDamping.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisPosRelaxationLoss.name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
                        vval = (vval | 4) - 4;
                        if (val.toBool())
                            vval |= 4;
                        joint->setIntProperty(propJoint_vortexFrictionEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
                        vval = (vval | 4) - 4;
                        if (val.toBool())
                            vval |= 4;
                        joint->setIntProperty(propJoint_vortexFrictionProportionalBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisPosFrictionCoeff.name, val.toDouble());
                    if (annJson.getValue(sub2, "maxForce", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisPosFrictionMaxForce.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisPosFrictionLoss.name, val.toDouble());
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
                        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
                        vval = (vval | 8) - 8;
                        if (val.toBool())
                            vval |= 8;
                        joint->setIntProperty(propJoint_vortexRelaxationEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisOrientRelaxStiffness.name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisOrientRelaxDamping.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisOrientRelaxLoss.name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
                        vval = (vval | 8) - 8;
                        if (val.toBool())
                            vval |= 8;
                        joint->setIntProperty(propJoint_vortexFrictionEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
                        vval = (vval | 8) - 8;
                        if (val.toBool())
                            vval |= 8;
                        joint->setIntProperty(propJoint_vortexFrictionProportionalBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisOrientFrictionCoeff.name, val.toDouble());
                    if (annJson.getValue(sub2, "maxTorque", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisOrientFrictionMaxTorque.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexXAxisOrientFrictionLoss.name, val.toDouble());
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
                        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
                        vval = (vval | 16) - 16;
                        if (val.toBool())
                            vval |= 16;
                        joint->setIntProperty(propJoint_vortexRelaxationEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisOrientRelaxStiffness.name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisOrientRelaxDamping.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisOrientRelaxLoss.name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
                        vval = (vval | 16) - 16;
                        if (val.toBool())
                            vval |= 16;
                        joint->setIntProperty(propJoint_vortexFrictionEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
                        vval = (vval | 16) - 16;
                        if (val.toBool())
                            vval |= 16;
                        joint->setIntProperty(propJoint_vortexFrictionProportionalBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisOrientFrictionCoeff.name, val.toDouble());
                    if (annJson.getValue(sub2, "maxTorque", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisOrientFrictionMaxTorque.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexYAxisOrientFrictionLoss.name, val.toDouble());
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
                        vval = joint->getIntPropertyValue(propJoint_vortexRelaxationEnabledBits.name);
                        vval = (vval | 32) - 32;
                        if (val.toBool())
                            vval |= 32;
                        joint->setIntProperty(propJoint_vortexRelaxationEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "stiffness", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisOrientRelaxStiffness.name, val.toDouble());
                    if (annJson.getValue(sub2, "damping", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisOrientRelaxDamping.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisOrientRelaxLoss.name, val.toDouble());
                }
                if (annJson.getValue(sub, "friction", QJsonValue::Object, val, allErrors))
                {
                    QJsonObject sub2(val.toObject());
                    if (annJson.getValue(sub2, "enabled", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionEnabledBits.name);
                        vval = (vval | 32) - 32;
                        if (val.toBool())
                            vval |= 32;
                        joint->setIntProperty(propJoint_vortexFrictionEnabledBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "proportional", QJsonValue::Bool, val, allErrors))
                    {
                        vval = joint->getIntPropertyValue(propJoint_vortexFrictionProportionalBits.name);
                        vval = (vval | 32) - 32;
                        if (val.toBool())
                            vval |= 32;
                        joint->setIntProperty(propJoint_vortexFrictionProportionalBits.name, vval);
                    }
                    if (annJson.getValue(sub2, "value", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisOrientFrictionCoeff.name, val.toDouble());
                    if (annJson.getValue(sub2, "maxTorque", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisOrientFrictionMaxTorque.name, val.toDouble());
                    if (annJson.getValue(sub2, "loss", QJsonValue::Double, val, allErrors))
                        joint->setFloatProperty(propJoint_vortexZAxisOrientFrictionLoss.name, val.toDouble());
                }
            }
            double w[3];
            if (annJson.getValue(vortex, "posPid", w, 3, allErrors))
                joint->setVector3Property(propJoint_vortexPosPid.name, C3Vector(w));
        }
    }
}

void CEngineProperties::_writeShape(int engine, int shapeHandle, CAnnJson &annJson) const
{
    CShape *shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    CDynMaterialObject *mat = shape->getDynMaterial();
    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        annJson.addJson(jbullet, "stickyContact", mat->getBoolPropertyValue(propMaterial_bulletSticky.name),
                        "only Bullet V2.78");
        annJson.addJson(jbullet, "frictionOld", mat->getFloatPropertyValue(propMaterial_bulletFriction0.name),
                        "only Bullet V2.78");
        annJson.addJson(jbullet, "friction", mat->getFloatPropertyValue(propMaterial_bulletFriction.name),
                        "only Bullet V2.83 and later");
        annJson.addJson(jbullet, "restitution", mat->getFloatPropertyValue(propMaterial_bulletRestitution.name));
        annJson.addJson(jbullet, "linearDamping", mat->getFloatPropertyValue(propMaterial_bulletLinearDamping.name));
        annJson.addJson(jbullet, "angularDamping", mat->getFloatPropertyValue(propMaterial_bulletAngularDamping.name));
        QJsonObject jbulletCollMargin;
        annJson.addJson(jbulletCollMargin, "enabled",
                        mat->getBoolPropertyValue(propMaterial_bulletNonDefaultCollisionMargin.name));
        annJson.addJson(jbulletCollMargin, "value",
                        mat->getFloatPropertyValue(propMaterial_bulletNonDefaultCollisionMarginFactor.name));
        annJson.addJson(jbullet, "customCollisionMargin", jbulletCollMargin);
        QJsonObject jbulletCollMarginConvex;
        annJson.addJson(jbulletCollMarginConvex, "enabled",
                        mat->getBoolPropertyValue(propMaterial_bulletNonDefaultCollisionMarginConvex.name));
        annJson.addJson(jbulletCollMarginConvex, "value",
                        mat->getFloatPropertyValue(propMaterial_bulletNonDefaultCollisionMarginFactorConvex.name));
        annJson.addJson(jbullet, "customCollisionMarginConvex", jbulletCollMarginConvex);
        annJson.addJson(jbullet, "autoShrinkConvexMeshes",
                        mat->getBoolPropertyValue(propMaterial_bulletAutoShrinkConvex.name));
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        annJson.addJson(jode, "friction", mat->getFloatPropertyValue(propMaterial_odeFriction.name));
        annJson.addJson(jode, "softErp", mat->getFloatPropertyValue(propMaterial_odeSoftErp.name));
        annJson.addJson(jode, "softCfm", mat->getFloatPropertyValue(propMaterial_odeSoftCfm.name));
        annJson.addJson(jode, "linearDamping", mat->getFloatPropertyValue(propMaterial_odeLinearDamping.name));
        annJson.addJson(jode, "angularDamping", mat->getFloatPropertyValue(propMaterial_odeAngularDamping.name));
        annJson.addJson(jode, "maxContacts", mat->getIntPropertyValue(propMaterial_odeMaxContacts.name));
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        annJson.addJson(jnewton, "staticFriction", mat->getFloatPropertyValue(propMaterial_newtonStaticFriction.name));
        annJson.addJson(jnewton, "kineticFriction", mat->getFloatPropertyValue(propMaterial_newtonKineticFriction.name));
        annJson.addJson(jnewton, "restitution", mat->getFloatPropertyValue(propMaterial_newtonRestitution.name));
        annJson.addJson(jnewton, "linearDrag", mat->getFloatPropertyValue(propMaterial_newtonLinearDrag.name));
        annJson.addJson(jnewton, "angularDrag", mat->getFloatPropertyValue(propMaterial_newtonAngularDrag.name));
        annJson.addJson(jnewton, "fastMoving", mat->getBoolPropertyValue(propMaterial_newtonFastMoving.name));
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        C3Vector vec3;
        mat->getVector3Property(propMaterial_mujocoFriction.name, &vec3);
        annJson.addJson(jmujoco, "friction", vec3.data, 3);
        double vec[2];
        mat->getVector2Property(propMaterial_mujocoSolref.name, vec);
        annJson.addJson(jmujoco, "solref", vec, 2);
        std::vector<double> v;
        mat->getVectorProperty(propMaterial_mujocoSolimp.name, v);
        annJson.addJson(jmujoco, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "condim", mat->getIntPropertyValue(propMaterial_mujocoCondim.name));
        annJson.addJson(jmujoco, "solmix", mat->getFloatPropertyValue(propMaterial_mujocoSolmix.name));
        annJson.addJson(jmujoco, "margin", mat->getFloatPropertyValue(propMaterial_mujocoMargin.name));
        annJson.addJson(jmujoco, "priority", mat->getIntPropertyValue(propMaterial_mujocoPriority.name));
        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        annJson.addJson(jvortex, "restitution", mat->getFloatPropertyValue(propMaterial_vortexRestitution.name));
        annJson.addJson(jvortex, "restitutionThreshold",
                        mat->getFloatPropertyValue(propMaterial_vortexRestitutionThreshold.name));
        annJson.addJson(jvortex, "compliance", mat->getFloatPropertyValue(propMaterial_vortexCompliance.name));
        annJson.addJson(jvortex, "damping", mat->getFloatPropertyValue(propMaterial_vortexDamping.name));
        annJson.addJson(jvortex, "adhesiveForce", mat->getFloatPropertyValue(propMaterial_vortexAdhesiveForce.name));
        annJson.addJson(jvortex, "linearVelDamping",
                        mat->getFloatPropertyValue(propMaterial_vortexLinearVelocityDamping.name));
        annJson.addJson(jvortex, "angularVelDamping",
                        mat->getFloatPropertyValue(propMaterial_vortexAngularVelocityDamping.name));
        QJsonObject jvortexAutoAngularDamping;
        annJson.addJson(jvortexAutoAngularDamping, "enabled",
                        mat->getBoolPropertyValue(propMaterial_vortexAutoAngularDamping.name));
        annJson.addJson(jvortexAutoAngularDamping, "tensionRatio",
                        mat->getFloatPropertyValue(propMaterial_vortexAutoAngularDampingTensionRatio.name));
        annJson.addJson(jvortex, "autoAngularDamping", jvortexAutoAngularDamping);
        annJson.addJson(jvortex, "skinThickness", mat->getFloatPropertyValue(propMaterial_vortexSkinThickness.name));
        annJson.addJson(jvortex, "autoSlip", mat->getBoolPropertyValue(propMaterial_vortexAutoSlip.name));
        annJson.addJson(jvortex, "fastMoving", mat->getBoolPropertyValue(propMaterial_vortexFastMoving.name));
        annJson.addJson(jvortex, "primitiveAsConvex",
                        mat->getBoolPropertyValue(propMaterial_vortexPrimitiveShapesAsConvex.name));
        annJson.addJson(jvortex, "convexAsRandom",
                        mat->getBoolPropertyValue(propMaterial_vortexConvexShapesAsRandom.name));
        annJson.addJson(jvortex, "randomAsTerrain",
                        mat->getBoolPropertyValue(propMaterial_vortexRandomShapesAsTerrain.name));
        QJsonObject jvortexautoSleep;
        annJson.addJson(jvortexautoSleep, "linearSpeed",
                        mat->getFloatPropertyValue(propMaterial_vortexAutoSleepLinearSpeedThreshold.name));
        annJson.addJson(jvortexautoSleep, "linearAccel",
                        mat->getFloatPropertyValue(propMaterial_vortexAutoSleepLinearAccelerationThreshold.name));
        annJson.addJson(jvortexautoSleep, "angularSpeed",
                        mat->getFloatPropertyValue(propMaterial_vortexAutoSleepAngularSpeedThreshold.name));
        annJson.addJson(jvortexautoSleep, "angularAccel",
                        mat->getFloatPropertyValue(propMaterial_vortexAutoSleepAngularAccelerationThreshold.name));
        annJson.addJson(jvortexautoSleep, "steps",
                        mat->getIntPropertyValue(propMaterial_vortexAutoSleepStepLiveThreshold.name));
        annJson.addJson(jvortex, "autoSleepThreshold", jvortexautoSleep);
        std::string fricModelInfo("0=box, 1=scaled box, 2=prop low, 3=prop high, 4=scaled box fast, 5=neutral, 6=none");
        QJsonObject jvortexlinPrimAxis;
        C3Vector v;
        mat->getVector3Property(propMaterial_vortexPrimaryAxisVector.name, &v);
        annJson.addJson(jvortexlinPrimAxis, "value", v.data, 3);
        annJson.addJson(jvortexlinPrimAxis, "frictionModel",
                        mat->getIntPropertyValue(propMaterial_vortexPrimaryLinearAxisFrictionModel.name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexlinPrimAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryLinearAxisFriction.name));
        annJson.addJson(jvortexlinPrimAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale.name));
        annJson.addJson(jvortexlinPrimAxis, "slip",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryLinearAxisSlip.name));
        annJson.addJson(jvortexlinPrimAxis, "slide",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryLinearAxisSlide.name));
        annJson.addJson(jvortex, "linearPrimaryAxis", jvortexlinPrimAxis);
        QJsonObject jvortexlinSecAxis;
        annJson.addJson(jvortexlinSecAxis, "followPrimaryAxis",
                        mat->getBoolPropertyValue(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name));
        annJson.addJson(jvortexlinSecAxis, "frictionModel",
                        mat->getIntPropertyValue(propMaterial_vortexSecondaryLinearAxisFrictionModel.name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexlinSecAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryLinearAxisFriction.name));
        annJson.addJson(jvortexlinSecAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale.name));
        annJson.addJson(jvortexlinSecAxis, "slip",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryLinearAxisSlip.name));
        annJson.addJson(jvortexlinSecAxis, "slide",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryLinearAxisSlide.name));
        annJson.addJson(jvortex, "linearSecondaryAxis", jvortexlinSecAxis);
        QJsonObject jvortexangPrimAxis;
        annJson.addJson(jvortexangPrimAxis, "frictionModel",
                        mat->getIntPropertyValue(propMaterial_vortexPrimaryAngularAxisFrictionModel.name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexangPrimAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryAngularAxisFriction.name));
        annJson.addJson(jvortexangPrimAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale.name));
        annJson.addJson(jvortexangPrimAxis, "slip",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryAngularAxisSlip.name));
        annJson.addJson(jvortexangPrimAxis, "slide",
                        mat->getFloatPropertyValue(propMaterial_vortexPrimaryAngularAxisSlide.name));
        annJson.addJson(jvortex, "angularPrimaryAxis", jvortexangPrimAxis);
        QJsonObject jvortexangSecAxis;
        annJson.addJson(jvortexangSecAxis, "followPrimaryAxis",
                        mat->getBoolPropertyValue(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name));
        annJson.addJson(jvortexangSecAxis, "frictionModel",
                        mat->getIntPropertyValue(propMaterial_vortexSecondaryAngularAxisFrictionModel.name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexangSecAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryAngularAxisFriction.name));
        annJson.addJson(jvortexangSecAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale.name));
        annJson.addJson(jvortexangSecAxis, "slip",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryAngularAxisSlip.name));
        annJson.addJson(jvortexangSecAxis, "slide",
                        mat->getFloatPropertyValue(propMaterial_vortexSecondaryAngularAxisSlide.name));
        annJson.addJson(jvortex, "angularSecondaryAxis", jvortexangSecAxis);
        QJsonObject jvortexangNormAxis;
        annJson.addJson(jvortexangNormAxis, "followPrimaryAxis",
                        mat->getBoolPropertyValue(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name));
        annJson.addJson(jvortexangNormAxis, "frictionModel",
                        mat->getIntPropertyValue(propMaterial_vortexNormalAngularAxisFrictionModel.name),
                        fricModelInfo.c_str());
        annJson.addJson(jvortexangNormAxis, "frictionCoeff",
                        mat->getFloatPropertyValue(propMaterial_vortexNormalAngularAxisFriction.name));
        annJson.addJson(jvortexangNormAxis, "staticFrictionScale",
                        mat->getFloatPropertyValue(propMaterial_vortexNormalAngularAxisStaticFrictionScale.name));
        annJson.addJson(jvortexangNormAxis, "slip",
                        mat->getFloatPropertyValue(propMaterial_vortexNormalAngularAxisSlip.name));
        annJson.addJson(jvortexangNormAxis, "slide",
                        mat->getFloatPropertyValue(propMaterial_vortexNormalAngularAxisSlide.name));
        annJson.addJson(jvortex, "angularNormalAxis", jvortexangNormAxis);
        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readShape(int engine, int shapeHandle, CAnnJson &annJson, std::string *allErrors) const
{
    CShape *shape = App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    CDynMaterialObject *mat = shape->getDynMaterial();
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet, "stickyContact", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_bulletSticky.name, val.toBool());
            if (annJson.getValue(bullet, "frictionOld", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_bulletFriction0.name, val.toDouble());
            if (annJson.getValue(bullet, "friction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_bulletFriction.name, val.toDouble());
            if (annJson.getValue(bullet, "restitution", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_bulletRestitution.name, val.toDouble());
            if (annJson.getValue(bullet, "linearDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_bulletLinearDamping.name, val.toDouble());
            if (annJson.getValue(bullet, "angularDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_bulletAngularDamping.name, val.toDouble());
            if (annJson.getValue(bullet, "customCollisionMargin", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(propMaterial_bulletNonDefaultCollisionMargin.name, val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_bulletNonDefaultCollisionMarginFactor.name, val.toDouble());
            }
            if (annJson.getValue(bullet, "customCollisionMarginConvex", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(propMaterial_bulletNonDefaultCollisionMarginConvex.name, val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_bulletNonDefaultCollisionMarginFactorConvex.name, val.toDouble());
            }
            if (annJson.getValue(bullet, "autoShrinkConvexMeshes", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_bulletAutoShrinkConvex.name, val.toBool());
        }
    }

    if (engine == sim_physics_ode)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "ode", QJsonValue::Object, val, allErrors))
        {
            QJsonObject ode(val.toObject());
            if (annJson.getValue(ode, "friction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_odeFriction.name, val.toDouble());
            if (annJson.getValue(ode, "softErp", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_odeSoftErp.name, val.toDouble());
            if (annJson.getValue(ode, "softCfm", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_odeSoftCfm.name, val.toDouble());
            if (annJson.getValue(ode, "linearDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_odeLinearDamping.name, val.toDouble());
            if (annJson.getValue(ode, "angularDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_odeAngularDamping.name, val.toDouble());
            if (annJson.getValue(ode, "maxContacts", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(propMaterial_odeMaxContacts.name, val.toInt());
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
            QJsonObject newton(val.toObject());
            if (annJson.getValue(newton, "staticFriction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_newtonStaticFriction.name, val.toDouble());
            if (annJson.getValue(newton, "kineticFriction", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_newtonKineticFriction.name, val.toDouble());
            if (annJson.getValue(newton, "restitution", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_newtonRestitution.name, val.toDouble());
            if (annJson.getValue(newton, "linearDrag", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_newtonLinearDrag.name, val.toDouble());
            if (annJson.getValue(newton, "angularDrag", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_newtonAngularDrag.name, val.toDouble());
            if (annJson.getValue(newton, "fastMoving", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_newtonFastMoving.name, val.toBool());
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
                C3Vector vect3(v);
                mat->setVector3Property(propMaterial_mujocoFriction.name, &vect3);
            }
            if (annJson.getValue(mujoco, "solref", v, 2, allErrors))
                mat->setVector2Property(propMaterial_mujocoSolref.name, v);
            if (annJson.getValue(mujoco, "solimp", v, 5, allErrors))
                mat->setVectorProperty(propMaterial_mujocoSolimp.name, v, 5);
            if (annJson.getValue(mujoco, "condim", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(propMaterial_mujocoCondim.name, val.toInt());
            if (annJson.getValue(mujoco, "solmix", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_mujocoSolmix.name, val.toDouble());
            if (annJson.getValue(mujoco, "margin", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_mujocoMargin.name, val.toDouble());
            if (annJson.getValue(mujoco, "priority", QJsonValue::Double, val, allErrors))
                mat->setIntProperty(propMaterial_mujocoPriority.name, val.toInt());
        }
    }

    if (engine == sim_physics_vortex)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "vortex", QJsonValue::Object, val, allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex, "restitution", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexRestitution.name, val.toDouble());
            if (annJson.getValue(vortex, "restitutionThreshold", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexRestitutionThreshold.name, val.toDouble());
            if (annJson.getValue(vortex, "compliance", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexCompliance.name, val.toDouble());
            if (annJson.getValue(vortex, "damping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexDamping.name, val.toDouble());
            if (annJson.getValue(vortex, "adhesiveForce", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexAdhesiveForce.name, val.toDouble());
            if (annJson.getValue(vortex, "linearVelDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexLinearVelocityDamping.name, val.toDouble());
            if (annJson.getValue(vortex, "angularVelDamping", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexAngularVelocityDamping.name, val.toDouble());
            if (annJson.getValue(vortex, "autoAngularDamping", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "enabled", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(propMaterial_vortexAutoAngularDamping.name, val.toBool());
                if (annJson.getValue(sub, "tensionRatio", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexAutoAngularDampingTensionRatio.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "skinThickness", QJsonValue::Double, val, allErrors))
                mat->setFloatProperty(propMaterial_vortexSkinThickness.name, val.toDouble());
            if (annJson.getValue(vortex, "autoSlip", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_vortexAutoSlip.name, val.toBool());
            if (annJson.getValue(vortex, "fastMoving", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_vortexFastMoving.name, val.toBool());
            if (annJson.getValue(vortex, "primitiveAsConvex", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_vortexPrimitiveShapesAsConvex.name, val.toBool());
            if (annJson.getValue(vortex, "convexAsRandom", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_vortexConvexShapesAsRandom.name, val.toBool());
            if (annJson.getValue(vortex, "randomAsTerrain", QJsonValue::Bool, val, allErrors))
                mat->setBoolProperty(propMaterial_vortexRandomShapesAsTerrain.name, val.toBool());
            if (annJson.getValue(vortex, "autoSleepThreshold", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "linearSpeed", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexAutoSleepLinearSpeedThreshold.name, val.toDouble());
                if (annJson.getValue(sub, "linearAccel", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexAutoSleepLinearAccelerationThreshold.name, val.toDouble());
                if (annJson.getValue(sub, "angularSpeed", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexAutoSleepAngularSpeedThreshold.name, val.toDouble());
                if (annJson.getValue(sub, "angularAccel", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexAutoSleepAngularAccelerationThreshold.name, val.toDouble());
                if (annJson.getValue(sub, "steps", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(propMaterial_vortexAutoSleepStepLiveThreshold.name, val.toInt());
            }
            if (annJson.getValue(vortex, "linearPrimaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                double v[3];
                if (annJson.getValue(sub, "value", v, 3, allErrors))
                {
                    C3Vector v3(v);
                    mat->setVector3Property(propMaterial_vortexPrimaryAxisVector.name, &v3);
                }
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(propMaterial_vortexPrimaryLinearAxisFrictionModel.name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryLinearAxisFriction.name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryLinearAxisStaticFrictionScale.name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryLinearAxisSlip.name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryLinearAxisSlide.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "linearSecondaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "followPrimaryAxis", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(propMaterial_vortexSecondaryLinearAxisSameAsPrimaryLinearAxis.name, val.toInt());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(propMaterial_vortexSecondaryLinearAxisFrictionModel.name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryLinearAxisFriction.name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryLinearAxisStaticFrictionScale.name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryLinearAxisSlip.name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryLinearAxisSlide.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "angularPrimaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(propMaterial_vortexPrimaryAngularAxisFrictionModel.name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryAngularAxisFriction.name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryAngularAxisStaticFrictionScale.name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryAngularAxisSlip.name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexPrimaryAngularAxisSlide.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "angularSecondaryAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "followPrimaryAxis", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(propMaterial_vortexSecondaryAngularAxisSameAsPrimaryAngularAxis.name, val.toInt());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(propMaterial_vortexSecondaryAngularAxisFrictionModel.name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryAngularAxisFriction.name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryAngularAxisStaticFrictionScale.name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryAngularAxisSlip.name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexSecondaryAngularAxisSlide.name, val.toDouble());
            }
            if (annJson.getValue(vortex, "angularNormalAxis", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "followPrimaryAxis", QJsonValue::Bool, val, allErrors))
                    mat->setBoolProperty(propMaterial_vortexNormalAngularAxisSameAsPrimaryAngularAxis.name, val.toInt());
                if (annJson.getValue(sub, "frictionModel", QJsonValue::Double, val, allErrors))
                    mat->setIntProperty(propMaterial_vortexNormalAngularAxisFrictionModel.name, val.toInt());
                if (annJson.getValue(sub, "frictionCoeff", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexNormalAngularAxisFriction.name, val.toDouble());
                if (annJson.getValue(sub, "staticFrictionScale", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexNormalAngularAxisStaticFrictionScale.name, val.toDouble());
                if (annJson.getValue(sub, "slip", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexNormalAngularAxisSlip.name, val.toDouble());
                if (annJson.getValue(sub, "slide", QJsonValue::Double, val, allErrors))
                    mat->setFloatProperty(propMaterial_vortexNormalAngularAxisSlide.name, val.toDouble());
            }
        }
    }
}

void CEngineProperties::_writeGlobal(int engine, CAnnJson &annJson) const
{
    std::string comment;
    double fv;
    int iv;
    bool bv;
    if (engine == sim_physics_bullet)
    {
        QJsonObject jbullet;
        iv = _getGlobalIntParam(propDyn_bulletSolver.name, comment,
                                "0=sequential impulse, 1=NNCG, 2=dantzig, 3=projected Gauss-Seidel");
        annJson.addJson(jbullet, "solver", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_bulletIterations.name, comment);
        annJson.addJson(jbullet, "iterations", iv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_bulletComputeInertias.name, comment);
        annJson.addJson(jbullet, "computeInertias", bv, comment.c_str());
        QJsonObject jbulletInternalScaling;
        bv = _getGlobalBoolParam(propDyn_bulletInternalScalingFull.name, comment);
        annJson.addJson(jbulletInternalScaling, "full", bv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_bulletInternalScalingScaling.name, comment);
        annJson.addJson(jbulletInternalScaling, "value", fv, comment.c_str());
        annJson.addJson(jbullet, "internalScaling", jbulletInternalScaling);
        fv = _getGlobalFloatParam(propDyn_bulletCollMarginScaling.name, comment);
        annJson.addJson(jbullet, "collisionMarginScaling", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "bullet", jbullet);
    }

    if (engine == sim_physics_ode)
    {
        QJsonObject jode;
        QJsonObject jodeQuickstep;
        bv = _getGlobalBoolParam(propDyn_odeQuickStepEnabled.name, comment);
        annJson.addJson(jodeQuickstep, "enabled", bv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_odeQuickStepIterations.name, comment);
        annJson.addJson(jodeQuickstep, "iterations", iv, comment.c_str());
        annJson.addJson(jode, "quickStep", jodeQuickstep);
        bv = _getGlobalBoolParam(propDyn_odeComputeInertias.name, comment);
        annJson.addJson(jode, "computeInertias", bv, comment.c_str());
        QJsonObject jodeInternalScaling;
        bv = _getGlobalBoolParam(propDyn_odeInternalScalingFull.name, comment);
        annJson.addJson(jodeInternalScaling, "full", bv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_odeInternalScalingScaling.name, comment);
        annJson.addJson(jodeInternalScaling, "value", fv, comment.c_str());
        annJson.addJson(jode, "internalScaling", jodeInternalScaling);
        fv = _getGlobalFloatParam(propDyn_odeGlobalErp.name, comment);
        annJson.addJson(jode, "globalErp", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_odeGlobalCfm.name, comment);
        annJson.addJson(jode, "globalCfm", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "ode", jode);
    }

    if (engine == sim_physics_newton)
    {
        QJsonObject jnewton;
        iv = _getGlobalIntParam(propDyn_newtonIterations.name, comment);
        annJson.addJson(jnewton, "iterations", iv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_newtonComputeInertias.name, comment);
        annJson.addJson(jnewton, "computeInertias", bv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_newtonMultithreading.name, comment);
        annJson.addJson(jnewton, "multithreading", bv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_newtonExactSolver.name, comment);
        annJson.addJson(jnewton, "exactSolver", bv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_newtonHighJointAccuracy.name, comment);
        annJson.addJson(jnewton, "highJointAccuracy", bv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_newtonContactMergeTolerance.name, comment);
        annJson.addJson(jnewton, "contactMergeTolerance", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "newton", jnewton);
    }

    if (engine == sim_physics_mujoco)
    {
        QJsonObject jmujoco;
        iv = _getGlobalIntParam(propDyn_mujocoIntegrator.name, comment, "0=euler, 1=rk4, 2=implicit");
        annJson.addJson(jmujoco, "integrator", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoSolver.name, comment, "0=pgs, 1=cg, 2=newton");
        annJson.addJson(jmujoco, "solver", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoIterations.name, comment);
        annJson.addJson(jmujoco, "integrations", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoRebuildTrigger.name, comment,
                                "bit coded: bit0=object added, bit1=object removed, bit2=hierarchy changed, "
                                "bit3=dynamic refresh flag found,  bit4=xml injections, bit5=CoppeliaSim particles");
        annJson.addJson(jmujoco, "rebuildTrigger", iv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_mujocoComputeInertias.name, comment);
        annJson.addJson(jmujoco, "computeInertias", bv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoNjMax.name, comment);
        annJson.addJson(jmujoco, "njmax", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoNconMax.name, comment);
        annJson.addJson(jmujoco, "nconmax", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoNstack.name, comment);
        annJson.addJson(jmujoco, "nstack", iv, comment.c_str());
        iv = _getGlobalIntParam(propDyn_mujocoCone.name, comment, "0=pyramidal, 1=elliptic");
        annJson.addJson(jmujoco, "cone", iv, comment.c_str());
        QJsonObject jmujocoKinematic;
        iv = _getGlobalIntParam(propDyn_mujocoKinematicBodiesOverrideFlags.name, comment,
                                "0=do not override, 1=all disabled, 2=all enabled");
        annJson.addJson(jmujocoKinematic, "overrideFlags", iv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_mujocoKinematicBodiesMass.name, comment);
        annJson.addJson(jmujocoKinematic, "mass", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_mujocoKinematicBodiesInertia.name, comment);
        annJson.addJson(jmujocoKinematic, "inertia", fv, comment.c_str());
        annJson.addJson(jmujoco, "kinematicBodies", jmujocoKinematic);
        fv = _getGlobalFloatParam(propDyn_mujocoBoundMass.name, comment);
        annJson.addJson(jmujoco, "boundMass", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_mujocoBoundInertia.name, comment);
        annJson.addJson(jmujoco, "boundInertia", fv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_mujocoBalanceInertias.name, comment);
        annJson.addJson(jmujoco, "balanceInertias", bv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_mujocoMultithreaded.name, comment);
        annJson.addJson(jmujoco, "multithreaded", bv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_mujocoMulticcd.name, comment);
        annJson.addJson(jmujoco, "multiccd", bv, comment.c_str());
        QJsonObject jmujocoContacts;
        bv = _getGlobalBoolParam(propDyn_mujocoContactParamsOverride.name, comment);
        annJson.addJson(jmujocoContacts, "override", bv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_mujocoContactParamsMargin.name, comment);
        annJson.addJson(jmujocoContacts, "margin", fv, comment.c_str());
        double w[5];
        _getGlobalFloatParams(propDyn_mujocoContactParamsSolref.name, w, comment);
        annJson.addJson(jmujocoContacts, "solref", w, 2, comment.c_str());
        _getGlobalFloatParams(propDyn_mujocoContactParamsSolimp.name, w, comment);
        annJson.addJson(jmujocoContacts, "solimp", w, 5, comment.c_str());
        annJson.addJson(jmujoco, "contactParams", jmujocoContacts);
        fv = _getGlobalFloatParam(propDyn_mujocoImpRatio.name, comment);
        annJson.addJson(jmujoco, "impratio", fv, comment.c_str());
        _getGlobalFloatParams(propDyn_mujocoWind.name, w, comment);
        annJson.addJson(jmujoco, "wind", w, 3, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_mujocoDensity.name, comment);
        annJson.addJson(jmujoco, "density", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_mujocoViscosity.name, comment);
        annJson.addJson(jmujoco, "viscosity", fv, comment.c_str());
        annJson.addJson(annJson.getMainObject()[0], "mujoco", jmujoco);
    }

    if (engine == sim_physics_vortex)
    {
        QJsonObject jvortex;
        bv = _getGlobalBoolParam(propDyn_vortexComputeInertias.name, comment);
        annJson.addJson(jvortex, "computeInertias", bv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_vortexContactTolerance.name, comment);
        annJson.addJson(jvortex, "contactTolerance", fv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_vortexAutoSleep.name, comment);
        annJson.addJson(jvortex, "autoSleep", bv, comment.c_str());
        bv = _getGlobalBoolParam(propDyn_vortexMultithreading.name, comment);
        annJson.addJson(jvortex, "multithreading", bv, comment.c_str());
        QJsonObject jvortexConstr;
        fv = _getGlobalFloatParam(propDyn_vortexConstraintsLinearCompliance.name, comment);
        annJson.addJson(jvortexConstr, "linearCompliance", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_vortexConstraintsLinearDamping.name, comment);
        annJson.addJson(jvortexConstr, "linearDamping", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_vortexConstraintsLinearKineticLoss.name, comment);
        annJson.addJson(jvortexConstr, "linearKineticLoss", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_vortexConstraintsAngularCompliance.name, comment);
        annJson.addJson(jvortexConstr, "angularCompliance", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_vortexConstraintsAngularDamping.name, comment);
        annJson.addJson(jvortexConstr, "angularDamping", fv, comment.c_str());
        fv = _getGlobalFloatParam(propDyn_vortexConstraintsAngularKineticLoss.name, comment);
        annJson.addJson(jvortexConstr, "angularKineticLoss", fv, comment.c_str());
        annJson.addJson(jvortex, "constraints", jvortexConstr);
        annJson.addJson(annJson.getMainObject()[0], "vortex", jvortex);
    }
}

void CEngineProperties::_readGlobal(int engine, CAnnJson &annJson, std::string *allErrors) const
{
    QJsonValue val;

    if (engine == sim_physics_bullet)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "bullet", QJsonValue::Object, val, allErrors))
        {
            QJsonObject bullet(val.toObject());
            if (annJson.getValue(bullet, "solver", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_bulletSolver.name,
                                                                        val.toInt());
            if (annJson.getValue(bullet, "iterations", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_bulletIterations.name,
                                                                        val.toInt());
            if (annJson.getValue(bullet, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_bulletComputeInertias.name,
                                                                         val.toBool());
            if (annJson.getValue(bullet, "internalScaling", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "full", QJsonValue::Bool, val, allErrors))
                    App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_bulletInternalScalingFull.name,
                                                                             val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_bulletInternalScalingScaling.name,
                                                                              val.toDouble());
            }
            if (annJson.getValue(bullet, "collisionMarginScaling", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_bulletCollMarginScaling.name,
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
                    App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_odeQuickStepEnabled.name, val.toBool());
                if (annJson.getValue(sub, "iterations", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setIntProperty(propDyn_odeQuickStepIterations.name, val.toInt());
            }
            if (annJson.getValue(ode, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_odeComputeInertias.name, val.toBool());
            if (annJson.getValue(ode, "internalScaling", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "full", QJsonValue::Bool, val, allErrors))
                    App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_odeInternalScalingFull.name,
                                                                             val.toBool());
                if (annJson.getValue(sub, "value", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_odeInternalScalingScaling.name,
                                                                              val.toDouble());
            }
            if (annJson.getValue(ode, "globalErp", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_odeGlobalErp.name, val.toDouble());
            if (annJson.getValue(ode, "globalCfm", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_odeGlobalCfm.name, val.toDouble());
        }
    }

    if (engine == sim_physics_newton)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "newton", QJsonValue::Object, val, allErrors))
        {
            QJsonObject newton(val.toObject());
            if (annJson.getValue(newton, "iterations", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_newtonIterations.name,
                                                                        val.toInt());
            if (annJson.getValue(newton, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_newtonComputeInertias.name,
                                                                         val.toBool());
            if (annJson.getValue(newton, "multithreading", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_newtonMultithreading.name,
                                                                         val.toBool());
            if (annJson.getValue(newton, "exactSolver", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_newtonExactSolver.name, val.toBool());
            if (annJson.getValue(newton, "highJointAccuracy", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_newtonHighJointAccuracy.name,
                                                                         val.toBool());
            if (annJson.getValue(newton, "contactMergeTolerance", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_newtonContactMergeTolerance.name,
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
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoIntegrator.name, val.toInt());
            if (annJson.getValue(mujoco, "solver", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoSolver.name, val.toInt());
            if (annJson.getValue(mujoco, "integrations", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoIterations.name, val.toInt());
            if (annJson.getValue(mujoco, "rebuildTrigger", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoRebuildTrigger.name, val.toInt());
            if (annJson.getValue(mujoco, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_mujocoComputeInertias.name,
                                                                         val.toBool());
            if (annJson.getValue(mujoco, "njmax", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoNjMax.name, val.toInt());
            if (annJson.getValue(mujoco, "nconmax", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoNconMax.name, val.toInt());
            if (annJson.getValue(mujoco, "nstack", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoNstack.name, val.toInt());
            if (annJson.getValue(mujoco, "cone", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoCone.name, val.toInt());
            if (annJson.getValue(mujoco, "kinematicBodies", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "overrideFlags", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setIntProperty(propDyn_mujocoKinematicBodiesOverrideFlags.name, val.toInt());
                if (annJson.getValue(sub, "mass", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoKinematicBodiesMass.name,
                                                                              val.toDouble());
                if (annJson.getValue(sub, "inertia", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoKinematicBodiesInertia.name,
                                                                              val.toDouble());
            }
            if (annJson.getValue(mujoco, "boundMass", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoBoundMass.name, val.toDouble());
            if (annJson.getValue(mujoco, "boundInertia", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoBoundInertia.name,
                                                                          val.toDouble());
            if (annJson.getValue(mujoco, "balanceInertias", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_mujocoBalanceInertias.name,
                                                                         val.toBool());
            if (annJson.getValue(mujoco, "multithreaded", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_mujocoMultithreaded.name, val.toBool());
            if (annJson.getValue(mujoco, "multiccd", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_mujocoMulticcd.name, val.toBool());
            if (annJson.getValue(mujoco, "contactParams", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "override", QJsonValue::Bool, val, allErrors))
                    App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_mujocoContactParamsOverride.name,
                                                                             val.toBool());
                if (annJson.getValue(sub, "margin", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoContactParamsMargin.name,
                                                                              val.toDouble());
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                    App::currentWorld->dynamicsContainer->setVector2Property(propDyn_mujocoContactParamsSolref.name, w);
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                    App::currentWorld->dynamicsContainer->setVectorProperty(propDyn_mujocoContactParamsSolimp.name, w, 5);
            }
            if (annJson.getValue(mujoco, "impratio", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoImpRatio.name, val.toDouble());
            if (annJson.getValue(mujoco, "wind", w, 3, allErrors))
            {
                C3Vector w3(w);
                App::currentWorld->dynamicsContainer->setVector3Property(propDyn_mujocoWind.name, &w3);
            }
            if (annJson.getValue(mujoco, "density", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoDensity.name, val.toDouble());
            if (annJson.getValue(mujoco, "viscosity", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_mujocoViscosity.name, val.toDouble());
        }
    }

    if (engine == sim_physics_vortex)
    {
        if (annJson.getValue(annJson.getMainObject()[0], "vortex", QJsonValue::Object, val, allErrors))
        {
            QJsonObject vortex(val.toObject());
            if (annJson.getValue(vortex, "computeInertias", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_vortexComputeInertias.name,
                                                                         val.toBool());
            if (annJson.getValue(vortex, "contactTolerance", QJsonValue::Double, val, allErrors))
                App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexContactTolerance.name,
                                                                          val.toDouble());
            if (annJson.getValue(vortex, "autoSleep", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_vortexAutoSleep.name, val.toBool());
            if (annJson.getValue(vortex, "multithreading", QJsonValue::Bool, val, allErrors))
                App::currentWorld->dynamicsContainer->setBoolProperty(propDyn_vortexMultithreading.name,
                                                                         val.toBool());
            if (annJson.getValue(vortex, "constraints", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "linearCompliance", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexConstraintsLinearCompliance.name, val.toDouble());
                if (annJson.getValue(sub, "linearDamping", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexConstraintsLinearDamping.name, val.toDouble());
                if (annJson.getValue(sub, "linearKineticLoss", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexConstraintsLinearKineticLoss.name, val.toDouble());
                if (annJson.getValue(sub, "angularCompliance", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexConstraintsAngularCompliance.name, val.toDouble());
                if (annJson.getValue(sub, "angularDamping", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexConstraintsAngularDamping.name, val.toDouble());
                if (annJson.getValue(sub, "angularKineticLoss", QJsonValue::Double, val, allErrors))
                    App::currentWorld->dynamicsContainer->setFloatProperty(propDyn_vortexConstraintsAngularKineticLoss.name, val.toDouble());
            }
        }
    }
}

double CEngineProperties::_getGlobalFloatParam(const char* item, std::string &comment,
                                               const char *additionalComment /*=nullptr*/) const
{
    double retVal = App::currentWorld->dynamicsContainer->getFloatPropertyValue(item, false);
    double def = App::currentWorld->dynamicsContainer->getFloatPropertyValue(item, true);
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

int CEngineProperties::_getGlobalIntParam(const char* item, std::string &comment,
                                          const char *additionalComment /*=nullptr*/) const
{
    int retVal = App::currentWorld->dynamicsContainer->getIntPropertyValue(item, false);
    int def = App::currentWorld->dynamicsContainer->getIntPropertyValue(item, true);
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

bool CEngineProperties::_getGlobalBoolParam(const char* item, std::string &comment,
                                            const char *additionalComment /*=nullptr*/) const
{
    bool retVal = App::currentWorld->dynamicsContainer->getBoolPropertyValue(item, false);
    bool def = App::currentWorld->dynamicsContainer->getBoolPropertyValue(item, true);
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

void CEngineProperties::_getGlobalFloatParams(const char* item, double *w, std::string &comment, const char *additionalComment /*=nullptr*/) const
{
    std::vector<double> p;
    std::vector<double> p0;
    double vec[2];
    if (App::currentWorld->dynamicsContainer->getVectorProperty(item, p, false) == 1)
        App::currentWorld->dynamicsContainer->getVectorProperty(item, p0, true);
    else if (App::currentWorld->dynamicsContainer->getVector2Property(item, vec, false) == 1)
    {
        double vec0[2];
        App::currentWorld->dynamicsContainer->getVector2Property(item, vec0, true);
        for (size_t i = 0; i < 2; i++)
        {
            p.push_back(vec[i]);
            p0.push_back(vec0[i]);
        }
    }
    else
    {

        C3Vector v;
        C3Vector v0;
        App::currentWorld->dynamicsContainer->getVector3Property(item, v, false);
        App::currentWorld->dynamicsContainer->getVector3Property(item, v0, true);
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

void CEngineProperties::_writeDummy(int engine, int dummyHandle, CAnnJson &annJson) const
{
    CDummy *dummy = App::currentWorld->sceneObjects->getDummyFromHandle(dummyHandle);

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
        annJson.addJson(jmujocoLimits, "enabled", dummy->getBoolPropertyValue(propDummy_mujocoLimitsEnabled.name));
        double vec[2];
        dummy->getVector2Property(propDummy_mujocoLimitsRange.name, vec);
        annJson.addJson(jmujocoLimits, "range", vec, 2);
        dummy->getVector2Property(propDummy_mujocoLimitsSolref.name, vec);
        annJson.addJson(jmujocoLimits, "solref", vec, 2);
        std::vector<double> v;
        dummy->getVectorProperty(propDummy_mujocoLimitsSolimp.name, v);
        annJson.addJson(jmujocoLimits, "solimp", v.data(), 5);
        annJson.addJson(jmujoco, "limits", jmujocoLimits);
        QJsonObject jmujocoSpring;
        annJson.addJson(jmujocoSpring, "stiffness", dummy->getFloatPropertyValue(propDummy_mujocoSpringStiffness.name));
        annJson.addJson(jmujocoSpring, "damping", dummy->getFloatPropertyValue(propDummy_mujocoSpringDamping.name));
        annJson.addJson(jmujocoSpring, "length", dummy->getFloatPropertyValue(propDummy_mujocoSpringLength.name));
        annJson.addJson(jmujoco, "spring", jmujocoSpring);
        annJson.addJson(jmujoco, "margin", dummy->getFloatPropertyValue(propDummy_mujocoMargin.name));
        QJsonObject jmujocoDependency;
        int h = dummy->getIntPropertyValue(propDummy_mujocoJointProxyHandle.name);
        std::string nameAndPath;
        if (h >= 0)
        {
            CJoint *obj = App::currentWorld->sceneObjects->getJointFromHandle(h);
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

void CEngineProperties::_readDummy(int engine, int dummyHandle, CAnnJson &annJson, std::string *allErrors) const
{
    CDummy *dummy = App::currentWorld->sceneObjects->getDummyFromHandle(dummyHandle);
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
                    dummy->setBoolProperty(propDummy_mujocoLimitsEnabled.name, val.toBool());
                if (annJson.getValue(sub, "range", w, 2, allErrors))
                    dummy->setVector2Property(propDummy_mujocoLimitsRange.name, w);
                if (annJson.getValue(sub, "solref", w, 2, allErrors))
                    dummy->setVector2Property(propDummy_mujocoLimitsSolref.name, w);
                if (annJson.getValue(sub, "solimp", w, 5, allErrors))
                    dummy->setVectorProperty(propDummy_mujocoLimitsSolimp.name, w, 5);
            }
            if (annJson.getValue(mujoco, "spring", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "stiffness", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(propDummy_mujocoSpringStiffness.name, val.toDouble());
                if (annJson.getValue(sub, "damping", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(propDummy_mujocoSpringDamping.name, val.toDouble());
                if (annJson.getValue(sub, "length", QJsonValue::Double, val, allErrors))
                    dummy->setFloatProperty(propDummy_mujocoSpringLength.name, val.toDouble());
            }
            if (annJson.getValue(mujoco, "margin", QJsonValue::Double, val, allErrors))
                dummy->setFloatProperty(propDummy_mujocoMargin.name, val.toDouble());
            if (annJson.getValue(mujoco, "jointProxy", QJsonValue::Object, val, allErrors))
            {
                QJsonObject sub(val.toObject());
                if (annJson.getValue(sub, "name", QJsonValue::String, val, allErrors))
                {
                    int h = -1;
                    std::string str(val.toString().toStdString());
                    if (str.size() > 0)
                    {
                        CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromPath(nullptr, str.c_str(), 0);
                        if ((obj != nullptr) && (obj->getObjectType() == sim_sceneobject_joint))
                        {
                            h = obj->getObjectHandle();
                            dummy->setIntProperty(propDummy_mujocoJointProxyHandle.name, h);
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
                        dummy->setIntProperty(propDummy_mujocoJointProxyHandle.name, h);
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
