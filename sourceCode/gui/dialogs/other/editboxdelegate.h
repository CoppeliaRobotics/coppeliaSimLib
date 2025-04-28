
#ifndef EDITBOXDELEGATE_H
#define EDITBOXDELEGATE_H

#include <QItemDelegate>

class CEditBoxDelegate : public QItemDelegate
{
    Q_OBJECT
  public:
    explicit CEditBoxDelegate(QObject* parent = 0);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    std::string initialText;
};
#endif // EDITBOXDELEGATE_H
