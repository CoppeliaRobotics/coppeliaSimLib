#ifndef QDLGHEIGHTFIELDDIMENSION_H
#define QDLGHEIGHTFIELDDIMENSION_H

#include <vDialog.h>
#include <simLib/simTypes.h>

namespace Ui
{
class CQDlgHeightfieldDimension;
}

class CQDlgHeightfieldDimension : public VDialog
{
    Q_OBJECT

  public:
    explicit CQDlgHeightfieldDimension(QWidget* parent = 0);
    ~CQDlgHeightfieldDimension();

    void cancelEvent() override;
    void okEvent() override;

    void refresh();

    double xSize;
    double ySize;
    double zScaling;
    double xSizeTimesThisGivesYSize;

  private slots:
    void on_qqSizeX_editingFinished();

    void on_qqSizeY_editingFinished();

    void on_qqScalingZ_editingFinished();

    void on_qqOk_accepted();

  private:
    Ui::CQDlgHeightfieldDimension* ui;
};

#endif // QDLGHEIGHTFIELDDIMENSION_H
