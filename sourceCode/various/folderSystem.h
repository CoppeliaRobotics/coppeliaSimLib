#pragma once

class CFolderSystem
{
public:
    CFolderSystem();
    virtual ~CFolderSystem();

    static std::string getPathFromFull(const char* full); // no final backslash or slash is returned
    static std::string getNameFromFull(const char* full);

    std::string getExecutablePath() const;
    std::string getSystemPath() const;
    std::string getScenesPath() const;
    void setScenesPath(const char* path);
    std::string getModelsPath() const;
    void setModelsPath(const char* path);
    std::string getCadFilesPath() const;
    void setCadFilesPath(const char* path);
    std::string getTexturesPath() const;
    void setTexturesPath(const char* path);
    std::string getVideosPath() const;
    void setVideosPath(const char* path);
    std::string getOtherFilesPath() const;
    void setOtherFilesPath(const char* path);
    std::string getExtScriptEditorTempPath() const;
    void setExtScriptEditorTempPath(const char* path);
    std::string getRemoteApiTempPath() const;
    void setRemoteApiTempPath(const char* path);

private:
    std::string _executablePath;
    std::string _systemPath;
    std::string _scenesPath;
    std::string _modelsPath;
    std::string _cadFilesPath;
    std::string _texturesPath;
    std::string _videosPath;
    std::string _otherFilesPath;
    std::string _extScriptEditorTempPath;
    std::string _remoteApiTempPath;
};
