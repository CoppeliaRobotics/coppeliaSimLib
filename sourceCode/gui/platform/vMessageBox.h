
#pragma once

#include <QWidget>
#include "uiThread.h"

// FULLY STATIC CLASS
class VMessageBox  
{
public:
    static unsigned short information(QWidget* parent,const char* title,const char* message,unsigned short flags);
    static unsigned short question(QWidget* parent,const char* title,const char* message,unsigned short flags);
    static unsigned short warning(QWidget* parent,const char* title,const char* message,unsigned short flags);
    static unsigned short critical(QWidget* parent,const char* title,const char* message,unsigned short flags);
    static unsigned short informationSystemModal(QWidget* parent,const char* title,const char* message,unsigned short flags);

private:
    static unsigned short _displayBox(QWidget* parent,const char* title,const char* message,unsigned short flags);
};
