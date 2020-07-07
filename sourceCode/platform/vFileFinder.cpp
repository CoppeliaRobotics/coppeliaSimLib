#include "vFileFinder.h"
#include "vVarious.h"
#ifdef SIM_WITH_QT
#include <QDateTime>
#include <QDir>
#else
#include <algorithm>
#ifdef WIN_SIM
#include "_dirent.h"
#else
#include <sys/stat.h>
#include <dirent.h>
#endif
#endif

VFileFinder::VFileFinder()
{
}

VFileFinder::~VFileFinder()
{
}

int VFileFinder::searchFilesWithExtension(const char* pathWithoutTerminalSlash,const char* extension)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,extension,0));
}

int VFileFinder::searchFolders(const char* pathWithoutTerminalSlash)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,"",1));
}

int VFileFinder::searchFilesOrFolders(const char* pathWithoutTerminalSlash)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,"",2));
}


int VFileFinder::_searchFilesOrFolders(const char* pathWithoutTerminalSlash,const char* extension,int mode)
{ // mode=0 --> file, mode=1 --> folder, mode=2 --> file and folder
    std::string theExtension(extension);
#ifndef SIM_WITH_QT
    _searchResult.clear();
    DIR* dir;
    struct dirent* ent;
    if ((dir=opendir(pathWithoutTerminalSlash))!=nullptr)
    {
        std::transform(theExtension.begin(),theExtension.end(),theExtension.begin(),::tolower);
        while ((ent=readdir(dir))!=nullptr)
        {
            SFileOrFolder f;
            if ( ((ent->d_type==DT_DIR)&&(mode>0))||((ent->d_type==DT_REG)&&(mode!=1)) )
            {
                bool goOn=true;
                if ((mode==0)&&((theExtension.compare("*")!=0)))
                { // take into account the extension
                    std::string ext(VVarious::splitPath_fileExtension(ent->d_name));
                    std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
                    goOn=(ext.compare(theExtension)==0);
                }
                if (goOn)
                {
                    f.isFile=(ent->d_type!=DT_DIR);
                    f.name=ent->d_name;
                    f.path=pathWithoutTerminalSlash;
                    std::string fileAndPath(pathWithoutTerminalSlash);
                    fileAndPath+='/';
                    fileAndPath+=ent->d_name;
#ifdef WIN_SIM
                    f.lastWriteTime=0;
#else // WIN_SIM
                    struct stat attrib;
                    stat(fileAndPath.c_str(),&attrib);
                    f.lastWriteTime=attrib.st_ctime;
#endif // WIN_SIM
                    _searchResult.push_back(f);
                }
            }
        }
        closedir(dir);
    }
    return(int(_searchResult.size()));
#else
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
        f.lastWriteTime=lastWriteTime.toTime_t();
        _searchResult.push_back(f);
    }
    return(int(_searchResult.size()));
#endif
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
