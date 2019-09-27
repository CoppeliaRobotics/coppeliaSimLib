
#pragma once

#include <QListWidget>

class CModelListWidgetBase : public QListWidget
{
    Q_OBJECT

public:
    CModelListWidgetBase();
    virtual ~CModelListWidgetBase();

protected:
    void updateGeometries();
};
