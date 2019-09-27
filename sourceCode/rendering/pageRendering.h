
#include "app.h"
#include "rendering.h"

#ifdef SIM_WITH_GUI
void _drawSeparationLines(int _pageType,const int* _pageSize);
#endif

void displayContainerPage(CSPage* page,const int* position,const int* size);
void displayContainerPageOverlay(const int* position,const int* size,int activePageIndex,int focusObject);
void displayContainerPageWatermark(const int* position,const int* size,int tagId);
void displayPage(CSPage* page,int auxViewResizingAction,int viewIndexOfResizingAction);

