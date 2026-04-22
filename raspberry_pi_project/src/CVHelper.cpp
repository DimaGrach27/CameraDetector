#include "CVHelper.h"

#include <opencv2/opencv.hpp>

namespace CVHelper
{
    cv::Rect FaceRowToRect(const cv::Mat &faceRow)
    {
        if (faceRow.empty())
        {
            return cv::Rect();
        }

        const int x = static_cast<int>(faceRow.at<float>(0, 0));
        const int y = static_cast<int>(faceRow.at<float>(0, 1));
        const int w = static_cast<int>(faceRow.at<float>(0, 2));
        const int h = static_cast<int>(faceRow.at<float>(0, 3));

        return cv::Rect(x, y, w, h);
    }
}
