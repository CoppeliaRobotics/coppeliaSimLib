
#pragma once

#include <QTreeWidget>
#include <map>

class C3DObject;

class CSceneHierarchyWidget : public QTreeWidget
{
    Q_OBJECT

public:
    CSceneHierarchyWidget();
    virtual ~CSceneHierarchyWidget();

    void mouseDoubleClickEvent(QMouseEvent* aEvent);

    void rebuild();
    void refresh();
    void removeAll();

private:
    QTreeWidgetItem* _buildObject(C3DObject* it);
    QTreeWidgetItem* _buildObjectWithHierarchy(C3DObject* it);

    std::map<int,QTreeWidgetItem*> _allTreeItems;

    int _doubleClickedColumn;

private slots:
    void onItemCollapsed(QTreeWidgetItem* item);
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTreeWidgetItem* item,int column);


};
