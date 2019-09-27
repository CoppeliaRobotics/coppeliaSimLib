
#ifndef QDLGCOLOR_H
#define QDLGCOLOR_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgColor;
}

class CQDlgColor : public CDlgEx 
{
    Q_OBJECT

public:
    explicit CQDlgColor(QWidget *parent = 0);
    ~CQDlgColor();

    void refresh();

    void initializationEvent();
    void cancelEvent();
    bool needsDestruction();
    bool isLinkedDataValid();
    bool doesInstanceSwitchRequireDestruction();

    void computeRGBValues();
    void computeHSLValues();

    static void displayDlg(int objType,int objID1,int objID2,int colComponent,QWidget* theParentWindow,bool doNotCloseMaterialDlg=false,bool doNotCloseLightMaterialDlg=false,bool appendColorComponentInName=false);
    static void displayDlgModal(int objType,int objID1,int objID2,int colComponent,QWidget* theParentWindow,bool doNotCloseMaterialDlg=false,bool doNotCloseLightMaterialDlg=false,bool appendColorComponentInName=false);
    void initializeDlg(int objType,int objID1,int objID2,int colComponent,bool appendColorComponentInName);

    float redState;
    float greenState;
    float blueState;
    float hueState;
    float saturationState;
    float luminosityState;
    int _objType;
    int _objID1;
    int _objID2;
    int _colComponent;
    bool inRefreshPart;
    int _validityCheck1;
    int _validityCheck2;

private slots:
    void on_qqRedValue_editingFinished();

    void on_qqGreenValue_editingFinished();

    void on_qqBlueValue_editingFinished();

    void on_qqHueValue_editingFinished();

    void on_qqSaturationValue_editingFinished();

    void on_qqLuminosityValue_editingFinished();

    void on_qqRedSlider_sliderMoved(int position);

    void on_qqGreenSlider_sliderMoved(int position);

    void on_qqBlueSlider_sliderMoved(int position);

    void on_qqHueSlider_sliderMoved(int position);

    void on_qqSaturationSlider_sliderMoved(int position);

    void on_qqLuminositySlider_sliderMoved(int position);

    void on_qqRedSlider_sliderReleased();

    void on_qqGreenSlider_sliderReleased();

    void on_qqBlueSlider_sliderReleased();

    void on_qqHueSlider_sliderReleased();

    void on_qqSaturationSlider_sliderReleased();

    void on_qqLuminositySlider_sliderReleased();

private:
    void _getRGBFromItem();
    void _setRGBToItem();

    Ui::CQDlgColor *ui;
};

#endif // QDLGCOLOR_H
