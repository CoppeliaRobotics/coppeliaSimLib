
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "globalGuiTextureContainer.h"
#include "global.h"
#include "imgLoaderSaver.h"
#include "oGL.h"
#include "rendering.h"
#include "globalGuiTextureContainerBase.h"

CGlobalGuiTextureContainer::CGlobalGuiTextureContainer()
{
    int xres,yres;
    bool rgba;
    unsigned char* dat;

    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/world.tga",xres,yres,rgba,nullptr);
    addObject(WORLD_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/s_shape.tga",xres,yres,rgba,nullptr);
    addObject(SIMPLE_SHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/p_shape.tga",xres,yres,rgba,nullptr);
    addObject(PURE_SHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/h_shape.tga",xres,yres,rgba,nullptr);
    addObject(HEIGHTFIELD_SHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/mp_shape.tga",xres,yres,rgba,nullptr);
    addObject(PURE_MULTISHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/m_shape.tga",xres,yres,rgba,nullptr);
    addObject(MULTI_SHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/r_joint.tga",xres,yres,rgba,nullptr);
    addObject(REVOLUTE_JOINT_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/p_joint.tga",xres,yres,rgba,nullptr);
    addObject(PRISMATIC_JOINT_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/spheric.tga",xres,yres,rgba,nullptr);
    addObject(SPHERICAL_JOINT_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/camera.tga",xres,yres,rgba,nullptr);
    addObject(CAMERA_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/o_lighti.tga",xres,yres,rgba,nullptr);
    addObject(OMNI_LIGHT_OFF_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/o_lighta.tga",xres,yres,rgba,nullptr);
    addObject(OMNI_LIGHT_ON_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/s_lighti.tga",xres,yres,rgba,nullptr);
    addObject(SPOT_LIGHT_OFF_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/s_lighta.tga",xres,yres,rgba,nullptr);
    addObject(SPOT_LIGHT_ON_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/d_lighti.tga",xres,yres,rgba,nullptr);
    addObject(DIR_LIGHT_OFF_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/d_lighta.tga",xres,yres,rgba,nullptr);
    addObject(DIR_LIGHT_ON_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/graph.tga",xres,yres,rgba,nullptr);
    addObject(GRAPH_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/dummy.tga",xres,yres,rgba,nullptr);
    addObject(DUMMY_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/octree.tga",xres,yres,rgba,nullptr);
    addObject(OCTREE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/pointcloud.tga",xres,yres,rgba,nullptr);
    addObject(POINTCLOUD_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/model.tga",xres,yres,rgba,nullptr);
    addObject(MODEL_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/sensor.tga",xres,yres,rgba,nullptr);
    addObject(PROXIMITYSENSOR_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/rendsens.tga",xres,yres,rgba,nullptr);
    addObject(VISIONSENSOR_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/path.tga",xres,yres,rgba,nullptr);
    addObject(PATH_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;

    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/mirror.tga",xres,yres,rgba,nullptr);
    addObject(MIRROR_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/clipplane.tga",xres,yres,rgba,nullptr);
    addObject(CLIPPLANE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;

    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/convexs.tga",xres,yres,rgba,nullptr);
    addObject(CONVEX_SHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/convexm.tga",xres,yres,rgba,nullptr);
    addObject(CONVEX_MULTISHAPE_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;

    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/minus.tga",xres,yres,rgba,nullptr);
    addObject(MINUS_SIGN_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/plus.tga",xres,yres,rgba,nullptr);
    addObject(PLUS_SIGN_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/mscript.tga",xres,yres,rgba,nullptr);
    addObject(MAIN_SCRIPT_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/mcscript.tga",xres,yres,rgba,nullptr);
    addObject(MAIN_SCRIPT_CUSTOMIZED_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/script.tga",xres,yres,rgba,nullptr);
    addObject(SCRIPT_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/scriptDisabled.tga",xres,yres,rgba,nullptr);
    addObject(SCRIPTDISABLED_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/scriptthreaded.tga",xres,yres,rgba,nullptr);
    addObject(SCRIPT_THREADED_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/scriptthreadedDisabled.tga",xres,yres,rgba,nullptr);
    addObject(SCRIPTDISABLED_THREADED_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/scriptparameters.tga",xres,yres,rgba,nullptr);
    addObject(SCRIPT_PARAMETERS_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/scriptparametersactive.tga",xres,yres,rgba,nullptr);
    addObject(SCRIPT_PARAMETERS_ACTIVE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/pathpoint.tga",xres,yres,rgba,nullptr);
    addObject(PATH_POINT_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/vertex.tga",xres,yres,rgba,nullptr);
    addObject(VERTEX_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/triangle.tga",xres,yres,rgba,nullptr);
    addObject(TRIANGLE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/edge.tga",xres,yres,rgba,nullptr);
    addObject(EDGE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/2delement.tga",xres,yres,rgba,nullptr);
    addObject(BUTTON_BLOCK_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/mill.tga",xres,yres,rgba,nullptr);
    addObject(MILL_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/ftsensor.tga",xres,yres,rgba,nullptr);
    addObject(FORCE_SENSOR_TREE_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/warning.tga",xres,yres,rgba,nullptr);
    addObject(WARNING_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/dynamics.tga",xres,yres,rgba,nullptr);
    addObject(DYNAMICS_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/folder.tga",xres,yres,rgba,nullptr);
    addObject(FOLDER_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/customizationScript.tga",xres,yres,rgba,nullptr);
    addObject(CUSTOMIZATIONSCRIPT_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/customizationScriptDisabled.tga",xres,yres,rgba,nullptr);
    addObject(CUSTOMIZATIONSCRIPTDISABLED_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;


    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/checkedbgd.tga",xres,yres,rgba,nullptr);
    addObject(CHECKED_BACKGROUND_PICTURE,xres,yres,rgba,false,true,dat);
    delete[] dat;
    

    // Cursors:
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_arrow.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_ARROW,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_finger.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_FINGER,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_alldir.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_ALL_DIR,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_horizdir.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_HORIZONTAL_DIR,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_vertdir.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_VERTICAL_DIR,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_slash.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_SLASH_DIR,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_backslash.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_BACKSLASH_DIR,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_nobutt.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_NO_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_leftbutt.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_LEFT_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_middlebutt.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_MIDDLE_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_wheel.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_WHEEL_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_rightbutt.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_RIGHT_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_shift.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_SHIFT_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;
    dat=CImageLoaderSaver::loadQTgaImageData(":/targaFiles/cur_ctrl.tga",xres,yres,rgba,nullptr);
    addObject(CURSOR_CTRL_BUTTON,xres,yres,rgba,false,true,dat);
    delete[] dat;

    int tag;
    std::string str(CGlobalGuiTextureContainerBase::handleVerSpec_getWatermarkStr1(tag));
    if (str.size()>0)
    {
        dat=CImageLoaderSaver::loadQTgaImageData(str.c_str(),xres,yres,rgba,nullptr);
        addObject(tag,xres,yres,rgba,false,true,dat);
        delete[] dat;
    }
    str=CGlobalGuiTextureContainerBase::handleVerSpec_getWatermarkStr2(tag);
    if (str.size()>0)
    {
        dat=CImageLoaderSaver::loadQTgaImageData(str.c_str(),xres,yres,rgba,nullptr);
        addObject(tag,xres,yres,rgba,false,true,dat);
        delete[] dat;
    }
}

CGlobalGuiTextureContainer::~CGlobalGuiTextureContainer()
{
    removeAllObjects();
}

void CGlobalGuiTextureContainer::addObject(int pictureIndex,int sizeX,int sizeY,bool rgba,bool horizFlip,bool vertFlip,unsigned char* data)
{
    CTextureObject* it=new CTextureObject(sizeX,sizeY);
    it->setImage(rgba,horizFlip,vertFlip,data);
    _allTextureObjects.push_back(it);
    _allPictureIndices.push_back(pictureIndex);
}

void CGlobalGuiTextureContainer::removeAllObjects()
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
        delete _allTextureObjects[i];
    _allTextureObjects.clear();
    _allPictureIndices.clear();
}

void CGlobalGuiTextureContainer::startTextureDisplay(int pictureIndex)
{
    for (size_t i=0;i<_allPictureIndices.size();i++)
    {
        if (_allPictureIndices[i]==pictureIndex)
        {
            _startTextureDisplay(_allTextureObjects[i],true,0,false,false);
            break;
        }
    }
}

void CGlobalGuiTextureContainer::endTextureDisplay()
{
    _endTextureDisplay();
}
