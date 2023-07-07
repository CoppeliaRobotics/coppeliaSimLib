#include <vFile.h>
#include <simStringTable.h>
#ifdef SIM_LIB
#include <app.h>
#endif
#ifdef SIM_WITH_GUI
#include <QMessageBox>
#endif
#include <simStrings.h>
#include <vVarious.h>
#include <vFileFinder.h>

unsigned short VFile::CREATE_WRITE      =1;
unsigned short VFile::SHARE_EXCLUSIVE   =2;
unsigned short VFile::READ              =4;
unsigned short VFile::SHARE_DENY_NONE   =8;

VFile::VFile(const char* filename,unsigned short flags,bool dontThrow)
{
    _pathAndFilename=filename;
    _theFile=new QFile(QString::fromLocal8Bit(filename));
    QFlags<QIODevice::OpenModeFlag> openFlags=0;
    if (flags&CREATE_WRITE)
        openFlags|=QIODevice::Truncate|QIODevice::WriteOnly;
    if (flags&READ)
        openFlags|=QIODevice::ReadOnly;

    // Create the path directories if needed (added on 13/6/2012 because of a bug report of Matthias F�ller):
    if (flags&CREATE_WRITE)
    {
        QFileInfo pathInfo(QString::fromLocal8Bit(filename));
        QDir dir("");
        dir.mkpath(pathInfo.path());
    }

    std::exception dummyException;
    if (!_theFile->open(openFlags))
    {
        delete _theFile;
        _theFile=nullptr;
        if (!dontThrow)
            throw dummyException;
    }
    else
    {
        if (flags&CREATE_WRITE)
            _theFile->resize(0);
    }
}

VFile::VFile(const char* filename)
{ // opens a Qt resource file
    _theFile=new QFile(QString::fromLocal8Bit(filename));
    std::exception dummyException;
    if (!_theFile->exists())
    {
       throw dummyException;
    }
}

VFile::~VFile()
{
    delete _theFile;
}

void VFile::reportAndHandleFileExceptionError(VFILE_EXCEPTION_TYPE e)
{
#ifdef SIM_LIB
    App::logMsg(sim_verbosity_errors,"file exception error: %s",e.what());
#endif
#ifdef SIM_WITH_GUI
    // stl file exceptions:
    if (App::isQtAppBuilt())
        QMessageBox::critical(nullptr,IDSN_FILE_EXCEPTION_ERROR,e.what());
#endif
}

void VFile::eraseFile(const char* filenameAndPath)
{
    try
    {
        QFile::remove(filenameAndPath);
    }
    catch(VFILE_EXCEPTION_TYPE e)
    {
        VFile::reportAndHandleFileExceptionError(e);
    }
}


bool VFile::doesFileExist(const char* filenameAndPath)
{
    return(_doesFileOrFolderExist(filenameAndPath,false));
}

bool VFile::doesFolderExist(const char* foldernameAndPath)
{
    return(_doesFileOrFolderExist(foldernameAndPath,true));
}

bool VFile::_doesFileOrFolderExist(const char* filenameOrFoldernameAndPath,bool checkForFolder)
{
    QString dat(QString::fromLocal8Bit(filenameOrFoldernameAndPath));
    if (checkForFolder)
    {
        QDir dir(dat);
        return(dir.exists());       
    }
    else
    {
        QFile file(dat);
        return(file.exists());
    }
}

bool VFile::createFolder(const char* pathAndName)
{
    QDir qdir("");
    return(qdir.mkdir(QString::fromLocal8Bit(pathAndName)));
}

quint64 VFile::getLength()
{
    return(_theFile->size());
}

void VFile::close()
{
    if (_theFile!=nullptr)
        _theFile->close();
}

WFile* VFile::getFile()
{
    return(_theFile);
}

std::string VFile::getPathAndFilename()
{
    return(_pathAndFilename);
}


bool VFile::flush()
{
    return(_theFile->flush());
}


int VFile::eraseFilesWithPrefix(const char* pathWithoutTerminalSlash,const char* prefix)
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
            {
                eraseFile(foundItem->path.c_str());
                cnt++;
            }
        }
        foundItem=finder.getFoundItem(index++);
    }
    return(cnt);
}
