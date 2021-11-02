#include "_colorObject_.h"
#include "simConst.h"

_CColorObject_::_CColorObject_()
{
}

_CColorObject_::~_CColorObject_()
{
}

bool _CColorObject_::_isSame(const _CColorObject_* it) const
{
    bool retVal=true;
    for (size_t i=0;i<15;i++)
    {
        if (it->_colors[i]!=_colors[i])
        {
            retVal=false;
            break;
        }
    }
    retVal=retVal&&(_shininess==it->_shininess);
    retVal=retVal&&(_opacity==it->_opacity);
    retVal=retVal&&(_translucid==it->_translucid);
    retVal=retVal&&(_colorName==it->_colorName);
    retVal=retVal&&(_extensionString==it->_extensionString);
    return(retVal);
}

bool _CColorObject_::getTranslucid() const
{
    return(_translucid);
}

float _CColorObject_::getOpacity() const
{
    return(_opacity);
}

int _CColorObject_::getShininess() const
{
    return(_shininess);
}

std::string _CColorObject_::getColorName() const
{
    return(_colorName);
}

std::string _CColorObject_::getExtensionString() const
{
    return(_extensionString);
}

void _CColorObject_::getColors(float col[15]) const
{
    for (size_t i=0;i<15;i++)
        col[i]=_colors[i];
}

const float* _CColorObject_::getColorsPtr() const
{
    return(_colors);
}

float* _CColorObject_::getColorsPtr()
{
    return(_colors);
}

void _CColorObject_::setColors(const float col[15])
{
    bool diff=false;
    for (size_t i=0;i<15;i++)
    {
        if (_colors[i]!=col[i])
        {
            diff=true;
            break;
        }
    }
    if (diff)
    {
        if (getObjectCanChange())
        {
            for (size_t i=0;i<15;i++)
                _colors[i]=col[i];
        }
        if (getObjectCanSync())
            _setColors_send(col);
    }
}

void _CColorObject_::setTranslucid(bool e)
{
    bool diff=(_translucid!=e);
    if (diff)
    {
        if (getObjectCanChange())
            _translucid=e;
        if (getObjectCanSync())
            _setTranslucid_send(e);
    }
}

void _CColorObject_::setOpacity(float e)
{
    bool diff=(_opacity!=e);
    if (diff)
    {
        if (getObjectCanChange())
            _opacity=e;
        if (getObjectCanSync())
            _setOpacity_send(e);
    }
}

void _CColorObject_::setShininess(int e)
{
    bool diff=(_shininess!=e);
    if (diff)
    {
        if (getObjectCanChange())
            _shininess=e;
        if (getObjectCanSync())
            _setShininess_send(e);
    }
}

void _CColorObject_::setColorName(const char* nm)
{
    bool diff=(_colorName!=nm);
    if (diff)
    {
        if (getObjectCanChange())
            _colorName=nm;
        if (getObjectCanSync())
            _setColorName_send(nm);
    }
}

void _CColorObject_::setExtensionString(const char* nm)
{
    bool diff=(_extensionString!=nm);
    if (diff)
    {
        if (getObjectCanChange())
            _extensionString=nm;
        if (getObjectCanSync())
            _setExtensionString_send(nm);
    }
}

void _CColorObject_::_setTranslucid_send(bool e) const
{
    sendBool(e,sim_syncobj_colorobject_settranslucid);
}

void _CColorObject_::_setOpacity_send(float e) const
{
    sendFloat(e,sim_syncobj_colorobject_settransparency);
}

void _CColorObject_::_setShininess_send(int e) const
{
    sendInt32(e,sim_syncobj_colorobject_setshininess);
}

void _CColorObject_::_setColorName_send(const char* nm) const
{
    sendString(nm,sim_syncobj_colorobject_setcolorname);
}

void _CColorObject_::_setExtensionString_send(const char* nm) const
{
    sendString(nm,sim_syncobj_colorobject_setextensionstring);
}

void _CColorObject_::_setColors_send(const float col[15]) const
{
    sendFloatArray(col,15,sim_syncobj_colorobject_setcolors);
}

void _CColorObject_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    if (msg.msg==sim_syncobj_colorobject_settranslucid)
    {
        setTranslucid(((bool*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_colorobject_settransparency)
    {
        setOpacity(((float*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_colorobject_setshininess)
    {
        setShininess(((int*)msg.data)[0]);
        return;
    }
    if (msg.msg==sim_syncobj_colorobject_setcolorname)
    {
        setColorName(((char*)msg.data));
        return;
    }
    if (msg.msg==sim_syncobj_colorobject_setextensionstring)
    {
        setExtensionString(((char*)msg.data));
        return;
    }
    if (msg.msg==sim_syncobj_colorobject_setcolors)
    {
        setColors(((float*)msg.data));
        return;
    }
}
