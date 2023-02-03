
#include <editboxdelegate.h>
#include <QLineEdit>

CEditBoxDelegate::CEditBoxDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget* CEditBoxDelegate::createEditor(QWidget *parent,const QStyleOptionViewItem &/* option */,const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void CEditBoxDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
 { // start of edit
     QLineEdit *editBox = static_cast<QLineEdit*>(editor);
     editBox->setText(initialText.c_str());
}

void CEditBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
 { // end of edit
     QLineEdit *editBox = static_cast<QLineEdit*>(editor);
     model->setData(index, editBox->text(), Qt::EditRole);
 }

void CEditBoxDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
 {
     editor->setGeometry(option.rect);
 }
