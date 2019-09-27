
#pragma once

#include <QItemDelegate>

#define TREEITEMDELEGATE_ICONSPACING 2

class CSceneHierarchyItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    CSceneHierarchyItemDelegate(QWidget* parent=0);
    virtual ~CSceneHierarchyItemDelegate();

    void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    static QSize decoSize;

private slots:
};
