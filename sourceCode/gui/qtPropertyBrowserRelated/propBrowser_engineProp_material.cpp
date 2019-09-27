#include "propBrowser_engineProp_material.h"

#include "tt.h"
#include "gV.h"
#include "dynMaterialObject.h"
#include "app.h"

bool CPropBrowserEngineMaterial::_bulletPropertiesExpanded=true;
bool CPropBrowserEngineMaterial::_odePropertiesExpanded=true;
bool CPropBrowserEngineMaterial::_vortexPropertiesExpanded=true;
bool CPropBrowserEngineMaterial::_newtonPropertiesExpanded=true;
bool CPropBrowserEngineMaterial::_vortexAutosleepPropertiesExpanded=false;
bool CPropBrowserEngineMaterial::_vortexPrimLinAxisPropertiesExpanded=false;
bool CPropBrowserEngineMaterial::_vortexSecLinAxisPropertiesExpanded=false;
bool CPropBrowserEngineMaterial::_vortexPrimAngAxisPropertiesExpanded=false;
bool CPropBrowserEngineMaterial::_vortexSecAngAxisPropertiesExpanded=false;
bool CPropBrowserEngineMaterial::_vortexNormAngAxisPropertiesExpanded=false;
bool CPropBrowserEngineMaterial::_vortexPrimLinAxisVectorPropertiesExpanded=false;
int CPropBrowserEngineMaterial::_dlgSizeX=500;
int CPropBrowserEngineMaterial::_dlgSizeY=920;
int CPropBrowserEngineMaterial::_dlgPosX=800;
int CPropBrowserEngineMaterial::_dlgPosY=50;
int CPropBrowserEngineMaterial::_splitterPos=250;

CPropBrowserEngineMaterial::CPropBrowserEngineMaterial(QWidget* pParent) : QtTreePropertyBrowser(pParent)
{
    show(nullptr);
}

CPropBrowserEngineMaterial::~CPropBrowserEngineMaterial()
{
    enableNotifications(false);

    _bulletPropertiesExpanded=isExpanded(bulletGroupItem);
    _odePropertiesExpanded=isExpanded(odeGroupItem);
    _vortexPropertiesExpanded=isExpanded(vortexGroupItem);
    _newtonPropertiesExpanded=isExpanded(newtonGroupItem);
    _vortexAutosleepPropertiesExpanded=isExpanded(autoSleepItem);
    _vortexPrimLinAxisPropertiesExpanded=isExpanded(linAxisPrimItem);
    _vortexSecLinAxisPropertiesExpanded=isExpanded(linAxisSecItem);
    _vortexPrimAngAxisPropertiesExpanded=isExpanded(angAxisPrimItem);
    _vortexSecAngAxisPropertiesExpanded=isExpanded(angAxisSecItem);
    _vortexNormAngAxisPropertiesExpanded=isExpanded(angAxisNormItem);
    _vortexPrimLinAxisVectorPropertiesExpanded=isExpanded(linAxisPrimVectorItem);

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


void CPropBrowserEngineMaterial::reject()
{
    done(QDialog::Accepted);
}

void CPropBrowserEngineMaterial::accept()
{
    done(QDialog::Accepted);
}

void CPropBrowserEngineMaterial::show(QWidget* parentWindow)
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
    setWindowTitle("Physics Engines Properties - Material");


    variantManager = new QtVariantPropertyManager();
    variantFactory = new QtVariantEditorFactory();
    setFactoryForManager(variantManager,variantFactory);


    buttonManager = new ButtonEditManager();
    buttonFactory  = new PushButtonEditFactory();
    setFactoryForManager(buttonManager,buttonFactory);

    QStringList defMatTypeEnum;
    defMatTypeEnum << "None" << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_default).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_highfriction).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_lowfriction).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_nofriction).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_reststackgrasp).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_foot).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_wheel).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_gripper).c_str() << CDynMaterialObject::getDefaultMaterialName(sim_dynmat_floor).c_str();
    p_copyFromPredefined = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"Apply predefined settings:");
    p_copyFromPredefined->setAttribute("enumNames", defMatTypeEnum);
    QtBrowserItem* anItem=addProperty(p_copyFromPredefined);
    setBackgroundColor(anItem,QTPROPERTYBROWSER_COLOR_GREY);

//    p_materialName = variantManager->addProperty(QVariant::String,"Material name");
//    anItem=addProperty(p_materialName);
//    setBackgroundColor(anItem,QTPROPERTYBROWSER_COLOR_GREY);

    // Bullet:
    QtProperty *bulletGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Bullet properties");
    bulletGroup->theBrightness=140;
    bulletGroupItem=addProperty(bulletGroup);
    setBackgroundColor(bulletGroupItem,QTPROPERTYBROWSER_COLOR_RED);
    setExpanded(bulletGroupItem,_bulletPropertiesExpanded);

    // ODE::
    QtProperty *odeGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"ODE properties");
    odeGroup->theBrightness=140;
    odeGroupItem=addProperty(odeGroup);
    setBackgroundColor(odeGroupItem,QTPROPERTYBROWSER_COLOR_GREEN);
    setExpanded(odeGroupItem,_odePropertiesExpanded);

    // Vortex:
    QtProperty *vortexGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Vortex properties");
    vortexGroup->theBrightness=140;
    vortexGroupItem=addProperty(vortexGroup);
    setBackgroundColor(vortexGroupItem,QTPROPERTYBROWSER_COLOR_BLUE);
    setExpanded(vortexGroupItem,_vortexPropertiesExpanded);

    // Newton:
    QtProperty *newtonGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Newton properties");
    newtonGroup->theBrightness=140;
    newtonGroupItem=addProperty(newtonGroup);
    setBackgroundColor(newtonGroupItem,QTPROPERTYBROWSER_COLOR_PURPLE);
    setExpanded(newtonGroupItem,_newtonPropertiesExpanded);

    // Bullet:
    p_bulletFrictionOld = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletFrictionOld);
    p_bulletFriction = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletFriction);
    p_bulletRestitution = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletRestitution);
    p_bulletLinDamping = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletLinDamping);
    p_bulletAngDamping = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletAngDamping);
    p_bulletStickyContact = variantManager->addProperty(QVariant::Bool,"");
    bulletGroup->addSubProperty(p_bulletStickyContact);
    p_bulletAutoShrink = variantManager->addProperty(QVariant::Bool,"");
    bulletGroup->addSubProperty(p_bulletAutoShrink);
    p_bulletCustomMarginScaling = variantManager->addProperty(QVariant::Bool,"");
    bulletGroup->addSubProperty(p_bulletCustomMarginScaling);
    p_bulletMarginScalingFactor = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletMarginScalingFactor);

    // ODE:
    p_odeFriction = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeFriction);
    p_odeMaxContacts = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeMaxContacts);
    p_odeSoftERP = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeSoftERP);
    p_odeSoftCFM = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeSoftCFM);
    p_odeLinDamping = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeLinDamping);
    p_odeAngDamping = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeAngDamping);

    // Vortex:
    p_vortexRestitution = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexRestitution);
    p_vortexRestitutionThreshold = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexRestitutionThreshold);
    p_vortexCompliance = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexCompliance);
    p_vortexDamping = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexDamping);
    p_vortexAdhesiveForce = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexAdhesiveForce);
    p_vortexLinVelocityDamping = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexLinVelocityDamping);
    p_vortexAngVelocityDamping = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexAngVelocityDamping);
    p_vortexAutoAngularDampingEnabled = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexAutoAngularDampingEnabled);
    p_vortexAutoAngularDampingTensionRatio = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexAutoAngularDampingTensionRatio);
    p_vortexSkinThickness = variantManager->addProperty(QVariant::String,"");
    vortexGroup->addSubProperty(p_vortexSkinThickness);
    p_vortexAutoSlip = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexAutoSlip);
    p_vortexFastMoving = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexFastMoving);
    p_vortexPureAsConvex = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexPureAsConvex);
    p_vortexConvexAsRandom = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexConvexAsRandom);
    p_vortexRandomAsTerrain = variantManager->addProperty(QVariant::Bool,"");
    vortexGroup->addSubProperty(p_vortexRandomAsTerrain);

    QtProperty* autoSleep = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Auto-sleep");
    vortexGroup->addSubProperty(autoSleep);
    autoSleepItem=getSubPropertyBrowserItem(vortexGroupItem,autoSleep);
    setExpanded(autoSleepItem,_vortexAutosleepPropertiesExpanded);


    p_autoSleepLinSpeedThreshold = variantManager->addProperty(QVariant::String,"");
    autoSleep->addSubProperty(p_autoSleepLinSpeedThreshold);
    p_autoSleepLinAccelThreshold = variantManager->addProperty(QVariant::String,"");
    autoSleep->addSubProperty(p_autoSleepLinAccelThreshold);
    p_autoSleepAngSpeedThreshold = variantManager->addProperty(QVariant::String,"");
    autoSleep->addSubProperty(p_autoSleepAngSpeedThreshold);
    p_autoSleepAngAccelThreshold = variantManager->addProperty(QVariant::String,"");
    autoSleep->addSubProperty(p_autoSleepAngAccelThreshold);
    p_autoSleepStepsThreshold = variantManager->addProperty(QVariant::String,"");
    autoSleep->addSubProperty(p_autoSleepStepsThreshold);


    linAxisPrim = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(linAxisPrim);
    linAxisPrimItem=getSubPropertyBrowserItem(vortexGroupItem,linAxisPrim);
    setExpanded(linAxisPrimItem,_vortexPrimLinAxisPropertiesExpanded);

    linAxisSec = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(linAxisSec);
    linAxisSecItem=getSubPropertyBrowserItem(vortexGroupItem,linAxisSec);
    setExpanded(linAxisSecItem,_vortexSecLinAxisPropertiesExpanded);

    angAxisPrim = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(angAxisPrim);
    angAxisPrimItem=getSubPropertyBrowserItem(vortexGroupItem,angAxisPrim);
    setExpanded(angAxisPrimItem,_vortexPrimAngAxisPropertiesExpanded);

    angAxisSec = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(angAxisSec);
    angAxisSecItem=getSubPropertyBrowserItem(vortexGroupItem,angAxisSec);
    setExpanded(angAxisSecItem,_vortexSecAngAxisPropertiesExpanded);

    angAxisNorm = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(angAxisNorm);
    angAxisNormItem=getSubPropertyBrowserItem(vortexGroupItem,angAxisNorm);
    setExpanded(angAxisNormItem,_vortexNormAngAxisPropertiesExpanded);

    linAxisPrimVector = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    linAxisPrim->addSubProperty(linAxisPrimVector);
    linAxisPrimVectorItem=getSubPropertyBrowserItem(linAxisPrimItem,linAxisPrimVector);
    setExpanded(linAxisPrimVectorItem,_vortexPrimLinAxisVectorPropertiesExpanded);

    p_primAxisVectorX = variantManager->addProperty(QVariant::String,"");
    linAxisPrimVector->addSubProperty(p_primAxisVectorX);
    p_primAxisVectorY = variantManager->addProperty(QVariant::String,"");
    linAxisPrimVector->addSubProperty(p_primAxisVectorY);
    p_primAxisVectorZ = variantManager->addProperty(QVariant::String,"");
    linAxisPrimVector->addSubProperty(p_primAxisVectorZ);

    QStringList frictionModelEnum;
    frictionModelEnum << "Box" << "Scaled box" << "Box proportional low" << "Box proportional high" << "Scaled box fast" << "Neutral" << "None";
    p_vortexFrictionModelLinPrim = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_vortexFrictionModelLinPrim->setAttribute("enumNames", frictionModelEnum);
    linAxisPrim->addSubProperty(p_vortexFrictionModelLinPrim);
    p_vortexFrictionCoeffLinPrim = variantManager->addProperty(QVariant::String,"");
    linAxisPrim->addSubProperty(p_vortexFrictionCoeffLinPrim);
    p_vortexStaticFrictionScaleLinPrim = variantManager->addProperty(QVariant::String,"");
    linAxisPrim->addSubProperty(p_vortexStaticFrictionScaleLinPrim);
    p_vortexSlipLinPrim = variantManager->addProperty(QVariant::String,"");
    linAxisPrim->addSubProperty(p_vortexSlipLinPrim);
    p_vortexSlideLinPrim = variantManager->addProperty(QVariant::String,"");
    linAxisPrim->addSubProperty(p_vortexSlideLinPrim);

    p_vortexLinSecFollowsLinPrim = variantManager->addProperty(QVariant::Bool,"");
    linAxisSec->addSubProperty(p_vortexLinSecFollowsLinPrim);
    p_vortexFrictionModelLinSec = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_vortexFrictionModelLinSec->setAttribute("enumNames", frictionModelEnum);
    linAxisSec->addSubProperty(p_vortexFrictionModelLinSec);
    p_vortexFrictionCoeffLinSec = variantManager->addProperty(QVariant::String,"");
    linAxisSec->addSubProperty(p_vortexFrictionCoeffLinSec);
    p_vortexStaticFrictionScaleLinSec = variantManager->addProperty(QVariant::String,"");
    linAxisSec->addSubProperty(p_vortexStaticFrictionScaleLinSec);
    p_vortexSlipLinSec = variantManager->addProperty(QVariant::String,"");
    linAxisSec->addSubProperty(p_vortexSlipLinSec);
    p_vortexSlideLinSec = variantManager->addProperty(QVariant::String,"");
    linAxisSec->addSubProperty(p_vortexSlideLinSec);

    p_vortexFrictionModelAngPrim = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_vortexFrictionModelAngPrim->setAttribute("enumNames", frictionModelEnum);
    angAxisPrim->addSubProperty(p_vortexFrictionModelAngPrim);
    p_vortexFrictionCoeffAngPrim = variantManager->addProperty(QVariant::String,"");
    angAxisPrim->addSubProperty(p_vortexFrictionCoeffAngPrim);
    p_vortexStaticFrictionScaleAngPrim = variantManager->addProperty(QVariant::String,"");
    angAxisPrim->addSubProperty(p_vortexStaticFrictionScaleAngPrim);
    p_vortexSlipAngPrim = variantManager->addProperty(QVariant::String,"");
    angAxisPrim->addSubProperty(p_vortexSlipAngPrim);
    p_vortexSlideAngPrim = variantManager->addProperty(QVariant::String,"");
    angAxisPrim->addSubProperty(p_vortexSlideAngPrim);

    p_vortexAngSecFollowsAngPrim = variantManager->addProperty(QVariant::Bool,"");
    angAxisSec->addSubProperty(p_vortexAngSecFollowsAngPrim);
    p_vortexFrictionModelAngSec = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_vortexFrictionModelAngSec->setAttribute("enumNames", frictionModelEnum);
    angAxisSec->addSubProperty(p_vortexFrictionModelAngSec);
    p_vortexFrictionCoeffAngSec = variantManager->addProperty(QVariant::String,"");
    angAxisSec->addSubProperty(p_vortexFrictionCoeffAngSec);
    p_vortexStaticFrictionScaleAngSec = variantManager->addProperty(QVariant::String,"");
    angAxisSec->addSubProperty(p_vortexStaticFrictionScaleAngSec);
    p_vortexSlipAngSec = variantManager->addProperty(QVariant::String,"");
    angAxisSec->addSubProperty(p_vortexSlipAngSec);
    p_vortexSlideAngSec = variantManager->addProperty(QVariant::String,"");
    angAxisSec->addSubProperty(p_vortexSlideAngSec);

    p_vortexAngNormFollowsAngPrim = variantManager->addProperty(QVariant::Bool,"");
    angAxisNorm->addSubProperty(p_vortexAngNormFollowsAngPrim);
    p_vortexFrictionModelAngNorm = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_vortexFrictionModelAngNorm->setAttribute("enumNames", frictionModelEnum);
    angAxisNorm->addSubProperty(p_vortexFrictionModelAngNorm);
    p_vortexFrictionCoeffAngNorm = variantManager->addProperty(QVariant::String,"");
    angAxisNorm->addSubProperty(p_vortexFrictionCoeffAngNorm);
    p_vortexStaticFrictionScaleAngNorm = variantManager->addProperty(QVariant::String,"");
    angAxisNorm->addSubProperty(p_vortexStaticFrictionScaleAngNorm);
    p_vortexSlipAngNorm = variantManager->addProperty(QVariant::String,"");
    angAxisNorm->addSubProperty(p_vortexSlipAngNorm);
    p_vortexSlideAngNorm = variantManager->addProperty(QVariant::String,"");
    angAxisNorm->addSubProperty(p_vortexSlideAngNorm);


    // Newton:
    p_newtonStaticFriction = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonStaticFriction);
    p_newtonKineticFriction = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonKineticFriction);
    p_newtonRestitution = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonRestitution);
    p_newtonLinearDrag = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonLinearDrag);
    p_newtonAngularDrag = variantManager->addProperty(QVariant::String,"");
    newtonGroup->addSubProperty(p_newtonAngularDrag);
    p_newtonFastMoving = variantManager->addProperty(QVariant::Bool,"");
    newtonGroup->addSubProperty(p_newtonFastMoving);


    refresh();

    enableNotifications(true);

//  setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint);
//  setWindowModality(Qt::ApplicationModal);
    setGeometry(_dlgPosX,_dlgPosY,_dlgSizeX,_dlgSizeY);
}

void CPropBrowserEngineMaterial::enableNotifications(bool enable)
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

void CPropBrowserEngineMaterial::refresh()
{
    CShape* it=App::ct->objCont->getLastSelection_shape();
    CGeomWrap* geom=it->geomData->geomInfo;
    CDynMaterialObject* mat=it->getDynMaterial();
    bool editable=true;

    p_copyFromPredefined->setValue(0);
//    // Material name:
//    p_materialName->setValue(mat->getObjectName().c_str());
//    p_materialName->setEnabled(false);


    // Bullet property names:
    p_bulletFrictionOld->setPropertyName("Friction (only Bullet V2.78)");
    p_bulletFriction->setPropertyName("Friction (after Bullet V2.78)");
    p_bulletRestitution->setPropertyName("Restitution");
    p_bulletLinDamping->setPropertyName("Linear damping");
    p_bulletAngDamping->setPropertyName("Angular damping");
    p_bulletStickyContact->setPropertyName("Sticky contact (only Bullet V2.78)");
    p_bulletAutoShrink->setPropertyName("Auto-shrink convex mesh");
    p_bulletCustomMarginScaling->setPropertyName("Custom collision margin");
    p_bulletMarginScalingFactor->setPropertyName("Custom collision margin factor");

    // Bullet parameters:
    float b_oldFriction=mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr);
    float b_newFriction=mat->getEngineFloatParam(sim_bullet_body_friction,nullptr);
    float b_restitution=mat->getEngineFloatParam(sim_bullet_body_restitution,nullptr);
    float b_linDamp=mat->getEngineFloatParam(sim_bullet_body_lineardamping,nullptr);
    float b_angDamp=mat->getEngineFloatParam(sim_bullet_body_angulardamping,nullptr);
    float b_nonDefCollMargFact=mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,nullptr);
    float b_nonDefCollMargFact_convex=mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,nullptr);
    bool b_sticky=mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr);
    bool b_useNonDefCollMargin=mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,nullptr);
    bool b_useNonDefCollMargin_convex=mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,nullptr);
    bool b_autoShrink=mat->getEngineBoolParam(sim_bullet_body_autoshrinkconvex,nullptr);
    p_bulletFrictionOld->setValue(tt::floatToEInfString(b_oldFriction,false).c_str());
    p_bulletFrictionOld->setEnabled(editable);
    p_bulletFriction->setValue(tt::floatToEInfString(b_newFriction,false).c_str());
    p_bulletFriction->setEnabled(editable);
    p_bulletRestitution->setValue(tt::floatToEInfString(b_restitution,false).c_str());
    p_bulletRestitution->setEnabled(editable);
    p_bulletLinDamping->setValue(tt::floatToEInfString(b_linDamp,false).c_str());
    p_bulletLinDamping->setEnabled(editable);
    p_bulletAngDamping->setValue(tt::floatToEInfString(b_angDamp,false).c_str());
    p_bulletAngDamping->setEnabled(editable);
    p_bulletStickyContact->setValue(b_sticky);
    p_bulletStickyContact->setEnabled(editable);
    p_bulletAutoShrink->setValue(b_autoShrink);
    p_bulletAutoShrink->setEnabled(editable);
    const bool convexAndNotPure=(geom->isConvex()&&(!geom->isPure()));
    bool b_collMargin=b_useNonDefCollMargin;
    if (convexAndNotPure)
        b_collMargin=b_useNonDefCollMargin_convex;
    p_bulletCustomMarginScaling->setValue(b_collMargin);
    p_bulletCustomMarginScaling->setEnabled(editable);
    float b_collMarginF=b_nonDefCollMargFact;
    if (convexAndNotPure)
        b_collMarginF=b_nonDefCollMargFact_convex;
    p_bulletMarginScalingFactor->setValue(tt::floatToEInfString(b_collMarginF,false).c_str());
    p_bulletMarginScalingFactor->setEnabled(b_collMargin);


    // ODE property names:
    p_odeFriction->setPropertyName("Friction");
    p_odeMaxContacts->setPropertyName("Maximum contacts");
    p_odeSoftERP->setPropertyName("Soft ERP");
    p_odeSoftCFM->setPropertyName("Soft CFM");
    p_odeLinDamping->setPropertyName("Linear damping");
    p_odeAngDamping->setPropertyName("Angular damping");

    // ODE parameters:
    float o_friction=mat->getEngineFloatParam(sim_ode_body_friction,nullptr);
    float o_softErp=mat->getEngineFloatParam(sim_ode_body_softerp,nullptr);
    float o_softCfm=mat->getEngineFloatParam(sim_ode_body_softcfm,nullptr);
    float o_linDamp=mat->getEngineFloatParam(sim_ode_body_lineardamping,nullptr);
    float o_angDamp=mat->getEngineFloatParam(sim_ode_body_angulardamping,nullptr);
    int o_maxContacts=mat->getEngineIntParam(sim_ode_body_maxcontacts,nullptr);
    p_odeFriction->setValue(tt::floatToEInfString(o_friction,false).c_str());
    p_odeFriction->setEnabled(editable);
    p_odeMaxContacts->setValue(tt::intToString(o_maxContacts).c_str());
    p_odeMaxContacts->setEnabled(editable);
    p_odeSoftERP->setValue(tt::floatToEInfString(o_softErp,false).c_str());
    p_odeSoftERP->setEnabled(editable);
    p_odeSoftCFM->setValue(tt::floatToEInfString(o_softCfm,false).c_str());
    p_odeSoftCFM->setEnabled(editable);
    p_odeLinDamping->setValue(tt::floatToEInfString(o_linDamp,false).c_str());
    p_odeLinDamping->setEnabled(editable);
    p_odeAngDamping->setValue(tt::floatToEInfString(o_angDamp,false).c_str());
    p_odeAngDamping->setEnabled(editable);


    // Vortex property names:
    p_vortexRestitution->setPropertyName("Restitution");
    p_vortexRestitutionThreshold->setPropertyName("Restitution threshold");
    p_vortexCompliance->setPropertyName(gv::formatUnitStr("Compliance","s^2/k").c_str());
    p_vortexDamping->setPropertyName(gv::formatUnitStr("Damping","k/s").c_str());
    p_vortexAdhesiveForce->setPropertyName(gv::formatUnitStr("Adhesive force","k*m/s^2").c_str());
    p_vortexLinVelocityDamping->setPropertyName(gv::formatUnitStr("Linear velocity damping","k/s").c_str());
    p_vortexAngVelocityDamping->setPropertyName(gv::formatUnitStr("Angular velocity damping","k*m^2/s").c_str());
    p_vortexAutoAngularDampingEnabled->setPropertyName("Auto angular damping enabled");
    p_vortexAutoAngularDampingTensionRatio->setPropertyName("Auto angular damping tension ratio");
    p_vortexSkinThickness->setPropertyName(gv::formatUnitStr("Skin thickness","m").c_str());
    p_vortexAutoSlip->setPropertyName("Auto-slip enabled");
    p_vortexFastMoving->setPropertyName("Fast moving");
    p_vortexPureAsConvex->setPropertyName("Treat pure shape as VxConvexMesh");
    p_vortexConvexAsRandom->setPropertyName("Treat convex shape as VxTriangleMeshBVTree");
    p_vortexRandomAsTerrain->setPropertyName("Treat random shape as VxTriangleMeshUVGrid");
    p_autoSleepLinSpeedThreshold->setPropertyName(gv::formatUnitStr("Threshold linear speed","m/s").c_str());
    p_autoSleepLinAccelThreshold->setPropertyName(gv::formatUnitStr("Threshold linear acceleration","m/s^2").c_str());
    p_autoSleepAngSpeedThreshold->setPropertyName(gv::formatUnitStr("Threshold angular speed","r/s").c_str());
    p_autoSleepAngAccelThreshold->setPropertyName(gv::formatUnitStr("Threshold angular acceleration","r/s^2").c_str());
    p_autoSleepStepsThreshold->setPropertyName("Threshold steps");

    p_vortexFrictionModelLinPrim->setPropertyName("Friction model");
    p_vortexFrictionCoeffLinPrim->setPropertyName("Friction coefficient");
    p_vortexStaticFrictionScaleLinPrim->setPropertyName("Static friction scale");
    p_vortexSlipLinPrim->setPropertyName(gv::formatUnitStr("Slip","s/k").c_str());
    p_vortexSlideLinPrim->setPropertyName(gv::formatUnitStr("Slide","m/s").c_str());

    p_primAxisVectorX->setPropertyName("X");
    p_primAxisVectorY->setPropertyName("Y");
    p_primAxisVectorZ->setPropertyName("Z");

    p_vortexLinSecFollowsLinPrim->setPropertyName("Use same values as linear primary axis");
    p_vortexFrictionModelLinSec->setPropertyName("Friction model");
    p_vortexFrictionCoeffLinSec->setPropertyName("Friction coefficient");
    p_vortexStaticFrictionScaleLinSec->setPropertyName("Static friction scale");
    p_vortexSlipLinSec->setPropertyName(gv::formatUnitStr("Slip","s/k").c_str());
    p_vortexSlideLinSec->setPropertyName(gv::formatUnitStr("Slide","m/s").c_str());

    p_vortexFrictionModelAngPrim->setPropertyName("Friction model");
    p_vortexFrictionCoeffAngPrim->setPropertyName("Friction coefficient");
    p_vortexStaticFrictionScaleAngPrim->setPropertyName("Static friction scale");
    p_vortexSlipAngPrim->setPropertyName(gv::formatUnitStr("Slip","s/(k*m^2)").c_str());
    p_vortexSlideAngPrim->setPropertyName(gv::formatUnitStr("Slide","r/s").c_str());

    p_vortexAngSecFollowsAngPrim->setPropertyName("Use same values as angular primary axis");
    p_vortexFrictionModelAngSec->setPropertyName("Friction model");
    p_vortexFrictionCoeffAngSec->setPropertyName("Friction coefficient");
    p_vortexStaticFrictionScaleAngSec->setPropertyName("Static friction scale");
    p_vortexSlipAngSec->setPropertyName(gv::formatUnitStr("Slip","s/(k*m^2)").c_str());
    p_vortexSlideAngSec->setPropertyName(gv::formatUnitStr("Slide","r/s").c_str());

    p_vortexAngNormFollowsAngPrim->setPropertyName("Use same values as angular primary axis");
    p_vortexFrictionModelAngNorm->setPropertyName("Friction model");
    p_vortexFrictionCoeffAngNorm->setPropertyName("Friction coefficient");
    p_vortexStaticFrictionScaleAngNorm->setPropertyName("Static friction scale");
    p_vortexSlipAngNorm->setPropertyName(gv::formatUnitStr("Slip","s/(k*m^2)").c_str());
    p_vortexSlideAngNorm->setPropertyName(gv::formatUnitStr("Slide","r/s").c_str());


    // Vortex parameters:
    float frictionCoeff_primary_linearAxis=mat->getEngineFloatParam(sim_vortex_body_primlinearaxisfriction,nullptr);
    float frictionCoeff_secondary_linearAxis=mat->getEngineFloatParam(sim_vortex_body_seclinearaxisfriction,nullptr);
    float frictionCoeff_primary_angularAxis=mat->getEngineFloatParam(sim_vortex_body_primangularaxisfriction,nullptr);
    float frictionCoeff_secondary_angularAxis=mat->getEngineFloatParam(sim_vortex_body_secangularaxisfriction,nullptr);
    float frictionCoeff_normal_angularAxis=mat->getEngineFloatParam(sim_vortex_body_normalangularaxisfriction,nullptr);
    float staticFrictionScale_primary_linearAxis=mat->getEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,nullptr);
    float staticFrictionScale_secondary_linearAxis=mat->getEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,nullptr);
    float staticFrictionScale_primary_angularAxis=mat->getEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,nullptr);
    float staticFrictionScale_secondary_angularAxis=mat->getEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,nullptr);
    float staticFrictionScale_normal_angularAxis=mat->getEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,nullptr);
    float compliance=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_compliance,nullptr),1.0f,true);
    float damping=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_damping,nullptr),1.0f,true);
    float restitution=mat->getEngineFloatParam(sim_vortex_body_restitution,nullptr);
    float restitutionThreshold=mat->getEngineFloatParam(sim_vortex_body_restitutionthreshold,nullptr);
    float adhesiveForce=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_adhesiveforce,nullptr),1.0f,true);
    float linearVelocityDamping=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_linearvelocitydamping,nullptr),1.0f,true);
    float angularVelocityDamping=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_angularvelocitydamping,nullptr),1.0f,true);
    float slide_primary_linearAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslide,nullptr),1.0f,true);
    float slide_secondary_linearAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_seclinearaxisslide,nullptr),1.0f,true);
    float slide_primary_angularAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primangularaxisslide,nullptr),gv::angularVelToUser,true);
    float slide_secondary_angularAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_secangularaxisslide,nullptr),gv::angularVelToUser,true);
    float slide_normal_angularAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_normalangularaxisslide,nullptr),gv::angularVelToUser,true);
    float slip_primary_linearAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslip,nullptr),1.0f,true);
    float slip_secondary_linearAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_seclinearaxisslip,nullptr),1.0f,true);
    float slip_primary_angularAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primangularaxisslip,nullptr),1.0f,true);
    float slip_secondary_angularAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_secangularaxisslip,nullptr),1.0f,true);
    float slip_normal_angularAxis=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_normalangularaxisslip,nullptr),1.0f,true);
    float autoSleep_linear_speed_threshold=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold,nullptr),1.0f,true);
    float autoSleep_linear_accel_threshold=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold,nullptr),1.0f,true);
    float autoSleep_angular_speed_threshold=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold,nullptr),gv::angularVelToUser,true);
    float autoSleep_angular_accel_threshold=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold,nullptr),gv::angularAccelToUser,true);
    float skin_thickness=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_skinthickness,nullptr),1.0f,true);;
    float autoAngularDampingTensionRatio=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio,nullptr),1,true);
    float primaryAxisVectorX=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primaxisvectorx,nullptr),1,false);
    float primaryAxisVectorY=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primaxisvectory,nullptr),1,false);
    float primaryAxisVectorZ=tt::floatToUserFloat(mat->getEngineFloatParam(sim_vortex_body_primaxisvectorz,nullptr),1,false);

    int frictionModel_primary_linearAxis=mat->getEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,nullptr);
    int frictionModel_secondary_linearAxis=mat->getEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,nullptr);
    int frictionModel_primary_angularAxis=mat->getEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,nullptr);
    int frictionModel_secondary_angularAxis=mat->getEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,nullptr);
    int frictionModel_normal_angularAxis=mat->getEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,nullptr);
    int autoSleepStepLiveThreshold=mat->getEngineIntParam(sim_vortex_body_autosleepsteplivethreshold,nullptr);
    //int uniqueId=mat->getEngineIntParam(sim_vortex_body_materialuniqueid);

    bool treatPureShapesAsConvexShapes=mat->getEngineBoolParam(sim_vortex_body_pureshapesasconvex,nullptr);
    bool treatConvexShapesAsRandomShapes=mat->getEngineBoolParam(sim_vortex_body_convexshapesasrandom,nullptr);
    bool treatRandomShapesAsTerrain=mat->getEngineBoolParam(sim_vortex_body_randomshapesasterrain,nullptr);
    bool fastMoving=mat->getEngineBoolParam(sim_vortex_body_fastmoving,nullptr);
    bool autoSlip=mat->getEngineBoolParam(sim_vortex_body_autoslip,nullptr);
    bool linearSecFollowsLinearPrim=mat->getEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,nullptr);
    bool angularSecFollowsAngularPrim=mat->getEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,nullptr);
    bool angularNormFollowsAngularPrim=mat->getEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,nullptr);
    bool autoAngularDampingEnabled=mat->getEngineBoolParam(sim_vortex_body_autoangulardamping,nullptr);

    std::string linAxis1Text("Linear primary axis (friction: ");
    std::string linAxis2Text("Linear secondary axis (friction: ");
    std::string angAxis1Text("Angular primary axis (friction: ");
    std::string angAxis2Text("Angular secondary axis (friction: ");
    std::string angAxis3Text("Angular normal axis (friction: ");

    std::string linAxisVectorText("Axis orientation: [");



    p_vortexRestitution->setValue(tt::floatToEInfString(restitution,false).c_str());
    p_vortexRestitution->setEnabled(editable);
    p_vortexRestitutionThreshold->setValue(tt::floatToEInfString(restitutionThreshold,false).c_str());
    p_vortexRestitutionThreshold->setEnabled(editable);
    p_vortexCompliance->setValue(tt::floatToEInfString(compliance,false).c_str());
    p_vortexCompliance->setEnabled(editable);
    p_vortexDamping->setValue(tt::floatToEInfString(damping,false).c_str());
    p_vortexDamping->setEnabled(editable);
    p_vortexAdhesiveForce->setValue(tt::floatToEInfString(adhesiveForce,false).c_str());
    p_vortexAdhesiveForce->setEnabled(editable);
    p_vortexLinVelocityDamping->setValue(tt::floatToEInfString(linearVelocityDamping,false).c_str());
    p_vortexLinVelocityDamping->setEnabled(editable);
    p_vortexAngVelocityDamping->setValue(tt::floatToEInfString(angularVelocityDamping,false).c_str());
    p_vortexAngVelocityDamping->setEnabled(editable);
    p_vortexAutoAngularDampingEnabled->setValue(autoAngularDampingEnabled);
    p_vortexAutoAngularDampingEnabled->setEnabled(editable);
    p_vortexAutoAngularDampingTensionRatio->setValue(tt::floatToEInfString(autoAngularDampingTensionRatio,false).c_str());
    p_vortexAutoAngularDampingTensionRatio->setEnabled(editable&&autoAngularDampingEnabled);
    p_vortexSkinThickness->setValue(tt::floatToEInfString(skin_thickness,false).c_str());
    p_vortexSkinThickness->setEnabled(editable);
    p_vortexAutoSlip->setValue(autoSlip);
    p_vortexAutoSlip->setEnabled(editable);
    p_vortexFastMoving->setValue(fastMoving);
    p_vortexFastMoving->setEnabled(editable);
    p_vortexPureAsConvex->setValue(treatPureShapesAsConvexShapes);
    p_vortexPureAsConvex->setEnabled(editable);
    p_vortexConvexAsRandom->setValue(treatConvexShapesAsRandomShapes);
    p_vortexConvexAsRandom->setEnabled(editable);
    p_vortexRandomAsTerrain->setValue(treatRandomShapesAsTerrain);
    p_vortexRandomAsTerrain->setEnabled(editable);
    p_autoSleepLinSpeedThreshold->setValue(tt::floatToEInfString(autoSleep_linear_speed_threshold,false).c_str());
    p_autoSleepLinSpeedThreshold->setEnabled(editable);
    p_autoSleepLinAccelThreshold->setValue(tt::floatToEInfString(autoSleep_linear_accel_threshold,false).c_str());
    p_autoSleepLinAccelThreshold->setEnabled(editable);
    p_autoSleepAngSpeedThreshold->setValue(tt::floatToEInfString(autoSleep_angular_speed_threshold,false).c_str());
    p_autoSleepAngSpeedThreshold->setEnabled(editable);
    p_autoSleepAngAccelThreshold->setValue(tt::floatToEInfString(autoSleep_angular_accel_threshold,false).c_str());
    p_autoSleepAngAccelThreshold->setEnabled(editable);
    p_autoSleepStepsThreshold->setValue(tt::intToString(autoSleepStepLiveThreshold).c_str());
    p_autoSleepStepsThreshold->setEnabled(editable);

    p_vortexFrictionModelLinPrim->setValue(frictionModel_primary_linearAxis);
    p_vortexFrictionModelLinPrim->setEnabled(editable);
    p_vortexFrictionCoeffLinPrim->setValue(tt::floatToEInfString(frictionCoeff_primary_linearAxis,false).c_str());
    p_vortexFrictionCoeffLinPrim->setEnabled(editable);
    p_vortexStaticFrictionScaleLinPrim->setValue(tt::floatToEInfString(staticFrictionScale_primary_linearAxis,false).c_str());
    p_vortexStaticFrictionScaleLinPrim->setEnabled(editable);
    p_vortexSlipLinPrim->setValue(tt::floatToEInfString(slip_primary_linearAxis,false).c_str());
    p_vortexSlipLinPrim->setEnabled(editable);
    p_vortexSlideLinPrim->setValue(tt::floatToEInfString(slide_primary_linearAxis,false).c_str());
    p_vortexSlideLinPrim->setEnabled(editable);

    p_primAxisVectorX->setValue(tt::floatToEInfString(primaryAxisVectorX,true).c_str());
    p_primAxisVectorX->setEnabled(editable);
    p_primAxisVectorY->setValue(tt::floatToEInfString(primaryAxisVectorY,true).c_str());
    p_primAxisVectorY->setEnabled(editable);
    p_primAxisVectorZ->setValue(tt::floatToEInfString(primaryAxisVectorZ,true).c_str());
    p_primAxisVectorZ->setEnabled(editable);

    linAxisVectorText+=tt::floatToEInfString(primaryAxisVectorX,true);
    linAxisVectorText+=", ";
    linAxisVectorText+=tt::floatToEInfString(primaryAxisVectorY,true);
    linAxisVectorText+=", ";
    linAxisVectorText+=tt::floatToEInfString(primaryAxisVectorZ,true);
    linAxisVectorText+="]";

    if (frictionModel_primary_linearAxis==6)
        linAxis1Text+=tt::floatToEInfString(0.0f,false);
    else
        linAxis1Text+=tt::floatToEInfString(frictionCoeff_primary_linearAxis,false);
    linAxis1Text+=")";

    p_vortexLinSecFollowsLinPrim->setValue(linearSecFollowsLinearPrim);
    p_vortexLinSecFollowsLinPrim->setEnabled(editable);
    p_vortexFrictionModelLinSec->setValue(frictionModel_secondary_linearAxis);
    p_vortexFrictionModelLinSec->setEnabled((!linearSecFollowsLinearPrim)&&editable);
    p_vortexFrictionCoeffLinSec->setValue(tt::floatToEInfString(frictionCoeff_secondary_linearAxis,false).c_str());
    p_vortexFrictionCoeffLinSec->setEnabled((!linearSecFollowsLinearPrim)&&editable);
    p_vortexStaticFrictionScaleLinSec->setValue(tt::floatToEInfString(staticFrictionScale_secondary_linearAxis,false).c_str());
    p_vortexStaticFrictionScaleLinSec->setEnabled((!linearSecFollowsLinearPrim)&&editable);
    p_vortexSlipLinSec->setValue(tt::floatToEInfString(slip_secondary_linearAxis,false).c_str());
    p_vortexSlipLinSec->setEnabled((!linearSecFollowsLinearPrim)&&editable);
    p_vortexSlideLinSec->setValue(tt::floatToEInfString(slide_secondary_linearAxis,false).c_str());
    p_vortexSlideLinSec->setEnabled((!linearSecFollowsLinearPrim)&&editable);

    if (frictionModel_secondary_linearAxis==6)
        linAxis2Text+=tt::floatToEInfString(0.0f,false);
    else
        linAxis2Text+=tt::floatToEInfString(frictionCoeff_secondary_linearAxis,false);
    linAxis2Text+=")";

    p_vortexFrictionModelAngPrim->setValue(frictionModel_primary_angularAxis);
    p_vortexFrictionModelAngPrim->setEnabled(editable);
    p_vortexFrictionCoeffAngPrim->setValue(tt::floatToEInfString(frictionCoeff_primary_angularAxis,false).c_str());
    p_vortexFrictionCoeffAngPrim->setEnabled(editable);
    p_vortexStaticFrictionScaleAngPrim->setValue(tt::floatToEInfString(staticFrictionScale_primary_angularAxis,false).c_str());
    p_vortexStaticFrictionScaleAngPrim->setEnabled(editable);
    p_vortexSlipAngPrim->setValue(tt::floatToEInfString(slip_primary_angularAxis,false).c_str());
    p_vortexSlipAngPrim->setEnabled(editable);
    p_vortexSlideAngPrim->setValue(tt::floatToEInfString(slide_primary_angularAxis,false).c_str());
    p_vortexSlideAngPrim->setEnabled(editable);

    if (frictionModel_primary_angularAxis==6)
        angAxis1Text+=tt::floatToEInfString(0.0f,false);
    else
        angAxis1Text+=tt::floatToEInfString(frictionCoeff_primary_angularAxis,false);
    angAxis1Text+=")";



    p_vortexAngSecFollowsAngPrim->setValue(angularSecFollowsAngularPrim);
    p_vortexAngSecFollowsAngPrim->setEnabled(editable);
    p_vortexFrictionModelAngSec->setValue(frictionModel_secondary_angularAxis);
    p_vortexFrictionModelAngSec->setEnabled((!angularSecFollowsAngularPrim)&&editable);
    p_vortexFrictionCoeffAngSec->setValue(tt::floatToEInfString(frictionCoeff_secondary_angularAxis,false).c_str());
    p_vortexFrictionCoeffAngSec->setEnabled((!angularSecFollowsAngularPrim)&&editable);
    p_vortexStaticFrictionScaleAngSec->setValue(tt::floatToEInfString(staticFrictionScale_secondary_angularAxis,false).c_str());
    p_vortexStaticFrictionScaleAngSec->setEnabled((!angularSecFollowsAngularPrim)&&editable);
    p_vortexSlipAngSec->setValue(tt::floatToEInfString(slip_secondary_angularAxis,false).c_str());
    p_vortexSlipAngSec->setEnabled((!angularSecFollowsAngularPrim)&&editable);
    p_vortexSlideAngSec->setValue(tt::floatToEInfString(slide_secondary_angularAxis,false).c_str());
    p_vortexSlideAngSec->setEnabled((!angularSecFollowsAngularPrim)&&editable);

    if (frictionModel_secondary_angularAxis==6)
        angAxis2Text+=tt::floatToEInfString(0.0f,false);
    else
        angAxis2Text+=tt::floatToEInfString(frictionCoeff_secondary_angularAxis,false);
    angAxis2Text+=")";

    p_vortexAngNormFollowsAngPrim->setValue(angularNormFollowsAngularPrim);
    p_vortexAngNormFollowsAngPrim->setEnabled(editable);
    p_vortexFrictionModelAngNorm->setValue(frictionModel_normal_angularAxis);
    p_vortexFrictionModelAngNorm->setEnabled((!angularNormFollowsAngularPrim)&&editable);
    p_vortexFrictionCoeffAngNorm->setValue(tt::floatToEInfString(frictionCoeff_normal_angularAxis,false).c_str());
    p_vortexFrictionCoeffAngNorm->setEnabled((!angularNormFollowsAngularPrim)&&editable);
    p_vortexStaticFrictionScaleAngNorm->setValue(tt::floatToEInfString(staticFrictionScale_normal_angularAxis,false).c_str());
    p_vortexStaticFrictionScaleAngNorm->setEnabled((!angularNormFollowsAngularPrim)&&editable);
    p_vortexSlipAngNorm->setValue(tt::floatToEInfString(slip_normal_angularAxis,false).c_str());
    p_vortexSlipAngNorm->setEnabled((!angularNormFollowsAngularPrim)&&editable);
    p_vortexSlideAngNorm->setValue(tt::floatToEInfString(slide_normal_angularAxis,false).c_str());
    p_vortexSlideAngNorm->setEnabled((!angularNormFollowsAngularPrim)&&editable);

    if (frictionModel_normal_angularAxis==6)
        angAxis3Text+=tt::floatToEInfString(0.0f,false);
    else
        angAxis3Text+=tt::floatToEInfString(frictionCoeff_normal_angularAxis,false);
    angAxis3Text+=")";

    // some group labels:
    linAxisPrim->setPropertyName(linAxis1Text.c_str());
    linAxisSec->setPropertyName(linAxis2Text.c_str());
    angAxisPrim->setPropertyName(angAxis1Text.c_str());
    angAxisSec->setPropertyName(angAxis2Text.c_str());
    angAxisNorm->setPropertyName(angAxis3Text.c_str());

    linAxisPrimVector->setPropertyName(linAxisVectorText.c_str());


    // Newton property names:
    p_newtonStaticFriction->setPropertyName("Static friction");
    p_newtonKineticFriction->setPropertyName("Kinetic friction");
    p_newtonRestitution->setPropertyName("Restitution");
    p_newtonLinearDrag->setPropertyName("Linear drag");
    p_newtonAngularDrag->setPropertyName("Angular drag");
    p_newtonFastMoving->setPropertyName("Fast moving");

    // Newton parameters:
    float newtonStaticFriction=mat->getEngineFloatParam(sim_newton_body_staticfriction,nullptr);
    float newtonKineticFriction=mat->getEngineFloatParam(sim_newton_body_kineticfriction,nullptr);
    float newtonRestitution=mat->getEngineFloatParam(sim_newton_body_restitution,nullptr);
    float newtonLinearDrag=mat->getEngineFloatParam(sim_newton_body_lineardrag,nullptr);
    float newtonAngularDrag=mat->getEngineFloatParam(sim_newton_body_angulardrag,nullptr);

    bool newtonFastMoving=mat->getEngineBoolParam(sim_newton_body_fastmoving,nullptr);

    p_newtonStaticFriction->setValue(tt::floatToEInfString(newtonStaticFriction,false).c_str());
    p_newtonStaticFriction->setEnabled(editable);
    p_newtonKineticFriction->setValue(tt::floatToEInfString(newtonKineticFriction,false).c_str());
    p_newtonKineticFriction->setEnabled(editable);
    p_newtonRestitution->setValue(tt::floatToEInfString(newtonRestitution,false).c_str());
    p_newtonRestitution->setEnabled(editable);
    p_newtonLinearDrag->setValue(tt::floatToEInfString(newtonLinearDrag,false).c_str());
    p_newtonLinearDrag->setEnabled(editable);
    p_newtonAngularDrag->setValue(tt::floatToEInfString(newtonAngularDrag,false).c_str());
    p_newtonAngularDrag->setEnabled(editable);
    p_newtonFastMoving->setValue(newtonFastMoving);
    p_newtonFastMoving->setEnabled(editable);

}

QtBrowserItem* CPropBrowserEngineMaterial::getSubPropertyBrowserItem(const QtBrowserItem* parentBrowserItem,const QtProperty* childProperty)
{
    QList<QtBrowserItem*> children=parentBrowserItem->children();
    for (int i=0;i<int(children.size());i++)
    {
        if (children[i]->property()==childProperty)
            return(children[i]);
    }
    return(nullptr);
}


void CPropBrowserEngineMaterial::catchPropertyChanges(QtProperty *_prop, QVariant value)
{
    enableNotifications(false);
    handlePropertyChanges(_prop);
    enableNotifications(true);
}
void CPropBrowserEngineMaterial::catchPropertyChangesString(QtProperty *_prop, QString value)
{
    enableNotifications(false);
    handlePropertyChanges(_prop);
    enableNotifications(true);
}

void CPropBrowserEngineMaterial::handlePropertyChanges(QtProperty *_prop)
{
    CShape* it=App::ct->objCont->getLastSelection_shape();
    CGeomWrap* geom=it->geomData->geomInfo;
    CDynMaterialObject* mat=it->getDynMaterial();

    float f;
    int a;

    // Did we set a predefined material?
    if (_prop==p_copyFromPredefined)
    {
        int matId=((QtVariantProperty*)_prop)->value().toInt()+sim_dynmat_default-1;
        if (matId>=sim_dynmat_default)
            mat->generateDefaultMaterial(matId);
    }

//    // Did we change the material name?
//    if (_prop==p_materialName)
//    {
//        std::string name(((QtVariantProperty*)_prop)->value().toString().toStdString());
//        mat->setObjectName(name.c_str());
//    }


    // Bullet
    bool convexAndNotPure=(geom->isConvex()&&(!geom->isPure()));
    if (_prop==p_bulletFrictionOld)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_bullet_body_oldfriction,f);
    }
    if (_prop==p_bulletFriction)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_bullet_body_friction,f);
    }
    if (_prop==p_bulletRestitution)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_bullet_body_restitution,f);
    }
    if (_prop==p_bulletLinDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_bullet_body_lineardamping,f);
    }
    if (_prop==p_bulletAngDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_bullet_body_angulardamping,f);
    }
    if (_prop==p_bulletStickyContact)
        mat->setEngineBoolParam(sim_bullet_body_sticky,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_bulletAutoShrink)
        mat->setEngineBoolParam(sim_bullet_body_autoshrinkconvex,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_bulletCustomMarginScaling)
    {
        if (convexAndNotPure)
            mat->setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,((QtVariantProperty*)_prop)->value().toBool());
        else
            mat->setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,((QtVariantProperty*)_prop)->value().toBool());
    }
    if (_prop==p_bulletMarginScalingFactor)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
        {
            if (convexAndNotPure)
                mat->setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,f);
            else
                mat->setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,f);
        }
    }


    // ODE
    if (_prop==p_odeFriction)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_ode_body_friction,f);
    }
    if (_prop==p_odeMaxContacts)
    {
        if (tt::stringToInt(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),a))
            mat->setEngineIntParam(sim_ode_body_maxcontacts,a);
    }
    if (_prop==p_odeSoftERP)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_ode_body_softerp,f);
    }
    if (_prop==p_odeSoftCFM)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_ode_body_softcfm,f);
    }
    if (_prop==p_odeLinDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_ode_body_lineardamping,f);
    }
    if (_prop==p_odeAngDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_ode_body_angulardamping,f);
    }

    // Vortex
    if (_prop==p_vortexRestitution)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_restitution,f);
    }
    if (_prop==p_vortexRestitutionThreshold)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_restitutionthreshold,f);
    }
    if (_prop==p_vortexCompliance)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_compliance,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexAdhesiveForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_adhesiveforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexLinVelocityDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_linearvelocitydamping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexAngVelocityDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_angularvelocitydamping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexAutoAngularDampingEnabled)
        mat->setEngineBoolParam(sim_vortex_body_autoangulardamping,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexAutoAngularDampingTensionRatio)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio,f);
    }
    if (_prop==p_vortexSkinThickness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            mat->setEngineFloatParam(sim_vortex_body_skinthickness,tt::userFloatToFloat(f,1.0f,false));
    }
    if (_prop==p_vortexAutoSlip)
        mat->setEngineBoolParam(sim_vortex_body_autoslip,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexFastMoving)
        mat->setEngineBoolParam(sim_vortex_body_fastmoving,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexPureAsConvex)
        mat->setEngineBoolParam(sim_vortex_body_pureshapesasconvex,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexConvexAsRandom)
        mat->setEngineBoolParam(sim_vortex_body_convexshapesasrandom,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexRandomAsTerrain)
        mat->setEngineBoolParam(sim_vortex_body_randomshapesasterrain,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_autoSleepLinSpeedThreshold)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_autoSleepLinAccelThreshold)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_autoSleepAngSpeedThreshold)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold,tt::userFloatToFloat(f,gv::userToAngularVel,true));
    }
    if (_prop==p_autoSleepAngAccelThreshold)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold,tt::userFloatToFloat(f,gv::userToAngularAccel,true));
    }
    if (_prop==p_autoSleepStepsThreshold)
    {
        if (tt::stringToInt(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),a))
            mat->setEngineIntParam(sim_vortex_body_autosleepsteplivethreshold,a);
    }


    if (_prop==p_primAxisVectorX)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            mat->setEngineFloatParam(sim_vortex_body_primaxisvectorx,tt::userFloatToFloat(f,1,false));
    }

    if (_prop==p_primAxisVectorY)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            mat->setEngineFloatParam(sim_vortex_body_primaxisvectory,tt::userFloatToFloat(f,1,false));
    }

    if (_prop==p_primAxisVectorZ)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            mat->setEngineFloatParam(sim_vortex_body_primaxisvectorz,tt::userFloatToFloat(f,1,false));
    }


    if (_prop==p_vortexFrictionModelLinPrim)
        mat->setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,((QtVariantProperty*)_prop)->value().toInt());
    if (_prop==p_vortexFrictionCoeffLinPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,f);
    }
    if (_prop==p_vortexStaticFrictionScaleLinPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,f);
    }
    if (_prop==p_vortexSlipLinPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primlinearaxisslip,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexSlideLinPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primlinearaxisslide,tt::userFloatToFloat(f,1.0f,true));
    }


    if (_prop==p_vortexLinSecFollowsLinPrim)
        mat->setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexFrictionModelLinSec)
        mat->setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,((QtVariantProperty*)_prop)->value().toInt());
    if (_prop==p_vortexFrictionCoeffLinSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,f);
    }
    if (_prop==p_vortexStaticFrictionScaleLinSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,f);
    }
    if (_prop==p_vortexSlipLinSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslip,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexSlideLinSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslide,tt::userFloatToFloat(f,1.0f,true));
    }


    if (_prop==p_vortexFrictionModelAngPrim)
        mat->setEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,((QtVariantProperty*)_prop)->value().toInt());
    if (_prop==p_vortexFrictionCoeffAngPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primangularaxisfriction,f);
    }
    if (_prop==p_vortexStaticFrictionScaleAngPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,f);
    }
    if (_prop==p_vortexSlipAngPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primangularaxisslip,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexSlideAngPrim)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_primangularaxisslide,tt::userFloatToFloat(f,gv::userToAngularVel,true));
    }


    if (_prop==p_vortexAngSecFollowsAngPrim)
        mat->setEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexFrictionModelAngSec)
        mat->setEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,((QtVariantProperty*)_prop)->value().toInt());
    if (_prop==p_vortexFrictionCoeffAngSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_secangularaxisfriction,f);
    }
    if (_prop==p_vortexStaticFrictionScaleAngSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,f);
    }
    if (_prop==p_vortexSlipAngSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_secangularaxisslip,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexSlideAngSec)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_secangularaxisslide,tt::userFloatToFloat(f,gv::userToAngularVel,true));
    }


    if (_prop==p_vortexAngNormFollowsAngPrim)
        mat->setEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexFrictionModelAngNorm)
        mat->setEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,((QtVariantProperty*)_prop)->value().toInt());
    if (_prop==p_vortexFrictionCoeffAngNorm)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_normalangularaxisfriction,f);
    }
    if (_prop==p_vortexStaticFrictionScaleAngNorm)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,f);
    }
    if (_prop==p_vortexSlipAngNorm)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslip,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexSlideAngNorm)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslide,tt::userFloatToFloat(f,gv::userToAngularVel,true));
    }

    //************************************
    if (mat->getEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,nullptr))
    {
        mat->setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,mat->getEngineFloatParam(sim_vortex_body_primlinearaxisfriction,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,mat->getEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslide,mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslide,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslip,mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslip,nullptr));
        mat->setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,mat->getEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,nullptr));
    }
    if (mat->getEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,nullptr))
    {
        mat->setEngineFloatParam(sim_vortex_body_secangularaxisfriction,mat->getEngineFloatParam(sim_vortex_body_primangularaxisfriction,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,mat->getEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_secangularaxisslide,mat->getEngineFloatParam(sim_vortex_body_primangularaxisslide,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_secangularaxisslip,mat->getEngineFloatParam(sim_vortex_body_primangularaxisslip,nullptr));
        mat->setEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,mat->getEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,nullptr));
    }
    if (mat->getEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,nullptr))
    {
        mat->setEngineFloatParam(sim_vortex_body_normalangularaxisfriction,mat->getEngineFloatParam(sim_vortex_body_primangularaxisfriction,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,mat->getEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslide,mat->getEngineFloatParam(sim_vortex_body_primangularaxisslide,nullptr));
        mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslip,mat->getEngineFloatParam(sim_vortex_body_primangularaxisslip,nullptr));
        mat->setEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,mat->getEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,nullptr));
    }
    //************************************




    // Newton
    if (_prop==p_newtonStaticFriction)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_newton_body_staticfriction,f);
    }
    if (_prop==p_newtonKineticFriction)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_newton_body_kineticfriction,f);
    }
    if (_prop==p_newtonRestitution)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_newton_body_restitution,f);
    }
    if (_prop==p_newtonLinearDrag)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_newton_body_lineardrag,f);
    }
    if (_prop==p_newtonAngularDrag)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,true))
            mat->setEngineFloatParam(sim_newton_body_angulardrag,f);
    }
    if (_prop==p_newtonFastMoving)
        mat->setEngineBoolParam(sim_newton_body_fastmoving,((QtVariantProperty*)_prop)->value().toBool());

    refresh();
}
