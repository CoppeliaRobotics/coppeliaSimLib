/*
Source code based on the V-REP library source code from Coppelia
Robotics AG on September 2019

Copyright (C) 2006-2023 Coppelia Robotics AG
Copyright (C) 2019 Robot Nordic ApS

All rights reserved.

GNU GPL license:
================
The code in this file is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

See the GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*/

#include <thumbnailRendering.h>

void displayThumbnail(CThumbnail* thumbnail, int posX, int posY, const char* name, int sc)
{
    int borderWidth = sc * 1;
    int textSpaceHeight = MODEL_BROWSER_TEXT_SPACE_HEIGHT * sc;
    ogl::disableLighting_useWithCare(); // only temporarily
    glColor3d(ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[0],
              ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[1],
              ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[2]);
    glBegin(GL_QUADS);
    glVertex3i(posX + borderWidth, posY - borderWidth, 0);
    glVertex3i(posX + borderWidth + sc * 128, posY - borderWidth, 0);
    glVertex3i(posX + borderWidth + sc * 128, posY - borderWidth - sc * 128 - textSpaceHeight, 0);
    glVertex3i(posX + borderWidth, posY - borderWidth - sc * 128 - textSpaceHeight, 0);
    glEnd();
    ogl::enableLighting_useWithCare();
    const char* _thumbnailRGBAImage = thumbnail->getPointerToUncompressedImage();
    if (_thumbnailRGBAImage == nullptr)
    {
        ogl::setTextColor(0.5, 0.0, 0.0);
        int textWidth = ogl::getTextLengthInPixels("Thumbnail");
        ogl::drawText(posX + borderWidth + sc * 64 - textWidth / 2,
                      posY - borderWidth - sc * 64 + ogl::getFontHeight() / 2, 0, "Thumbnail", true);
        textWidth = ogl::getTextLengthInPixels("not available");
        ogl::drawText(posX + borderWidth + sc * 64 - textWidth / 2,
                      posY - borderWidth - sc * 64 - ogl::getFontHeight() / 2, 0, "not available", true);
    }
    else
    {
        ogl::disableLighting_useWithCare();                           // only temporarily
        ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!
        unsigned int textureName;
        textureName = ogl::genTexture(); // glGenTextures(1,&textureName);
        glBindTexture(GL_TEXTURE_2D, textureName);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, _thumbnailRGBAImage);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // GL_REPEAT
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glEnable(GL_TEXTURE_2D);
        glColor4d(ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[0],
                  ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[1],
                  ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[2], 1.0); // 0.125); // fading

        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);
        glVertex3i(posX + borderWidth, posY - borderWidth, 0);
        glTexCoord2f(1.0, 1.0);
        glVertex3i(posX + borderWidth + sc * 128, posY - borderWidth, 0);
        glTexCoord2f(1.0, 0.0);
        glVertex3i(posX + borderWidth + sc * 128, posY - borderWidth - sc * 128, 0);
        glTexCoord2f(0.0, 0.0);
        glVertex3i(posX + borderWidth, posY - borderWidth - sc * 128, 0);
        glEnd();
        ogl::enableLighting_useWithCare();
        glDisable(GL_TEXTURE_2D);
        ogl::setBlending(false);
        glDisable(GL_ALPHA_TEST);
        ogl::delTexture(textureName); // glDeleteTextures(1,&textureName);
    }
    ogl::setTextColor(ogl::colorGrey);

    std::vector<std::string> separateWords;
    std::vector<int> wordWidths;
    ogl::getSeparateWords(name, separateWords);
    ogl::getSeparateWordWidths(separateWords, wordWidths);

    int spaceWidth = ogl::getSeparateWordWidths(separateWords, wordWidths);
    int width;
    std::string line = ogl::getTextThatFitIntoPixelWidth(separateWords, wordWidths, spaceWidth, 128 * sc, width, false);
    if (separateWords.size() == 0)
    { // simple, the whole text fits onto one line!
        ogl::drawText(posX + borderWidth + sc * 64 - width / 2, posY - borderWidth - sc * 128 - textSpaceHeight / 2, 0,
                      line, true);
    }
    else
    { // trickier, the text doesn't fit onto one line!
        ogl::drawText(posX + borderWidth + sc * 64 - width / 2, posY - borderWidth - sc * 128 - textSpaceHeight / 4, 0,
                      line, true);
        int dotDotWidth = ogl::getTextLengthInPixels("..");
        line = ogl::getTextThatFitIntoPixelWidth(separateWords, wordWidths, spaceWidth, sc * 128 - dotDotWidth, width,
                                                 true);
        if (separateWords.size() == 0)
        { // All the text fits
            ogl::drawText(posX + borderWidth + sc * 64 - width / 2,
                          posY - borderWidth - sc * 128 - 3 * textSpaceHeight / 4, 0, line, true);
        }
        else
        { // Not all the text fits
            width += dotDotWidth;
            line += "..";
            ogl::drawText(posX + borderWidth + sc * 64 - width / 2,
                          posY - borderWidth - sc * 128 - 3 * textSpaceHeight / 4, 0, line, true);
        }
    }
}
