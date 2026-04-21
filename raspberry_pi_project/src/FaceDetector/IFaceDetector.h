#pragma once

#include <opencv2/opencv.hpp>

class IFaceDetector
{
public:
    virtual ~IFaceDetector() = default;
    virtual cv::Mat DetectLargestFace(const cv::Mat& frame) = 0;
};

struct FaceDetection
{
    cv::Rect Box;
    float Score = 0.0f;
    std::array<cv::Point2f, 5> Landmarks;
};