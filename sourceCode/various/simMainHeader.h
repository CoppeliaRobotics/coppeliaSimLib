#ifndef SIM_MAINHEADER_INCLUDED
#define SIM_MAINHEADER_INCLUDED

#ifdef __cplusplus

#include "simConfig.h"

#define STRINGIFY(x) #x
#define TOSTR(x) STRINGIFY(x)

#if defined(_MSC_VER)
#if _MSC_VER == 1920
#define SIM_COMPILER_STR "MSVC2019"
#elif _MSC_VER == 1910
#define SIM_COMPILER_STR "MSVC2017"
#elif _MSC_VER == 1900
#define SIM_COMPILER_STR "MSVC2015"
#elif _MSC_VER >= 1930
#define SIM_COMPILER_STR "MSVC2022"
#else
#define SIM_COMPILER_STR "MSVC(unknown)"
#endif
#elif defined(__GNUC__)
#define SIM_COMPILER_STR "GCC " TOSTR(__GNUC__) "." TOSTR(__GNUC_MINOR__) "." TOSTR(__GNUC_PATCHLEVEL__)
#elif defined(__clang__)
#define SIM_COMPILER_STR "Clang " TOSTR(__clang_major__) "." TOSTR(__clang_minor__) "." TOSTR(__clang_patchlevel__)
#else
#define SIM_COMPILER_STR "UnknownCompiler"
#endif


#ifdef SIM_WITH_GUI
// If the GUI wants to write to shared resources, use following:
// (there is no guarantee that the access to the resources will be obtained!)
#define IF_UI_EVENT_CAN_WRITE_DATA \
    for (CSimAndUiThreadSync uiEventWriteData(__func__); uiEventWriteData.uiThread_tryToLockForUiEventWrite(1);)

// If the GUI wants to read shared resources, use following:
// (there is no guarantee that the access to the resources will be obtained!)
#define IF_UI_EVENT_CAN_READ_DATA \
    for (CSimAndUiThreadSync uiEventReadData(__func__); uiEventReadData.uiThread_tryToLockForUiEventRead(1);)
#define IF_UI_EVENT_CAN_READ_DATA_NO_WAIT \
    for (CSimAndUiThreadSync uiEventReadData(__func__); uiEventReadData.uiThread_tryToLockForUiEventRead(0);)

// If the SIM thread wants to give all rights to the GUI thread, use following:
#define SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING                \
    CSimAndUiThreadSync simThreadAllowAnythingForUiThread(__func__); \
    simThreadAllowAnythingForUiThread.simThread_temporarilyAllowUiThreadToReadAndWrite()

// If the SIM or GUI thread wants to write to shared resources, use following:
// (the API is accessible to both threads: SIM and GUI. Access via GUI should be avoided as much as possible,
// since there is never a guarantee that the GUI will obtain access to the shared resources (i.e. API
// function calls could unexpectedely return with a failure code). The SIM thread
// on the other hand will eventually always obtain access to the shared resources!)
#define IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA \
    for (CSimAndUiThreadSync writeData(__func__); writeData.simOrUiThread_tryToLockForWrite_cApi();)

// If the SIM or GUI thread wants to read to shared resources, use following:
// (the API is accessible to both threads: SIM and GUI. Access via GUI should be avoided as much as possible,
// since there is never a guarantee that the GUI will obtain access to the shared resources (i.e. API
// function calls could unexpectedely return with a failure code). The SIM thread
// on the other hand doesn't need to obtain read access to the shared resources (by default)!)

#define IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA \
    for (CSimAndUiThreadSync readData(__func__); readData.simOrUiThread_tryToLockForRead_cApi();)
#else
#define IF_UI_EVENT_CAN_WRITE_DATA
#define IF_UI_EVENT_CAN_READ_DATA
#define IF_UI_EVENT_CAN_READ_DATA_CMD(funcName)
#define IF_UI_EVENT_CAN_READ_DATA_NO_WAIT
#define SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING
#define IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
#define IF_C_API_SIM_OR_UI_THREAD_CAN_READ_DATA
#endif

// Trace commands:
#define C_API_START                                         \
    CFuncTrace funcTrace(__func__, sim_verbosity_traceall); \
    lastInfoString.clear()
#define TRACE_LUA_API CFuncTrace funcTrace(__func__, sim_verbosity_tracelua)
#define TRACE_INTERNAL CFuncTrace funcTrace(__func__, sim_verbosity_traceall)

#define SIMPLE_FUNCNAME_DEBUG printf("SYNC_DEBUG: %s, %s\n", typeid(*this).name(), __func__);

#ifdef WIN_SIM
#ifndef NOMINMAX
#define NOMINMAX
#endif
typedef void(__cdecl* VTHREAD_START_ADDRESS)(void*);
typedef unsigned int VTHREAD_ID_TYPE;
#define VTHREAD_RETURN_TYPE void
#define VTHREAD_RETURN_VAL void()
#define QT_MODAL_DLG_STYLE (Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint)
#define QT_MODELESS_DLG_STYLE (Qt::Tool)
#define QT_MODAL_SCINTILLA_DLG_STYLE (Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint)
#define QT_MODELESS_SCINTILLA_DLG_STYLE (Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint)
#else
typedef void* (*VTHREAD_START_ADDRESS)(void*);
#define VTHREAD_ID_TYPE pthread_t
#define VTHREAD_RETURN_TYPE void*
#define VTHREAD_RETURN_VAL 0
#define __stdcall __attribute__((stdcall))
#endif

#ifdef MAC_SIM
#define __cdecl
#define QT_MODAL_DLG_STYLE                  \
    (Qt::Dialog | Qt::CustomizeWindowHint | \
     Qt::WindowTitleHint) // Qt::Tool is buggy on Mac sinc Qt5.1 or so (modal dialogs are not 100% modal!)
#define QT_MODELESS_DLG_STYLE \
    (Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint) // Qt::Dialog is buggy on Mac (dialog doesn't stay 
                                                                     // on top. With Qt::Tool it stays on top)
#define QT_MODAL_SCINTILLA_DLG_STYLE (Qt::Dialog)
#define QT_MODELESS_SCINTILLA_DLG_STYLE (Qt::Tool) // Otherwise doesn't stay in front!!!
#endif

#ifdef LIN_SIM
#define __cdecl
#define QT_MODAL_DLG_STYLE (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint)
#define QT_MODELESS_DLG_STYLE (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint)
#define QT_MODAL_SCINTILLA_DLG_STYLE \
    (Qt::WindowCloseButtonHint) // WindowMaximizeButtonHint has a strange behaviour here. Can't get anything work right 
                                // except for a single close button!!!
#define QT_MODELESS_SCINTILLA_DLG_STYLE \
    (Qt::WindowCloseButtonHint) // WindowMaximizeButtonHint has a strange behaviour here. Can't get anything work right 
                                // except for a single close button!!!
#endif

#define VRGB(r, g, b) (quint32(quint8(r) | (quint8(g) << 8) | (quint8(b) << 16)))
#define VRGBW(a)                                                                                                    \
    (static_cast<unsigned int>(                                                                                     \
        (static_cast<unsigned char>(a[0]) | (static_cast<unsigned short>(static_cast<unsigned char>(a[1])) << 8)) | \
        ((static_cast<unsigned int>(static_cast<unsigned char>(a[2]))) << 16)))
#define VTHREAD_ARGUMENT_TYPE void*
#define VTHREAD_ID_DEAD 0

typedef unsigned int(__cdecl* SIMPLE_VTHREAD_START_ADDRESS)(void*);
#define SIMPLE_VTHREAD_RETURN_TYPE unsigned int
#define SIMPLE_VTHREAD_RETURN_VAL 0
#define SIMPLE_VTHREAD_ARGUMENT_TYPE void*
#define STRCONCAT(a, b) a b

#endif // __cplusplus
#endif // SIM_MAINHEADER_INCLUDED
