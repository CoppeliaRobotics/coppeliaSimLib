
#include <qdlgmodelthumbnailvisu.h>
#include <ui_qdlgmodelthumbnailvisu.h>
#include <thumbnail.h>
#include <app.h>

CQDlgModelThumbnailVisu::CQDlgModelThumbnailVisu(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgModelThumbnailVisu)
{
    ui->setupUi(this);
}

CQDlgModelThumbnailVisu::~CQDlgModelThumbnailVisu()
{
    delete ui;
}

void CQDlgModelThumbnailVisu::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgModelThumbnailVisu::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgModelThumbnailVisu::applyThumbnail(const CThumbnail* thumbnail)
{
    const char* rgba=((CThumbnail*)thumbnail)->getPointerToUncompressedImage();
    QImage img(128,128,QImage::Format_RGB32);
    QRgb value;
    for (int i=0;i<128;i++)
    {
        for (int j=0;j<128;j++)
        {
            char r=rgba[(i*128+j)*4+0];
            char g=rgba[(i*128+j)*4+1];
            char b=rgba[(i*128+j)*4+2];

            if (rgba[(i*128+j)*4+3]==0)
            {
                r=(char)255;
                g=(char)255;
                b=(char)255;
            }
            value=qRgb(r,g,b);
            img.setPixel(j,127-i,value);
        }
    }
    QPixmap pimg;
    pimg.convertFromImage(img);
    ui->qqImage->setPixmap(pimg);
}

void CQDlgModelThumbnailVisu::on_qqYesNo_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgModelThumbnailVisu::on_qqYesNo_rejected()
{
    defaultModalDialogEndRoutine(false);
}
