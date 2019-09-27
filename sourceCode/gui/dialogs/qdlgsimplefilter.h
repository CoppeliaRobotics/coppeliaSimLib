
#pragma once

#include "vDialog.h"
#include "simpleFilter.h"

class CQDlgSimpleFilter : public VDialog
{

public:
    CQDlgSimpleFilter(QWidget* pParent);
    virtual ~CQDlgSimpleFilter();

    virtual void refresh();

    void cancelEvent();
    void okEvent();
    void initializationEvent();

    void initializeDialogValues(CSimpleFilter* f);
    int appendDialogValues(std::vector<unsigned char>& byteParams,std::vector<int>& intParams,std::vector<float>& floatParams);

protected:
    int _filterVersion;
    std::vector<int> _intParameters;
    std::vector<float> _floatParameters;
    std::vector<unsigned char> _byteParameters;
};
