
#ifndef QDLGMODELTHUMBNAIL_H
#define QDLGMODELTHUMBNAIL_H

#include "vDialog.h"
#include "visionSensor.h"
#include "thumbnail.h"

namespace Ui {
    class CQDlgModelThumbnail;
}

class CQDlgModelThumbnail : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgModelThumbnail(QWidget *parent = 0);
    ~CQDlgModelThumbnail();

    void cancelEvent();
    void okEvent();

    void initialize();
    void actualizeBitmap();

    CVisionSensor* rs;
    double rotX;
    double rotY;
    double zoom;
    double shiftX;
    double shiftY;
    bool hideEdges;
    bool antialiasing;
    std::vector<int> sel;

    CThumbnail thumbnail;
    int modelBaseDummyID;
    bool thumbnailIsFromFile;

private slots:
    void on_qqFromFile_clicked();

    void on_qqZoomP_clicked();

    void on_qqZoomM_clicked();

    void on_qqVShiftP_clicked();

    void on_qqVShiftM_clicked();

    void on_qqVRotP_clicked();

    void on_qqVRotM_clicked();

    void on_qqHShiftM_clicked();

    void on_qqHShiftP_clicked();

    void on_qqHRotM_clicked();

    void on_qqHRotP_clicked();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqHideEdges_clicked(bool checked);

    void on_qqAntialiasing_clicked(bool checked);

private:
    Ui::CQDlgModelThumbnail *ui;
};

#endif // QDLGMODELTHUMBNAIL_H
