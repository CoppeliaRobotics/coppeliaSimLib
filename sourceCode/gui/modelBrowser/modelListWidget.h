
#pragma once

#include "thumbnail.h"
#include "modelListWidgetBase.h"

struct SModelThumbnailInfo
{
    CThumbnail* thumbnail;
    std::string nameWithExtension;
    std::string modelPathAndNameWithExtension; // set during a drag-and-drop motion
    unsigned int creationTime;
    unsigned char validFileFormat;
    unsigned char modelOrFolder;
    C7Vector modelTr;
    C3Vector modelBoundingBoxSize;
    float modelNonDefaultTranslationStepSize;
    C3Vector desiredDropPos; // set during a drag-and-drop motion
};

class CModelListWidget : public CModelListWidgetBase
{
    Q_OBJECT

public:
    CModelListWidget();
    virtual ~CModelListWidget();


    QMimeData* mimeData(const QList<QListWidgetItem *> items) const;
    void setFolder(const char* folderPath);

    SModelThumbnailInfo* getThumbnailInfoFromModelName(const char* nameWithExtension,int* index);
    void addThumbnail(CThumbnail* thumbN,const char* nameWithExtension,unsigned int creationTime,unsigned char modelOrFolder,bool validFileformat,C7Vector* optionalModelTr,C3Vector* optionalModelBoundingBoxSize,float* optionalModelNonDefaultTranslationStepSize);
    static CThumbnail* loadModelThumbnail(const char* pathAndFilename,int& result,C7Vector& modelTr,C3Vector& modelBoundingBoxSize,float& modelNonDefaultTranslationStepSize);
    void serializePart1(CSer& ar);
    void serializePart2(CSer& ar);

private:
    void clearAll();
    void _addThumbnailItemToList(int index);

    std::string _folderPath;
    std::vector<SModelThumbnailInfo> _allThumbnailsInfo;

signals:
    void itemClicked(QListWidgetItem* item);

private slots:
    void onItemClicked(QListWidgetItem* item);
};
