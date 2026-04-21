#include "FaceTracker.h"

#include <algorithm>
#include <string>

#include "CVHelper.h"
#include "FaceDetector/FaceDetector_CPU.h"
#include "FaceDetector/FaceDetector_Hailo.h"

FaceTracker::FaceTracker()
    : m_isTracking(false)
    , m_needRedetect(false)
    , m_horizontalOffsetPx(0)
    , m_horizontalOffsetNormalized(0.0f)
{
    m_faceDetector = std::make_unique<FaceDetector_CPU>();
}

FaceTracker::~FaceTracker()
{
}

void FaceTracker::StartTrack(const cv::Mat &frame, const int frameCount)
{
    if (!m_isTracking)
    {
        m_needRedetect = true;
    }
    else if (frameCount % RedetectIntervalFrames == 0)
    {
        m_needRedetect = true;
    }

    if (m_needRedetect)
    {
        m_face = m_faceDetector->DetectLargestFace(frame);
        if (m_face.empty())
        {
            m_isTracking = false;
            m_horizontalOffsetPx = 0;
            m_horizontalOffsetNormalized = 0.0f;
            return;
        }
        
        DrawLandmarks(frame, m_face);

        const cv::Rect detectedFace = CVHelper::FaceRowToRect(m_face);
        if (detectedFace.area() > 0)
        {
            m_tracker = CreateTracker();
            m_tracker->init(frame, detectedFace);
            m_trackedFace = detectedFace;
            m_isTracking = true;
            UpdateHorizontalOffset(frame);
        }
        else
        {
            m_isTracking = false;
            m_horizontalOffsetPx = 0;
            m_horizontalOffsetNormalized = 0.0f;
        }

        m_needRedetect = false;
        return;
    }

    cv::Rect updatedFace;
    const bool ok = m_tracker->update(frame, updatedFace);

    if (ok && updatedFace.area() > 0)
    {
        m_trackedFace = updatedFace;
        UpdateHorizontalOffset(frame);
    }
    else
    {
        m_isTracking = false;
        m_horizontalOffsetPx = 0;
        m_horizontalOffsetNormalized = 0.0f;
    }
}

void FaceTracker::Track(const cv::Mat &frame)
{
    if (m_isTracking)
    {
        cv::rectangle(frame, m_trackedFace, cv::Scalar(0, 255, 0), 2);
        cv::putText(frame, "Tracking", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
        const std::string offsetText =
            "offsetX: " + std::to_string(m_horizontalOffsetPx) +
            " px (" + std::to_string(m_horizontalOffsetNormalized) + ")";
        cv::putText(frame, offsetText, cv::Point(20, 80), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
        cv::line(
            frame,
            cv::Point(frame.cols / 2, 0),
            cv::Point(frame.cols / 2, frame.rows),
            cv::Scalar(255, 255, 0),
            1);

    }
    else
    {
        cv::putText(frame, "Searching face...", cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
    }
    
    m_faceRecognizer.Tick(frame, m_face);
    m_faceRecognizer.Recognize(frame, m_face);
}

bool FaceTracker::HasTrackedFace() const
{
    return m_isTracking;
}

int FaceTracker::GetHorizontalOffsetPx() const
{
    return m_horizontalOffsetPx;
}

float FaceTracker::GetHorizontalOffsetNormalized() const
{
    return m_horizontalOffsetNormalized;
}

cv::Ptr<cv::Tracker> FaceTracker::CreateTracker()
{
    return cv::TrackerKCF::create();
}

void FaceTracker::DrawLandmarks(const cv::Mat &frame, const cv::Mat &faceRow)
{
    for (int i = 0; i < 5; ++i)
    {
        const int px = static_cast<int>(faceRow.at<float>(0, 4 + i * 2));
        const int py = static_cast<int>(faceRow.at<float>(0, 5 + i * 2));

        cv::circle(frame, cv::Point(px, py), 3, cv::Scalar(0, 0, 255), -1);
    }
}

void FaceTracker::UpdateHorizontalOffset(const cv::Mat& frame)
{
    if (frame.empty() || m_trackedFace.area() <= 0)
    {
        m_horizontalOffsetPx = 0;
        m_horizontalOffsetNormalized = 0.0f;
        return;
    }

    const int frameCenterX = frame.cols / 2;
    const int faceCenterX = m_trackedFace.x + (m_trackedFace.width / 2);
    m_horizontalOffsetPx = faceCenterX - frameCenterX;

    const float halfFrameWidth = static_cast<float>(frame.cols) / 2.0f;
    if (halfFrameWidth <= 0.0f)
    {
        m_horizontalOffsetNormalized = 0.0f;
        return;
    }

    const float normalizedOffset = static_cast<float>(m_horizontalOffsetPx) / halfFrameWidth;
    m_horizontalOffsetNormalized = std::clamp(normalizedOffset, -1.0f, 1.0f);
}
