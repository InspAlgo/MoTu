#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Dark_Channel.h"
#include "deblurring.h"
#include "meiyancpp.h"
#include "ACE_CPP.h"
#include  <math.h>
#include "removal_vignetting.cpp"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("魔图秀秀"));

    this->InitPamater();
    this->InitMenu();

//    this->showMaximized();  // 全屏

    PAstocket::PyInit();
}

MainWindow::~MainWindow()
{
    PAstocket::PyFinit();
    delete ui;
}

// QImage 转成 cv::Mat
cv::Mat QImage2cvMat(const QImage &image)
{
    cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8UC4, (uchar*)image.bits(), image.bytesPerLine());
    cv::Mat mat2 = cv::Mat(mat.rows, mat.cols, CV_8UC3);
    int from_to[] = { 0,0, 1,1, 2,2 };
    cv::mixChannels(&mat, 1, &mat2, 1, from_to, 3);
    return mat2;
}

// 彩色图像的直方图均衡化
cv::Mat EqualizeHistColorImage(IplImage *pImage)
{
    int depth = pImage->depth;
    CvSize cv_size = cvGetSize(pImage);
    IplImage *pEquaImage;
    pEquaImage = cvCreateImage(cv_size, depth, 3);

    // 原图像分成各通道后再均衡化,最后合并即彩色图像的直方图均衡化
    const int MAX_CHANNEL = 4;
    IplImage *pImageChannel[MAX_CHANNEL] = { NULL };

    int i;
    for (i = 0; i < pImage->nChannels; i++)
        pImageChannel[i] = cvCreateImage(cvGetSize(pImage), pImage->depth, 1);

    cvSplit(pImage, pImageChannel[0], pImageChannel[1], pImageChannel[2], pImageChannel[3]);

    for (i = 0; i < pImage->nChannels; i++)
        cvEqualizeHist(pImageChannel[i], pImageChannel[i]);

    cvMerge(pImageChannel[0], pImageChannel[1], pImageChannel[2], pImageChannel[3], pEquaImage);

    for (i = 0; i < pImage->nChannels; i++)
        cvReleaseImage(&pImageChannel[i]);

    cv::Mat ret = cv::cvarrToMat(pEquaImage);
    return ret;
}

// 伽马变换
QImage gammaTrans(QImage image, double c, double r) 
{
    int color1, color2, color3, max = 255;
    max = pow(max, r) * c;

    for (int i = 0; i < image.width(); i++) 
    {
        for (int j = 0; j < image.height(); j++) 
        {
            color1 = QColor(image.pixel(i, j)).red();
            color1 = 255.0 / max * pow(color1, r) *c;
            color2 = QColor(image.pixel(i, j)).green();
            color2 = 255.0 / max * pow(color2, r) *c;
            color3 = QColor(image.pixel(i, j)).blue();
            color3 = 255.0 / max * pow(color3, r) *c;

            if (color1 > 255)
                color1 = 255;
            else if (color1 < 0)
                color1 = 0;
            if (color2 > 255)
                color2 = 255;
            else if (color2 < 0)
                color2 = 0;
            if (color3 > 255)
                color3 = 255;
            else if (color3 < 0)
                color3 = 0;

            image.setPixel(i, j, qRgb(color1, color2, color3));
        }
    }

    return QImage(image);
}

// 霓虹滤镜
void niHongFilter(Mat &srcImage)
{
    int rowNum = srcImage.rows;
    int colNum = srcImage.cols;

    for (int j = 0; j < rowNum - 1; j++) 
    {
        uchar* data = srcImage.ptr<uchar>(j);
        for (int i = 0; i < colNum - 1; i++) 
        {

            //当前像素的RGB分量
            int b1 = data[i * 3];
            int g1 = data[i * 3 + 1];
            int r1 = data[i * 3 + 2];

            //同行下一个像素的RGB分量
            int b2 = data[(i + 1) * 3];
            int g2 = data[(i + 1) * 3 + 1];
            int r2 = data[(i + 1) * 3 + 2];

            //指针移到下一行
            data = srcImage.ptr<uchar>(j + 1);

            //同列正下方的像素的RGB分量
            int b3 = data[i * 3];
            int g3 = data[i * 3 + 1];
            int r3 = data[i * 3 + 2];

            //指针移回来
            data = srcImage.ptr<uchar>(j);

            //计算新的RGB分量的值
            int R = 10 * sqrt((r1 - r2)*(r1 - r2) + (r1 - r3)*(r1 - r3));
            int G = 10 * sqrt((g1 - g2)*(g1 - g2) + (g1 - g3)*(g1 - g3));
            int B = 10 * sqrt((b1 - b2)*(b1 - b2) + (b1 - b3)*(b1 - b3));

            data[i * 3] = max(0, min(B, 255));;
            data[i * 3 + 1] = max(0, min(G, 255));;
            data[i * 3 + 2] = max(0, min(R, 255));;
        }
    }
}

/*
 * 事件重写
*/

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 当窗口大小变化时，图片控件也需要重新调整大小
    if(this->status_type != 0)
        this->AutoLabelSize();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->status_type == 1)
    {
        QMessageBox msg;
        msg.setWindowTitle(tr("警告"));
        msg.setText(tr("未保存图片"));
        msg.addButton(tr("取消"), QMessageBox::ActionRole);
        msg.addButton(tr("不保存"), QMessageBox::ActionRole);
        int ret = msg.exec();

        if (ret == 0)
            event->ignore();  // 需要引入头文件
        else
            event->accept();
        return;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Control)
    {
        if(e->isAutoRepeat())  // 按钮重复时不做处理
            return;
        this->ctrl_is_pressed = true;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Control)
    {
        if(e->isAutoRepeat())
            return;
        this->ctrl_is_pressed = false;
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);

    if(this->status_type != 0 && // 界面上必须要有图片且鼠标放在控件区
            ui->scrollArea->geometry().contains(this->mapFromGlobal(QCursor::pos())))
    {
        double width_height = (double)this->image->width() / (double)this->image->height();
        int label_width = ui->label_image->width();
        int label_height = ui->label_image->height();

        if(event->delta() > 0)
        {
            label_height *= 1.05;
            label_width = width_height * label_height;
        }
        else
        {
            label_height *= 0.95;
            label_width = width_height * label_height;
        }

        // 限制最小尺寸
        if(label_width > 100 && label_width > 100)
        {
            /* 鼠标相对于 ui->scrollArea 的位置，这样鼠标和 ui->label_image 的坐标系一致 */
            //int cur_x = event->pos().x() - ui->scrollArea->x();
            //int cur_y = event->pos().y() - ui->scrollArea->y() - ui->menuBar->height();

            // 原地中心缩放
            int diff_x = ui->label_image->x() - (label_width - ui->label_image->width()) / 2;
            int diff_y = ui->label_image->y() - (label_height - ui->label_image->height()) / 2;

            ui->label_image->resize(label_width, label_height);
            ui->label_image->move(diff_x, diff_y);
        } 
    }
}

bool MainWindow::eventFilter(QObject *, QEvent *evt)
{
    static QPoint lastPnt;
    static bool isHover = false;

    // 鼠标按下
    if(evt->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if(ui->label_image->rect().contains(e->pos()) && //is the mouse is clicking the key
            (e->button() == Qt::LeftButton)) //if the mouse click the right key
        {
            lastPnt = e->pos();
            isHover = true;
        }
    }

    // 鼠标移动
    else if(evt->type() == QEvent::MouseMove && isHover)
    {
        // 鼠标位置
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        int dx = e->pos().x() - lastPnt.x();
        int dy = e->pos().y() - lastPnt.y();

        // 修改对象位置
        ui->label_image->move(ui->label_image->x() + dx, ui->label_image->y() + dy);
    }
    else if(evt->type() == QEvent::MouseButtonRelease && isHover)
    {
        isHover = false;
    }

    return false;
}

/*
 * 初始化
*/

void MainWindow::InitPamater()
{
    this->status_type = 0;
    this->image = nullptr;
    this->ctrl_is_pressed = false;

    ui->label_image->setScaledContents(true);  // 设置控件上图片显示大小的属性为自适应控件
    ui->label_image->resize(0, 0);
    ui->label_image->installEventFilter(this);  // 事件捕捉，鼠标可以拖动图片控件

    this->is_face = false;
    this->offset_bg = 0.0;
    this->dark_channels = 0;

    for(int i = 0; i < 10; i++)
        this->pB_result_type[i] = 0;

    ui->pB_result_1->setText(tr(""));
    ui->pB_result_2->setText(tr(""));
    ui->pB_result_3->setText(tr(""));
    ui->pB_result_4->setText(tr(""));
    ui->pB_result_5->setText(tr(""));
    ui->pB_result_6->setText(tr(""));
    ui->pB_result_7->setText(tr(""));
    ui->pB_result_8->setText(tr(""));
}

void MainWindow::InitMenu()
{
    this->menu_file = ui->menuBar->addMenu(tr("文件"));
    this->file_open = this->menu_file->addAction(tr("打开"));
    connect(this->file_open, SIGNAL(triggered(bool)), this, SLOT(FileOpen()));  // Action 应使用 triggered() 信号
    this->file_save = this->menu_file->addAction(tr("保存"));
    connect(this->file_save, SIGNAL(triggered(bool)), this, SLOT(FileSave()));
    this->file_save_as = this->menu_file->addAction(tr("另存为"));
    connect(this->file_save_as, SIGNAL(triggered(bool)), this, SLOT(FileSaveAs()));
    this->file_close = this->menu_file->addAction(tr("关闭"));
    connect(this->file_close, SIGNAL(triggered(bool)), this, SLOT(FileClose()));
}

void MainWindow::AutoLabelSize()
{
    double width_height = (double)this->image->width() / (double)this->image->height();

    int label_width, label_height;
    if(this->image->width() < ui->scrollArea->width() -20)
        label_width = this->image->width();
    else
        label_width = 0;
    if(this->image->height() < ui->scrollArea->height() -20)
        label_height = this->image->height();
    else
        label_height = 0;

    if(label_width == 0 || label_height == 0)
    {
        label_height = ui->scrollArea->height() -20;
        label_width = int(label_height * width_height);

        for(;label_height >= ui->scrollArea->height() - 20 || label_width >= ui->scrollArea->width() -20;)
        {
            label_height--;
            label_width = int(label_height * width_height);
        }
    }

    ui->label_image->resize(label_width, label_height);

    int move_width = (ui->scrollArea->width() - label_width) / 2;
    int move_height = (ui->scrollArea->height() - label_height) / 2;
    ui->label_image->move(move_width, move_height);
}

/*
 * 菜单栏功能
*/

void MainWindow::FileOpen()
{
    if(this->status_type == 1)
    {
        QMessageBox::about(NULL, tr("警告"), tr("已加载图片！"));
        return;
    }

    this->InitPamater();

    delete this->image;
    this->image = nullptr;

    // 选择图片
    QString select_image = QFileDialog::getOpenFileName(
           this, tr("Open File"), "",
           tr("File (*.bmp *jpg *png)")
       );

    // 如果没有选择则直接返回
    if(select_image.isEmpty() || select_image.isNull())
    {
        QMessageBox::about(NULL, tr("警告"), tr("未选择图片！"));
        return;
    }

    QFileInfo file_info(select_image);
    this->path = file_info.path();
    this->image_name = file_info.fileName();
    this->file_type = file_info.suffix();

    // 加载图片，并在控件上显示图片
    this->image = new QImage(select_image);
    ui->label_image->setPixmap(QPixmap::fromImage(*this->image));

    // 调整控件大小
    this->AutoLabelSize();

    this->status_type = 2;
}

void MainWindow::FileSave()
{
    if(this->status_type == 0)
    {
        QMessageBox::about(NULL, tr("警告"), tr("未加载图片！"));
        return;
    }

    QString save_path = this->path + "/" + this->image_name + this->file_type;

    ui->label_image->pixmap()->toImage().save(save_path);
    this->status_type = 2;
}

void MainWindow::FileSaveAs()
{
    if(this->status_type == 0)
    {
        QMessageBox::about(NULL, tr("警告"), tr("未加载图片！"));
        return;
    }

    QString save_path;
    save_path = QFileDialog::getSaveFileName(this,
        tr("Save Image"), "", tr("Image Files (*.bmp *.jpg *.png);;BMP(*.bmp);;JPG(*.jpg *jpeg);;PNG(*.png)"));

    if (save_path.isNull() || save_path.isEmpty())
    {
        QMessageBox::about(NULL, tr("警告"), tr("未保存图片！"));
        return;
    }

    QFileInfo file_info(save_path);
    this->path = file_info.path();
    this->image_name = file_info.fileName();
    this->file_type = file_info.suffix();

    ui->label_image->pixmap()->toImage().save(save_path);

    this->status_type = 2;
}

void MainWindow::FileClose()
{
    if(this->status_type == 0)
    {
        QMessageBox::about(NULL, tr("警告"), tr("未加载图片！"));
        return;
    }
    else if(this->status_type == 1)
    {
        QMessageBox msg;
        msg.setWindowTitle(tr("警告"));
        msg.setText(tr("未保存图片"));
        msg.addButton(tr("取消"), QMessageBox::ActionRole);
        msg.addButton(tr("不保存"), QMessageBox::ActionRole);
        int ret = msg.exec();

        if (ret == 0)
            return;
    }

    this->InitPamater();

    this->status_type = 0;

    delete this->image;
    this->image = nullptr;

    ui->label_image->clear();
    ui->label_image->resize(0, 0);
}

/*
 * 一键自动化所有
*/

// 检测所有
void MainWindow::on_pB_check_clicked()
{
    this->on_pB_check_5_clicked();
    this->on_pB_check_6_clicked();

    if (!this->is_face)
    {
        this->on_pB_check_2_clicked();
        this->on_pB_check_4_clicked();
    }

    this->on_pB_check_1_clicked();
    this->on_pB_check_3_clicked();
}

// 美化所有
void MainWindow::on_pB_tuning_clicked()
{
    this->on_pB_result_2_clicked();
    this->on_pB_result_4_clicked();

    this->on_pB_result_5_clicked();

    this->on_pB_result_6_clicked();
    this->on_pB_result_3_clicked();

    this->on_pB_check_1_clicked();
    this->on_pB_result_1_clicked();
}

// 曝光检测——检测
void MainWindow::on_pB_check_1_clicked()
{
    if(this->status_type == 0)
        return;

    int width = ui->label_image->width();
    int height = ui->label_image->height();
    int temp;
    double da = 0; // 偏离128的均值
    double daAbsolute;  // da的绝对值
    double ma;  // 偏离128的平均偏差
    double maAbsolute;  // ma的绝对值
    double k;  // 亮度系数
    int hist[256];
    memset(hist, 0, sizeof(hist));

    QImage temp_img = ui->label_image->pixmap()->toImage();

    for(int i = 0; i < width; ++i)
    {
        for(int j = 0; j < height; ++j)
        {
            temp = qGray(temp_img.pixel(i,j));
            hist[temp]++;  // 计算灰度直方图的灰度数
            da+=(temp-128);
        }
    }
    da /= width*height;
    daAbsolute = fabs(da);

    for(int i=0; i<255; ++i)
    {
        ma+=fabs((i-128)-da)*hist[i];
    }
    ma /= width*height;
    maAbsolute = fabs(ma);

    k = daAbsolute/maAbsolute;

    if(k>=1)
    {
        if(da>=0)
        {
            ui->pB_result_1->setText(tr("过亮"));
            this->pB_result_type[1] = 1;
            this->offset_bg = da;
        }

        else
        {
            ui->pB_result_1->setText(tr("过暗"));
            this->pB_result_type[1] = -1;
            this->offset_bg = da;
        }
    }
    else
    {
        ui->pB_result_1->setText(tr("亮度正常"));
        this->pB_result_type[1] = 2;
        this->offset_bg = da;
    }

}

// 曝光程度
void  MainWindow::OffsetBG(double &c, double &r)
{
    if (this->offset_bg >= 0)
    {
        if (this->offset_bg <= 50)
        {
            c = 10; r = 2.9;
        }
        else if (this->offset_bg <= 100)
        {
            c = 10; r = 3.01;
        }
        else
        {
            c = 10; r = 3.05;
        }
    }
    else
    {
        if (this->offset_bg >= -15)
        {
            c = 0.99; r = 0.9;
        }
        else if (this->offset_bg >= -50)
        {
            c = 1; r = 0.9;
        }
        else if (this->offset_bg >= -80)
        {
            c = 0.99; r = 0.8;
        }
        else
        {
            c = 0.98; r = 0.6;
        }

    }
}

// 曝光检测——结果
void MainWindow::on_pB_result_1_clicked()
{
    if(this->pB_result_type[1] == 0 || this->pB_result_type[1] == 2)  // 如果检测结果不是 X 则需要美化
        return;

    QImage image_old = ui->label_image->pixmap()->toImage();
    QImage ret;
    double c, r;
    
    switch (this->pB_result_type[1])
    {    
        case 1: 
            OffsetBG(c, r);
            ret = gammaTrans(image_old, c, r);
            break;
        case -1:
            OffsetBG(c, r);
            ret = gammaTrans(image_old, c, r);
            break;
        default:
            break;
    }
    
    if (this->pB_result_type[1] == 1 || this->pB_result_type[1] == -1)
    {
        ui->label_image->setPixmap(QPixmap::fromImage(ret));

        this->status_type = 1;

        ui->pB_result_1->setText(tr("亮度正常"));
        this->pB_result_type[1] = 2;  // 完成美化
    }
}

// 去雾——检测
void MainWindow::on_pB_check_2_clicked()
{
    if(this->status_type == 0)
        return;

    QImage image = ui->label_image->pixmap()->toImage();
    Dark_Channel darkchannel(image);
    bool check = darkchannel.Detection();
    this->dark_channels = darkchannel.midel;
    if(check)
    {
        ui->pB_result_2->setText(tr("NO"));
        this->pB_result_type[2] = 1;
    }
    else
    {
        ui->pB_result_2->setText(tr("YES"));
        this->pB_result_type[2] = 2;
    }
    //image=darkchannel.HazeRemoval().copy();
}

// 去雾——结果
void MainWindow::on_pB_result_2_clicked()
{
    if(this->pB_result_type[2] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    QImage image = ui->label_image->pixmap()->toImage();
    Dark_Channel darkchannel(image);

    bool check = darkchannel.Detection();

    if(check)
    {
        this->status_type = 1;
        image=darkchannel.HazeRemoval().copy();

        ui->label_image->setPixmap(QPixmap::fromImage(image));

        ui->pB_result_2->setText(tr("YES"));
        this->pB_result_type[2] = 2;  // 完成美化
    }
}

// 去暗角——检测
void MainWindow::on_pB_check_3_clicked()
{
    if(this->status_type == 0)
        return;

     QImage image = ui->label_image->pixmap()->toImage();

    int width = image.width(), height = image.height();
    int sum1 = 0, sum2 = 0;
    int temp;
    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            if((i > width/8 && i < (width * 7)/8)
                    || (j > height /8 && j < (height * 7) /8))
                continue;

            temp = qGray(image.pixel(i,j));
            sum1 += temp;
        }
    }

    for(int i = width/4; i < width * 3/4; i++)
    {
        for(int j = height/4; j < height *3/4; j++)
        {
            temp = qGray(image.pixel(i,j));
            sum2 += temp;
        }
    }

    sum1 /= (width * height /16);
    sum2 /= (width * height /4);

    if(sum2 - sum1 > 50)
    {
        ui->pB_result_3->setText(tr("NO"));
        this->pB_result_type[3] = 1;
    }
    else
    {
        ui->pB_result_3->setText(tr("YES"));
        this->pB_result_type[3] = 2;
    }
}

// 去暗角——结果
void MainWindow::on_pB_result_3_clicked()
{
    if(this->pB_result_type[3] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;

    QImage image_old = ui->label_image->pixmap()->toImage();
    QImage ret = ui->label_image->pixmap()->toImage();

    ToolDark(image_old, ret);

    ui->label_image->setPixmap(QPixmap::fromImage(ret));

    ui->pB_result_3->setText(tr("YES"));
    this->pB_result_type[3] = 2;  // 完成美化
}

// 运动去模糊——检测
void MainWindow::on_pB_check_4_clicked()
{
    if(this->status_type == 0)
        return;

    ui->label_image->pixmap()->toImage().save("$temptoMat$.png");
    cv::Mat blurred = cv::imread("$temptoMat$.png");
    QFile::remove("$temptoMat$.png");

    cv::Mat grayBlurred;
    cvtColor(blurred, grayBlurred, CV_BGR2GRAY);

    if (!isBlurred(grayBlurred))
    {
        ui->pB_result_4->setText(tr("YES"));
        this->pB_result_type[4] = 2;
    }
    else
    {
        ui->pB_result_4->setText(tr("NO"));
        this->pB_result_type[4] = 1;
    }
}

// 运动去模糊——结果
void MainWindow::on_pB_result_4_clicked()
{
    if(this->pB_result_type[4] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;

    ui->label_image->pixmap()->toImage().save("$temptoMat$.jpg");
    cv::Mat blurred = cv::imread("$temptoMat$.jpg", CV_LOAD_IMAGE_COLOR);
    QFile::remove("$temptoMat$.jpg");

    cv::Mat deblurred;
    cv::Mat kernel;
    blindDeblurring(blurred, deblurred, kernel, 10);

    imwrite("$tempresult$.bmp", deblurred);

    QImage ret("$tempresult$.bmp");
    QFile::remove("$tempresult$.bmp");
    
    ui->label_image->setPixmap(QPixmap::fromImage(ret));

    ui->pB_result_4->setText(tr("YES"));
    this->pB_result_type[4] = 2;  // 完成美化
}

// 磨皮美颜——检测
void MainWindow::on_pB_check_5_clicked()
{
    if(this->status_type == 0)
        return;

    ui->label_image->pixmap()->toImage().save("$tempmeiyan$.png");
    cv::Mat mat_img = imread("$tempmeiyan$.png");
    QFile::remove("$tempmeiyan$.png");

    if (checkMeiYan(mat_img, this->is_face) == false)
    {
        ui->pB_result_5->setText(tr("YES"));
        this->pB_result_type[5] = 2;
    }
    else
    {
        ui->pB_result_5->setText(tr("NO"));
        this->pB_result_type[5] = 1;
    }
}

// 磨皮美颜——结果
void MainWindow::on_pB_result_5_clicked()
{
    if(this->pB_result_type[5] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    QImage image_old = ui->label_image->pixmap()->toImage();
    cv::Mat image = QImage2cvMat(image_old);

    if (checkMeiYan(image, this->is_face) != false)
    {
        Mat dst;
        
        int value1 = 3, value2 = 1;
        
        int dx = value1 * 5;    //双边滤波参数之一  
        double fc = value1 * 12.5; //双边滤波参数之一  
        int p = 50;//透明度  
        Mat temp1, temp2, temp3, temp4;
        
        //双边滤波  
        bilateralFilter(image, temp1, dx, fc, fc);
        //cv::imshow("1", image);
        temp2 = (temp1 - image + 128);
        
        //高斯模糊  
        GaussianBlur(temp2, temp3, Size(2 * value2 - 1, 2 * value2 - 1), 0, 0);
        //cv::imshow("2", image);
        temp4 = image + 2 * temp3 - 255;
        
        dst = (image*(100 - p) + temp4 * p) / 100;
        dst.copyTo(image);

        // 将 Mat 转成 QImage
        cv::Mat temp;
        cvtColor(image, temp, CV_BGR2RGB);
        QImage ret((const uchar *)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

        ui->label_image->setPixmap(QPixmap::fromImage(ret));

        this->status_type = 1;
        ui->pB_result_5->setText(tr("YES"));
        this->pB_result_type[5] = 2;  // 完成美化
    }

}

// 去红眼——检测
void MainWindow::on_pB_check_6_clicked()
{
    if(this->status_type == 0)
        return;

    QImage image = ui->label_image->pixmap()->toImage();

    cv::Mat mat_img = QImage2cvMat(image);

    cv::CascadeClassifier *haarcascade_eye = new CascadeClassifier("./haarcascade_eye.xml");
    std::vector<Rect> rect;
    haarcascade_eye->detectMultiScale(mat_img, rect, 1.03, 20, 0, cv::Size(40, 40), cv::Size(400, 400));

    delete haarcascade_eye;
    haarcascade_eye = nullptr;

    if(rect.size() != 0)
    {
        ui->pB_result_6->setText(tr("NO"));
        this->pB_result_type[6] = 1;
    }
    else
    {
        ui->pB_result_6->setText(tr("YES")); 
        this->pB_result_type[6] = 2;
    }
}

// 去红眼——结果
void MainWindow::on_pB_result_6_clicked()
{
    if(this->pB_result_type[6] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    QImage image_old = ui->label_image->pixmap()->toImage();

    cv::Mat image = QImage2cvMat(image_old);

    cv::CascadeClassifier *haarcascade_eye = new CascadeClassifier("./haarcascade_eye.xml");
    std::vector<Rect> rect;
    haarcascade_eye->detectMultiScale(image, rect, 1.03, 20, 0, cv::Size(40, 40), cv::Size(400, 400));

    delete haarcascade_eye;
    haarcascade_eye = nullptr;

    Scalar colors[] =
    {
        // 红橙黄绿青蓝紫  
        CV_RGB(255, 0, 0),
        CV_RGB(255, 97, 0),
        CV_RGB(255, 255, 0),
        CV_RGB(0, 255, 0),
        CV_RGB(0, 255, 255),
        CV_RGB(0, 0, 255),
        CV_RGB(160, 32, 240)
    };

    if (rect.size() != 0)
    {
        for (int i = 0; i < rect.size(); i++)
        {
            int h = 0;
            int *res = new int[rect[i].height*rect[i].width];
            for (int w = rect[i].y; w < rect[i].y + rect[i].height; w++)
            {
                for (int z = rect[i].x; z < rect[i].x + rect[i].width; z++)
                {
                    if ((image.at<Vec3b>(w, z)[2] > (image.at<Vec3b>(w, z)[0] + image.at<Vec3b>(w, z)[1] - 22)) && (image.at<Vec3b>(w, z)[2] > 80))
                        image.at<Vec3b>(w, z)[2] = 0;
                }
            }
        }

        // 将 Mat 转成 QImage
        cv::Mat temp;
        cvtColor(image, temp, CV_BGR2RGB);
        QImage ret((const uchar *)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888); 
       
        ui->label_image->setPixmap(QPixmap::fromImage(ret));

        ui->pB_result_6->setText(tr("YES"));
        this->status_type = 1;
        this->pB_result_type[6] = 2;  // 完成美化
    }
}

// 直方图均衡化——检测
void MainWindow::on_pB_check_7_clicked()
{
    if(this->status_type == 0)
        return;

    this->pB_result_type[7] = 1;
}

// 直方图均衡化——结果
void MainWindow::on_pB_result_7_clicked()
{
    if (this->pB_result_type[7] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    QImage image = ui->label_image->pixmap()->toImage();
    int image_width = image.width();
    int image_height = image.height();
    int pixel_sum = image_width * image_height;  // 总像素数
    int r_hist[256], g_hist[256], b_hist[256];  // rgb各分量灰度级数组
    int r_equ[256], g_equ[256], b_equ[256];  // 均衡化后 rgb 各分量灰度级数组
    memset(r_hist, 0, sizeof(r_hist));
    memset(g_hist, 0, sizeof(g_hist));
    memset(b_hist, 0, sizeof(b_hist));
    float r_old[256], g_old[256], b_old[256];  // 均衡化前各灰度级概率
    float r_new[256], g_new[256], b_new[256];  // 均衡化后各灰度级概率

    QRgb pixel_rgb;

    // 统计各灰度级数量
    for (int i = 0; i < image_width; i++)
    {
        for (int j = 0; j < image_height; j++)
        {
            pixel_rgb = image.pixel(i, j);
            r_hist[qRed(pixel_rgb)]++;
            g_hist[qGreen(pixel_rgb)]++;
            b_hist[qBlue(pixel_rgb)]++;
        }
    }

    // 均衡化前各灰度级概率
    for (int i = 0; i < 256; i++)
    {
        r_old[i] = (float)r_hist[i] / (float)pixel_sum;
        g_old[i] = (float)g_hist[i] / (float)pixel_sum;
        b_old[i] = (float)b_hist[i] / (float)pixel_sum;
    }

    // 均衡化后各灰度级概率
    r_new[0] = r_old[0];
    g_new[0] = g_old[0];
    b_new[0] = b_old[0];
    for (int i = 1; i < 256; i++)
    {
        r_new[i] = r_new[i - 1] + r_old[i];
        g_new[i] = g_new[i - 1] + g_old[i];
        b_new[i] = b_new[i - 1] + b_old[i];
    }

    // 均衡化后对应的像素值
    for (int i = 0; i < 256; i++)
    {
        r_equ[i] = int(r_new[i] * 255);
        g_equ[i] = int(g_new[i] * 255);
        b_equ[i] = int(b_new[i] * 255);
    }

    QImage ret = image;
    for (int i = 0; i < image_width; i++)
    {
        for (int j = 0; j < image_height; j++)
        {
            pixel_rgb = image.pixel(i, j);
            ret.setPixel(i, j,
                qRgba(r_equ[qRed(pixel_rgb)], g_equ[qGreen(pixel_rgb)], b_equ[qBlue(pixel_rgb)], qAlpha(pixel_rgb)));
        }
    }

    ui->label_image->setPixmap(QPixmap::fromImage(ret));

    ui->pB_result_7->setText(tr("YES"));
    this->status_type = 1;
    this->pB_result_type[7] = 2;  // 完成美化
}

// ACE——检测
void MainWindow::on_pB_check_8_clicked()
{
    if(this->status_type == 0)
        return;

    this->pB_result_type[8] = 1;
}

// ACE——结果
void MainWindow::on_pB_result_8_clicked()
{
    if(this->pB_result_type[8] != 1)  // 如果检测结果不是 X 则需要美化
        return;
    ui->label_image->pixmap()->toImage().save("$tempACE$.png");
    cv::Mat mat_img = cv::imread("$tempACE$.png");
    QFile::remove("$tempACE$.png");

    PAstocket a;
    cv::Mat new_mat = a.getmessage(mat_img);

    cv::imwrite("$tempACE$.png", new_mat);
    QImage ret("$tempACE$.png");
    QFile::remove("$tempACE$.png");

    QFile::remove("template.png");
    QFile::remove("enhance.jpg");

    ui->label_image->setPixmap(QPixmap::fromImage(ret));

    ui->pB_result_8->setText(tr("YES"));
    this->status_type = 1;
    this->pB_result_type[8] = 2;  // 完成美化
}

// 霓虹滤镜——检测
void MainWindow::on_pB_check_9_clicked()
{
    if(this->status_type == 0)
        return;

    this->pB_result_type[9] = 1;
}

// 霓虹滤镜——结果
void MainWindow::on_pB_result_9_clicked()
{
    if (this->pB_result_type[9] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    ui->label_image->pixmap()->toImage().save("$tempNH$.png");
    cv::Mat mat_img = cv::imread("$tempNH$.png");
    QFile::remove("$tempNH$.png");

    niHongFilter(mat_img);

    cv::imwrite("$tempNH$.png", mat_img);
    QImage ret("$tempNH$.png");
    QFile::remove("$tempNH$.png");

    ui->label_image->setPixmap(QPixmap::fromImage(ret));

    ui->pB_result_9->setText(tr("YES"));
    this->status_type = 1;
    this->pB_result_type[9] = 2;  // 完成美化
}

// 显示参数
void MainWindow::on_pushButton_clicked()
{
    if (this->status_type == 0)
    {
        QMessageBox::about(NULL, tr("警告"), tr("未加载图片！"));
        return;
    }

    int gray_sum = 0;     // 图片总灰度

    QImage label_image = ui->label_image->pixmap()->toImage();

    int image_width = label_image.width();
    int image_height = label_image.height();


    int pixel_gray = 0;  // 当前像素点的灰度值
    int pixel_sum = image_width * image_height;    // 总像素数

    int gray_array[256];  // 灰度数组
    memset(gray_array,0,sizeof(gray_array));

    // 统计灰度数组
    for(int i = 0; i < image_width; i++)
    {
        for(int j = 0; j < image_height; j++)
        {
            pixel_gray = qGray(label_image.pixel(i, j));
            gray_array[pixel_gray]++;
            gray_sum += pixel_gray;
        }
    }

    int gray_mean = gray_sum / pixel_sum;  // 平均灰度值

    // 求灰度中位数，从两端开始搜，两端较大的减去较小的
    int pointer_st = 0, pointer_ed = 255;
    int gray_copy_array[256];
    memcpy(gray_copy_array, gray_array,sizeof(gray_array));
    while(pointer_ed > pointer_st)
    {
        if(gray_copy_array[pointer_st] > gray_copy_array[pointer_ed])
        {
            gray_copy_array[pointer_st] -= gray_copy_array[pointer_ed];
            gray_copy_array[pointer_ed] = 0;
        }
        else
        {
            gray_copy_array[pointer_ed] -= gray_copy_array[pointer_st];
            gray_copy_array[pointer_st] = 0;
        }

        if(gray_copy_array[pointer_st] <= 0)
            pointer_st++;
        else
            gray_copy_array[pointer_st]--;

        if(gray_copy_array[pointer_ed] <= 0)
            pointer_ed--;
        else
            gray_copy_array[pointer_ed]--;
    }

    int gray_mid = (pointer_st + pointer_ed) / 2;  // 灰度中位数

    float gray_standard_deviation = 0;  // 灰度标准差
    int gray_max = 0;  // 最大灰度
    for(int i = 0; i < 256; i++)  // 加权平均，而非直接 1/n
    {
        if(gray_array[pixel_gray] > gray_max)  // 考虑减少复杂度，所以把求最多灰度值放这里了
            gray_max = gray_array[pixel_gray];

        gray_standard_deviation += float((gray_mean - i)*(gray_mean - i))/float(pixel_sum)*float(gray_array[i]);
    }

    gray_standard_deviation = sqrt(gray_standard_deviation);

    PamShow pam;
    pam.InitGray(gray_array, gray_max);
    pam.SetGray(pixel_sum, gray_mid, gray_mean, gray_standard_deviation);
    pam.SetBG(this->offset_bg);
    pam.SetDarkChannel(this->dark_channels);
    pam.exec();
}
