#pragma once

#include <QTreeWidget>
#include <modelListWidget.h>
#include <filesystem>

namespace fs = std::filesystem;

struct folderItem
{
    std::string virtualPath;
    std::string name;
    std::string absolutePath;
    QTreeWidgetItem* widgetItem;
    std::vector<folderItem*> subfolders;
    std::vector<std::string> associatedPaths;
    folderItem() {}
    folderItem(std::string vp, std::string n, std::string ap) : virtualPath(vp), name(n), absolutePath(ap) {}
};

class CModelFolderWidget : public QTreeWidget
{
    Q_OBJECT

public:
    CModelFolderWidget(CModelListWidget* modelListWidget,const char* folderName,const char* folderPath,const char* nameOfSelectedFolder);
    virtual ~CModelFolderWidget();

    void selectFolder(const char* folderPath);

    bool hasError() const;

private:
    bool _processDirectory(QTreeWidgetItem* parentTreeWidget, const fs::path& path, const std::string& rootPath, folderItem* parentItem, std::map<std::string, folderItem*>& rootPathMap, const char* nameOfSelectedFolder);
    std::vector<folderItem*> _exploreAndMergePaths(const std::vector<std::string>& rootPaths, const char* nameOfSelectedFolder);

    bool _hasError;
    CModelListWidget* _modelListWidget;

private slots:
    void onItemSelectionChanged();
};
