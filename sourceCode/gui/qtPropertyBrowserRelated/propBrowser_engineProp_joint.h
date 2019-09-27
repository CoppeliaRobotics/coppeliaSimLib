#ifndef PROP_BROWSER_ENGINE_JOINT
#define PROP_BROWSER_ENGINE_JOINT

#include <QApplication>
#include <QDate>
#include <QLocale>
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include <buttoneditmanager.h>
#include <buttoneditfactory.h>
#include "qttreepropertybrowser.h"

class CPropBrowserEngineJoint : public QtTreePropertyBrowser
{
    Q_OBJECT
public:
//  QtTreePropertyBrowser* propertyBrowser;

    QtVariantPropertyManager* variantManager;
    QtVariantEditorFactory* variantFactory;

    ButtonEditManager* buttonManager;
    PushButtonEditFactory* buttonFactory;


    CPropBrowserEngineJoint(QWidget* pParent);
    ~CPropBrowserEngineJoint();

    void show(QWidget* parentWindow);
    void refresh();

protected:
    QtBrowserItem* getSubPropertyBrowserItem(const QtBrowserItem* parentBrowserItem,const QtProperty* childProperty);
    void handlePropertyChanges(QtProperty *_prop);
    void enableNotifications(bool enable);

    QtProperty* p_applyAllButton;

    QtBrowserItem* bulletGroupItem;
    QtBrowserItem* odeGroupItem;
    QtBrowserItem* vortexGroupItem;
    QtBrowserItem* newtonGroupItem;

    // Bullet properties:
    QtVariantProperty* p_bulletNormalCFM;
    QtVariantProperty* p_bulletStopERP;
    QtVariantProperty* p_bulletStopCFM;

    // ODE properties:
    QtVariantProperty* p_odeNormalCFM;
    QtVariantProperty* p_odeStopERP;
    QtVariantProperty* p_odeStopCFM;
    QtVariantProperty* p_odeBounce;
    QtVariantProperty* p_odeFudgeFactor;

    // Vortex properties:
    QtProperty *vortexJointAxisFriction;

    QtProperty* vortexP0;
    QtProperty* vortexRelaxationP0;
    QtProperty* vortexFrictionP0;
    QtProperty* vortexP1;
    QtProperty* vortexRelaxationP1;
    QtProperty* vortexFrictionP1;
    QtProperty* vortexP2;
    QtProperty* vortexRelaxationP2;
    QtProperty* vortexFrictionP2;

    QtProperty* vortexA0;
    QtProperty* vortexRelaxationA0;
    QtProperty* vortexFrictionA0;
    QtProperty* vortexA1;
    QtProperty* vortexRelaxationA1;
    QtProperty* vortexFrictionA1;
    QtProperty* vortexA2;
    QtProperty* vortexRelaxationA2;
    QtProperty* vortexFrictionA2;


    QtVariantProperty* p_vortexJointAxisFrictionEnabled;
    QtVariantProperty* p_vortexJointAxisFrictionProportional;
    QtVariantProperty* p_vortexJointAxisFrictionCoefficient;
    QtVariantProperty* p_vortexJointAxisFrictionMaxForce;
    QtVariantProperty* p_vortexJointAxisFrictionLoss;

    QtVariantProperty* p_vortexJointAxisLimitLowerRestitution;
    QtVariantProperty* p_vortexJointAxisLimitUpperRestitution;
    QtVariantProperty* p_vortexJointAxisLimitLowerStiffness;
    QtVariantProperty* p_vortexJointAxisLimitUpperStiffness;
    QtVariantProperty* p_vortexJointAxisLimitLowerDamping;
    QtVariantProperty* p_vortexJointAxisLimitUpperDamping;
    QtVariantProperty* p_vortexJointAxisLimitLowerMaxForce;
    QtVariantProperty* p_vortexJointAxisLimitUpperMaxForce;

    QtVariantProperty* p_vortexJointDependencyJoint;
    QtVariantProperty* p_vortexJointDependencyFact;
    QtVariantProperty* p_vortexJointDependencyOff;

    QtVariantProperty* p_vortexRelaxationP0Enabled;
    QtVariantProperty* p_vortexRelaxationP0Stiffness;
    QtVariantProperty* p_vortexRelaxationP0Damping;
    QtVariantProperty* p_vortexRelaxationP0Loss;
    QtVariantProperty* p_vortexFrictionP0Enabled;
    QtVariantProperty* p_vortexFrictionP0Proportional;
    QtVariantProperty* p_vortexFrictionP0Coefficient;
    QtVariantProperty* p_vortexFrictionP0MaxForce;
    QtVariantProperty* p_vortexFrictionP0Loss;

    QtVariantProperty* p_vortexRelaxationP1Enabled;
    QtVariantProperty* p_vortexRelaxationP1Stiffness;
    QtVariantProperty* p_vortexRelaxationP1Damping;
    QtVariantProperty* p_vortexRelaxationP1Loss;
    QtVariantProperty* p_vortexFrictionP1Enabled;
    QtVariantProperty* p_vortexFrictionP1Proportional;
    QtVariantProperty* p_vortexFrictionP1Coefficient;
    QtVariantProperty* p_vortexFrictionP1MaxForce;
    QtVariantProperty* p_vortexFrictionP1Loss;

    QtVariantProperty* p_vortexRelaxationP2Enabled;
    QtVariantProperty* p_vortexRelaxationP2Stiffness;
    QtVariantProperty* p_vortexRelaxationP2Damping;
    QtVariantProperty* p_vortexRelaxationP2Loss;
    QtVariantProperty* p_vortexFrictionP2Enabled;
    QtVariantProperty* p_vortexFrictionP2Proportional;
    QtVariantProperty* p_vortexFrictionP2Coefficient;
    QtVariantProperty* p_vortexFrictionP2MaxForce;
    QtVariantProperty* p_vortexFrictionP2Loss;

    QtVariantProperty* p_vortexRelaxationA0Enabled;
    QtVariantProperty* p_vortexRelaxationA0Stiffness;
    QtVariantProperty* p_vortexRelaxationA0Damping;
    QtVariantProperty* p_vortexRelaxationA0Loss;
    QtVariantProperty* p_vortexFrictionA0Enabled;
    QtVariantProperty* p_vortexFrictionA0Proportional;
    QtVariantProperty* p_vortexFrictionA0Coefficient;
    QtVariantProperty* p_vortexFrictionA0MaxForce;
    QtVariantProperty* p_vortexFrictionA0Loss;

    QtVariantProperty* p_vortexRelaxationA1Enabled;
    QtVariantProperty* p_vortexRelaxationA1Stiffness;
    QtVariantProperty* p_vortexRelaxationA1Damping;
    QtVariantProperty* p_vortexRelaxationA1Loss;
    QtVariantProperty* p_vortexFrictionA1Enabled;
    QtVariantProperty* p_vortexFrictionA1Proportional;
    QtVariantProperty* p_vortexFrictionA1Coefficient;
    QtVariantProperty* p_vortexFrictionA1MaxForce;
    QtVariantProperty* p_vortexFrictionA1Loss;

    QtVariantProperty* p_vortexRelaxationA2Enabled;
    QtVariantProperty* p_vortexRelaxationA2Stiffness;
    QtVariantProperty* p_vortexRelaxationA2Damping;
    QtVariantProperty* p_vortexRelaxationA2Loss;
    QtVariantProperty* p_vortexFrictionA2Enabled;
    QtVariantProperty* p_vortexFrictionA2Proportional;
    QtVariantProperty* p_vortexFrictionA2Coefficient;
    QtVariantProperty* p_vortexFrictionA2MaxForce;
    QtVariantProperty* p_vortexFrictionA2Loss;



    // Vortex items:
    QtBrowserItem* vortexJointAxisFrictionItem;
    QtBrowserItem* vortexJointAxisLimitsItem;
    QtBrowserItem* vortexJointDependencyItem;

    QtBrowserItem* vortexP0Item;
    QtBrowserItem* vortexP0RelaxationItem;
    QtBrowserItem* vortexP0FrictionItem;

    QtBrowserItem* vortexP1Item;
    QtBrowserItem* vortexP1RelaxationItem;
    QtBrowserItem* vortexP1FrictionItem;

    QtBrowserItem* vortexP2Item;
    QtBrowserItem* vortexP2RelaxationItem;
    QtBrowserItem* vortexP2FrictionItem;

    QtBrowserItem* vortexA0Item;
    QtBrowserItem* vortexA0RelaxationItem;
    QtBrowserItem* vortexA0FrictionItem;

    QtBrowserItem* vortexA1Item;
    QtBrowserItem* vortexA1RelaxationItem;
    QtBrowserItem* vortexA1FrictionItem;

    QtBrowserItem* vortexA2Item;
    QtBrowserItem* vortexA2RelaxationItem;
    QtBrowserItem* vortexA2FrictionItem;


    // Newton properties:
    QtVariantProperty* p_newtonJointDependencyJoint;
    QtVariantProperty* p_newtonJointDependencyFact;
    QtVariantProperty* p_newtonJointDependencyOff;

    QtBrowserItem* newtonJointDependencyItem;


    static bool _bulletPropertiesExpanded;
    static bool _odePropertiesExpanded;
    static bool _vortexPropertiesExpanded;
    static bool _newtonPropertiesExpanded;

    static bool _vortexJointAxisFrictionExpanded;
    static bool _vortexJointAxisLimitsExpanded;
    static bool _vortexJointDependencyExpanded;

    static bool _vortexP0Expanded;
    static bool _vortexP0RelaxationExpanded;
    static bool _vortexP0FrictionExpanded;

    static bool _vortexP1Expanded;
    static bool _vortexP1RelaxationExpanded;
    static bool _vortexP1FrictionExpanded;

    static bool _vortexP2Expanded;
    static bool _vortexP2RelaxationExpanded;
    static bool _vortexP2FrictionExpanded;

    static bool _vortexA0Expanded;
    static bool _vortexA0RelaxationExpanded;
    static bool _vortexA0FrictionExpanded;

    static bool _vortexA1Expanded;
    static bool _vortexA1RelaxationExpanded;
    static bool _vortexA1FrictionExpanded;

    static bool _vortexA2Expanded;
    static bool _vortexA2RelaxationExpanded;
    static bool _vortexA2FrictionExpanded;


    static bool _newtonJointDependencyExpanded;

    static int _dlgSizeX;
    static int _dlgSizeY;
    static int _dlgPosX;
    static int _dlgPosY;
    static int _splitterPos;

public slots:
    void reject();
    void accept();

    void catchPropertyChanges(QtProperty *_prop, QVariant value);
    void catchPropertyChangesString(QtProperty *_prop, QString value);

};
#endif // PROP_BROWSER_ENGINE_JOINT
