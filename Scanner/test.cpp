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
#include <math.h>
#include <opencv2/calib3d.hpp>
using namespace std;
using namespace cv;
using namespace scanner;
using std::max;

static const int BLACK = 0;
static const int WHITE = 255;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

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

vector<vector<Point>> getPointLists(Mat image){
	IplImage pTemp(image);
	CvSeq* contour = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
	int a = cvFindContours(&pTemp, storage, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CvSeq** 
	CvSeq* allpointsSeq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage);
	vector<vector<Point>> pointLists;
	//// 直接使用CONTOUR中的矩形来画轮廓
	for (; contour; contour = contour->h_next)
	{
		int onetourlength = contour->total;
		//给点数组分配空间，记得释放 
		CvPoint *points = (CvPoint *)malloc(sizeof(CvPoint)* onetourlength);
		//printf("seqlength:%dn",seqlength); 
		CvSeqReader reader; //-- 读其中一个轮廓序列
		CvPoint pt = cvPoint(0, 0);
		cvStartReadSeq(contour, &reader); //开始提取 
		vector<Point> pointss;
		for (int i = 0; i < onetourlength; i++){
			CV_READ_SEQ_ELEM(pt, reader); //--读其中一个序列中的一个元素点
			pointss.push_back(Point(pt.x, pt.y));
			cvSeqPush(allpointsSeq, &pt);
		} //把这个轮廓点找出后，就可以用这些点画个封闭线 
		pointLists.push_back(pointss);
	}
	return pointLists;
}


int readImage(Mat image,Mat imageMat){
	CvSeq* contour = 0;

	namedWindow("灰色1");
	imshow("灰色1", imageMat);
	CvMemStorage* storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
	
	//contour = cvApproxPoly(contour, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 3, 1);
	//int a = cvFindContours(image, storage, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CvSeq** 


	//CvTreeNodeIterator iterator;
	//cvInitTreeNodeIterator(&iterator, contour, 3);
	////把所有轮廓的点收集起来 
	//CvSeq* allpointsSeq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage);
	//while (0 != (contour = (CvSeq*)cvNextTreeNode(&iterator)))
	//{
	//	//找到一个轮廓就可以用for循环提取里面的点了 ） 这个方法不推荐 --zhengjw 2013/1/14
	//	// 推荐 mode = CV_RETR_LIST; contours_num=cvFindContours(preimg, storage, &contours, sizeof(CvContour), mode, CV_CHAIN_APPROX_NONE, cvPoint(0,0)); //-- contours_num 表示的是一共有多少条轮廓线for (;contours!=0;contours=contours->h_next) //-- 指向下一个轮廓序列{ 这里遍历CvSeq里面的元素的方法很怪异 
	//	int onetourlength = contour->total;
	//	//给点数组分配空间，记得释放 
	//	CvPoint *points = (CvPoint *)malloc(sizeof(CvPoint)* onetourlength);
	//	//printf("seqlength:%dn",seqlength); 
	//	CvSeqReader reader; //-- 读其中一个轮廓序列
	//	CvPoint pt = cvPoint(0, 0);
	//	cvStartReadSeq(contour, &reader); //开始提取 
	//	for (int i = 0; i < onetourlength; i++){
	//		CV_READ_SEQ_ELEM(pt, reader); //--读其中一个序列中的一个元素点
	//		points[i] = pt;
	//		cvSeqPush(allpointsSeq, &pt);
	//	} //把这个轮廓点找出后，就可以用这些点画个封闭线 
	//	cvPolyLine(image, &points, &onetourlength, 1, 0, CV_RGB(0, 255, 0), 2, 8, 0);
	//} //-- zhengjw 2013/1/14//刚刚已经画出了找出的每个轮廓，还收集了所有轮廓点， 
	////因此还可以将这些点用一个围线包围起来，即把所有轮廓包围起来 
	////这里要用到新的函数 
	//CvSeq* hull;
	//hull = cvConvexHull2(allpointsSeq, 0, CV_CLOCKWISE, 0);


	IplImage pTemp(imageMat);
	IplImage* result = new IplImage(image);
	//CvMemStorage* storage = cvCreateMemStorage(0);
	//contour = cvCreateSeq(CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
	int a = cvFindContours(&pTemp, storage, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CvSeq** 
	//storage->block_size;
	cout << a << "--" << storage->block_size << endl;
	CvSeq* allpointsSeq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage);
	vector<vector<Point>> pointLists;
	//// 直接使用CONTOUR中的矩形来画轮廓
	for (; contour; contour = contour->h_next)
	{
		int onetourlength = contour->total;
		//给点数组分配空间，记得释放 
		CvPoint *points = (CvPoint *)malloc(sizeof(CvPoint)* onetourlength);
		//printf("seqlength:%dn",seqlength); 
		CvSeqReader reader; //-- 读其中一个轮廓序列
		CvPoint pt = cvPoint(0, 0);
		cvStartReadSeq(contour, &reader); //开始提取 
		vector<Point> pointss;
		for (int i = 0; i < onetourlength; i++){
			CV_READ_SEQ_ELEM(pt, reader); //--读其中一个序列中的一个元素点
			points[i] = pt;
			Point point(pt.x, pt.y);
			pointss.push_back(point);
			cvSeqPush(allpointsSeq, &pt);
		} //把这个轮廓点找出后，就可以用这些点画个封闭线 
		cvPolyLine(result, &points, &onetourlength, 1, 0, CV_RGB(0, 255, 0), 2, 8, 0);
		pointLists.push_back(pointss);
		//CvRect r = ((CvContour*)contour)->rect;
		////cout << "total--" << contour->total << endl;
		////cvStartReadSeq();
		//CvScalar color = CV_RGB(rand() & 255, rand() & 255, rand() & 255);
		///* 用1替代 CV_FILLED 所指示的轮廓外形 */
		//cvDrawContours(result, contour, color, color, -1, CV_FILLED, 8);//
		//if (r.height * r.width > 5000) // 面积小的方形抛弃掉 CONTOUR_MAX_AERA
		//{
		//	//cvRectangle(result, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 2);
		//	//cvRectangle(&pTemp, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(255, 0, 0), 1, CV_AA, 0);
		//	cout << r.x << "--" << r.y << "--" << r.height * r.width << endl;
		//}
	}
	Mat mtx = cv::cvarrToMat(result);
	//cvSaveImage("c:\\img.jpg", &pTemp);

	namedWindow("灰色");
	imshow("灰色", mtx);

	return 0;
}



const int w = 500;
int blurValue = 7;
int cannyValue = 20;

Mat temp;
static void on_trackbar1(int, void*)
{
	Mat cannyMat;
	int _blurValue = blurValue + 2;
	if (_blurValue < 1){
	_blurValue = 1;
	}
	if (_blurValue %2 == 0){
	_blurValue +=1;
	}

	cout << _blurValue << endl;
	GaussianBlur(temp, cannyMat, Size(_blurValue, _blurValue), 0);

	/*Mat cannyMat;
	Canny(blurMat, cannyMat, 50, 50, 3);
	int _blurValue = cannyValue * 10;
	Mat blurMat;
	GaussianBlur(temp, blurMat, Size(7, 7), 0);
	cout << _blurValue << endl;
	Mat cannyMat;
	Canny(blurMat, cannyMat, 50, _blurValue, 3);*/


	imshow("contours", cannyMat);
}

static void on_trackbar(int, void*)
{
	/*int _blurValue = blurValue + 2;
	if (_blurValue < 3){
		_blurValue = 3;
	}
	if (_blurValue %2 == 0){
		_blurValue +=1;
	}
	
	cout << _blurValue << endl;
	GaussianBlur(temp, blurMat, Size(_blurValue, _blurValue), 0);

	Mat cannyMat;
	Canny(blurMat, cannyMat, 50, 50, 3);*/
	int _blurValue = cannyValue * 10;
	Mat blurMat;
	GaussianBlur(temp, blurMat, Size(7, 7), 0);
	cout << _blurValue << endl;
	Mat cannyMat;
	Canny(blurMat, cannyMat, 50, _blurValue, 3);


	imshow("contours", cannyMat);
}

float distancePoint(Point2f a, Point2f b)
{
	float y = a.y - b.y;
	float x = a.x - b.x;
	return sqrt(pow(x, 2) + pow(y, 2));
}

void getPoint2fTris(Point2f centorPoint, Rect2f srcRect, vector<Point> scanPoints, Point2f* srcTri, int type = 0){
	for (int index = 0; index < scanPoints.size(); index++){
		float half = srcRect.height / 2;
		float xSub = (centorPoint.x - scanPoints[index].x);
		float ySub = (centorPoint.y - scanPoints[index].y);
		float x,y;
		if (index == 0){
			half = srcRect.height / 2;
			x = xSub > 0 ? centorPoint.x - abs(half / ySub * xSub) : centorPoint.x + abs(half / ySub * xSub);
			y = srcRect.y;
		}else if(index == 1){
			half = srcRect.width / 2;
			y =  ySub > 0 ? centorPoint.y - abs(half / xSub * ySub) : centorPoint.y + abs(half / xSub * ySub);
			x = srcRect.x + srcRect.width;
		}
		else if (index == 2){
			half = srcRect.height / 2;
			x = xSub > 0 ? centorPoint.x - abs(half / ySub * xSub) : centorPoint.x + abs(half / ySub * xSub);
			y = srcRect.y + srcRect.width;
		}
		else if (index == 3){
			half = srcRect.width / 2;
			y = ySub > 0 ? centorPoint.y - abs(half / xSub * ySub) : centorPoint.y + abs(half / xSub * ySub);
			x = srcRect.x;
		}
		if (type == 1){
			x = x - srcRect.x;
			y = y - srcRect.y;
		}
		srcTri[index] = Point2f(x, y);
	}
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

	
	
	try{
		Scanner docScanner(img);
		/*cvtColor(img.clone(), temp, COLOR_BGR2GRAY);
		Mat blurMat;
		GaussianBlur(temp, blurMat, Size(blurValue, blurValue), 0);

		namedWindow("contours", 1);
		createTrackbar("levels+3", "contours", &blurValue, 20, on_trackbar1);

		namedWindow("contourscanny", 1);
		createTrackbar("levels+3", "contourscanny", &cannyValue, 20, on_trackbar);
		on_trackbar(0, 0);
		on_trackbar1(0, 0);*/

		//Mat image = docScanner.resizeImage();
		//Mat img1 = docScanner.preprocessedImage(image, 50, 9);
		////IplImage result(img1);
		/////readImage(image, img1);


		//namedWindow("bina");
		//imshow("bina", img1);
		//imshow("bina", cv::cvarrToMat(&result));


		//docScanner.ShowInWindow("aaaa");
		//cout << "img " << img.channels() << "-temp " << temp.channels() << endl;
		
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
		float padding = 200;
		vector<Point> scanPoints;
		scanPoints = docScanner.scanPoint();
		if (scanPoints.size() == 4) {
			for (int i = 0; i < 4; ++i) {
				cout << scanPoints[i] << endl;
			}
		}

		IplImage result = IplImage(img);
		CvScalar color = CV_RGB(255, 0, 0);
		//cvRectangle(&result, cvPoint(scanPoints[0].x, scanPoints[0].y), cvPoint(scanPoints[2].x, scanPoints[2].y), CV_RGB(255, 0, 0));
		cvLine(&result, cvPoint(scanPoints[0].x, scanPoints[0].y), cvPoint(scanPoints[1].x, scanPoints[1].y), color);
		cvLine(&result, cvPoint(scanPoints[1].x, scanPoints[1].y), cvPoint(scanPoints[2].x, scanPoints[2].y), color);
		cvLine(&result, cvPoint(scanPoints[2].x, scanPoints[2].y), cvPoint(scanPoints[3].x, scanPoints[3].y), color);
		cvLine(&result, cvPoint(scanPoints[3].x, scanPoints[3].y), cvPoint(scanPoints[0].x, scanPoints[0].y), color);
		
		int xMin = scanPoints[0].x < scanPoints[3].x ? scanPoints[0].x : scanPoints[3].x ;
		int yMin = scanPoints[0].y < scanPoints[1].y ? scanPoints[0].y : scanPoints[1].y ;
		int xMax = scanPoints[1].x > scanPoints[2].x ? scanPoints[1].x : scanPoints[2].x ;
		int yMax = scanPoints[2].y > scanPoints[3].y ? scanPoints[2].y : scanPoints[3].y ;

		float width = xMax - xMin;
		float height = yMax - yMin;
		float rectWidth = (xMax - xMin) > (yMax - yMin) ? (xMax - xMin) : (yMax - yMin);
		CvRect rect(xMin, yMin, (xMax - xMin), (yMax - yMin));
		float scale = 2;
		//float padding = ((xMax - xMin) / scale);
		CvRect srcRect(xMin - padding, yMin - padding, (xMax - xMin) + padding *2, (yMax - yMin) + padding*2);

		cvRectangle(&result, cvPoint(srcRect.x, srcRect.y), cvPoint(srcRect.width + srcRect.x, srcRect.y + srcRect.height), CV_RGB(0, 255, 0));

		Point2f centorPoint = Point2f(rect.x + rect.width / 2, rect.y + rect.height / 2);
		cvCircle(&result, cvPoint(centorPoint.x, centorPoint.y), 5, cvScalar(0, 255, 0), 1);
		//cvCircle(&result, cvPoint(centorPoint.x - 47, centorPoint.y - 175), 5, cvScalar(0, 255, 0), 1);
		//cvCircle(&result, cvPoint(centorPoint.x - 60.4, srcRect.y), 5, cvScalar(0, 255, 0), 1);
		//--------------------------------
		float x = scanPoints[0].x;
		float y = scanPoints[0].y;

		cout << "point x = " << x << " y = " << y << endl;

		cout << "centorPoint x = " << centorPoint.x << " y = " << centorPoint.y << endl;
		cout << "srcRect x = " << srcRect.width << " y = " << srcRect.height << endl;


		//Point2f point1(centorPoint.x - (srcRect.height / 2 / (centorPoint.y - scanPoints[0].y) * (centorPoint.x - scanPoints[0].x)), srcRect.y);
		//cvCircle(&result, cvPoint(point1.x, point1.y), 5, cvScalar(0, 255, 0), 1);



		/*Point2f p0 = getPoint(0, centorPoint, srcRect, scanPoints);
		cvCircle(&result, cvPoint(p0.x, p0.y), 5, cvScalar(0, 255, 0), 1);
		Point2f p1 = getPoint(1, centorPoint, srcRect, scanPoints);
		cvCircle(&result, cvPoint(p1.x, p1.y), 5, cvScalar(0, 255, 0), 1);
		Point2f p2 = getPoint(2, centorPoint, srcRect, scanPoints);
		cvCircle(&result, cvPoint(p2.x, p2.y), 5, cvScalar(0, 255, 0), 1);
		Point2f p3 = getPoint(3, centorPoint, srcRect, scanPoints);
		cvCircle(&result, cvPoint(p3.x, p3.y), 5, cvScalar(0, 255, 0), 1);*/
		//---------------------------------


		cout << "rect  x= " << rect.x << " y = "<< rect.y <<" width = "<< rect.width << endl;


		
		cvSetImageROI(&result, srcRect);

		namedWindow("灰色1");
		//imshow("灰色1", warp_dst);
		imshow("灰色1",  cv::cvarrToMat(&result));
		
		Point2f srcTri[4];
		Point2f dstTri[4];
		Mat src, warp_dst, warp_rotate_dst, warp_mat;
		src = cv::cvarrToMat(&result);
		///// 设置目标图像的大小和类型与源图像一致
		warp_dst = Mat::zeros(src.rows, src.cols, src.type());
		//// TODO: 在此添加控件通知处理程序代码
		///// 设置源图像和目标图像上的三组点以计算仿射变换
		
		//在四边形内适用
		/*srcTri[0] = Point2f(scanPoints[0].x - srcRect.x, (scanPoints[0].y - srcRect.y));
		srcTri[1] = Point2f(scanPoints[1].x - srcRect.x, (scanPoints[1].y - srcRect.y));
		srcTri[2] = Point2f(scanPoints[2].x - srcRect.x, (scanPoints[2].y - srcRect.y));
		srcTri[3] = Point2f(scanPoints[3].x - srcRect.x, (scanPoints[3].y - srcRect.y));

		dstTri[0] = Point(0, 0);
		dstTri[1] = Point(cvRect.width, 0);
		dstTri[2] = Point(cvRect.width, cvRect.height);
		dstTri[3] = Point(0, cvRect.height);*/

		getPoint2fTris(centorPoint, srcRect, scanPoints, srcTri);


		
		for (int i = 0; i < 4; i++){
			cout << "tri" << i << " x=" << srcTri[i].x << "  y=" << srcTri[i].y << endl;
		}
		
		dstTri[0] = Point(0, 0);
		dstTri[1] = Point(srcRect.width, 0);
		dstTri[2] = Point(srcRect.width, srcRect.height);
		dstTri[3] = Point(0, srcRect.height);


		for (int i = 0; i < 4; i++){
			cout << "dst" << i << " x=" << dstTri[i].x << "  y=" << dstTri[i].y << endl;
		}


		warp_mat = getPerspectiveTransform(srcTri, dstTri);
		warpPerspective(src, warp_dst, warp_mat, warp_dst.size());

		/*CvMat* M = cvCreateMat(4, 4, CV_32FC1);
		int step = M->step / sizeof(float);
		float *data = M->data.fl;
		(data + i*step)[j] = 3.0;*/

		uchar* data = warp_mat.data;
		cout << warp_mat.cols << "rows " << warp_mat.rows << endl;
		for (int i = 0; i < warp_mat.cols; i++){
			for (int j = 0; j < warp_mat.rows; j++){
				cout << ((float)data[i * 3 + j]) << endl;
			}
		}

		/// 求得仿射变换
		//warp_mat = getAffineTransform(srcTri, dstTri);
		/// 对源图像应用上面求得的仿射变换
		//warpAffine(src, warp_dst, warp_mat, warp_dst.size());
		IplImage result1(warp_dst);
		cvCircle(&result1, cvPoint(centorPoint.x, centorPoint.y), 50, cvScalar(0, 255, 0), 1);


		namedWindow("灰色");
		//imshow("灰色", warp_dst);
		imshow("灰色",  cv::cvarrToMat(&result1));

		//Rect roiRect();
		//cvSetImageROI(&result, srcRect);
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

	//system("pause");
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


