#include "app.h"
#include "vMessageBox.h"
#include <QMessageBox>

enum {
        VMESSAGEBOX_APP_MODAL       =8,
        VMESSAGEBOX_INFO_TYPE       =16,
        VMESSAGEBOX_QUESTION_TYPE   =32,
        VMESSAGEBOX_WARNING_TYPE    =64,
        VMESSAGEBOX_CRITICAL_TYPE   =128,
};

unsigned short VMessageBox::informationSystemModal(QWidget* parent,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{ // Don't forget: parent can be nullptr at application start-up!
    unsigned short retVal=defaultAnswer;
    if (App::getDlgVerbosity()>=sim_verbosity_infos)
    {
        flags|=VMESSAGEBOX_INFO_TYPE;
        retVal=_displayBox(parent,title,message,flags);
    }
    else
    {
        std::string str("from suppressed dialog box: ");
        str+=message;
        App::logMsg(sim_verbosity_infos,str.c_str());
    }
    return(retVal);
}

unsigned short VMessageBox::information(QWidget* parent,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{ // Don't forget: parent can be nullptr at application start-up!
    unsigned short retVal=defaultAnswer;
    if (App::getDlgVerbosity()>=sim_verbosity_infos)
    {
        flags|=VMESSAGEBOX_APP_MODAL|VMESSAGEBOX_INFO_TYPE;
        retVal=_displayBox(parent,title,message,flags);
    }
    else
    {
        std::string str("from suppressed dialog box: ");
        str+=message;
        App::logMsg(sim_verbosity_infos,str.c_str());
    }
    return(retVal);
}

unsigned short VMessageBox::question(QWidget* parent,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{ // Don't forget: parent can be nullptr at application start-up!
    unsigned short retVal=defaultAnswer;
    if (App::getDlgVerbosity()>=sim_verbosity_questions)
    {
        flags|=VMESSAGEBOX_APP_MODAL|VMESSAGEBOX_QUESTION_TYPE;
        retVal=_displayBox(parent,title,message,flags);
    }
    else
    {
        std::string str("from suppressed dialog box: ");
        str+=message;
        App::logMsg(sim_verbosity_warnings,str.c_str());
    }
    return(retVal);
}

unsigned short VMessageBox::warning(QWidget* parent,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{ // Don't forget: parent can be nullptr at application start-up!
    unsigned short retVal=defaultAnswer;
    if (App::getDlgVerbosity()>=sim_verbosity_warnings)
    {
        flags|=VMESSAGEBOX_APP_MODAL|VMESSAGEBOX_WARNING_TYPE;
        retVal=_displayBox(parent,title,message,flags);
    }
    else
    {
        std::string str("from suppressed dialog box: ");
        str+=message;
        App::logMsg(sim_verbosity_warnings,str.c_str());
    }
    return(retVal);
}

unsigned short VMessageBox::critical(QWidget* parent,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{ // Don't forget: parent can be nullptr at application start-up!
    unsigned short retVal=defaultAnswer;
    if (App::getDlgVerbosity()>=sim_verbosity_errors)
    {
        flags|=VMESSAGEBOX_APP_MODAL|VMESSAGEBOX_CRITICAL_TYPE;
        retVal=_displayBox(parent,title,message,flags);
    }
    else
    {
        std::string str("from suppressed dialog box: ");
        str+=message;
        App::logMsg(sim_verbosity_errors,str.c_str());
    }
    return(retVal);
}

unsigned short VMessageBox::_displayBox(QWidget* parent,const char* title,const char* message,unsigned short flags)
{ // Don't forget: parent can be nullptr at application start-up!
    unsigned short lower=(flags&7);
    unsigned short upper=flags-lower;

    QMessageBox msg(parent);
    msg.setWindowTitle(title);
    msg.setText(message);
    if (lower==VMESSAGEBOX_OKELI)
        msg.addButton(QMessageBox::Ok);
    if (lower==VMESSAGEBOX_YES_NO)
    {
        msg.addButton(QMessageBox::Yes);
        msg.addButton(QMessageBox::No);
    }
    if (lower==VMESSAGEBOX_YES_NO_CANCEL)
    {
        msg.addButton(QMessageBox::Yes);
        msg.addButton(QMessageBox::No);
        msg.addButton(QMessageBox::Cancel);
    }
    if (lower==VMESSAGEBOX_OK_CANCEL)
    {
        msg.addButton(QMessageBox::Ok);
        msg.addButton(QMessageBox::Cancel);
    }
    if (upper&VMESSAGEBOX_INFO_TYPE)
        msg.setIcon(QMessageBox::Information);
    if (upper&VMESSAGEBOX_QUESTION_TYPE)
        msg.setIcon(QMessageBox::Question);
    if (upper&VMESSAGEBOX_WARNING_TYPE)
        msg.setIcon(QMessageBox::Warning);
    if (upper&VMESSAGEBOX_CRITICAL_TYPE)
        msg.setIcon(QMessageBox::Critical);

// Following doesn't always work correctly (at least on Qt: the dialog sometimes simply doesn't display!!!!!):
//  if (alwaysOnTop)
//      msg.setWindowFlags(Qt::WindowStaysOnTopHint);

    int result=msg.exec();

    if (result==QMessageBox::Cancel)
        return(VMESSAGEBOX_REPLY_CANCEL);
    if (result==QMessageBox::No)
        return(VMESSAGEBOX_REPLY_NO);
    if (result==QMessageBox::Yes)
        return(VMESSAGEBOX_REPLY_YES);
    if (result==QMessageBox::Ok)
        return(VMESSAGEBOX_REPLY_OK);
    return(VMESSAGEBOX_REPLY_CANCEL); // Can happen when the close button was clicked!
}
