
#include <vFileDialog.h>
#include <QFileDialog>

int VFileDialog::_fileDialogNative=-1; // default

void VFileDialog::setFileDialogNative(int n)
{
    _fileDialogNative=n;
}

std::string VFileDialog::getOpenFileName(QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    return(_getOpenOrSaveFileName(true,parent,option,title,startPath,initFilename,allowAnyFile,extensionName,extension1,extension2,extension3,extension4,extension5,extension6,extension7,extension8,extension9,extension10));
}

std::string VFileDialog::getSaveFileName(QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    return(_getOpenOrSaveFileName(false,parent,option,title,startPath,initFilename,allowAnyFile,extensionName,extension1,extension2,extension3,extension4,extension5,extension6,extension7,extension8,extension9,extension10));
}

std::string VFileDialog::_getOpenOrSaveFileName(bool open,QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    QString retString;
    QString pathAndOptionalFilename(QString::fromLocal8Bit(startPath));
    if (strlen(initFilename)!=0)
    {
        if (pathAndOptionalFilename.length()==0)
            pathAndOptionalFilename=QString::fromLocal8Bit(initFilename);
        else
        {
            QChar c=pathAndOptionalFilename.at(pathAndOptionalFilename.length()-1);
            if ( (c!='/')&&(c!='\\') )
                pathAndOptionalFilename+='/';
            pathAndOptionalFilename+=QString::fromLocal8Bit(initFilename);
        }
    }
    std::string filter=extensionName;
    if (std::string(extensionName).find(";;")==std::string::npos)
    {
        filter=std::string(extensionName)+" (*.";
        filter+=extension1;
        if (strlen(extension2)!=0)
            filter+=std::string(" *.")+extension2;
        if (strlen(extension3)!=0)
            filter+=std::string(" *.")+extension3;
        if (strlen(extension4)!=0)
            filter+=std::string(" *.")+extension4;
        if (strlen(extension5)!=0)
            filter+=std::string(" *.")+extension5;
        if (strlen(extension6)!=0)
            filter+=std::string(" *.")+extension6;
        if (strlen(extension7)!=0)
            filter+=std::string(" *.")+extension7;
        if (strlen(extension8)!=0)
            filter+=std::string(" *.")+extension8;
        if (strlen(extension9)!=0)
            filter+=std::string(" *.")+extension9;
        if (strlen(extension10)!=0)
            filter+=std::string(" *.")+extension10;
        filter+=")";
        if (allowAnyFile)
            filter+=";;all (*.*)";
    }

    bool native=true;
#ifndef WIN_SIM // native dialogs have a bug on MacOS/Linux versions: the initial directory is not set. Because of that, we don't use native dialogs
    native=false;
#endif
    if (_fileDialogNative!=-1)
    {
        if (_fileDialogNative==0)
            native=true;
        if (_fileDialogNative==1)
            native=false;
    }

#ifndef WIN_SIM
    // On Linux (and sometimes on Mac too), we have several problems if the message queue is not fully processed,
    // but only partially processed (e.g. App::qtApp->processEvents(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers)).
    // A quick/dirty fix is tweak1 below. The full fix is to process all messages while
    // waiting for a lock, which is done now by default.
    QFileDialog dlg(parent,title,pathAndOptionalFilename,filter.c_str());
    dlg.setNameFilter(filter.c_str());
    if (open)
    {
        dlg.setFileMode(QFileDialog::ExistingFile);
        dlg.setAcceptMode(QFileDialog::AcceptOpen);
    }
    else
    {
        dlg.setFileMode(QFileDialog::AnyFile);
        dlg.setAcceptMode(QFileDialog::AcceptSave);
    }
    if (!native)
        dlg.setOptions(QFileDialog::DontUseNativeDialog);

    if (dlg.exec())
    {
        QStringList qfiles=dlg.selectedFiles();
        if (qfiles.size()==1)
            retString=qfiles.at(0);
    }
#else
    if (!native)
    {
        if (open)
            retString=QFileDialog::getOpenFileName(parent,title,pathAndOptionalFilename,filter.c_str(),0,QFileDialog::DontUseNativeDialog);
        else
            retString=QFileDialog::getSaveFileName(parent,title,pathAndOptionalFilename,filter.c_str(),0,QFileDialog::DontUseNativeDialog);
    }
    else
    {
        if (open)
            retString=QFileDialog::getOpenFileName(parent,title,pathAndOptionalFilename,filter.c_str());
        else
            retString=QFileDialog::getSaveFileName(parent,title,pathAndOptionalFilename,filter.c_str());
    }
#endif

    // the extension is sometimes omited! Check it:
    std::string rs(retString.toLocal8Bit().data());
    if ((!open)&&(std::string(extension1).compare("*")!=0)&&(strlen(extension1)!=0)&&(strlen(extension2)==0)&&(rs.size()>0))
    {
        bool addExtension=false;
        if (rs.size()>strlen(extension1)+1)
        {
            if (rs[rs.size()-1-strlen(extension1)]!='.')
                addExtension=true;
            else
            {
                for (unsigned int i=0;i<strlen(extension1);i++)
                {
                    if (rs[rs.size()-strlen(extension1)+i]!=extension1[i])
                        addExtension=true;
                }
            }
        }
        else
            addExtension=true;
        if (addExtension)
        {
            rs+='.';
            rs+=extension1;
        }
    }
    return(rs);
}


bool VFileDialog::getOpenFileNames(std::vector<std::string>& files,QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    files.clear();
    QString pathAndOptionalFilename(QString::fromLocal8Bit(startPath));
    if (strlen(initFilename)!=0)
        pathAndOptionalFilename=QString::fromLocal8Bit(initFilename);
    std::string filter=std::string(extensionName)+" (*.";
    filter+=extension1;
    if (strlen(extension2)!=0)
        filter+=std::string(" *.")+extension2;
    if (strlen(extension3)!=0)
        filter+=std::string(" *.")+extension3;
    if (strlen(extension4)!=0)
        filter+=std::string(" *.")+extension4;
    if (strlen(extension5)!=0)
        filter+=std::string(" *.")+extension5;
    if (strlen(extension6)!=0)
        filter+=std::string(" *.")+extension6;
    if (strlen(extension7)!=0)
        filter+=std::string(" *.")+extension7;
    if (strlen(extension8)!=0)
        filter+=std::string(" *.")+extension8;
    if (strlen(extension9)!=0)
        filter+=std::string(" *.")+extension9;
    if (strlen(extension10)!=0)
        filter+=std::string(" *.")+extension10;
    filter+=")";
    if (allowAnyFile)
        filter+=";;all (*.*)";

    bool native=true;
#ifndef WIN_SIM // native dialogs have a bug on MacOS/Linux versions: the initial directory is not set. Because of that, we don't use native dialogs
    native=false;
#endif
    if (_fileDialogNative!=-1)
    {
        if (_fileDialogNative==0)
            native=true;
        if (_fileDialogNative==1)
            native=false;
    }

    QStringList qfiles;

#ifndef WIN_SIM
    // On Linux (and sometimes on Mac too), the static functions sometimes just display a blank dialog. For that reason,
    // we use the non-static version on those platforms! The "show()/hide()" below might be important, otherwise we have the same
    // problem as with the static version of the functions!
    QFileDialog dlg(parent,title,pathAndOptionalFilename,filter.c_str());
    dlg.setNameFilter(filter.c_str());
    dlg.setFileMode(QFileDialog::ExistingFiles);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    if (!native)
        dlg.setOptions(QFileDialog::DontUseNativeDialog);
    if (dlg.exec())
        qfiles=dlg.selectedFiles();
#else
    if (!native)
        qfiles=QFileDialog::getOpenFileNames(parent,title,pathAndOptionalFilename,filter.c_str(),0,QFileDialog::DontUseNativeDialog);
    else
        qfiles=QFileDialog::getOpenFileNames(parent,title,pathAndOptionalFilename,filter.c_str());
#endif

    for (size_t i=0;i<qfiles.size();i++)
        files.push_back(qfiles.at(int(i)).toLocal8Bit().data());
    return(files.size()>0);
}

std::string VFileDialog::getExistingFolder(QWidget* parent,const char* title,const char* startPath)
{
    QString retString;
    QString path(QString::fromLocal8Bit(startPath));

    bool native=true;
#ifndef WIN_SIM // native dialogs have a bug on MacOS/Linux versions: the initial directory is not set. Because of that, we don't use native dialogs
    native=false;
#endif
    if (_fileDialogNative!=-1)
    {
        if (_fileDialogNative==0)
            native=true;
        if (_fileDialogNative==1)
            native=false;
    }
    QFileDialog::Options options=QFileDialog::ShowDirsOnly;
    if (!native)
        options|=QFileDialog::DontUseNativeDialog;
    retString=QFileDialog::getExistingDirectory(parent,title,path,options);
    return(retString.toLocal8Bit().data());
}

