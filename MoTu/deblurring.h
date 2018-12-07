#ifndef DEBLURRING_H
#define DEBLURRING_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

void computeDFT(const Mat &image, Mat *result);
void computeIDFT(Mat *input, Mat &result);
void wienerFilter(const Mat &blurredImage, const Mat &known, Mat &unknown, float k);
void rotate(const Mat &src, Mat &dst);
void blindDeblurringOneChannel(const Mat &blurred, Mat &kernel, int kernelSize, int iters, float noisePower);
void blindDeblurring(const Mat &blurred, Mat &deblurred, Mat &kernel, int iters);
void applyConstraints(Mat &image, float thresholdValue);
Mat getAutoCerrelation(const Mat &blurred);
int estimateKernelSize(const Mat &blurred);
void cropBorder(Mat &image);
float measureBlur(const Mat &grayBlurred);
bool isBlurred(const Mat &grayBlurred);
float getInvSNR(const Mat &grayBlurred);
Mat erosion(const Mat &grayImage, int erosionSize);
Mat sharpImage(const Mat &grayImage, float sigmar);

#endif // DEBLURRING_H