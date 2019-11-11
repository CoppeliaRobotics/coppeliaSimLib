#include "qdlginteractiveik.h"
#include "ui_qdlginteractiveik.h"
#include "app.h"
#include "tt.h"
#include "gV.h"
#include "simStrings.h"

CQDlgInteractiveIk::CQDlgInteractiveIk(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgInteractiveIk)
{
    _dlgType=INTERACTIVE_IK_DLG;
    ui->setupUi(this);
}

CQDlgInteractiveIk::~CQDlgInteractiveIk()
{
    delete ui;
}

void CQDlgInteractiveIk::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    //bool noEditModeNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();

    selectLineEdit(lineEditToSelect);
}

