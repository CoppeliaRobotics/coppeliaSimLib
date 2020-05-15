
#pragma once

#include <QTreeWidget>
#include <map>

class CSceneObject;

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
    QTreeWidgetItem* _buildObject(CSceneObject* it);
    QTreeWidgetItem* _buildObjectWithHierarchy(CSceneObject* it);

    std::map<int,QTreeWidgetItem*> _allTreeItems;

    int _doubleClickedColumn;

private slots:
    void onItemCollapsed(QTreeWidgetItem* item);
    void onItemExpanded(QTreeWidgetItem* item);
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTreeWidgetItem* item,int column);


};
