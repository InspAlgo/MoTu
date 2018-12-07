#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "ACE_CPP.h"
#include "windows.h"
#include <Python.h>
#include <QMessagebox>

using namespace std;

void PAstocket::PyInit()
{
	Py_Initialize();
}

void PAstocket::PyFinit()
{
    Py_Finalize();
}

cv::Mat PAstocket::getmessage(cv::Mat image)
{
    cv::imwrite("template.png", image);

    PyObject * pModule = PyImport_ImportModule("ACE");
    PyObject * pv = PyObject_GetAttrString(pModule, "FACE");
    PyObject* args = PyTuple_New(0);   // 0个参数
    PyObject* pRet = PyObject_CallObject(pv, args);

    cv::Mat re = cv::imread("enhance.jpg");

	return re;
}

//int main()
//{
//	PAstocket a;
//	Mat re = imread("1.png");
//	imshow("result", a.getmessage(re));
//	waitKey(0);
//}