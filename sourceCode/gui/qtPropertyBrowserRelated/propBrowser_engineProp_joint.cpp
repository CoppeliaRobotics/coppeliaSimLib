#include "propBrowser_engineProp_joint.h"

#include "tt.h"
#include "gV.h"
#include "app.h"


bool CPropBrowserEngineJoint::_bulletPropertiesExpanded=true;
bool CPropBrowserEngineJoint::_odePropertiesExpanded=true;
bool CPropBrowserEngineJoint::_vortexPropertiesExpanded=true;
bool CPropBrowserEngineJoint::_newtonPropertiesExpanded=true;

bool CPropBrowserEngineJoint::_vortexJointAxisFrictionExpanded=false;
bool CPropBrowserEngineJoint::_vortexJointAxisLimitsExpanded=false;
bool CPropBrowserEngineJoint::_vortexJointDependencyExpanded=false;

bool CPropBrowserEngineJoint::_vortexP0Expanded=false;
bool CPropBrowserEngineJoint::_vortexP0RelaxationExpanded=true;
bool CPropBrowserEngineJoint::_vortexP0FrictionExpanded=true;

bool CPropBrowserEngineJoint::_vortexP1Expanded=false;
bool CPropBrowserEngineJoint::_vortexP1RelaxationExpanded=true;
bool CPropBrowserEngineJoint::_vortexP1FrictionExpanded=true;

bool CPropBrowserEngineJoint::_vortexP2Expanded=false;
bool CPropBrowserEngineJoint::_vortexP2RelaxationExpanded=true;
bool CPropBrowserEngineJoint::_vortexP2FrictionExpanded=true;

bool CPropBrowserEngineJoint::_vortexA0Expanded=false;
bool CPropBrowserEngineJoint::_vortexA0RelaxationExpanded=true;
bool CPropBrowserEngineJoint::_vortexA0FrictionExpanded=true;

bool CPropBrowserEngineJoint::_vortexA1Expanded=false;
bool CPropBrowserEngineJoint::_vortexA1RelaxationExpanded=true;
bool CPropBrowserEngineJoint::_vortexA1FrictionExpanded=true;

bool CPropBrowserEngineJoint::_vortexA2Expanded=false;
bool CPropBrowserEngineJoint::_vortexA2RelaxationExpanded=true;
bool CPropBrowserEngineJoint::_vortexA2FrictionExpanded=true;

bool CPropBrowserEngineJoint::_newtonJointDependencyExpanded=false;


int CPropBrowserEngineJoint::_dlgSizeX=500;
int CPropBrowserEngineJoint::_dlgSizeY=470;
int CPropBrowserEngineJoint::_dlgPosX=800;
int CPropBrowserEngineJoint::_dlgPosY=100;
int CPropBrowserEngineJoint::_splitterPos=250;

CPropBrowserEngineJoint::CPropBrowserEngineJoint(QWidget* pParent) : QtTreePropertyBrowser(pParent)
{
    show(nullptr);
}

CPropBrowserEngineJoint::~CPropBrowserEngineJoint()
{
    enableNotifications(false);

    _bulletPropertiesExpanded=isExpanded(bulletGroupItem);
    _odePropertiesExpanded=isExpanded(odeGroupItem);
    _vortexPropertiesExpanded=isExpanded(vortexGroupItem);
    _newtonPropertiesExpanded=isExpanded(newtonGroupItem);

    _vortexJointAxisFrictionExpanded=isExpanded(vortexJointAxisFrictionItem);
    _vortexJointAxisLimitsExpanded=isExpanded(vortexJointAxisLimitsItem);
    _vortexJointDependencyExpanded=isExpanded(vortexJointDependencyItem);

    _vortexP0Expanded=isExpanded(vortexP0Item);
    _vortexP0RelaxationExpanded=isExpanded(vortexP0RelaxationItem);
    _vortexP0FrictionExpanded=isExpanded(vortexP0FrictionItem);

    _vortexP1Expanded=isExpanded(vortexP1Item);
    _vortexP1RelaxationExpanded=isExpanded(vortexP1RelaxationItem);
    _vortexP1FrictionExpanded=isExpanded(vortexP1FrictionItem);

    _vortexP2Expanded=isExpanded(vortexP2Item);
    _vortexP2RelaxationExpanded=isExpanded(vortexP2RelaxationItem);
    _vortexP2FrictionExpanded=isExpanded(vortexP2FrictionItem);

    _vortexA0Expanded=isExpanded(vortexA0Item);
    _vortexA0RelaxationExpanded=isExpanded(vortexA0RelaxationItem);
    _vortexA0FrictionExpanded=isExpanded(vortexA0FrictionItem);

    _vortexA1Expanded=isExpanded(vortexA1Item);
    _vortexA1RelaxationExpanded=isExpanded(vortexA1RelaxationItem);
    _vortexA1FrictionExpanded=isExpanded(vortexA1FrictionItem);

    _vortexA2Expanded=isExpanded(vortexA2Item);
    _vortexA2RelaxationExpanded=isExpanded(vortexA2RelaxationItem);
    _vortexA2FrictionExpanded=isExpanded(vortexA2FrictionItem);

    _newtonJointDependencyExpanded=isExpanded(newtonJointDependencyItem);

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


void CPropBrowserEngineJoint::reject()
{
    done(QDialog::Accepted);
}

void CPropBrowserEngineJoint::accept()
{
    done(QDialog::Accepted);
}

void CPropBrowserEngineJoint::show(QWidget* parentWindow)
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
    setWindowTitle("Physics Engines Properties - Joints");


    variantManager = new QtVariantPropertyManager();
    variantFactory = new QtVariantEditorFactory();
    setFactoryForManager(variantManager,variantFactory);


    buttonManager = new ButtonEditManager();
    buttonFactory  = new PushButtonEditFactory();
    setFactoryForManager(buttonManager,buttonFactory);


    p_applyAllButton = buttonManager->addProperty("Apply all properties to selected joints");
    buttonManager->setValue(p_applyAllButton,"Apply");
    QtBrowserItem* anItem=addProperty(p_applyAllButton);
    setBackgroundColor(anItem,QTPROPERTYBROWSER_COLOR_GREY);


    QtVariantProperty *bulletGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Bullet properties");
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

    // Bullet properties:
    p_bulletNormalCFM = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletNormalCFM);

    p_bulletStopERP = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletStopERP);

    p_bulletStopCFM = variantManager->addProperty(QVariant::String,"");
    bulletGroup->addSubProperty(p_bulletStopCFM);


    // ODE properties:
    p_odeNormalCFM = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeNormalCFM);

    p_odeStopERP = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeStopERP);

    p_odeStopCFM = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeStopCFM);

    p_odeBounce = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeBounce);

    p_odeFudgeFactor = variantManager->addProperty(QVariant::String,"");
    odeGroup->addSubProperty(p_odeFudgeFactor);


    // Vortex properties:
    vortexJointAxisFriction = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexJointAxisFriction);
    vortexJointAxisFrictionItem=getSubPropertyBrowserItem(vortexGroupItem,vortexJointAxisFriction);
    setExpanded(vortexJointAxisFrictionItem,_vortexJointAxisFrictionExpanded);

    p_vortexJointAxisFrictionEnabled = variantManager->addProperty(QVariant::Bool,"");
    vortexJointAxisFriction->addSubProperty(p_vortexJointAxisFrictionEnabled);

    p_vortexJointAxisFrictionProportional = variantManager->addProperty(QVariant::Bool,"");
    vortexJointAxisFriction->addSubProperty(p_vortexJointAxisFrictionProportional);

    p_vortexJointAxisFrictionCoefficient = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisFriction->addSubProperty(p_vortexJointAxisFrictionCoefficient);

    p_vortexJointAxisFrictionMaxForce = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisFriction->addSubProperty(p_vortexJointAxisFrictionMaxForce);

    p_vortexJointAxisFrictionLoss = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisFriction->addSubProperty(p_vortexJointAxisFrictionLoss);



    QtProperty *vortexJointAxisLimits = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Joint axis limits");
    vortexGroup->addSubProperty(vortexJointAxisLimits);
    vortexJointAxisLimitsItem=getSubPropertyBrowserItem(vortexGroupItem,vortexJointAxisLimits);
    setExpanded(vortexJointAxisLimitsItem,_vortexJointAxisLimitsExpanded);

    p_vortexJointAxisLimitLowerRestitution = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitLowerRestitution);
    p_vortexJointAxisLimitUpperRestitution = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitUpperRestitution);

    p_vortexJointAxisLimitLowerStiffness = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitLowerStiffness);
    p_vortexJointAxisLimitUpperStiffness = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitUpperStiffness);

    p_vortexJointAxisLimitLowerDamping = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitLowerDamping);
    p_vortexJointAxisLimitUpperDamping = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitUpperDamping);

    p_vortexJointAxisLimitLowerMaxForce = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitLowerMaxForce);
    p_vortexJointAxisLimitUpperMaxForce = variantManager->addProperty(QVariant::String,"");
    vortexJointAxisLimits->addSubProperty(p_vortexJointAxisLimitUpperMaxForce);



    QtProperty *vortexJointDependency = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Joint dependency");
    vortexGroup->addSubProperty(vortexJointDependency);
    vortexJointDependencyItem=getSubPropertyBrowserItem(vortexGroupItem,vortexJointDependency);
    setExpanded(vortexJointDependencyItem,_vortexJointDependencyExpanded);



    CJoint* it=App::ct->objCont->getLastSelection_joint();
    QStringList jointNamesEnum;
    jointNamesEnum << "None";
    std::vector<std::string> names;
    std::vector<int> ids;
    for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
    {
        CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
        if ( (it2!=it)&&(it2->getVortexDependentJointId()!=it->getObjectHandle())&&(it2->getJointType()!=sim_joint_spherical_subtype) )
        {
            names.push_back(it2->getObjectName());
            ids.push_back(it2->getObjectHandle());
        }
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        jointNamesEnum << names[i].c_str();

    p_vortexJointDependencyJoint = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_vortexJointDependencyJoint->setAttribute("enumNames", jointNamesEnum);
    vortexJointDependency->addSubProperty(p_vortexJointDependencyJoint);
    p_vortexJointDependencyFact = variantManager->addProperty(QVariant::String,"");
    vortexJointDependency->addSubProperty(p_vortexJointDependencyFact);
    p_vortexJointDependencyOff = variantManager->addProperty(QVariant::String,"");
    vortexJointDependency->addSubProperty(p_vortexJointDependencyOff);



    vortexP0 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexP0);
    vortexP0Item=getSubPropertyBrowserItem(vortexGroupItem,vortexP0);
    setExpanded(vortexP0Item,_vortexP0Expanded);


    vortexRelaxationP0 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexP0->addSubProperty(vortexRelaxationP0);
    vortexP0RelaxationItem=getSubPropertyBrowserItem(vortexP0Item,vortexRelaxationP0);
    setExpanded(vortexP0RelaxationItem,_vortexP0RelaxationExpanded);

    p_vortexRelaxationP0Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexRelaxationP0->addSubProperty(p_vortexRelaxationP0Enabled);

    p_vortexRelaxationP0Stiffness = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP0->addSubProperty(p_vortexRelaxationP0Stiffness);

    p_vortexRelaxationP0Damping = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP0->addSubProperty(p_vortexRelaxationP0Damping);

    p_vortexRelaxationP0Loss = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP0->addSubProperty(p_vortexRelaxationP0Loss);


    vortexFrictionP0 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexP0->addSubProperty(vortexFrictionP0);
    vortexP0FrictionItem=getSubPropertyBrowserItem(vortexP0Item,vortexFrictionP0);
    setExpanded(vortexP0FrictionItem,_vortexP0FrictionExpanded);

    p_vortexFrictionP0Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionP0->addSubProperty(p_vortexFrictionP0Enabled);

    p_vortexFrictionP0Proportional = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionP0->addSubProperty(p_vortexFrictionP0Proportional);

    p_vortexFrictionP0Coefficient = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP0->addSubProperty(p_vortexFrictionP0Coefficient);

    p_vortexFrictionP0MaxForce = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP0->addSubProperty(p_vortexFrictionP0MaxForce);

    p_vortexFrictionP0Loss = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP0->addSubProperty(p_vortexFrictionP0Loss);




    vortexP1 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexP1);
    vortexP1Item=getSubPropertyBrowserItem(vortexGroupItem,vortexP1);
    setExpanded(vortexP1Item,_vortexP1Expanded);


    vortexRelaxationP1 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexP1->addSubProperty(vortexRelaxationP1);
    vortexP1RelaxationItem=getSubPropertyBrowserItem(vortexP1Item,vortexRelaxationP1);
    setExpanded(vortexP1RelaxationItem,_vortexP1RelaxationExpanded);

    p_vortexRelaxationP1Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexRelaxationP1->addSubProperty(p_vortexRelaxationP1Enabled);

    p_vortexRelaxationP1Stiffness = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP1->addSubProperty(p_vortexRelaxationP1Stiffness);

    p_vortexRelaxationP1Damping = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP1->addSubProperty(p_vortexRelaxationP1Damping);

    p_vortexRelaxationP1Loss = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP1->addSubProperty(p_vortexRelaxationP1Loss);


    vortexFrictionP1 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexP1->addSubProperty(vortexFrictionP1);
    vortexP1FrictionItem=getSubPropertyBrowserItem(vortexP1Item,vortexFrictionP1);
    setExpanded(vortexP1FrictionItem,_vortexP1FrictionExpanded);

    p_vortexFrictionP1Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionP1->addSubProperty(p_vortexFrictionP1Enabled);

    p_vortexFrictionP1Proportional = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionP1->addSubProperty(p_vortexFrictionP1Proportional);

    p_vortexFrictionP1Coefficient = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP1->addSubProperty(p_vortexFrictionP1Coefficient);

    p_vortexFrictionP1MaxForce = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP1->addSubProperty(p_vortexFrictionP1MaxForce);

    p_vortexFrictionP1Loss = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP1->addSubProperty(p_vortexFrictionP1Loss);



    vortexP2 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexP2);
    vortexP2Item=getSubPropertyBrowserItem(vortexGroupItem,vortexP2);
    setExpanded(vortexP2Item,_vortexP2Expanded);


    vortexRelaxationP2 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexP2->addSubProperty(vortexRelaxationP2);
    vortexP2RelaxationItem=getSubPropertyBrowserItem(vortexP2Item,vortexRelaxationP2);
    setExpanded(vortexP2RelaxationItem,_vortexP2RelaxationExpanded);

    p_vortexRelaxationP2Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexRelaxationP2->addSubProperty(p_vortexRelaxationP2Enabled);

    p_vortexRelaxationP2Stiffness = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP2->addSubProperty(p_vortexRelaxationP2Stiffness);

    p_vortexRelaxationP2Damping = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP2->addSubProperty(p_vortexRelaxationP2Damping);

    p_vortexRelaxationP2Loss = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationP2->addSubProperty(p_vortexRelaxationP2Loss);


    vortexFrictionP2 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexP2->addSubProperty(vortexFrictionP2);
    vortexP2FrictionItem=getSubPropertyBrowserItem(vortexP2Item,vortexFrictionP2);
    setExpanded(vortexP2FrictionItem,_vortexP2FrictionExpanded);

    p_vortexFrictionP2Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionP2->addSubProperty(p_vortexFrictionP2Enabled);

    p_vortexFrictionP2Proportional = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionP2->addSubProperty(p_vortexFrictionP2Proportional);

    p_vortexFrictionP2Coefficient = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP2->addSubProperty(p_vortexFrictionP2Coefficient);

    p_vortexFrictionP2MaxForce = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP2->addSubProperty(p_vortexFrictionP2MaxForce);

    p_vortexFrictionP2Loss = variantManager->addProperty(QVariant::String,"");
    vortexFrictionP2->addSubProperty(p_vortexFrictionP2Loss);






    vortexA0 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexA0);
    vortexA0Item=getSubPropertyBrowserItem(vortexGroupItem,vortexA0);
    setExpanded(vortexA0Item,_vortexA0Expanded);

    vortexRelaxationA0 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexA0->addSubProperty(vortexRelaxationA0);
    vortexA0RelaxationItem=getSubPropertyBrowserItem(vortexA0Item,vortexRelaxationA0);
    setExpanded(vortexA0RelaxationItem,_vortexA0RelaxationExpanded);

    p_vortexRelaxationA0Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexRelaxationA0->addSubProperty(p_vortexRelaxationA0Enabled);

    p_vortexRelaxationA0Stiffness = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA0->addSubProperty(p_vortexRelaxationA0Stiffness);

    p_vortexRelaxationA0Damping = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA0->addSubProperty(p_vortexRelaxationA0Damping);

    p_vortexRelaxationA0Loss = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA0->addSubProperty(p_vortexRelaxationA0Loss);


    vortexFrictionA0 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexA0->addSubProperty(vortexFrictionA0);
    vortexA0FrictionItem=getSubPropertyBrowserItem(vortexA0Item,vortexFrictionA0);
    setExpanded(vortexA0FrictionItem,_vortexA0FrictionExpanded);

    p_vortexFrictionA0Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionA0->addSubProperty(p_vortexFrictionA0Enabled);

    p_vortexFrictionA0Proportional = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionA0->addSubProperty(p_vortexFrictionA0Proportional);

    p_vortexFrictionA0Coefficient = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA0->addSubProperty(p_vortexFrictionA0Coefficient);

    p_vortexFrictionA0MaxForce = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA0->addSubProperty(p_vortexFrictionA0MaxForce);

    p_vortexFrictionA0Loss = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA0->addSubProperty(p_vortexFrictionA0Loss);


    vortexA1 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexA1);
    vortexA1Item=getSubPropertyBrowserItem(vortexGroupItem,vortexA1);
    setExpanded(vortexA1Item,_vortexA1Expanded);


    vortexRelaxationA1 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexA1->addSubProperty(vortexRelaxationA1);
    vortexA1RelaxationItem=getSubPropertyBrowserItem(vortexA1Item,vortexRelaxationA1);
    setExpanded(vortexA1RelaxationItem,_vortexA1RelaxationExpanded);

    p_vortexRelaxationA1Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexRelaxationA1->addSubProperty(p_vortexRelaxationA1Enabled);

    p_vortexRelaxationA1Stiffness = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA1->addSubProperty(p_vortexRelaxationA1Stiffness);

    p_vortexRelaxationA1Damping = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA1->addSubProperty(p_vortexRelaxationA1Damping);

    p_vortexRelaxationA1Loss = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA1->addSubProperty(p_vortexRelaxationA1Loss);


    vortexFrictionA1 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexA1->addSubProperty(vortexFrictionA1);
    vortexA1FrictionItem=getSubPropertyBrowserItem(vortexA1Item,vortexFrictionA1);
    setExpanded(vortexA1FrictionItem,_vortexA1FrictionExpanded);

    p_vortexFrictionA1Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionA1->addSubProperty(p_vortexFrictionA1Enabled);

    p_vortexFrictionA1Proportional = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionA1->addSubProperty(p_vortexFrictionA1Proportional);

    p_vortexFrictionA1Coefficient = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA1->addSubProperty(p_vortexFrictionA1Coefficient);

    p_vortexFrictionA1MaxForce = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA1->addSubProperty(p_vortexFrictionA1MaxForce);

    p_vortexFrictionA1Loss = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA1->addSubProperty(p_vortexFrictionA1Loss);



    vortexA2 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexGroup->addSubProperty(vortexA2);
    vortexA2Item=getSubPropertyBrowserItem(vortexGroupItem,vortexA2);
    setExpanded(vortexA2Item,_vortexA2Expanded);


    vortexRelaxationA2 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexA2->addSubProperty(vortexRelaxationA2);
    vortexA2RelaxationItem=getSubPropertyBrowserItem(vortexA2Item,vortexRelaxationA2);
    setExpanded(vortexA2RelaxationItem,_vortexA2RelaxationExpanded);

    p_vortexRelaxationA2Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexRelaxationA2->addSubProperty(p_vortexRelaxationA2Enabled);

    p_vortexRelaxationA2Stiffness = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA2->addSubProperty(p_vortexRelaxationA2Stiffness);

    p_vortexRelaxationA2Damping = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA2->addSubProperty(p_vortexRelaxationA2Damping);

    p_vortexRelaxationA2Loss = variantManager->addProperty(QVariant::String,"");
    vortexRelaxationA2->addSubProperty(p_vortexRelaxationA2Loss);


    vortexFrictionA2 = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"");
    vortexA2->addSubProperty(vortexFrictionA2);
    vortexA2FrictionItem=getSubPropertyBrowserItem(vortexA2Item,vortexFrictionA2);
    setExpanded(vortexA2FrictionItem,_vortexA2FrictionExpanded);

    p_vortexFrictionA2Enabled = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionA2->addSubProperty(p_vortexFrictionA2Enabled);

    p_vortexFrictionA2Proportional = variantManager->addProperty(QVariant::Bool,"");
    vortexFrictionA2->addSubProperty(p_vortexFrictionA2Proportional);

    p_vortexFrictionA2Coefficient = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA2->addSubProperty(p_vortexFrictionA2Coefficient);

    p_vortexFrictionA2MaxForce = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA2->addSubProperty(p_vortexFrictionA2MaxForce);

    p_vortexFrictionA2Loss = variantManager->addProperty(QVariant::String,"");
    vortexFrictionA2->addSubProperty(p_vortexFrictionA2Loss);




    // Newton properties:
    QtProperty *newtonJointDependency = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),"Joint dependency");
    newtonGroup->addSubProperty(newtonJointDependency);
    newtonJointDependencyItem=getSubPropertyBrowserItem(newtonGroupItem,newtonJointDependency);
    setExpanded(newtonJointDependencyItem,_newtonJointDependencyExpanded);

    jointNamesEnum.clear();
    names.clear();
    ids.clear();
    jointNamesEnum << "None";
    for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
    {
        CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
        if ( (it2!=it)&&(it2->getNewtonDependentJointId()!=it->getObjectHandle())&&(it2->getJointType()!=sim_joint_spherical_subtype) )
        {
            names.push_back(it2->getObjectName());
            ids.push_back(it2->getObjectHandle());
        }
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        jointNamesEnum << names[i].c_str();

    p_newtonJointDependencyJoint = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(),"");
    p_newtonJointDependencyJoint->setAttribute("enumNames", jointNamesEnum);
    newtonJointDependency->addSubProperty(p_newtonJointDependencyJoint);
    p_newtonJointDependencyFact = variantManager->addProperty(QVariant::String,"");
    newtonJointDependency->addSubProperty(p_newtonJointDependencyFact);
    p_newtonJointDependencyOff = variantManager->addProperty(QVariant::String,"");
    newtonJointDependency->addSubProperty(p_newtonJointDependencyOff);

    refresh();

    enableNotifications(true);

//  setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint);
//  setWindowModality(Qt::ApplicationModal);
    setGeometry(_dlgPosX,_dlgPosY,_dlgSizeX,_dlgSizeY);
}

void CPropBrowserEngineJoint::enableNotifications(bool enable)
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

void CPropBrowserEngineJoint::refresh()
{
    CJoint* it=App::ct->objCont->getLastSelection_joint();

    // Apply all button:
    p_applyAllButton->setEnabled(App::ct->objCont->getJointNumberInSelection()>1);

    // Bullet property names:
    p_bulletNormalCFM->setPropertyName("Normal CFM");
    p_bulletStopERP->setPropertyName("Stop ERP");
    p_bulletStopCFM->setPropertyName("Stop CFM");

    // Bullet parameters:
    p_bulletNormalCFM->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr),false).c_str());
    p_bulletStopERP->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_bullet_joint_stoperp,nullptr),false).c_str());
    p_bulletStopCFM->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr),false).c_str());

    // ODE property names:
    p_odeNormalCFM->setPropertyName("Normal CFM");
    p_odeStopERP->setPropertyName("Stop ERP");
    p_odeStopCFM->setPropertyName("Stop CFM");
    p_odeBounce->setPropertyName("Bounce");
    p_odeFudgeFactor->setPropertyName("Fudge factor");

    // ODE parameters:
    p_odeNormalCFM->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_ode_joint_normalcfm,nullptr),false).c_str());
    p_odeStopERP->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_ode_joint_stoperp,nullptr),false).c_str());
    p_odeStopCFM->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_ode_joint_stopcfm,nullptr),false).c_str());
    p_odeBounce->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_ode_joint_bounce,nullptr),false).c_str());
    p_odeFudgeFactor->setValue(tt::floatToEInfString(it->getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr),false).c_str());

    // Vortex property names:
    p_vortexJointAxisFrictionEnabled->setPropertyName("Enabled");
    p_vortexJointAxisFrictionProportional->setPropertyName("Proportional");
    p_vortexJointAxisFrictionCoefficient->setPropertyName("Coefficient");
    if (it->getJointType()==sim_joint_prismatic_subtype)
    {
        p_vortexJointAxisFrictionMaxForce->setPropertyName(gv::formatUnitStr("Maximum force","k*m/s^2").c_str());
        p_vortexJointAxisFrictionLoss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());
    }
    else
    {
        p_vortexJointAxisFrictionMaxForce->setPropertyName(gv::formatUnitStr("Maximum torque","k*m^2/s^2").c_str());
        p_vortexJointAxisFrictionLoss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());
    }

    p_vortexJointAxisLimitLowerRestitution->setPropertyName("Lower restitution");
    p_vortexJointAxisLimitUpperRestitution->setPropertyName("Upper restitution");
    if (it->getJointType()==sim_joint_prismatic_subtype)
    {
        p_vortexJointAxisLimitLowerStiffness->setPropertyName(gv::formatUnitStr("Lower stiffness","k/s^2").c_str());
        p_vortexJointAxisLimitUpperStiffness->setPropertyName(gv::formatUnitStr("Upper stiffness","k/s^2").c_str());
        p_vortexJointAxisLimitLowerDamping->setPropertyName(gv::formatUnitStr("Lower damping","k/s").c_str());
        p_vortexJointAxisLimitUpperDamping->setPropertyName(gv::formatUnitStr("Upper damping","k/s").c_str());
        p_vortexJointAxisLimitLowerMaxForce->setPropertyName(gv::formatUnitStr("Lower maximum force","k*m/s^2").c_str());
        p_vortexJointAxisLimitUpperMaxForce->setPropertyName(gv::formatUnitStr("Upper maximum force","k*m/s^2").c_str());
    }
    else
    {
        p_vortexJointAxisLimitLowerStiffness->setPropertyName(gv::formatUnitStr("Lower stiffness","k*m/s^2").c_str());
        p_vortexJointAxisLimitUpperStiffness->setPropertyName(gv::formatUnitStr("Upper stiffness","k*m/s^2").c_str());
        p_vortexJointAxisLimitLowerDamping->setPropertyName(gv::formatUnitStr("Lower damping","k*m/s").c_str());
        p_vortexJointAxisLimitUpperDamping->setPropertyName(gv::formatUnitStr("Upper damping","k*m/s").c_str());
        p_vortexJointAxisLimitLowerMaxForce->setPropertyName(gv::formatUnitStr("Lower maximum torque","k*m^2/s^2").c_str());
        p_vortexJointAxisLimitUpperMaxForce->setPropertyName(gv::formatUnitStr("Upper maximum torque","k*m^2/s^2").c_str());
    }


    p_vortexJointDependencyJoint->setPropertyName("Dependent joint");
    p_vortexJointDependencyFact->setPropertyName("Multiplication factor");
    p_vortexJointDependencyOff->setPropertyName("Offset");


    p_vortexRelaxationP0Enabled->setPropertyName("Enabled");
    p_vortexRelaxationP0Stiffness->setPropertyName(gv::formatUnitStr("Stiffness","k/s^2").c_str());
    p_vortexRelaxationP0Damping->setPropertyName(gv::formatUnitStr("Damping","k/s").c_str());
    p_vortexRelaxationP0Loss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());
    p_vortexFrictionP0Enabled->setPropertyName("Enabled");
    p_vortexFrictionP0Proportional->setPropertyName("Proportional");
    p_vortexFrictionP0Coefficient->setPropertyName("Coefficient");
    p_vortexFrictionP0MaxForce->setPropertyName(gv::formatUnitStr("Maximum force","k*m/s^2").c_str());
    p_vortexFrictionP0Loss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());

    p_vortexRelaxationP1Enabled->setPropertyName("Enabled");
    p_vortexRelaxationP1Stiffness->setPropertyName(gv::formatUnitStr("Stiffness","k/s^2").c_str());
    p_vortexRelaxationP1Damping->setPropertyName(gv::formatUnitStr("Damping","k/s").c_str());
    p_vortexRelaxationP1Loss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());
    p_vortexFrictionP1Enabled->setPropertyName("Enabled");
    p_vortexFrictionP1Proportional->setPropertyName("Proportional");
    p_vortexFrictionP1Coefficient->setPropertyName("Coefficient");
    p_vortexFrictionP1MaxForce->setPropertyName(gv::formatUnitStr("Maximum force","k*m/s^2").c_str());
    p_vortexFrictionP1Loss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());

    p_vortexRelaxationP2Enabled->setPropertyName("Enabled");
    p_vortexRelaxationP2Stiffness->setPropertyName(gv::formatUnitStr("Stiffness","k/s^2").c_str());
    p_vortexRelaxationP2Damping->setPropertyName(gv::formatUnitStr("Damping","k/s").c_str());
    p_vortexRelaxationP2Loss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());
    p_vortexFrictionP2Enabled->setPropertyName("Enabled");
    p_vortexFrictionP2Proportional->setPropertyName("Proportional");
    p_vortexFrictionP2Coefficient->setPropertyName("Coefficient");
    p_vortexFrictionP2MaxForce->setPropertyName(gv::formatUnitStr("Maximum force","k*m/s^2").c_str());
    p_vortexFrictionP2Loss->setPropertyName(gv::formatUnitStr("Loss","s/k").c_str());

    p_vortexRelaxationA0Enabled->setPropertyName("Enabled");
    p_vortexRelaxationA0Stiffness->setPropertyName(gv::formatUnitStr("Stiffness","k*m/s^2").c_str());
    p_vortexRelaxationA0Damping->setPropertyName(gv::formatUnitStr("Damping","k*m/s").c_str());
    p_vortexRelaxationA0Loss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());
    p_vortexFrictionA0Enabled->setPropertyName("Enabled");
    p_vortexFrictionA0Proportional->setPropertyName("Proportional");
    p_vortexFrictionA0Coefficient->setPropertyName("Coefficient");
    p_vortexFrictionA0MaxForce->setPropertyName(gv::formatUnitStr("Maximum torque","k*m^2/s^2").c_str());
    p_vortexFrictionA0Loss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());

    p_vortexRelaxationA1Enabled->setPropertyName("Enabled");
    p_vortexRelaxationA1Stiffness->setPropertyName(gv::formatUnitStr("Stiffness","k*m/s^2").c_str());
    p_vortexRelaxationA1Damping->setPropertyName(gv::formatUnitStr("Damping","k*m/s").c_str());
    p_vortexRelaxationA1Loss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());
    p_vortexFrictionA1Enabled->setPropertyName("Enabled");
    p_vortexFrictionA1Proportional->setPropertyName("Proportional");
    p_vortexFrictionA1Coefficient->setPropertyName("Coefficient");
    p_vortexFrictionA1MaxForce->setPropertyName(gv::formatUnitStr("Maximum torque","k*m^2/s^2").c_str());
    p_vortexFrictionA1Loss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());

    p_vortexRelaxationA2Enabled->setPropertyName("Enabled");
    p_vortexRelaxationA2Stiffness->setPropertyName(gv::formatUnitStr("Stiffness","k*m/s^2").c_str());
    p_vortexRelaxationA2Damping->setPropertyName(gv::formatUnitStr("Damping","k*m/s").c_str());
    p_vortexRelaxationA2Loss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());
    p_vortexFrictionA2Enabled->setPropertyName("Enabled");
    p_vortexFrictionA2Proportional->setPropertyName("Proportional");
    p_vortexFrictionA2Coefficient->setPropertyName("Coefficient");
    p_vortexFrictionA2MaxForce->setPropertyName(gv::formatUnitStr("Maximum torque","k*m^2/s^2").c_str());
    p_vortexFrictionA2Loss->setPropertyName(gv::formatUnitStr("Loss","s/(k*m^2)").c_str());


    // Vortex parameters:

    float limit_lower_damping=it->getEngineFloatParam(sim_vortex_joint_lowerlimitdamping,nullptr);
    float limit_upper_damping=it->getEngineFloatParam(sim_vortex_joint_upperlimitdamping,nullptr);
    float limit_lower_stiffness=it->getEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,nullptr);
    float limit_upper_stiffness=it->getEngineFloatParam(sim_vortex_joint_upperlimitstiffness,nullptr);
    float limit_lower_restitution=it->getEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,nullptr);
    float limit_upper_restitution=it->getEngineFloatParam(sim_vortex_joint_upperlimitrestitution,nullptr);
    float limit_lower_maxForce=it->getEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,nullptr);
    float limit_upper_maxForce=it->getEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,nullptr);
    float motorConstraint_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,nullptr);
    float motorConstraint_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,nullptr);
    float motorConstraint_friction_loss=it->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,nullptr);
    float P0_loss=it->getEngineFloatParam(sim_vortex_joint_p0loss,nullptr);
    float P0_stiffness=it->getEngineFloatParam(sim_vortex_joint_p0stiffness,nullptr);
    float P0_damping=it->getEngineFloatParam(sim_vortex_joint_p0damping,nullptr);
    float P0_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_p0frictioncoeff,nullptr);
    float P0_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,nullptr);
    float P0_friction_loss=it->getEngineFloatParam(sim_vortex_joint_p0frictionloss,nullptr);
    float P1_loss=it->getEngineFloatParam(sim_vortex_joint_p1loss,nullptr);
    float P1_stiffness=it->getEngineFloatParam(sim_vortex_joint_p1stiffness,nullptr);
    float P1_damping=it->getEngineFloatParam(sim_vortex_joint_p1damping,nullptr);
    float P1_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_p1frictioncoeff,nullptr);
    float P1_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,nullptr);
    float P1_friction_loss=it->getEngineFloatParam(sim_vortex_joint_p1frictionloss,nullptr);
    float P2_loss=it->getEngineFloatParam(sim_vortex_joint_p2loss,nullptr);
    float P2_stiffness=it->getEngineFloatParam(sim_vortex_joint_p2stiffness,nullptr);
    float P2_damping=it->getEngineFloatParam(sim_vortex_joint_p2damping,nullptr);
    float P2_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_p2frictioncoeff,nullptr);
    float P2_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,nullptr);
    float P2_friction_loss=it->getEngineFloatParam(sim_vortex_joint_p2frictionloss,nullptr);
    float A0_loss=it->getEngineFloatParam(sim_vortex_joint_a0loss,nullptr);
    float A0_stiffness=it->getEngineFloatParam(sim_vortex_joint_a0stiffness,nullptr);
    float A0_damping=it->getEngineFloatParam(sim_vortex_joint_a0damping,nullptr);
    float A0_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_a0frictioncoeff,nullptr);
    float A0_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,nullptr);
    float A0_friction_loss=it->getEngineFloatParam(sim_vortex_joint_a0frictionloss,nullptr);
    float A1_loss=it->getEngineFloatParam(sim_vortex_joint_a1loss,nullptr);
    float A1_stiffness=it->getEngineFloatParam(sim_vortex_joint_a1stiffness,nullptr);
    float A1_damping=it->getEngineFloatParam(sim_vortex_joint_a1damping,nullptr);
    float A1_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_a1frictioncoeff,nullptr);
    float A1_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,nullptr);
    float A1_friction_loss=it->getEngineFloatParam(sim_vortex_joint_a1frictionloss,nullptr);
    float A2_loss=it->getEngineFloatParam(sim_vortex_joint_a2loss,nullptr);
    float A2_stiffness=it->getEngineFloatParam(sim_vortex_joint_a2stiffness,nullptr);
    float A2_damping=it->getEngineFloatParam(sim_vortex_joint_a2damping,nullptr);
    float A2_friction_coeff=it->getEngineFloatParam(sim_vortex_joint_a2frictioncoeff,nullptr);
    float A2_friction_maxForce=it->getEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,nullptr);
    float A2_friction_loss=it->getEngineFloatParam(sim_vortex_joint_a2frictionloss,nullptr);
    float dependentJointFact=it->getEngineFloatParam(sim_vortex_joint_dependencyfactor,nullptr);
    float dependentJointOff=it->getEngineFloatParam(sim_vortex_joint_dependencyoffset,nullptr);

    bool motorFrictionEnabled=it->getEngineBoolParam(sim_vortex_joint_motorfrictionenabled,nullptr);
    bool motorFrictionProportional=it->getEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,nullptr);
    int vval;
    vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
    bool P0_relaxation_enabled=((vval&1)!=0);
    bool P1_relaxation_enabled=((vval&2)!=0);
    bool P2_relaxation_enabled=((vval&4)!=0);
    bool A0_relaxation_enabled=((vval&8)!=0);
    bool A1_relaxation_enabled=((vval&16)!=0);
    bool A2_relaxation_enabled=((vval&32)!=0);
    vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
    bool P0_friction_enabled=((vval&1)!=0);
    bool P1_friction_enabled=((vval&2)!=0);
    bool P2_friction_enabled=((vval&4)!=0);
    bool A0_friction_enabled=((vval&8)!=0);
    bool A1_friction_enabled=((vval&16)!=0);
    bool A2_friction_enabled=((vval&32)!=0);
    vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
    bool P0_friction_proportional=((vval&1)!=0);
    bool P1_friction_proportional=((vval&2)!=0);
    bool P2_friction_proportional=((vval&4)!=0);
    bool A0_friction_proportional=((vval&8)!=0);
    bool A1_friction_proportional=((vval&16)!=0);
    bool A2_friction_proportional=((vval&32)!=0);

    int dependentJointId=it->getEngineIntParam(sim_vortex_joint_dependentobjectid,nullptr);

    std::string jointAxisFrictionText("Joint axis friction");
    std::string xAxisPositionText("X axis position");
    std::string xAxisPRelaxationText("Relaxation");
    std::string xAxisPFrictionText("Friction");
    std::string yAxisPositionText("Y axis position");
    std::string yAxisPRelaxationText("Relaxation");
    std::string yAxisPFrictionText("Friction");
    std::string zAxisPositionText("Z axis position");
    std::string zAxisPRelaxationText("Relaxation");
    std::string zAxisPFrictionText("Friction");
    std::string xAxisOrientationText("X axis orientation");
    std::string xAxisARelaxationText("Relaxation");
    std::string xAxisAFrictionText("Friction");
    std::string yAxisOrientationText("Y axis orientation");
    std::string yAxisARelaxationText("Relaxation");
    std::string yAxisAFrictionText("Friction");
    std::string zAxisOrientationText("Z axis orientation");
    std::string zAxisARelaxationText("Relaxation");
    std::string zAxisAFrictionText("Friction");


    std::vector<std::string> depNames;
    std::vector<int> depIds;
    for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
    {
        CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
        if ( (it2!=it)&&(it2->getVortexDependentJointId()!=it->getObjectHandle())&&(it2->getJointType()!=sim_joint_spherical_subtype) )
        {
            depNames.push_back(it2->getObjectName());
            depIds.push_back(it2->getObjectHandle());
        }
    }
    tt::orderStrings(depNames,depIds);


    if (it->getJointType()!=sim_joint_spherical_subtype)
    {
        if (motorFrictionEnabled)
            jointAxisFrictionText+=" (on)";
        else
            jointAxisFrictionText+=" (off)";


        p_vortexJointAxisFrictionEnabled->setValue(motorFrictionEnabled);
        p_vortexJointAxisFrictionEnabled->setEnabled(true);
        p_vortexJointAxisFrictionProportional->setValue(motorFrictionProportional);
        p_vortexJointAxisFrictionProportional->setEnabled(motorFrictionEnabled);
        p_vortexJointAxisFrictionCoefficient->setValue(tt::floatToEInfString(motorConstraint_friction_coeff,false).c_str());
        p_vortexJointAxisFrictionCoefficient->setEnabled(motorFrictionEnabled&&motorFrictionProportional);
        if (it->getJointType()==sim_joint_prismatic_subtype)
        {
            p_vortexJointAxisFrictionMaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(motorConstraint_friction_maxForce,1.0f,true),false).c_str());
            p_vortexJointAxisFrictionLoss->setValue(tt::floatToEInfString(tt::floatToUserFloat(motorConstraint_friction_loss,1.0f,true),false).c_str());
        }
        else
        {
            p_vortexJointAxisFrictionMaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(motorConstraint_friction_maxForce,1.0f,true),false).c_str());
            p_vortexJointAxisFrictionLoss->setValue(tt::floatToEInfString(tt::floatToUserFloat(motorConstraint_friction_loss,1.0f,true),false).c_str());
        }
        p_vortexJointAxisFrictionMaxForce->setEnabled(motorFrictionEnabled&&(!motorFrictionProportional));
        p_vortexJointAxisFrictionLoss->setEnabled(motorFrictionEnabled);


        p_vortexJointAxisLimitLowerRestitution->setValue(tt::floatToEInfString(limit_lower_restitution,false).c_str());
        p_vortexJointAxisLimitLowerRestitution->setEnabled(true);
        p_vortexJointAxisLimitUpperRestitution->setValue(tt::floatToEInfString(limit_upper_restitution,false).c_str());
        p_vortexJointAxisLimitUpperRestitution->setEnabled(true);
        if (it->getJointType()==sim_joint_prismatic_subtype)
        {
            p_vortexJointAxisLimitLowerStiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_lower_stiffness,1.0f,true),false).c_str());
            p_vortexJointAxisLimitUpperStiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_upper_stiffness,1.0f,true),false).c_str());
            p_vortexJointAxisLimitLowerDamping->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_lower_damping,1.0f,true),false).c_str());
            p_vortexJointAxisLimitUpperDamping->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_upper_damping,1.0f,true),false).c_str());
            p_vortexJointAxisLimitLowerMaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_lower_maxForce,1.0f,true),false).c_str());
            p_vortexJointAxisLimitUpperMaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_upper_maxForce,1.0f,true),false).c_str());
        }
        else
        {
            p_vortexJointAxisLimitLowerStiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_lower_stiffness,1.0f,true),false).c_str());
            p_vortexJointAxisLimitUpperStiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_upper_stiffness,1.0f,true),false).c_str());
            p_vortexJointAxisLimitLowerDamping->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_lower_damping,1.0f,true),false).c_str());
            p_vortexJointAxisLimitUpperDamping->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_upper_damping,1.0f,true),false).c_str());
            p_vortexJointAxisLimitLowerMaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_lower_maxForce,1.0f,true),false).c_str());
            p_vortexJointAxisLimitUpperMaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(limit_upper_maxForce,1.0f,true),false).c_str());
        }
        p_vortexJointAxisLimitLowerStiffness->setEnabled(true);
        p_vortexJointAxisLimitUpperStiffness->setEnabled(true);
        p_vortexJointAxisLimitLowerDamping->setEnabled(true);
        p_vortexJointAxisLimitUpperDamping->setEnabled(true);
        p_vortexJointAxisLimitLowerMaxForce->setEnabled(true);
        p_vortexJointAxisLimitUpperMaxForce->setEnabled(true);

        // find the correct index:
//      dependentJointId
        int depIndex=0; // this means "none"
        if (dependentJointId>=0)
        {
            CJoint* aJoint=App::ct->objCont->getJoint(dependentJointId);
            if (aJoint!=nullptr)
            {
                for (int i=0;i<int(depNames.size());i++)
                {
                    if (depNames[i].compare(aJoint->getObjectName())==0)
                    {
                        depIndex=i+1;
                        break;
                    }
                }
            }
        }
        p_vortexJointDependencyJoint->setValue(depIndex);
        p_vortexJointDependencyJoint->setEnabled(it->getJointType()!=sim_joint_spherical_subtype);
        p_vortexJointDependencyFact->setValue(tt::floatToEInfString(dependentJointFact,true).c_str());
        p_vortexJointDependencyFact->setEnabled(it->getJointType()!=sim_joint_spherical_subtype);
        p_vortexJointDependencyOff->setValue(tt::floatToEInfString(dependentJointOff,true).c_str());
        p_vortexJointDependencyOff->setEnabled(it->getJointType()!=sim_joint_spherical_subtype);

        p_vortexRelaxationA0Enabled->setValue(A0_relaxation_enabled);
        p_vortexRelaxationA0Enabled->setEnabled(true);
        p_vortexRelaxationA0Stiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(A0_stiffness,1.0f,true),false).c_str());
        p_vortexRelaxationA0Stiffness->setEnabled(A0_relaxation_enabled);
        p_vortexRelaxationA0Damping->setValue(tt::floatToEInfString(tt::floatToUserFloat(A0_damping,1.0f,true),false).c_str());
        p_vortexRelaxationA0Damping->setEnabled(A0_relaxation_enabled);
        p_vortexRelaxationA0Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(A0_loss,1.0f,true),false).c_str());
        p_vortexRelaxationA0Loss->setEnabled(A0_relaxation_enabled);
        p_vortexFrictionA0Enabled->setValue(A0_friction_enabled);
        p_vortexFrictionA0Enabled->setEnabled(true);
        p_vortexFrictionA0Proportional->setValue(A0_friction_proportional);
        p_vortexFrictionA0Proportional->setEnabled(A0_friction_enabled);
        p_vortexFrictionA0Coefficient->setValue(tt::floatToEInfString(A0_friction_coeff,false).c_str());
        p_vortexFrictionA0Coefficient->setEnabled(A0_friction_enabled&&A0_friction_proportional);
        p_vortexFrictionA0MaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(A0_friction_maxForce,1.0f,true),false).c_str());
        p_vortexFrictionA0MaxForce->setEnabled(A0_friction_enabled&&(!A0_friction_proportional));
        p_vortexFrictionA0Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(A0_friction_loss,1.0f,true),false).c_str());
        p_vortexFrictionA0Loss->setEnabled(A0_friction_enabled);

        if (A0_relaxation_enabled)
        {
            xAxisOrientationText+=" (relaxation: on";
            xAxisARelaxationText+=" (on)";
        }
        else
        {
            xAxisOrientationText+=" (relaxation: off";
            xAxisARelaxationText+=" (off)";
        }
        if (A0_friction_enabled)
        {
            xAxisOrientationText+=", friction: on)";
            xAxisAFrictionText+=" (on)";
        }
        else
        {
            xAxisOrientationText+=", friction: off)";
            xAxisAFrictionText+=" (off)";
        }

        p_vortexRelaxationA1Enabled->setValue(A1_relaxation_enabled);
        p_vortexRelaxationA1Enabled->setEnabled(true);
        p_vortexRelaxationA1Stiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(A1_stiffness,1.0f,true),false).c_str());
        p_vortexRelaxationA1Stiffness->setEnabled(A1_relaxation_enabled);
        p_vortexRelaxationA1Damping->setValue(tt::floatToEInfString(tt::floatToUserFloat(A1_damping,1.0f,true),false).c_str());
        p_vortexRelaxationA1Damping->setEnabled(A1_relaxation_enabled);
        p_vortexRelaxationA1Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(A1_loss,1.0f,true),false).c_str());
        p_vortexRelaxationA1Loss->setEnabled(A1_relaxation_enabled);
        p_vortexFrictionA1Enabled->setValue(A1_friction_enabled);
        p_vortexFrictionA1Enabled->setEnabled(true);
        p_vortexFrictionA1Proportional->setValue(A1_friction_proportional);
        p_vortexFrictionA1Proportional->setEnabled(A1_friction_enabled);
        p_vortexFrictionA1Coefficient->setValue(tt::floatToEInfString(A1_friction_coeff,false).c_str());
        p_vortexFrictionA1Coefficient->setEnabled(A1_friction_enabled&&A1_friction_proportional);
        p_vortexFrictionA1MaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(A1_friction_maxForce,1.0f,true),false).c_str());
        p_vortexFrictionA1MaxForce->setEnabled(A1_friction_enabled&&(!A1_friction_proportional));
        p_vortexFrictionA1Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(A1_friction_loss,1.0f,true),false).c_str());
        p_vortexFrictionA1Loss->setEnabled(A1_friction_enabled);

        if (A1_relaxation_enabled)
        {
            yAxisOrientationText+=" (relaxation: on";
            yAxisARelaxationText+=" (on)";
        }
        else
        {
            yAxisOrientationText+=" (relaxation: off";
            yAxisARelaxationText+=" (off)";
        }
        if (A1_friction_enabled)
        {
            yAxisOrientationText+=", friction: on)";
            yAxisAFrictionText+=" (on)";
        }
        else
        {
            yAxisOrientationText+=", friction: off)";
            yAxisAFrictionText+=" (off)";
        }

        if (it->getJointType()==sim_joint_prismatic_subtype)
        { // prismatic joints
            p_vortexRelaxationA2Enabled->setValue(A2_relaxation_enabled);
            p_vortexRelaxationA2Enabled->setEnabled(true);
            p_vortexRelaxationA2Stiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(A2_stiffness,1.0f,true),false).c_str());
            p_vortexRelaxationA2Stiffness->setEnabled(A2_relaxation_enabled);
            p_vortexRelaxationA2Damping->setValue(tt::floatToEInfString(tt::floatToUserFloat(A2_damping,1.0f,true),false).c_str());
            p_vortexRelaxationA2Damping->setEnabled(A2_relaxation_enabled);
            p_vortexRelaxationA2Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(A2_loss,1.0f,true),false).c_str());
            p_vortexRelaxationA2Loss->setEnabled(A2_relaxation_enabled);
            p_vortexFrictionA2Enabled->setValue(A2_friction_enabled);
            p_vortexFrictionA2Enabled->setEnabled(true);
            p_vortexFrictionA2Proportional->setValue(A2_friction_proportional);
            p_vortexFrictionA2Proportional->setEnabled(A2_friction_enabled);
            p_vortexFrictionA2Coefficient->setValue(tt::floatToEInfString(A2_friction_coeff,false).c_str());
            p_vortexFrictionA2Coefficient->setEnabled(A2_friction_enabled&&A2_friction_proportional);
            p_vortexFrictionA2MaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(A2_friction_maxForce,1.0f,true),false).c_str());
            p_vortexFrictionA2MaxForce->setEnabled(A2_friction_enabled&&(!A2_friction_proportional));
            p_vortexFrictionA2Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(A2_friction_loss,1.0f,true),false).c_str());
            p_vortexFrictionA2Loss->setEnabled(A2_friction_enabled);

            if (A2_relaxation_enabled)
            {
                zAxisOrientationText+=" (relaxation: on";
                zAxisARelaxationText+=" (on)";
            }
            else
            {
                zAxisOrientationText+=" (relaxation: off";
                zAxisARelaxationText+=" (off)";
            }
            if (A2_friction_enabled)
            {
                zAxisOrientationText+=", friction: on)";
                zAxisAFrictionText+=" (on)";
            }
            else
            {
                zAxisOrientationText+=", friction: off)";
                zAxisAFrictionText+=" (off)";
            }
        }
        else
        { // revolute joints
            p_vortexRelaxationA2Enabled->setValue("");
            p_vortexRelaxationA2Enabled->setEnabled(false);
            p_vortexRelaxationA2Stiffness->setValue("");
            p_vortexRelaxationA2Stiffness->setEnabled(false);
            p_vortexRelaxationA2Damping->setValue("");
            p_vortexRelaxationA2Damping->setEnabled(false);
            p_vortexRelaxationA2Loss->setValue("");
            p_vortexRelaxationA2Loss->setEnabled(false);
            p_vortexFrictionA2Enabled->setValue("");
            p_vortexFrictionA2Enabled->setEnabled(false);
            p_vortexFrictionA2Proportional->setValue("");
            p_vortexFrictionA2Proportional->setEnabled(false);
            p_vortexFrictionA2Coefficient->setValue("");
            p_vortexFrictionA2Coefficient->setEnabled(false);
            p_vortexFrictionA2MaxForce->setValue("");
            p_vortexFrictionA2MaxForce->setEnabled(false);
            p_vortexFrictionA2Loss->setValue("");
            p_vortexFrictionA2Loss->setEnabled(false);

            zAxisOrientationText+=" (n/a)";
            zAxisARelaxationText+=" (n/a)";
            zAxisAFrictionText+=" (n/a)";
        }
    }
    else
    {
        jointAxisFrictionText+=" (n/a)";

        p_vortexJointAxisFrictionEnabled->setValue(false);
        p_vortexJointAxisFrictionEnabled->setEnabled(false);
        p_vortexJointAxisFrictionProportional->setValue(false);
        p_vortexJointAxisFrictionProportional->setEnabled(false);
        p_vortexJointAxisFrictionCoefficient->setValue("");
        p_vortexJointAxisFrictionCoefficient->setEnabled(false);
        p_vortexJointAxisFrictionMaxForce->setValue("");
        p_vortexJointAxisFrictionMaxForce->setEnabled(false);
        p_vortexJointAxisFrictionLoss->setValue("");
        p_vortexJointAxisFrictionLoss->setEnabled(false);

        p_vortexJointAxisLimitLowerRestitution->setValue("");
        p_vortexJointAxisLimitLowerRestitution->setEnabled(false);
        p_vortexJointAxisLimitUpperRestitution->setValue("");
        p_vortexJointAxisLimitUpperRestitution->setEnabled(false);
        p_vortexJointAxisLimitLowerStiffness->setValue("");
        p_vortexJointAxisLimitLowerStiffness->setEnabled(false);
        p_vortexJointAxisLimitUpperStiffness->setValue("");
        p_vortexJointAxisLimitUpperStiffness->setEnabled(false);
        p_vortexJointAxisLimitLowerDamping->setValue("");
        p_vortexJointAxisLimitLowerDamping->setEnabled(false);
        p_vortexJointAxisLimitUpperDamping->setValue("");
        p_vortexJointAxisLimitUpperDamping->setEnabled(false);
        p_vortexJointAxisLimitLowerMaxForce->setValue("");
        p_vortexJointAxisLimitLowerMaxForce->setEnabled(false);
        p_vortexJointAxisLimitUpperMaxForce->setValue("");
        p_vortexJointAxisLimitUpperMaxForce->setEnabled(false);

        p_vortexJointDependencyJoint->setValue("");
        p_vortexJointDependencyJoint->setEnabled(false);
        p_vortexJointDependencyFact->setValue("");
        p_vortexJointDependencyFact->setEnabled(false);
        p_vortexJointDependencyOff->setValue("");
        p_vortexJointDependencyOff->setEnabled(false);

        p_vortexRelaxationA0Enabled->setValue("");
        p_vortexRelaxationA0Enabled->setEnabled(false);
        p_vortexRelaxationA0Stiffness->setValue("");
        p_vortexRelaxationA0Stiffness->setEnabled(false);
        p_vortexRelaxationA0Damping->setValue("");
        p_vortexRelaxationA0Damping->setEnabled(false);
        p_vortexRelaxationA0Loss->setValue("");
        p_vortexRelaxationA0Loss->setEnabled(false);
        p_vortexFrictionA0Enabled->setValue("");
        p_vortexFrictionA0Enabled->setEnabled(false);
        p_vortexFrictionA0Proportional->setValue("");
        p_vortexFrictionA0Proportional->setEnabled(false);
        p_vortexFrictionA0Coefficient->setValue("");
        p_vortexFrictionA0Coefficient->setEnabled(false);
        p_vortexFrictionA0MaxForce->setValue("");
        p_vortexFrictionA0MaxForce->setEnabled(false);
        p_vortexFrictionA0Loss->setValue("");
        p_vortexFrictionA0Loss->setEnabled(false);

        xAxisOrientationText+=" (n/a)";
        xAxisARelaxationText+=" (n/a)";
        xAxisAFrictionText+=" (n/a)";

        p_vortexRelaxationA1Enabled->setValue("");
        p_vortexRelaxationA1Enabled->setEnabled(false);
        p_vortexRelaxationA1Stiffness->setValue("");
        p_vortexRelaxationA1Stiffness->setEnabled(false);
        p_vortexRelaxationA1Damping->setValue("");
        p_vortexRelaxationA1Damping->setEnabled(false);
        p_vortexRelaxationA1Loss->setValue("");
        p_vortexRelaxationA1Loss->setEnabled(false);
        p_vortexFrictionA1Enabled->setValue("");
        p_vortexFrictionA1Enabled->setEnabled(false);
        p_vortexFrictionA1Proportional->setValue("");
        p_vortexFrictionA1Proportional->setEnabled(false);
        p_vortexFrictionA1Coefficient->setValue("");
        p_vortexFrictionA1Coefficient->setEnabled(false);
        p_vortexFrictionA1MaxForce->setValue("");
        p_vortexFrictionA1MaxForce->setEnabled(false);
        p_vortexFrictionA1Loss->setValue("");
        p_vortexFrictionA1Loss->setEnabled(false);

        yAxisOrientationText+=" (n/a)";
        yAxisARelaxationText+=" (n/a)";
        yAxisAFrictionText+=" (n/a)";

        p_vortexRelaxationA2Enabled->setValue("");
        p_vortexRelaxationA2Enabled->setEnabled(false);
        p_vortexRelaxationA2Stiffness->setValue("");
        p_vortexRelaxationA2Stiffness->setEnabled(false);
        p_vortexRelaxationA2Damping->setValue("");
        p_vortexRelaxationA2Damping->setEnabled(false);
        p_vortexRelaxationA2Loss->setValue("");
        p_vortexRelaxationA2Loss->setEnabled(false);
        p_vortexFrictionA2Enabled->setValue("");
        p_vortexFrictionA2Enabled->setEnabled(false);
        p_vortexFrictionA2Proportional->setValue("");
        p_vortexFrictionA2Proportional->setEnabled(false);
        p_vortexFrictionA2Coefficient->setValue("");
        p_vortexFrictionA2Coefficient->setEnabled(false);
        p_vortexFrictionA2MaxForce->setValue("");
        p_vortexFrictionA2MaxForce->setEnabled(false);
        p_vortexFrictionA2Loss->setValue("");
        p_vortexFrictionA2Loss->setEnabled(false);

        zAxisOrientationText+=" (n/a)";
        zAxisARelaxationText+=" (n/a)";
        zAxisAFrictionText+=" (n/a)";
    }

    p_vortexRelaxationP0Enabled->setValue(P0_relaxation_enabled);
    p_vortexRelaxationP0Enabled->setEnabled(true);
    p_vortexRelaxationP0Stiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(P0_stiffness,1.0f,true),false).c_str());
    p_vortexRelaxationP0Stiffness->setEnabled(P0_relaxation_enabled);
    p_vortexRelaxationP0Damping->setValue(tt::floatToEInfString(tt::floatToUserFloat(P0_damping,1.0f,true),false).c_str());
    p_vortexRelaxationP0Damping->setEnabled(P0_relaxation_enabled);
    p_vortexRelaxationP0Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(P0_loss,1.0f,true),false).c_str());
    p_vortexRelaxationP0Loss->setEnabled(P0_relaxation_enabled);
    p_vortexFrictionP0Enabled->setValue(P0_friction_enabled);
    p_vortexFrictionP0Enabled->setEnabled(true);
    p_vortexFrictionP0Proportional->setValue(P0_friction_proportional);
    p_vortexFrictionP0Proportional->setEnabled(P0_friction_enabled);
    p_vortexFrictionP0Coefficient->setValue(tt::floatToEInfString(P0_friction_coeff,false).c_str());
    p_vortexFrictionP0Coefficient->setEnabled(P0_friction_enabled&&P0_friction_proportional);
    p_vortexFrictionP0MaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(P0_friction_maxForce,1.0f,true),false).c_str());
    p_vortexFrictionP0MaxForce->setEnabled(P0_friction_enabled&&(!P0_friction_proportional));
    p_vortexFrictionP0Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(P0_friction_loss,1.0f,true),false).c_str());
    p_vortexFrictionP0Loss->setEnabled(P0_friction_enabled);

    if (P0_relaxation_enabled)
    {
        xAxisPositionText+=" (relaxation: on";
        xAxisPRelaxationText+=" (on)";
    }
    else
    {
        xAxisPositionText+=" (relaxation: off";
        xAxisPRelaxationText+=" (off)";
    }
    if (P0_friction_enabled)
    {
        xAxisPositionText+=", friction: on)";
        xAxisPFrictionText+=" (on)";
    }
    else
    {
        xAxisPositionText+=", friction: off)";
        xAxisPFrictionText+=" (off)";
    }

    p_vortexRelaxationP1Enabled->setValue(P1_relaxation_enabled);
    p_vortexRelaxationP1Enabled->setEnabled(true);
    p_vortexRelaxationP1Stiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(P1_stiffness,1.0f,true),false).c_str());
    p_vortexRelaxationP1Stiffness->setEnabled(P1_relaxation_enabled);
    p_vortexRelaxationP1Damping->setValue(tt::floatToEInfString(tt::floatToUserFloat(P1_damping,1.0f,true),false).c_str());
    p_vortexRelaxationP1Damping->setEnabled(P1_relaxation_enabled);
    p_vortexRelaxationP1Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(P1_loss,1.0f,true),false).c_str());
    p_vortexRelaxationP1Loss->setEnabled(P1_relaxation_enabled);
    p_vortexFrictionP1Enabled->setValue(P1_friction_enabled);
    p_vortexFrictionP1Enabled->setEnabled(true);
    p_vortexFrictionP1Proportional->setValue(P1_friction_proportional);
    p_vortexFrictionP1Proportional->setEnabled(P1_friction_enabled);
    p_vortexFrictionP1Coefficient->setValue(tt::floatToEInfString(P1_friction_coeff,false).c_str());
    p_vortexFrictionP1Coefficient->setEnabled(P1_friction_enabled&&P1_friction_proportional);
    p_vortexFrictionP1MaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(P1_friction_maxForce,1.0f,true),false).c_str());
    p_vortexFrictionP1MaxForce->setEnabled(P1_friction_enabled&&(!P1_friction_proportional));
    p_vortexFrictionP1Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(P1_friction_loss,1.0f,true),false).c_str());
    p_vortexFrictionP1Loss->setEnabled(P1_friction_enabled);

    if (P1_relaxation_enabled)
    {
        yAxisPositionText+=" (relaxation: on";
        yAxisPRelaxationText+=" (on)";
    }
    else
    {
        yAxisPositionText+=" (relaxation: off";
        yAxisPRelaxationText+=" (off)";
    }
    if (P1_friction_enabled)
    {
        yAxisPositionText+=", friction: on)";
        yAxisPFrictionText+=" (on)";
    }
    else
    {
        yAxisPositionText+=", friction: off)";
        yAxisPFrictionText+=" (off)";
    }

    if (it->getJointType()!=sim_joint_prismatic_subtype)
    { // revolute or spherical joints
        p_vortexRelaxationP2Enabled->setValue(P2_relaxation_enabled);
        p_vortexRelaxationP2Enabled->setEnabled(true);
        p_vortexRelaxationP2Stiffness->setValue(tt::floatToEInfString(tt::floatToUserFloat(P2_stiffness,1.0f,true),false).c_str());
        p_vortexRelaxationP2Stiffness->setEnabled(P2_relaxation_enabled);
        p_vortexRelaxationP2Damping->setValue(tt::floatToEInfString(tt::floatToUserFloat(P2_damping,1.0f,true),false).c_str());
        p_vortexRelaxationP2Damping->setEnabled(P2_relaxation_enabled);
        p_vortexRelaxationP2Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(P2_loss,1.0f,true),false).c_str());
        p_vortexRelaxationP2Loss->setEnabled(P2_relaxation_enabled);
        p_vortexFrictionP2Enabled->setValue(P2_friction_enabled);
        p_vortexFrictionP2Enabled->setEnabled(true);
        p_vortexFrictionP2Proportional->setValue(P2_friction_proportional);
        p_vortexFrictionP2Proportional->setEnabled(P2_friction_enabled);
        p_vortexFrictionP2Coefficient->setValue(tt::floatToEInfString(P2_friction_coeff,false).c_str());
        p_vortexFrictionP2Coefficient->setEnabled(P2_friction_enabled&&P2_friction_proportional);
        p_vortexFrictionP2MaxForce->setValue(tt::floatToEInfString(tt::floatToUserFloat(P2_friction_maxForce,1.0f,true),false).c_str());
        p_vortexFrictionP2MaxForce->setEnabled(P2_friction_enabled&&(!P2_friction_proportional));
        p_vortexFrictionP2Loss->setValue(tt::floatToEInfString(tt::floatToUserFloat(P2_friction_loss,1.0f,true),false).c_str());
        p_vortexFrictionP2Loss->setEnabled(P2_friction_enabled);

        if (P2_relaxation_enabled)
        {
            zAxisPositionText+=" (relaxation: on";
            zAxisPRelaxationText+=" (on)";
        }
        else
        {
            zAxisPositionText+=" (relaxation: off";
            zAxisPRelaxationText+=" (off)";
        }
        if (P2_friction_enabled)
        {
            zAxisPositionText+=", friction: on)";
            zAxisPFrictionText+=" (on)";
        }
        else
        {
            zAxisPositionText+=", friction: off)";
            zAxisPFrictionText+=" (off)";
        }
    }
    else
    { // prismatic joints
        p_vortexRelaxationP2Enabled->setValue("");
        p_vortexRelaxationP2Enabled->setEnabled(false);
        p_vortexRelaxationP2Stiffness->setValue("");
        p_vortexRelaxationP2Stiffness->setEnabled(false);
        p_vortexRelaxationP2Damping->setValue("");
        p_vortexRelaxationP2Damping->setEnabled(false);
        p_vortexRelaxationP2Loss->setValue("");
        p_vortexRelaxationP2Loss->setEnabled(false);
        p_vortexFrictionP2Enabled->setValue("");
        p_vortexFrictionP2Enabled->setEnabled(false);
        p_vortexFrictionP2Proportional->setValue("");
        p_vortexFrictionP2Proportional->setEnabled(false);
        p_vortexFrictionP2Coefficient->setValue("");
        p_vortexFrictionP2Coefficient->setEnabled(false);
        p_vortexFrictionP2MaxForce->setValue("");
        p_vortexFrictionP2MaxForce->setEnabled(false);
        p_vortexFrictionP2Loss->setValue("");
        p_vortexFrictionP2Loss->setEnabled(false);

        zAxisPositionText+=" (n/a)";
        zAxisPRelaxationText+=" (n/a)";
        zAxisPFrictionText+=" (n/a)";
    }

    // some group labels:
    vortexJointAxisFriction->setPropertyName(jointAxisFrictionText.c_str());

    vortexP0->setPropertyName(xAxisPositionText.c_str());
    vortexRelaxationP0->setPropertyName(xAxisPRelaxationText.c_str());
    vortexFrictionP0->setPropertyName(xAxisPFrictionText.c_str());

    vortexP1->setPropertyName(yAxisPositionText.c_str());
    vortexRelaxationP1->setPropertyName(yAxisPRelaxationText.c_str());
    vortexFrictionP1->setPropertyName(yAxisPFrictionText.c_str());

    vortexP2->setPropertyName(zAxisPositionText.c_str());
    vortexRelaxationP2->setPropertyName(zAxisPRelaxationText.c_str());
    vortexFrictionP2->setPropertyName(zAxisPFrictionText.c_str());

    vortexA0->setPropertyName(xAxisOrientationText.c_str());
    vortexRelaxationA0->setPropertyName(xAxisARelaxationText.c_str());
    vortexFrictionA0->setPropertyName(xAxisAFrictionText.c_str());

    vortexA1->setPropertyName(yAxisOrientationText.c_str());
    vortexRelaxationA1->setPropertyName(yAxisARelaxationText.c_str());
    vortexFrictionA1->setPropertyName(yAxisAFrictionText.c_str());

    vortexA2->setPropertyName(zAxisOrientationText.c_str());
    vortexRelaxationA2->setPropertyName(zAxisARelaxationText.c_str());
    vortexFrictionA2->setPropertyName(zAxisAFrictionText.c_str());



    // Newton property names:
    p_newtonJointDependencyJoint->setPropertyName("Dependent joint");
    p_newtonJointDependencyFact->setPropertyName("Multiplication factor");
    p_newtonJointDependencyOff->setPropertyName("Offset");

    // Newton parameters:
    float newtonDependentJointFact=it->getEngineFloatParam(sim_newton_joint_dependencyfactor,nullptr);
    float newtonDependentJointOff=it->getEngineFloatParam(sim_newton_joint_dependencyoffset,nullptr);

    int newtonDependentJointId=it->getEngineIntParam(sim_newton_joint_dependentobjectid,nullptr);

    depNames.clear();
    depIds.clear();
    for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
    {
        CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
        if ( (it2!=it)&&(it2->getNewtonDependentJointId()!=it->getObjectHandle())&&(it2->getJointType()!=sim_joint_spherical_subtype) )
        {
            depNames.push_back(it2->getObjectName());
            depIds.push_back(it2->getObjectHandle());
        }
    }
    tt::orderStrings(depNames,depIds);

    if (it->getJointType()!=sim_joint_spherical_subtype)
    {
        // find the correct index:
        int depIndex=0; // this means "none"
        if (newtonDependentJointId>=0)
        {
            CJoint* aJoint=App::ct->objCont->getJoint(newtonDependentJointId);
            if (aJoint!=nullptr)
            {
                for (int i=0;i<int(depNames.size());i++)
                {
                    if (depNames[i].compare(aJoint->getObjectName())==0)
                    {
                        depIndex=i+1;
                        break;
                    }
                }
            }
        }
        p_newtonJointDependencyJoint->setValue(depIndex);
        p_newtonJointDependencyJoint->setEnabled(it->getJointType()!=sim_joint_spherical_subtype);
        p_newtonJointDependencyFact->setValue(tt::floatToEInfString(newtonDependentJointFact,true).c_str());
        p_newtonJointDependencyFact->setEnabled(it->getJointType()!=sim_joint_spherical_subtype);
        p_newtonJointDependencyOff->setValue(tt::floatToEInfString(newtonDependentJointOff,true).c_str());
        p_newtonJointDependencyOff->setEnabled(it->getJointType()!=sim_joint_spherical_subtype);
    }
    else
    {
        p_newtonJointDependencyJoint->setValue("");
        p_newtonJointDependencyJoint->setEnabled(false);
        p_newtonJointDependencyFact->setValue("");
        p_newtonJointDependencyFact->setEnabled(false);
        p_newtonJointDependencyOff->setValue("");
        p_newtonJointDependencyOff->setEnabled(false);
    }

}

QtBrowserItem* CPropBrowserEngineJoint::getSubPropertyBrowserItem(const QtBrowserItem* parentBrowserItem,const QtProperty* childProperty)
{
    QList<QtBrowserItem*> children=parentBrowserItem->children();
    for (int i=0;i<int(children.size());i++)
    {
        if (children[i]->property()==childProperty)
            return(children[i]);
    }
    return(nullptr);
}


void CPropBrowserEngineJoint::catchPropertyChanges(QtProperty *_prop, QVariant value)
{
    enableNotifications(false);
    handlePropertyChanges(_prop);
    enableNotifications(true);
}
void CPropBrowserEngineJoint::catchPropertyChangesString(QtProperty *_prop, QString value)
{
    enableNotifications(false);
    handlePropertyChanges(_prop);
    enableNotifications(true);
}

void CPropBrowserEngineJoint::handlePropertyChanges(QtProperty *_prop)
{
    CJoint* it=App::ct->objCont->getLastSelection_joint();

    float f;

    // Did we press the apply all button?
    if (_prop==p_applyAllButton)
    {
        // We modify the UI AND SIM thread resources:
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_ALLENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(it->getObjectHandle());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
        {
            CJoint* anotherJoint=App::ct->objCont->getJoint(App::ct->objCont->getSelID(i));
            if (anotherJoint!=nullptr)
            {
                cmd.intParams.push_back(anotherJoint->getObjectHandle());
                it->copyEnginePropertiesTo(anotherJoint);
            }
        }
        App::appendSimulationThreadCommand(cmd);
    }

    // Bullet
    if (_prop==p_bulletNormalCFM)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_bullet_joint_normalcfm,f);
    }
    if (_prop==p_bulletStopERP)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_bullet_joint_stoperp,f);
    }
    if (_prop==p_bulletStopCFM)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_bullet_joint_stopcfm,f);
    }

    // ODE
    if (_prop==p_odeNormalCFM)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_ode_joint_normalcfm,f);
    }
    if (_prop==p_odeStopERP)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_ode_joint_stoperp,f);
    }
    if (_prop==p_odeStopCFM)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_ode_joint_stopcfm,f);
    }
    if (_prop==p_odeBounce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_ode_joint_bounce,f);
    }
    if (_prop==p_odeFudgeFactor)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_ode_joint_fudgefactor,f);
    }

    // Vortex
    if (_prop==p_vortexJointAxisFrictionEnabled)
        it->setEngineBoolParam(sim_vortex_joint_motorfrictionenabled,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexJointAxisFrictionProportional)
        it->setEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,((QtVariantProperty*)_prop)->value().toBool());
    if (_prop==p_vortexJointAxisFrictionCoefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,f);
    }
    if (_prop==p_vortexJointAxisFrictionMaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexJointAxisFrictionLoss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,tt::userFloatToFloat(f,1.0f,true));
    }



    if (_prop==p_vortexJointAxisLimitLowerRestitution)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,f);
    }
    if (_prop==p_vortexJointAxisLimitUpperRestitution)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_upperlimitrestitution,f);
    }
    if (_prop==p_vortexJointAxisLimitLowerStiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexJointAxisLimitUpperStiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_upperlimitstiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexJointAxisLimitLowerDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_lowerlimitdamping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexJointAxisLimitUpperDamping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_upperlimitdamping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexJointAxisLimitLowerMaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexJointAxisLimitUpperMaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }


    // Dependency
    if (_prop==p_vortexJointDependencyJoint)
    {
        int theIndex=((QtVariantProperty*)_prop)->value().toInt();

        std::vector<std::string> depNames;
        std::vector<int> depIds;
        for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
        {
            CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
            if ( (it2!=it)&&(it2->getVortexDependentJointId()!=it->getObjectHandle())&&(it2->getJointType()!=sim_joint_spherical_subtype) )
            {
                depNames.push_back(it2->getObjectName());
                depIds.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(depNames,depIds);

        if (theIndex==0)
            it->setEngineIntParam(sim_vortex_joint_dependentobjectid,-1);
        else
        {
            if (int(depIds.size())>(theIndex-1))
                it->setEngineIntParam(sim_vortex_joint_dependentobjectid,depIds[theIndex-1]);
        }
    }

    if (_prop==p_vortexJointDependencyFact)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            it->setEngineFloatParam(sim_vortex_joint_dependencyfactor,tt::userFloatToFloat(f,1.0f,false));
    }
    if (_prop==p_vortexJointDependencyOff)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            it->setEngineFloatParam(sim_vortex_joint_dependencyoffset,tt::userFloatToFloat(f,1.0f,false));
    }


    // P0
    if (_prop==p_vortexRelaxationP0Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr)|1;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=1;
        it->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
    }
    if (_prop==p_vortexRelaxationP0Stiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p0stiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationP0Damping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p0damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationP0Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p0loss,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionP0Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr)|1;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=1;
        it->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
    }
    if (_prop==p_vortexFrictionP0Proportional)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr)|1;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=1;
        it->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
    }
    if (_prop==p_vortexFrictionP0Coefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p0frictioncoeff,f);
    }
    if (_prop==p_vortexFrictionP0MaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionP0Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p0frictionloss,tt::userFloatToFloat(f,1.0f,true));
    }

    // P1
    if (_prop==p_vortexRelaxationP1Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr)|2;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=2;
        it->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
    }
    if (_prop==p_vortexRelaxationP1Stiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p1stiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationP1Damping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p1damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationP1Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p1loss,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionP1Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr)|2;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=2;
        it->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
    }
    if (_prop==p_vortexFrictionP1Proportional)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr)|2;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=2;
        it->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
    }
    if (_prop==p_vortexFrictionP1Coefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p1frictioncoeff,f);
    }
    if (_prop==p_vortexFrictionP1MaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionP1Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p1frictionloss,tt::userFloatToFloat(f,1.0f,true));
    }

    // P2
    if (_prop==p_vortexRelaxationP2Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr)|4;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=4;
        it->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
    }
    if (_prop==p_vortexRelaxationP2Stiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p2stiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationP2Damping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p2damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationP2Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p2loss,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionP2Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr)|4;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=4;
        it->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
    }
    if (_prop==p_vortexFrictionP2Proportional)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr)|4;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=4;
        it->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
    }
    if (_prop==p_vortexFrictionP2Coefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p2frictioncoeff,f);
    }
    if (_prop==p_vortexFrictionP2MaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionP2Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_p2frictionloss,tt::userFloatToFloat(f,1.0f,true));
    }


    // A0
    if (_prop==p_vortexRelaxationA0Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr)|8;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=8;
        it->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
    }
    if (_prop==p_vortexRelaxationA0Stiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a0stiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationA0Damping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a0damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationA0Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a0loss,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionA0Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr)|8;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=8;
        it->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
    }
    if (_prop==p_vortexFrictionA0Proportional)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr)|8;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=8;
        it->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
    }
    if (_prop==p_vortexFrictionA0Coefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a0frictioncoeff,f);
    }
    if (_prop==p_vortexFrictionA0MaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionA0Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a0frictionloss,tt::userFloatToFloat(f,1.0f,true));
    }


    // A1
    if (_prop==p_vortexRelaxationA1Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr)|16;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=16;
        it->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
    }
    if (_prop==p_vortexRelaxationA1Stiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a1stiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationA1Damping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a1damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationA1Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a1loss,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionA1Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr)|16;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=16;
        it->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
    }
    if (_prop==p_vortexFrictionA1Proportional)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr)|16;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=16;
        it->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
    }
    if (_prop==p_vortexFrictionA1Coefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a1frictioncoeff,f);
    }
    if (_prop==p_vortexFrictionA1MaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionA1Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a1frictionloss,tt::userFloatToFloat(f,1.0f,true));
    }

    // A2
    if (_prop==p_vortexRelaxationA2Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr)|32;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=32;
        it->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
    }
    if (_prop==p_vortexRelaxationA2Stiffness)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a2stiffness,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationA2Damping)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a2damping,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexRelaxationA2Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a2loss,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionA2Enabled)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr)|32;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=32;
        it->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
    }
    if (_prop==p_vortexFrictionA2Proportional)
    {
        int vval=it->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr)|32;
        if (!((QtVariantProperty*)_prop)->value().toBool())
            vval-=32;
        it->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
    }
    if (_prop==p_vortexFrictionA2Coefficient)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a2frictioncoeff,f);
    }
    if (_prop==p_vortexFrictionA2MaxForce)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,tt::userFloatToFloat(f,1.0f,true));
    }
    if (_prop==p_vortexFrictionA2Loss)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,false,false))
            it->setEngineFloatParam(sim_vortex_joint_a2frictionloss,tt::userFloatToFloat(f,1.0f,true));
    }

    // Newton

    // Dependency
    if (_prop==p_newtonJointDependencyJoint)
    {
        int theIndex=((QtVariantProperty*)_prop)->value().toInt();

        std::vector<std::string> depNames;
        std::vector<int> depIds;
        for (int i=0;i<int(App::ct->objCont->jointList.size());i++)
        {
            CJoint* it2=App::ct->objCont->getJoint(App::ct->objCont->jointList[i]);
            if ( (it2!=it)&&(it2->getNewtonDependentJointId()!=it->getObjectHandle())&&(it2->getJointType()!=sim_joint_spherical_subtype) )
            {
                depNames.push_back(it2->getObjectName());
                depIds.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(depNames,depIds);

        if (theIndex==0)
            it->setEngineIntParam(sim_newton_joint_dependentobjectid,-1);
        else
        {
            if (int(depIds.size())>(theIndex-1))
                it->setEngineIntParam(sim_newton_joint_dependentobjectid,depIds[theIndex-1]);
        }
    }

    if (_prop==p_newtonJointDependencyFact)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            it->setEngineFloatParam(sim_newton_joint_dependencyfactor,tt::userFloatToFloat(f,1.0f,false));
    }
    if (_prop==p_newtonJointDependencyOff)
    {
        if (tt::stringToFloat(((QtVariantProperty*)_prop)->value().toString().toStdString().c_str(),f,true,false))
            it->setEngineFloatParam(sim_newton_joint_dependencyoffset,tt::userFloatToFloat(f,1.0f,false));
    }

    refresh();
}
