
#ifndef QDLGSHAPEEDITIONCONTAINER_H
#define QDLGSHAPEEDITIONCONTAINER_H

#include <dlgEx.h>
#include <QVBoxLayout>

namespace Ui {
    class CQDlgShapeEditionContainer;
}

class CQDlgShapeEditionContainer : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgShapeEditionContainer(QWidget *parent = 0);
    ~CQDlgShapeEditionContainer();

    void refresh();
    void cancelEvent();
    void okEvent();

    QVBoxLayout* bl;
    int currentPage;
    int desiredPage;

    CDlgEx* pageDlgs[3];
    int originalHeights[3];

private slots:
    void on_qqTriangleMode_clicked();

    void on_qqVertexMode_clicked();

    void on_qqEdgeMode_clicked();

private:
    Ui::CQDlgShapeEditionContainer *ui;
};

#endif // QDLGSHAPEEDITIONCONTAINER_H
