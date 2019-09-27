
#include "sceneHierarchyItemDelegate.h"

QSize CSceneHierarchyItemDelegate::decoSize;

CSceneHierarchyItemDelegate::CSceneHierarchyItemDelegate(QWidget* parent) : QItemDelegate(parent)
{
}

CSceneHierarchyItemDelegate::~CSceneHierarchyItemDelegate()
{
}

void CSceneHierarchyItemDelegate::paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index ) const
{
    int data=index.model()->data(index,Qt::UserRole).toInt();
    if (index.column()==0)
    {
        QStyleOptionViewItem opt=option;
        int xPosOffset=0;

        if (data&1)
        { // item is selected
            opt.features=QStyleOptionViewItem::None;
            opt.state=QStyle::State_Active|QStyle::State_Selected;
            if (data&2)
                opt.palette.setColor(QPalette::Highlight,QColor(70,183,255)); // first selection
            else
                opt.palette.setColor(QPalette::Highlight,QColor(61,157,254)); // not first selection
        }

        drawBackground(painter,opt,index);

        QIcon modelIcon;
        if ((data&8)!=0)
        { // item is a model base
            if ((data&16)!=0)
                modelIcon.addFile(":/iconFiles/model2.png");
            else
                modelIcon.addFile(":/iconFiles/model.png");
        }
        else
        { // item is a not a model base
            if ((data&16)!=0)
                modelIcon.addFile(":/variousImageFiles/hline2.png");
            else
                modelIcon.addFile(":/variousImageFiles/hline.png");
        }

        QPixmap modelIconPixmap = modelIcon.pixmap(option.decorationSize);
        QRect modelIconRect=option.rect;
        modelIconRect.setWidth(option.decorationSize.width());
        drawDecoration(painter,opt,modelIconRect,modelIconPixmap);
        xPosOffset+=option.decorationSize.width()+TREEITEMDELEGATE_ICONSPACING;

        QIcon objectIcon=qvariant_cast<QIcon>(index.model()->data(index,Qt::DecorationRole)); // There is always the object type icon
        QPixmap objectIconPixmap = objectIcon.pixmap(option.decorationSize);
        decoSize=option.decorationSize;
        QRect objectIconRect=option.rect;
        objectIconRect.setX(objectIconRect.x()+xPosOffset);
        objectIconRect.setWidth(option.decorationSize.width());
        drawDecoration(painter,opt,objectIconRect,objectIconPixmap);

        xPosOffset+=option.decorationSize.width()+TREEITEMDELEGATE_ICONSPACING;

        QString text=index.model()->data(index,Qt::DisplayRole).toString();
        QRect textRect=option.rect;
        textRect.setX(textRect.x()+xPosOffset);
        if ((data&7)==4)
        { // item is not selected and not visible
            opt.features=QStyleOptionViewItem::None;
            opt.state=QStyle::State_None;
            opt.palette.setColor(QPalette::WindowText,QColor(156,156,156));
        }
        drawDisplay(painter,opt,textRect,text);
    }
    else
//        QItemDelegate::paint(painter,option,index);
//    /*
    {
        QStyleOptionViewItem opt=option;
        if (data&1)
        {
            opt.features=QStyleOptionViewItem::None;
            opt.state=QStyle::State_Active|QStyle::State_Selected;
            if (data&2)
                opt.palette.setColor(QPalette::Highlight,QColor(70,183,255));
            else
                opt.palette.setColor(QPalette::Highlight,QColor(61,157,254));
        }
        drawBackground(painter,opt,index);

        QString text=index.model()->data(index,Qt::DisplayRole).toString();
        QIcon icon=qvariant_cast<QIcon>(index.model()->data(index,Qt::DecorationRole));
        QRect displayRect=option.rect;
        if (!icon.isNull())
        {
            QPixmap iconPixmap=icon.pixmap(option.decorationSize);
            QRect rect=option.rect;
            rect.setWidth(iconPixmap.width());
            drawDecoration(painter,opt,rect,iconPixmap);
            displayRect.setX(displayRect.x()+opt.decorationSize.width()+TREEITEMDELEGATE_ICONSPACING);
        }
        if ((data&7)==4)
        {
            opt.features=QStyleOptionViewItem::None;
            opt.state=QStyle::State_None;
            opt.palette.setColor(QPalette::WindowText,QColor(156,156,156));
        }
        drawDisplay(painter,opt,displayRect,text);
    }
//    */
}

QSize CSceneHierarchyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize retVal=QItemDelegate::sizeHint(option,index);
    if (index.column()==0)
    {
        QIcon icon=qvariant_cast<QIcon>(index.model()->data(index,Qt::DecorationRole));
        if (!icon.isNull())
            retVal.setWidth(retVal.width()+option.decorationSize.width()+TREEITEMDELEGATE_ICONSPACING);
    }
    return retVal;
//    2 * QStyle::PM_FocusFrameHMargin
}
