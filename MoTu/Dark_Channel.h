#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include<QImage>
#include<iostream>
#include<vector>
#include <algorithm>
using namespace std;

class Dark_Channel
{
public:
	QImage MainImage;
	Dark_Channel(QImage sendimage);

	~Dark_Channel();
	QImage darkChannel1;

	bool Detection();

	uchar light(vector<uchar> inputIamgeMax);

	QImage HazeRemoval();

private:
	int Getmin(int a, int b, int c);

};

/*
使用方法，初始化必须传入一个qimage对象，然后调用Detection会返回一个bool值（false表示不建议使用此算法，true表示建议）
调用HazeRemoval()返回一个优化完的QImage
Dark_Channel darkchannel(image);
darkchannel.Detection();
image=darkchannel.HazeRemoval().copy();
*/
