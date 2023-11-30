#include <simInternal.h>
#include <modelFolderWidget.h>
#include <app.h>
#include <vFileFinder.h>
#include <simStrings.h>
#include <tt.h>
#include <iostream>
#include <map>
#include <utils.h>

CModelFolderWidget::CModelFolderWidget(CModelListWidget *modelListWidget, const char *folderName,
                                       const char *folderPath, const char *nameOfSelectedFolder)
    : QTreeWidget()
{
    _hasError = false;
    _modelListWidget = modelListWidget;
    setColumnCount(1);
    setAnimated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    std::string st =
        "QTreeView::branch:has-siblings:!adjoins-item{border-image: url(:/variousImageFiles/iline.png) 0;}";
    st += "QTreeView::branch:has-siblings:adjoins-item{border-image: url(:/variousImageFiles/tline.png) 0;}";
    st += "QTreeView::branch:!has-children:!has-siblings:adjoins-item{border-image: url(:/variousImageFiles/lline.png) "
          "0;}";
    st += "QTreeView::branch:has-children:!has-siblings:closed,QTreeView::branch:closed:has-children:has-siblings{"
          "border-image: none;image: url(:/variousImageFiles/plus.png);}";
    st += "QTreeView::branch:open:has-children:!has-siblings,QTreeView::branch:open:has-children:has-siblings{border-"
          "image: none;image: url(:/variousImageFiles/minus.png);}";
    st += "QTreeView::branch:selected{background: palette(base);}";
    if (App::userSettings->darkMode)
        st += "QTreeView::item:selected {background: palette(base); color: #050505}";
    else
        st += "QTreeView::item:selected {background-color: #3d9dfe; color: #f1f1f1}";
    setStyleSheet(st.c_str());
    //    setHeaderHidden(true);

    setSortingEnabled(true);
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
    if (VFile::doesFolderExist(folderPath))
    {
        std::vector<std::string> rootPaths;
        rootPaths.push_back(folderPath);
        // overlays <userSettings>/userContent/models and <userSettings>/overlays/*/models if present
        std::string tmp(App::folders->getUserSettingsPath() + "/userContent/models");
        if (VFile::doesFolderExist(tmp.c_str()))
            rootPaths.push_back(tmp);
        tmp = App::folders->getUserSettingsPath() + "/overlays";
        if (VFile::doesFolderExist(tmp.c_str()))
        {
            for (const auto &entry : fs::directory_iterator(tmp))
            {
                if (entry.is_directory())
                {
                    // Check if this subfolder contains a "Models" subfolder
                    fs::path modelsPath = entry.path() / "models";
                    if (fs::exists(modelsPath) && fs::is_directory(modelsPath))
                        rootPaths.push_back(modelsPath.string());
                }
            }
        }
        auto rootItems = _exploreAndMergePaths(rootPaths, nameOfSelectedFolder);
        sortItems(0, Qt::AscendingOrder);

        setHeaderLabel(folderName);
    }
    else
    {
        setHeaderLabel(IDSOGL_MODEL_FOLDER_NOT_FOUND);
        _hasError = true;
    }
}

CModelFolderWidget::~CModelFolderWidget()
{
}

void CModelFolderWidget::onItemSelectionChanged()
{
    const QList<QTreeWidgetItem *> sel = selectedItems();
    if (sel.size() != 0)
    {
        QTreeWidgetItem *item = sel[0];
        QVariant path(item->data(0, Qt::UserRole));
        _modelListWidget->setFolder(path.toString().toStdString().c_str());
    }
}

bool CModelFolderWidget::hasError() const
{
    return (_hasError);
}

void CModelFolderWidget::selectFolder(const char *folderPath)
{
    clearSelection();
    const QList<QTreeWidgetItem *> items = findItems("*", Qt::MatchRecursive | Qt::MatchWildcard);
    for (int i = 0; i < items.size(); i++)
    {
        QTreeWidgetItem *item = items[i];
        QVariant path(item->data(0, Qt::UserRole));
        if (path.toString().compare(folderPath) == 0)
        {
            bool first = true;
            while (true)
            {
                item->setSelected(first);
                item->setExpanded(true);
                item = item->parent();
                first = false;
                if (item == nullptr)
                    break;
            }
        }
    }
}

bool CModelFolderWidget::_processDirectory(QTreeWidgetItem *parentTreeWidget, const fs::path &path,
                                           const std::string &rootPath, folderItem *parentItem,
                                           std::map<std::string, folderItem *> &rootPathMap,
                                           const char *nameOfSelectedFolder)
{
    bool retVal = false;
    for (const auto &entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            std::string fileName = entry.path().stem().string();
            std::string relativePath = fs::relative(entry.path(), rootPath).parent_path().string() + "/" + fileName;
            std::string absPath = entry.path().string();
            utils::replaceSubstring(absPath, "\\", "/");
            folderItem *folderItm;
            if (rootPathMap.find(relativePath) == rootPathMap.end())
            {
                folderItm =
                    new folderItem(entry.path().string(), entry.path().filename().string(), entry.path().string());
                folderItm->widgetItem =
                    new QTreeWidgetItem(parentTreeWidget, QStringList(QString(folderItm->name.c_str())));
                folderItm->widgetItem->setIcon(0, *(new QIcon(":/variousImageFiles/folder.png")));
                folderItm->widgetItem->setData(0, Qt::UserRole, QVariant(absPath.c_str()));
                parentTreeWidget->sortChildren(0, Qt::AscendingOrder);
                parentItem->subfolders.push_back(folderItm);
                rootPathMap[relativePath] = folderItm;
                if (nameOfSelectedFolder != nullptr)
                {
                    if (fileName == nameOfSelectedFolder)
                    {
                        retVal = true;
                        folderItm->widgetItem->setExpanded(true);
                        folderItm->widgetItem->setSelected(true);
                    }
                }
            }
            else
            {
                folderItm = rootPathMap[relativePath];
                folderItm->widgetItem->setData(
                    0, Qt::UserRole,
                    QVariant((folderItm->widgetItem->data(0, Qt::UserRole).toString().toStdString() + "," + absPath)
                                 .c_str()));
            }
            folderItm->associatedPaths.push_back(entry.path().string());
            if (_processDirectory(folderItm->widgetItem, entry.path(), rootPath, folderItm, rootPathMap,
                                  nameOfSelectedFolder))
                retVal = true;
        }
    }

    std::sort(parentItem->subfolders.begin(), parentItem->subfolders.end(),
              [](const folderItem *a, const folderItem *b) { return a->name < b->name; });

    if (retVal)
        parentTreeWidget->setExpanded(true);

    return retVal;
}

std::vector<folderItem *> CModelFolderWidget::_exploreAndMergePaths(const std::vector<std::string> &rootPaths,
                                                                    const char *nameOfSelectedFolder)
{
    std::vector<std::string> subfolders;
    for (const auto &rootPath : rootPaths)
    {
        try
        {
            for (const auto &entry : fs::directory_iterator(rootPath))
            {
                std::string nm = entry.path().stem().string();
                if ((entry.is_directory()) && (nm[0] != '.')) // avoid e.g. ".git" and other hidden files
                    subfolders.push_back(entry.path().string());
            }
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }

    std::vector<folderItem *> rootItems;
    std::map<std::string, folderItem *> rootPathMap;

    for (const auto &rootPath : subfolders)
    {
        fs::path rootParent(rootPath);
        std::string folderName = rootParent.stem().string();
        folderItem *rootItem;
        if (rootPathMap.find(folderName) == rootPathMap.end())
        {
            rootItem = new folderItem(rootPath, fs::path(rootPath).filename().string(), rootPath);
            rootItem->widgetItem = new QTreeWidgetItem((QTreeWidget *)0, QStringList(QString(rootItem->name.c_str())));
            rootItem->widgetItem->setIcon(0, *(new QIcon(":/variousImageFiles/folder.png")));
            rootItem->widgetItem->setData(0, Qt::UserRole, QVariant(rootPath.c_str()));
            insertTopLevelItem(0, rootItem->widgetItem);
            rootItems.push_back(rootItem);
            rootPathMap[folderName] = rootItem;
            if (nameOfSelectedFolder != nullptr)
            {
                if (folderName == nameOfSelectedFolder)
                {
                    rootItem->widgetItem->setExpanded(true);
                    rootItem->widgetItem->setSelected(true);
                }
            }
        }
        else
        {
            rootItem = rootPathMap[folderName];
            rootItem->widgetItem->setData(
                0, Qt::UserRole,
                QVariant(
                    (rootItem->widgetItem->data(0, Qt::UserRole).toString().toStdString() + "," + rootPath).c_str()));
        }
        rootItem->associatedPaths.push_back(rootPath);
        if (_processDirectory(rootItem->widgetItem, rootPath, rootParent.parent_path().string(), rootItem, rootPathMap,
                              nameOfSelectedFolder))
            rootItem->widgetItem->setExpanded(true);
    }

    return rootItems;
}
