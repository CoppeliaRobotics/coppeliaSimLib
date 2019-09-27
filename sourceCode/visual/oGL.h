
// This file includes many many very ugly things and requires some serious refactoring!!!!

#pragma once

#include "vrepMainHeader.h"
#include "textureProperty.h"
#include "VPoint.h"
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
    static std::vector<float> buffer;
    static std::vector<int> buffer_i;

    static inline void addBuffer2DPoints(float x,float y)
    {
        buffer.push_back(x);
        buffer.push_back(y);
    };
    static inline void addBuffer2DPoints_i(int x,int y)
    {
        buffer_i.push_back(x);
        buffer_i.push_back(y);
    };
    static inline void addBuffer3DPoints(float x,float y,float z)
    {
        buffer.push_back(x);
        buffer.push_back(y);
        buffer.push_back(z);
    };
    static inline void addBuffer3DPoints(const float xyz[3])
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
    static void setBlendedMaterialColor(int colorMode,float r,float g,float b,float weight,float r2=0.5f,float g2=0.5f,float b2=0.5f);
    static void setBlendedMaterialColor(int colorMode,float rgb[3],float weight,float rgb2[3]);

    static void setBlending(bool on,int param1=0,int param2=0);
    static void setAlpha(float a);
    static void setShininess(int shin);

    static void disableLighting_useWithCare();
    static void enableLighting_useWithCare();

    static void drawRandom3dLines(const float* pts,int ptsCnt,bool connected,const float normalVectorForDiffuseComp[3]);
    static inline void drawSingle3dPoint(const float* pt,const float normalVectorForDiffuseComp[3])
    {
        glBegin(GL_POINTS);
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3fv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0f,0.0f,1.0f);
        glVertex3fv(pt);
        glEnd();
    };
    static inline void drawSingle3dLine(const float* pt1,const float* pt2,const float normalVectorForDiffuseComp[3])
    {
        glBegin(GL_LINES);
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3fv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0f,0.0f,1.0f);
        glVertex3fv(pt1);
        glVertex3fv(pt2);
        glEnd();
    };
    static inline void drawSingle3dLine(float p1x,float p1y,float p1z,float p2x,float p2y,float p2z,const float normalVectorForDiffuseComp[3])
    {
        glBegin(GL_LINES);
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3fv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0f,0.0f,1.0f);
        glVertex3f(p1x,p1y,p1z);
        glVertex3f(p2x,p2y,p2z);
        glEnd();
    };

    static inline void drawSingle2dLine_i(float p1x,float p1y,float p2x,float p2y)
    {
        glBegin(GL_LINES);
        glVertex3i(p1x,p1y,0);
        glVertex3i(p2x,p2y,0);
        glEnd();
    };

    static void drawRandom3dPoints(const float* pts,int ptsCnt,const float normalVectorForDiffuseComp[3]);
    static void drawRandom3dPointsEx(const float* pts,int ptsCnt,const float* normals,const float* cols,const float* sizes,bool colsAreEmission,const float normalVectorForDiffuseComp[3]);
    static void drawBitmapTextTo3dPosition(const float pos[3],const std::string& txt,const float normalVectorForDiffuseComp[3]);
    static void drawBitmapTextTo3dPosition(float x,float y,float z,const std::string& txt,const float normalVectorForDiffuseComp[3]);

    static void drawRandom2dLines(const float* pts,int ptsCnt,bool connected,float zCoord);
    static void drawRandom2dLines_i(const int* pts,int ptsCnt,bool connected,int zCoord);
    static void drawRandom2dPoints(const float* pts,int ptsCnt,float zCoord);
    static void drawBitmapTextTo2dPosition(float posX,float posY,const std::string& txt);

    static void drawPlane(float sizeX,float sizeY);
    static void drawBox(float x,float y,float z,bool solid,const float normalVectorForDiffuseComp[3]);
    static void drawCylinder(float d,float l,int div,int openEnds,bool smooth);
    static void drawCircle(float d,int div);
    static void drawDisk(float d,int div);
    static void drawCone(float d,float l,int div,bool openEnds,bool smooth);
    static void drawSphere(float r,int sides,int faces,bool smooth);

    static void drawReference(float size,bool line,bool setColors,bool emissiveColor,const float normalVectorForDiffuseComp[3]);

    static void perspectiveSpecial(float fovy,float aspect,float zNear,float zFar);
    static void _frustumSpecial(float left,float right,float bottom,float top,float nearval,float farval);
    static void pickMatrixSpecial(int x,int y,int width,int height,int viewport[4]);






    static void setRichTextColor(float col[3]);
    static void getRichTextColor(float col[3]);
    static void setRichTextDefaultColor(float col[3]);
    static void getRichTextDefaultColor(float col[3]);




    static void drawQuad(int posX,int posY,int sizeX,int sizeY,float col[3]);
    static void drawBorder(int posX,int posY,int sizeX,int sizeY,float col[3],int type,int borderSize=3);

    static void drawOutlineText(const char* txt,const C7Vector& trOrig,float textHeight,const C4X4Matrix* cameraAbsConfig,bool overlay,bool left,bool right,bool culling,const float* textColor,const float* backColor=nullptr,bool sizeInPixels=false,int windowHeight=0,float verticalViewSizeOrAngle=0.0f,bool perspective=true,bool fullyFacingCamera=true);
    static void drawText(int posX,int posY,int posZ,std::string txt,bool verticallyCentered=false);
    static void drawBitmapTextBackgroundIntoScene(float posX,float posY,float posZ,std::string txt);
    static void drawBitmapTextIntoScene(float posX,float posY,float posZ,const std::string& txt);
    static void drawTexti(int posX,int posY,int posZ,std::string txt);
//  static void drawCenteredText(int posX,int posY,int posZ,std::string txt,bool verticallyCentered=false);
    static int getTextLengthInPixels(const std::string& txt);
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
        bool editing,int editionPos,float sliderPos,bool vertical,int timeInMs,float secondTextColor[3],CTextureProperty* backgroundTexture,VPoint* backgroundPos,VPoint* backgroundSize,CTextureProperty* foregroundTexture);

    static int getMultilineTextInfo(const std::string& text,std::vector<std::string>& lines,int* textMaxWidth=nullptr,int* textHeight=nullptr,int* charHeight=nullptr);
    static int getRichTextInfo(std::string& text,std::vector<int>& iconsAndPos);
    static void drawRichText(int posX,int posY,int size,std::string& text,std::vector<int>& iconsAndPos,int alignement,float backgroundColor[3],float secondTextColor[3]);
    static void drawRichTextIcon(int centerPosX,int centerPosY,int size,int icon);

    static void drawButtonEdit(VPoint p,VPoint s,bool selected,bool mainSel);
private:
    static void drawText(std::string t);
    static void setTextPosition(float x,float y,float z);

    static bool colorsLocked;
    static int fontIndex;
    static char opacBitmap[10000];

    static unsigned int outlineFontBase;
    static float outlineFontSizes[256];
    static float outlineFontHeight;
    static float outlineFontCenter;


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
    static float HIERARCHY_COLLECTION_SELECTION_COLOR_BRIGHT[3];
    static float HIERARCHY_COLLECTION_SELECTION_COLOR_DARK[3];
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
};
