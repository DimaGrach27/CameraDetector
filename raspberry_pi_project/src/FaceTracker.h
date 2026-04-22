#pragma once

#include <opencv2/tracking.hpp>

#include "FaceDetector/IFaceDetector.h"
#include "FaceRecognizer.h"

class FaceTracker
{
public:
    FaceTracker();
    ~FaceTracker();

    void StartTrack(const cv::Mat& frame, const int frameCount);
    void Track(const cv::Mat& frame);
    bool HasTrackedFace() const;
    int GetHorizontalOffsetPx() const;
    float GetHorizontalOffsetNormalized() const;

private:
    cv::Ptr<cv::Tracker> CreateTracker();
    void DrawLandmarks(const cv::Mat& frame, const cv::Mat& faceRow);
    void UpdateHorizontalOffset(const cv::Mat& frame);

private:
    static constexpr int RedetectIntervalFrames = 60;

    bool m_isTracking;
    bool m_needRedetect;

    cv::Ptr<cv::Tracker> m_tracker;
    cv::Rect m_trackedFace;
    cv::Mat m_face;
    int m_horizontalOffsetPx;
    float m_horizontalOffsetNormalized;

    std::unique_ptr<IFaceDetector> m_faceDetector;
    FaceRecognizer m_faceRecognizer;
};
