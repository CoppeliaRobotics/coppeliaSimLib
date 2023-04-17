#include <codeEditorContainer.h>
#include <pluginContainer.h>
#include <simInternal.h>
#include <simStrings.h>
#include <vVarious.h>
#include <app.h>
#include <utils.h>

int CCodeEditorContainer::_nextUniqueId=0;

const char* CCodeEditorContainer::toBoolStr(bool v)
{
    static const char _true[]="true";
    static const char _false[]="false";
    if (v)
        return(_true);
    return(_false);
}

std::string CCodeEditorContainer::getColorStr(const int rgbCol[3])
{
    QString retVal=QString("%1 %2 %3").arg(rgbCol[0]).arg(rgbCol[1]).arg(rgbCol[2]);
    return(retVal.toStdString());
}

void CCodeEditorContainer::getKeywords(sim::tinyxml2::XMLDocument* doc,sim::tinyxml2::XMLElement* parentNode,int scriptType,bool threaded)
{
    sim::tinyxml2::XMLElement* keywords1Node=doc->NewElement("keywords1");
    parentNode->InsertEndChild(keywords1Node);
    getFuncKeywords(doc,keywords1Node,scriptType,threaded);
    sim::tinyxml2::XMLElement* keywords2Node=doc->NewElement("keywords2");
    parentNode->InsertEndChild(keywords2Node);
    getVarKeywords(doc,keywords2Node,scriptType,threaded);
}

void CCodeEditorContainer::getFuncKeywords(sim::tinyxml2::XMLDocument* doc,sim::tinyxml2::XMLElement* parentNode,int scriptType,bool threaded)
{
    std::vector<std::string> t;
    CScriptObject::getMatchingFunctions("",t); // basically all functions
    for (size_t i=0;i<t.size();i++)
    {
        std::string tip(CScriptObject::getFunctionCalltip(t[i].c_str()));
        sim::tinyxml2::XMLElement* itemNode=doc->NewElement("item");
        parentNode->InsertEndChild(itemNode);
        itemNode->SetAttribute("word",t[i].c_str());
        itemNode->SetAttribute("autocomplete",toBoolStr(true));
        itemNode->SetAttribute("calltip",tip.c_str());
    }
}

void CCodeEditorContainer::getVarKeywords(sim::tinyxml2::XMLDocument* doc,sim::tinyxml2::XMLElement* parentNode,int scriptType,bool threaded)
{
    std::vector<std::string> t;
    CScriptObject::getMatchingConstants("",t); // basically all constants
    for (size_t i=0;i<t.size();i++)
    {
        sim::tinyxml2::XMLElement* itemNode=doc->NewElement("item");
        parentNode->InsertEndChild(itemNode);
        itemNode->SetAttribute("word",t[i].c_str());
        itemNode->SetAttribute("autocomplete",toBoolStr(true));
    }
}

std::string CCodeEditorContainer::translateXml(const char* oldXml,const char* callback)
{
    sim::tinyxml2::XMLDocument xmlNewDoc;
    sim::tinyxml2::XMLElement* editorNode=xmlNewDoc.NewElement("editor");
    xmlNewDoc.InsertFirstChild(editorNode);

    if (strlen(callback)>0)
        editorNode->SetAttribute("on-close",callback);
    if (oldXml!=nullptr)
    {
        sim::tinyxml2::XMLDocument xmldoc;
        sim::tinyxml2::XMLError error=xmldoc.Parse(oldXml);
        if(error==sim::tinyxml2::XML_NO_ERROR)
        {
            sim::tinyxml2::XMLElement* rootElement=xmldoc.FirstChildElement();
            const char* val=rootElement->Attribute("title");
            if (val!=nullptr)
                editorNode->SetAttribute("title",val);
            val=rootElement->Attribute("editable");
            if (val!=nullptr)
                editorNode->SetAttribute("editable",val);
            val=rootElement->Attribute("searchable");
            if (val!=nullptr)
                editorNode->SetAttribute("searchable",val);
            val=rootElement->Attribute("tabWidth");
            if (val!=nullptr)
                editorNode->SetAttribute("tab-width",val);
            val=rootElement->Attribute("textColor");
            if (val!=nullptr)
                editorNode->SetAttribute("text-col",val);
            val=rootElement->Attribute("backgroundColor");
            if (val!=nullptr)
                editorNode->SetAttribute("background-col",val);
            val=rootElement->Attribute("selectionColor");
            if (val!=nullptr)
                editorNode->SetAttribute("selection-col",val);
            val=rootElement->Attribute("size");
            if (val!=nullptr)
                editorNode->SetAttribute("size",val);
            val=rootElement->Attribute("position");
            if (val!=nullptr)
                editorNode->SetAttribute("position",val);
            val=rootElement->Attribute("isLua");
            if (val!=nullptr)
            {
                if (std::string(val).compare("true"))
                    editorNode->SetAttribute("lang","lua");
            }
            val=rootElement->Attribute("commentColor");
            if (val!=nullptr)
                editorNode->SetAttribute("comment-col",val);
            val=rootElement->Attribute("numberColor");
            if (val!=nullptr)
                editorNode->SetAttribute("number-col",val);
            val=rootElement->Attribute("stringColor");
            if (val!=nullptr)
                editorNode->SetAttribute("string-col",val);
            val=rootElement->Attribute("characterColor");
            if (val!=nullptr)
                editorNode->SetAttribute("character-col",val);
            val=rootElement->Attribute("operatorColor");
            if (val!=nullptr)
                editorNode->SetAttribute("operator-col",val);
            val=rootElement->Attribute("identifierColor");
            if (val!=nullptr)
                editorNode->SetAttribute("identifier-col",val);
            val=rootElement->Attribute("preprocessorColor");
            if (val!=nullptr)
                editorNode->SetAttribute("preprocessor-col",val);
            val=rootElement->Attribute("wordColor");
            if (val!=nullptr)
                editorNode->SetAttribute("keyword3-col",val);
            val=rootElement->Attribute("word4Color");
            if (val!=nullptr)
                editorNode->SetAttribute("keyword4-col",val);
            bool csimKeywords=false;
            val=rootElement->Attribute("useCoppeliaSimKeywords");
            if (val!=nullptr)
                csimKeywords=(strcmp(val,"true")==0);
            else
            {
                val=rootElement->Attribute("useVrepKeywords");
                if (val!=nullptr)
                    csimKeywords=(strcmp(val,"true")==0);
            }

            editorNode->SetAttribute("resizable",toBoolStr(true));
            editorNode->SetAttribute("placement","absolute");
            if (App::userSettings->scriptEditorFont.compare("")!=0) // defaults are decided in the code editor plugin
                editorNode->SetAttribute("font",App::userSettings->scriptEditorFont.c_str());
            editorNode->SetAttribute("font-bold",toBoolStr(App::userSettings->scriptEditorBoldFont));
            editorNode->SetAttribute("toolbar",toBoolStr(false));
            editorNode->SetAttribute("statusbar",toBoolStr(false));
            editorNode->SetAttribute("can-restart",toBoolStr(false));
            editorNode->SetAttribute("max-lines",0);
            editorNode->SetAttribute("wrap-word",toBoolStr(false));
            editorNode->SetAttribute("closeable",toBoolStr(true));
            editorNode->SetAttribute("activate",toBoolStr(false));
            editorNode->SetAttribute("line-numbers",toBoolStr(true));
            editorNode->SetAttribute("tab-width",4);
            int fontSize=12;
            #ifdef MAC_SIM
                fontSize=16; // bigger fonts here
            #endif
            if (App::userSettings->scriptEditorFontSize!=-1)
                fontSize=App::userSettings->scriptEditorFontSize;
            editorNode->SetAttribute("font-size",fontSize);

            sim::tinyxml2::XMLElement* keywords1=rootElement->FirstChildElement("keywords1");
            if (keywords1!=nullptr)
            {
                val=keywords1->Attribute("color");
                if (val!=nullptr)
                    editorNode->SetAttribute("keyword1-col",val);
            }
            sim::tinyxml2::XMLElement* keywords2=rootElement->FirstChildElement("keywords2");
            if (keywords2!=nullptr)
            {
                val=keywords2->Attribute("color");
                if (val!=nullptr)
                    editorNode->SetAttribute("keyword2-col",val);
            }

            sim::tinyxml2::XMLElement* keywordsNode1=xmlNewDoc.NewElement("keywords1");
            editorNode->InsertEndChild(keywordsNode1);

            if (keywords1!=nullptr)
            {
                sim::tinyxml2::XMLElement* item=keywords1->FirstChildElement("item");
                while (item!=nullptr)
                {
                    sim::tinyxml2::XMLElement* itemNode=xmlNewDoc.NewElement("item");
                    keywordsNode1->InsertEndChild(itemNode);

                    val=item->Attribute("word");
                    if (val!=nullptr)
                        itemNode->SetAttribute("word",val);
                    val=item->Attribute("autocomplete");
                    if (val!=nullptr)
                        itemNode->SetAttribute("autocomplete",val);
                    val=item->Attribute("calltip");
                    if (val!=nullptr)
                        itemNode->SetAttribute("calltip",val);
                    item=item->NextSiblingElement("item");
                }
                if (csimKeywords)
                    getFuncKeywords(&xmlNewDoc,keywordsNode1,sim_scripttype_childscript,false);
            }

            sim::tinyxml2::XMLElement* keywordsNode2=xmlNewDoc.NewElement("keywords2");
            editorNode->InsertEndChild(keywordsNode2);

            if (keywords2!=nullptr)
            {
                sim::tinyxml2::XMLElement* item=keywords2->FirstChildElement("item");
                while (item!=nullptr)
                {
                    sim::tinyxml2::XMLElement* itemNode=xmlNewDoc.NewElement("item");
                    keywordsNode2->InsertEndChild(itemNode);

                    val=item->Attribute("word");
                    if (val!=nullptr)
                        itemNode->SetAttribute("word",val);
                    val=item->Attribute("autocomplete");
                    if (val!=nullptr)
                        itemNode->SetAttribute("autocomplete",val);
                    val=item->Attribute("calltip");
                    if (val!=nullptr)
                        itemNode->SetAttribute("calltip",val);
                    item=item->NextSiblingElement("item");
                }
                if (csimKeywords)
                    getVarKeywords(&xmlNewDoc,keywordsNode2,sim_scripttype_childscript,false);
            }
        }
    }
    sim::tinyxml2::XMLPrinter printer;
    xmlNewDoc.Print(&printer);
//  printf("%s\n",printer.CStr());
    return(std::string(printer.CStr()));
}

CCodeEditorContainer::CCodeEditorContainer()
{
}

CCodeEditorContainer::~CCodeEditorContainer()
{
}

void CCodeEditorContainer::announceScriptStateWillBeErased(int scriptHandle)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].callingScriptHandle==scriptHandle)
        {
            int handle=_allEditors[i].handle;
            int pas[4];
            CPluginContainer::codeEditor_close(handle,pas);
            // Here we need to find the correct index again, ordering might have changed (see above):
            for (size_t j=0;j<_allEditors.size();j++)
            {
                if (_allEditors[j].handle==handle)
                {
                    _allEditors.erase(_allEditors.begin()+j);
                    break;
                }
            }
            break;
        }
    }
}

int CCodeEditorContainer::openScriptWithExternalEditor(int scriptHandle)
{
    int retVal=-1;
    CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
    if (it!=nullptr)
    {
        if (!App::currentWorld->environment->getSceneLocked())
        {
            std::string fname(it->getFilenameForExternalScriptEditor());
            VVarious::executeExternalApplication(App::userSettings->externalScriptEditor.c_str(),fname.c_str(),App::folders->getExecutablePath().c_str(),VVARIOUS_SHOWNORMAL); // executable directory needed because otherwise the shellExecute command might switch directories!
            retVal=scriptHandle;
        }
        else
            App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
    }
    return(retVal);
}

int CCodeEditorContainer::open(const char* initText,const char* xml,int callingScriptHandle)
{
    CScriptObject* it=App::worldContainer->getScriptFromHandle(callingScriptHandle);
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        if (it!=nullptr)
        {
            if (!App::currentWorld->environment->getSceneLocked())
            {
                retVal=CPluginContainer::codeEditor_open(initText,xml);
                SCodeEditor inf;
                inf.handle=retVal;
                inf.scriptHandle=-1;
                inf.callingScriptHandle=callingScriptHandle;
                inf.sceneUniqueId=App::currentWorld->environment->getSceneUniqueID();
                inf.openAcrossScenes=( (it->getScriptType()==sim_scripttype_sandboxscript)||(it->getScriptType()==sim_scripttype_addonscript) );
                inf.closeAtSimulationEnd=it->isSimulationScript();
                inf.systemVisibility=true;
                inf.userVisibility=true;
                inf.closeAfterCallbackCalled=false;
                inf.restartScriptWhenClosing=false;
                inf.callbackFunction="";
                inf.uniqueId=_nextUniqueId++;
                _allEditors.push_back(inf);
            }
            else
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        }
    }
    else
        App::logMsg(sim_verbosity_errors,"code editor plugin was not found.");
    return(retVal);
}

int CCodeEditorContainer::openSimulationScript(int scriptHandle,int callingScriptHandle)
{
    int retVal=-1;
    CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
    if (it!=nullptr)
    {
        if (!App::currentWorld->environment->getSceneLocked())
        {
            if (App::userSettings->externalScriptEditor.size()==0)
            {
                int sceneId=App::currentWorld->environment->getSceneUniqueID();
                for (size_t i=0;i<_allEditors.size();i++)
                {
                    if ( (_allEditors[i].scriptHandle==scriptHandle)&&(_allEditors[i].sceneUniqueId==sceneId) )
                        return(_allEditors[i].handle);
                }
                if (CPluginContainer::isCodeEditorPluginAvailable())
                {
                    int posAndSize[4];
                    it->getPreviousEditionWindowPosAndSize(posAndSize);

                    sim::tinyxml2::XMLDocument xmlDoc;
                    sim::tinyxml2::XMLElement* editorNode=xmlDoc.NewElement("editor");
                    xmlDoc.InsertFirstChild(editorNode);
                    editorNode->SetAttribute("title",it->getDescriptiveName().c_str());
                    editorNode->SetAttribute("position",QString("%1 %2").arg(posAndSize[0]).arg(posAndSize[1]).toStdString().c_str());
                    editorNode->SetAttribute("size",QString("%1 %2").arg(posAndSize[2]).arg(posAndSize[3]).toStdString().c_str());
                    editorNode->SetAttribute("resizable",toBoolStr(true));
                    editorNode->SetAttribute("closeable",toBoolStr(true));
                    editorNode->SetAttribute("placement","absolute");
                    if (App::userSettings->scriptEditorFont.compare("")!=0) // defaults are decided in the code editor plugin
                        editorNode->SetAttribute("font",App::userSettings->scriptEditorFont.c_str());
                    editorNode->SetAttribute("font-bold",toBoolStr(App::userSettings->scriptEditorBoldFont));
                    editorNode->SetAttribute("toolbar",toBoolStr(true));
                    editorNode->SetAttribute("statusbar",toBoolStr(false));
                    editorNode->SetAttribute("wrap-word",toBoolStr(false));
                    editorNode->SetAttribute("can-restart",toBoolStr(!( (it->getScriptType()==sim_scripttype_mainscript)||it->getThreadedExecution_oldThreads() )));
                    editorNode->SetAttribute("max-lines",0);
                    editorNode->SetAttribute("activate",toBoolStr(true));
                    editorNode->SetAttribute("editable",toBoolStr(true));
                    editorNode->SetAttribute("searchable",toBoolStr(true));
                    editorNode->SetAttribute("line-numbers",toBoolStr(true));
                    editorNode->SetAttribute("tab-width",4);
                    editorNode->SetAttribute("script-up-to-date",it->getIsUpToDate());
                    if (it->getLanguage()==CScriptObject::lang_lua)
                    {
                        editorNode->SetAttribute("lang","lua");
                        editorNode->SetAttribute("search-paths",it->getSearchPath_lua().c_str());
                    }
                    if (it->getLanguage()==CScriptObject::lang_python)
                    {
                        editorNode->SetAttribute("lang","python");
                        editorNode->SetAttribute("search-paths",it->getSearchPath_python().c_str());
                    }
                    int fontSize=12;
                    #ifdef MAC_SIM
                        fontSize=16; // bigger fonts here
                    #endif
                    if (App::userSettings->scriptEditorFontSize!=-1)
                        fontSize=App::userSettings->scriptEditorFontSize;
                    editorNode->SetAttribute("font-size",fontSize);
                    editorNode->SetAttribute("text-col","0 0 0");
                    if (it->getScriptType()==sim_scripttype_mainscript)
                    {
                        editorNode->SetAttribute("background-col",getColorStr(App::userSettings->mainScriptColor_background).c_str());
                        editorNode->SetAttribute("selection-col",getColorStr(App::userSettings->mainScriptColor_selection).c_str());
                        editorNode->SetAttribute("comment-col",getColorStr(App::userSettings->mainScriptColor_comment).c_str());
                        editorNode->SetAttribute("number-col",getColorStr(App::userSettings->mainScriptColor_number).c_str());
                        editorNode->SetAttribute("string-col",getColorStr(App::userSettings->mainScriptColor_string).c_str());
                        editorNode->SetAttribute("character-col",getColorStr(App::userSettings->mainScriptColor_character).c_str());
                        editorNode->SetAttribute("operator-col",getColorStr(App::userSettings->mainScriptColor_operator).c_str());
                        editorNode->SetAttribute("identifier-col",getColorStr(App::userSettings->mainScriptColor_identifier).c_str());
                        editorNode->SetAttribute("preprocessor-col",getColorStr(App::userSettings->mainScriptColor_preprocessor).c_str());
                        editorNode->SetAttribute("keyword1-col",getColorStr(App::userSettings->mainScriptColor_word2).c_str());
                        editorNode->SetAttribute("keyword2-col",getColorStr(App::userSettings->mainScriptColor_word3).c_str());
                        editorNode->SetAttribute("keyword3-col",getColorStr(App::userSettings->mainScriptColor_word).c_str());
                        editorNode->SetAttribute("keyword4-col",getColorStr(App::userSettings->mainScriptColor_word4).c_str());
                    }
                    if (it->getScriptType()==sim_scripttype_childscript)
                    {
                        editorNode->SetAttribute("background-col",getColorStr(App::userSettings->childScriptColor_background).c_str());
                        editorNode->SetAttribute("selection-col",getColorStr(App::userSettings->childScriptColor_selection).c_str());
                        editorNode->SetAttribute("comment-col",getColorStr(App::userSettings->childScriptColor_comment).c_str());
                        editorNode->SetAttribute("number-col",getColorStr(App::userSettings->childScriptColor_number).c_str());
                        editorNode->SetAttribute("string-col",getColorStr(App::userSettings->childScriptColor_string).c_str());
                        editorNode->SetAttribute("character-col",getColorStr(App::userSettings->childScriptColor_character).c_str());
                        editorNode->SetAttribute("operator-col",getColorStr(App::userSettings->childScriptColor_operator).c_str());
                        editorNode->SetAttribute("identifier-col",getColorStr(App::userSettings->childScriptColor_identifier).c_str());
                        editorNode->SetAttribute("preprocessor-col",getColorStr(App::userSettings->childScriptColor_preprocessor).c_str());
                        editorNode->SetAttribute("keyword1-col",getColorStr(App::userSettings->childScriptColor_word2).c_str());
                        editorNode->SetAttribute("keyword2-col",getColorStr(App::userSettings->childScriptColor_word3).c_str());
                        editorNode->SetAttribute("keyword3-col",getColorStr(App::userSettings->childScriptColor_word).c_str());
                        editorNode->SetAttribute("keyword4-col",getColorStr(App::userSettings->childScriptColor_word4).c_str());
                    }

                    getKeywords(&xmlDoc,editorNode,it->getScriptType(),it->getThreadedExecution_oldThreads());

                    sim::tinyxml2::XMLPrinter printer;
                    xmlDoc.Print(&printer);
                    //printf("%s\n",printer.CStr());

                    retVal=CPluginContainer::codeEditor_open(it->getScriptText(),printer.CStr());
                    SCodeEditor inf;
                    inf.handle=retVal;
                    inf.scriptHandle=scriptHandle;
                    inf.callingScriptHandle=callingScriptHandle;
                    inf.sceneUniqueId=App::currentWorld->environment->getSceneUniqueID();
                    inf.openAcrossScenes=false;
                    inf.closeAtSimulationEnd=false;
                    inf.systemVisibility=true;
                    inf.userVisibility=true;
                    inf.closeAfterCallbackCalled=false;
                    inf.restartScriptWhenClosing=false;
                    inf.callbackFunction="";
                    inf.uniqueId=_nextUniqueId++;
                    _allEditors.push_back(inf);
                }
                else
                    App::logMsg(sim_verbosity_errors,"code editor plugin was not found.");
            }
            else
                retVal=openScriptWithExternalEditor(scriptHandle);
        }
        else
            App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
    }
    return(retVal);
}

int CCodeEditorContainer::openCustomizationScript(int scriptHandle,int callingScriptHandle)
{
    int retVal=-1;
    if (App::userSettings->externalScriptEditor.size()==0)
    {
        int sceneId=App::currentWorld->environment->getSceneUniqueID();
        for (size_t i=0;i<_allEditors.size();i++)
        {
            if ( (_allEditors[i].scriptHandle==scriptHandle)&&(_allEditors[i].sceneUniqueId==sceneId) )
                return(_allEditors[i].handle);
        }
        CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptHandle);
        if (CPluginContainer::isCodeEditorPluginAvailable())
        {
            if (it!=nullptr)
            {
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    int posAndSize[4];
                    it->getPreviousEditionWindowPosAndSize(posAndSize);

                    sim::tinyxml2::XMLDocument xmlDoc;
                    sim::tinyxml2::XMLElement* editorNode=xmlDoc.NewElement("editor");
                    xmlDoc.InsertFirstChild(editorNode);
                    editorNode->SetAttribute("title",it->getDescriptiveName().c_str());
                    editorNode->SetAttribute("position",QString("%1 %2").arg(posAndSize[0]).arg(posAndSize[1]).toStdString().c_str());
                    editorNode->SetAttribute("size",QString("%1 %2").arg(posAndSize[2]).arg(posAndSize[3]).toStdString().c_str());
                    editorNode->SetAttribute("resizable",toBoolStr(true));
                    editorNode->SetAttribute("closeable",toBoolStr(true));
                    editorNode->SetAttribute("placement","absolute");
                    if (App::userSettings->scriptEditorFont.compare("")!=0) // defaults are decided in the code editor plugin
                        editorNode->SetAttribute("font",App::userSettings->scriptEditorFont.c_str());
                    editorNode->SetAttribute("font-bold",toBoolStr(App::userSettings->scriptEditorBoldFont));
                    editorNode->SetAttribute("toolbar",toBoolStr(true));
                    editorNode->SetAttribute("statusbar",toBoolStr(false));
                    editorNode->SetAttribute("wrap-word",toBoolStr(false));
                    editorNode->SetAttribute("can-restart",toBoolStr(true));
                    editorNode->SetAttribute("max-lines",0);
                    editorNode->SetAttribute("activate",toBoolStr(true));
                    editorNode->SetAttribute("editable",toBoolStr(true));
                    editorNode->SetAttribute("searchable",toBoolStr(true));
                    editorNode->SetAttribute("line-numbers",toBoolStr(true));
                    editorNode->SetAttribute("tab-width",4);
                    editorNode->SetAttribute("script-up-to-date",it->getIsUpToDate());
                    if (it->getLanguage()==CScriptObject::lang_lua)
                    {
                        editorNode->SetAttribute("lang","lua");
                        editorNode->SetAttribute("search-paths",it->getSearchPath_lua().c_str());
                    }
                    if (it->getLanguage()==CScriptObject::lang_python)
                    {
                        editorNode->SetAttribute("lang","python");
                        editorNode->SetAttribute("search-paths",it->getSearchPath_python().c_str());
                    }
                    int fontSize=12;
                    #ifdef MAC_SIM
                        fontSize=16; // bigger fonts here
                    #endif
                    if (App::userSettings->scriptEditorFontSize!=-1)
                        fontSize=App::userSettings->scriptEditorFontSize;
                    editorNode->SetAttribute("font-size",fontSize);
                    editorNode->SetAttribute("text-col","0 0 0");
                    editorNode->SetAttribute("background-col",getColorStr(App::userSettings->customizationScriptColor_background).c_str());
                    editorNode->SetAttribute("selection-col",getColorStr(App::userSettings->customizationScriptColor_selection).c_str());
                    editorNode->SetAttribute("comment-col",getColorStr(App::userSettings->customizationScriptColor_comment).c_str());
                    editorNode->SetAttribute("number-col",getColorStr(App::userSettings->customizationScriptColor_number).c_str());
                    editorNode->SetAttribute("string-col",getColorStr(App::userSettings->customizationScriptColor_string).c_str());
                    editorNode->SetAttribute("character-col",getColorStr(App::userSettings->customizationScriptColor_character).c_str());
                    editorNode->SetAttribute("operator-col",getColorStr(App::userSettings->customizationScriptColor_operator).c_str());
                    editorNode->SetAttribute("identifier-col",getColorStr(App::userSettings->customizationScriptColor_identifier).c_str());
                    editorNode->SetAttribute("preprocessor-col",getColorStr(App::userSettings->customizationScriptColor_preprocessor).c_str());
                    editorNode->SetAttribute("keyword1-col",getColorStr(App::userSettings->customizationScriptColor_word2).c_str());
                    editorNode->SetAttribute("keyword2-col",getColorStr(App::userSettings->customizationScriptColor_word3).c_str());
                    editorNode->SetAttribute("keyword3-col",getColorStr(App::userSettings->customizationScriptColor_word).c_str());
                    editorNode->SetAttribute("keyword4-col",getColorStr(App::userSettings->customizationScriptColor_word4).c_str());

                    getKeywords(&xmlDoc,editorNode,it->getScriptType(),it->getThreadedExecution_oldThreads());

                    sim::tinyxml2::XMLPrinter printer;
                    xmlDoc.Print(&printer);
                    //printf("%s\n",printer.CStr());

                    retVal=CPluginContainer::codeEditor_open(it->getScriptText(),printer.CStr());
                    SCodeEditor inf;
                    inf.handle=retVal;
                    inf.scriptHandle=scriptHandle;
                    inf.callingScriptHandle=callingScriptHandle;
                    inf.sceneUniqueId=App::currentWorld->environment->getSceneUniqueID();
                    inf.openAcrossScenes=false;
                    inf.closeAtSimulationEnd=false;
                    inf.systemVisibility=true;
                    inf.userVisibility=true;
                    inf.closeAfterCallbackCalled=false;
                    inf.restartScriptWhenClosing=false;//true;
                    inf.callbackFunction="";
                    inf.uniqueId=_nextUniqueId++;
                    _allEditors.push_back(inf);
                }
                else
                    App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            }
        }
        else
            App::logMsg(sim_verbosity_errors,"code editor plugin was not found.");
    }
    else
        retVal=openScriptWithExternalEditor(scriptHandle);
    return(retVal);
}

int CCodeEditorContainer::openConsole(const char* title,int maxLines,int mode,const int position[2],const int size[2],const int textColor[3],const int backColor[3],int callingScriptHandle)
{
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        int _position[2]={100,100};
        int _size[2]={640,200};
        int _textColor[3]={0,0,0};
        int _backColor[3]={255,255,255};
        for (size_t i=0;i<2;i++)
        {
            if (position!=nullptr)
                _position[i]=position[i];
            if (size!=nullptr)
                _size[i]=size[i];
        }
        for (size_t i=0;i<3;i++)
        {
            if (textColor!=nullptr)
                _textColor[i]=textColor[i];
            if (backColor!=nullptr)
                _backColor[i]=backColor[i];
        }

        sim::tinyxml2::XMLDocument xmlDoc;
        sim::tinyxml2::XMLElement* editorNode=xmlDoc.NewElement("editor");
        xmlDoc.InsertFirstChild(editorNode);
        editorNode->SetAttribute("title",title);
        editorNode->SetAttribute("position",QString("%1 %2").arg(_position[0]).arg(_position[1]).toStdString().c_str());
        editorNode->SetAttribute("size",QString("%1 %2").arg(_size[0]).arg(_size[1]).toStdString().c_str());

        editorNode->SetAttribute("resizable",toBoolStr(true));
        editorNode->SetAttribute("closeable",toBoolStr((mode&4)!=0));
        editorNode->SetAttribute("placement","absolute");
        if (App::userSettings->scriptEditorFont.compare("")!=0) // defaults are decided in the code editor plugin
            editorNode->SetAttribute("font",App::userSettings->scriptEditorFont.c_str());
        editorNode->SetAttribute("font-bold",toBoolStr(App::userSettings->scriptEditorBoldFont));
        editorNode->SetAttribute("toolbar",toBoolStr(false));
        editorNode->SetAttribute("statusbar",toBoolStr(false));
        editorNode->SetAttribute("wrap-word",toBoolStr((mode&2)!=0));
        editorNode->SetAttribute("can-restart",toBoolStr(false));
        editorNode->SetAttribute("max-lines",maxLines);
        editorNode->SetAttribute("activate",toBoolStr(false));
        editorNode->SetAttribute("editable",toBoolStr(false));
        editorNode->SetAttribute("searchable",toBoolStr(false));
        editorNode->SetAttribute("line-numbers",toBoolStr(false));
        editorNode->SetAttribute("tab-width",4);
        editorNode->SetAttribute("lang","none");
        int fontSize=12;
        #ifdef MAC_SIM
            fontSize=16; // bigger fonts here
        #endif
        if (App::userSettings->scriptEditorFontSize!=-1)
            fontSize=App::userSettings->scriptEditorFontSize;
        editorNode->SetAttribute("font-size",fontSize);
        editorNode->SetAttribute("text-col",getColorStr(_textColor).c_str());
        editorNode->SetAttribute("background-col",getColorStr(_backColor).c_str());

        sim::tinyxml2::XMLPrinter printer;
        xmlDoc.Print(&printer);
        //printf("%s\n",printer.CStr());

        retVal=CPluginContainer::codeEditor_open("",printer.CStr());
        SCodeEditor inf;
        inf.handle=retVal;
        inf.scriptHandle=-1;
        inf.callingScriptHandle=callingScriptHandle;
        inf.sceneUniqueId=App::currentWorld->environment->getSceneUniqueID();
        inf.openAcrossScenes=((mode&16)>0);
        inf.closeAtSimulationEnd=((mode&1)>0);
        inf.systemVisibility=true;
        inf.userVisibility=true;
        inf.closeAfterCallbackCalled=false;
        inf.restartScriptWhenClosing=false;
        inf.restartScriptWhenClosing=false;
        inf.callbackFunction="";
        inf.uniqueId=_nextUniqueId++;
        _allEditors.push_back(inf);
    }
    else
        App::logMsg(sim_verbosity_errors,"code editor plugin was not found.");
    return(retVal);
}

std::string CCodeEditorContainer::openModalTextEditor(const char* initText,const char* xml,int windowSizeAndPos[4],bool oldXml/*=false*/) const
{
    std::string retVal;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        std::string newXml;
        if (xml!=nullptr)
        {
            if (oldXml)
                newXml=translateXml(xml,"");
            else
                newXml=xml;
        }
        int posAndSize[4];
        CPluginContainer::codeEditor_openModal(initText,newXml.c_str(),retVal,posAndSize);
        if (windowSizeAndPos!=nullptr)
        {
            windowSizeAndPos[0]=posAndSize[2];
            windowSizeAndPos[1]=posAndSize[3];
            windowSizeAndPos[2]=posAndSize[0];
            windowSizeAndPos[3]=posAndSize[1];
        }
    }
    else
        App::logMsg(sim_verbosity_errors,"code editor plugin was not found.");
    return(retVal);
}

int CCodeEditorContainer::openTextEditor(const char* initText,const char* xml,const char* callback,int callingScriptHandle,bool isSimulationScript)
{
    int retVal=-1;
    if (CPluginContainer::isCodeEditorPluginAvailable())
    {
        std::string newXml;
        newXml=translateXml(xml,callback);
        retVal=CPluginContainer::codeEditor_open(initText,newXml.c_str());
        SCodeEditor inf;
        inf.handle=retVal;
        inf.scriptHandle=-1;
        inf.callingScriptHandle=callingScriptHandle;
        inf.sceneUniqueId=App::currentWorld->environment->getSceneUniqueID();
        inf.openAcrossScenes=false;
        inf.closeAtSimulationEnd=isSimulationScript;
        inf.systemVisibility=true;
        inf.userVisibility=true;
        inf.closeAfterCallbackCalled=true;
        inf.restartScriptWhenClosing=false;
        inf.callbackFunction=callback;
        inf.uniqueId=_nextUniqueId++;
        _allEditors.push_back(inf);
    }
    else
        App::logMsg(sim_verbosity_errors,"code editor plugin was not found.");
    return(retVal);
}

bool CCodeEditorContainer::close(int handle,int posAndSize[4],std::string* txt,std::string* callback)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            if (callback!=nullptr)
                callback[0]=_allEditors[i].callbackFunction;
            std::string _txt;
            CScriptObject* it=App::worldContainer->getScriptFromHandle(_allEditors[i].scriptHandle);
            if (CPluginContainer::codeEditor_getText(handle,_txt,nullptr))
            {
                if (txt!=nullptr)
                    txt[0]=_txt;
                if (it!=nullptr)
                {
                    applyChanges(_allEditors[i].handle);
                    if (_allEditors[i].restartScriptWhenClosing)
                        resetScript(_allEditors[i].scriptHandle); // this can also trigger closing of another editor, see below
                }
            }
            int pas[4];
            CPluginContainer::codeEditor_close(handle,pas);
            if (it!=nullptr)
                it->setPreviousEditionWindowPosAndSize(pas);
            if (posAndSize!=nullptr)
            {
                for (size_t j=0;j<4;j++)
                    posAndSize[j]=pas[j];
            }

            // Here we need to find the correct index again, ordering might have changed (see above):
            for (size_t j=0;j<_allEditors.size();j++)
            {
                if (_allEditors[j].handle==handle)
                {
                    _allEditors.erase(_allEditors.begin()+j);
                    break;
                }
            }
            return(true);
        }
    }
    return(false);
}

void CCodeEditorContainer::applyChanges(int handle) const
{
    if (App::userSettings->externalScriptEditor.size()>0)
    {
        for (size_t i=0;i<App::currentWorld->embeddedScriptContainer->allScripts.size();i++)
            App::currentWorld->embeddedScriptContainer->allScripts[i]->fromFileToBuffer();
    }
    int sceneId=App::currentWorld->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].sceneUniqueId==sceneId)
        {
            if ( (_allEditors[i].handle==handle)||(handle==-1) )
            {
                std::string _txt;
                CScriptObject* it=App::worldContainer->getScriptFromHandle(_allEditors[i].scriptHandle);
                if (it!=nullptr)
                {
                    if (CPluginContainer::codeEditor_getText(_allEditors[i].handle,_txt,nullptr))
                        it->setScriptText(_txt.c_str());
                }
            }
        }
    }
}

bool CCodeEditorContainer::closeFromScriptHandle(int scriptHandle,int posAndSize[4],bool ignoreChange)
{
    if (App::userSettings->externalScriptEditor.size()==0)
    {
        for (size_t i=0;i<_allEditors.size();i++)
        {
            if (_allEditors[i].scriptHandle==scriptHandle)
            {
                std::string txt;
                CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
                if (!ignoreChange)
                    applyChanges(_allEditors[i].handle);
                int pas[4];
                CPluginContainer::codeEditor_close(_allEditors[i].handle,pas);
                if (it!=nullptr)
                    it->setPreviousEditionWindowPosAndSize(pas);
                if (posAndSize!=nullptr)
                {
                    for (size_t j=0;j<4;j++)
                        posAndSize[j]=pas[j];
                }
                _allEditors.erase(_allEditors.begin()+i);
                return(true);
            }
        }
    }
    else
    {
        applyChanges(-1);
        return(true);
    }
    return(false);
}

void CCodeEditorContainer::closeAll()
{ // before unloading the code editor plugin
    for (size_t i=0;i<_allEditors.size();i++)
        CPluginContainer::codeEditor_close(_allEditors[i].handle,nullptr);
    _allEditors.clear();
}

void CCodeEditorContainer::restartScript(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            std::string txt;
            CScriptObject* it=App::worldContainer->getScriptFromHandle(_allEditors[i].scriptHandle);
            if (CPluginContainer::codeEditor_getText(handle,txt,nullptr))
            {
                if ( (it!=nullptr)&&(!it->getThreadedExecution_oldThreads()) )
                {
                    applyChanges(_allEditors[i].handle);
                    resetScript(_allEditors[i].scriptHandle);
                }
            }
            break;
        }
    }
}

void CCodeEditorContainer::resetScript(int scriptHandle) const
{
    CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptHandle);
    if ( (it!=nullptr)&&it->resetScript() )
    {
        std::string msg(it->getDescriptiveName());
        msg+=" was reset.";
        App::logMsg(sim_verbosity_msgs,msg.c_str());
    }
}

bool CCodeEditorContainer::isHandleValid(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(true);
    }
    return(false);
}

std::string CCodeEditorContainer::getText(int handle,int posAndSize[4]) const
{
    std::string retVal;
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            CPluginContainer::codeEditor_getText(handle,retVal,posAndSize);
            break;
        }
    }
    return(retVal);
}

bool CCodeEditorContainer::setText(int handle,const char* txt) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            CPluginContainer::codeEditor_setText(handle,txt,0);
            return(true);
        }
    }
    return(false);
}

bool CCodeEditorContainer::appendText(int handle,const char* txt) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            CPluginContainer::codeEditor_setText(handle,txt,1);
            return(true);
        }
    }
    return(false);
}

bool CCodeEditorContainer::hasSomethingBeenModifiedInCurrentScene() const
{
    bool retVal=false;
    int sceneId=App::currentWorld->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].sceneUniqueId==sceneId)
        {
            CScriptObject* it=App::worldContainer->getScriptFromHandle(_allEditors[i].scriptHandle);
            std::string txt;
            if ( (it!=nullptr)&&CPluginContainer::codeEditor_getText(_allEditors[i].handle,txt,nullptr) )
            {
                std::string txt2(it->getScriptText());
                utils::removeSpacesAtBeginningAndEnd(txt);
                utils::removeSpacesAtBeginningAndEnd(txt2);
                if (txt.compare(txt2.c_str())!=0)
                {
                    retVal=true;
                    break;
                }
            }
        }
    }
    return(retVal);
}

int CCodeEditorContainer::getCallingScriptHandle(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(_allEditors[i].callingScriptHandle);
    }
    return(-1);
}

bool CCodeEditorContainer::getCloseAfterCallbackCalled(int handle) const
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(_allEditors[i].closeAfterCallbackCalled);
    }
    return(false);
}

void CCodeEditorContainer::simulationAboutToStart() const
{
    if (App::userSettings->externalScriptEditor.size()==0)
    {
        int sceneId=App::currentWorld->environment->getSceneUniqueID();
        for (size_t i=0;i<_allEditors.size();i++)
        {
            if ( (_allEditors[i].sceneUniqueId==sceneId)&&(_allEditors[i].scriptHandle>=0) )
            {
                CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(_allEditors[i].scriptHandle);
                if ( (it!=nullptr)&&((it->getScriptType()==sim_scripttype_mainscript)||(it->getScriptType()==sim_scripttype_childscript)) )
                    applyChanges(_allEditors[i].handle);
            }
        }
    }
    else
        applyChanges(-1);
}

void CCodeEditorContainer::simulationAboutToEnd()
{
    int sceneId=App::currentWorld->environment->getSceneUniqueID();
    for (int i=0;i<int(_allEditors.size());i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneId)&&_allEditors[i].closeAtSimulationEnd )
        {
            CPluginContainer::codeEditor_close(_allEditors[i].handle,nullptr);
            _allEditors.erase(_allEditors.begin()+i);
            i--;
        }
    }
}

void CCodeEditorContainer::saveOrCopyOperationAboutToHappen() const
{
    applyChanges(-1);
}

bool CCodeEditorContainer::areSceneEditorsOpen() const
{
    int sceneId=App::currentWorld->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneId)&&(_allEditors[i].scriptHandle>=0) )
            return(true);
    }
    return(false);
}

int CCodeEditorContainer::getHandleFromUniqueId(int uid)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].uniqueId==uid)
            return(_allEditors[i].handle);
    }
    return(-1);
}

int CCodeEditorContainer::getUniqueId(int handle)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
            return(_allEditors[i].uniqueId);
    }
    return(-1);
}

void CCodeEditorContainer::sceneClosed(int sceneUniqueId)
{
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if ( (_allEditors[i].sceneUniqueId==sceneUniqueId)&&(!_allEditors[i].openAcrossScenes) )
        {
            CPluginContainer::codeEditor_close(_allEditors[i].handle,nullptr);
            _allEditors.erase(_allEditors.begin()+i);
            i--;
        }
    }
}

void CCodeEditorContainer::showOrHideAll(bool showState)
{
    if (App::currentWorld->environment!=nullptr)
    {
        int sceneId=App::currentWorld->environment->getSceneUniqueID();
        for (size_t i=0;i<_allEditors.size();i++)
        {
            if ( (_allEditors[i].sceneUniqueId==sceneId)&&(!_allEditors[i].openAcrossScenes) )
            {
                _allEditors[i].systemVisibility=showState;
                if (showState&&_allEditors[i].userVisibility)
                    CPluginContainer::codeEditor_show(_allEditors[i].handle,1);
                else
                    CPluginContainer::codeEditor_show(_allEditors[i].handle,0);
            }
        }
    }
}

int CCodeEditorContainer::showOrHide(int handle,bool showState)
{
    int retVal=-1;
    int sceneId=App::currentWorld->environment->getSceneUniqueID();
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            retVal=0;
            if (_allEditors[i].userVisibility!=showState)
                retVal=1;
            _allEditors[i].userVisibility=showState;
            if (showState&&_allEditors[i].systemVisibility)
                CPluginContainer::codeEditor_show(_allEditors[i].handle,1);
            else
                CPluginContainer::codeEditor_show(_allEditors[i].handle,0);
            break;
        }
    }
    return(retVal);
}

int CCodeEditorContainer::getShowState(int handle) const
{ // ret -1: no such handle, 0=user hidden, 1=user visible
    int retVal=-1;
    for (size_t i=0;i<_allEditors.size();i++)
    {
        if (_allEditors[i].handle==handle)
        {
            retVal=0;
            if (_allEditors[i].userVisibility)
                retVal=1;
            break;
        }
    }
    return(retVal);
}
