
#pragma once

#include <QTreeWidget>
#include "modelListWidget.h"

class CModelFolderWidget : public QTreeWidget
{
    Q_OBJECT

public:
    CModelFolderWidget(CModelListWidget* modelListWidget,const char* folderName,const char* folderPath,const char* nameOfSelectedFolder);
    virtual ~CModelFolderWidget();

    void selectFolder(const char* folderPath);

    bool hasError() const;

private:
    QTreeWidgetItem* _buildChild(QTreeWidgetItem** itemToSelect,const char* folderName,const char* folderPath,const char* nameOfSelectedFolder) const;

    bool _hasError;
    CModelListWidget* _modelListWidget;

private slots:
    void onItemSelectionChanged();
};
