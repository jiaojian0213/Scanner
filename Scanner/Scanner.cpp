//
// Created by qiulinmin on 8/1/17.
//
#include <Scanner.h>
#include <iostream>
using namespace scanner;
using namespace cv;
using namespace std;

static bool sortByArea(const vector<Point> &v1, const vector<Point> &v2) {
	double v1Area = fabs(contourArea(Mat(v1)));
	double v2Area = fabs(contourArea(Mat(v2)));
	return v1Area > v2Area;
}

Scanner::Scanner(cv::Mat& bitmap) {
	srcBitmap = bitmap;
}

Scanner::~Scanner() {
}

vector<vector<Point>> Scanner::getPointLists(Mat &image){
	IplImage pTemp(image);
	CvSeq* contour = 0;
	CvMemStorage* storage = cvCreateMemStorage(0);
	contour = cvCreateSeq(CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
	int a = cvFindContours(&pTemp, storage, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//CvSeq** 
	vector<vector<Point>> pointLists;
	//// 直接使用CONTOUR中的矩形来画轮廓
	for (; contour; contour = contour->h_next)
	{
		int onetourlength = contour->total;
		CvSeqReader reader; //-- 读其中一个轮廓序列
		CvPoint pt = cvPoint(0, 0);
		cvStartReadSeq(contour, &reader); //开始提取 
		vector<Point> pointss;
		for (int i = 0; i < onetourlength; i++){
			CV_READ_SEQ_ELEM(pt, reader); //--读其中一个序列中的一个元素点
			pointss.push_back(Point(pt.x, pt.y));
		} //把这个轮廓点找出后，就可以用这些点画个封闭线 
		pointLists.push_back(pointss);
	}
	return pointLists;
}

vector<Point> Scanner::scanPoint() {
	vector<Point> result;
	int cannyValue[] = { 100, 150, 300 };
	int blurValue[] = { 3, 7, 11, 15 };
	//缩小图片尺寸
	Mat image = resizeImage();
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 4; j++){
			std::cout << i << "---" << j << std::endl;
			//预处理图片
			Mat scanImage = preprocessedImage(image, cannyValue[i], blurValue[j]);
			vector<vector<Point>> contours;
			//vector<Vec4i> hierarchy;
			////提取边框
			//findContours(scanImage, contours, hierarchy, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		
			contours = getPointLists(scanImage);

			//按面积排序
			std::sort(contours.begin(), contours.end(), sortByArea);
			if (contours.size() > 0) {
				vector<Point> contour = contours[0];
				double arc = arcLength(contour, true);
				vector<Point> outDP;
				//多变形逼近
				approxPolyDP(Mat(contour), outDP, 0.01 * arc, true);
				//筛选去除相近的点
				vector<Point> selectedPoints = selectPoints(outDP);
				if (selectedPoints.size() != 4) {
					//如果筛选出来之后不是四边形
					continue;
					//return selectedPoints;
				}
				else {
					int widthMin = selectedPoints[0].x;
					int widthMax = selectedPoints[0].x;
					int heightMin = selectedPoints[0].y;
					int heightMax = selectedPoints[0].y;
					for (int k = 0; k < 4; k++) {
						if (selectedPoints[k].x < widthMin) {
							widthMin = selectedPoints[k].x;
						}
						if (selectedPoints[k].x > widthMax) {
							widthMax = selectedPoints[k].x;
						}
						if (selectedPoints[k].y < heightMin) {
							heightMin = selectedPoints[k].y;
						}
						if (selectedPoints[k].y > heightMax) {
							heightMax = selectedPoints[k].y;
						}
					}
					//选择区域外围矩形面积
					int selectArea = (widthMax - widthMin) * (heightMax - heightMin);
					int imageArea = scanImage.cols * scanImage.rows;
					if (selectArea < (imageArea / 20)) {
						result.clear();
						//筛选出来的区域太小
						continue;
					}
					else {
						result = selectedPoints;
						if (result.size() != 4) {
							Point2f p[4];
							p[0] = Point2f(0, 0);
							p[1] = Point2f(image.cols, 0);
							p[2] = Point2f(image.cols, image.rows);
							p[3] = Point2f(0, image.rows);
							result.push_back(p[0]);
							result.push_back(p[1]);
							result.push_back(p[2]);
							result.push_back(p[3]);
						}
						for (Point &p : result) {
							p.x *= resizeScale;
							p.y *= resizeScale;
						}
						// 按左上，右上，右下，左下排序
						vector<Point> resultPoints = sortPointClockwise(result);
						return resultPoints;
					}
				}
			}
			std::cout << i << "---" << j << std::endl;
		}
	}
	//当没选出所需要区域时，如果还没做过直方图均衡化则尝试使用均衡化，但该操作只执行一次，若还无效，则判定为图片不能裁出有效区域，返回整张图
	if (!isHisEqual){
		isHisEqual = true;
		return scanPoint();
	}
	if (result.size() != 4) {
		Point2f p[4];
		p[0] = Point2f(0, 0);
		p[1] = Point2f(image.cols, 0);
		p[2] = Point2f(image.cols, image.rows);
		p[3] = Point2f(0, image.rows);
		result.push_back(p[0]);
		result.push_back(p[1]);
		result.push_back(p[2]);
		result.push_back(p[3]);
	}
	for (Point &p : result) {
		p.x *= resizeScale;
		p.y *= resizeScale;
	}
	// 按左上，右上，右下，左下排序
	return sortPointClockwise(result);
}

Mat Scanner::resizeImage() {
	int width = srcBitmap.cols;
	int height = srcBitmap.rows;
	int maxSize = width > height ? width : height;
	if (maxSize > resizeThreshold) {
		resizeScale = 1.0f * maxSize / resizeThreshold;
		width = static_cast<int>(width / resizeScale);
		height = static_cast<int>(height / resizeScale);
		Size size(width, height);
		Mat resizedBitmap(size, CV_8UC3);
		resize(srcBitmap, resizedBitmap, size);
		return resizedBitmap;
	}
	return srcBitmap;
}

Mat Scanner::preprocessedImage(Mat &image, int cannyValue, int blurValue) {
	Mat grayMat;
	cvtColor(image, grayMat, CV_BGR2GRAY);

	if (isHisEqual){
		equalizeHist(grayMat, grayMat);
	}
	Mat blurMat;
	GaussianBlur(grayMat, blurMat, Size(blurValue, blurValue), 0);

	Mat thresholdMat;
	adaptiveThreshold(blurMat, thresholdMat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 25, 5.0);



	Mat cannyMat;
	Canny(thresholdMat, cannyMat, 50, cannyValue, 3);

	//threshold(cannyMat, thresholdMat, 0, 255, CV_THRESH_OTSU);
	return cannyMat;
	//adaptiveThreshold(grayMat, thresholdMat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 25, 5.0);
	//return blurMat;
}

vector<Point> Scanner::selectPoints(vector<Point> points) {
	if (points.size() > 4) {
		Point &p = points[0];
		int minX = p.x;
		int maxX = p.x;
		int minY = p.y;
		int maxY = p.y;
		//得到一个矩形去包住所有点
		for (int i = 1; i < points.size(); i++) {
			if (points[i].x < minX) {
				minX = points[i].x;
			}
			if (points[i].x > maxX) {
				maxX = points[i].x;
			}
			if (points[i].y < minY) {
				minY = points[i].y;
			}
			if (points[i].y > maxY) {
				maxY = points[i].y;
			}
		}
		//矩形中心点
		Point center = Point((minX + maxX) / 2, (minY + maxY) / 2);
		//分别得出左上，左下，右上，右下四堆中的结果点
		Point p0 = choosePoint(center, points, 0);
		Point p1 = choosePoint(center, points, 1);
		Point p2 = choosePoint(center, points, 2);
		Point p3 = choosePoint(center, points, 3);
		points.clear();
		//如果得到的点不是０，即是得到的结果点
		if (!(p0.x == 0 && p0.y == 0)){
			points.push_back(p0);
		}
		if (!(p1.x == 0 && p1.y == 0)){
			points.push_back(p1);
		}
		if (!(p2.x == 0 && p2.y == 0)){
			points.push_back(p2);
		}
		if (!(p3.x == 0 && p3.y == 0)){
			points.push_back(p3);
		}
	}
	return points;
}

//type代表左上，左下，右上，右下等方位
Point Scanner::choosePoint(Point center, std::vector<cv::Point> &points, int type) {
	int index = -1;
	int minDis = 0;
	//四个堆都是选择距离中心点较远的点
	if (type == 0) {
		for (int i = 0; i < points.size(); i++) {
			if (points[i].x < center.x && points[i].y < center.y) {
				int dis = static_cast<int>(sqrt(pow((points[i].x - center.x), 2) + pow((points[i].y - center.y), 2)));
				if (dis > minDis){
					index = i;
					minDis = dis;
				}
			}
		}
	}
	else if (type == 1) {
		for (int i = 0; i < points.size(); i++) {
			if (points[i].x < center.x && points[i].y > center.y) {
				int dis = static_cast<int>(sqrt(pow((points[i].x - center.x), 2) + pow((points[i].y - center.y), 2)));
				if (dis > minDis){
					index = i;
					minDis = dis;
				}
			}
		}
	}
	else if (type == 2) {
		for (int i = 0; i < points.size(); i++) {
			if (points[i].x > center.x && points[i].y < center.y) {
				int dis = static_cast<int>(sqrt(pow((points[i].x - center.x), 2) + pow((points[i].y - center.y), 2)));
				if (dis > minDis){
					index = i;
					minDis = dis;
				}
			}
		}

	}
	else if (type == 3) {
		for (int i = 0; i < points.size(); i++) {
			if (points[i].x > center.x && points[i].y > center.y) {
				int dis = static_cast<int>(sqrt(pow((points[i].x - center.x), 2) + pow((points[i].y - center.y), 2)));
				if (dis > minDis){
					index = i;
					minDis = dis;
				}
			}
		}
	}

	if (index != -1){
		return Point(points[index].x, points[index].y);
	}
	return Point(0, 0);
}

vector<Point> Scanner::sortPointClockwise(vector<Point> points) {
	if (points.size() != 4) {
		return points;
	}

	Point unFoundPoint;
	vector<Point> result = { unFoundPoint, unFoundPoint, unFoundPoint, unFoundPoint };

	long minDistance = -1;
	for (Point &point : points) {
		long distance = point.x * point.x + point.y * point.y;
		if (minDistance == -1 || distance < minDistance) {
			result[0] = point;
			minDistance = distance;
		}
	}
	if (result[0] != unFoundPoint) {
		Point &leftTop = result[0];
		points.erase(std::remove(points.begin(), points.end(), leftTop));
		if ((pointSideLine(leftTop, points[0], points[1]) * pointSideLine(leftTop, points[0], points[2])) < 0) {
			result[2] = points[0];
		}
		else if ((pointSideLine(leftTop, points[1], points[0]) * pointSideLine(leftTop, points[1], points[2])) < 0) {
			result[2] = points[1];
		}
		else if ((pointSideLine(leftTop, points[2], points[0]) * pointSideLine(leftTop, points[2], points[1])) < 0) {
			result[2] = points[2];
		}
	}
	if (result[0] != unFoundPoint && result[2] != unFoundPoint) {
		Point &leftTop = result[0];
		Point &rightBottom = result[2];
		points.erase(std::remove(points.begin(), points.end(), rightBottom));
		if (pointSideLine(leftTop, rightBottom, points[0]) > 0) {
			result[1] = points[0];
			result[3] = points[1];
		}
		else {
			result[1] = points[1];
			result[3] = points[0];
		}
	}

	if (result[0] != unFoundPoint && result[1] != unFoundPoint && result[2] != unFoundPoint && result[3] != unFoundPoint) {
		return result;
	}

	return points;
}

long long Scanner::pointSideLine(Point &lineP1, Point &lineP2, Point &point) {
	long x1 = lineP1.x;
	long y1 = lineP1.y;
	long x2 = lineP2.x;
	long y2 = lineP2.y;
	long x = point.x;
	long y = point.y;
	return (x - x1)*(y2 - y1) - (y - y1)*(x2 - x1);
}



