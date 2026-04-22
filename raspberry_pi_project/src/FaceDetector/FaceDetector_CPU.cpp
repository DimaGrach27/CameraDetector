#include "FaceDetector_CPU.h"

#include <string>
#include <iostream>

#include <opencv2/opencv.hpp>

FaceDetector_CPU::FaceDetector_CPU()
{
    // const std::string faceCascadePath = "/opt/homebrew/opt/opencv/share/opencv4/haarcascades/haarcascade_frontalface_default.xml";
    const std::string modelPath = "resources/models/face_detection_yunet_2023mar.onnx";

    m_faceCascadeYN = cv::FaceDetectorYN::create(
        modelPath,
        "",
        cv::Size(320, 320),
        ScoreThreshold,
        NmsThreshold,
        TopK,
        BackendId,
        TargetId
    );
}

FaceDetector_CPU::~FaceDetector_CPU()
{

}

cv::Mat FaceDetector_CPU::DetectLargestFace(const cv::Mat& frame)
{
    if (frame.empty())
    {
        return cv::Mat();
    }

    m_faceCascadeYN->setInputSize(frame.size());

    cv::Mat faces;
    m_faceCascadeYN->detect(frame, faces);

    if (faces.rows == 0)
    {
        return cv::Mat();
    }

    int bestRow = 0;
    float bestArea = 0.0f;

    for (int row = 0; row < faces.rows; ++row)
    {
        const float x = faces.at<float>(row, 0);
        const float y = faces.at<float>(row, 1);
        const float w = faces.at<float>(row, 2);
        const float h = faces.at<float>(row, 3);

        const float area = w * h;

        if (area > bestArea)
        {
            bestArea = area;
            bestRow = row;
        }
    }

    return faces.row(bestRow).clone();
}
