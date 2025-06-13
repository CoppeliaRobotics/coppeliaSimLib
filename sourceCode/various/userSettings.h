#pragma once

#include <colorObject.h>

class CUserSettings
{
  public:
    CUserSettings();
    virtual ~CUserSettings();

    void setUndoRedoEnabled(bool isEnabled);
    bool getUndoRedoEnabled();
    bool getUndoRedoOnlyPartialWithCameras();

    void saveUserSettings(bool outputMsgs = true);
    void loadUserSettings();

    void setTranslationStepSize(double s);
    double getTranslationStepSize();
    void setRotationStepSize(double s);
    double getRotationStepSize();
    int getIdleFps();
    void setIdleFps(int fps);
    void setIdleFps_session(int fps);
    int getAbortScriptExecutionTiming();

    int getNextFreeServerPortToUse();
    void setNextFreeServerPortToUse(int p);

    std::string _overrideConsoleVerbosity;
    std::string _overrideStatusbarVerbosity;
    std::string _consoleLogFilter;
    std::string _overrideDialogVerbosity;
    bool undecoratedStatusbarMessages;
    bool timeStamp;
    bool displayWorldReference;
    bool useGlFinish;
    bool useGlFinish_visionSensors;
    bool oglCompatibilityTweak1;
    double stereoDist;
    int vsync;
    double identicalVertexTolerance;
    bool compressFiles;
    int triCountInOBB;
    bool runCustomizationScripts;
    bool runAddOns;
    bool test1;
    int macChildDialogType;

    int undoRedoLevelCount;
    int undoRedoMaxBufferSize;
    bool alwaysShowConsole;
    bool forceBugFix_rel30002;
    bool statusbarInitiallyVisible;
    bool modelBrowserInitiallyVisible;
    bool sceneHierarchyInitiallyVisible;
    bool sceneHierarchyHiddenDuringSimulation;
    int autoSaveDelay;
    int timeInMsForUndoRedoTooLongWarning;
    int bugFix1;
    int readDelay;
    int writeDelay;
    bool useBuffers; // differentiate binary strings and buffers in Lua, true since V4.7
    int scriptConversion;
    int notifyDeprecated;

    bool middleMouseButtonSwitchesModes;
    bool navigationBackwardCompatibility;
    double colorAdjust_backCompatibility;
    bool specificGpuTweak;
    bool disableOpenGlBasedCustomUi;
    bool disableVisibleEdges;
    bool showOldDlgs;
    bool enableOldRenderableBehaviour;
    bool enableOldMirrorObjects;
    int allowOldEduRelease;
    int threadedScriptsStoppingGraceTime;
    bool antiAliasing;
    double dynamicActivityRange;
    int freeServerPortStart;
    int freeServerPortRange;
    bool darkMode;
    int initWindowSize[2];
    int initWindowPos[2];
    int renderingSurfaceVShift;
    int renderingSurfaceVResize;
    int offscreenContextType;
    int fboType;
    bool forceFboViaExt;
    int vboOperation;
    int vboPersistenceInMs;
    int fileDialogs;
    std::string preferredSandboxLang;
    double mouseWheelZoomFactor;

    bool doNotWritePersistentData;
    bool doNotShowUpdateCheckMessage;
    bool doNotShowProgressBars;
    bool suppressStartupDialogs;
    bool suppressXmlOverwriteMsg;
    bool allowSettingsWrite;
    int noVersionCheck;

    std::string scriptEditorFont;
    bool scriptEditorBoldFont;
    int scriptEditorFontSize;

    std::string additionalLuaPath;
    std::string additionalPythonPath;
    std::string defaultPython;
    bool execUnsafe;
    bool execUnsafeExt;

    int guiFontSize_Win;
    int guiFontSize_Mac;
    int guiFontSize_Linux;

    std::string defaultDirectoryForScenes;
    std::string defaultDirectoryForModels;
    std::string defaultDirectoryForImportExport;
    std::string defaultDirectoryForMiscFiles;

    int codeColor_background[3];
    int codeColor_selection[3];
    int codeColor_comment[3];
    int codeColor_number[3];
    int codeColor_string[3];
    int codeColor_character[3];
    int codeColor_operator[3];
    int codeColor_preprocessor[3];
    int codeColor_identifier[3];
    int codeColor_word[3];
    int codeColor_word2[3];
    int codeColor_word3[3];
    int codeColor_word4[3];

    int highResDisplay;
    int guessedDisplayScalingThresholdFor2xOpenGl;
    int oglScaling;
    double guiScaling;
    bool noEdgesWhenMouseDownInCameraView;
    bool noTexturesWhenMouseDownInCameraView;
    bool noCustomUisWhenMouseDownInCameraView;
    int hierarchyRefreshCnt;
    int desktopRecordingIndex;
    int desktopRecordingWidth;
    std::string externalScriptEditor;
    int xmlExportSplitSize;
    bool xmlExportKnownFormats;

    std::string license_lite;
    std::string license_edu;
    std::string license_pro;
    std::string license;
    std::string licenseEndpoint;
    bool floatingLicenseEnabled;
    std::string floatingLicenseServer;
    int floatingLicensePort;
    bool keepDongleOpen;

    std::string mem;

  private:
    void _setIntVector3(int v[3], int a, int b, int c);
    bool _undoRedoEnabled;
    bool _undoRedoOnlyPartialWithCameras;
    double _translationStepSize;
    double _rotationStepSize;
    int _idleFps;
    int _idleFps_session;
    int _nextfreeServerPortToUse;
    int _abortScriptExecutionButton;
};
