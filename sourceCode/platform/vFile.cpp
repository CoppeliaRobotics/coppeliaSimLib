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
#ifndef SIM_WITH_QT
#include <sys/stat.h>
#include <sys/types.h>
#endif

unsigned short VFile::CREATE_WRITE      =1;
unsigned short VFile::SHARE_EXCLUSIVE   =2;
unsigned short VFile::READ              =4;
unsigned short VFile::SHARE_DENY_NONE   =8;

VFile::VFile(const char* filename,unsigned short flags,bool dontThrow)
{
    _pathAndFilename=filename;
#ifndef SIM_WITH_QT
    if (flags&CREATE_WRITE)
    { // Create the path directories if needed
        std::string f(VVarious::splitPath_path(filename));
        if (!doesFolderExist(f.c_str()))
            createFolder(f.c_str());
    }

    std::ios_base::openmode openFlags=std::ios_base::binary;
    if (flags&CREATE_WRITE)
        openFlags|=std::ios_base::out|std::ios_base::trunc;
    if (flags&READ)
        openFlags|=std::ios_base::in;
    _theFile=new WFile(filename,openFlags);
    std::exception dummyException;
    _fileLength=0;
    if (!_theFile->is_open())
    {
        if (!dontThrow)
            throw dummyException;
    }
    else
    {
        if (flags&READ)
        {
            _theFile->seekg(0,std::ios::beg);
            std::streampos b=_theFile->tellg();
            _theFile->seekg(0,std::ios::end);
            std::streampos e=_theFile->tellg();
            _theFile->seekg(0,std::ios::beg);
            _fileLength=(e-b);
        }
    }
#else
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
#endif
}

VFile::VFile(const char* filename)
{ // opens a Qt resource file
#ifdef SIM_WITH_QT
    _theFile=new QFile(QString::fromLocal8Bit(filename));
    std::exception dummyException;
    if (!_theFile->exists())
    {
       throw dummyException;
    }
#endif
}

VFile::~VFile()
{
#ifndef SIM_WITH_QT
    if (_theFile!=nullptr)
    {
        _theFile->close();
        delete _theFile;
    }
#else
    delete _theFile;
#endif
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
#ifndef SIM_WITH_QT
        std::remove(filenameAndPath);
#else
        QFile::remove(filenameAndPath);
#endif
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
#ifndef SIM_WITH_QT
    struct stat info;
    if (stat(filenameOrFoldernameAndPath,&info)!=0)
        return(false); // actually means this cannot be accessed
    else
    {
        if (info.st_mode&S_IFDIR)
            return(checkForFolder);
        else
            return(!checkForFolder);
    }
    return(false);
#else
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
#endif
}

bool VFile::createFolder(const char* pathAndName)
{
#ifndef SIM_WITH_QT
#ifdef WIN_SIM
    return(CreateDirectoryA(pathAndName,nullptr)!=0);
#else // WIN_SIM
    std::string tmp("mkdir -p ");
    tmp+=pathAndName;
    system(tmp.c_str());
    return(true);
#endif // WIN_SIM
#else
    QDir qdir("");
    return(qdir.mkdir(QString::fromLocal8Bit(pathAndName)));
#endif
}

quint64 VFile::getLength()
{
#ifndef SIM_WITH_QT
    return(_fileLength);
#else
    return(_theFile->size());
#endif
}

void VFile::close()
{
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
#ifndef SIM_WITH_QT
    _theFile->flush();
    return(true);
#else
    return(_theFile->flush());
#endif
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
