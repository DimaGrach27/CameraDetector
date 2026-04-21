#pragma once

#include <opencv2/opencv.hpp>

#include "IFaceDetector.h"

class FaceDetector_CPU : public IFaceDetector
{
public:
    FaceDetector_CPU();
    ~FaceDetector_CPU() override;

    cv::Mat DetectLargestFace(const cv::Mat& frame) override;
    
private:
    static constexpr float ScoreThreshold = 0.85f;
    static constexpr float NmsThreshold = 0.3f;
    static constexpr int TopK = 5000;

    static constexpr int BackendId = 0;
    static constexpr int TargetId = 0;

    cv::Ptr<cv::FaceDetectorYN> m_faceCascadeYN;
};