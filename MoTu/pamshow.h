#ifndef PAMSHOW_H
#define PAMSHOW_H

#include <QDialog>
#include <QPainter>

namespace Ui {
class PamShow;
}

class PamShow : public QDialog
{
    Q_OBJECT

public:
    explicit PamShow(QWidget *parent = 0);
    ~PamShow();

    void InitGray(int *gray_array, int gray_max);
    void SetGray(int pixel_sum, int gray_mid, int gray_mean, float gray_sd);
    void SetBG(double da);
    void SetDarkChannel(long midel);

private:
    Ui::PamShow *ui;

    int *gray_array;  // 灰度数组
    int gray_max;  // 灰度数组中数目最多的那个灰度值的个数
    int pixel_sum;
    int gray_mid;
    int gray_mean;
    float gray_sd;
};

#endif // PAMSHOW_H
