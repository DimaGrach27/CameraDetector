#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/face.hpp>

class FaceRecognizer
{
public:
    FaceRecognizer();
    ~FaceRecognizer();

    void Tick(const cv::Mat& frame, const cv::Mat& faceRow);
    void Recognize(const cv::Mat& frame, const cv::Mat& faceRow);

private:
    cv::Ptr<cv::FaceRecognizerSF> m_faceRecognizer;
    cv::Mat m_registeredFeature;

    bool m_hasRegisteredFace;
};