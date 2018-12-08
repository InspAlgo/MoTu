﻿#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "meiyancpp.h"

String cascadeName = "haarcascade_frontalface_alt2.xml";//人脸的训练数据									
bool isDetected = false;
bool isFace = false;

//int main(int argc, const char** argv)
//{
//	Mat image;
//	CascadeClassifier cascade;//创建级联分类器对象
//	double scale = 1.3;
//
//
//	image = imread("test.jpg");
//
//	//namedWindow("磨皮前");//opencv2.0以后用namedWindow函数会自动销毁窗口
//	cv::imshow("磨皮前", image);
//	
//
//	if (!cascade.load(cascadeName))//从指定的文件目录中加载级联分类器
//	{
//		cerr << "级联分类器加载错误" << endl;
//		return 0;
//	}
//
//	if (!image.empty())//读取图片数据不能为空
//	{
//		detectAndDraw(image, cascade, scale);
//		if (isDetected == false)
//		{
//			Mat dst;
//
//			int value1 = 3, value2 = 1;
//
//			int dx = value1 * 5;    //双边滤波参数之一  
//			double fc = value1 * 12.5; //双边滤波参数之一  
//			int p = 50;//透明度  
//			Mat temp1, temp2, temp3, temp4;
//
//			//双边滤波  
//			bilateralFilter(image, temp1, dx, fc, fc);
//			//cv::imshow("1", image);
//			temp2 = (temp1 - image + 128);
//
//			//高斯模糊  
//			GaussianBlur(temp2, temp3, Size(2 * value2 - 1, 2 * value2 - 1), 0, 0);
//			//cv::imshow("2", image);
//			temp4 = image + 2 * temp3 - 255;
//
//			dst = (image*(100 - p) + temp4 * p) / 100;
//			dst.copyTo(image);
//		}
//		namedWindow("磨皮后");
//		cv::imshow("磨皮后", image);
//		
//		waitKey(0);
//	}
//
//	return 0;
//}

bool checkMeiYan(Mat &mat_img, bool &is_face)
{
    CascadeClassifier cascade;//创建级联分类器对象
    double scale = 1.3;

    if (!cascade.load("haarcascade_frontalface_alt2.xml"))  // 从指定的文件目录中加载级联分类器
    {
        cerr << "级联分类器加载错误" << endl;
        return false;
    }

    detectAndDraw(mat_img, cascade, scale);

    is_face = isFace;

    if (isDetected == false)
        return false;
    else
        return true;
}

void detectAndDraw(Mat& img,
	CascadeClassifier& cascade,
	double scale)
{
	int i = 0;
	double t = 0;
	vector<Rect> faces;
	const static Scalar colors[] = { CV_RGB(0,0,255),
		CV_RGB(0,128,255),
		CV_RGB(0,255,255),
		CV_RGB(0,255,0),
		CV_RGB(255,128,0),
		CV_RGB(255,255,0),
		CV_RGB(255,0,0),
		CV_RGB(255,0,255) };//用不同的颜色表示不同的人脸

	Mat gray, smallImg(cvRound(img.rows / scale), cvRound(img.cols / scale), CV_8UC1);//将图片缩小，加快检测速度

	cvtColor(img, gray, COLOR_RGB2GRAY);//因为用的是类haar特征，所以都是基于灰度图像的，这里要转换成灰度图像
	resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);//将尺寸缩小到1/scale,用线性插值
	equalizeHist(smallImg, smallImg);//直方图均衡




	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		| CASCADE_SCALE_IMAGE
		,
		Size(30, 30));

        

	for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++)
	{
		isDetected = true;
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center, left, right;
		Scalar color = colors[i % 8];
		int radius;
		center.x = cvRound((r->x + r->width*0.5)*scale);//还原成原来的大小
		center.y = cvRound((r->y + r->height*0.5)*scale);
		radius = cvRound((r->width + r->height)*0.25*scale);


		left.x = center.x - radius;
		left.y = cvRound(center.y - radius * 1.3);

		if (left.y < 0)
		{
			left.y = 0;
		}

		right.x = center.x + radius;
		right.y = cvRound(center.y + radius * 1.3);

		if (right.y > img.rows)
		{
			right.y = img.rows;
		}

		//rectangle(img, left, right, Scalar(255, 0, 0));  // 人脸识别矩形框
        if (fabs(right.x - left.x) >= 50)
            isFace = true;

		Mat roi = img(Range(left.y, right.y), Range(left.x, right.x));
		Mat dst;

		int value1 = 3, value2 = 1;

		int dx = value1 * 5;    //双边滤波参数之一  
		double fc = value1 * 12.5; //双边滤波参数之一  
		int p = 50;//透明度  
		Mat temp1, temp2, temp3, temp4;

		//双边滤波  
		bilateralFilter(roi, temp1, dx, fc, fc);
		//cv::imshow("1", temp1);
		temp2 = (temp1 - roi + 128);
		//cv::imshow("2", temp2);
		//高斯模糊  
		GaussianBlur(temp2, temp3, Size(2 * value2 - 1, 2 * value2 - 1), 0, 0);

		temp4 = roi + 2 * temp3 - 255;

		dst = (roi*(100 - p) + temp4 * p) / 100;


		dst.copyTo(roi);
	}

}