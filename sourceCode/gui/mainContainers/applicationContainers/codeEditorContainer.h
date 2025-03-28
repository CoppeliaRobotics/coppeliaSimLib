#pragma once

#include <scriptObject.h>
#include <vector>
#include <QString>
#include <tinyxml2.h>

struct SCodeEditor
{
    int handle;
    int uniqueId;
    int scriptHandle;
    long long int scriptUid;
    int callingScriptHandle;
    long long int callingScriptUid;
    int sceneUniqueId;
    bool openAcrossScenes;
    bool closeAtSimulationEnd;
    bool systemVisibility;
    bool userVisibility;
    bool closeAfterCallbackCalled;
    bool restartScriptWhenClosing;
    std::string callbackFunction;
};

class CCodeEditorContainer
{
  public:
    CCodeEditorContainer();
    virtual ~CCodeEditorContainer();

    // From UI thread:
    void showOrHideAll(bool showState);
    void closeAll();

    // From sim thread:
    int openScriptWithExternalEditor(int scriptHandle);
    void announceScriptStateWillBeErased(int scriptHandle, long long int scriptUid);

    int open(const char* initText, const char* xml, int callingScriptHandle); // func. sim.textEditorOpen
    int openSimulationScript(int scriptHandle);                               // main and simulation scripts
    int openCustomizationScript(int scriptHandle);                            // customization scripts
    int openConsole(const char* title, int maxLines, int mode, const int position[2], const int size[2],
                    const int textColor[3], const int backColor[3], int callingScriptHandle);
    std::string openModalTextEditor(const char* initText, const char* xml, int windowSizeAndPos[4],
                                    bool oldXml) const; // modal C func. sim.openTextEditor
    int openTextEditor_old(const char* initText, const char* xml, const char* callback,
                           const CScriptObject* requestOrigin); // deprec. func. sim.openTextEditor
    bool close(int handle, int posAndSize[4], std::string* txt, std::string* callback);
    void applyChanges(int handle) const;
    bool closeFromScriptUid(long long int scriptUid, int posAndSize[4], bool ignoreChange);
    void restartScript(int handle) const;
    int getCallingScriptHandle(int handle) const;
    bool getCloseAfterCallbackCalled(int handle) const;
    void simulationAboutToStart() const;
    void simulationAboutToEnd();
    void saveOrCopyOperationAboutToHappen() const;
    bool setText(int handle, const char* txt) const;
    std::string getText(int handle, int posAndSize[4]) const;
    bool isHandleValid(int handle) const;
    int showOrHide(int handle, bool showState);
    int getShowState(int handle) const;
    bool appendText(int handle, const char* txt) const;
    bool hasSomethingBeenModifiedInCurrentScene() const;

    // From any thread:
    int getHandleFromUniqueId(int uid);
    int getUniqueId(int handle);
    bool areSceneEditorsOpen() const;
    void sceneClosed(int sceneUniqueId);
    static std::string getColorStr(const int rgbCol[3]);
    static void getKeywords(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentNode,
                            const CScriptObject* requestOrigin);
    static void getFuncKeywords(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentNode,
                                const CScriptObject* requestOrigin);
    static void getVarKeywords(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parentNode,
                               const CScriptObject* requestOrigin);
    static std::string translateXml(const char* oldXml, const char* callback, const CScriptObject* requestOrigin);
    static const char* toBoolStr(bool v);

  protected:
    std::vector<SCodeEditor> _allEditors;
    static int _nextUniqueId;
};
