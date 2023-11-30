#pragma once

#include <thumbnail.h>
#include <modelListWidgetBase.h>

struct SModelThumbnailInfo
{
    CThumbnail *thumbnail;
    std::string name;
    std::string filepath;
    unsigned int creationTime;
    unsigned char validFileFormat;
    unsigned char modelOrFolder; // 0=folder, 1=file
    C7Vector modelTr;
    C3Vector modelBoundingBoxSize;
    double modelNonDefaultTranslationStepSize;
    C3Vector desiredDropPos; // set during a drag-and-drop motion
};

class CModelListWidget : public CModelListWidgetBase
{
    Q_OBJECT

  public:
    CModelListWidget();
    virtual ~CModelListWidget();

    QMimeData *mimeData(const QList<QListWidgetItem *> items) const;
    void setFolder(const char *folderPath);

    SModelThumbnailInfo *getThumbnailInfoFromModelName(const char *name, int *index);
    void addThumbnail(CThumbnail *thumbN, const char *filepath, const char *suffix, unsigned int creationTime,
                      unsigned char modelOrFolder, bool validFileformat, C7Vector *optionalModelTr,
                      C3Vector *optionalModelBoundingBoxSize, double *optionalModelNonDefaultTranslationStepSize);
    static CThumbnail *loadModelThumbnail(const char *pathAndFilename, int &result, C7Vector &modelTr,
                                          C3Vector &modelBoundingBoxSize, double &modelNonDefaultTranslationStepSize);
    void serializePart1(CSer &ar);
    void serializePart2(CSer &ar);

  private:
    std::string _getFirstDifferentDir(const char *pathA, const char *pathB);
    void clearAll();
    void _addThumbnailItemToList(int index);
    std::vector<SModelThumbnailInfo> _allThumbnailsInfo;

  signals:
    void itemClicked(QListWidgetItem *item);

  private slots:
    void onItemClicked(QListWidgetItem *item);
};
