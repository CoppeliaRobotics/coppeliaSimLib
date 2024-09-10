#include <simInternal.h>
#include <modelListWidget.h>
#include <vFileFinder.h>
#include <imgLoaderSaver.h>
#include <app.h>
#include <simFlavor.h>
#include <sha256.h>
#include <QMimeData>
#include <QScrollBar>
#include <utils.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CModelListWidget::CModelListWidget() : CModelListWidgetBase()
{
    // setSortingEnabled(true);
    setViewMode(QListView::IconMode);
    setGridSize(QSize(160, 180));
    setIconSize(QSize(128, 128));

    setWrapping(true);
    setFlow(QListView::LeftToRight);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(false);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setWordWrap(true);
    setTextElideMode(Qt::ElideNone);

    //    setStyleSheet(QString("QListWidget:item:selected:active { background: transparent;} "));

    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(onItemClicked(QListWidgetItem *)));
}

CModelListWidget::~CModelListWidget()
{
    clearAll();
}

void CModelListWidget::clearAll()
{
    for (size_t i = 0; i < _allThumbnailsInfo.size(); i++)
        delete _allThumbnailsInfo[i].thumbnail;
    _allThumbnailsInfo.clear();
    clear();
}

void CModelListWidget::addThumbnail(CThumbnail *thumbN, const char *filepath, const char *suffix,
                                    unsigned int creationTime, unsigned char modelOrFolder, bool validFileformat,
                                    C7Vector *optionalModelTr, C3Vector *optionalModelBoundingBoxSize,
                                    double *optionalModelNonDefaultTranslationStepSize)
{
    SModelThumbnailInfo info;
    info.thumbnail = thumbN;

    info.name = VVarious::splitPath_fileBase(filepath) + suffix;
    info.filepath = filepath;
    info.creationTime = creationTime;
    info.modelOrFolder = modelOrFolder;
    info.validFileFormat = validFileformat;
    if (optionalModelTr)
        info.modelTr = optionalModelTr[0];
    else
        info.modelTr.setIdentity();
    if (optionalModelBoundingBoxSize)
        info.modelBoundingBoxSize = optionalModelBoundingBoxSize[0];
    else
        info.modelBoundingBoxSize.clear();
    if (optionalModelNonDefaultTranslationStepSize)
        info.modelNonDefaultTranslationStepSize = optionalModelNonDefaultTranslationStepSize[0];
    else
        info.modelNonDefaultTranslationStepSize = 0.0;
    _allThumbnailsInfo.push_back(info);

    _addThumbnailItemToList((int)_allThumbnailsInfo.size() - 1);
}

void CModelListWidget::_addThumbnailItemToList(int index)
{
    std::string str(_allThumbnailsInfo[index].name);
    int cnt = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] != ' ')
            cnt++;
        else
            cnt = 0;
        if (cnt > 20)
        {
            str.insert(i, "\n"); // changed from " " on 26.06.2024
            cnt = 0;
        }
    }
    QListWidgetItem *item = new QListWidgetItem(str.c_str());
    if (_allThumbnailsInfo[index].modelOrFolder == 1)
    {
        const char *rgba = _allThumbnailsInfo[index].thumbnail->getPointerToUncompressedImage();
        QImage img(128, 128, QImage::Format_ARGB32);
        QRgb value;
        for (int i = 0; i < 128; i++)
        {
            for (int j = 0; j < 128; j++)
            {
                char r = rgba[(i * 128 + j) * 4 + 0];
                char g = rgba[(i * 128 + j) * 4 + 1];
                char b = rgba[(i * 128 + j) * 4 + 2];
                char a = rgba[(i * 128 + j) * 4 + 3];
                value = qRgba(r, g, b, a);
                img.setPixel(j, 127 - i, value);
            }
        }
        QPixmap pimg;
        pimg.convertFromImage(img);
        item->setIcon(*(new QIcon(pimg)));
    }
    else
        item->setIcon(*(new QIcon(":/variousImageFiles/128x128folder.png")));
    item->setData(Qt::UserRole, index);
    insertItem(index, item);
    // sortItems();
}

CThumbnail *CModelListWidget::loadModelThumbnail(const char *pathAndFilename, int &result, C7Vector &modelTr,
                                                 C3Vector &modelBoundingBoxSize,
                                                 double &modelNonDefaultTranslationStepSize)
{ // result: -1=model not recognized, 0=model has no thumbnail, 1=model has thumbnail
    result = -1;
    CThumbnail *retThumbnail = nullptr;
    if (VFile::doesFileExist(pathAndFilename))
    {
        CSer serObj(pathAndFilename, CSer::getFileTypeFromName(pathAndFilename));

        result = serObj.readOpenBinary(1, true);
        if (result == 1)
        {
            result = 0;

            CThumbnail *thumbO = App::currentWorld->environment->modelThumbnail_notSerializedHere.copyYourself();
            modelTr.setIdentity();
            modelBoundingBoxSize.clear();
            modelNonDefaultTranslationStepSize = 0.0;
            App::currentWorld->loadModel(serObj, true, false, &modelTr, &modelBoundingBoxSize,
                                         &modelNonDefaultTranslationStepSize);
            retThumbnail = App::currentWorld->environment->modelThumbnail_notSerializedHere.copyYourself();
            App::currentWorld->environment->modelThumbnail_notSerializedHere.copyFrom(thumbO);
            delete thumbO;
            if (retThumbnail->getPointerToUncompressedImage() != nullptr)
                result = 1;
            serObj.readClose();
        }
    }
    return (retThumbnail);
}

void CModelListWidget::setFolder(const char *folderPath)
{
    clearAll();
    std::vector<int> initialSelection(App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr()[0]);
    if (folderPath != nullptr)
    {
        // 1. Get all files and their info from the folder:
        std::string fp(folderPath);
        std::string p;

        struct sost
        {
            std::string name;
            std::string path;
            std::string suffix;
            unsigned int creationTime;
            unsigned char model;
        };
        std::vector<sost> allItems;

        std::map<std::string, int> visitedItems;
        std::map<std::string, int> visitedItemsCnt;
        std::string fpath;
        while (utils::extractCommaSeparatedWord(fp, p))
        {
            VFileFinder finder;
            finder.searchFolders(p.c_str());
            int index = 0;
            SFileOrFolder *foundItem = finder.getFoundItem(index++);
            while (foundItem != nullptr)
            {
                utils::replaceSubstring(foundItem->path, "\\", "/");
                std::string base(VVarious::splitPath_fileBase(foundItem->path.c_str()));
                if ((foundItem->name != ".") && (foundItem->name != ".."))
                { // We don't wanna the . and .. folders
                    if (visitedItems.find(base) == visitedItems.end())
                    {
                        sost it;
                        it.path = foundItem->path;
                        it.name = VVarious::splitPath_fileBase(it.path.c_str());
                        it.creationTime = (unsigned int)foundItem->lastWriteTime;
                        it.model = 0;
                        visitedItems[base] = allItems.size();
                        allItems.push_back(it);
                    }
                    else
                    {
                        int ind = visitedItems[base];
                        allItems[ind].path += "," + foundItem->path;
                    }
                }
                foundItem = finder.getFoundItem(index++);
            }

            std::string suffix;
            if (fpath.size() == 0)
                fpath = p;
            else
                suffix = " (" + _getFirstDifferentDir(fpath.c_str(), p.c_str()) + ")";
            finder.searchFiles(p.c_str(), "ttm", nullptr);
            index = 0;
            foundItem = finder.getFoundItem(index++);
            while (foundItem != nullptr)
            {
                utils::replaceSubstring(foundItem->path, "\\", "/");
                std::string base(VVarious::splitPath_fileBase(foundItem->path.c_str()));
                if (visitedItemsCnt.find(base) == visitedItemsCnt.end())
                    visitedItemsCnt[base] = 1;
                else
                    visitedItemsCnt[base] = 2;
                sost it;
                it.path = foundItem->path;
                it.name = VVarious::splitPath_fileBase(it.path.c_str());
                it.suffix = suffix;
                if (visitedItems.find(base) == visitedItems.end())
                    visitedItems[base] = allItems.size();
                it.creationTime = (unsigned int)foundItem->lastWriteTime;
                it.model = 1;
                allItems.push_back(it);
                foundItem = finder.getFoundItem(index++);
            }
        }

        for (size_t i = 0; i < allItems.size(); i++)
        {
            std::string suff = allItems[i].suffix;
            if (suff.size() > 0)
            {
                std::string base(VVarious::splitPath_fileBase(allItems[i].path.c_str()));
                if (visitedItemsCnt[base] < 2)
                    allItems[i].suffix.clear();
            }
        }

        std::sort(allItems.begin(), allItems.end(),
                  [](const sost &a, const sost &b) { return a.name + a.suffix < b.name + b.suffix; });

        // 2. Check if a thumbnail file exists:
        clearAll();
        std::string fn(sha256(folderPath));
        fn = CFolderSystem::getUserSettingsPath() + "/modelThumbnails/" + fn + ".ttmt";
        bool thumbnailFileExistsAndWasLoaded = false;
        if (VFile::doesFileExist(fn.c_str()))
        {
            CSer serObj(fn.c_str(), CSer::filetype_csim_bin_thumbnails_file);
            int result = serObj.readOpenBinary(1, false);
            if (result == 1)
            {
                thumbnailFileExistsAndWasLoaded = true;
                serializePart1(serObj);
                // b. do we have the same number of files?
                if (_allThumbnailsInfo.size() != allItems.size())
                    thumbnailFileExistsAndWasLoaded = false;
                else
                { // we have the same number of files. Check if the names and last write times are same:
                    bool same = true;
                    for (size_t i = 0; i < allItems.size(); i++)
                    {
                        if (allItems[i].path.compare(_allThumbnailsInfo[i].filepath) != 0)
                        {
                            same = false;
                            break;
                        }
                        if (allItems[i].creationTime != _allThumbnailsInfo[i].creationTime)
                        {
                            same = false;
                            break;
                        }
                        if (allItems[i].model != _allThumbnailsInfo[i].modelOrFolder)
                        { // Check also if both are models or folders!
                            same = false;
                            break;
                        }
                    }
                    if (same)
                        serializePart2(serObj);
                    else
                        thumbnailFileExistsAndWasLoaded = false;
                }
                serObj.readClose();
            }
        }
        // 3. Now load all thumbnails freshly (if needed):
        if ((!thumbnailFileExistsAndWasLoaded) && (allItems.size() != 0))
        {
            clearAll();
            for (size_t k = 0; k < 2; k++)
            { // file or folder (folders first)
                for (size_t i = 0; i < allItems.size(); i++)
                {
                    if (allItems[i].model == k)
                    {
                        if (k == 0)
                        { // we have a folder here!
                            int xres, yres;
                            bool rgba;
                            unsigned char *thumbnail = CImageLoaderSaver::loadQTgaImageData(
                                ":/targaFiles/128x128folder.tga", xres, yres, rgba, nullptr);
                            CThumbnail *foldThumb = new CThumbnail();
                            foldThumb->setUncompressedThumbnailImage((char *)thumbnail, true, false);
                            delete[] thumbnail;
                            addThumbnail(foldThumb, allItems[i].path.c_str(), allItems[i].suffix.c_str(),
                                         allItems[i].creationTime, 0, true, nullptr, nullptr, nullptr);
                        }
                        else
                        { // we have a model here
                            int result;
                            C7Vector modelTr;
                            C3Vector modelBBs;
                            double ndss;
                            CThumbnail *thumbnail =
                                loadModelThumbnail(allItems[i].path.c_str(), result, modelTr, modelBBs, ndss);
                            if (thumbnail != nullptr)
                                addThumbnail(thumbnail, allItems[i].path.c_str(), allItems[i].suffix.c_str(),
                                             allItems[i].creationTime, 1, result >= 0, &modelTr, &modelBBs, &ndss);
                        }
                    }
                }
            }
            // 4. Serialize the thumbnail file for fast access in future:
            CSer serObj(fn.c_str(), CSer::filetype_csim_bin_thumbnails_file);
            serObj.writeOpenBinary(App::userSettings->compressFiles);
            serializePart1(serObj);
            serializePart2(serObj);
            serObj.writeClose();
        }
    }
    App::currentWorld->sceneObjects->setSelectedObjectHandles(initialSelection.data(), initialSelection.size());
}

std::string CModelListWidget::_getFirstDifferentDir(const char *pathA, const char *pathB)
{
    fs::path fsPathA(pathA);
    fs::path fsPathB(pathB);

    std::vector<std::string> dirsA;
    std::vector<std::string> dirsB;

    for (const auto &part : fsPathA)
        dirsA.push_back(part.string());
    for (const auto &part : fsPathB)
        dirsB.push_back(part.string());

    std::reverse(dirsA.begin(), dirsA.end());
    std::reverse(dirsB.begin(), dirsB.end());

    auto itA = dirsA.begin();
    auto itB = dirsB.begin();
    while (itA != dirsA.end() && itB != dirsB.end())
    {
        if (*itA != *itB)
            return *itB;
        ++itA;
        ++itB;
    }

    return "";
}

void CModelListWidget::serializePart1(CSer &ar)
{
    if (ar.isStoring())
    { // Storing
        ar.storeDataName("_n6");
        ar << int(_allThumbnailsInfo.size());
        for (size_t i = 0; i < _allThumbnailsInfo.size(); i++)
        {
            ar << _allThumbnailsInfo[i].name;
            ar << _allThumbnailsInfo[i].filepath;
            ar << _allThumbnailsInfo[i].creationTime;
            ar << _allThumbnailsInfo[i].modelOrFolder;
            ar << _allThumbnailsInfo[i].validFileFormat;
            for (int j = 0; j < 7; j++)
                ar << _allThumbnailsInfo[i].modelTr(j);
            for (int j = 0; j < 3; j++)
                ar << _allThumbnailsInfo[i].modelBoundingBoxSize(j);
            ar << _allThumbnailsInfo[i].modelNonDefaultTranslationStepSize;
        }
        ar.flush();

        ar.storeDataName(SER_END_OF_OBJECT);
    }
    else
    { // Loading
        int byteQuantity;
        std::string theName = "";
        while (theName.compare(SER_END_OF_OBJECT) != 0)
        {
            theName = ar.readDataName();
            if (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                bool noHit = true;
                if (theName.compare("_n6") == 0)
                {
                    noHit = false;
                    ar >> byteQuantity;
                    int thmbCnt;
                    ar >> thmbCnt;
                    _allThumbnailsInfo.resize(thmbCnt);
                    for (int i = 0; i < thmbCnt; i++)
                    {
                        std::string dum;
                        ar >> dum;
                        _allThumbnailsInfo[i].name = dum;
                        ar >> dum;
                        _allThumbnailsInfo[i].filepath = dum;
                        unsigned int dum2;
                        ar >> dum2;
                        _allThumbnailsInfo[i].creationTime = dum2;
                        unsigned char dum3;
                        ar >> dum3;
                        _allThumbnailsInfo[i].modelOrFolder = dum3;
                        ar >> dum3;
                        _allThumbnailsInfo[i].validFileFormat = dum3;
                        for (int j = 0; j < 7; j++)
                            ar >> _allThumbnailsInfo[i].modelTr(j);
                        _allThumbnailsInfo[i].modelTr.Q.normalize();
                        for (int j = 0; j < 3; j++)
                            ar >> _allThumbnailsInfo[i].modelBoundingBoxSize(j);
                        ar >> _allThumbnailsInfo[i].modelNonDefaultTranslationStepSize;
                    }
                }

                if (noHit)
                    ar.loadUnknownData();
            }
        }
    }
}

void CModelListWidget::serializePart2(CSer &ar)
{
    if (ar.isStoring())
    { // Storing
        for (size_t i = 0; i < _allThumbnailsInfo.size(); i++)
        {
            ar.storeDataName("Tc2");
            ar.setCountingMode();
            _allThumbnailsInfo[i].thumbnail->serialize(ar);
            if (ar.setWritingMode())
                _allThumbnailsInfo[i].thumbnail->serialize(ar);
        }

        ar.storeDataName(SER_END_OF_OBJECT);
    }
    else
    { // Loading
        int byteQuantity;
        std::string theName = "";
        char *compressedImage = new char[128 * 64 * 3];
        int thumbIndex = 0;
        while (theName.compare(SER_END_OF_OBJECT) != 0)
        {
            theName = ar.readDataName();
            if (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                bool noHit = true;
                if (theName.compare("Tci") == 0)
                { // for backward compatibility! (serialization version 15 sometime!)
                    noHit = false;
                    ar >> byteQuantity;
                    unsigned char imagePresent;
                    ar >> imagePresent;
                    char dum;
                    CThumbnail *it = new CThumbnail();
                    if (imagePresent != 0)
                    {
                        for (int j = 0; j < 128 * 64 * 3; j++)
                        {
                            ar >> dum;
                            compressedImage[j] = dum;
                        }
                        it->setCompressedThumbnailImage(compressedImage);
                    }
                    _allThumbnailsInfo[thumbIndex++].thumbnail = it;
                    _addThumbnailItemToList(thumbIndex - 1);
                }
                if (theName.compare("Tc2") == 0)
                {
                    noHit = false;
                    ar >> byteQuantity;
                    CThumbnail *it = new CThumbnail();
                    it->serialize(ar);
                    _allThumbnailsInfo[thumbIndex++].thumbnail = it;
                    _addThumbnailItemToList(thumbIndex - 1);
                }
                if (noHit)
                    ar.loadUnknownData();
            }
        }
        delete[] compressedImage;
    }
}

QMimeData *CModelListWidget::mimeData(const QList<QListWidgetItem *> items) const
{
    if (items.size() != 1)
        return (nullptr);
    QListWidgetItem *item = items[0];
    int index = item->data(Qt::UserRole).toInt();
    QMimeData *data = new QMimeData();
    SModelThumbnailInfo *info = (SModelThumbnailInfo *)&_allThumbnailsInfo[index];
    data->setText(info->name.c_str());
    return (data);
}

SModelThumbnailInfo *CModelListWidget::getThumbnailInfoFromModelName(const char *name, int *index)
{
    for (size_t i = 0; i < _allThumbnailsInfo.size(); i++)
    {
        if (_allThumbnailsInfo[i].name.compare(name) == 0)
        {
            if (index != nullptr)
                index[0] = (int)i;
            return (&_allThumbnailsInfo[i]);
        }
    }
    if (index != nullptr)
        index[0] = -1;
    return (nullptr);
}

void CModelListWidget::onItemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (_allThumbnailsInfo[index].modelOrFolder == 0)
        GuiApp::mainWindow->modelFolderWidget->selectFolder(_allThumbnailsInfo[index].filepath.c_str());
}
