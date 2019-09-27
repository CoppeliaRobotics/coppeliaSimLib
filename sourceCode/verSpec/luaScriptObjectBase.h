
#pragma once

class CLuaScriptObject;

class CLuaScriptObjectBase
{
public:
    CLuaScriptObjectBase(){}
    virtual ~CLuaScriptObjectBase(){}

    void handleVerSpec_adjustScriptText1(CLuaScriptObject* scriptObject,bool doIt,bool doIt2){}
    void handleVerSpec_adjustScriptText2(CLuaScriptObject* scriptObject,bool doIt){}
    void handleVerSpec_adjustScriptText3(CLuaScriptObject* scriptObject,bool doIt){}
    void handleVerSpec_adjustScriptText4(CLuaScriptObject* scriptObject,bool doIt){}
    void handleVerSpec_adjustScriptText5(CLuaScriptObject* scriptObject,bool doIt){}
    void handleVerSpec_adjustScriptText6(CLuaScriptObject* scriptObject,bool doIt){}
    void handleVerSpec_adjustScriptText7(CLuaScriptObject* scriptObject,bool doIt){}
    void handleVerSpec_adjustScriptText8(CLuaScriptObject* scriptObject,int adjust){}
    void handleVerSpec_adjustScriptText9(CLuaScriptObject* scriptObject){}
    void handleVerSpec_adjustScriptText10(CLuaScriptObject* scriptObject,bool doIt){}
};
