#include <modelListWidgetBase.h>
#include <QScrollBar>

CModelListWidgetBase::CModelListWidgetBase() : QListWidget()
{
}

CModelListWidgetBase::~CModelListWidgetBase()
{
}

void CModelListWidgetBase::updateGeometries()
{
    QListWidget::updateGeometries();
    verticalScrollBar()->setSingleStep(10);
}
