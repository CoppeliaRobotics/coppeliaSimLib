
#pragma once

#include "vMenubar.h"

static void handleVerSpec_sceneObjectOperations_addMenu1(VMenu* menu)
{
    int selItems=App::ct->objCont->getSelSize();
    int selDummies=App::ct->objCont->getDummyNumberInSelection();
    int shapeNumber=App::ct->objCont->getShapeNumberInSelection();
    int pathNumber=App::ct->objCont->getPathNumberInSelection();
    int simpleShapeNumber=0;
    std::vector<C3DObject*> objects;
    App::ct->objCont->getSelectedObjects(objects);
    for (unsigned int i=0;i<objects.size();i++)
    {
        if (objects[i]->getObjectType()==sim_object_shape_type)
        {
            CShape* it=(CShape*)objects[i];
            if (it->geomData->geomInfo->isGeometric())
                simpleShapeNumber++;
        }
    }


    bool noSim=App::ct->simulation->isSimulationStopped();
    bool lastSelIsShape=App::ct->objCont->isLastSelectionAShape();
    bool lastSelIsNonPureShape=false;
    bool lastSelIsNonGrouping=false;
    if (lastSelIsShape)
    {
        CShape* sh=App::ct->objCont->getLastSelection_shape();
        lastSelIsNonPureShape=!sh->geomData->geomInfo->isPure();
        lastSelIsNonGrouping=!sh->isCompound();
    }

    //bool lastSelIsPath=App::ct->objCont->isLastSelectionAPath();
    bool hasChildScriptAttached=false;
    bool hasCustomizationScriptAttached=false;
    if (selItems==1)
    {
        hasChildScriptAttached=(App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(App::ct->objCont->getSelID(0))!=nullptr);
        hasCustomizationScriptAttached=(App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(App::ct->objCont->getSelID(0))!=nullptr);
    }
    std::vector<int> rootSel;
    for (int i=0;i<App::ct->objCont->getSelSize();i++)
        rootSel.push_back(App::ct->objCont->getSelID(i));
    CSceneObjectOperations::addRootObjectChildrenToSelection(rootSel);
    int shapesInRootSel=App::ct->objCont->getShapeNumberInSelection(&rootSel);
    if (App::getEditModeType()==NO_EDIT_MODE)
    {
        menu->appendMenuItem(App::ct->undoBufferContainer->canUndo(),false,SCENE_OBJECT_OPERATION_UNDO_SOOCMD,IDSN_UNDO);
        menu->appendMenuItem(App::ct->undoBufferContainer->canRedo(),false,SCENE_OBJECT_OPERATION_REDO_SOOCMD,IDSN_REDO);
        menu->appendMenuSeparator();
        menu->appendMenuItem(selItems>1,false,SCENE_OBJECT_OPERATION_MAKE_PARENT_SOOCMD,IDS_MAKE_LAST_SELECTED_OBJECTS_PARENT_MENU_ITEM);
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_MAKE_ORPHANS_SOOCMD,IDS_MAKE_SELECTED_OBJECT_S__ORPHAN_MENU_ITEM);
        menu->appendMenuSeparator();
        menu->appendMenuItem((shapesInRootSel>0)&&noSim,false,SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_SHAPES_SOOCMD,IDS_CONVEX_MORPH_MENU_ITEM);
        menu->appendMenuItem((shapesInRootSel>0)&&noSim,false,SCENE_OBJECT_OPERATION_MORPH_INTO_CONVEX_DECOMPOSITION_SOOCMD,IDS_CONVEX_DECOMPOSITION_MORPH_MENU_ITEM);
        menu->appendMenuItem(lastSelIsShape&&(selItems==1)&&noSim&&lastSelIsNonPureShape,false,SCENE_OBJECT_OPERATION_DECIMATE_SHAPE_SOOCMD,IDS_MESH_DECIMATION_MENU_ITEM);
        menu->appendMenuItem(lastSelIsShape&&(selItems==1)&&noSim&&lastSelIsNonPureShape&&lastSelIsNonGrouping,false,SCENE_OBJECT_OPERATION_EXTRACT_SHAPE_INSIDE_SOOCMD,IDS_REMOVE_SHAPE_INSIDE_MENU_ITEM);

        if ((selItems==2)&&(selDummies==2))
        { // we have 2 selected dummies we might want to link/unlink:
            CDummy* dumA=App::ct->objCont->getDummy(App::ct->objCont->getSelID(0));
            CDummy* dumB=App::ct->objCont->getDummy(App::ct->objCont->getSelID(1));
            if ((dumA!=nullptr)&&(dumB!=nullptr))
            {
                if (dumA->getLinkedDummyID()==dumB->getObjectHandle())
                    menu->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_UNLINK_DUMMIES_SOOCMD,IDS_UNLINK_SELECTED_DUMMIES_MENU_ITEM);
                else
                {
                    VMenu* dummyLinking=new VMenu();
                    dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_IK_TIP_TARGET_SOOCMD,IDS_DUMMY_LINK_TYPE_IK_TIP_TARGET);
                    dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_LOOP_CLOSURE_SOOCMD,IDS_DUMMY_LINK_TYPE_GCS_LOOP_CLOSURE);
                    dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TIP_SOOCMD,IDS_DUMMY_LINK_TYPE_GCS_TIP);
                    dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_GCS_TARGET_SOOCMD,IDS_DUMMY_LINK_TYPE_GCS_TARGET);
                    dummyLinking->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_LINK_DUMMIES_DYNAMICS_LOOP_CLOSURE_SOOCMD,IDS_DUMMY_LINK_TYPE_DYNAMICS_LOOP_CLOSURE);
                    menu->appendMenuAndDetach(dummyLinking,true,IDS_LINK_SELECTED_DUMMIES_LINK_TYPE_MENU_ITEM);
                }
            }
        }
        menu->appendMenuSeparator();
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_OBJECT_FULL_COPY_SOOCMD,IDS_COPY_SELECTED_OBJECTS_MENU_ITEM);
        menu->appendMenuItem(!App::ct->copyBuffer->isBufferEmpty(),false,SCENE_OBJECT_OPERATION_PASTE_OBJECTS_SOOCMD,IDS_PASTE_BUFFER_MENU_ITEM);
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_DELETE_OBJECTS_SOOCMD,IDS_DELETE_SELECTED_OBJECTS_MENU_ITEM);
        menu->appendMenuItem(selItems>0,false,SCENE_OBJECT_OPERATION_OBJECT_FULL_CUT_SOOCMD,IDS_CUT_SELECTED_OBJECTS_MENU_ITEM);
        menu->appendMenuSeparator();
        menu->appendMenuItem(true,false,SCENE_OBJECT_OPERATION_SELECT_ALL_OBJECTS_SOOCMD,IDSN_SELECT_ALL_MENU_ITEM);
        menu->appendMenuSeparator();

        VMenu* removing=new VMenu();
        removing->appendMenuItem(hasChildScriptAttached&&noSim,false,SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CHILD_SCRIPT_SOOCMD,IDSN_ASSOCIATED_CHILD_SCRIPT_MENU_ITEM);
        removing->appendMenuItem(hasCustomizationScriptAttached&&noSim,false,SCENE_OBJECT_OPERATION_REMOVE_ASSOCIATED_CUSTOMIZATION_SCRIPT_SOOCMD,IDSN_ASSOCIATED_CUSTOMIZATION_SCRIPT_MENU_ITEM);
        menu->appendMenuAndDetach(removing,(hasChildScriptAttached||hasCustomizationScriptAttached)&&noSim,IDSN_REMOVE_MENU_ITEM);
        menu->appendMenuSeparator();

        if (App::ct->objCont->isLastSelectionAPath())
        {
            menu->appendMenuItem((selItems==1)&&noSim,false,SCENE_OBJECT_OPERATION_EMPTY_PATH_SOOCMD,IDS_EMPTY_LAST_SELECTED_PATH_MENU_ITEM);
            menu->appendMenuItem((selItems==1)&&noSim,false,SCENE_OBJECT_OPERATION_ROLL_PATH_POINTS_FORWARD_SOOCMD,IDS_ROLL_PATH_POINTS_FORWARD_MENU_ITEM);
            menu->appendMenuItem((selItems==1)&&noSim,false,SCENE_OBJECT_OPERATION_ROLL_PATH_POINTS_BACKWARD_SOOCMD,IDS_ROLL_PATH_POINTS_BACKWARD_MENU_ITEM);
            menu->appendMenuSeparator();
        }

        VMenu* grouping=new VMenu();
        grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>1)&&noSim,false,SCENE_OBJECT_OPERATION_GROUP_SHAPES_SOOCMD,IDS_GROUP_SELECTED_SHAPES_MENU_ITEM);
        grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>0)&&noSim,false,SCENE_OBJECT_OPERATION_UNGROUP_SHAPES_SOOCMD,IDS_UNGROUP_SELECTED_SHAPES_MENU_ITEM);
        grouping->appendMenuSeparator();
        grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>1)&&noSim,false,SCENE_OBJECT_OPERATION_MERGE_SHAPES_SOOCMD,IDS_MERGE_SELECTED_SHAPES_MENU_ITEM);
        grouping->appendMenuItem((shapeNumber==selItems)&&(selItems>0)&&noSim,false,SCENE_OBJECT_OPERATION_DIVIDE_SHAPES_SOOCMD,IDS_DIVIDE_SELECTED_SHAPES_MENU_ITEM);
        grouping->appendMenuSeparator();
        grouping->appendMenuItem((pathNumber==selItems)&&(selItems>1)&&noSim,false,SCENE_OBJECT_OPERATION_MERGE_PATHS_SOOCMD,IDS_MERGE_SELECTED_PATHS_MENU_ITEM);
        menu->appendMenuAndDetach(grouping,true,IDS_GROUPING_MERGING_MENU_ITEM);

        VMenu* align=new VMenu();
        align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_WORLD_SOOCMD,IDS_ALIGN_SELECTED_SHAPE_WORLD_MENU_ITEM);
        align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_MAIN_AXIS_SOOCMD,IDS_ALIGN_SELECTED_SHAPE_MAIN_AXIS_MENU_ITEM);
        align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_TUBE_MAIN_AXIS_SOOCMD,IDSN_ALIGN_BB_WITH_TUBE);
        align->appendMenuItem((shapeNumber==selItems)&&(selItems!=0)&&noSim,false,SCENE_OBJECT_OPERATION_ALIGN_BOUNDING_BOX_WITH_CUBOID_MAIN_AXIS_SOOCMD,IDSN_ALIGN_BB_WITH_CUBOID);
        menu->appendMenuAndDetach(align,true,IDS_BOUNDING_BOX_ALIGNMENT_MENU_ITEM);
    }
}
