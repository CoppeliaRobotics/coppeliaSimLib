
#include "vrepMainHeader.h"
#include "qdlgprimitives.h"
#include "ui_qdlgprimitives.h"
#include "tt.h"
#include "gV.h"
#include "v_repStrings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgPrimitives::CQDlgPrimitives(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgPrimitives)
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

void CQDlgPrimitives::initialize(int type,const C3Vector* sizes)
{
    if (sizes!=nullptr)
    {
        xSize=(*sizes)(0);
        ySize=(*sizes)(1);
        zSize=(*sizes)(2);
    }
    else
    {
        xSize=0.1f;
        ySize=0.1f;
        zSize=0.1f;
    }
    subdivX=0;
    subdivY=0;
    subdivZ=0;
    if (type==2)
        faces=16;
    else
        faces=0;
    sides=32;
    discSubdiv=0;
    smooth=true;
    openEnds=0;
    pure=true;
    dynamic=((type!=0)&&(type!=4)); // (planes and discs are almost never used dynamic!)
    cone=false;
    density=1000.0f;
    negativeVolume=false;
    negativeVolumeScaling=0.5f;
    primitiveType=type;
    sizesAreLocked=(sizes!=nullptr);
    refresh();
}


void CQDlgPrimitives::refresh()
{
    ui->qqDynamic->setChecked(dynamic);
    ui->qqPure->setChecked(pure);
    ui->qqDensity->setEnabled(pure);
    ui->qqNegativeVolume->setEnabled(((primitiveType==1)||(primitiveType==2)||(primitiveType==3)));
    ui->qqNegativeVolume->setChecked(negativeVolume);
    ui->qqNegativeVolumeScaling->setEnabled(((primitiveType==1)||(primitiveType==2)||(primitiveType==3))&&negativeVolume);
    ui->qqDensity->setText(tt::getFString(false,density,1).c_str());

    ui->qqXSize->setText(tt::getEString(false,xSize,4).c_str());
    ui->qqYSize->setText(tt::getEString(false,ySize,4).c_str());

    if (primitiveType==0)
    { // Plane
        setWindowTitle(strTranslate(IDSN_PRIMITIVE_PLANE));

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled(!sizesAreLocked);
        ui->qqZSize->setEnabled(false);

        ui->qqXSubdiv->setEnabled(true);
        ui->qqYSubdiv->setEnabled(true);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaces->setEnabled(false);
        ui->qqSides->setEnabled(false);

        ui->qqZSize->setText("");

        ui->qqFaces->setText("");
        ui->qqSides->setText("");

        ui->qqXSubdiv->setText(tt::getIString(false,subdivX).c_str());
        ui->qqYSubdiv->setText(tt::getIString(false,subdivY).c_str());
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(false);
        ui->qqSmooth->setChecked(false);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");

        ui->qqCone->setEnabled(false);
        ui->qqCone->setChecked(false);

        ui->qqNegativeVolumeScaling->setText("");
    }
    if (primitiveType==1)
    { // Box
        setWindowTitle(strTranslate(IDSN_PRIMITIVE_RECTANGLE));

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled(!sizesAreLocked);
        ui->qqZSize->setEnabled(!sizesAreLocked);

        ui->qqXSubdiv->setEnabled(true);
        ui->qqYSubdiv->setEnabled(true);
        ui->qqZSubdiv->setEnabled(true);

        ui->qqFaces->setEnabled(false);
        ui->qqSides->setEnabled(false);

        ui->qqZSize->setText(tt::getEString(false,zSize,4).c_str());

        ui->qqFaces->setText("");
        ui->qqSides->setText("");

        ui->qqXSubdiv->setText(tt::getIString(false,subdivX).c_str());
        ui->qqYSubdiv->setText(tt::getIString(false,subdivY).c_str());
        ui->qqZSubdiv->setText(tt::getIString(false,subdivZ).c_str());

        ui->qqSmooth->setEnabled(false);
        ui->qqSmooth->setChecked(false);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");

        ui->qqCone->setEnabled(false);
        ui->qqCone->setChecked(false);

        ui->qqNegativeVolumeScaling->setText(tt::getFString(false,negativeVolumeScaling,3).c_str());
    }
    if (primitiveType==2)
    { // Sphere
        setWindowTitle(strTranslate(IDSN_PRIMITIVE_SPHERE));

        ui->qqXSize->setEnabled(!sizesAreLocked);

        // Remove (!pure) in following 2 lines if pure spheroids are allowed;
        ui->qqYSize->setEnabled((!pure)&&(!sizesAreLocked));
        ui->qqZSize->setEnabled((!pure)&&(!sizesAreLocked));

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaces->setEnabled(true);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText(tt::getEString(false,zSize,4).c_str());

        ui->qqFaces->setText(tt::getIString(false,faces).c_str());
        ui->qqSides->setText(tt::getIString(false,sides).c_str());


        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(true);
        ui->qqSmooth->setChecked(smooth);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(false);
        ui->qqDiscSubdiv->setText("");

        ui->qqCone->setEnabled(false);
        ui->qqCone->setChecked(false);

        ui->qqNegativeVolumeScaling->setText(tt::getFString(false,negativeVolumeScaling,3).c_str());
    }
    if (primitiveType==3)
    { // Cylinder
        setWindowTitle(strTranslate(IDSN_PRIMITIVE_CYLINDER));

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled((!pure)&&(!sizesAreLocked));
        ui->qqZSize->setEnabled(!sizesAreLocked);

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaces->setEnabled(true);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText(tt::getEString(false,zSize,4).c_str());

        ui->qqFaces->setText(tt::getIString(false,faces).c_str());
        ui->qqSides->setText(tt::getIString(false,sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(true);
        ui->qqSmooth->setChecked(smooth);

        ui->qqOpen->setEnabled(!pure);
        ui->qqOpen->setChecked(openEnds==3);

        ui->qqDiscSubdiv->setEnabled(true);
        ui->qqDiscSubdiv->setText(tt::getIString(false,discSubdiv).c_str());

        ui->qqCone->setEnabled(true);
        ui->qqCone->setChecked(cone);

        ui->qqNegativeVolumeScaling->setText(tt::getFString(false,negativeVolumeScaling,3).c_str());
    }
    if (primitiveType==4)
    { // Disc
        setWindowTitle(strTranslate(IDSN_PRIMITIVE_DISC));

        ui->qqXSize->setEnabled(!sizesAreLocked);
        ui->qqYSize->setEnabled((!pure)&&(!sizesAreLocked));
        ui->qqZSize->setEnabled(false);

        ui->qqXSubdiv->setEnabled(false);
        ui->qqYSubdiv->setEnabled(false);
        ui->qqZSubdiv->setEnabled(false);

        ui->qqFaces->setEnabled(false);
        ui->qqSides->setEnabled(true);

        ui->qqZSize->setText("");

        ui->qqFaces->setText("");
        ui->qqSides->setText(tt::getIString(false,sides).c_str());

        ui->qqXSubdiv->setText("");
        ui->qqYSubdiv->setText("");
        ui->qqZSubdiv->setText("");

        ui->qqSmooth->setEnabled(false);
        ui->qqSmooth->setChecked(false);

        ui->qqOpen->setEnabled(false);
        ui->qqOpen->setChecked(false);

        ui->qqDiscSubdiv->setEnabled(true);
        ui->qqDiscSubdiv->setText(tt::getIString(false,discSubdiv).c_str());

        ui->qqCone->setEnabled(false);
        ui->qqCone->setChecked(false);

        ui->qqNegativeVolumeScaling->setText("");
    }

}

void CQDlgPrimitives::on_qqDynamic_clicked()
{
    dynamic=!dynamic;
    refresh();
}

void CQDlgPrimitives::on_qqPure_clicked()
{
    pure=!pure;
    _adjustValuesForPurePrimitive();

    if (pure&&negativeVolume)
        App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_SELECTING_NEGATIVE_VOLUME),VMESSAGEBOX_OKELI);
    if (pure&&cone)
        App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_SELECTING_PURE_CONE),VMESSAGEBOX_OKELI);
    if (pure&&((sides<32)||((faces<16)&&(primitiveType==2))))
        App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_PURE_SHAPES_HAVE_LOW_POLYCOUNT),VMESSAGEBOX_OKELI);

    refresh();
}

void CQDlgPrimitives::on_qqXSize_editingFinished()
{
    if (!ui->qqXSize->isModified())
        return;
    bool ok;
    float newVal=ui->qqXSize->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0001f,1000.0f,newVal);
        xSize=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqYSize_editingFinished()
{
    if (!ui->qqYSize->isModified())
        return;
    bool ok;
    float newVal=ui->qqYSize->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0001f,1000.0f,newVal);
        ySize=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqZSize_editingFinished()
{
    if (!ui->qqZSize->isModified())
        return;
    bool ok;
    float newVal=ui->qqZSize->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0001f,1000.0f,newVal);
        zSize=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqXSubdiv_editingFinished()
{
    if (!ui->qqXSubdiv->isModified())
        return;
    bool ok;
    int newVal=ui->qqXSubdiv->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(0,100,newVal);
        subdivX=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqYSubdiv_editingFinished()
{
    if (!ui->qqYSubdiv->isModified())
        return;
    bool ok;
    int newVal=ui->qqYSubdiv->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(0,100,newVal);
        subdivY=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqZSubdiv_editingFinished()
{
    if (!ui->qqZSubdiv->isModified())
        return;
    bool ok;
    int newVal=ui->qqZSubdiv->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(0,100,newVal);
        subdivZ=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqSides_editingFinished()
{
    if (!ui->qqSides->isModified())
        return;
    bool ok;
    int newVal=ui->qqSides->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(3,100,newVal);
        if (pure&&(newVal<32)&&(newVal<sides))
            App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_PURE_SHAPES_HAVE_LOW_POLYCOUNT),VMESSAGEBOX_OKELI);
        sides=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqFaces_editingFinished()
{
    if (!ui->qqFaces->isModified())
        return;
    bool ok;
    int newVal=ui->qqFaces->text().toInt(&ok);
    if (ok)
    {
        if (primitiveType==2) // Sphere
        {
            tt::limitValue(2,50,newVal);
            if (pure&&(newVal<16)&&(newVal<faces))
                App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_PURE_SHAPES_HAVE_LOW_POLYCOUNT),VMESSAGEBOX_OKELI);
        }
        if (primitiveType==3) // Cylinder
            tt::limitValue(0,50,newVal);
        faces=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqDiscSubdiv_editingFinished()
{
    if (!ui->qqDiscSubdiv->isModified())
        return;
    bool ok;
    int newVal=ui->qqDiscSubdiv->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(0,50,newVal);
        discSubdiv=newVal;
        _adjustValuesForPurePrimitive();
    }
    refresh();
}

void CQDlgPrimitives::on_qqSmooth_clicked()
{
    smooth=!smooth;
    _adjustValuesForPurePrimitive();
    refresh();
}

void CQDlgPrimitives::on_qqOpen_clicked()
{
    if (openEnds==0)
        openEnds=3;
    else
        openEnds=0;
    if (openEnds!=0)
        pure=false;
    _adjustValuesForPurePrimitive();
    refresh();
}

void CQDlgPrimitives::on_qqCone_clicked()
{
    cone=!cone;
    if (cone)
        negativeVolume=false;
    _adjustValuesForPurePrimitive();
    if (pure&&cone)
        App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_SELECTING_PURE_CONE),VMESSAGEBOX_OKELI);
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

void CQDlgPrimitives::_adjustValuesForPurePrimitive()
{
    if (pure)
    {
        if (primitiveType==2)
        { // sphere
            // Comment out following if pure spheroids are allowed;
            ySize=xSize;
            zSize=xSize;
        }
        if (primitiveType==3)
        { // cylinder
            ySize=xSize;
            openEnds=0;
        }
        if (primitiveType==4)
        { // disc
            ySize=xSize;
        }
    }
}

void CQDlgPrimitives::on_qqDensity_editingFinished()
{
    if (!ui->qqDensity->isModified())
        return;
    bool ok;
    float newVal=ui->qqDensity->text().toFloat(&ok);
    if (ok)
    {
        density=newVal;
        tt::limitValue(0.1f,100000.0f,density);
    }
    refresh();
}

void CQDlgPrimitives::on_qqNegativeVolume_clicked()
{
    negativeVolume=!negativeVolume;
    if (negativeVolume)
        cone=false;
    if (pure&&negativeVolume)
        App::uiThread->messageBox_warning(this,strTranslate("Primitives"),strTranslate(IDS_WARNING_WHEN_SELECTING_NEGATIVE_VOLUME),VMESSAGEBOX_OKELI);
    refresh();
}

void CQDlgPrimitives::on_qqNegativeVolumeScaling_editingFinished()
{
    if (!ui->qqNegativeVolumeScaling->isModified())
        return;
    bool ok;
    float newVal=ui->qqNegativeVolumeScaling->text().toFloat(&ok);
    if (ok)
    {
        negativeVolumeScaling=newVal;
        tt::limitValue(0.01f,0.99f,negativeVolumeScaling);
    }
    refresh();
}

