#ifndef MEIYANCPP_H
#define MEIYANCPP_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <iostream>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"

using namespace std;
using namespace cv;

void detectAndDraw(Mat& img,
    CascadeClassifier& cascade,
    double scale);
bool checkMeiYan(Mat &mat_img, bool &is_face);

#endif // MEIYANCPP_H