#pragma once

#include <simMath/3Vector.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>
#include <sceneObject.h>

struct SUIThreadCommand
{
    int cmdId;
    std::vector<bool> boolParams;
    std::vector<int> intParams;
    std::vector<unsigned int> uintParams;
    std::vector<double> floatParams;
    std::vector<void *> objectParams;
    std::vector<std::string> stringParams;
    std::vector<C3Vector> posParams;
    std::vector<C4Vector> quatParams;
    std::vector<C7Vector> transfParams;
};

enum
{
    VMESSAGEBOX_OKELI = 0,
    VMESSAGEBOX_YES_NO = 1,
    VMESSAGEBOX_YES_NO_CANCEL = 2,
    VMESSAGEBOX_OK_CANCEL = 3,
};

enum
{
    VMESSAGEBOX_REPLY_CANCEL = 0,
    VMESSAGEBOX_REPLY_NO = 1,
    VMESSAGEBOX_REPLY_YES = 2,
    VMESSAGEBOX_REPLY_OK = 3,
    VMESSAGEBOX_REPLY_ERROR = 4,
};

enum
{
    OPEN_OR_CLOSE_UITHREADCMD = 0,

    CREATE_DEFAULT_MENU_BAR_UITHREADCMD,
    KEEP_THUMBNAIL_QUESTION_DLG_UITHREADCMD,
    SELECT_THUMBNAIL_DLG_UITHREADCMD,
    HEIGHTFIELD_DIMENSION_DLG_UITHREADCMD,
    OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_UITHREADCMD,
    OPEN_MODAL_MODEL_PROPERTIES_UITHREADCMD,
    REFRESH_DIALOGS_UITHREADCMD,
    CALL_DIALOG_FUNCTION_UITHREADCMD,
    TOGGLE_VISUALIZATION_UITHREADCMD,
    VISUALIZATION_OFF_UITHREADCMD,
    VISUALIZATION_ON_UITHREADCMD,
    OPEN_HIERARCHY_UITHREADCMD,
    CLOSE_HIERARCHY_UITHREADCMD,

    MAIN_WINDOW_START_MWUITHREADCMD,
    MAIN_WINDOW_NON_EDIT_MODE_DLG_CLOSE_MWUITHREADCMD,
    MAIN_WINDOW_NON_EDIT_MODE_DLG_REOPEN_MWUITHREADCMD,
    MAIN_WINDOW_VIEW_SELECTOR_DLG_CLOSE_MWUITHREADCMD,
    MAIN_WINDOW_VIEW_SELECTOR_DLG_REOPEN_MWUITHREADCMD,
    MAIN_WINDOW_PAGE_SELECTOR_DLG_CLOSE_MWUITHREADCMD,
    MAIN_WINDOW_PAGE_SELECTOR_DLG_REOPEN_MWUITHREADCMD,
    MAIN_WINDOW_SET_FULLSCREEN_MWTHREADCMD,
    MAIN_WINDOW_ACTIVATE_MWUITHREADCMD,
    MAIN_WINDOW_CLOSE_DLG_MWUITHREADCMD,
    MAIN_WINDOW_OPEN_DLG_OR_BRING_TO_FRONT_MWUITHREADCMD,
    MAIN_WINDOW_END_MWUITHREADCMD,

    DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD,
    SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD,
    DISPLAY_MESH_DECIMATION_DIALOG_UITHREADCMD,

    DISPLAY_OR_HIDE_PROGRESS_DIALOG_UITHREADCMD,

    PLUGIN_START_PLUGUITHREADCMD,
    PLUGIN_LOAD_AND_START_PLUGUITHREADCMD,
    PLUGIN_STOP_AND_UNLOAD_PLUGUITHREADCMD,
    PLUGIN_END_PLUGUITHREADCMD,

    DESTROY_GL_TEXTURE_UITHREADCMD,
    DISPLAY_FILE_DLG_UITHREADCMD,
    DISPLAY_MSG_WITH_CHECKBOX_UITHREADCMD,
    DISPLAY_MSGBOX_UITHREADCMD,
    DETECT_VISION_SENSOR_ENTITY_UITHREADCMD,
    DISPLAY_SAVE_DLG_UITHREADCMD,
    DISPLAY_OPEN_DLG_UITHREADCMD,
    DISPLAY_OPEN_DLG_MULTIFILE_UITHREADCMD,
    DLG_INPUT_GET_FLOAT_UITHREADCMD,
    CREATE_GL_CONTEXT_FBO_TEXTURE_IF_NEEDED_UITHREADCMD,
    SET_FILEDIALOG_NATIVE_UITHREADCMD,
    SHOW_PRIMITIVE_SHAPE_DLG_UITHREADCMD,

    LOG_MSG_TO_STATUSBAR_UITHREADCMD,
    CLEAR_STATUSBAR_UITHREADCMD,

    INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD,
    INSTANCE_WAS_JUST_CREATED_UITHREADCMD,
    INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD,
    INSTANCE_HAS_CHANGE_UITHREADCMD,
    INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD,
    NEW_SCENE_NAME_UITHREADCMD,
    SIMULATION_ABOUT_TO_START_UITHREADCMD,
    SIMULATION_JUST_ENDED_UITHREADCMD,
    EDIT_MODE_ABOUT_TO_START_UITHREADCMD,
    EDIT_MODE_JUST_ENDED_UITHREADCMD,
    COPY_TEXT_TO_CLIPBOARD_UITHREADCMD,
    INSTANCE_PASS_FROM_UITHREAD_UITHREADCMD,
    CALL_PLUGIN_INITUI_FROM_UITHREAD_UITHREADCMD,
    CALL_PLUGIN_CLEANUPUI_FROM_UITHREAD_UITHREADCMD,
    KY_UITHREADCMD,
    RG_UITHREADCMD,

    MENUBAR_COLOR_UITHREADCMD,
};

#include <QObject>
class CUiThread : public QObject
{
    Q_OBJECT

  public:
    CUiThread();
    virtual ~CUiThread();

    bool executeCommandViaUiThread(SUIThreadCommand *cmdIn, SUIThreadCommand *cmdOut);
    void showOrHideProgressBar(bool show, double pos = 999.0, const char *txt = nullptr);
    bool showOrHideEmergencyStop(bool show, const char *txt);

  private:
    volatile int _frameRendered;

  signals:
    void _executeCommandViaUiThread(SUIThreadCommand *cmdIn, SUIThreadCommand *cmdOut);

  private slots:
    void __executeCommandViaUiThread(SUIThreadCommand *cmdIn, SUIThreadCommand *cmdOut);

  public:
    void renderScene();
    void setFrameRendered();
    bool messageBox_checkbox(void *parentWidget, const char *title, const char *message, const char *checkboxMessage,
                             bool isWarning);
    void setFileDialogsNative(int n);
    std::string getOpenFileName(void *parentWidget, unsigned short option, const char *title, const char *startPath,
                                const char *initFilename, bool allowAnyFile, const char *extensionName,
                                const char *extension1, const char *extension2 = "", const char *extension3 = "",
                                const char *extension4 = "", const char *extension5 = "", const char *extension6 = "",
                                const char *extension7 = "", const char *extension8 = "", const char *extension9 = "",
                                const char *extension10 = "");
    bool getOpenFileNames(std::vector<std::string> &files, void *parentWidget, unsigned short option, const char *title,
                          const char *startPath, const char *initFilename, bool allowAnyFile, const char *extensionName,
                          const char *extension1, const char *extension2 = "", const char *extension3 = "",
                          const char *extension4 = "", const char *extension5 = "", const char *extension6 = "",
                          const char *extension7 = "", const char *extension8 = "", const char *extension9 = "",
                          const char *extension10 = "");
    std::string getSaveFileName(void *parentWidget, unsigned short option, const char *title, const char *startPath,
                                const char *initFilename, bool allowAnyFile, const char *extensionName,
                                const char *extension1, const char *extension2 = "", const char *extension3 = "",
                                const char *extension4 = "", const char *extension5 = "", const char *extension6 = "",
                                const char *extension7 = "", const char *extension8 = "", const char *extension9 = "",
                                const char *extension10 = "");
    std::string getOpenOrSaveFileName_api(int mode, const char *title, const char *startPath, const char *initName,
                                          const char *extName, const char *ext);
    bool showPrimitiveShapeDialog(int type, const C3Vector *optionalSizesIn, C3Vector &sizes, int subdiv[3],
                                  int &facesSubdiv, int &sides, int &discSubdiv, bool &smooth, bool &openEnds,
                                  bool &dynamic, double &density);
    unsigned short messageBox_informationSystemModal(void *parentWidget, const char *title, const char *message,
                                                     unsigned short flags, unsigned short defaultAnswer);
    unsigned short messageBox_information(void *parentWidget, const char *title, const char *message,
                                          unsigned short flags, unsigned short defaultAnswer);
    unsigned short messageBox_question(void *parentWidget, const char *title, const char *message, unsigned short flags,
                                       unsigned short defaultAnswer);
    unsigned short messageBox_warning(void *parentWidget, const char *title, const char *message, unsigned short flags,
                                      unsigned short defaultAnswer);
    unsigned short messageBox_critical(void *parentWidget, const char *title, const char *message, unsigned short flags,
                                       unsigned short defaultAnswer);
    bool checkExecuteUnsafeOk(const char *what, const char *arg, const char *idStr);
    bool dialogInputGetFloat(void *parentWidget, const char *title, const char *msg, double def, double minV,
                             double maxV, int decimals, double *outFloat);

  private:
    unsigned short _messageBox(int type, void *parentWidget, const char *title, const char *message,
                               unsigned short flags, unsigned short defaultAnswer);

  signals:
    void _renderScene();

  private slots:
    void __renderScene();
};
