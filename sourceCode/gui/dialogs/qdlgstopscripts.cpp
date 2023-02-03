#include <qdlgstopscripts.h>
#include <ui_qdlgstopscripts.h>
#include <simFlavor.h>

bool CQDlgStopScripts::stopScriptNow=false;
bool CQDlgStopScripts::visible=false;

CQDlgStopScripts::CQDlgStopScripts(QWidget *parent) :
    QDialog(parent,Qt::SplashScreen),
    ui(new Ui::CQDlgStopScripts)
{
    ui->setupUi(this);
    setVisible(false);
    visible=false;
    ui->qqStop->setEnabled(CSimFlavor::getBoolVal(5));
}

CQDlgStopScripts::~CQDlgStopScripts()
{
    delete ui;
}

void CQDlgStopScripts::setScriptName(const char* name)
{
    std::string txt(CSimFlavor::getStringVal(0));
    if (txt.length()==0)
    {
        txt=name;
        txt+=":\nabort execution";
    }
    ui->qqStop->setText(txt.c_str());
}

void CQDlgStopScripts::on_qqStop_clicked()
{
    setVisible(false);
    stopScriptNow=true;
    visible=false;
}
