#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include"Dark_Channel.h"


Dark_Channel::Dark_Channel(QImage sendimage)
{
	MainImage = sendimage.copy();
}

Dark_Channel::~Dark_Channel()
{
}
int Dark_Channel::Getmin(int a,int b,int c)
{
	if (a > b&&a > c)
		return a;
	if (b > a&&b > c)
		return c;
	if (c > a&&c > b)
		return c;
}
bool Dark_Channel::Detection()
{
	// 模板尺寸
	int scale = 7;
	//边界扩充
	int radius = (scale - 1) / 2;
	QImage ret(MainImage.width(), MainImage.height(), QImage::Format_Indexed8);
	QImage border(ret.width() + radius * 2, ret.height() + radius * 2, QImage::Format_Indexed8);

	ret.setColorCount(256);
	for (int i = 0; i < 256; i++)
	{
		ret.setColor(i, qRgb(i, i, i));
	}
	for (int i = 0; i < MainImage.height(); i++)
	{
		uchar *pDest = (uchar *)ret.scanLine(i);
		for (int j = 0; j < MainImage.width(); j++)
		{
			int r = QColor(MainImage.pixel(j, i)).red();
			int g = QColor(MainImage.pixel(j, i)).green();
			int b = QColor(MainImage.pixel(j, i)).blue();
			pDest[j] = Getmin(r, g, b);
		}
	}

	border.setColorCount(256);
	for (int i = 0; i < 256; i++)
	{
		border.setColor(i, qRgb(i, i, i));
	}
	for (int i = 0; i < border.height(); i++)
	{
		uchar *retI = (uchar *)border.scanLine(i);
		for (int j = 0; j < border.width(); j++)
		{
			retI[j] = 255;
		}
	}
	for (int i = 0; i < MainImage.height(); i++)
	{
		uchar *pDest = (uchar *)border.scanLine(i+ radius);
		uchar *pDest2 = (uchar *)ret.scanLine(i);
		for (int j = 0; j < MainImage.width(); j++)
		{
			pDest[j + radius] = pDest2[j];
		}
	}

	long midel = 0;
	//最小值滤波
	int a = 0;
	//cout << sizeof(borderI)/sizeof(int) << endl;
	int time = 0;
	for (int i = radius; i < border.height() - radius; i++)
	{
		uchar *retI = (uchar *)ret.scanLine(i-radius);
		for (int j = radius; j < border.width() - radius; j++)
		{
			int min = 9999;
			int max = -1;
			for (int t = - radius;t<= radius;t++)
			{
				uchar *borderI = (uchar *)border.scanLine(t + i);

				for (int h = -radius; h <= radius; h++)
				{

					if (borderI[j + h] < min)
						min = borderI[j + h];
				}
				
			}
			time++;
			retI[j - radius] = min;
			midel +=min ;
		}
	}
	darkChannel1 = ret.copy();
	midel = midel / (MainImage.width()*MainImage.height());
	if (midel > 55)
		return true;
	else
		return false;
}

uchar Dark_Channel::light(vector<uchar> inputIamgeMax)
{
	uchar maxA = 0;
	for (int i = 0; i < inputIamgeMax.size() - 1; i++)
	{

		if (maxA < inputIamgeMax[i + 1])
		{
			maxA = inputIamgeMax[i + 1];
		}
	}
	return maxA;
}

void getmaxmin(int &min, int &max, uchar *num,int LEN)
{
	int MINI = 9999;
	int MAXA = -9999;
	int min_i;
	int max_i;
	for (int i = 0; i < LEN; i++)
	{
		if (num[i] < MINI)
		{
			MINI = num[i];
			min_i = i;
		}
		if (num[i] > MAXA)
		{
			MAXA = num[i];
			max_i = i;
		}
	}
	min = min_i;
	max = max_i;
}
QImage Dark_Channel::HazeRemoval()
{
	vector<uchar> inputMax;
	QImage temp = darkChannel1.copy();
	uchar *pDest = (uchar *)temp.scanLine(0);
	int max;
	int min;
	for (long i = 0; i < ((temp.width()*temp.height()) / 1000); i++)
	{
		getmaxmin(min, max, pDest, (temp.width()*temp.height()));
		//max = min;
		inputMax.push_back(uchar(pDest[max]));
		pDest[max] = pDest[min];
		//此处缺少一个画圆的算法
	}
	uchar A = light(inputMax);
	double w = 0.65;
	//uchar A = 149;
	QImage ret(MainImage.width(), MainImage.height(), MainImage.format());

	//for (int i = 0; i < 256; i++)
	//{
	//	ret.setColor(i, qRgb(i, i, i));
	//}
	for (int i = 0; i < MainImage.height(); i++)
	{
		uchar *pDest = (uchar *)ret.scanLine(i);
		uchar *pdark = (uchar *)darkChannel1.scanLine(i);
		for (int j = 0; j < MainImage.width(); j++)
		{
			int r = abs((1 - w * pdark[j] / double(A)) * 255);
			int g = r;
			int b = r;
			//cout << r << endl;;
			//int r = QColor(MainImage.pixel(j, i)).red();
			//int g = QColor(MainImage.pixel(j, i)).green();
			//int b = QColor(MainImage.pixel(j, i)).blue();
			ret.setPixel(j, i, qRgb(r, g, b));
		}
	}
	QImage tempi(MainImage.width(), MainImage.height(), MainImage.format());
	//tempi.setColorCount(256);
	//for (int i = 0; i < 256; i++)
	//{
	//	tempi.setColor(i, qRgb(i, i, i));
	//}
	double t0 = 0.1;
	//for (int i = 0; i < MainImage.height(); i++)
	//{
	//	uchar *ptemp = (uchar *)tempi.scanLine(i);
	//	uchar *pMain = (uchar *)MainImage.scanLine(i);
	//	for (int j = 0; j < MainImage.width(); j++)
	//	{
	//		int r = abs(QColor(MainImage.pixel(j, i)).red() - A);
	//		int g = abs(QColor(MainImage.pixel(j, i)).green() - A);
	//		int b = abs(QColor(MainImage.pixel(j, i)).blue() - A);
	//		tempi.setPixel(j, i, qRgb(r, g, b));

	//	}
	//}
	for (int i = 0; i < MainImage.height(); i++)
	{
		uchar *ptemp = (uchar *)tempi.scanLine(i);
		uchar *pMain = (uchar *)MainImage.scanLine(i);
		for (int j = 0; j < MainImage.width(); j++)
		{
			int r = abs(QColor(MainImage.pixel(j, i)).red());
			int g = abs(QColor(MainImage.pixel(j, i)).green() );
			int b = abs(QColor(MainImage.pixel(j, i)).blue());
			double rr=abs(QColor(ret.pixel(j, i)).red());
			//double tmax = (rr / 255) < t0 ? t0 : (rr / 255);
			double tmax;
			if (double(rr / 255.0) < t0)
			{
				tmax = t0;
			}
			else
				tmax = (rr / 255);
			//cout << "r   g   b tmax" << r << "--" << g << "--" << b << "--" << tmax << endl;
			//cout << rr << endl;
			int r2= abs((r - A) / tmax + A) > 255 ? 255 : abs((r - A) / tmax + A);
			int g2 = abs((g - A) / tmax + A) > 255 ? 255 : abs((g - A) / tmax + A);
			int b2 = abs((b - A) / tmax + A) > 255 ? 255 : abs((b - A) / tmax + A);

			tempi.setPixel(j, i, qRgb(r2, g2, b2));
		}
	}
	return tempi;
}
