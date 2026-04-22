#pragma once

#include <opencv2/opencv.hpp>

#include <hailo/hailort.hpp>

#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "IFaceDetector.h"

class FaceDetector_Hailo : public IFaceDetector
{
public:
    FaceDetector_Hailo();
    ~FaceDetector_Hailo() override;

    bool Initialize(const std::string& hefPath);

    cv::Mat DetectLargestFace(const cv::Mat& frame) override;
    std::vector<FaceDetection> Detect(const cv::Mat& frame);

    std::optional<FaceDetection> DetectLargest(const cv::Mat& frame);

private:
    struct PreprocessResult
    {
        cv::Mat InputTensor;
        float Scale = 1.0f;
        int PadX = 0;
        int PadY = 0;
        int OriginalWidth = 0;
        int OriginalHeight = 0;
    };

    struct RawOutput
    {
        std::map<std::string, std::vector<uint8_t>> Tensors;
    };

    struct ScrfdOutputNames
    {
        std::string Scores8;
        std::string Boxes8;
        std::string Landmarks8;

        std::string Scores16;
        std::string Boxes16;
        std::string Landmarks16;

        std::string Scores32;
        std::string Boxes32;
        std::string Landmarks32;
    };

private:
    PreprocessResult Preprocess(const cv::Mat& frame) const;

    bool RunInference(
        const cv::Mat& inputTensor,
        RawOutput& rawOutput
    );

    std::vector<FaceDetection> Postprocess(
        const RawOutput& rawOutput,
        const PreprocessResult& preprocessResult
    ) const;

    std::optional<FaceDetection> SelectLargest(
        const std::vector<FaceDetection>& detections
    ) const;

    float CalculateArea(const FaceDetection& detection) const;

    void DecodeStride(
        const std::vector<uint8_t>& scoresBytes,
        const std::vector<uint8_t>& boxesBytes,
        const std::vector<uint8_t>& landmarksBytes,
        int stride,
        const PreprocessResult& preprocessResult,
        std::vector<FaceDetection>& detections
        ) const;

    void ApplyNms(std::vector<FaceDetection>& detections) const;
    
    float CalculateIoU(
        const cv::Rect& left,
        const cv::Rect& right
    ) const;
    
    float ClampFloat(
        float value,
        float minValue,
        float maxValue
    ) const;

private:
    bool m_isInitialized = false;

    int m_inputWidth = 640;
    int m_inputHeight = 640;

    float m_scoreThreshold = 0.5f;
    float m_nmsThreshold = 0.4f;

    std::shared_ptr<hailort::VDevice> m_vdevice;
    std::shared_ptr<hailort::InferModel> m_inferModel;

    std::string m_inputName;
    size_t m_inputFrameSize = 0;

    std::vector<std::string> m_outputNames;
    std::map<std::string, size_t> m_outputFrameSizes;
    ScrfdOutputNames m_outputs;
};