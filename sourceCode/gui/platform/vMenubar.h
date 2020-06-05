
#pragma once

#include <QMenu>
#include <QMenuBar>

class VMenu
{
public:
    VMenu();
    virtual ~VMenu();

    void appendMenuAndDetach(VMenu* childMenu,bool enabled,const char* label);
    void appendMenuItem(bool enabled,bool checkMark,int commandID,const char* label,bool showCheckmarkSpot=false);
    void appendMenuSeparator();
    int trackPopupMenu();
    void clear();
    QMenu* getQMenu();

protected:
    QMenu* _qMenu;
    std::vector<VMenu*> _children;
};



class VMenubar  
{
public:
    VMenubar();
    virtual ~VMenubar();

    void appendMenuAndDetach(VMenu* childMenu,bool enabled,const char* label);
    void appendMenuItem(bool enabled,bool checkMark,int commandID,const char* label,bool showCheckmarkSpot=false);
    void appendMenuSeparator();
    void clear();

    static int memorizedCommand;
    static bool doNotExecuteCommandButMemorizeIt;

protected:
    QMenuBar* _qMenubar;
    std::vector<VMenu*> _children;
};
