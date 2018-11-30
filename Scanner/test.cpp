#include <opencv2/core/core.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <opencv2/imgproc/types_c.h>

#include <windows.h>
#include <iostream>
#include <string>
#include <Image.h>
#include <vector>

#include <Scanner.h>

using namespace std;
using namespace cv;
using namespace scanner;


static const int BLACK = 0;
static const int WHITE = 255;


void setPixel(Mat image, int x, int y, uchar pValue);
int getPixel(Mat image, int x, int y);
Mat toGray(Mat image);
Mat NaiveRemoveNoise(Mat image, int pNum);
int handlerImage(std::string path,std::string name);
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
			cout << strPath + "\\"+findData.cFileName << endl;
			handlerImage(strPath, findData.cFileName);
			return;

		}
	} while (FindNextFileA(hFindFine, &findData));
	//关闭文件搜索句柄
	FindClose(hFindFine);
}

void test(){
	IplImage* src;
	string strFilePath = "C:\\Users\\jiaojian\\Desktop\\耳标图片\\111.jpg";
	// 第一条命令行参数确定了图像的文件名。
	Mat image = imread(strFilePath);
	Mat bb = toGray(image);
	src =new IplImage(bb);
	
	
		IplImage* dst = cvCreateImage(cvGetSize(src), 8, 3);
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* contour = 0;
		cvThreshold(src, src, 1, 255, CV_THRESH_BINARY);
		cvNamedWindow("Source", 1);
		cvShowImage("Source", src);
		cvFindContours(src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP,
			CV_CHAIN_APPROX_SIMPLE);
		cvZero(dst);
		for (; contour != 0; contour = contour->h_next)
		{
			CvScalar color = CV_RGB(rand() & 255, rand() & 255, rand() & 255);
			/* 用1替代 CV_FILLED 所指示的轮廓外形 */
			cvDrawContours(dst, contour, color, color, -1, CV_FILLED, 8);
		}
		cvNamedWindow("Components", 1);
		cvShowImage("Components", dst);
		cvWaitKey(0);
	
}

int read(IplImage* image){
	CvSeq* contour = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
	
	contour = cvApproxPoly(contour, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 3, 1);
	int a = cvFindContours(image, storage, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CvSeq** 


	CvTreeNodeIterator iterator;
	cvInitTreeNodeIterator(&iterator, contour, 3);
	//把所有轮廓的点收集起来 
	CvSeq* allpointsSeq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage);
	while (0 != (contour = (CvSeq*)cvNextTreeNode(&iterator)))
	{
		//找到一个轮廓就可以用for循环提取里面的点了 ） 这个方法不推荐 --zhengjw 2013/1/14
		// 推荐 mode = CV_RETR_LIST; contours_num=cvFindContours(preimg, storage, &contours, sizeof(CvContour), mode, CV_CHAIN_APPROX_NONE, cvPoint(0,0)); //-- contours_num 表示的是一共有多少条轮廓线for (;contours!=0;contours=contours->h_next) //-- 指向下一个轮廓序列{ 这里遍历CvSeq里面的元素的方法很怪异 
		int onetourlength = contour->total;
		//给点数组分配空间，记得释放 
		CvPoint *points = (CvPoint *)malloc(sizeof(CvPoint)* onetourlength);
		//printf("seqlength:%dn",seqlength); 
		CvSeqReader reader; //-- 读其中一个轮廓序列
		CvPoint pt = cvPoint(0, 0);
		cvStartReadSeq(contour, &reader); //开始提取 
		for (int i = 0; i < onetourlength; i++){
			CV_READ_SEQ_ELEM(pt, reader); //--读其中一个序列中的一个元素点
			points[i] = pt;
			cvSeqPush(allpointsSeq, &pt);
		} //把这个轮廓点找出后，就可以用这些点画个封闭线 
		cvPolyLine(image, &points, &onetourlength, 1, 0, CV_RGB(0, 255, 0), 2, 8, 0);
	} //-- zhengjw 2013/1/14//刚刚已经画出了找出的每个轮廓，还收集了所有轮廓点， 
	//因此还可以将这些点用一个围线包围起来，即把所有轮廓包围起来 
	//这里要用到新的函数 
	CvSeq* hull;
	hull = cvConvexHull2(allpointsSeq, 0, CV_CLOCKWISE, 0);


	//IplImage pTemp(dd);
	//CvMemStorage* storage = cvCreateMemStorage(0);
	//point_seq = cvCreateSeq(CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
	//int a = cvFindContours(&pTemp, storage, &point_seq, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CvSeq** 
	//storage->block_size;
	////
	//cout << a << "--" << storage->block_size << endl;

	//// 直接使用CONTOUR中的矩形来画轮廓
	//for (; point_seq; point_seq = point_seq->h_next)
	//{
	//	CvRect r = ((CvContour*)point_seq)->rect;
	//	point_seq->total;
	//	//cvStartReadSeq();
	//	CvScalar color = CV_RGB(rand() & 255, rand() & 255, rand() & 255);
	//	/* 用1替代 CV_FILLED 所指示的轮廓外形 */
	//	cvDrawContours(&result, point_seq, color, color, -1, CV_FILLED, 8);
	//	CvScalar color1 = CV_RGB(rand() & 255, rand() & 255, rand() & 0);
	//	if (r.height * r.width > 5000) // 面积小的方形抛弃掉 CONTOUR_MAX_AERA
	//	{
	//		cvRectangle(&result, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 2);
	//		//cvRectangle(&pTemp, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 1, CV_AA, 0);

	//		cout << r.x << "--" << r.y << "--" << r.height * r.width << endl;
	//	}
	//}
	//Mat mtx = cv::cvarrToMat(&result);
	////cvSaveImage("c:\\img.jpg", &pTemp);

	//namedWindow("灰色");
	//imshow("灰色", mtx);

	return 0;
}
Mat resizeImage(Mat srcBitmap) {
	int resizeThreshold = 500;
	int width = srcBitmap.cols;
	int height = srcBitmap.rows;
	int maxSize = width > height ? width : height;
	if (maxSize > resizeThreshold) {
		int resizeScale = 1.0f * maxSize / resizeThreshold;
		width = static_cast<int>(width / resizeScale);
		height = static_cast<int>(height / resizeScale);
		Size size(width, height);
		Mat resizedBitmap(size, CV_8UC3);
		resize(srcBitmap, resizedBitmap, size);
		return resizedBitmap;
	}
	return srcBitmap;
}
bool isHisEqual = false;
Mat preprocessedImage(Mat &image, int cannyValue, int blurValue) {
	Mat grayMat;
	Mat thresholdMat;
	cvtColor(image, grayMat, CV_BGR2GRAY);
	//threshold(cannyMat, thresholdMat, 0, 255, CV_THRESH_BINARY);
	adaptiveThreshold(grayMat, thresholdMat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 25, 5.0);

	if (isHisEqual){
		equalizeHist(thresholdMat, thresholdMat);
	}
	Mat blurMat;
	GaussianBlur(thresholdMat, blurMat, Size(blurValue, blurValue), 0);
	Mat cannyMat;
	Canny(blurMat, cannyMat, 50, cannyValue, 3);
	return grayMat;
}

void teste(Mat srcBitmap){
	vector<Point> result;
	int cannyValue[] = { 100, 150, 300 };
	int blurValue[] = { 3, 7, 11, 15 };
	//缩小图片尺寸
	Mat image = resizeImage(srcBitmap);
	/*for (int i = 0; i < 3; i++){
		for (int j = 0; j < 4; j++){
			std::cout << i << "---" << j << std::endl;*/
			//预处理图片
			Mat scanImage = preprocessedImage(image, cannyValue[0], blurValue[0]);
			vector<vector<Point>> contours(10000000);
			vector<Vec4i> hierarchy;
			Mat gg;
			//提取边框
		
			//findContours(scanImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
			findContours(scanImage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
			//return contours;
		//按面积排序
		//std::sort(contours.begin(), contours.end(), sortByArea);
		//if (contours.size() > 0) {
		//	vector<Point> contour = contours[0];
		//	double arc = arcLength(contour, true);
		//	vector<Point> outDP;
		//	//多变形逼近
		//	approxPolyDP(Mat(contour), outDP, 0.01 * arc, true);
		//	//筛选去除相近的点
		//	vector<Point> selectedPoints = selectPoints(outDP);
		//	/*if (selectedPoints.size() != 4) {*/
		//		//如果筛选出来之后不是四边形
		//		//continue;
		//		return selectedPoints;
		//}
		//else {
		//	int widthMin = selectedPoints[0].x;
		//	int widthMax = selectedPoints[0].x;
		//	int heightMin = selectedPoints[0].y;
		//	int heightMax = selectedPoints[0].y;
		//	for (int k = 0; k < 4; k++) {
		//		if (selectedPoints[k].x < widthMin) {
		//			widthMin = selectedPoints[k].x;
		//		}
		//		if (selectedPoints[k].x > widthMax) {
		//			widthMax = selectedPoints[k].x;
		//		}
		//		if (selectedPoints[k].y < heightMin) {
		//			heightMin = selectedPoints[k].y;
		//		}
		//		if (selectedPoints[k].y > heightMax) {
		//			heightMax = selectedPoints[k].y;
		//		}
		//	}
		//	//选择区域外围矩形面积
		//	int selectArea = (widthMax - widthMin) * (heightMax - heightMin);
		//	int imageArea = scanImage.cols * scanImage.rows;
		//	if (selectArea < (imageArea / 20)) {
		//		result.clear();
		//		//筛选出来的区域太小
		//		//continue;
		//	}
		//	else {
		//		result = selectedPoints;
		//		if (result.size() != 4) {
		//			Point2f p[4];
		//			p[0] = Point2f(0, 0);
		//			p[1] = Point2f(image.cols, 0);
		//			p[2] = Point2f(image.cols, image.rows);
		//			p[3] = Point2f(0, image.rows);
		//			result.push_back(p[0]);
		//			result.push_back(p[1]);
		//			result.push_back(p[2]);
		//			result.push_back(p[3]);
		//		}
		//		for (Point &p : result) {
		//			p.x *= resizeScale;
		//			p.y *= resizeScale;
		//		}
		//		// 按左上，右上，右下，左下排序
		//		vector<Point> resultPoints = sortPointClockwise(result);
		//		return resultPoints;
		//	}
		//}
		//}
		/*std::cout << i << "---" << j << std::endl;
		}
		}*/
		//当没选出所需要区域时，如果还没做过直方图均衡化则尝试使用均衡化，但该操作只执行一次，若还无效，则判定为图片不能裁出有效区域，返回整张图
		//if (!isHisEqual){
		//	isHisEqual = true;
		//	return scanPoint();
		//}
		//if (result.size() != 4) {
		//	Point2f p[4];
		//	p[0] = Point2f(0, 0);
		//	p[1] = Point2f(image.cols, 0);
		//	p[2] = Point2f(image.cols, image.rows);
		//	p[3] = Point2f(0, image.rows);
		//	result.push_back(p[0]);
		//	result.push_back(p[1]);
		//	result.push_back(p[2]);
		//	result.push_back(p[3]);
		//}
		//for (Point &p : result) {
		//	p.x *= resizeScale;
		//	p.y *= resizeScale;
		//}
		//// 按左上，右上，右下，左下排序
		//return sortPointClockwise(result);
	

	
}

int main(int argc, char* argv[])
{
	
	/*for (int i = 0; i < 3; i++){
		for (int j = 0; j < 4; j++){

			cout << i << "---" << j << endl;
			if (i == 2 && j == 2){
				continue;
			}
		}
	}*/
	//搜索D盘下Test文件夹下的所有文件
	string strFilePath = "C:\\Users\\jiaojian\\Desktop\\耳标图片\\111.jpg";
	Mat img = imread(strFilePath);

	//teste(img);
	IplImage result(img);
	try{
		//test();
		Mat temp;
		//cvtColor(img, temp, COLOR_BGR2GRAY);
		//cout << "img " << img.channels() << "-temp " << temp.channels() << endl;
		Scanner docScanner(img);
		//int cannyValue[] = { 100, 150, 300 };
		//int blurValue[] = { 3, 7, 11, 15 };
		//CvSeq* point_seq = 0;
		////for (int i = 0; i < 3; i++){
		//	//for (int j = 0; j < 4; j++){
		//		Mat dd = docScanner.preprocessedImage(img, cannyValue[2], blurValue[3]);
				/*vector<Mat> contours;
				vector<vector<Point>> contours0;
				vector<Vec4i> hierarchy;
				contours0.clear();
				findContours(dd, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
				contours0.clear();
				findContours(dd, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);*/
				////提取边框 RETR_TREE, CHAIN_APPROX_SIMPLE
				// //findContours(dd, contours1, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
				//
				////break;

				//namedWindow("灰色");
				//imshow("灰色", dd);
			//}
		//}
		vector<Point> scanPoints;
		scanPoints = docScanner.scanPoint();
		if (scanPoints.size() == 4) {
			for (int i = 0; i < 4; ++i) {
				cout << scanPoints[i] << endl;
			}
		}
		cvRectangle(&result, cvPoint(scanPoints[0].x, scanPoints[0].y), cvPoint(scanPoints[2].x, scanPoints[2].y), CV_RGB(255, 0, 0), 1, CV_AA, 0);

		namedWindow("灰色");
		imshow("灰色", cv::cvarrToMat(&result));
	}
	catch (cv::Exception &e){
		cout << "aaa " << e.err << e.msg << endl;
	}
	
	try{

		string strFilePath = "C:\\Users\\jiaojian\\Desktop\\耳标图片";
		//FindFile(strFilePath);
		//cv::waitKey(50000);

		//string strFilePath = "C:\\Users\\jiaojian\\Desktop\\ttt.jpg";

		//Mat mat = imread(strFilePath);
		// mat = toGray(mat);
		//IplImage* img = new IplImage(mat);

		///*cvAdaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C,
		//	CV_THRESH_BINARY, 17, 5);*/

		//cvThreshold(img, img, 100, 255, CV_THRESH_OTSU);

		//namedWindow("灰色");
		//imshow("灰色", cv::cvarrToMat(img));
		
	}
	catch (cv::Exception &e){
		cout << e.err << e.msg << endl;
	}
	cv::waitKey(0);
	/*
	std::vector<int> vec1;
	vec1.push_back(1);
	vec1.push_back(2);
	
	std::vector<int> vec2(vec1);
	std::vector<int>::iterator it;
	it = vec2.begin();
	cout <<"it = "<< &it << endl;
	vec2.insert(it, 5);
	it = vec2.begin();
	cout << "it = " <<&it << endl;
	cout << "it = " << &*(&it) << vec2[0]<< endl;
	vec2.insert(it, 2, 6);
	
	std::vector<int> vec3(vec2);
	vec3.assign(3, 5);
	
	for (int i = 0; i<vec3.size(); ++i){
		cout << vec3[i] << endl;
	}*/

	system("pause");
	return 0;
}


int handlerImage(std::string path, std::string name){
	Mat image, gray;
	image = imread(path + "\\" + name);
	if (!image.data)
		return -1;
	//cvtColor(image, gray, CV_BGR2GRAY);

	Mat result, result1, result2, result3;
	
	Mat image1 = toGray(image);
	IplImage* iplImage = new IplImage(image1);

	cvAdaptiveThreshold(iplImage, iplImage, 255, CV_ADAPTIVE_THRESH_MEAN_C, 1, 25, 5);

	IplConvKernel* kernel = cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_RECT);

	//cvErode(iplImage2, iplImage2, kernel, 1);
	//cvMorphologyEx(iplImage, iplImage, kernel, kernel, CV_MOP_OPEN, 1);
	//cvErode(iplImage2, iplImage2, kernel, 1);
	cvMorphologyEx(iplImage, iplImage, NULL, kernel, CV_MOP_CLOSE, 1);
	cvDilate(iplImage, iplImage, kernel, 1);

	cvReleaseStructuringElement(&kernel);


	Mat aa = cvarrToMat(iplImage);
	//bitwise_not(aa, result2);
	//Mat dd = NaiveRemoveNoise(result2, 1);



	Image img = Image(aa);

	Mat temp1 = aa.clone();

	//img.toGray();
	//img.Binarization();
	img.NaiveRemoveNoise(1);
	img.ContoursRemoveNoise( 10);
	//img.FloodFillDivide(data, 10, argv[2], 0);

	//namedWindow(name + "灰色");
	//imshow(name + "灰色", temp1);

	//img.ShowInWindow("aa");
	//cvNamedWindow((name + "灰色").c_str());
	//cvShowImage((name + "灰色").c_str(), iplImage);


	//Mat mtx = cv::cvarrToMat(iplImage);
	//bitwise_not(result1, result2);

	
	
	



	std::string outFileName = path + "\\d\\" + name;
	//imwrite(outFileName, dd);
	cout << outFileName << endl;
	return 0;
}



Mat toGray(Mat image){
	Mat iGray;
	cvtColor(image, iGray, COLOR_BGR2GRAY);
	return iGray;
}

void setPixel(Mat image,int x, int y, uchar pValue){
	*(image.data + y*image.step[0] + x) = pValue;
}

int getPixel(Mat image, int x, int y){
	return *(image.data + y*image.step[0] + x);
}
//s = cvGet2D(iplImage, j, i);
//nValue = s.val[0];

Mat NaiveRemoveNoise(Mat image, int pNum)
{
	//naive remove noise
	int i, j, m, n, nValue, nCount;
	int nWidth = image.cols;
	int nHeight = image.rows;
	//set boundry to be white
	for (i = 0; i < nWidth; ++i)
	{
		setPixel(image,i, 0, WHITE);
		setPixel(image,i, nHeight - 1, WHITE);
	}
	for (i = 0; i < nHeight; ++i)
	{
		setPixel(image,0, i, WHITE);
		setPixel(image,nWidth - 1, i, WHITE);
	}
	//if the neighbor of a point is white but it is black, delete it
	for (j = 1; j < nHeight; ++j)
	for (i = 1; i < nWidth; ++i)
	{
		nValue = getPixel(image,i, j);
		if (!nValue)
		{
			nCount = 0;
			for (m = i - 1; m <= i + 1; ++m)
			for (n = j - 1; n <= j + 1; ++n)
			{
				if (!getPixel(image, m, n))
					nCount++;
			}
			if (nCount <= pNum)
				setPixel(image,i, j, WHITE);
		}
		else
		{
			nCount = 0;
			for (m = i - 1; m <= i + 1; ++m)
			for (n = j - 1; n <= j + 1; ++n)
			{
				if (!getPixel(image, m, n))
					nCount++;
			}
			if (nCount >= 7)
				setPixel(image,i, j, BLACK);
		}
	}
	return image;
}


