#include <simInternal.h>
#include <addOperations.h>
#include <fileOperations.h>
#include <tt.h>
#include <meshRoutines.h>
#include <sceneObjectOperations.h>
#include <simFlavor.h>
#include <app.h>
#include <meshManip.h>
#include <mesh.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>
#include <set>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CAddOperations::CAddOperations()
{
}

CAddOperations::~CAddOperations()
{
}

CShape *CAddOperations::addPrimitiveShape(int type, const C3Vector &psizes, int options, const int subdiv[3],
                                          int faceSubdiv, int sides, int discSubdiv, bool dynamic, int pure,
                                          double density)
{ // pure=0: create non-pure, pure=1: create pure if possible, pure=2: force pure creation
    int sdiv[3] = {0, 0, 0};
    if (subdiv != nullptr)
    {
        sdiv[0] = subdiv[0];
        sdiv[1] = subdiv[1];
        sdiv[2] = subdiv[2];
    }
    C3Vector sizes = psizes;
    CShape *shape = nullptr;
    if (type == sim_primitiveshape_plane)
    {
        sizes(2) = 0.0001;
        int divX = sdiv[0] + 1;
        int divY = sdiv[1] + 1;
        double xhSize = sizes(0) / 2.0;
        double yhSize = sizes(1) / 2.0;
        double xs = sizes(0) / ((double)divX);
        double ys = sizes(1) / ((double)divY);
        std::vector<double> vertices;
        std::vector<int> indices;
        std::vector<double> normals;
        // We first create the vertices:
        for (int i = 0; i < (divY + 1); i++)
        { // along y
            for (int j = 0; j < (divX + 1); j++)
            { // along x
                tt::addToFloatArray(&vertices, -xhSize + j * xs, -yhSize + i * ys, 0.0);
            }
        }
        // Now we create the indices:
        for (int i = 0; i < divY; i++)
        { // along y
            for (int j = 0; j < divX; j++)
            { // along x
                tt::addToIntArray(&indices, i * (divX + 1) + j, i * (divX + 1) + j + 1, (i + 1) * (divX + 1) + j + 1);
                tt::addToIntArray(&indices, i * (divX + 1) + j, (i + 1) * (divX + 1) + j + 1, (i + 1) * (divX + 1) + j);
            }
        }
        // And now the normals:
        for (int i = 0; i < divX * divY * 6; i++)
            tt::addToFloatArray(&normals, 0.0, 0.0, 1.0);

        shape = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
        shape->setObjectAlias_direct(IDSOGL_PLANE);
        shape->setObjectName_direct_old(IDSOGL_PLANE);
        shape->setLocalTransformation(C3Vector(0.0, 0.0, 0.002)); // we shift the plane so that it is above the floor
        shape->getMesh()->setMass(sizes(0) * sizes(1) * density * 0.001); // we assume 1mm thickness
        shape->getMesh()->setPMI(C3Vector(sizes(1) * sizes(1) / 12.0, sizes(0) * sizes(0) / 12.0,
                                          (sizes(0) * sizes(0) + sizes(1) * sizes(1)) / 12.0));
    }

    if (type == sim_primitiveshape_cuboid)
    {
        int divX = sdiv[0] + 1;
        int divY = sdiv[1] + 1;
        int divZ = sdiv[2] + 1;
        double xhSize = sizes(0) / 2.0;
        double yhSize = sizes(1) / 2.0;
        double zhSize = sizes(2) / 2.0;
        std::vector<double> vertices;
        std::vector<int> indices;
        int theDiv[3] = {divX, divY, divZ};

        CMeshRoutines::createCube(vertices, indices, C3Vector(xhSize * 2.0, yhSize * 2.0, zhSize * 2.0), theDiv);

        shape = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
        shape->setObjectAlias_direct(IDSOGL_RECTANGLE);
        shape->setObjectName_direct_old(IDSOGL_RECTANGLE);
        shape->setLocalTransformation(
            C3Vector(0.0, 0.0, zhSize)); // we shift the rectangle so that it sits on the floor
        shape->getMesh()->setMass(sizes(0) * sizes(1) * sizes(2) * density);
        shape->getMesh()->setPMI(C3Vector((sizes(1) * sizes(1) + sizes(2) * sizes(2)) / 12.0,
                                          (sizes(0) * sizes(0) + sizes(2) * sizes(2)) / 12.0,
                                          (sizes(0) * sizes(0) + sizes(1) * sizes(1)) / 12.0));
    }

    if (type == sim_primitiveshape_spheroid)
    {
        if (pure == 2)
        {
            sizes(1) = sizes(0);
            sizes(2) = sizes(0);
        }
        if ((sizes(1) != sizes(0)) || (sizes(2) != sizes(0)))
            pure = 0;
        double xhSize = sizes(0) / 2.0;
        double yhSize = sizes(1) / 2.0;
        double zhSize = sizes(2) / 2.0;
        std::vector<double> vertices;
        std::vector<int> indices;
        if (sides == 0)
            sides = 32;
        if (sides < 3)
            sides = 3;
        faceSubdiv = sides / 2;

        CMeshRoutines::createSphere(vertices, indices, C3Vector(xhSize * 2.0, yhSize * 2.0, zhSize * 2.0), sides,
                                    faceSubdiv);

        shape = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
        shape->setObjectAlias_direct(IDSOGL_SPHERE);
        shape->setObjectName_direct_old(IDSOGL_SPHERE);
        shape->setLocalTransformation(C3Vector(0.0, 0.0, zhSize)); // we shift the sphere so that it sits on the floor
        double avR = (sizes(0) + sizes(1) + sizes(2)) / 6.0;

        shape->getMesh()->setMass((4.0 * piValue / 3.0) * avR * avR * avR * density);
        shape->getMesh()->setPMI(C3Vector(2.0 * avR * avR / 5.0, 2.0 * avR * avR / 5.0, 2.0 * avR * avR / 5.0));
        double avr2 = avR * 2.0;
        shape->getMesh()->scaleMassAndInertia(cbrt(sizes(0) * sizes(1) * sizes(2)) / avr2);
    }

    if ((type == sim_primitiveshape_cylinder) || (type == sim_primitiveshape_cone))
    {
        if (pure == 2)
            sizes(1) = sizes(0);
        if ((sizes(1) != sizes(0)) || ((options & 4) != 0))
            pure = 0;
        double xhSize = sizes(0) / 2.0;
        double yhSize = sizes(1) / 2.0;
        double zhSize = sizes(2) / 2.0;
        int discDiv = discSubdiv + 1;
        std::vector<double> vertices;
        std::vector<int> indices;
        if (sides == 0)
            sides = 32;
        if (sides < 3)
            sides = 3;

        CMeshRoutines::createCylinder(vertices, indices, C3Vector(xhSize * 2.0, yhSize * 2.0, zhSize * 2.0), sides,
                                      faceSubdiv + 1, discDiv, (options & 4) != 0, type == sim_primitiveshape_cone);

        shape = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
        if (type == sim_primitiveshape_cone)
        {
            shape->setObjectAlias_direct("Cone");
            shape->setObjectName_direct_old("Cone");
        }
        else
        {
            shape->setObjectAlias_direct(IDSOGL_CYLINDER);
            shape->setObjectName_direct_old(IDSOGL_CYLINDER);
        }
        shape->setLocalTransformation(C3Vector(0.0, 0.0, zhSize)); // Now we shift the cylinder so it sits on the floor
        double avR = (sizes(0) + sizes(1)) / 4.0;
        double divider = 1.0;
        if (type == sim_primitiveshape_cone)
            divider = 3.0;

        shape->getMesh()->setMass(piValue * avR * avR * divider * sizes(2) * density);
        if (type == sim_primitiveshape_cone)
            shape->getMesh()->setPMI(C3Vector(3.0 * (0.25 * avR * avR + sizes(2) * sizes(2)) / 5.0,
                                              3.0 * (0.25 * avR * avR + sizes(2) * sizes(2)) / 5.0,
                                              3.0 * avR * avR / 10.0));
        else
            shape->getMesh()->setPMI(C3Vector((3.0 * avR * avR + sizes(2) * sizes(2)) / 12.0,
                                              (3.0 * avR * avR + sizes(2) * sizes(2)) / 12.0, avR * avR / 2.0));
        double avR2 = avR * 2.0;
        shape->getMesh()->scaleMassAndInertia(cbrt(sizes(0) * sizes(1) * avR2) / avR2);
    }

    if (type == sim_primitiveshape_capsule)
    { // sizes(2) is the total length of the capsule. maxs=sizes(2)-max(sizes(0),sizes(1)) is the length of the cyl.
      // part
        if (sizes(0) > sizes(2) - 0.0001)
            sizes(0) = sizes(2) - 0.0001;
        if (sizes(1) > sizes(2) - 0.0001)
            sizes(1) = sizes(2) - 0.0001;
        double maxs = std::max<double>(sizes(0), sizes(1));
        double cylLength = sizes(2) - maxs;
        if (pure == 2)
            sizes(1) = sizes(0);
        if (sizes(1) != sizes(0))
            pure = 0;
        std::vector<double> vertices;
        std::vector<int> indices;
        if (sides == 0)
            sides = 32;
        if (sides < 3)
            sides = 3;

        CMeshRoutines::createCapsule(vertices, indices, C3Vector(sizes(0), sizes(1), cylLength), sides, faceSubdiv);

        shape = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
        shape->setObjectAlias_direct(IDSOGL_CAPSULE);
        shape->setObjectName_direct_old(IDSOGL_CAPSULE);
        shape->setLocalTransformation(C3Vector(0.0, 0.0, sizes(2) * 0.5));

        // For now, approximation:
        double avR = (sizes(0) + sizes(1)) / 4.0;
        double l = cylLength + maxs * 0.75;
        shape->getMesh()->setMass(piValue * avR * avR * l * density);
        shape->getMesh()->setPMI(
            C3Vector((3.0 * avR * avR + l * l) / 12.0, (3.0 * avR * avR + l * l) / 12.0, avR * avR / 2.0));
        double avR2 = avR * 2.0;
        shape->getMesh()->scaleMassAndInertia(cbrt(sizes(0) * sizes(1) * avR2) / avR2);
    }

    if (type == sim_primitiveshape_disc)
    {
        if (pure == 2)
            sizes(1) = sizes(0);
        if (sizes(1) != sizes(0))
            pure = 0;
        sizes(2) = 0.0001;
        double xhSize = sizes(0) / 2.0;
        double yhSize = sizes(1) / 2.0;
        int discDiv = discSubdiv + 1;
        std::vector<double> vertices;
        std::vector<int> indices;
        double dd = 1.0 / ((double)discDiv);
        if (sides == 0)
            sides = 32;
        if (sides < 3)
            sides = 3;
        double sa = 2.0 * piValue / ((double)sides);
        // The two middle vertices:
        int sideStart = 1;
        tt::addToFloatArray(&vertices, 0.0, 0.0, 0.0);
        for (int i = 0; i < sides; i++)
        { // The side vertices:
            tt::addToFloatArray(&vertices, (double)cos(sa * i), (double)sin(sa * i), 0.0);
        }
        int dstStart = (int)vertices.size() / 3;
        // The disc subdivision vertices:
        for (int i = 1; i < discDiv; i++)
            for (int j = 0; j < sides; j++)
                tt::addToFloatArray(&vertices, (1.0 - dd * i) * (double)cos(sa * j),
                                    (1.0 - dd * i) * (double)sin(sa * j), 0.0);
        // We set up the indices:
        for (int i = 0; i < sides - 1; i++)
        {
            if (discDiv == 1)
                tt::addToIntArray(&indices, 0, i + sideStart, (i + 1) + sideStart);
            else
            {
                tt::addToIntArray(&indices, 0, dstStart + i + sides * (discDiv - 2),
                                  dstStart + i + sides * (discDiv - 2) + 1);
                for (int j = 0; j < discDiv - 2; j++)
                {
                    tt::addToIntArray(&indices, dstStart + j * sides + i, dstStart + j * sides + i + 1,
                                      dstStart + (j + 1) * sides + i);
                    tt::addToIntArray(&indices, dstStart + j * sides + i + 1, dstStart + (j + 1) * sides + i + 1,
                                      dstStart + (j + 1) * sides + i);
                }
                tt::addToIntArray(&indices, sideStart + i, sideStart + (i + 1), dstStart + i);
                tt::addToIntArray(&indices, sideStart + (i + 1), dstStart + i + 1, dstStart + i);
            }
        }

        // We have to close the cylinder here:
        // First top and bottom part:
        if (discDiv == 1)
            tt::addToIntArray(&indices, 0, (sides - 1) + sideStart, sideStart);
        else
        {
            tt::addToIntArray(&indices, 0, dstStart + sides * (discDiv - 1) - 1, dstStart + sides * (discDiv - 2));
            for (int j = 0; j < discDiv - 2; j++)
            {
                tt::addToIntArray(&indices, dstStart + j * sides + sides - 1, dstStart + j * sides,
                                  dstStart + (j + 2) * sides - 1);
                tt::addToIntArray(&indices, dstStart + j * sides, dstStart + (j + 1) * sides,
                                  dstStart + (j + 2) * sides - 1);
            }
            tt::addToIntArray(&indices, sideStart + (sides - 1), sideStart, dstStart + sides - 1);
            tt::addToIntArray(&indices, sideStart, dstStart, dstStart + sides - 1);
        }
        // Now we scale the disc:
        for (int i = 0; i < int(vertices.size()) / 3; i++)
        {
            C3Vector p(vertices[3 * i + 0], vertices[3 * i + 1], 0.0);
            p(0) = p(0) * xhSize;
            p(1) = p(1) * yhSize;
            vertices[3 * i + 0] = p(0);
            vertices[3 * i + 1] = p(1);
        }

        shape = new CShape(C7Vector::identityTransformation, vertices, indices, nullptr, nullptr, 0);
        shape->setObjectAlias_direct(IDSOGL_DISC);
        shape->setObjectName_direct_old(IDSOGL_DISC);
        shape->setLocalTransformation(
            C3Vector(0.0, 0.0, 0.002)); // Now we shift the disc so it sits just above the floor
        double avR = (sizes(0) + sizes(1)) / 4.0;
        shape->getMesh()->setMass(piValue * avR * avR * density * 0.001); // we assume 1mm thickness
        shape->getMesh()->setPMI(C3Vector(3.0 * (avR * avR) / 12.0, 3.0 * (avR * avR) / 12.0, avR * avR / 2.0));
        double avR2 = avR * 2.0;
        shape->getMesh()->scaleMassAndInertia(cbrt(sizes(0) * sizes(1) * avR2) / avR2);
    }

    if (shape != nullptr)
    {
        shape->getSingleMesh()->color.setDefaultValues();
        shape->setColor(nullptr, sim_colorcomponent_ambient_diffuse, 1.0, 1.0, 1.0);
        shape->setObjectAltName_direct_old(
            tt::getObjectAltNameFromObjectName(shape->getObjectName_old().c_str()).c_str());
        if ((options & 2) == 0)
        {
            shape->getSingleMesh()->setShadingAngle(30.0 * degToRad);
            shape->getSingleMesh()->setEdgeThresholdAngle(30.0 * degToRad);
        }
        if (pure != 0)
            shape->getSingleMesh()->setPurePrimitiveType(type, sizes(0), sizes(1), sizes(2));
        if (dynamic)
        {
            int propToRemove = sim_objectspecialproperty_collidable | sim_objectspecialproperty_measurable |
                               sim_objectspecialproperty_detectable;
            shape->setLocalObjectSpecialProperty((shape->getLocalObjectSpecialProperty() | propToRemove) -
                                                 propToRemove);
            shape->setRespondable(true);
            shape->setStatic(false);
            shape->setColor(nullptr, sim_colorcomponent_ambient_diffuse, 0.85f, 0.85f, 1.0f);
            shape->setRespondable(true);
        }
        shape->getDynMaterial()->generateDefaultMaterial(sim_dynmat_reststackgrasp);
        shape->setCulling((options & 1) != 0);
        App::currentWorld->sceneObjects->addObjectToScene(shape, false, true);
    }
    return (shape);
}

#ifdef SIM_WITH_GUI
void CAddOperations::addMenu(VMenu *menu, CSView *subView, bool onlyCamera)
{
    std::vector<CSceneObject *> sel;
    App::currentWorld->sceneObjects->getSelectedObjects(sel, -1, true, true);
    int shapeCnt = 0;
    int dummyCnt = 0;
    int octreeCnt = 0;
    int ptcloudCnt = 0;
    for (size_t i = 0; i < sel.size(); i++)
    {
        int t = sel[i]->getObjectType();
        if (t == sim_object_shape_type)
            shapeCnt++;
        if (t == sim_object_dummy_type)
            dummyCnt++;
        if (t == sim_object_octree_type)
            octreeCnt++;
        if (t == sim_object_pointcloud_type)
            ptcloudCnt++;
    }

    // subView can be null
    bool canAddChildScript = false;
    bool canAddCustomizationScript = false;
    if (App::currentWorld->sceneObjects->getSelectionCount() == 1)
    {
        canAddChildScript = (App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                                 sim_scripttype_childscript,
                                 App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0)) == nullptr) &&
                            App::currentWorld->simulation->isSimulationStopped();
        canAddCustomizationScript =
            (App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                 sim_scripttype_customizationscript,
                 App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0)) == nullptr) &&
            App::currentWorld->simulation->isSimulationStopped();
    }

    bool linkedObjIsInexistentOrNotGraphNorRenderingSens = true;
    if (subView != nullptr)
        linkedObjIsInexistentOrNotGraphNorRenderingSens =
            ((App::currentWorld->sceneObjects->getGraphFromHandle(subView->getLinkedObjectID()) == nullptr) &&
             (App::currentWorld->sceneObjects->getVisionSensorFromHandle(subView->getLinkedObjectID()) == nullptr));
    bool itemsPresent = false;
    if (subView != nullptr)
    {
        menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_FLOATING_VIEW_ACCMD, IDS_FLOATING_VIEW_MENU_ITEM);
        itemsPresent = true;
        if (onlyCamera)
        {
            if (linkedObjIsInexistentOrNotGraphNorRenderingSens)
            {
                VMenu *camera = new VMenu();
                camera->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                       ADD_COMMANDS_ADD_PERSPECTIVE_CAMERA_ACCMD, "Perspective type");
                camera->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                       ADD_COMMANDS_ADD_ORTHOGONAL_CAMERA_ACCMD, "Orthogonal type");
                menu->appendMenuAndDetach(camera, true, "Camera");
            }
            else
                menu->appendMenuItem(false, false, 0, "Camera");
        }
    }
    CSceneObject *associatedViewable = nullptr;
    if (subView != nullptr)
        associatedViewable = App::currentWorld->sceneObjects->getObjectFromHandle(subView->getLinkedObjectID());
    if ((subView == nullptr) ||
        ((associatedViewable != nullptr) && (associatedViewable->getObjectType() == sim_object_camera_type)))
    {
        if (!onlyCamera)
        {
            if (itemsPresent)
                menu->appendMenuSeparator();
            VMenu *prim = new VMenu();
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_PLANE_ACCMD, IDS_PLANE_MENU_ITEM);
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_DISC_ACCMD, IDS_DISC_MENU_ITEM);
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD, IDS_RECTANGLE_MENU_ITEM);
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD, IDS_SPHERE_MENU_ITEM);
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD, IDS_CYLINDER_MENU_ITEM);
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_CAPSULE_ACCMD, IDS_CAPSULE_MENU_ITEM);
            prim->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRIMITIVE_CONE_ACCMD, IDS_CONE_MENU_ITEM);
            menu->appendMenuAndDetach(prim, true, IDS_PRIMITIVE_SHAPE_MENU_ITEM);
            itemsPresent = true;

            if (CSimFlavor::getBoolVal(7))
            {
                VMenu *joint = new VMenu();
                joint->appendMenuItem(true, false, ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD, IDS_REVOLUTE_MENU_ITEM);
                joint->appendMenuItem(true, false, ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD, IDS_PRISMATIC_MENU_ITEM);
                joint->appendMenuItem(true, false, ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD, IDS_SPHERICAL_MENU_ITEM);
                menu->appendMenuAndDetach(joint, true, IDS_JOINT_MENU_ITEM);
                itemsPresent = true;
            }
        }
        else
        {
            if (itemsPresent)
                menu->appendMenuSeparator();
        }

        if (linkedObjIsInexistentOrNotGraphNorRenderingSens)
        {
            VMenu *camera = new VMenu();
            camera->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                   ADD_COMMANDS_ADD_PERSPECTIVE_CAMERA_ACCMD, "Perspective type");
            camera->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                   ADD_COMMANDS_ADD_ORTHOGONAL_CAMERA_ACCMD, "Orthogonal type");
            menu->appendMenuAndDetach(camera, true, "Camera");
        }
        else
            menu->appendMenuItem(false, false, 0, "Camera");

        if (CSimFlavor::getBoolVal(7) && App::userSettings->enableOldMirrorObjects)
            menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_MIRROR_ACCMD, IDS_MIRROR_MENU_ITEM);

        itemsPresent = true;

        if (CSimFlavor::getBoolVal(7) && (!onlyCamera))
        {
            VMenu *light = new VMenu();
            light->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                  ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD, IDS_OMNIDIRECTIONAL_MENU_ITEM);
            light->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                  ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD, IDS_SPOTLIGHT_MENU_ITEM);
            light->appendMenuItem(linkedObjIsInexistentOrNotGraphNorRenderingSens, false,
                                  ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD, IDS_DIRECTIONAL_MENU_ITEM);
            if (linkedObjIsInexistentOrNotGraphNorRenderingSens)
                menu->appendMenuAndDetach(light, true, IDS_LIGHT_MENU_ITEM);
            else
            {
                menu->appendMenuItem(false, false, 0, IDS_LIGHT_MENU_ITEM);
                delete light;
            }

            menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_DUMMY_ACCMD, IDS_DUMMY_MENU_ITEM);
            menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_POINTCLOUD_ACCMD, IDS_POINTCLOUD_MENU_ITEM);
            menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_OCTREE_ACCMD, IDS_OCTREE_MENU_ITEM);

            menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_GRAPH_ACCMD, IDS_GRAPH_MENU_ITEM);

            VMenu *sens = new VMenu();
            sens->appendMenuItem(true, false, ADD_COMMANDS_ADD_RAY_PROXSENSOR_ACCMD, IDS_RAY_TYPE_MENU_ITEM);
            sens->appendMenuItem(true, false, ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD,
                                 IDS_RANDOMIZED_RAY_TYPE_MENU_ITEM);
            sens->appendMenuItem(true, false, ADD_COMMANDS_ADD_PYRAMID_PROXSENSOR_ACCMD, IDS_PYRAMID_TYPE_MENU_ITEM);
            sens->appendMenuItem(true, false, ADD_COMMANDS_ADD_CYLINDER_PROXSENSOR_ACCMD, IDS_CYLINDER_TYPE_MENU_ITEM);
            sens->appendMenuItem(true, false, ADD_COMMANDS_ADD_DISC_PROXSENSOR_ACCMD, IDS_DISC_TYPE_MENU_ITEM);
            sens->appendMenuItem(true, false, ADD_COMMANDS_ADD_CONE_PROXSENSOR_ACCMD, IDS_CONE_TYPE_MENU_ITEM);

            menu->appendMenuAndDetach(sens, true, IDS_PROXSENSOR_MENU_ITEM);

            VMenu *camera = new VMenu();
            camera->appendMenuItem(true, false, ADD_COMMANDS_ADD_VISION_SENSOR_PERSPECTIVE_ACCMD, "Perspective type");
            camera->appendMenuItem(true, false, ADD_COMMANDS_ADD_VISION_SENSOR_ORTHOGONAL_ACCMD, "Orthogonal type");
            menu->appendMenuAndDetach(camera, true, "Vision sensor");

            menu->appendMenuItem(true, false, ADD_COMMANDS_ADD_FORCE_SENSOR_ACCMD, IDSN_FORCE_SENSOR);

            VMenu *pathM = new VMenu();
            pathM->appendMenuItem(true, false, ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD, IDS_SEGMENT_TYPE_MENU_ITEM);
            pathM->appendMenuItem(true, false, ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD, IDS_CIRCLE_TYPE_MENU_ITEM);
            menu->appendMenuAndDetach(pathM, true, IDSN_PATH);

            if (App::userSettings->useSceneObjectScripts)
            {
                VMenu *script = new VMenu();

                VMenu *childScript = new VMenu();
                VMenu *childScriptNonThreaded = new VMenu();
                childScriptNonThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_NON_THREADED_CHILD_SCRIPT_LUA_ACCMD, "Lua");
                childScriptNonThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_NON_THREADED_CHILD_SCRIPT_PYTHON_ACCMD, "Python");
                childScript->appendMenuAndDetach(childScriptNonThreaded, true, "Non threaded");
                VMenu *childScriptThreaded = new VMenu();
                childScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_LUA_ACCMD, "Lua");
                childScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_PYTHON_ACCMD, "Python");
                childScript->appendMenuAndDetach(childScriptThreaded, true, "Threaded");
                script->appendMenuAndDetach(childScript, true, "child script");

                VMenu *customizationScript = new VMenu();
                VMenu *customizationScriptNonThreaded = new VMenu();
                customizationScriptNonThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_NON_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD, "Lua");
                customizationScriptNonThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_NON_THREADED_CUSTOMIZATION_SCRIPT_PYTHON_ACCMD, "Python");
                customizationScript->appendMenuAndDetach(customizationScriptNonThreaded, true, "Non threaded");
                VMenu *customizationScriptThreaded = new VMenu();
                customizationScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD, "Lua");
                customizationScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_PYTHON_ACCMD, "Python");
                customizationScript->appendMenuAndDetach(customizationScriptThreaded, true, "Threaded");
                script->appendMenuAndDetach(customizationScript, true, "customization script");

                menu->appendMenuAndDetach(script, true, "Script");
            }
            else
            {
                VMenu *childScript = new VMenu();
                VMenu *childScriptNonThreaded = new VMenu();
                childScriptNonThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_NON_THREADED_CHILD_SCRIPT_LUA_ACCMD,
                                                       "Lua");
                childScriptNonThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_NON_THREADED_CHILD_SCRIPT_PYTHON_ACCMD,
                                                       "Python");
                childScript->appendMenuAndDetach(childScriptNonThreaded, canAddChildScript, "Non threaded");
                VMenu *childScriptThreaded = new VMenu();
                childScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_LUA_ACCMD, "Lua");
                childScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_PYTHON_ACCMD,
                                                    "Python");
                if (App::userSettings->keepOldThreadedScripts)
                    childScriptThreaded->appendMenuItem(true, false, ADD_COMMANDS_ADD_oldTHREADED_CHILD_SCRIPT_LUA_ACCMD,
                                                        "Lua (deprecated, compatibility version)");
                childScript->appendMenuAndDetach(childScriptThreaded, canAddChildScript, "Threaded");
                menu->appendMenuAndDetach(childScript, canAddChildScript, "Associated child script");

                VMenu *customizationScript = new VMenu();
                VMenu *customizationScriptNonThreaded = new VMenu();
                customizationScriptNonThreaded->appendMenuItem(
                    true, false, ADD_COMMANDS_ADD_NON_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD, "Lua");
                customizationScriptNonThreaded->appendMenuItem(
                    true, false, ADD_COMMANDS_ADD_NON_THREADED_CUSTOMIZATION_SCRIPT_PYTHON_ACCMD, "Python");
                customizationScript->appendMenuAndDetach(customizationScriptNonThreaded, canAddCustomizationScript,
                                                         "Non threaded");
                VMenu *customizationScriptThreaded = new VMenu();
                customizationScriptThreaded->appendMenuItem(
                    true, false, ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD, "Lua");
                customizationScriptThreaded->appendMenuItem(
                    true, false, ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_PYTHON_ACCMD, "Python");
                customizationScript->appendMenuAndDetach(customizationScriptThreaded, canAddCustomizationScript,
                                                         "Threaded");
                menu->appendMenuAndDetach(customizationScript, canAddCustomizationScript,
                                          "Associated customization script");
            }
        }
    }
}

CShape *CAddOperations::addPrimitive_withDialog(int command, const C3Vector *optSizes)
{ // if optSizes is not nullptr, then sizes are locked in the dialog
    CShape *retVal = nullptr;
    int pType = -1;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_PLANE_ACCMD)
        pType = sim_primitiveshape_plane;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD)
        pType = sim_primitiveshape_cuboid;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD)
        pType = sim_primitiveshape_spheroid;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_CAPSULE_ACCMD)
        pType = sim_primitiveshape_capsule;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD)
        pType = sim_primitiveshape_cylinder;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_DISC_ACCMD)
        pType = sim_primitiveshape_disc;
    if (command == ADD_COMMANDS_ADD_PRIMITIVE_CONE_ACCMD)
        pType = sim_primitiveshape_cone;
    if (pType != -1)
    {
        C3Vector sizes;
        int subdiv[3];
        int faceSubdiv, sides, discSubdiv;
        bool smooth, dynamic, openEnds;
        double density;
        if (GuiApp::uiThread->showPrimitiveShapeDialog(pType, optSizes, sizes, subdiv, faceSubdiv, sides, discSubdiv,
                                                       smooth, openEnds, dynamic, density))
        {
            int options = 0;
            if (!smooth)
                options |= 2;
            if (openEnds)
                options |= 4;
            int p = 1;
            if (dynamic)
                p = 2;
            retVal =
                addPrimitiveShape(pType, sizes, options, subdiv, faceSubdiv, sides, discSubdiv, dynamic, p, density);
        }
    }
    return (retVal);
}

bool CAddOperations::processCommand(int commandID, CSView *subView)
{ // Return value is true if the command belonged to Add menu and was executed
    if ((commandID == ADD_COMMANDS_ADD_PRIMITIVE_PLANE_ACCMD) || (commandID == ADD_COMMANDS_ADD_PRIMITIVE_DISC_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_PRIMITIVE_CONE_ACCMD) || (commandID == ADD_COMMANDS_ADD_PRIMITIVE_CAPSULE_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_PRIMITIVE_SHAPE);

            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CShape *newShape = addPrimitive_withDialog(commandID, nullptr);
            int shapeHandle = -1;
            if (newShape != nullptr)
                shapeHandle = newShape->getObjectHandle();
            if (shapeHandle != -1)
            {
                if (sel != nullptr)
                {
                    App::currentWorld->sceneObjects->setObjectParent(newShape, sel, true);
                    sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
                }
                App::currentWorld->sceneObjects->deselectObjects();
                App::currentWorld->sceneObjects->selectObject(shapeHandle);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
                App::logMsg(sim_verbosity_msgs, "Operation aborted.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == ADD_COMMANDS_ADD_FLOATING_VIEW_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_FLOATING_VIEW);
            App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex())
                ->addFloatingView();
            App::undoRedo_sceneChanged("");
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ((commandID == ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD) || (commandID == ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_JOINT);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CJoint *newObject = nullptr;
            if (commandID == ADD_COMMANDS_ADD_REVOLUTE_JOINT_ACCMD)
                newObject = new CJoint(sim_joint_revolute_subtype);
            if (commandID == ADD_COMMANDS_ADD_PRISMATIC_JOINT_ACCMD)
                newObject = new CJoint(sim_joint_prismatic_subtype);
            if (commandID == ADD_COMMANDS_ADD_SPHERICAL_JOINT_ACCMD)
                newObject = new CJoint(sim_joint_spherical_subtype);
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID == ADD_COMMANDS_ADD_PERSPECTIVE_CAMERA_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_ORTHOGONAL_CAMERA_ACCMD) || (commandID == ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD) || (commandID == ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            int lo = -1;
            if (subView != nullptr)
                lo = subView->getLinkedObjectID();
            CCamera *camera = App::currentWorld->sceneObjects->getCameraFromHandle(lo);
            CGraph *graph = App::currentWorld->sceneObjects->getGraphFromHandle(lo);
            if (graph != nullptr)
                return (true);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CCamera *myNewCamera = nullptr;
            CLight *myNewLight = nullptr;
            if ((commandID == ADD_COMMANDS_ADD_PERSPECTIVE_CAMERA_ACCMD) ||
                (commandID == ADD_COMMANDS_ADD_ORTHOGONAL_CAMERA_ACCMD))
            {
                App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_CAMERA);
                myNewCamera = new CCamera();
                myNewCamera->setPerspective(commandID == ADD_COMMANDS_ADD_PERSPECTIVE_CAMERA_ACCMD);
                App::currentWorld->sceneObjects->addObjectToScene(myNewCamera, false, true);
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            else
            {
                App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_LIGHT);
                int tp;
                if (commandID == ADD_COMMANDS_ADD_OMNI_LIGHT_ACCMD)
                    tp = sim_light_omnidirectional_subtype;
                if (commandID == ADD_COMMANDS_ADD_SPOT_LIGHT_ACCMD)
                    tp = sim_light_spot_subtype;
                if (commandID == ADD_COMMANDS_ADD_DIR_LIGHT_ACCMD)
                    tp = sim_light_directional_subtype;
                myNewLight = new CLight(tp);
                App::currentWorld->sceneObjects->addObjectToScene(myNewLight, false, true);
                App::logMsg(sim_verbosity_msgs, "done.");
            }
            CSceneObject *addedObject = myNewCamera;
            if (addedObject == nullptr)
                addedObject = myNewLight;
            addedObject->setLocalTransformation(C3Vector(0.0, 0.0, 1.0));
            addedObject->setLocalTransformation(C4Vector(piValue * 0.5, 0.0, 0.0));
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(addedObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }

            if (camera != nullptr)
            {
                if (myNewCamera != nullptr)
                {
                    App::currentWorld->sceneObjects->selectObject(myNewCamera->getObjectHandle());
                    C7Vector m(camera->getFullCumulativeTransformation());
                    myNewCamera->setLocalTransformation(m);
                    myNewCamera->scaleObject(camera->getCameraSize() / myNewCamera->getCameraSize());
                    C3Vector hs(myNewCamera->getBBHSize());
                    m = myNewCamera->getLocalTransformation();
                    double averageSize = (hs(0) + hs(1) + hs(2)) / 1.5;
                    double shiftForward = camera->getNearClippingPlane() + hs(2) * 2.0 + 3.0 * averageSize;
                    m.X += (m.Q.getAxis(2) * shiftForward);
                    myNewCamera->setLocalTransformation(m.X);
                }
            }
            else if (subView != nullptr)
            { // When we want to add a camera to an empty window
                if (myNewCamera != nullptr)
                {
                    C7Vector m;
                    m.X = C3Vector(-1.12, 1.9, 1.08);
                    m.Q.setEulerAngles(C3Vector(110.933 * degToRad, 28.703 * degToRad, -10.41 * degToRad));
                    myNewCamera->setLocalTransformation(m);
                    subView->setLinkedObjectID(myNewCamera->getObjectHandle(), false);
                }
            }
            App::currentWorld->sceneObjects->selectObject(addedObject->getObjectHandle());
            App::undoRedo_sceneChanged("");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.objectParams.push_back(subView);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == ADD_COMMANDS_ADD_MIRROR_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_MIRROR);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CMirror *newObject = new CMirror();
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            App::currentWorld->sceneObjects->setObjectAbsoluteOrientation(newObject->getObjectHandle(),
                                                                          C3Vector(piValD2, 0.0, 0.0));
            App::currentWorld->sceneObjects->setObjectAbsolutePosition(
                newObject->getObjectHandle(), C3Vector(0.0, 0.0, newObject->getMirrorHeight() * 0.5));

            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged("");
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == ADD_COMMANDS_ADD_DUMMY_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_DUMMY);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CDummy *newObject = new CDummy();
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged("");
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == ADD_COMMANDS_ADD_OCTREE_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_AN_OCTREE);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            COcTree *newObject = new COcTree();
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged("");
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == ADD_COMMANDS_ADD_POINTCLOUD_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_POINTCLOUD);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CPointCloud *newObject = new CPointCloud();
            /*
                        std::vector<double> v;
                        for (size_t i=0;i<50000;i++)
                        {
                            double x=sin(double(i)/500.0);
                            double y=cos(double(i)/500.0);
                            double z=double(i)/50000.0;
                            v.push_back(x);
                            v.push_back(y);
                            v.push_back(z);
                        }
                        newObject->insertPoints(&v[0],v.size()/3,true,nullptr);
                        //*/
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::undoRedo_sceneChanged("");
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID >= ADD_COMMANDS_ADD_NON_THREADED_CHILD_SCRIPT_LUA_ACCMD) &&
        (commandID <= ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_PYTHON_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool isLua = (commandID < ADD_COMMANDS_ADD_NON_THREADED_CHILD_SCRIPT_PYTHON_ACCMD);
            bool isThreaded = (commandID == ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_LUA_ACCMD) || (commandID == ADD_COMMANDS_ADD_THREADED_CHILD_SCRIPT_PYTHON_ACCMD);
            if (App::userSettings->useSceneObjectScripts)
            {
                CSceneObject* sel = nullptr;
                if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                    sel = App::currentWorld->sceneObjects->getLastSelectionObject();
                int scriptHandle = App::currentWorld->sceneObjects->addDefaultScript(sim_scripttype_childscript, isThreaded, isLua);
                if ( (sel != nullptr) && (scriptHandle != -1) )
                {
                    CSceneObject* script = App::currentWorld->sceneObjects->getObjectFromHandle(scriptHandle);
                    App::currentWorld->sceneObjects->setObjectParent(script, sel, false);
                    sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
                    App::currentWorld->sceneObjects->deselectObjects();
                    App::currentWorld->sceneObjects->selectObject(scriptHandle);
                }
            }
            else
            { // legacy scripts
                if (App::currentWorld->sceneObjects->getSelectionCount() == 1)
                {
                    int scriptID = App::currentWorld->sceneObjects->embeddedScriptContainer->insertDefaultScript(
                        sim_scripttype_childscript, isThreaded, isLua,
                        commandID == ADD_COMMANDS_ADD_oldTHREADED_CHILD_SCRIPT_LUA_ACCMD);
                    CScriptObject *script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptObjectFromHandle(scriptID);
                    if (script != nullptr)
                        script->setObjectHandleThatScriptIsAttachedTo(
                            App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                }
            }
            App::undoRedo_sceneChanged("");
            GuiApp::setFullDialogRefreshFlag();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ((commandID >= ADD_COMMANDS_ADD_NON_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD) &&
        (commandID <= ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_PYTHON_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool isLua = (commandID <= ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD);
            bool isThreaded = (commandID == ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_LUA_ACCMD) || (commandID == ADD_COMMANDS_ADD_THREADED_CUSTOMIZATION_SCRIPT_PYTHON_ACCMD);
            if (App::userSettings->useSceneObjectScripts)
                App::currentWorld->sceneObjects->addDefaultScript(sim_scripttype_customizationscript, isThreaded, isLua);
            else
            { // legacy scripts
                if (App::currentWorld->sceneObjects->getSelectionCount() == 1)
                {
                    int scriptID = App::currentWorld->sceneObjects->embeddedScriptContainer->insertDefaultScript(
                        sim_scripttype_customizationscript, isThreaded, isLua);
                    CScriptObject *script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptObjectFromHandle(scriptID);
                    if (script != nullptr)
                        script->setObjectHandleThatScriptIsAttachedTo(
                            App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
                }
            }
            App::undoRedo_sceneChanged("");
            GuiApp::setFullDialogRefreshFlag();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ((commandID == ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD) || (commandID == ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_PATH);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            std::string txt;
            if (commandID == ADD_COMMANDS_ADD_PATH_SEGMENT_ACCMD)
                txt += "local "
                       "pathData={-2.5000e-01,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,1.0000e+00,2."
                       "5000e-01,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,0.0000e+00,1.0000e+00}";
            else
                txt += "local "
                       "pathData={2.5490e-01,0.0000e+00,0.0000e+00,-2.1073e-08,-2.9802e-08,2.1073e-08,1.0000e+00,2."
                       "3549e-01,9.7545e-02,0.0000e+00,0.0000e+00,0.0000e+00,1.9509e-01,9.8079e-01,1.8024e-01,1.8024e-"
                       "01,0.0000e+00,4.4703e-08,0.0000e+00,3.8268e-01,9.2388e-01,9.7545e-02,2.3549e-01,0.0000e+00,-2."
                       "9802e-08,-2.9802e-08,5.5557e-01,8.3147e-01,-1.1142e-08,2.5490e-01,0.0000e+00,0.0000e+00,0."
                       "0000e+00,7.0711e-01,7.0711e-01,-9.7545e-02,2.3549e-01,0.0000e+00,-2.9802e-08,-5.9605e-08,8."
                       "3147e-01,5.5557e-01,-1.8024e-01,1.8024e-01,0.0000e+00,-8.9407e-08,-4.4703e-08,9.2388e-01,3."
                       "8268e-01,-2.3549e-01,9.7545e-02,0.0000e+00,0.0000e+00,0.0000e+00,9.8079e-01,1.9509e-01,-2."
                       "5490e-01,3.8488e-08,0.0000e+00,-2.9802e-08,-2.1073e-08,1.0000e+00,2.1073e-08,-2.3549e-01,-9."
                       "7545e-02,0.0000e+00,-5.9605e-08,4.4703e-08,9.8079e-01,-1.9509e-01,-1.8024e-01,-1.8024e-01,0."
                       "0000e+00,0.0000e+00,-4.4703e-08,9.2388e-01,-3.8268e-01,-9.7545e-02,-2.3549e-01,0.0000e+00,-5."
                       "9605e-08,5.9605e-08,8.3147e-01,-5.5557e-01,3.0396e-09,-2.5490e-01,0.0000e+00,0.0000e+00,0."
                       "0000e+00,7.0711e-01,-7.0711e-01,9.7545e-02,-2.3549e-01,0.0000e+00,0.0000e+00,0.0000e+00,5."
                       "5557e-01,-8.3147e-01,1.8024e-01,-1.8024e-01,0.0000e+00,0.0000e+00,0.0000e+00,3.8268e-01,-9."
                       "2388e-01,2.3549e-01,-9.7545e-02,0.0000e+00,-2.2352e-08,0.0000e+00,1.9509e-01,-9.8079e-01}";
            txt += "\nlocal path=sim.createPath(pathData,";
            int opt = 0;
            if (commandID == ADD_COMMANDS_ADD_PATH_CIRCLE_ACCMD)
                opt += 2;
            txt += std::to_string(opt) + ",100)\nsim.setObjectSelection({path})";
            App::worldContainer->sandboxScript->executeScriptString(txt.c_str(), nullptr);
            if ( (sel != nullptr) && (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1) )
            {
                CSceneObject* path = App::currentWorld->sceneObjects->getLastSelectionObject();
                App::currentWorld->sceneObjects->setObjectParent(path, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::undoRedo_sceneChanged("");
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == ADD_COMMANDS_ADD_GRAPH_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_GRAPH);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CGraph *newObject = new CGraph();
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);

            if (App::userSettings->useSceneObjectScripts)
            {
                CScript *script = new CScript(sim_scripttype_customizationscript, "graph=require('graph_customization')", 0);
                script->setScriptExecPriority(sim_scriptexecorder_last);
                App::currentWorld->sceneObjects->addObjectToScene(script, false, true);
                App::currentWorld->sceneObjects->setObjectParent(script, newObject, true);
                newObject->setObjectProperty(newObject->getObjectProperty() | sim_objectproperty_collapsed);
            }
            else
            {
                CScriptObject *scriptObj = new CScriptObject(sim_scripttype_customizationscript);
                App::currentWorld->sceneObjects->embeddedScriptContainer->insertScript(scriptObj);
                scriptObj->setObjectHandleThatScriptIsAttachedTo(newObject->getObjectHandle());
                scriptObj->setScriptText("graph=require('graph_customization')");
                newObject->setScriptExecPriority(sim_scriptexecorder_last);
            }

            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID == ADD_COMMANDS_ADD_VISION_SENSOR_PERSPECTIVE_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_VISION_SENSOR_ORTHOGONAL_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_VISION_SENSOR);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CVisionSensor *newObject = new CVisionSensor();
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            newObject->setPerspective(commandID == ADD_COMMANDS_ADD_VISION_SENSOR_PERSPECTIVE_ACCMD);
            if (sel != nullptr)
            {
                newObject->setLocalTransformation(C3Vector(0.0, 0.0, newObject->getVisionSensorSize() * 2.0));
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            else
            {
                bool isSet = false;
                if (subView != nullptr)
                {
                    C7Vector m;
                    int lo = subView->getLinkedObjectID();
                    CCamera *camera = App::currentWorld->sceneObjects->getCameraFromHandle(lo);
                    CVisionSensor *sens = App::currentWorld->sceneObjects->getVisionSensorFromHandle(lo);
                    isSet = ((camera != nullptr) || (sens != nullptr));
                    if (isSet)
                    {
                        if (camera != nullptr)
                            m = camera->getLocalTransformation();
                        if (sens != nullptr)
                            m = sens->getLocalTransformation();
                        newObject->setLocalTransformation(m);
                        C3Vector hs(newObject->getBBHSize());
                        double averageSize = (hs(0) + hs(1) + hs(2)) / 1.5;
                        double shiftForward;
                        if (camera != nullptr)
                            shiftForward = camera->getNearClippingPlane() + hs(2) * 2.0 + 3.0 * averageSize;
                        if (sens != nullptr)
                            shiftForward = sens->getNearClippingPlane() + hs(2) * 2.0 + 3.0 * averageSize;
                        m.X += (m.Q.getAxis(2) * shiftForward);
                        newObject->setLocalTransformation(m.X);
                    }
                }
                if (!isSet)
                    newObject->setLocalTransformation(C3Vector(0.0, 0.0, newObject->getVisionSensorSize() * 2.0));
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.objectParams.push_back(subView);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == ADD_COMMANDS_ADD_FORCE_SENSOR_ACCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_FORCE_SENSOR);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CForceSensor *newObject = new CForceSensor();
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID == ADD_COMMANDS_ADD_RAY_PROXSENSOR_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_PYRAMID_PROXSENSOR_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_CYLINDER_PROXSENSOR_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_DISC_PROXSENSOR_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_CONE_PROXSENSOR_ACCMD) ||
        (commandID == ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs, IDSNS_ADDING_A_PROXIMITY_SENSOR);
            CSceneObject* sel = nullptr;
            if (App::currentWorld->sceneObjects->getObjectCountInSelection() == 1)
                sel = App::currentWorld->sceneObjects->getLastSelectionObject();
            CProxSensor *newObject = nullptr;
            if (commandID == ADD_COMMANDS_ADD_RANDOMIZED_RAY_PROXSENSOR_ACCMD)
            {
                newObject = new CProxSensor(sim_proximitysensor_ray_subtype);
                newObject->setRandomizedDetection(true);
            }
            else
                newObject = new CProxSensor(commandID - ADD_COMMANDS_ADD_PYRAMID_PROXSENSOR_ACCMD +
                                            sim_proximitysensor_pyramid_subtype);
            App::currentWorld->sceneObjects->addObjectToScene(newObject, false, true);
            if (sel != nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(newObject, sel, true);
                sel->setObjectProperty((sel->getObjectProperty() | sim_objectproperty_collapsed) - sim_objectproperty_collapsed);
            }
            App::undoRedo_sceneChanged("");
            App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
            App::logMsg(sim_verbosity_msgs, "done.");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    return (false);
}
#endif
