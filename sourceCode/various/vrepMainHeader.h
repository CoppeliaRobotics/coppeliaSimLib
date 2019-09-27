#ifndef VREP_MAINHEADER_INCLUDED
#define VREP_MAINHEADER_INCLUDED

#ifdef __cplusplus

#include "vrepConfig.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <map>

#ifdef SIM_WITHOUT_QT_AT_ALL
    #define IF_UI_EVENT_CAN_WRITE_DATA if(true)
    #define IF_UI_EVENT_CAN_WRITE_DATA_CMD(funcName) if(true)
    #define IF_UI_EVENT_CAN_READ_DATA if(true)
    #define IF_UI_EVENT_CAN_READ_DATA_CMD(funcName) if(true)
    #define IF_UI_EVENT_CAN_READ_DATA_NO_WAIT if(true)
    #define SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING
    #define IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA if(true)
    #define IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA if(true)
    typedef unsigned int quint32;
    typedef unsigned long long int quint64;
#else
    // If the GUI wants to write to shared resources, use following:
    // (there is no guarantee that the access to the resources will be obtained!)
    #define IF_UI_EVENT_CAN_WRITE_DATA for(CSimAndUiThreadSync uiEventWriteData(__func__);uiEventWriteData.uiThread_tryToLockForUiEventWrite(800);)
    #define IF_UI_EVENT_CAN_WRITE_DATA_CMD(funcName) for(CSimAndUiThreadSync uiEventWriteData(funcName);uiEventWriteData.uiThread_tryToLockForUiEventWrite(800);)

    // If the GUI wants to read shared resources, use following:
    // (there is no guarantee that the access to the resources will be obtained!)
    #define IF_UI_EVENT_CAN_READ_DATA for(CSimAndUiThreadSync uiEventReadData(__func__);uiEventReadData.uiThread_tryToLockForUiEventRead(5);)
    #define IF_UI_EVENT_CAN_READ_DATA_CMD(funcName) for(CSimAndUiThreadSync uiEventReadData(funcName);uiEventReadData.uiThread_tryToLockForUiEventRead(5);)
    #define IF_UI_EVENT_CAN_READ_DATA_NO_WAIT for(CSimAndUiThreadSync uiEventReadData(__func__);uiEventReadData.uiThread_tryToLockForUiEventRead(0);)

    // If the SIM thread wants to give all rights to the GUI thread, use following:
    #define SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING CSimAndUiThreadSync simThreadAllowAnythingForUiThread(__func__);simThreadAllowAnythingForUiThread.simThread_temporarilyAllowUiThreadToReadAndWrite()

    // If the SIM or GUI thread wants to write to shared resources, use following:
    // (the API is accessible to both threads: SIM and GUI. Access via GUI should be avoided as much as possible,
    // since there is never a guarantee that the GUI will obtain access to the shared resources (i.e. API
    // function calls could unexpectedely return with a failure code). The SIM thread
    // on the other hand will eventually always obtain access to the shared resources!)
    #define IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA  for(CSimAndUiThreadSync writeData(__func__);writeData.simOrUiThread_tryToLockForWrite_cApi();)

    // If the SIM or GUI thread wants to read to shared resources, use following:
    // (the API is accessible to both threads: SIM and GUI. Access via GUI should be avoided as much as possible,
    // since there is never a guarantee that the GUI will obtain access to the shared resources (i.e. API
    // function calls could unexpectedely return with a failure code). The SIM thread
    // on the other hand doesn't need to obtain read access to the shared resources (by default)!)

    #define IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA  for(CSimAndUiThreadSync readData(__func__);readData.simOrUiThread_tryToLockForRead_cApi();)
#endif // SIM_WITHOUT_QT_AT_ALL

// Debug commands:
#define FUNCTION_DEBUG CFuncDebug funcDebug(__func__,1)
#define FUNCTION_INSIDE_DEBUG(theTExt) CFuncDebug::print(theTExt,1)
#define C_API_FUNCTION_DEBUG CFuncDebug funcDebug(__func__,2)
#define LUA_API_FUNCTION_DEBUG CFuncDebug funcDebug(__func__,4)
#define MUST_BE_UI_THREAD

// Resource lock command:
#define EASYLOCK(mutex) CEasyLock easyLock(mutex,__func__)

// Undo point announcements:
#ifdef SIM_WITH_GUI
    #define POST_SCENE_CHANGED_ANNOUNCEMENT(theTExt) if(VThread::isCurrentThreadTheUiThread()){SSimulationThreadCommand cmd;cmd.cmdId=999999;cmd.stringParams.push_back(theTExt);App::appendSimulationThreadCommand(cmd);}else{App::ct->undoBufferContainer->announceChange();}
    #define POST_SCENE_CHANGED_CLEAR_REDO_BUFFER_ANNOUNCEMENT() if(VThread::isCurrentThreadTheUiThread()){SSimulationThreadCommand cmd;cmd.cmdId=999995;cmd.stringParams.push_back("");App::appendSimulationThreadCommand(cmd);}else{App::ct->undoBufferContainer->emptyRedoBuffer();}
    #define POST_SCENE_CHANGED_GRADUAL_ANNOUNCEMENT(theTExt) if(VThread::isCurrentThreadTheUiThread()){SSimulationThreadCommand cmd;cmd.cmdId=999996;cmd.stringParams.push_back(theTExt);App::appendSimulationThreadCommand(cmd);}else{App::ct->undoBufferContainer->announceChangeGradual();}
    #define POST_SCENE_CHANGED_START_ANNOUNCEMENT(theTExt) if(VThread::isCurrentThreadTheUiThread()){SSimulationThreadCommand cmd;cmd.cmdId=999997;cmd.stringParams.push_back(theTExt);App::appendSimulationThreadCommand(cmd);}else{App::ct->undoBufferContainer->announceChangeStart();}
    #define POST_SCENE_CHANGED_END_ANNOUNCEMENT() if(VThread::isCurrentThreadTheUiThread()){SSimulationThreadCommand cmd;cmd.cmdId=999998;cmd.stringParams.push_back("");App::appendSimulationThreadCommand(cmd);}else{App::ct->undoBufferContainer->announceChangeEnd();}
#else
    #define POST_SCENE_CHANGED_ANNOUNCEMENT(theTExt)
    #define POST_SCENE_CHANGED_CLEAR_REDO_BUFFER_ANNOUNCEMENT()
    #define POST_SCENE_CHANGED_GRADUAL_ANNOUNCEMENT(theTExt)
    #define POST_SCENE_CHANGED_START_ANNOUNCEMENT(theTExt)
    #define POST_SCENE_CHANGED_END_ANNOUNCEMENT()
#endif

#ifdef WIN_VREP
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    typedef void( __cdecl* VTHREAD_START_ADDRESS )( void* );
    typedef unsigned int VTHREAD_ID_TYPE;
    typedef unsigned long VTHREAD_AFFINITY_MASK;
    #define VTHREAD_RETURN_TYPE void
    #define VTHREAD_RETURN_VAL void()
    #define QT_MODAL_DLG_STYLE (Qt::Tool|Qt::CustomizeWindowHint|Qt::WindowTitleHint)
    #define QT_MODELESS_DLG_STYLE (Qt::Tool)
    #define QT_MODAL_SCINTILLA_DLG_STYLE (Qt::Dialog|Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint)
    #define QT_MODELESS_SCINTILLA_DLG_STYLE (Qt::Dialog|Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint)
#else
    typedef void*(*VTHREAD_START_ADDRESS)(void *);
    #define VTHREAD_ID_TYPE pthread_t
    typedef unsigned int VTHREAD_AFFINITY_MASK;
    #define VTHREAD_RETURN_TYPE void*
    #define VTHREAD_RETURN_VAL 0
    #define __stdcall __attribute__((stdcall))
#endif

#ifdef MAC_VREP
    #define __cdecl
    #define QT_MODAL_DLG_STYLE (Qt::Dialog|Qt::CustomizeWindowHint|Qt::WindowTitleHint) //Qt::Tool is buggy on Mac sinc Qt5.1 or so (modal dialogs are not 100% modal!)
    #define QT_MODELESS_DLG_STYLE (Qt::Tool|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint) //Qt::Dialog is buggy on Mac (dialog doesn't stay on top. With Qt::Tool it stays on top)
    #define QT_MODAL_SCINTILLA_DLG_STYLE (Qt::Dialog) 
    #define QT_MODELESS_SCINTILLA_DLG_STYLE (Qt::Tool) // Otherwise doesn't stay in front!!!
#endif

#ifdef LIN_VREP
    #define __cdecl
    #define QT_MODAL_DLG_STYLE (Qt::Dialog|Qt::CustomizeWindowHint|Qt::WindowTitleHint)
    #define QT_MODELESS_DLG_STYLE (Qt::Dialog|Qt::CustomizeWindowHint|Qt::WindowCloseButtonHint)
    #define QT_MODAL_SCINTILLA_DLG_STYLE (Qt::WindowCloseButtonHint) // WindowMaximizeButtonHint has a strange behaviour here. Can't get anything work right except for a single close button!!!
    #define QT_MODELESS_SCINTILLA_DLG_STYLE (Qt::WindowCloseButtonHint) // WindowMaximizeButtonHint has a strange behaviour here. Can't get anything work right except for a single close button!!!
#endif

typedef unsigned long long int suint64;
#define VRGB(r,g,b) (quint32(quint8(r)|(quint8(g) << 8)|(quint8(b) << 16)))
#define VRGBW(a) (static_cast<unsigned int>((static_cast<unsigned char>(a[0])|(static_cast<unsigned short>(static_cast<unsigned char>(a[1]))<<8))|((static_cast<unsigned int>(static_cast<unsigned char>(a[2])))<<16)))
#define VTHREAD_ARGUMENT_TYPE void*
#define VTHREAD_ID_DEAD 0

#ifdef SIM_WITHOUT_QT_AT_ALL
    typedef VTHREAD_START_ADDRESS SIMPLE_VTHREAD_START_ADDRESS;
    #define SIMPLE_VTHREAD_RETURN_TYPE VTHREAD_RETURN_TYPE
    #define SIMPLE_VTHREAD_RETURN_VAL VTHREAD_RETURN_VAL
    #define SIMPLE_VTHREAD_ARGUMENT_TYPE VTHREAD_ARGUMENT_TYPE
#else
    typedef unsigned int(__cdecl* SIMPLE_VTHREAD_START_ADDRESS)(void*);
    #define SIMPLE_VTHREAD_RETURN_TYPE unsigned int
    #define SIMPLE_VTHREAD_RETURN_VAL 0
    #define SIMPLE_VTHREAD_ARGUMENT_TYPE void*
#endif


#endif // __cplusplus
#endif // VREP_MAINHEADER_INCLUDED
