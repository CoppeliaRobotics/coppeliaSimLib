#pragma once

#include <string>
#include <simTypes.h>

class CSimRecorder
{
public:
    CSimRecorder(const char* initialPath);
    virtual ~CSimRecorder();

    bool recordFrameIfNeeded(int resX,int resY,int posX,int posY);
    bool willNextFrameBeRecorded();
    bool getIsRecording();
    bool startRecording(bool manualStart);
    void stopRecording(bool manualStop);
    void setRecordEveryXRenderedFrame(int x);
    int getRecordEveryXRenderedFrame();
    void setFrameRate(int fps);
    int getFrameRate();
    int getFrameCount();
    void setPath(const char* path);
    void setPathAndFilename(const char* pathAndF); // when set via API
    void setRecorderEnabled(bool e);
    bool getRecorderEnabled();
    std::string getPath(char* userSet);
    void setShowCursor(bool show);
    bool getShowCursor();
    void setShowButtonStates(bool show);
    bool getShowButtonStates();
    bool getManualStart();
    bool getHideInfoTextAndStatusBar();
    void setHideInfoTextAndStatusBar(bool hide);
    void setRecordingSizeChanged(int newXsize,int newYsize);
    void getRecordingSize(int& x, int&y);
    void setAutoFrameRate(bool a);
    bool getAutoFrameRate();

    void setDesktopRecording(bool dr);
    bool getDesktopRecording() const;

    void setEncoderIndex(int index);
    int getEncoderIndex();
    std::string getEncoderString(int index);

    void setShowSavedMessage(bool s);

protected:

    bool _isRecording;
    int _frameCount;
    int _simulationFrameCount;
    int _recordEveryXRenderedFrame;
    bool _initFailed;
    int _frameRate;
    bool _recordDesktopInstead;
    int _resolution[2];
    int _resolutionInfo[2];
    int _position[2];
    bool _recorderEnabled;
    bool _showCursor;
    bool _showButtonStates;
    bool _hideInfoTextAndStatusBar;
    bool _autoFrameRate;
    bool _manualStart;
    std::string _path;
    std::string _pathAndFilename;
    std::string _filenameAndPathAndExtension;
    int _encoderIndex;
    bool _showSavedMessage;
    double _simulationTimeOfLastFrame;

    bool _aviGenInitialized;

    unsigned char* _buffer;
    unsigned char* _tempBuffer;
};
