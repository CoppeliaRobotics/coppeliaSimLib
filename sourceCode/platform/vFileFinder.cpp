#include "vrepMainHeader.h"
#include "vFileFinder.h"
#include "vVarious.h"
#ifndef SIM_WITHOUT_QT_AT_ALL
#include <QDateTime>
#include <QDir>
#else
#include <algorithm>
#ifdef WIN_VREP
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

int VFileFinder::searchFilesWithExtension(const std::string& pathWithoutTerminalSlash,const std::string& extension)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,extension,0));
}

int VFileFinder::searchFolders(const std::string& pathWithoutTerminalSlash)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,"",1));
}

int VFileFinder::searchFilesOrFolders(const std::string& pathWithoutTerminalSlash)
{
    return(_searchFilesOrFolders(pathWithoutTerminalSlash,"",2));
}


int VFileFinder::_searchFilesOrFolders(const std::string& pathWithoutTerminalSlash,std::string extension,int mode)
{ // mode=0 --> file, mode=1 --> folder, mode=2 --> file and folder
#ifdef SIM_WITHOUT_QT_AT_ALL
    _searchResult.clear();
    DIR* dir;
    struct dirent* ent;
    if ((dir=opendir(pathWithoutTerminalSlash.c_str()))!=nullptr)
    {
        std::transform(extension.begin(),extension.end(),extension.begin(),::tolower);
        while ((ent=readdir(dir))!=nullptr)
        {
            SFileOrFolder f;
            if ( ((ent->d_type==DT_DIR)&&(mode>0))||((ent->d_type==DT_REG)&&(mode!=1)) )
            {
                bool goOn=true;
                if ((mode==0)&&((extension.compare("*")!=0)))
                { // take into account the extension
                    std::string ext(VVarious::splitPath_fileExtension(ent->d_name));
                    std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
                    goOn=(ext.compare(extension)==0);
                }
                if (goOn)
                {
                    f.isFile=(ent->d_type!=DT_DIR);
                    f.name=ent->d_name;
                    f.path=pathWithoutTerminalSlash;
                    std::string fileAndPath(pathWithoutTerminalSlash);
                    fileAndPath+='/';
                    fileAndPath+=ent->d_name;
#ifdef WIN_VREP
                    // TODO_SIM_WITHOUT_QT_AT_ALL
                    f.lastWriteTime=0;
#else // WIN_VREP
                    struct stat attrib;
                    stat(fileAndPath.c_str(),&attrib);
                    f.lastWriteTime=attrib.st_ctime;
#endif // WIN_VREP
                    _searchResult.push_back(f);
                }
            }
        }
        closedir(dir);
    }
    return(int(_searchResult.size()));
#else
    _searchResult.clear();
    QDir dir(QString::fromLocal8Bit(pathWithoutTerminalSlash.c_str()));

    if (mode==0)
    { // file
        dir.setFilter(QDir::Files|QDir::Hidden);//|QDir::NoSymLinks); 11/4/2013
        dir.setSorting(QDir::Name|QDir::IgnoreCase);
        if (extension.length()!=0)
        {
            QStringList filters;
            std::string tmp("*.");
            tmp+=extension;
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


