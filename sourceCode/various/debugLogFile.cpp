#include "debugLogFile.h"
#include "vDateTime.h"
#include "tt.h"
#include "v_repConst.h"
#include "vMutex.h"

bool CDebugLogFile::_debugToFile=false;
bool CDebugLogFile::_addTime=true;
VFile* CDebugLogFile::_file=nullptr;
VArchive* CDebugLogFile::_archive=nullptr;

CDebugLogFile::CDebugLogFile()
{
}

CDebugLogFile::~CDebugLogFile()
{
}

void CDebugLogFile::setDebugToFile(bool toFile)
{
    _debugToFile=toFile;
}

bool CDebugLogFile::getDebugToFile()
{
    return(_debugToFile);
}

bool CDebugLogFile::getAddTime()
{
    return(_addTime);
}

void CDebugLogFile::setAddTime(bool t)
{
    _addTime=t;
}

void CDebugLogFile::addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2)
{
    std::string txt(txt1);
    txt+=txt2;
    addDebugText(forceAlsoToConsole,txt.c_str());
}

void CDebugLogFile::addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3)
{
    std::string txt(txt1);
    txt+=txt2;
    txt+=txt3;
    addDebugText(forceAlsoToConsole,txt.c_str());
}

void CDebugLogFile::addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3,const char* txt4)
{
    std::string txt(txt1);
    txt+=txt2;
    txt+=txt3;
    txt+=txt4;
    addDebugText(forceAlsoToConsole,txt.c_str());
}

void CDebugLogFile::addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3,const char* txt4,const char* txt5)
{
    std::string txt(txt1);
    txt+=txt2;
    txt+=txt3;
    txt+=txt4;
    txt+=txt5;
    addDebugText(forceAlsoToConsole,txt.c_str());
}

void CDebugLogFile::addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3,const char* txt4,const char* txt5,const char* txt6)
{
    std::string txt(txt1);
    txt+=txt2;
    txt+=txt3;
    txt+=txt4;
    txt+=txt5;
    txt+=txt6;
    addDebugText(forceAlsoToConsole,txt.c_str());
}

void CDebugLogFile::addDebugText(bool forceAlsoToConsole,const char* txt)
{
    static VMutex mutex;
    mutex.lock_simple();
    std::string theTxt(txt);
    if (_addTime)
    {
        static int st=VDateTime::getTimeInMs();
        theTxt=tt::FNb(8,VDateTime::getTimeDiffInMs(st))+" "+theTxt;
    }

    if (_debugToFile&&forceAlsoToConsole)
        printf("%s",theTxt.c_str());
    if (!_debugToFile)
        printf("%s",theTxt.c_str());
    else
    {
        if (_file==nullptr)
        {
            _file=new VFile("debugLog.txt",VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
            _archive=new VArchive(_file,VArchive::STORE);
        }

        static bool first=true;
        if (first)
        {
            std::string t(VREP_VERSION_STR);
            t+=" ";
            t+=VREP_PROGRAM_VERSION;
            t+=" ";
            t+=VREP_PROGRAM_REVISION;
#ifndef SIM_WITHOUT_QT_AT_ALL
            t+=", Qt ";
            t+=QT_VERSION_STR;
            t+=" (CT) ";
            t+=qVersion();
            t+=" (RT)";
#else
            t+=", Non-Qt version";
#endif

#ifdef WIN_VREP
            t+=", Windows";
#endif
#ifdef MAC_VREP
            t+=", MacOS";
#endif
#ifdef LIN_VREP
            t+=", Linux";
#endif
            for (int i=0;i<int(t.length());i++)
                (*_archive) << t[i];
            (*_archive) << ((unsigned char)13) << ((unsigned char)10);
        }
        first=false;

        if (theTxt[theTxt.length()-1]=='\n')
        {
            for (int i=0;i<int(theTxt.length())-1;i++)
                (*_archive) << theTxt[i];
            (*_archive) << ((unsigned char)13) << ((unsigned char)10);
        }
        else
        {
            for (int i=0;i<int(theTxt.length());i++)
                (*_archive) << theTxt[i];

        }
        _file->flush();
    }
    mutex.unlock_simple();
}
