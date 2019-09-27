
#include "vrepMainHeader.h"
#include "vrepQApp.h"
#include "app.h"

CVrepQApp::CVrepQApp(int& argc ,char** argv) : QAPP(argc,argv)
{
}

CVrepQApp::~CVrepQApp()
{
}

bool CVrepQApp::notify(QObject* object,QEvent* event)
{
#ifdef SIM_WITH_GUI
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent* keyEvent=static_cast<QKeyEvent*>(event);
        int key=keyEvent->key();
        if (key==Qt::Key_Escape)
            App::setFullScreen(false);
    }
#endif
    return(QAPP::notify(object,event));
};
