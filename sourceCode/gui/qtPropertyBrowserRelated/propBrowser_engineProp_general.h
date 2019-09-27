#ifndef PROP_BROWSER_ENGINE_GENERAL
#define PROP_BROWSER_ENGINE_GENERAL

#include <QApplication>
#include <QDate>
#include <QLocale>
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include <buttoneditmanager.h>
#include <buttoneditfactory.h>
#include "qttreepropertybrowser.h"

class CPropBrowserEngineGeneral : public QtTreePropertyBrowser//QObject
{
    Q_OBJECT
public:
//  QtTreePropertyBrowser* propertyBrowser;

    QtVariantPropertyManager* variantManager;
    QtVariantEditorFactory* variantFactory;

    ButtonEditManager* buttonManager;
    PushButtonEditFactory* buttonFactory;


    CPropBrowserEngineGeneral(QWidget* pParent);
    ~CPropBrowserEngineGeneral();

    void show(QWidget* parentWindow);
    void refresh();

protected:
    QtBrowserItem* getSubPropertyBrowserItem(const QtBrowserItem* parentBrowserItem,const QtProperty* childProperty);
    void handlePropertyChanges(QtProperty *_prop);
    void enableNotifications(bool enable);

    QtVariantProperty* p_configuration;

    QtVariantProperty* p_bulletTimeStep;
    QtVariantProperty* p_bulletConstraintSolvType;
    QtVariantProperty* p_bulletConstraintSolvIterat;
    QtVariantProperty* p_bulletInternalScaling;
    QtVariantProperty* p_bulletInternalFullScaling;
    QtVariantProperty* p_bulletCollMarginScaling;

    QtVariantProperty* p_odeTimeStep;
    QtVariantProperty* p_odeQuickStep;
    QtVariantProperty* p_odeIterations;
    QtVariantProperty* p_odeInternalScaling;
    QtVariantProperty* p_odeInternalFullScaling;
    QtVariantProperty* p_odeGlobalErp;
    QtVariantProperty* p_odeGlobalCfm;

    QtVariantProperty* p_vortexTimeStep;
//  QtVariantProperty* p_vortexInternalScaling;
//  QtVariantProperty* p_vortexInternalFullScaling;
    QtVariantProperty* p_vortexContactTolerance;
    QtVariantProperty* p_vortexAutoSleep;
    QtVariantProperty* p_vortexMultithreading;
    QtVariantProperty* p_vortexConstraintLinearCompliance;
    QtVariantProperty* p_vortexConstraintLinearDamping;
    QtVariantProperty* p_vortexConstraintLinearKinLoss;
    QtVariantProperty* p_vortexConstraintAngularCompliance;
    QtVariantProperty* p_vortexConstraintAngularDamping;
    QtVariantProperty* p_vortexConstraintAngularKinLoss;

    QtVariantProperty* p_newtonTimeStep;
    QtVariantProperty* p_newtonSolvingIterations;
    QtVariantProperty* p_newtonMultithreading;
    QtVariantProperty* p_newtonExactSolver;
    QtVariantProperty* p_newtonHighJointAccuracy;
    QtVariantProperty* p_newtonContactMergeTolerance;

    QtBrowserItem* bulletGroupItem;
    QtBrowserItem* odeGroupItem;
    QtBrowserItem* vortexGroupItem;
    QtBrowserItem* newtonGroupItem;
    QtBrowserItem* vortexConstraintItem;


    static bool _bulletPropertiesExpanded;
    static bool _odePropertiesExpanded;
    static bool _vortexPropertiesExpanded;
    static bool _newtonPropertiesExpanded;
    static bool _vortexConstraintPropertiesExpanded;

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
#endif // PROP_BROWSER_ENGINE_GENERAL
