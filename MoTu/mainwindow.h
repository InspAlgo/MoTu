#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QEvent>
#include <QCloseEvent>
#include <QMouseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event);  // 界面窗口尺寸变化时
    void closeEvent(QCloseEvent *event);  // 界面关闭前响应
    void wheelEvent(QWheelEvent *event);  // 滚轮缩放图片控件
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

protected slots:
    bool eventFilter(QObject *, QEvent *);  // 鼠标按住可以拖动图片控件

private:
    void InitPamater();  // 初始化成员变量
    void InitMenu();  // 初始化菜单栏
    void AutoLabelSize();  // 自动调整 label 大小，使其比例和图片比例相同

private slots:
    void FileOpen();
    void FileSave();
    void FileSaveAs();
    void FileClose();

    void on_pB_check_clicked();
    void on_pB_tuning_clicked();

    void on_pB_check_1_clicked();

    void on_pB_result_1_clicked();

    void on_pB_check_2_clicked();

    void on_pB_result_2_clicked();

    void on_pB_check_3_clicked();

    void on_pB_result_3_clicked();

    void on_pB_check_4_clicked();

    void on_pB_result_4_clicked();

    void on_pB_check_5_clicked();

    void on_pB_result_5_clicked();

    void on_pB_check_6_clicked();

    void on_pB_result_6_clicked();

    void on_pB_check_7_clicked();

    void on_pB_result_7_clicked();

    void on_pB_check_8_clicked();

    void on_pB_result_8_clicked();

    void on_pB_check_9_clicked();

    void on_pB_result_9_clicked();

private:
    Ui::MainWindow *ui;

    QMenu *menu_file;  // 菜单栏-文件
    QAction *file_open;  // 文件-打开
    QAction *file_save;  // 文件-保存
    QAction *file_save_as;  // 文件-另存为
    QAction *file_close;  // 文件-关闭

    QImage *image;  // 存显示的图片
    int status_type;  // 当前状态 0-未加载图片 1-编辑图片 2-已保存图片

    QString image_name;  // 不包含后缀的文件名
    QString file_type;  // 文件后缀
    QString path;  // 不包含文件名的路径

    bool ctrl_is_pressed;  // 判断 Ctrl 键是否被按住

    int pB_result_type[10];  // 检测结果

    bool is_face;  // 图片中是否含有人脸 无-false 有-true

};

#endif // MAINWINDOW_H
