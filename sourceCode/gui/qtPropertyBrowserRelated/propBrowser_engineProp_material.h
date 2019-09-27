#ifndef PROP_BROWSER_ENGINE_MATERIAL
#define PROP_BROWSER_ENGINE_MATERIAL

#include <QApplication>
#include <QDate>
#include <QLocale>
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include <buttoneditmanager.h>
#include <buttoneditfactory.h>
#include "qttreepropertybrowser.h"

class CPropBrowserEngineMaterial : public QtTreePropertyBrowser
{
    Q_OBJECT
public:
//  QtTreePropertyBrowser* propertyBrowser;

    QtVariantPropertyManager* variantManager;
    QtVariantEditorFactory* variantFactory;

    ButtonEditManager* buttonManager;
    PushButtonEditFactory* buttonFactory;


    CPropBrowserEngineMaterial(QWidget* pParent);
    ~CPropBrowserEngineMaterial();

    void show(QWidget* parentWindow);
    void refresh();

protected:
    QtBrowserItem* getSubPropertyBrowserItem(const QtBrowserItem* parentBrowserItem,const QtProperty* childProperty);
    void handlePropertyChanges(QtProperty *_prop);
    void enableNotifications(bool enable);

    QtVariantProperty* p_copyFromPredefined;
//    QtVariantProperty* p_materialName;

    QtVariantProperty* p_bulletFrictionOld;
    QtVariantProperty* p_bulletFriction;
    QtVariantProperty* p_bulletRestitution;
    QtVariantProperty* p_bulletLinDamping;
    QtVariantProperty* p_bulletAngDamping;
    QtVariantProperty* p_bulletStickyContact;
    QtVariantProperty* p_bulletAutoShrink;
    QtVariantProperty* p_bulletCustomMarginScaling;
    QtVariantProperty* p_bulletMarginScalingFactor;

    QtVariantProperty* p_odeFriction;
    QtVariantProperty* p_odeMaxContacts;
    QtVariantProperty* p_odeSoftERP;
    QtVariantProperty* p_odeSoftCFM;
    QtVariantProperty* p_odeLinDamping;
    QtVariantProperty* p_odeAngDamping;


    QtProperty* linAxisPrim;
    QtProperty* linAxisSec;
    QtProperty* angAxisPrim;
    QtProperty* angAxisSec;
    QtProperty* angAxisNorm;

    QtProperty* linAxisPrimVector;

    QtVariantProperty* p_vortexFrictionModelLinPrim;
    QtVariantProperty* p_vortexFrictionCoeffLinPrim;
    QtVariantProperty* p_vortexStaticFrictionScaleLinPrim;
    QtVariantProperty* p_vortexSlipLinPrim;
    QtVariantProperty* p_vortexSlideLinPrim;

    QtVariantProperty* p_vortexLinSecFollowsLinPrim;
    QtVariantProperty* p_vortexFrictionModelLinSec;
    QtVariantProperty* p_vortexFrictionCoeffLinSec;
    QtVariantProperty* p_vortexStaticFrictionScaleLinSec;
    QtVariantProperty* p_vortexSlipLinSec;
    QtVariantProperty* p_vortexSlideLinSec;

    QtVariantProperty* p_vortexFrictionModelAngPrim;
    QtVariantProperty* p_vortexFrictionCoeffAngPrim;
    QtVariantProperty* p_vortexStaticFrictionScaleAngPrim;
    QtVariantProperty* p_vortexSlipAngPrim;
    QtVariantProperty* p_vortexSlideAngPrim;

    QtVariantProperty* p_vortexAngSecFollowsAngPrim;
    QtVariantProperty* p_vortexFrictionModelAngSec;
    QtVariantProperty* p_vortexFrictionCoeffAngSec;
    QtVariantProperty* p_vortexStaticFrictionScaleAngSec;
    QtVariantProperty* p_vortexSlipAngSec;
    QtVariantProperty* p_vortexSlideAngSec;

    QtVariantProperty* p_vortexAngNormFollowsAngPrim;
    QtVariantProperty* p_vortexFrictionModelAngNorm;
    QtVariantProperty* p_vortexFrictionCoeffAngNorm;
    QtVariantProperty* p_vortexStaticFrictionScaleAngNorm;
    QtVariantProperty* p_vortexSlipAngNorm;
    QtVariantProperty* p_vortexSlideAngNorm;

    QtVariantProperty* p_vortexRestitution;
    QtVariantProperty* p_vortexRestitutionThreshold;
    QtVariantProperty* p_vortexCompliance;
    QtVariantProperty* p_vortexDamping;
    QtVariantProperty* p_vortexAdhesiveForce;
    QtVariantProperty* p_vortexLinVelocityDamping;
    QtVariantProperty* p_vortexAngVelocityDamping;
    QtVariantProperty* p_vortexAutoAngularDampingEnabled;
    QtVariantProperty* p_vortexAutoAngularDampingTensionRatio;
    QtVariantProperty* p_vortexFastMoving;
    QtVariantProperty* p_vortexPureAsConvex;
    QtVariantProperty* p_vortexConvexAsRandom;
    QtVariantProperty* p_vortexRandomAsTerrain;
    QtVariantProperty* p_vortexSkinThickness;
    QtVariantProperty* p_vortexAutoSlip;

    QtVariantProperty* p_autoSleepLinSpeedThreshold;
    QtVariantProperty* p_autoSleepLinAccelThreshold;
    QtVariantProperty* p_autoSleepAngSpeedThreshold;
    QtVariantProperty* p_autoSleepAngAccelThreshold;
    QtVariantProperty* p_autoSleepStepsThreshold;

    QtVariantProperty* p_primAxisVectorX;
    QtVariantProperty* p_primAxisVectorY;
    QtVariantProperty* p_primAxisVectorZ;

    QtVariantProperty* p_newtonStaticFriction;
    QtVariantProperty* p_newtonKineticFriction;
    QtVariantProperty* p_newtonRestitution;
    QtVariantProperty* p_newtonLinearDrag;
    QtVariantProperty* p_newtonAngularDrag;
    QtVariantProperty* p_newtonFastMoving;


    QtBrowserItem* bulletGroupItem;
    QtBrowserItem* odeGroupItem;
    QtBrowserItem* vortexGroupItem;
    QtBrowserItem* newtonGroupItem;
    QtBrowserItem* autoSleepItem;
    QtBrowserItem* linAxisPrimItem;
    QtBrowserItem* linAxisSecItem;
    QtBrowserItem* angAxisPrimItem;
    QtBrowserItem* angAxisSecItem;
    QtBrowserItem* angAxisNormItem;

    QtBrowserItem* linAxisPrimVectorItem;


    static bool _bulletPropertiesExpanded;
    static bool _odePropertiesExpanded;
    static bool _vortexPropertiesExpanded;
    static bool _newtonPropertiesExpanded;
    static bool _vortexAutosleepPropertiesExpanded;
    static bool _vortexPrimLinAxisPropertiesExpanded;
    static bool _vortexSecLinAxisPropertiesExpanded;
    static bool _vortexPrimAngAxisPropertiesExpanded;
    static bool _vortexSecAngAxisPropertiesExpanded;
    static bool _vortexNormAngAxisPropertiesExpanded;
    static bool _vortexPrimLinAxisVectorPropertiesExpanded;

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
#endif // PROP_BROWSER_ENGINE_MATERIAL
