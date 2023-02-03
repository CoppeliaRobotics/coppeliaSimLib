
#ifndef QDLGDETECTABLEPROPERTIES_H
#define QDLGDETECTABLEPROPERTIES_H

#include <vDialog.h>

namespace Ui {
    class CQDlgDetectableProperties;
}

class CQDlgDetectableProperties : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgDetectableProperties(QWidget *parent = 0);
    ~CQDlgDetectableProperties();

    void cancelEvent();
    void okEvent();
    void initializationEvent();

    void refresh();

    int objectProperties;

private slots:
    void on_qqUltrasonic_clicked();

    void on_qqInductive_clicked();

    void on_qqInfrared_clicked();

    void on_qqCapacitive_clicked();

    void on_qqLaser_clicked();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgDetectableProperties *ui;
};

#endif // QDLGDETECTABLEPROPERTIES_H
