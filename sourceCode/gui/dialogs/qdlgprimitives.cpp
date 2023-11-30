#include <qdlgprimitives.h>
#include <ui_qdlgprimitives.h>
#include <tt.h>
#include <utils.h>
#include <simStrings.h>
#include <app.h>
#include <vMessageBox.h>
#include <guiApp.h>

CQDlgPrimitives::CQDlgPrimitives(QWidget *parent) : VDialog(parent, QT_MODAL_DLG_STYLE), ui(new Ui::CQDlgPrimitives)
{
    ui->setupUi(this);
}

CQDlgPrimitives::~CQDlgPrimitives()
{
    delete ui;
}

void CQDlgPrimitives::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgPrimitives::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgPrimitives::initialize(int type, const C3Vector *sizes)
{
    if (sizes != nullptr)
    {
        xSize = (*sizes)(0);
        ySize = (*sizes)(1);
        zSize = (*sizes)(2);
    }
    else
    {
        xSize = 0.1;
        ySize = 0.1;
        if (type == sim_primitiveshape_capsule)
            zSize = 0.2;
        else
            zSize = 0.1;
    }
    subdivX = 0;
    subdivY = 0;
    subdivZ = 0;
    if (type == sim_primitiveshape_spheroid)
        faceSubdiv = 16;
    else
        faceSubdiv = 0;
    sides = 32;
    discSubdiv = 0;
    smooth = true;
    openEnds = false;
    dynamic =
        ((type != sim_primitiveshape_plane) && (type != sim_primitiveshape_disc) && (type != sim_primitiveshape_cone));
    density = 1000.0;
    primitiveType = type;
    sizesAreLocked = (sizes != nullptr);
    _correctDependentValues();
    refresh();
}

void CQDlgPrimitives::refresh()
{
    ui->qqDynamic->setChecked(dynamic);
    ui->qqDensity->setEnabled(dynamic);
    ui->qqDensity->setText(utils::getDensityString(density).c_str());

    ui->qqXSize->setText(utils::getSizeString(false, xSize).c_str());
    ui->qqYSize->setText(utils::getSizeString(false, ySize).c_str());

    if (primitiveType == sim_primitiveshape_plane)
    {
        setWindowTitle(IDSN_PRIMITIVE_PLANE);

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled(!sizesAreLocked);
        ui->qqZSize->setEnabled(false);

        ui->qqXSubdiv->setEnabled(true);
        ui->qqYSubdiv->setEnabled(true);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaceSubdiv->setEnabled(false);
        ui->qqSides->setEnabled(false);

        ui->qqZSize->setText("");

        ui->qqFaceSubdiv->setText("");
        ui->qqSides->setText("");

        ui->qqXSubdiv->setText(utils::getIntString(false, subdivX).c_str());
        ui->qqYSubdiv->setText(utils::getIntString(false, subdivY).c_str());
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(false);
        ui->qqSmooth->setChecked(false);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");
    }
    if (primitiveType == sim_primitiveshape_cuboid)
    {
        setWindowTitle(IDSN_PRIMITIVE_RECTANGLE);

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled(!sizesAreLocked);
        ui->qqZSize->setEnabled(!sizesAreLocked);

        ui->qqXSubdiv->setEnabled(true);
        ui->qqYSubdiv->setEnabled(true);
        ui->qqZSubdiv->setEnabled(true);

        ui->qqFaceSubdiv->setEnabled(false);
        ui->qqSides->setEnabled(false);

        ui->qqZSize->setText(utils::getSizeString(false, zSize).c_str());

        ui->qqFaceSubdiv->setText("");
        ui->qqSides->setText("");

        ui->qqXSubdiv->setText(utils::getIntString(false, subdivX).c_str());
        ui->qqYSubdiv->setText(utils::getIntString(false, subdivY).c_str());
        ui->qqZSubdiv->setText(utils::getIntString(false, subdivZ).c_str());

        ui->qqSmooth->setEnabled(false);
        ui->qqSmooth->setChecked(false);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");
    }
    if (primitiveType == sim_primitiveshape_spheroid)
    {
        setWindowTitle(IDSN_PRIMITIVE_SPHERE);

        ui->qqXSize->setEnabled(!sizesAreLocked);

        ui->qqYSize->setEnabled((!dynamic) && (!sizesAreLocked));
        ui->qqZSize->setEnabled((!dynamic) && (!sizesAreLocked));

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaceSubdiv->setEnabled(false);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText(utils::getSizeString(false, zSize).c_str());

        ui->qqFaceSubdiv->setText("");
        ui->qqSides->setText(utils::getIntString(false, sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(true);
        ui->qqSmooth->setChecked(smooth);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");
    }
    if (primitiveType == sim_primitiveshape_cylinder)
    {
        setWindowTitle(IDSN_PRIMITIVE_CYLINDER);

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled((!dynamic) && (!sizesAreLocked));
        ui->qqZSize->setEnabled(!sizesAreLocked);

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaceSubdiv->setEnabled(true);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText(utils::getSizeString(false, zSize).c_str());

        ui->qqFaceSubdiv->setText(utils::getIntString(false, faceSubdiv).c_str());
        ui->qqSides->setText(utils::getIntString(false, sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(true);
        ui->qqSmooth->setChecked(smooth);

        ui->qqOpen->setEnabled(!dynamic);
        ui->qqOpen->setChecked(openEnds);

        ui->qqDiscSubdiv->setEnabled(true);
        ui->qqDiscSubdiv->setText(utils::getIntString(false, discSubdiv).c_str());
    }
    if (primitiveType == sim_primitiveshape_cone)
    {
        setWindowTitle(IDSN_PRIMITIVE_CONE);

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled((!dynamic) && (!sizesAreLocked));
        ui->qqZSize->setEnabled(!sizesAreLocked);

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaceSubdiv->setEnabled(true);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText(utils::getSizeString(false, zSize).c_str());

        ui->qqFaceSubdiv->setText(utils::getIntString(false, faceSubdiv).c_str());
        ui->qqSides->setText(utils::getIntString(false, sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(true);
        ui->qqSmooth->setChecked(smooth);

        ui->qqOpen->setEnabled(!dynamic);
        ui->qqOpen->setChecked(openEnds);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");
    }
    if (primitiveType == sim_primitiveshape_disc)
    {
        setWindowTitle(IDSN_PRIMITIVE_DISC);

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled((!dynamic) && (!sizesAreLocked));
        ui->qqZSize->setEnabled(false);

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaceSubdiv->setEnabled(false);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText("");

        ui->qqFaceSubdiv->setText("");
        ui->qqSides->setText(utils::getIntString(false, sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(false);
        ui->qqSmooth->setChecked(false);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(true);
        ui->qqDiscSubdiv->setText(utils::getIntString(false, discSubdiv).c_str());
    }
    if (primitiveType == sim_primitiveshape_capsule)
    {
        setWindowTitle(IDSN_PRIMITIVE_CAPSULE);

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled((!dynamic) && (!sizesAreLocked));
        ui->qqZSize->setEnabled(!sizesAreLocked);

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaceSubdiv->setEnabled(true);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText(utils::getSizeString(false, zSize).c_str());

        ui->qqFaceSubdiv->setText(utils::getIntString(false, faceSubdiv).c_str());
        ui->qqSides->setText(utils::getIntString(false, sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(true);
        ui->qqSmooth->setChecked(smooth);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");
    }
}

void CQDlgPrimitives::on_qqDynamic_clicked()
{
    dynamic = !dynamic;
    _correctDependentValues();
    if (dynamic)
    {
        openEnds = false;
        if (sides < 32)
            GuiApp::uiThread->messageBox_warning(this, "Primitives", IDS_WARNING_WHEN_PURE_SHAPES_HAVE_LOW_POLYCOUNT,
                                                 VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
    }

    refresh();
}

void CQDlgPrimitives::on_qqXSize_editingFinished()
{
    if (!ui->qqXSize->isModified())
        return;
    bool ok;
    double newVal = GuiApp::getEvalDouble(ui->qqXSize->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0.0001, 1000.0, newVal);
        xSize = newVal;
        if (primitiveType == sim_primitiveshape_capsule)
        {
            if (xSize > zSize - 0.0001)
                xSize = zSize - 0.0001;
        }
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqYSize_editingFinished()
{
    if (!ui->qqYSize->isModified())
        return;
    bool ok;
    double newVal = GuiApp::getEvalDouble(ui->qqYSize->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0.0001, 1000.0, newVal);
        ySize = newVal;
        if (primitiveType == sim_primitiveshape_capsule)
        {
            if (ySize > zSize - 0.0001)
                ySize = zSize - 0.0001;
        }
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqZSize_editingFinished()
{
    if (!ui->qqZSize->isModified())
        return;
    bool ok;
    double newVal = GuiApp::getEvalDouble(ui->qqZSize->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0.0001, 1000.0, newVal);
        zSize = newVal;
        if (primitiveType == sim_primitiveshape_capsule)
        {
            double mmax = std::max<double>(xSize, ySize);
            if (zSize < mmax + 0.0001)
                zSize = mmax + 0.0001;
        }
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqXSubdiv_editingFinished()
{
    if (!ui->qqXSubdiv->isModified())
        return;
    bool ok;
    int newVal = (int)GuiApp::getEvalInt(ui->qqXSubdiv->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0, 100, newVal);
        subdivX = newVal;
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqYSubdiv_editingFinished()
{
    if (!ui->qqYSubdiv->isModified())
        return;
    bool ok;
    int newVal = (int)GuiApp::getEvalInt(ui->qqYSubdiv->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0, 100, newVal);
        subdivY = newVal;
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqZSubdiv_editingFinished()
{
    if (!ui->qqZSubdiv->isModified())
        return;
    bool ok;
    int newVal = (int)GuiApp::getEvalInt(ui->qqZSubdiv->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0, 100, newVal);
        subdivZ = newVal;
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqSides_editingFinished()
{
    if (!ui->qqSides->isModified())
        return;
    bool ok;
    int newVal = (int)GuiApp::getEvalInt(ui->qqSides->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(3, 100, newVal);
        if (dynamic && (newVal < 32) && (newVal < sides))
            GuiApp::uiThread->messageBox_warning(this, "Primitives", IDS_WARNING_WHEN_PURE_SHAPES_HAVE_LOW_POLYCOUNT,
                                                 VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
        sides = newVal;
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqFaceSubdiv_editingFinished()
{
    if (!ui->qqFaceSubdiv->isModified())
        return;
    bool ok;
    int newVal = (int)GuiApp::getEvalInt(ui->qqFaceSubdiv->text().toStdString().c_str(), &ok);
    if (ok)
    {
        if (primitiveType == sim_primitiveshape_spheroid)
            tt::limitValue(2, 50, newVal);
        if (primitiveType == sim_primitiveshape_cylinder)
            tt::limitValue(0, 50, newVal);
        faceSubdiv = newVal;
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqDiscSubdiv_editingFinished()
{
    if (!ui->qqDiscSubdiv->isModified())
        return;
    bool ok;
    int newVal = (int)GuiApp::getEvalInt(ui->qqDiscSubdiv->text().toStdString().c_str(), &ok);
    if (ok)
    {
        tt::limitValue(0, 50, newVal);
        discSubdiv = newVal;
        _correctDependentValues();
    }
    refresh();
}

void CQDlgPrimitives::on_qqSmooth_clicked()
{
    smooth = !smooth;
    _correctDependentValues();
    refresh();
}

void CQDlgPrimitives::on_qqOpen_clicked()
{
    openEnds = !openEnds;
    if (openEnds)
        dynamic = false;
    _correctDependentValues();
    refresh();
}

void CQDlgPrimitives::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgPrimitives::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgPrimitives::_correctDependentValues()
{
    if (dynamic)
    {
        if (primitiveType == sim_primitiveshape_spheroid)
        {
            ySize = xSize;
            zSize = xSize;
        }
        if (primitiveType == sim_primitiveshape_cylinder)
        {
            ySize = xSize;
            openEnds = false;
        }
        if (primitiveType == sim_primitiveshape_capsule)
            ySize = xSize;
        if (primitiveType == sim_primitiveshape_cone)
            ySize = xSize;
        if (primitiveType == sim_primitiveshape_disc)
            ySize = xSize;
    }
    if (primitiveType == sim_primitiveshape_capsule)
    {
        double mmax = std::max<double>(xSize, ySize);
        if (zSize < mmax + 0.0001)
            zSize = mmax + 0.0001;
    }
}

void CQDlgPrimitives::on_qqDensity_editingFinished()
{
    if (!ui->qqDensity->isModified())
        return;
    bool ok;
    double newVal = GuiApp::getEvalDouble(ui->qqDensity->text().toStdString().c_str(), &ok);
    if (ok)
    {
        density = newVal;
        tt::limitValue(0.1, 100000.0, density);
    }
    refresh();
}
