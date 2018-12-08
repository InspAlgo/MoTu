#include "pamshow.h"
#include "ui_pamshow.h"

PamShow::PamShow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PamShow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("图片参数"));
    ui->label_gray->setScaledContents(true);  // 设置控件上图片显示大小的属性为自适应控件
}

PamShow::~PamShow()
{
    delete ui;
}

void PamShow::InitGray(int *gray_array, int gray_max)
{
    this->gray_array = gray_array;
    this->gray_max = gray_max;

    int maxcount = this->gray_max;

    int h_width = 400, h_height = 500;

    QImage hist(h_width, h_height,QImage::Format_RGB888);
    hist.fill(qRgb(255,255,255));

    QPainter p(&hist);
    p.setRenderHint(QPainter::Antialiasing, true);  // 设置反走样，避免绘制出来的线条出现锯齿
    p.setBrush(QBrush(QColor(255,255,255)));
    p.drawRect(0,0,h_width,h_height);
    p.translate(0,h_height);

    p.drawLine(0,0,100,100);

    int wid=h_width;
    int hei=h_height;

    p.drawLine(5,-10,wid-10,-10);// 横轴
    p.drawLine(10,-10,10,-hei+10);//纵轴

    float xstep = float(wid-20) / 256;
    float ystep = float(hei-20) / maxcount;

    for (int i = 0; i < 256; i++)
    {

       p.setPen(QColor(13,13,13));
       p.setBrush(QColor(13,13,13));
    //        if(i != 255)
    //            p.drawLine(QPointF(5+(i+0.5)*xstep,-10-ystep*this->gray_array[i]),QPointF(10+(i+1.5)*xstep,-10-ystep*this->gray_array[i+1]));
       p.drawRect(10+i*xstep,-10,xstep,-10-ystep*this->gray_array[i]);

       if(i % 32 == 0||i==255)
       {
           p.drawText(QPointF(5+(i-0.5)*xstep,0),QString::number(i));
       }
    }

    ui->label_gray->setPixmap(QPixmap::fromImage(hist));
}

void PamShow::SetGray(int pixel_sum, int gray_mid, int gray_mean, float gray_sd)
{
    this->pixel_sum = pixel_sum;
    this->gray_mid = gray_mid;
    this->gray_mean = gray_mean;
    this->gray_sd = gray_sd;

    this->ui->label_res_1->setText(QString::number(this->pixel_sum));
    this->ui->label_res_2->setText(QString::number(this->gray_mid));
    this->ui->label_res_3->setText(QString::number(this->gray_mean));
    this->ui->label_res_4->setText(QString::number(this->gray_sd));
    this->ui->label_res_5->setText(QString::number(this->gray_max));
}

void PamShow::SetBG(double da)
{
    ui->label_res_6->setText(QString::number(da));
}

void PamShow::SetDarkChannel(long midel)
{
    ui->label_res_7->setText(QString::number(midel));
}
