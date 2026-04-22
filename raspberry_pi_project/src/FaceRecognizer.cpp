#include "FaceRecognizer.h"

#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/face.hpp>

#include "CVHelper.h"

FaceRecognizer::FaceRecognizer()
{
    const std::string recognizerModelPath = "resources/models/face_recognition_sface_2021dec.onnx";

    m_faceRecognizer = cv::FaceRecognizerSF::create(
        recognizerModelPath,
        ""
    );
}

FaceRecognizer::~FaceRecognizer()
{

}

void FaceRecognizer::Tick(const cv::Mat& frame, const cv::Mat& faceRow)
{
    if (faceRow.empty() || frame.empty())
    {
        return;
    }

    // const int key = cv::waitKey(1);
    // if (key == 'r' || key == 'R')
    // {
    //     cv::Mat alignedFace;
    //     m_faceRecognizer->alignCrop(frame, faceRow, alignedFace);

    //     cv::Mat currentFeature;
    //     m_faceRecognizer->feature(alignedFace, currentFeature);

    //     m_registeredFeature = currentFeature.clone();
    //     m_hasRegisteredFace = true;
    //     std::cout << "Face registered." << std::endl;
    // }
}

void FaceRecognizer::Recognize(const cv::Mat& frame, const cv::Mat& faceRow)
{
    if (faceRow.empty() || frame.empty())
    {
        return;
    }

    if (!m_hasRegisteredFace)
    {
        return;
    }

    constexpr double CosineMatchThreshold = 0.363;
    constexpr double L2MatchThreshold = 1.128;

    cv::Mat alignedFace;
    m_faceRecognizer->alignCrop(frame, faceRow, alignedFace);
    cv::Mat currentFeature;
    m_faceRecognizer->feature(alignedFace, currentFeature);

    cv::Rect faceRect = CVHelper::FaceRowToRect(faceRow);

    const double cosineScore = m_faceRecognizer->match(
        m_registeredFeature,
        currentFeature,
        cv::FaceRecognizerSF::FR_COSINE);
    
    const double l2Score = m_faceRecognizer->match(
        m_registeredFeature,
        currentFeature,
        cv::FaceRecognizerSF::FR_NORM_L2);
    
    const bool isMatch = cosineScore >= CosineMatchThreshold && l2Score <= L2MatchThreshold;
    const std::string label = isMatch ? "MATCH" : "NO MATCH";
    const cv::Scalar color = isMatch ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
    cv::putText(
        frame,
        label,
        cv::Point(faceRect.x, std::max(30, faceRect.y - 10)),
        cv::FONT_HERSHEY_SIMPLEX,
        0.9,
        color,
        2);
    
    std::cout
        << "cosine=" << cosineScore
        << " l2=" << l2Score
        << std::endl;
}
