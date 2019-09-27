
#include "propBrowser_engineProp_general.h"
#include "tt.h"
#include "gV.h"
#include "app.h"
#include "v_repStrings.h"
#include "vMessageBox.h"

bool CPropBrowserEngineGeneral::_bulletPropertiesExpanded=true;
bool CPropBrowserEngineGeneral::_odePropertiesExpanded=true;
bool CPropBrowserEngineGeneral::_vortexPropertiesExpanded=true;
bool CPropBrowserEngineGeneral::_newtonPropertiesExpanded=true;
bool CPropBrowserEngineGeneral::_vortexConstraintPropertiesExpanded=false;
int CPropBrowserEngineGeneral::_dlgSizeX=500;
int CPropBrowserEngineGeneral::_dlgSizeY=560;
int CPropBrowserEngineGeneral::_dlgPosX=800;
int CPropBrowserEngineGeneral::_dlgPosY=100;
int CPropBrowserEngineGeneral::_splitterPos=250;

CPropBrowserEngineGeneral::CPropBrowserEngineGeneral(QWidget* pParent) : QtTreePropertyBrowser(pParent)
{
    show(nullptr);
}

CPropBrowserEngineGeneral::~CPropBrowserEngineGeneral()
{
    enableNotifications(false);

    _bulletPropertiesExpanded=isExpanded(bulletGroupItem);
    _odePropertiesExpanded=isExpanded(odeGroupItem);
    _vortexPropertiesExpanded=isExpanded(vortexGroupItem);
    _newtonPropertiesExpanded=isExpanded(newtonGroupItem);
    _vortexConstraintPropertiesExpanded=isExpanded(vortexConstraintItem);

    if ((windowState()&Qt::WindowMaximized)==0)
    {
        QRect geom(geometry());
        _dlgSizeX=geom.width();
        _dlgSizeY=geom.height();
        _dlgPosX=geom.x();
        _dlgPosY=geom.y();
        _splitterPos=splitterPosition();
    }

    // following 2 needed, otherwise the prop browser will eventually crash "out of the blue" (it keeps track of things in a static map)
    unsetFactoryForManager(buttonManager); // this will output a Qt error/warning to the console. Not good.
    unsetFactoryForManager(variantManager);


    delete buttonFactory;
    delete buttonManager;
    delete variantFactory;
    delete variantManager;
}


void CPropBrowserEngineGeneral::reject()
{
    done(QDialog::Accepted);
}

void CPropBrowserEngineGeneral::accept()
{
    done(QDialog::Accepted);
}


void CPropBrowserEngineGeneral::show(QWidget* parentWindow)
{
    setPropertiesWithoutValueMarked(true); // so that group properties stand out
    setRootIsDecorated(true); // keep true, otherwise subcategories won't work correctly
    setAlternatingRowColors(true);
    setHeaderVisible(true);
    //setIndentation(60);
    //setStatusTip("bla");
    //setToolTip("Bla");
    setResizeMode(QtTreePropertyBrowser::Interactive);
    setSplitterPosition(_splitterPos);
    setWindowTitle("Physics Engines Properties - General");


    variantManager = new QtVariantPropertyManager();
    variantFactory = new QtVariantEditorFactory();
    setFactoryForManager(variantManager,variantFactory);


    buttonManager = new ButtonEditManager();
    buttonFactory  = new PushButtonEditFactory();
    setFactoryForManager(buttonManager,buttonFactory);


    QStringList configurationEnum;
    configurationEnum << "Very accurate" << "Accurate (default)" << "Fast" << "Very fast" << "Customized";
    p_configuration = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_configuration->setAttribute("enumNames", configurationEnum);
    QtBrowserItem* anItem=addProperty(p_configuration);
    setBackgroundColor(anItem,QTPROPERTYBROWSER_COLOR_GREY);


    QtProperty *bulletGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Bullet properties");
    bulletGroup->theBrightness=140;
    bulletGroupItem=addProperty(bulletGroup);
    setBackgroundColor(bulletGroupItem,QTPROPERTYBROWSER_COLOR_RED);
    setExpanded(bulletGroupItem,_bulletPropertiesExpanded);

    QtProperty *odeGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"ODE properties");
    odeGroup->theBrightness=140;
    odeGroupItem=addProperty(odeGroup);
    setBackgroundColor(odeGroupItem,QTPROPERTYBROWSER_COLOR_GREEN);
    setExpanded(odeGroupItem,_odePropertiesExpanded);

    QtProperty *vortexGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Vortex properties");
    vortexGroup->theBrightness=140;
    vortexGroupItem=addProperty(vortexGroup);
    setBackgroundColor(vortexGroupItem,QTPROPERTYBROWSER_COLOR_BLUE);
    setExpanded(vortexGroupItem,_vortexPropertiesExpanded);

    QtProperty *newtonGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Newton properties");
    newtonGroup->theBrightness=140;
    newtonGroupItem=addProperty(newtonGroup);
    setBackgroundColor(newtonGroupItem,QTPROPERTYBROWSER_COLOR_PURPLE);
    setExpanded(newtonGroupItem,_newtonPropertiesExpanded);

    // Bullet:
    p_bulletTimeStep = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletTimeStep);

    QStringList bulletSolverType;
    bulletSolverType << "sequential impulse" << "NNCG" << "dantzig" << "projected Gauss-Seidel";
    p_bulletConstraintSolvType = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_bulletConstraintSolvType->setAttribute("enumNames", bulletSolverType);
    bulletGroup->addSubProperty(p_bulletConstraintSolvType);

    p_bulletConstraintSolvIterat = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletConstraintSolvIterat);
    p_bulletInternalScaling = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletInternalScaling);
    p_bulletInternalFullScaling = variantManager->addProperty(QVariant::Bool,"");
    bulletGroup->addSubProperty(p_bulletInternalFullScaling);
    p_bulletCollMarginScaling = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletCollMarginScaling);

    // ODE:
    p_odeTimeStep = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeTimeStep);
    p_odeQuickStep = variantManager->addProperty(QVariant::Bool,"");
    odeGroup->addSubProperty(p_odeQuickStep);
    p_odeIterations = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeIterations);
    p_odeInternalScaling = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeInternalScaling);
    p_odeInternalFullScaling = variantManager->addProperty(QVariant::Bool,"");
    odeGroup->addSubProperty(p_odeInternalFullScaling);
    p_odeGlobalErp = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeGlobalErp);
    p_odeGlobalCfm = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeGlobalCfm);

    // Vortex:
    p_vortexTimeStep = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexTimeStep);
    p_vortexContactTolerance = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexContactTolerance);
    p_vortexAutoSleep = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexAutoSleep);
    p_vortexMultithreading = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexMultithreading);

    QtProperty* vortexConstraint = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Constraint properties");
    vortexGroup->addSubProperty(vortexConstraint);
    vortexConstraintItem=getSubPropertyBrowserItem(vortexGroupItem,vortexConstraint);
    setExpanded(vortexConstraintItem,_vortexConstraintPropertiesExpanded);

    p_vortexConstraintLinearCompliance = variantManager->addProperty(QVariant::String,"");
    vortexConstraint->addSubProperty(p_vortexConstraintLinearCompliance);
    p_vortexConstraintLinearDamping = variantManager->addProperty(QVariant::String,"");
    vortexConstraint->addSubProperty(p_vortexConstraintLinearDamping);
    p_vortexConstraintLinearKinLoss = variantManager->addProperty(QVariant::String,"");
    vortexConstraint->addSubProperty(p_vortexConstraintLinearKinLoss);
    p_vortexConstraintAngularCompliance = variantManager->addProperty(QVariant::String,"");
    vortexConstraint->addSubProperty(p_vortexConstraintAngularCompliance);
    p_vortexConstraintAngularDamping = variantManager->addProperty(QVariant::String,"");
    vortexConstraint->addSubProperty(p_vortexConstraintAngularDamping);
    p_vortexConstraintAngularKinLoss = variantManager->addProperty(QVariant::String,"");
    vortexConstraint->addSubProperty(p_vortexConstraintAngularKinLoss);

    // Newton:
    p_newtonTimeStep = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonTimeStep);
    p_newtonSolvingIterations = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonSolvingIterations);
    p_newtonMultithreading = variantManager->addProperty(QVariant::Bool,"");
    newtonGroup->addSubProperty(p_newtonMultithreading);
    p_newtonExactSolver = variantManager->addProperty(QVariant::Bool,"");
    newtonGroup->addSubProperty(p_newtonExactSolver);
    p_newtonHighJointAccuracy = variantManager->addProperty(QVariant::Bool,"");
    newtonGroup->addSubProperty(p_newtonHighJointAccuracy);
    p_newtonContactMergeTolerance = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonContactMergeTolerance);

    refresh();

    enableNotifications(true);

    setGeometry(_dlgPosX,_dlgPosY,_dlgSizeX,_dlgSizeY);

}

void CPropBrowserEngineGeneral::enableNotifications(bool enable)
{
    if (enable)
    {
        QObject::connect(variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),this,SLOT(catchPropertyChanges(QtProperty*,QVariant)),Qt::QueuedConnection);
        QObject::connect(buttonManager,SIGNAL(valueChanged(QtProperty*,QString)),this,SLOT(catchPropertyChangesString(QtProperty*,QString)),Qt::QueuedConnection);
    }
    else
    {
        QObject::disconnect(variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),this,SLOT(catchPropertyChanges(QtProperty*,QVariant)));
        QObject::disconnect(buttonManager,SIGNAL(valueChanged(QtProperty*,QString)),this,SLOT(catchPropertyChangesString(QtProperty*,QString)));
    }
}

void CPropBrowserEngineGeneral::refresh()
{
    int confIndex=App::ct->dynamicsContainer->getUseDynamicDefaultCalculationParameters();

    // Configuration drop box:
    p_configuration->setPropertyName("Configuration");
    p_configuration->setValue(confIndex);

    // Bullet property names:
    p_bulletTimeStep->setPropertyName(gv::formatUnitStr("Bullet time step","s").c_str());
    p_bulletConstraintSolvType->setPropertyName("Constraint solver type (not for Bullet 2.78)");
    p_bulletConstraintSolvIterat->setPropertyName("Constraint solving iterations");
    p_bulletInternalScaling->setPropertyName("Internal scaling");
    p_bulletInternalFullScaling->setPropertyName("Full scaling");
    p_bulletCollMarginScaling->setPropertyName("Collision margin scaling");

    // Bullet parameters:
    p_bulletTimeStep->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_bullet_global_stepsize,nullptr),false).c_str());
    p_bulletTimeStep->setEnabled(confIndex==4);
    p_bulletConstraintSolvType->setValue(App::ct->dynamicsContainer->getEngineIntParam(sim_bullet_global_constraintsolvertype,nullptr));
    p_bulletConstraintSolvType->setEnabled(confIndex==4);
    p_bulletConstraintSolvIterat->setValue(tt::intToString(App::ct->dynamicsContainer->getEngineIntParam(sim_bullet_global_constraintsolvingiterations,nullptr)).c_str());
    p_bulletConstraintSolvIterat->setEnabled(confIndex==4);
    p_bulletInternalScaling->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_bullet_global_internalscalingfactor,nullptr),false).c_str());
    p_bulletInternalScaling->setEnabled(confIndex==4);
    p_bulletInternalFullScaling->setValue(App::ct->dynamicsContainer->getEngineBoolParam(sim_bullet_global_fullinternalscaling,nullptr));
    p_bulletInternalFullScaling->setEnabled(confIndex==4);
    p_bulletCollMarginScaling->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_bullet_global_collisionmarginfactor,nullptr),false).c_str());
    p_bulletCollMarginScaling->setEnabled(confIndex==4);


    // ODE property names:
    p_odeTimeStep->setPropertyName(gv::formatUnitStr("ODE time step","s").c_str());
    p_odeQuickStep->setPropertyName("Use 'quickStep'");
    p_odeIterations->setPropertyName("QuickStep iterations");
    p_odeInternalScaling->setPropertyName("Internal scaling");
    p_odeInternalFullScaling->setPropertyName("Full scaling");
    p_odeGlobalErp->setPropertyName("Global ERP");
    p_odeGlobalCfm->setPropertyName("Global CFM");

    // ODE parameters:
    p_odeTimeStep->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_ode_global_stepsize,nullptr),false).c_str());
    p_odeTimeStep->setEnabled(confIndex==4);
    p_odeQuickStep->setValue(App::ct->dynamicsContainer->getEngineBoolParam(sim_ode_global_quickstep,nullptr));
    p_odeQuickStep->setEnabled(confIndex==4);
    p_odeIterations->setValue(tt::intToString(App::ct->dynamicsContainer->getEngineIntParam(sim_ode_global_constraintsolvingiterations,nullptr)).c_str());
    p_odeIterations->setEnabled((confIndex==4)&&App::ct->dynamicsContainer->getEngineBoolParam(sim_ode_global_quickstep,nullptr));
    p_odeInternalScaling->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_ode_global_internalscalingfactor,nullptr),false).c_str());
    p_odeInternalScaling->setEnabled(confIndex==4);
    p_odeInternalFullScaling->setValue(App::ct->dynamicsContainer->getEngineBoolParam(sim_ode_global_fullinternalscaling,nullptr));
    p_odeInternalFullScaling->setEnabled(confIndex==4);
    p_odeGlobalErp->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_ode_global_erp,nullptr),false).c_str());
    p_odeGlobalErp->setEnabled(confIndex==4);
    p_odeGlobalCfm->setValue(tt::floatToEInfString(App::ct->dynamicsContainer->getEngineFloatParam(sim_ode_global_cfm,nullptr),false).c_str());
    p_odeGlobalCfm->setEnabled(confIndex==4);


    // Vortex property names:
    p_vortexTimeStep->setPropertyName(gv::formatUnitStr("Vortex time step","s").c_str());
    p_vortexContactTolerance->setPropertyName(gv::formatUnitStr("Contact tolerance","m").c_str());
    p_vortexAutoSleep->setPropertyName("Auto-sleep");
    p_vortexMultithreading->setPropertyName("Multithreading");

    p_vortexConstraintLinearCompliance->setPropertyName(gv::formatUnitStr("Linear compliance","s^2/k").c_str());
    p_vortexConstraintLinearDamping->setPropertyName(gv::formatUnitStr("Linear damping","k/s").c_str());
    p_vortexConstraintLinearKinLoss->setPropertyName(gv::formatUnitStr("Linear kinetic loss","s/k").c_str());

    p_vortexConstraintAngularCompliance->setPropertyName(gv::formatUnitStr("Angular compliance","s^2/(k*m)").c_str());
    p_vortexConstraintAngularDamping->setPropertyName(gv::formatUnitStr("Angular damping","k*m^2/s").c_str());
    p_vortexConstraintAngularKinLoss->setPropertyName(gv::formatUnitStr("Angular kinetic loss","s/(k*m^2)").c_str());


    // Vortex parameters:

    float stepSize=App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_stepsize,nullptr);
    float contactTolerance=App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_contacttolerance,nullptr);
    float constraint_linear_compliance=tt::floatToUserFloat(App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_constraintlinearcompliance,nullptr),1.0f,true);
    float constraint_linear_damping=tt::floatToUserFloat(App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_constraintlineardamping,nullptr),1.0f,true);
    float constraint_linear_kineticLoss=tt::floatToUserFloat(App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_constraintlinearkineticloss,nullptr),1.0f,true);
    float constraint_angular_compliance=tt::floatToUserFloat(App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_constraintangularcompliance,nullptr),1.0f,true);
    float constraint_angular_damping=tt::floatToUserFloat(App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_constraintangulardamping,nullptr),1.0f,true);
    float constraint_angular_kineticLoss=tt::floatToUserFloat(App::ct->dynamicsContainer->getEngineFloatParam(sim_vortex_global_constraintangularkineticloss,nullptr),1.0f,true);
    bool autoSleep=App::ct->dynamicsContainer->getEngineBoolParam(sim_vortex_global_autosleep,nullptr);
    bool multiThreading=App::ct->dynamicsContainer->getEngineBoolParam(sim_vortex_global_multithreading,nullptr);

    p_vortexTimeStep->setValue(tt::floatToEInfString(stepSize,false).c_str());
    p_vortexTimeStep->setEnabled(confIndex==4);
    p_vortexContactTolerance->setValue(tt::floatToEInfString(contactTolerance,false).c_str());
    p_vortexContactTolerance->setEnabled(confIndex==4);
    p_vortexAutoSleep->setValue(autoSleep);
    p_vortexAutoSleep->setEnabled(confIndex==4);
    p_vortexMultithreading->setValue(multiThreading);
    p_vortexMultithreading->setEnabled(confIndex==4);

    p_vortexConstraintLinearCompliance->setValue(tt::floatToEInfString(constraint_linear_compliance,false).c_str());
    p_vortexConstraintLinearCompliance->setEnabled(confIndex==4);
    p_vortexConstraintLinearDamping->setValue(tt::floatToEInfString(constraint_linear_damping,false).c_str());
    p_vortexConstraintLinearDamping->setEnabled(confIndex==4);
    p_vortexConstraintLinearKinLoss->setValue(tt::floatToEInfString(constraint_linear_kineticLoss,false).c_str());
    p_vortexConstraintLinearKinLoss->setEnabled(confIndex==4);
    p_vortexConstraintAngularCompliance->setValue(tt::floatToEInfString(constraint_angular_compliance,false).c_str());
    p_vortexConstraintAngularCompliance->setEnabled(confIndex==4);
    p_vortexConstraintAngularDamping->setValue(tt::floatToEInfString(constraint_angular_damping,false).c_str());
    p_vortexConstraintAngularDamping->setEnabled(confIndex==4);
    p_vortexConstraintAngularKinLoss->setValue(tt::floatToEInfString(constraint_angular_kineticLoss,false).c_str());
    p_vortexConstraintAngularKinLoss->setEnabled(confIndex==4);


    // Newton property names:
    p_newtonTimeStep->setPropertyName(gv::formatUnitStr("Newton time step","s").c_str());
    p_newtonSolvingIterations->setPropertyName("Constraint solving iterations");
    p_newtonMultithreading->setPropertyName("Multithreading");
    p_newtonExactSolver->setPropertyName("Exact solver");
    p_newtonHighJointAccuracy->setPropertyName("High joint accuracy");
    p_newtonContactMergeTolerance->setPropertyName(gv::formatUnitStr("Contact merge tolerance","m").c_str());

    // Newton parameters:
    float newtonStepSize=App::ct->dynamicsContainer->getEngineFloatParam(sim_newton_global_stepsize,nullptr);
    float newtonContactMergeTolerance=App::ct->dynamicsContainer->getEngineFloatParam(sim_newton_global_contactmergetolerance,nullptr);
    int newtonIterations=App::ct->dynamicsContainer->getEngineIntParam(sim_newton_global_constraintsolvingiterations,nullptr);
    bool newtonMultithreading=App::ct->dynamicsContainer->getEngineBoolParam(sim_newton_global_multithreading,nullptr);
    bool newtonExactSolver=App::ct->dynamicsContainer->getEngineBoolParam(sim_newton_global_exactsolver,nullptr);
    bool newtonHighJointAccuracy=App::ct->dynamicsContainer->getEngineBoolParam(sim_newton_global_highjointaccuracy,nullptr);

    p_newtonTimeStep->setValue(tt::floatToEInfString(newtonStepSize,false).c_str());
    p_newtonTimeStep->setEnabled(confIndex==4);
    p_newtonSolvingIterations->setValue(tt::getIString(false,newtonIterations).c_str());
    p_newtonSolvingIterations->setEnabled(confIndex==4);
    p_newtonMultithreading->setValue(newtonMultithreading);
    p_newtonMultithreading->setEnabled(confIndex==4);
    p_newtonExactSolver->setValue(newtonExactSolver);
    p_newtonExactSolver->setEnabled(confIndex==4);
    p_newtonHighJointAccuracy->setValue(newtonHighJointAccuracy);
    p_newtonHighJointAccuracy->setEnabled(confIndex==4);
    p_newtonContactMergeTolerance->setValue(tt::floatToEInfString(newtonContactMergeTolerance,false).c_str());
    p_newtonContactMergeTolerance->setEnabled(confIndex==4);
}

QtBrowserItem* CPropBrowserEngineGeneral::getSubPropertyBrowserItem(const QtBrowserItem* parentBrowserItem,const QtProperty* childProperty)
{
    QList<QtBrowserItem*> children=parentBrowserItem->children();
    for (int i=0;i<int(children.size());i++)
    {
        if (children[i]->property()==childProperty)
            return(children[i]);
    }
    return(nullptr);
}


void CPropBrowserEngineGeneral::catchPropertyChanges(QtProperty *_prop, QVariant value)
{
    enableNotifications(false);
    handlePropertyChanges(_prop);
    enableNotifications(true);
}
void CPropBrowserEngineGeneral::catchPropertyChangesString(QtProperty *_prop, QString value)
{
    enableNotifications(false);
    handlePropertyChanges(_prop);
    enableNotifications(true);
}

void CPropBrowserEngineGeneral::handlePropertyChanges(QtProperty *_prop)
{
    float f;
    int a;

    // Config drop box:
    if (_prop==p_configuration)
        App::ct->dynamicsContainer->setUseDynamicDefaultCalculationParameters(((QtVariantProperty*)_prop)->value().toInt());

    // Bullet
    if (_prop==p_bulletTimeStep)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_bullet_global_stepsize,f,true);
    }
    if (_prop==p_bulletConstraintSolvType)
        App::ct->dynamicsContainer->setEngineIntParam(sim_bullet_global_constraintsolvertype,((QtVariantProperty*)_prop)->value().toInt(),false);
    if (_prop==p_bulletConstraintSolvIterat)
    {
        if (tt::stringToInt(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),a))
            App::ct->dynamicsContainer->setEngineIntParam(sim_bullet_global_constraintsolvingiterations,a,true);
    }
    if (_prop==p_bulletInternalScaling)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_bullet_global_internalscalingfactor,f,true);
    }
    if (_prop==p_bulletInternalFullScaling)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_bullet_global_fullinternalscaling,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_bulletCollMarginScaling)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_bullet_global_collisionmarginfactor,f,true);
    }


    // ODE
    if (_prop==p_odeTimeStep)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_stepsize,f,true);
    }
    if (_prop==p_odeQuickStep)
    {
        bool b=((QtVariantProperty*)_prop)->value().toBool();
        App::ct->dynamicsContainer->setEngineBoolParam(sim_ode_global_quickstep,b,true);
        if (!b)
            App::uiThread->messageBox_warning(this,strTranslate(IDS_ODE),strTranslate(IDS_WARNING_ODE_NOT_USING_QUICKSTEP),VMESSAGEBOX_OKELI);
    }
    if (_prop==p_odeIterations)
    {
        if (tt::stringToInt(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),a))
            App::ct->dynamicsContainer->setEngineIntParam(sim_ode_global_constraintsolvingiterations,a,true);
    }
    if (_prop==p_odeInternalScaling)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_internalscalingfactor,f,true);
    }
    if (_prop==p_odeInternalFullScaling)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_ode_global_fullinternalscaling,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_odeGlobalErp)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_erp,f,true);
    }
    if (_prop==p_odeGlobalCfm)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_ode_global_cfm,f,true);
    }


    // Vortex
    if (_prop==p_vortexTimeStep)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_stepsize,tt::userFloatToFloat(f,1.0f,false),true);
    }
    if (_prop==p_vortexContactTolerance)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_contacttolerance,tt::userFloatToFloat(f,1.0f,false),true);
    }
    if (_prop==p_vortexAutoSleep)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_vortex_global_autosleep,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_vortexMultithreading)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_vortex_global_multithreading,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_vortexConstraintLinearCompliance)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_constraintlinearcompliance,tt::userFloatToFloat(f,1.0f,true),true);
    }
    if (_prop==p_vortexConstraintLinearDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_constraintlineardamping,tt::userFloatToFloat(f,1.0f,true),true);
    }
    if (_prop==p_vortexConstraintLinearKinLoss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_constraintlinearkineticloss,tt::userFloatToFloat(f,1.0f,true),true);
    }
    if (_prop==p_vortexConstraintAngularCompliance)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_constraintangularcompliance,tt::userFloatToFloat(f,1.0f,true),true);
    }
    if (_prop==p_vortexConstraintAngularDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_constraintangulardamping,tt::userFloatToFloat(f,1.0f,true),true);
    }
    if (_prop==p_vortexConstraintAngularKinLoss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_vortex_global_constraintangularkineticloss,tt::userFloatToFloat(f,1.0f,true),true);
    }

    // Newton parameters:
    if (_prop==p_newtonTimeStep)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_newton_global_stepsize,f,true);
    }
    if (_prop==p_newtonSolvingIterations)
    {
        if (tt::stringToInt(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),a))
            App::ct->dynamicsContainer->setEngineIntParam(sim_newton_global_constraintsolvingiterations,a,true);
    }
    if (_prop==p_newtonMultithreading)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_newton_global_multithreading,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_newtonExactSolver)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_newton_global_exactsolver,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_newtonHighJointAccuracy)
        App::ct->dynamicsContainer->setEngineBoolParam(sim_newton_global_highjointaccuracy,((QtVariantProperty*)_prop)->value().toBool(),true);
    if (_prop==p_newtonContactMergeTolerance)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            App::ct->dynamicsContainer->setEngineFloatParam(sim_newton_global_contactmergetolerance,f,true);
    }

    refresh();
}
