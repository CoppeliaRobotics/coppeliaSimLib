#include <vFileFinder.h>
#include <vVarious.h>
#include <QDateTime>
#include <QDir>

VFileFinder::VFileFinder()
{
}

VFileFinder::~VFileFinder()
{
}

int VFileFinder::searchFiles(const char* pathWithoutTerminalSlash,const char* extension,const char* filter)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,extension,0,filter));
}

int VFileFinder::searchFolders(const char* pathWithoutTerminalSlash)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,"",1,nullptr));
}

int VFileFinder::searchFilesOrFolders(const char* pathWithoutTerminalSlash)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,"",2,nullptr));
}


int VFileFinder::_searchFilesOrFolders(const char* pathWithoutTerminalSlash,const char* extension,int mode,const char* filter)
{ // mode=0 --> file, mode=1 --> folder, mode=2 --> file and folder
    std::string theExtension(extension);
    _searchResult.clear();
    QDir dir(QString::fromLocal8Bit(pathWithoutTerminalSlash));

    if (mode==0)
    { // file
        dir.setFilter(QDir::Files|QDir::Hidden);//|QDir::NoSymLinks); 11/4/2013
        dir.setSorting(QDir::Name|QDir::IgnoreCase);
        if (theExtension.length()!=0)
        {
            QStringList filters;
            std::string tmp("*.");
            if (filter!=nullptr)
                tmp=filter+std::string(".");
            tmp+=theExtension;
            filters << tmp.c_str();
            dir.setNameFilters(filters);
        }
    }
    if (mode==1)
    { // folder
        dir.setFilter(QDir::Dirs|QDir::Hidden|QDir::NoSymLinks);
        dir.setSorting(QDir::Name|QDir::IgnoreCase);
    }
    if (mode==2)
    { // folder + files
        dir.setFilter(QDir::Files|QDir::Dirs|QDir::Hidden); //|QDir::NoSymLinks); // 11/4/2013
        dir.setSorting(QDir::Name|QDir::IgnoreCase);
    }
    QFileInfoList list=dir.entryInfoList();
    for (int i=0;i<list.size();++i)
    {
        QFileInfo fileInfo=list.at(i);
        SFileOrFolder f;
        f.isFile=!fileInfo.isDir();
        f.name=fileInfo.fileName().toLocal8Bit().data();
        f.path=fileInfo.filePath().toLocal8Bit().data();
        QDateTime lastWriteTime(fileInfo.lastModified());
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            f.lastWriteTime=lastWriteTime.toTime_t();
        #else
            f.lastWriteTime=lastWriteTime.toSecsSinceEpoch();
        #endif
        _searchResult.push_back(f);
    }
    return(int(_searchResult.size()));
}

SFileOrFolder* VFileFinder::getFoundItem(int index)
{
    if ( (index<0)||(index>=int(_searchResult.size())) )
        return(nullptr);
    return(&_searchResult[index]);
}


int VFileFinder::countFiles(const char* pathWithoutTerminalSlash)
{
    int cnt=0;
    VFileFinder finder;
    finder.searchFilesOrFolders(pathWithoutTerminalSlash);
    int index=0;
    SFileOrFolder* foundItem=finder.getFoundItem(index++);
    while (foundItem!=nullptr)
    {
        if (foundItem->isFile)
            cnt++;
        foundItem=finder.getFoundItem(index++);
    }
    return(cnt);
}

int VFileFinder::countFolders(const char* pathWithoutTerminalSlash)
{
    int cnt=0;
    VFileFinder finder;
    finder.searchFilesOrFolders(pathWithoutTerminalSlash);
    int index=0;
    SFileOrFolder* foundItem=finder.getFoundItem(index++);
    while (foundItem!=nullptr)
    {
        if (!foundItem->isFile)
        {
            std::string filename(foundItem->name);
            if ( (filename!=".")&&(filename!="..") )
                cnt++;
        }
        foundItem=finder.getFoundItem(index++);
    }
    return(cnt);
}

int VFileFinder::countFilesWithPrefix(const char* pathWithoutTerminalSlash,const char* prefix)
{
    int cnt=0;
    VFileFinder finder;
    finder.searchFilesOrFolders(pathWithoutTerminalSlash);
    int index=0;
    SFileOrFolder* foundItem=finder.getFoundItem(index++);
    while (foundItem!=nullptr)
    {
        if (foundItem->isFile)
        {
            std::string filename(foundItem->name);
            if (filename.find(prefix)==0)
                cnt++;
        }
        foundItem=finder.getFoundItem(index++);
    }
    return(cnt);
}
