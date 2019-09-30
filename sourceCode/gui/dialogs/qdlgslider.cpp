
#include "qdlgslider.h"
#include "ui_qdlgslider.h"
#include "tt.h"

CQDlgSlider::CQDlgSlider(QWidget *parent) : VDialog(parent,QT_MODAL_DLG_STYLE), ui(new Ui::CQDlgSlider)
{
    ui->setupUi(this);
}

CQDlgSlider::~CQDlgSlider()
{
    delete ui;
}

void CQDlgSlider::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSlider::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgSlider::refresh()
{
    if (opMode==0)
    {
        setWindowTitle("Mesh Decimation");
        ui->qqSlider->setMinimum(100);
        ui->qqSlider->setMaximum(900);
        std::string txt("Shape contains currently ");
        txt+=tt::getIString(false,triCnt);
        txt+=" triangles.";
        ui->qqText1->setText(txt.c_str());
        decimationPercent=tt::getLimitedFloat(0.1f,0.9f,decimationPercent);
        ui->qqSlider->setSliderPosition(int(decimationPercent*1000.0f));
        _displayDecimationText();
    }
}


void CQDlgSlider::on_qqOkCancelButtons_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSlider::on_qqOkCancelButtons_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSlider::on_qqSlider_sliderMoved(int position)
{
    if (opMode==0)
    {
        decimationPercent=float(position)/1000.0f;
        _displayDecimationText();
    }
}

void CQDlgSlider::_displayDecimationText()
{
    std::string txt="Decimate by ";
    txt+=tt::getIString(false,int(decimationPercent*100.0f));
    txt+="% (resulting shape will contain about ";
    txt+=tt::getIString(false,int(float(triCnt)*decimationPercent));
    txt+=" triangles)";
    ui->qqText2->setText(txt.c_str());
}
