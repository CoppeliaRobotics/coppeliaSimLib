#pragma once

#include <string>
#include "syncObject.h"

enum {
    sim_syncobj_colorobject_setcolors=0,
    sim_syncobj_colorobject_settranslucid,
    sim_syncobj_colorobject_settransparency,
    sim_syncobj_colorobject_setshininess,
    sim_syncobj_colorobject_setcolorname,
    sim_syncobj_colorobject_setextensionstring,
};

class _CColorObject_ : public CSyncObject
{
public:
    _CColorObject_();
    virtual ~_CColorObject_();

    // Overridden from _CSyncObject_:
    virtual void synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg);

    void getColors(float col[15]) const;
    const float* getColorsPtr() const;
    float* getColorsPtr();
    bool getTranslucid() const;
    float getOpacity() const;
    int getShininess() const;
    std::string getColorName() const;
    std::string getExtensionString() const;

    virtual void setColors(const float col[15]);
    virtual void setTranslucid(bool e);
    virtual void setOpacity(float e);
    virtual void setShininess(int e);
    virtual void setColorName(const char* nm);
    virtual void setExtensionString(const char* nm);

protected:
    virtual void _setColors_send(const float col[15]) const;
    virtual void _setTranslucid_send(bool e) const;
    virtual void _setOpacity_send(float e) const;
    virtual void _setShininess_send(int e) const;
    virtual void _setColorName_send(const char* nm) const;
    virtual void _setExtensionString_send(const char* nm) const;

    bool _isSame(const _CColorObject_* it) const;

    float _colors[15];
    int _shininess;
    float _opacity;
    bool _translucid;
    std::string _colorName;
    std::string _extensionString;
};
