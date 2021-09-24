#pragma once

#ifdef SIM_WITH_QT
    #include <QTemporaryDir>
#endif

class CFolderSystem
{
public:
    CFolderSystem();
    virtual ~CFolderSystem();

    static std::string getPathFromFull(const char* full); // no final backslash or slash is returned
    static std::string getNameFromFull(const char* full);

    std::string getExecutablePath() const;
    std::string getSystemPath() const;
    std::string getResourcesPath() const;
    std::string getScenesPath() const;
    void setScenesPath(const char* path);
    std::string getModelsPath() const;
    void setModelsPath(const char* path);
    std::string getImportExportPath() const;
    void setImportExportPath(const char* path);
    std::string getTexturesPath() const;
    void setTexturesPath(const char* path);
    std::string getVideosPath() const;
    void setVideosPath(const char* path);
    std::string getOtherFilesPath() const;
    void setOtherFilesPath(const char* path);
    std::string getAppDataPath() const;
    std::string getTempDataPath() const;
    std::string getSceneTempDataPath() const;

private:
#ifdef SIM_WITH_QT
    QTemporaryDir* _tempDir;
#endif
    std::string _executablePath;
    std::string _systemPath;
    std::string _resourcesPath;
    std::string _scenesPath;
    std::string _modelsPath;
    std::string _importExportPath;
    std::string _texturesPath;
    std::string _videosPath;
    std::string _otherFilesPath;
    std::string _appDataPath;
    std::string _tempDataPath;
};
