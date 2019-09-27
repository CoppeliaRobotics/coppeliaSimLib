
#ifndef QDLGSTOPSCRIPTS_H
#define QDLGSTOPSCRIPTS_H

#include <QDialog>
namespace Ui {
    class CQDlgStopScripts;
}

class CQDlgStopScripts : public QDialog
{
    Q_OBJECT

public:
    explicit CQDlgStopScripts(QWidget *parent = 0);
    ~CQDlgStopScripts();

    void setScriptName(const char* name);
    static bool stopScriptNow;
    static bool visible;

private slots:
    void on_qqStop_clicked();

private:
    Ui::CQDlgStopScripts *ui;
};

#endif // QDLGSTOPSCRIPTS_H
