
#pragma once

#include "vrepMainHeader.h"
#include <QWidget>
#include "uiThread.h"

// FULLY STATIC CLASS
class VMessageBox  
{
public:
    static unsigned short information(QWidget* parent,const std::string& title,const std::string& message,unsigned short flags);
    static unsigned short question(QWidget* parent,const std::string& title,const std::string& message,unsigned short flags);
    static unsigned short warning(QWidget* parent,const std::string& title,const std::string& message,unsigned short flags);
    static unsigned short critical(QWidget* parent,const std::string& title,const std::string& message,unsigned short flags);
    static unsigned short informationSystemModal(QWidget* parent,const std::string& title,const std::string& message,unsigned short flags);

private:
    static unsigned short _displayBox(QWidget* parent,const std::string& title,const std::string& message,unsigned short flags);
};
