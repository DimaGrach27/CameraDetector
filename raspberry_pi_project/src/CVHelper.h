#pragma once

#include <opencv2/opencv.hpp>

namespace CVHelper
{
    cv::Rect FaceRowToRect(const cv::Mat& faceRow);
}