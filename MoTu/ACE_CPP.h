#ifndef ACECPP_H
#define ACECPP_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "opencv2/core/core.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>

class PAstocket 
{
public:
    cv::Mat getmessage(cv::Mat image);

    /* 使用静态一次调用 防止重复调用出现崩溃 
    ** 在主界面的构造和析构函数中调用 
    */
    static void PyInit();
    static void PyFinit();
};

#endif // ACECPP_H
