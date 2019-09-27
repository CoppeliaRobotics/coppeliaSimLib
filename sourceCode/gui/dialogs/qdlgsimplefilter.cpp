
#include "vrepMainHeader.h"
#include "qdlgsimplefilter.h"
#include "app.h"

CQDlgSimpleFilter::CQDlgSimpleFilter(QWidget* pParent) : VDialog(pParent,QT_MODAL_DLG_STYLE)
{
}

CQDlgSimpleFilter::~CQDlgSimpleFilter()
{
}

void CQDlgSimpleFilter::refresh()
{
}

void CQDlgSimpleFilter::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter::okEvent()
{  // nothing here
}

void CQDlgSimpleFilter::initializationEvent()
{
    defaultDialogInitializationRoutine();
    refresh();
}

void CQDlgSimpleFilter::initializeDialogValues(CSimpleFilter* f)
{
    f->getParameters(_byteParameters,_intParameters,_floatParameters,_filterVersion);
    refresh();
}

int CQDlgSimpleFilter::appendDialogValues(std::vector<unsigned char>& byteParams,std::vector<int>& intParams,std::vector<float>& floatParams)
{
    byteParams.insert(byteParams.end(),_byteParameters.begin(),_byteParameters.end());
    intParams.insert(intParams.end(),_intParameters.begin(),_intParameters.end());
    floatParams.insert(floatParams.end(),_floatParameters.begin(),_floatParameters.end());
    return(_filterVersion);
}
