
#ifndef QDLGOPENGLSETTINGS_H
#define QDLGOPENGLSETTINGS_H

#include "vDialog.h"

namespace Ui {
    class CQDlgOpenGlSettings;
}

class CQDlgOpenGlSettings : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgOpenGlSettings(QWidget *parent = 0);
    ~CQDlgOpenGlSettings();

    void cancelEvent();
    void okEvent();

    void refresh();

    int offscreenContextType;
    int fboType;
    int vboOperation;
//    int exclusiveGuiRendering;
//    bool compatibilityTweak1;
//    bool glFinish_normal;
//    bool glFinish_visionSensors;
//    int glVersionMajor;
//    int glVersionMinor;
    int idleFps;
//    bool forceExt;

private slots:

    void on_qqOk_accepted();

    void on_qqContextDefault_clicked();

    void on_qqContextOffscreen_clicked();

    void on_qqContextVisibleWindow_clicked();

    void on_qqContextInvisibleWindow_clicked();

    void on_qqFboDefault_clicked();

    void on_qqFboNonQt_clicked();

    void on_qqFboQt_clicked();

//    void on_qqGuiRenderingOnly_clicked();

//    void on_qqCompatibilityTweak1_clicked();

//    void on_qqUsingGlFinish_clicked();

//    void on_qqUsingGlFinishVisionSensors_clicked();

//    void on_qqMajorOpenGlVersion_editingFinished();

//    void on_qqMinorOpenGlVersion_editingFinished();

    void on_qqIdleFps_editingFinished();

//    void on_qqForceExt_clicked();

    void on_qqVboDefault_clicked();

    void on_qqVboDisabled_clicked();

    void on_qqVboEnabled_clicked();

private:
    Ui::CQDlgOpenGlSettings *ui;
};

#endif // QDLGOPENGLSETTINGS_H
