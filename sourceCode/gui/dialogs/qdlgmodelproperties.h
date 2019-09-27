
#ifndef QDLGMODELPROPERTIES_H
#define QDLGMODELPROPERTIES_H

#include <QAbstractButton>
#include "vDialog.h"
#include "3DObject.h"

namespace Ui {
    class CQDlgModelProperties;
}

class CQDlgModelProperties : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgModelProperties(QWidget *parent = 0);
    ~CQDlgModelProperties();

    void cancelEvent();
    void okEvent();

    void refresh();

    C3DObject* modelBaseObject;

private slots:
    void on_qqSelectThumbnail_clicked();

    void on_qqNotVisible_clicked();

    void on_qqNotCollidable_clicked();

    void on_qqNotMeasurable_clicked();

    void on_qqNotRenderable_clicked();

    void on_qqNotCuttable_clicked();

    void on_qqNotDetectable_clicked();

    void on_qqNotDynamic_clicked();

    void on_qqNotRespondable_clicked();

    void on_qqScriptsInactive_clicked();

    void on_qqNotInsideModelBBox_clicked(bool checked);

    void on_qqClose_clicked(QAbstractButton *button);

private:
    Ui::CQDlgModelProperties *ui;
};

#endif // QDLGMODELPROPERTIES_H
