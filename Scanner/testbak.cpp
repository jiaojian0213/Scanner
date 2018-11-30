/*
#include <opencv2/core/core.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <opencv2/imgproc/types_c.h>

#include <windows.h>
#include <iostream>
#include <string>
using namespace std;
using namespace cv;

static const int BLACK = 0;
static const int WHITE = 255;


void setPixel(Mat image, int x, int y, uchar pValue);
Mat toGray(Mat image);
void LutFunc(Mat &I);
void AtFunc(Mat &I);
void PtrFunc(Mat &I);
void  IteratorFunc(Mat &I);
int showImage(const std::string path, const std::string name);
int handlerImage1(std::string path, std::string name);
int handlerImage(std::string path, std::string name);
void FindFile(const std::string strPath)
{

	WIN32_FIND_DATAA  findData = { 0 };
	string strFindPath = strPath + "\\*.*";
	//查找第一个文件
	HANDLE hFindFine = FindFirstFileA(strFindPath.c_str(), &findData);
	if (INVALID_HANDLE_VALUE == hFindFine)
		printf("Error:%d", GetLastError());
	//循环递归查找文件
	do
	{
		//判断该文件是否为文件夹
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (findData.cFileName[0] == '.')
				continue;
			cout << "在" << strPath << "找到文件夹:\t" << findData.cFileName << endl;
			string strNeedFindPaht = strPath + "\\" + findData.cFileName;
			//若该文件是文件夹，则递归进行查找该文件夹下的文件
			FindFile(strNeedFindPaht);
		}
		else
		{
			//cout << "在" << strPath << "找到文件:\t" << findData.cFileName << endl;
			cout << strPath + "\\" + findData.cFileName << endl;

			//showImage(strPath + "\\" + findData.cFileName, findData.cFileName);
			handlerImage1(strPath, findData.cFileName);
			//return;

		}
	} while (FindNextFileA(hFindFine, &findData));
	//关闭文件搜索句柄
	FindClose(hFindFine);
}


int mainss(int argc, char* argv[])
{
	//搜索D盘下Test文件夹下的所有文件
	string strFilePath = "C:\\Users\\jiaojian\\Desktop\\耳标图片";
	FindFile(strFilePath);
	//system("pause");
	cv::waitKey(0);
	return 0;
}


int handlerImage1(std::string path, std::string name){
	Mat image, gray;
	image = imread(path + "\\" + name);
	if (!image.data)
		return -1;
	//cvtColor(image, gray, CV_BGR2GRAY);

	Mat result, result1, result2, result3;

	Mat image1 = toGray(image);
	IplImage* iplImage = new IplImage(image1);

	cvAdaptiveThreshold(iplImage, iplImage, 250, CV_ADAPTIVE_THRESH_MEAN_C, 1, 25, 5);

	IplConvKernel* kernel = cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_RECT);

	//cvErode(iplImage2, iplImage2, kernel, 1);
	//cvMorphologyEx(iplImage, iplImage, kernel, kernel, CV_MOP_OPEN, 1);
	//cvErode(iplImage2, iplImage2, kernel, 1);
	cvMorphologyEx(iplImage, iplImage, NULL, kernel, CV_MOP_CLOSE, 1);
	cvDilate(iplImage, iplImage, kernel, 1);

	cvReleaseStructuringElement(&kernel);


	Mat aa = cvarrToMat(iplImage);
	bitwise_not(aa, result2);
	//namedWindow(name + "灰色");
	//imshow(name + "灰色", result2);
	//cvNamedWindow((name + "灰色").c_str());
	//cvShowImage((name + "灰色").c_str(), iplImage);

	//namedWindow(name + "二值化图像");
	//Mat mtx = cv::cvarrToMat(iplImage);
	//imshow(name + "二值化图像", mtx);
	//namedWindow(name + "二值化图像1");
	//Mat mtx = cv::cvarrToMat(iplImage);
	//bitwise_not(result1, result2);


	//erode(result2, result3, NULL, Point(-1, -1), 5, 0, morphologyDefaultBorderValue());
	//imshow(name + "二值化图像1", result3);

	std::string outFileName = path + "\\c\\" + name;
	imwrite(outFileName, result2);
	cout << outFileName << endl;
	return 0;
}



int handlerImage(std::string path, std::string name)
{
	Mat image, gray;
	image = imread(path + "\\" + name);
	if (!image.data)
		return -1;


	namedWindow(name + "image");
	imshow(name + "image", image);
	/*
	cvtColor(image, gray, CV_BGR2GRAY);
	Mat img_lut, img_at, img_ptr, img_iterator;
	gray.copyTo(img_lut);

	gray.copyTo(img_at);
	gray.copyTo(img_ptr);
	gray.copyTo(img_iterator);
	double  TickCount = (double)getTickCount();
	LutFunc(img_lut);
	std::cout << "lut function" <<
	((double)getTickCount() - TickCount) / getTickFrequency() * 1000 << "ms" << std::endl;
	TickCount = (double)getTickCount();
	AtFunc(img_at);
	std::cout << "at function" <<
	((double)getTickCount() - TickCount) / getTickFrequency() * 1000 << "ms" << std::endl;
	TickCount = (double)getTickCount();
	PtrFunc(img_ptr);
	std::cout << "ptr function" <<
	((double)getTickCount() - TickCount) / getTickFrequency() * 1000 << "ms" << std::endl;
	TickCount = (double)getTickCount();
	IteratorFunc(img_iterator);
	std::cout << "iterator function" <<
	((double)getTickCount() - TickCount) / getTickFrequency() * 1000 << "ms" << std::endl;
	namedWindow(name+"sample");
	imshow(name + "sample", img_lut);

	int nWidth = img_lut.cols;
	int nHeight = img_lut.rows;

	int j, i;
	cout << nWidth << "---" << nHeight << endl;

	//Mat result;
	//result = image.clone();
	//进行二值化处理，选择30，200.0为阈值
	//threshold(image, result, 30, 200.0, CV_THRESH_BINARY);
	//namedWindow(name + "二值化图像");
	//imshow(name + "二值化图像", result);



	/
	Mat image1 = toGray(image);
	IplImage* iplImage = new IplImage(image1.clone());

	//IplImage* iplImage2 = cvCreateImage(cvSize(iplImage->width, iplImage->height), IPL_DEPTH_8U, 1);
	//Binarization(iplImage);
	Mat result, result1, result2, result3;
	//threshold(image1.clone(), result, 140, 250.0, CV_THRESH_BINARY);
	adaptiveThreshold(image1.clone(), result1, 250, CV_ADAPTIVE_THRESH_MEAN_C, 1, 25, 5);
	//cvAdaptiveThreshold(iplImage, iplImage2, 250, CV_ADAPTIVE_THRESH_MEAN_C, 1, 25, 5);

	//namedWindow(name + "灰色");
	//imshow(name + "灰色", image1);

	//namedWindow(name + "二值化图像");
	//Mat mtx = cv::cvarrToMat(iplImage);
	//imshow(name + "二值化图像", mtx);
	namedWindow(name + "二值化图像1");
	//Mat mtx = cv::cvarrToMat(iplImage);
	bitwise_not(result1, result2);

	//IplImage* iplImage2 = new IplImage(result2.clone());
	//IplImage* iplImage1 = NULL;

	IplConvKernel* kernel = cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_RECT);

	//

	//cvErode(iplImage2, iplImage2, kernel, 1);
	cvMorphologyEx(iplImage, iplImage, NULL, kernel, CV_MOP_OPEN, 1);
	cvErode(iplImage, iplImage, kernel, 1);
	//cvMorphologyEx(iplImage2, iplImage2, NULL, kernel, CV_MOP_CLOSE, 2);

	//dilate(result2, result2, Mat(kernel), Point(-1, -1), 5, 0, morphologyDefaultBorderValue());
	//erode(result2, result3, kernel, Point(-1, -1), 5, 0, morphologyDefaultBorderValue());

	cvReleaseStructuringElement(&kernel);

	imshow(name + "二值化图像1", cv::cvarrToMat(iplImage));

	std::string outFileName = path + "\\c\\" + name;
	//imwrite(outFileName, cv::cvarrToMat(iplImage));
	cout << outFileName << endl;
	return 0;
}
Mat applyLookUp(const cv::Mat& image, const cv::Mat& lookup) {
	Mat result;
	cv::LUT(image, lookup, result);
	return result;
}
void LutFunc(Mat &I)
{
	Mat lut(1, 256, CV_8U);
	for (int i = 0; i < 256; i++) {
		lut.at<uchar>(i) = 255 - i;
	}
	I = applyLookUp(I, lut);
}

void AtFunc(Mat &I)
{
	for (int i = 0; i < I.rows; ++i)
	for (int j = 0; j < I.cols; ++j)
		I.at<uchar>(i, j) = 255 - I.at<uchar>(i, j);
}

void PtrFunc(Mat &I)
{
	for (int i = 0; i < I.rows; ++i)
	{
		uchar *p = I.ptr<uchar>(i);
		for (int j = 0; j < I.cols; ++j)
		{
			p[j] = 255 - p[j];
		}
	}
}
void  IteratorFunc(Mat &I)
{
	MatIterator_<uchar> it, end;
	for (it = I.begin<uchar>(), end = I.end<uchar>(); it != end; ++it)
		*it = 255 - *it;
}
//-------------------- -
//作者：menghuanxiy
//来源：CSDN
//原文：https ://blog.csdn.net/menghuanxiy/article/details/79837108 
//版权声明：本文为博主原创文章，转载请附上博文链接！



//k-mean method to find a threshold by itself
void Binarization(IplImage* iplImage)
{
	int i, j, nWidth, nHeight;
	int nBack_count, nData_count;
	int nBack_sum, nData_sum;
	uchar ucThre, ucThre_new;
	int nValue;

	nWidth = iplImage->width;
	nHeight = iplImage->height;
	//initial the threshold
	ucThre = 0;
	ucThre_new = 127;

	std::cout << "Initial Threshold is :" << (int)ucThre_new << std::endl;

	std::cout << "***********************************" << std::endl;
	CvScalar s;
	s = cvGet2D(iplImage, 0, 0); // get the (i,j) pixel value
	cout << nWidth << "--0-" << s.val[0] << endl;
	cout << nWidth << "--1-" << s.val[1] << endl;
	cout << nWidth << "--2-" << s.val[2] << endl;
	cout << nWidth << "--3-" << s.val[3] << endl;
	//printf("intensity=%f\n", s.val[0]);
	//s.val[0] = 111;
	//cvSet2D(img, i, j, s);

	while (ucThre != ucThre_new) {
		nBack_sum = nData_sum = 0;
		nBack_count = nData_count = 0;

		for (j = 0; j < nHeight; ++j)
		{
			for (i = 0; i < nWidth; ++i)
			{

				s = cvGet2D(iplImage, j, i);
				nValue = s.val[0];
				if (nValue > ucThre_new)
				{
					nBack_sum += nValue;
					nBack_count++;
				}
				else {
					nData_sum += nValue;
					nData_count++;
				}
			}
		}// end for i

		nBack_sum = nBack_sum / nBack_count;
		nData_sum = nData_sum / nData_count;
		ucThre = ucThre_new;
		ucThre_new = (nBack_sum + nData_sum) / 2;
	}// end while

	std::cout << "After Binarization threshold is :" << (int)ucThre_new << std::endl;

	int nBlack = 0;
	int nWhite = 0;

	for (j = 0; j < nHeight; ++j)
	{
		for (i = 0; i < nWidth; ++i)
		{
			s = cvGet2D(iplImage, j, i);
			nValue = s.val[0];
			if (nValue > ucThre_new)
			{
				s.val[0] = (uchar)WHITE;
				//s.val[1] = (uchar)WHITE;
				//s.val[2] = (uchar)WHITE;
				cvSet2D(iplImage, j, i, s);
				//setPixel(i, j, (uchar)WHITE);
				nWhite++;
			}
			else {
				s.val[0] = (uchar)BLACK;
				//s.val[1] = (uchar)BLACK;
				//s.val[2] = (uchar)BLACK;
				cvSet2D(iplImage, j, i, s);
				//setPixel(i, j, BLACK);
				nBlack++;
			}
		}
	}

	//backgroud is black,swap black and white
	/*if (nBlack > nWhite)
	{
	for (i = 0; i < nHeight; ++i)
	for (j = 0; j < nWidth; ++j)
	{
	s = cvGet2D(iplImage, i, j);
	nValue = s.val[0];
	if (!nValue){
	s.val[0] = (uchar)WHITE;
	cvSet2D(iplImage, i, j, s);
	}else{
	s.val[0] = (uchar)BLACK;
	cvSet2D(iplImage, i, j, s);
	}
	}
	}
	*/
	std::cout << "Binarization finished!" << std::endl;

}

Mat toGray(Mat image){
	Mat iGray;
	cvtColor(image, iGray, COLOR_BGR2GRAY);
	return iGray;
}

void setPixel(Mat image, int x, int y, uchar pValue){
	*(image.data + y*image.step[0] + x) = pValue;
}

*/
