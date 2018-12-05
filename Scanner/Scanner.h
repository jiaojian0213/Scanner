//
// Created by qiulinmin on 8/1/17.
//

#ifndef CROPPER_DOC_SCANNER_H
#define CROPPER_DOC_SCANNER_H


#include "opencv2/core/core_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

namespace scanner{

    class Scanner {
    public:
        int resizeThreshold = 500;

        Scanner(cv::Mat& bitmap);
        virtual ~Scanner();
		std::vector<std::vector<cv::Point>> getPointLists(cv::Mat &image);
        std::vector<cv::Point> scanPoint();
		cv::Mat resizeImage();
		cv::Mat preprocessedImage(cv::Mat &image, int cannyValue, int blurValue);

		//show the obj m_Mat on window
		void ShowInWindow(const char *pWinName);
    private:
        cv::Mat srcBitmap;
        float resizeScale = 1.0f;

        bool isHisEqual = false;





        cv::Point choosePoint(cv::Point center, std::vector<cv::Point> &points, int type);

        std::vector<cv::Point> selectPoints(std::vector<cv::Point> points);

        std::vector<cv::Point> sortPointClockwise(std::vector<cv::Point> vector);

        long long pointSideLine(cv::Point& lineP1, cv::Point& lineP2, cv::Point& point);
    };

}

#endif //CROPPER_DOC_SCANNER_H
