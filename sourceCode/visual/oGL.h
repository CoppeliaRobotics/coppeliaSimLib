// This file includes many many very ugly things and requires some serious refactoring!!!!

#pragma once

#include "textureProperty.h"
#include "softButton.h"
#ifdef USING_QOPENGLWIDGET
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif

struct SOglFont
{
    std::string fontName;
    int fontHeight;
    unsigned int fontBase;
    int fontWidths[256];
};

//FULLY STATIC CLASS
class ogl  
{
public:
    static std::vector<double> buffer;
    static std::vector<int> buffer_i;

    static inline void addBuffer2DPoints(double x,double y)
    {
        buffer.push_back(x);
        buffer.push_back(y);
    };
    static inline void addBuffer2DPoints_i(int x,int y)
    {
        buffer_i.push_back(x);
        buffer_i.push_back(y);
    };
    static inline void addBuffer3DPoints(double x,double y,double z)
    {
        buffer.push_back(x);
        buffer.push_back(y);
        buffer.push_back(z);
    };
    static inline void addBuffer3DPoints(const double xyz[3])
    {
        buffer.push_back(xyz[0]);
        buffer.push_back(xyz[1]);
        buffer.push_back(xyz[2]);
    };

    static void setTextColor(float r,float g,float b);
    static void setTextColor(float rgb[3]);
    static void setMaterialColor(const float ambientCol[3],const float specularCol[3],const float emissiveCol[3]);
    static void setMaterialColor(float amb_r,float amb_g,float amb_b,float spec_r,float spec_g,float spec_b,float em_r,float em_g,float em_b);
    static void setMaterialColor(int colorMode,const float col[3]);
    static void setMaterialColor(int colorMode,float r,float g,float b);
    static void setBlendedMaterialColor(int colorMode,float r,float g,float b,float weight,float r2=0.5,float g2=0.5,float b2=0.5);
    static void setBlendedMaterialColor(int colorMode,float rgb[3],float weight,float rgb2[3]);

    static void setBlending(bool on,int param1=0,int param2=0);
    static void setAlpha(double a);
    static void setShininess(int shin);

    static void disableLighting_useWithCare();
    static void enableLighting_useWithCare();

    static void drawRandom3dLines(const double* pts,int ptsCnt,bool connected,const double normalVectorForDiffuseComp[3]);
    static inline void drawSingle3dPoint(const double* pt,const double normalVectorForDiffuseComp[3])
    {
        glBegin(GL_POINTS);
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3dv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0,0.0,1.0);
        glVertex3dv(pt);
        glEnd();
    };
    static inline void drawSingle3dLine(const double* pt1,const double* pt2,const double normalVectorForDiffuseComp[3])
    {
        glBegin(GL_LINES);
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3dv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0,0.0,1.0);
        glVertex3dv(pt1);
        glVertex3dv(pt2);
        glEnd();
    };
    static inline void drawSingle3dLine(double p1x,double p1y,double p1z,double p2x,double p2y,double p2z,const double normalVectorForDiffuseComp[3])
    {
        glBegin(GL_LINES);
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3dv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0,0.0,1.0);
        glVertex3d(p1x,p1y,p1z);
        glVertex3d(p2x,p2y,p2z);
        glEnd();
    };

    static inline void drawSingle2dLine_i(double p1x,double p1y,double p2x,double p2y)
    {
        glBegin(GL_LINES);
        glVertex3i(p1x,p1y,0);
        glVertex3i(p2x,p2y,0);
        glEnd();
    };

    static void drawRandom3dPoints(const double* pts,int ptsCnt,const double normalVectorForDiffuseComp[3]);
    static void drawRandom3dPointsEx(const double* pts,int ptsCnt,const double* normals,const float* cols,const double* sizes,bool colsAreEmission,const double normalVectorForDiffuseComp[3],int colComp=4);
    static void drawBitmapTextTo3dPosition(const double pos[3],const char* txt,const double normalVectorForDiffuseComp[3]);
    static void drawBitmapTextTo3dPosition(double x,double y,double z,const char* txt,const double normalVectorForDiffuseComp[3]);

    static void drawRandom2dLines(const double* pts,int ptsCnt,bool connected,double zCoord);
    static void drawRandom2dLines_i(const int* pts,int ptsCnt,bool connected,int zCoord);
    static void drawRandom2dPoints(const double* pts,int ptsCnt,double zCoord);
    static void drawBitmapTextTo2dPosition(double posX,double posY,const char* txt);

    static void drawPlane(double sizeX,double sizeY);
    static void drawBox(double x,double y,double z,bool solid,const double normalVectorForDiffuseComp[3]);
    static void drawCylinder(double d,double l,int div,int openEnds,bool smooth);
    static void drawCircle(double d,int div);
    static void drawDisk(double d,int div);
    static void drawCone(double d,double l,int div,bool openEnds,bool smooth);
    static void drawSphere(double r,int sides,int faces,bool smooth);

    static void drawReference(double size,bool line,bool setColors,bool emissiveColor,const double normalVectorForDiffuseComp[3]);

    static void perspectiveSpecial(double fovy,double aspect,double zNear,double zFar);
    static void _frustumSpecial(double left,double right,double bottom,double top,double nearval,double farval);
    static void pickMatrixSpecial(int x,int y,int width,int height,int viewport[4]);






    static void setRichTextColor(float col[3]);
    static void getRichTextColor(float col[3]);
    static void setRichTextDefaultColor(float col[3]);
    static void getRichTextDefaultColor(float col[3]);




    static void drawQuad(int posX,int posY,int sizeX,int sizeY,float col[3]);
    static void drawBorder(int posX,int posY,int sizeX,int sizeY,float col[3],int type,int borderSize=3);

    static void drawOutlineText(const char* txt,const C7Vector& trOrig,double textHeight,const C4X4Matrix* cameraAbsConfig,bool overlay,bool left,bool right,bool culling,const float* textColor,const float* backColor=nullptr,bool sizeInPixels=false,int windowHeight=0,double verticalViewSizeOrAngle=0.0,bool perspective=true,bool fullyFacingCamera=true);
    static void drawText(int posX,int posY,int posZ,std::string txt,bool verticallyCentered=false);
    static void drawBitmapTextBackgroundIntoScene(double posX,double posY,double posZ,std::string txt);
    static void drawBitmapTextIntoScene(double posX,double posY,double posZ,const char* txt);
    static void drawTexti(int posX,int posY,int posZ,std::string txt);
//  static void drawCenteredText(int posX,int posY,int posZ,std::string txt,bool verticallyCentered=false);
    static int getTextLengthInPixels(const char* txt);
    static std::string getTextThatFitIntoPixelWidth(std::vector<std::string>& separateWords,std::vector<int>& widths,int spaceWidth,int pixelWidth,int& textCharCount_pixelWidth,bool allowWordCut);
    static int getSeparateWords(const char* txt,std::vector<std::string>& separateWords);
    static int getSeparateWordWidths(const std::vector<std::string>& separateWords,std::vector<int>& widths);
    static int getFontHeight();
    static int getFontCharWidth(unsigned char c);
    static int getInterline();

    static void loadBitmapFont(const unsigned char* fontData,int fontHeight,int theFontIndex);
    static void loadOutlineFont(const int* fontData,const float* fontVertices);

    static void freeBitmapFonts();
    static void freeOutlineFont();
    static void selectBitmapFont(int index);

    static void drawButton(VPoint p,VPoint s,float txtColor[3],float _bckgrndColor1[3],
        float _bckgrndColor2[3],std::string text,int buttonTypeAndAttributes,
        bool editing,int editionPos,double sliderPos,bool vertical,int timeInMs,float secondTextColor[3],CTextureProperty* backgroundTexture,VPoint* backgroundPos,VPoint* backgroundSize,CTextureProperty* foregroundTexture);

    static int getRichTextInfo(std::string& text,std::vector<int>& iconsAndPos);
    static void drawRichText(int posX,int posY,int size,std::string& text,std::vector<int>& iconsAndPos,int alignement,float backgroundColor[3],float secondTextColor[3]);
    static void drawRichTextIcon(int centerPosX,int centerPosY,int size,int icon);

    static void drawButtonEdit(VPoint p,VPoint s,bool selected,bool mainSel);
private:
    static void drawText(std::string t);
    static void setTextPosition(double x,double y,double z);

    static bool colorsLocked;
    static int fontIndex;
    static char opacBitmap[10000];

    static unsigned int outlineFontBase;
    static double outlineFontSizes[256];
    static double outlineFontHeight;
    static double outlineFontCenter;


    static float currentRichTextColor[3];
    static float defaultRichTextColor[3];


    static SOglFont oglFonts[];
public:
    static float colorBlack[3];
    static float colorDarkGrey[3];
    static float colorGrey[3];
    static float colorLightGrey[3];
    static float colorWhite[3];
    static float colorRed[3];
    static float colorGreen[3];
    static float colorBlue[3];
    static float colorYellow[3];
    static float colorCyan[3];
    static float colorPurple[3];

    static float _lastAmbientDiffuseAlpha[4];


    static float HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR[3];
    static float HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT[3];
    static float HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK[3];
    static float HIERARCHY_DROP_LOCATION_COLOR[3];
    static float HIERARCHY_WORLD_CLICK_COLOR[3];

    static float HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[3];
    static float HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK[3];
    static float HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE[3];
    static float HIERARCHY_AND_BROWSER_TEXT_COLOR_INVISIBLE[3];
    static float HIERARCHY_UNACTIVE_WORLD_COLOR[3];

    static float HIERARCHY_AND_BROWSER_LINE_COLOR[3];
    static float HIERARCHY_AND_BROWSER_SCROLLBAR_BACK_COLOR[3];
    static float HIERARCHY_AND_BROWSER_SCROLLBAR_COLOR[3];

    static float TITLE_BAR_COLOR[3];
    static float TITLE_BAR_BUTTON_COLOR[3];
    static float SEPARATION_LINE_COLOR[3];

    static float MANIPULATION_MODE_OVERLAY_COLOR[4];
    static float MANIPULATION_MODE_OVERLAY_GRID_COLOR[4];

    static float HIERARCHY_NO_SELECTION_RED_BRIGHT[3];
    static float HIERARCHY_NO_SELECTION_RED_DARK[3];

    static float HIERARCHY_NO_SELECTION_GREEN_BRIGHT[3];
    static float HIERARCHY_NO_SELECTION_GREEN_DARK[3];

    static float HIERARCHY_NO_SELECTION_BLUE_BRIGHT[3];
    static float HIERARCHY_NO_SELECTION_BLUE_DARK[3];

    static unsigned int genTexture();
    static void delTexture(unsigned int t);
    static std::vector<unsigned int> _allTextureNames;
    static std::vector<bool> _allTextureNamesAv;
};
