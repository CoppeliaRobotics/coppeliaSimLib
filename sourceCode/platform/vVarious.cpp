#include <vVarious.h>
#include <tt.h>
#include <vFile.h>
#include <vArchive.h>
#include <utils.h>
#include <vDateTime.h>
#include <string.h>
#ifdef WIN_SIM
    #include <Windows.h>
    #include <Shellapi.h>
#else
    #include <unistd.h>
#endif
#ifdef SIM_WITH_GUI
    #include <QApplication>
    #include <QClipboard>
    #include <QDesktopServices>
#endif
#include <QUrl>
#include <QProcess>
#include <QDebug>

bool VVarious::executeExternalApplication(const char* file,const char* arguments,const char* switchToDirectory,int showFlag)
{
#ifdef WIN_SIM
    int sh=SW_SHOWDEFAULT;
    if (showFlag==VVARIOUS_SHOWMAXIMIZED)
        sh=SW_SHOWMAXIMIZED;
    if (showFlag==VVARIOUS_HIDE)
        sh=SW_HIDE;
    if (strlen(switchToDirectory)!=0)
        SetCurrentDirectoryA(switchToDirectory); // needed because otherwise the shellExecute command might switch directories!

    std::string cmd(file);
    if (strlen(file)>0)
    {
        if (file[0]=='@')
            cmd.erase(cmd.begin());
    }
    return (reinterpret_cast<long long>(ShellExecuteA(nullptr,"open",cmd.c_str(),arguments,nullptr,sh))>32);
#else // WIN_SIM
    QString cmd;
    if (strlen(file)>0)
    {
        if (file[0]!='@')
        {
            if (std::string(file).find('/')==std::string::npos)
                cmd="./";
            cmd+=QString::fromLocal8Bit(file);
        }
        else
        { // for system-wide commands (e.g. xdg-open)
            cmd=QString::fromLocal8Bit(file);
            cmd.remove(0,1);
        }
    }
    QStringList strList;
    std::string args(arguments);
    std::string word;
    while (tt::extractSpaceSeparatedWord(args,word))
        strList << QString(word.c_str());
    return(QProcess::startDetached(cmd,strList,QString::fromLocal8Bit(switchToDirectory),nullptr));
#endif
}

void VVarious::removePathFinalSlashOrBackslash(std::string& pathWithOrWithoutFinalThing)
{
    size_t l=pathWithOrWithoutFinalThing.length();
    if (l>0)
    {
        if ((pathWithOrWithoutFinalThing[l-1]=='/')||(pathWithOrWithoutFinalThing[l-1]=='\\'))
        { // this might be a / or \\ part of a double char. Check for that:
            bool removeIt=true;
            if (l>1)
            {
                if ((unsigned char)(pathWithOrWithoutFinalThing[l-2])>0x7F)
                    removeIt=false;
            }

            if (removeIt)
                pathWithOrWithoutFinalThing.erase(pathWithOrWithoutFinalThing.end()-1);
        }
    }
}

std::string VVarious::splitPath_path(const char* fullPathAndName)
{   // returns the absolute path, without a final / or backslash
    std::string retVal;
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.path().toLocal8Bit().data();
    size_t l=retVal.length();
    if (l!=0)
    {
        if ((retVal[l-1]=='/')||(retVal[l-1]=='\\'))
        { // this might be a / or \\ part of a double char. Check for that:
            bool removeIt=true;
            if (l>1)
            {
                if ((unsigned char)(retVal[l-2])>0x7F)
                    removeIt=false;
            }

            if (removeIt)
                retVal.erase(retVal.end()-1);
        }
    }
    return(retVal);
}

std::string VVarious::splitPath_fileBaseAndExtension(const char* fullPathAndName)
{ // returns the filename including extension
    std::string retVal;
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.fileName().toLocal8Bit().data();
    return(retVal);
}


std::string VVarious::splitPath_fileBase(const char* fullPathAndName)
{   // returns the base of a filename, without path or extension. for xxx/yyy/zzz.a.b.c will return zzz.a.b
    std::string retVal;
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.completeBaseName().toLocal8Bit().data();
    return(retVal);
}

std::string VVarious::splitPath_fileExtension(const char* fullPathAndName)
{   // returns the filename extension (without '.')
    std::string retVal;
    QFileInfo pathInfo(QString::fromLocal8Bit(fullPathAndName));
    retVal=pathInfo.suffix().toLocal8Bit().data();
    return(retVal);
}

bool VVarious::isAbsolutePath(const char* pathAndOptionalFilename)
{
    QFileInfo pathInfo(QString::fromLocal8Bit(pathAndOptionalFilename));
    return(pathInfo.isAbsolute());
}

WLibrary VVarious::openLibrary(const char* filename,std::string* errMsg)
{ // here we have the extension in the filename (.dll, .so or .dylib)
    WLibrary lib=new QLibrary(filename);
    if (!lib->load())
    {
        if (errMsg!=nullptr)
            errMsg[0]+=lib->errorString().toStdString();
        delete lib;
        lib=nullptr;
    }
    return(lib);
}

void VVarious::closeLibrary(WLibrary lib,std::string* errMsg)
{
    if (lib!=nullptr)
    {
        if ( (!lib->unload())&&(errMsg!=nullptr) )
        {
            errMsg[0]="error: library unload: ";
            errMsg[0]+=lib->errorString().toStdString();
        }
        delete lib;
    }
}

WLibraryFunc VVarious::resolveLibraryFuncName(WLibrary lib,const char* funcName)
{
    if (lib!=nullptr)
        return((void*)lib->resolve(funcName));
    return(nullptr);
}

#ifdef SIM_WITH_GUI
bool VVarious::copyTextToClipboard(const char* text)
{
    QClipboard *clipboard=QApplication::clipboard();
    clipboard->setText(text);
    return(true);
}

bool VVarious::openOfflineUrl(const char* url)
{
    return(QDesktopServices::openUrl(QUrl::fromLocalFile(url)));
}

bool VVarious::openOnlineUrl(const char* url)
{
    return(QDesktopServices::openUrl(QUrl(url)));
}

bool VVarious::openTextFile(const char* file)
{
    return(QDesktopServices::openUrl(QUrl::fromLocalFile(file)));
}
#endif
