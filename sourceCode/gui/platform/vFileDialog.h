
#pragma once

#include "vrepMainHeader.h"
#include <QWidget>

// FULLY STATIC CLASS
class VFileDialog  
{
public:
    static std::string getOpenFileName(QWidget* parent,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2="",const std::string& extension3="",const std::string& extension4="",const std::string& extension5="",const std::string& extension6="",const std::string& extension7="",const std::string& extension8="",const std::string& extension9="",const std::string& extension10="");
    static std::string getSaveFileName(QWidget* parent,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2="",const std::string& extension3="",const std::string& extension4="",const std::string& extension5="",const std::string& extension6="",const std::string& extension7="",const std::string& extension8="",const std::string& extension9="",const std::string& extension10="");
    static bool getOpenFileNames(std::vector<std::string>& files,QWidget* parent,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2="",const std::string& extension3="",const std::string& extension4="",const std::string& extension5="",const std::string& extension6="",const std::string& extension7="",const std::string& extension8="",const std::string& extension9="",const std::string& extension10="");
    static std::string getExistingFolder(QWidget* parent,const std::string& title,const std::string& startPath);

    static void setFileDialogNative(int n);

private:
    static std::string _getOpenOrSaveFileName(bool open,QWidget* parent,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2,const std::string& extension3,const std::string& extension4,const std::string& extension5,const std::string& extension6,const std::string& extension7,const std::string& extension8,const std::string& extension9,const std::string& extension10);

    static int _fileDialogNative;
};
