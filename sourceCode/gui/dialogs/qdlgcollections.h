
#ifndef QDLGCOLLECTIONS_H
#define QDLGCOLLECTIONS_H
#include "dlgEx.h"
#include <QListWidgetItem>

namespace Ui {
    class CQDlgCollections;
}

class CQDlgCollections : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgCollections(QWidget *parent = 0);
    ~CQDlgCollections();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void initializationEvent();

    int operationType;
    bool baseInclusive;
    bool tipInclusive;
    void refreshGroupList();
    void refreshSubGroupList();
    void refreshButtons();
    int getSelectedGroupID();
    void selectGroup(int groupID);
    int getAllowedOpType(int desiredOp);
    void doTheOperation(int opType,bool additive);

private slots:

    void on_qqNewCollection_clicked();

    void on_qqOverride_clicked();

    void on_qqCollectionList_itemSelectionChanged();

    void onDeletePressed();

    void on_qqVisualizeCollection_clicked();

    void on_qqCollectionList_itemChanged(QListWidgetItem *item);

    void on_qqAllObjects_clicked();

    void on_qqLooseObjects_clicked();

    void on_qqTrees_clicked();

    void on_qqBaseIncluded_clicked();

    void on_qqChains_clicked();

    void on_qqTipIncluded_clicked();

    void on_qqAdd_clicked();

    void on_qqSubtract_clicked();

private:
    Ui::CQDlgCollections *ui;
};

#endif // QDLGCOLLECTIONS_H
