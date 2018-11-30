//
// Created by qiulinmin on 8/1/17.
//

#ifndef CROPPER_DOC_SCANNER_H
#define CROPPER_DOC_SCANNER_H


#include "opencv2/core/core_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"


namespace scanner{

    class Scanner {
    public:
        int resizeThreshold = 500;

        Scanner(cv::Mat& bitmap);
        virtual ~Scanner();
        std::vector<cv::Point> scanPoint();
		cv::Mat preprocessedImage(cv::Mat &image, int cannyValue, int blurValue);
    private:
        cv::Mat srcBitmap;
        float resizeScale = 1.0f;

        bool isHisEqual = false;

        cv::Mat resizeImage();



        cv::Point choosePoint(cv::Point center, std::vector<cv::Point> &points, int type);

        std::vector<cv::Point> selectPoints(std::vector<cv::Point> points);

        std::vector<cv::Point> sortPointClockwise(std::vector<cv::Point> vector);

        long long pointSideLine(cv::Point& lineP1, cv::Point& lineP2, cv::Point& point);
    };

}

#endif //CROPPER_DOC_SCANNER_H
