
#pragma once

#include <QWidget>

// FULLY STATIC CLASS
class VFileDialog  
{
public:
    static std::string getOpenFileName(QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2="",const char* extension3="",const char* extension4="",const char* extension5="",const char* extension6="",const char* extension7="",const char* extension8="",const char* extension9="",const char* extension10="");
    static std::string getSaveFileName(QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2="",const char* extension3="",const char* extension4="",const char* extension5="",const char* extension6="",const char* extension7="",const char* extension8="",const char* extension9="",const char* extension10="");
    static bool getOpenFileNames(std::vector<std::string>& files,QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2="",const char* extension3="",const char* extension4="",const char* extension5="",const char* extension6="",const char* extension7="",const char* extension8="",const char* extension9="",const char* extension10="");
    static std::string getExistingFolder(QWidget* parent,const char* title,const char* startPath);

    static void setFileDialogNative(int n);

private:
    static std::string _getOpenOrSaveFileName(bool open,QWidget* parent,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10);

    static int _fileDialogNative;
};
