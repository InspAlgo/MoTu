#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "removal_vignetting.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->InitPamater();
    this->InitMenu();

//    this->showMaximized();  // 全屏
}

MainWindow::~MainWindow()
{
    delete ui;
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
        QMessageBox::about(NULL, tr("警告"), tr("未保存图片！"));
        event->ignore();  // 需要引入头文件
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
            ui->label_image->resize(label_width, label_height);
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
        ui->label_image->move(ui->label_image->x()+dx,ui->label_image->y()+dy);
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

    for(int i = 0; i < 9; i++)
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
        QMessageBox::about(NULL, tr("警告"), tr("未保存图片！"));
        return;
    }

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
    this->on_pB_check_1_clicked();
    this->on_pB_check_2_clicked();
    this->on_pB_check_3_clicked();
    this->on_pB_check_4_clicked();
    this->on_pB_check_5_clicked();
    this->on_pB_check_6_clicked();
    this->on_pB_check_7_clicked();
    this->on_pB_check_8_clicked();
}

// 美化所有
void MainWindow::on_pB_tuning_clicked()
{
    this->on_pB_result_1_clicked();
    this->on_pB_result_2_clicked();
    this->on_pB_result_3_clicked();
    this->on_pB_result_4_clicked();
    this->on_pB_result_5_clicked();
    this->on_pB_result_6_clicked();
    this->on_pB_result_7_clicked();
    this->on_pb_result_8_clicked();
}

// 曝光检测——检测
void MainWindow::on_pB_check_1_clicked()
{
    if(this->status_type == 0)
        return;

    int width = ui->label_image->width();
    int height = ui->label_image->height();
    int temp;
    double da = 0; //偏离128的均值
    double daAbsolute; //da的绝对值
    double ma; //偏离128的平均偏差
    double maAbsolute; //ma的绝对值
    double k; //亮度系数
    int hist[256];
    memset(hist, 0, sizeof(hist));

    QImage temp_img = ui->label_image->pixmap()->toImage();

    for(int i = 0; i < width; ++i)
    {
        for(int j = 0; j < height; ++j)
        {
            temp = qGray(temp_img.pixel(i,j));
            hist[temp]++; //计算灰度直方图的灰度数
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
        }

        else
        {
            ui->pB_result_1->setText(tr("过暗"));
            this->pB_result_type[1] = 1;
        }

    }
    else
    {
        ui->pB_result_1->setText(tr("亮度正常"));
        this->pB_result_type[1] = 2;
    }
}

// 曝光检测——结果
void MainWindow::on_pB_result_1_clicked()
{
    if(this->pB_result_type[1] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;

    ui->pB_result_1->setText(tr("亮度正常"));
    this->pB_result_type[1] = 2;  // 完成美化
}

// 去雾——检测
void MainWindow::on_pB_check_2_clicked()
{
    if(this->status_type == 0)
        return;
}

// 去雾——结果
void MainWindow::on_pB_result_2_clicked()
{
    if(this->pB_result_type[2] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;


    this->pB_result_type[2] = 2;  // 完成美化
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
}

// 运动去模糊——结果
void MainWindow::on_pB_result_4_clicked()
{
    if(this->pB_result_type[4] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;



    this->pB_result_type[4] = 2;  // 完成美化
}

// 磨皮美颜——检测
void MainWindow::on_pB_check_5_clicked()
{
    if(this->status_type == 0)
        return;
}

// 磨皮美颜——结果
void MainWindow::on_pB_result_5_clicked()
{
    if(this->pB_result_type[5] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;


    this->pB_result_type[5] = 2;  // 完成美化
}

// 去红眼——检测
void MainWindow::on_pB_check_6_clicked()
{
    if(this->status_type == 0)
        return;
}

// 去红眼——结果
void MainWindow::on_pB_result_6_clicked()
{
    if(this->pB_result_type[6] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;

    this->pB_result_type[6] = 2;  // 完成美化
}

// 直方图均衡化——检测
void MainWindow::on_pB_check_7_clicked()
{
    if(this->status_type == 0)
        return;
}

// 直方图均衡化——结果
void MainWindow::on_pB_result_7_clicked()
{
    if(this->pB_result_type[7] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;

    this->pB_result_type[7] = 2;  // 完成美化
}

// ACD——检测
void MainWindow::on_pB_check_8_clicked()
{
    if(this->status_type == 0)
        return;
}

// ACD——结果
void MainWindow::on_pb_result_8_clicked()
{
    if(this->pB_result_type[8] != 1)  // 如果检测结果不是 X 则需要美化
        return;

    this->status_type = 1;

    this->pB_result_type[8] = 2;  // 完成美化
}
