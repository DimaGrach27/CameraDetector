#include "FaceDetector_Hailo.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <cmath>

#if defined(__unix__)
#include <sys/mman.h>
#endif

namespace
{
    std::shared_ptr<uint8_t> PageAlignedAlloc(size_t size)
    {
#if defined(__unix__)
        void* addr = mmap(
            nullptr,
            size,
            PROT_WRITE | PROT_READ,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1,
            0
        );

        if (MAP_FAILED == addr)
        {
            throw std::bad_alloc();
        }

        return std::shared_ptr<uint8_t>(
            reinterpret_cast<uint8_t*>(addr),
            [size](uint8_t* ptr)
            {
                munmap(ptr, size);
            }
        );
#else
        return std::shared_ptr<uint8_t>(
            new uint8_t[size],
            std::default_delete<uint8_t[]>()
        );
#endif
    }

    const float* AsFloatPtr(const std::vector<uint8_t>& bytes)
    {
        return reinterpret_cast<const float*>(bytes.data());
    }
}

FaceDetector_Hailo::FaceDetector_Hailo()
{
    if (Initialize("resources/models/scrfd_2.5g.hef"))
    {
        m_outputs.Scores8 = "scrfd_2_5g/conv42";
        m_outputs.Boxes8 = "scrfd_2_5g/conv43";
        m_outputs.Landmarks8 = "scrfd_2_5g/conv44";

        m_outputs.Scores16 = "scrfd_2_5g/conv49";
        m_outputs.Boxes16 = "scrfd_2_5g/conv50";
        m_outputs.Landmarks16 = "scrfd_2_5g/conv51";

        m_outputs.Scores32 = "scrfd_2_5g/conv55";
        m_outputs.Boxes32 = "scrfd_2_5g/conv56";
        m_outputs.Landmarks32 = "scrfd_2_5g/conv57";
    }
}

FaceDetector_Hailo::~FaceDetector_Hailo()
{
}

cv::Mat FaceDetector_Hailo::DetectLargestFace(const cv::Mat& frame)
{
    const std::optional<FaceDetection> largestFace = DetectLargest(frame);

    if (!largestFace.has_value())
    {
        return cv::Mat();
    }

    return (cv::Mat_<float>(1, 15) <<
        static_cast<float>(largestFace->Box.x),
        static_cast<float>(largestFace->Box.y),
        static_cast<float>(largestFace->Box.width),
        static_cast<float>(largestFace->Box.height),
        largestFace->Landmarks[0].x,
        largestFace->Landmarks[0].y,
        largestFace->Landmarks[1].x,
        largestFace->Landmarks[1].y,
        largestFace->Landmarks[2].x,
        largestFace->Landmarks[2].y,
        largestFace->Landmarks[3].x,
        largestFace->Landmarks[3].y,
        largestFace->Landmarks[4].x,
        largestFace->Landmarks[4].y,
        largestFace->Score
    );
}

bool FaceDetector_Hailo::Initialize(const std::string& hefPath)
{
    if (!std::filesystem::exists(hefPath))
    {
        std::cerr << "HEF file does not exist: " << hefPath << std::endl;
        return false;
    }

    const auto fileSize = std::filesystem::file_size(hefPath);

    if (0 == fileSize)
    {
        std::cerr << "HEF file is empty: " << hefPath << std::endl;
        return false;
    }

    std::cout << "Using HEF: " << hefPath << std::endl;
    std::cout << "HEF size: " << fileSize << " bytes" << std::endl;

    try
    {
        if (hefPath.empty())
        {
            std::cerr << "HEF path is empty." << std::endl;
            return false;
        }

        m_vdevice = hailort::VDevice::create().expect("Failed to create VDevice");
        m_inferModel = m_vdevice->create_infer_model(hefPath).expect("Failed to create InferModel");

        const auto inputNames = m_inferModel->get_input_names();

        if (inputNames.empty())
        {
            std::cerr << "Model has no input tensors." << std::endl;
            return false;
        }

        m_inputName = inputNames.front();
        m_inputFrameSize = m_inferModel->input(m_inputName)->get_frame_size();

        m_outputNames = m_inferModel->get_output_names();

        if (m_outputNames.empty())
        {
            std::cerr << "Model has no output tensors." << std::endl;
            return false;
        }

        m_outputFrameSizes.clear();

        for (const auto& outputName : m_outputNames)
        {
            m_outputFrameSizes[outputName] =
                m_inferModel->output(outputName)->get_frame_size();
        }

        std::cout << "Hailo detector initialized." << std::endl;
        std::cout << "Input tensor name: " << m_inputName << std::endl;
        std::cout << "Input frame size: " << m_inputFrameSize << std::endl;

        for (const auto& outputName : m_outputNames)
        {
            std::cout
                << "Output tensor: "
                << outputName
                << ", size: "
                << m_outputFrameSizes[outputName]
                << std::endl;
        }

        m_isInitialized = true;
        return true;
    }
    catch (const hailort::hailort_error& exception)
    {
        std::cerr
            << "Failed to initialize FaceDetector_Hailo. status="
            << exception.status()
            << ", error="
            << exception.what()
            << std::endl;

        return false;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Failed to initialize FaceDetector_Hailo: "
            << exception.what()
            << std::endl;

        return false;
    }
}

std::vector<FaceDetection> FaceDetector_Hailo::Detect(const cv::Mat& frame)
{
    std::vector<FaceDetection> emptyResult;

    if (!m_isInitialized)
    {
        std::cerr << "FaceDetector_Hailo is not initialized." << std::endl;
        return emptyResult;
    }

    if (frame.empty())
    {
        return emptyResult;
    }

    const PreprocessResult preprocessResult = Preprocess(frame);

    RawOutput rawOutput;

    const bool isInferenceSuccessful = RunInference(
        preprocessResult.InputTensor,
        rawOutput
    );

    if (!isInferenceSuccessful)
    {
        return emptyResult;
    }

    return Postprocess(rawOutput, preprocessResult);
}

std::optional<FaceDetection> FaceDetector_Hailo::DetectLargest(const cv::Mat& frame)
{
    const std::vector<FaceDetection> detections = Detect(frame);
    return SelectLargest(detections);
}

FaceDetector_Hailo::PreprocessResult FaceDetector_Hailo::Preprocess(const cv::Mat& frame) const
{
    PreprocessResult result;
    result.OriginalWidth = frame.cols;
    result.OriginalHeight = frame.rows;

    const float scaleX = static_cast<float>(m_inputWidth) / static_cast<float>(frame.cols);
    const float scaleY = static_cast<float>(m_inputHeight) / static_cast<float>(frame.rows);

    result.Scale = std::min(scaleX, scaleY);

    const int resizedWidth = static_cast<int>(frame.cols * result.Scale);
    const int resizedHeight = static_cast<int>(frame.rows * result.Scale);

    result.PadX = (m_inputWidth - resizedWidth) / 2;
    result.PadY = (m_inputHeight - resizedHeight) / 2;

    cv::Mat resizedFrame;
    cv::resize(frame, resizedFrame, cv::Size(resizedWidth, resizedHeight));

    cv::Mat letterboxedFrame(
        m_inputHeight,
        m_inputWidth,
        CV_8UC3,
        cv::Scalar(0, 0, 0)
    );

    resizedFrame.copyTo(
        letterboxedFrame(
            cv::Rect(
                result.PadX,
                result.PadY,
                resizedWidth,
                resizedHeight
            )
        )
    );

    result.InputTensor = letterboxedFrame;
    return result;
}

bool FaceDetector_Hailo::RunInference(
    const cv::Mat& inputTensor,
    RawOutput& rawOutput
)
{
    try
    {
        if (!m_isInitialized)
        {
            return false;
        }

        if (inputTensor.empty())
        {
            return false;
        }

        if (!inputTensor.isContinuous())
        {
            std::cerr << "Input tensor must be continuous." << std::endl;
            return false;
        }

        const size_t inputBytes =
            static_cast<size_t>(inputTensor.total() * inputTensor.elemSize());

        if (inputBytes != m_inputFrameSize)
        {
            std::cerr
                << "Unexpected input size. expected="
                << m_inputFrameSize
                << ", actual="
                << inputBytes
                << std::endl;

            return false;
        }

        auto configuredInferModel =
            m_inferModel->configure().expect("Failed to configure infer model");

        auto bindings =
            configuredInferModel.create_bindings().expect("Failed to create bindings");

        auto inputBuffer = PageAlignedAlloc(m_inputFrameSize);
        std::memcpy(inputBuffer.get(), inputTensor.data, m_inputFrameSize);

        auto inputStatus = bindings.input(m_inputName)->set_buffer(
            hailort::MemoryView(inputBuffer.get(), m_inputFrameSize)
        );

        if (HAILO_SUCCESS != inputStatus)
        {
            throw hailort::hailort_error(inputStatus, "Failed to set input buffer");
        }

        std::vector<std::shared_ptr<uint8_t>> outputGuards;
        rawOutput.Tensors.clear();

        for (const auto& outputName : m_outputNames)
        {
            const size_t outputSize = m_outputFrameSizes.at(outputName);
            auto outputBuffer = PageAlignedAlloc(outputSize);

            auto outputStatus = bindings.output(outputName)->set_buffer(
                hailort::MemoryView(outputBuffer.get(), outputSize)
            );

            if (HAILO_SUCCESS != outputStatus)
            {
                throw hailort::hailort_error(outputStatus, "Failed to set output buffer");
            }

            outputGuards.push_back(outputBuffer);
        }

        auto job = configuredInferModel.run_async(bindings).expect("Failed to start async infer job");

        auto waitStatus = job.wait(std::chrono::milliseconds(1000));

        if (HAILO_SUCCESS != waitStatus)
        {
            throw hailort::hailort_error(waitStatus, "Failed waiting for infer job");
        }

        for (size_t index = 0; index < m_outputNames.size(); ++index)
        {
            const std::string& outputName = m_outputNames[index];
            const size_t outputSize = m_outputFrameSizes.at(outputName);

            rawOutput.Tensors[outputName] = std::vector<uint8_t>(
                outputGuards[index].get(),
                outputGuards[index].get() + outputSize
            );
        }

        return true;
    }
    catch (const hailort::hailort_error& exception)
    {
        std::cerr
            << "RunInference failed. status="
            << exception.status()
            << ", error="
            << exception.what()
            << std::endl;

        return false;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "RunInference failed: "
            << exception.what()
            << std::endl;

        return false;
    }
}

std::vector<FaceDetection> FaceDetector_Hailo::Postprocess(
    const RawOutput& rawOutput,
    const PreprocessResult& preprocessResult
) const
{
    std::vector<FaceDetection> detections;

    DecodeStride(
        rawOutput.Tensors.at(m_outputs.Scores8),
        rawOutput.Tensors.at(m_outputs.Boxes8),
        rawOutput.Tensors.at(m_outputs.Landmarks8),
        8,
        preprocessResult,
        detections
    );

    DecodeStride(
        rawOutput.Tensors.at(m_outputs.Scores16),
        rawOutput.Tensors.at(m_outputs.Boxes16),
        rawOutput.Tensors.at(m_outputs.Landmarks16),
        16,
        preprocessResult,
        detections
    );

    DecodeStride(
        rawOutput.Tensors.at(m_outputs.Scores32),
        rawOutput.Tensors.at(m_outputs.Boxes32),
        rawOutput.Tensors.at(m_outputs.Landmarks32),
        32,
        preprocessResult,
        detections
    );

    ApplyNms(detections);

    return detections;
}

void FaceDetector_Hailo::DecodeStride(
    const std::vector<uint8_t>& scoresBytes,
    const std::vector<uint8_t>& boxesBytes,
    const std::vector<uint8_t>& landmarksBytes,
    int stride,
    const PreprocessResult& preprocessResult,
    std::vector<FaceDetection>& detections
) const
{
    const float* scores = AsFloatPtr(scoresBytes);
    const float* boxes = AsFloatPtr(boxesBytes);
    const float* landmarks = AsFloatPtr(landmarksBytes);

    const int featureWidth = m_inputWidth / stride;
    const int featureHeight = m_inputHeight / stride;
    const int anchorsPerLocation = 2;
    const int locationsCount = featureWidth * featureHeight;

    for (int locationIndex = 0; locationIndex < locationsCount; ++locationIndex)
    {
        const int y = locationIndex / featureWidth;
        const int x = locationIndex % featureWidth;

        for (int anchorIndex = 0; anchorIndex < anchorsPerLocation; ++anchorIndex)
        {
            const int detectionIndex = locationIndex * anchorsPerLocation + anchorIndex;
            const float score = scores[detectionIndex];

            if (score < m_scoreThreshold)
            {
                continue;
            }

            const int boxBaseIndex = detectionIndex * 4;
            const float left = boxes[boxBaseIndex + 0] * static_cast<float>(stride);
            const float top = boxes[boxBaseIndex + 1] * static_cast<float>(stride);
            const float right = boxes[boxBaseIndex + 2] * static_cast<float>(stride);
            const float bottom = boxes[boxBaseIndex + 3] * static_cast<float>(stride);

            const float anchorCenterX = static_cast<float>(x * stride);
            const float anchorCenterY = static_cast<float>(y * stride);

            float x1 = anchorCenterX - left;
            float y1 = anchorCenterY - top;
            float x2 = anchorCenterX + right;
            float y2 = anchorCenterY + bottom;

            x1 = (x1 - static_cast<float>(preprocessResult.PadX)) / preprocessResult.Scale;
            y1 = (y1 - static_cast<float>(preprocessResult.PadY)) / preprocessResult.Scale;
            x2 = (x2 - static_cast<float>(preprocessResult.PadX)) / preprocessResult.Scale;
            y2 = (y2 - static_cast<float>(preprocessResult.PadY)) / preprocessResult.Scale;

            x1 = ClampFloat(x1, 0.0f, static_cast<float>(preprocessResult.OriginalWidth - 1));
            y1 = ClampFloat(y1, 0.0f, static_cast<float>(preprocessResult.OriginalHeight - 1));
            x2 = ClampFloat(x2, 0.0f, static_cast<float>(preprocessResult.OriginalWidth - 1));
            y2 = ClampFloat(y2, 0.0f, static_cast<float>(preprocessResult.OriginalHeight - 1));

            const int width = static_cast<int>(std::round(x2 - x1));
            const int height = static_cast<int>(std::round(y2 - y1));

            if (width <= 0 || height <= 0)
            {
                continue;
            }

            FaceDetection detection;
            detection.Score = score;
            detection.Box = cv::Rect(
                static_cast<int>(std::round(x1)),
                static_cast<int>(std::round(y1)),
                width,
                height
            );

            const int landmarkBaseIndex = detectionIndex * 10;

            for (int pointIndex = 0; pointIndex < 5; ++pointIndex)
            {
                float landmarkX =
                    anchorCenterX +
                    landmarks[landmarkBaseIndex + pointIndex * 2 + 0] * static_cast<float>(stride);

                float landmarkY =
                    anchorCenterY +
                    landmarks[landmarkBaseIndex + pointIndex * 2 + 1] * static_cast<float>(stride);

                landmarkX =
                    (landmarkX - static_cast<float>(preprocessResult.PadX)) / preprocessResult.Scale;

                landmarkY =
                    (landmarkY - static_cast<float>(preprocessResult.PadY)) / preprocessResult.Scale;

                landmarkX = ClampFloat(
                    landmarkX,
                    0.0f,
                    static_cast<float>(preprocessResult.OriginalWidth - 1)
                );

                landmarkY = ClampFloat(
                    landmarkY,
                    0.0f,
                    static_cast<float>(preprocessResult.OriginalHeight - 1)
                );

                detection.Landmarks[pointIndex] = cv::Point2f(landmarkX, landmarkY);
            }

            detections.push_back(detection);
        }
    }
}

void FaceDetector_Hailo::ApplyNms(std::vector<FaceDetection>& detections) const
{
    if (detections.empty())
    {
        return;
    }

    std::sort(
        detections.begin(),
        detections.end(),
        [](const FaceDetection& left, const FaceDetection& right)
        {
            return left.Score > right.Score;
        }
    );

    std::vector<FaceDetection> filteredDetections;
    std::vector<bool> suppressed(detections.size(), false);

    for (size_t currentIndex = 0; currentIndex < detections.size(); ++currentIndex)
    {
        if (suppressed[currentIndex])
        {
            continue;
        }

        filteredDetections.push_back(detections[currentIndex]);

        for (size_t nextIndex = currentIndex + 1; nextIndex < detections.size(); ++nextIndex)
        {
            if (suppressed[nextIndex])
            {
                continue;
            }

            const float iou = CalculateIoU(
                detections[currentIndex].Box,
                detections[nextIndex].Box
            );

            if (iou > m_nmsThreshold)
            {
                suppressed[nextIndex] = true;
            }
        }
    }

    detections = std::move(filteredDetections);
}

float FaceDetector_Hailo::CalculateIoU(
    const cv::Rect& left,
    const cv::Rect& right
) const
{
    const int intersectionX1 = std::max(left.x, right.x);
    const int intersectionY1 = std::max(left.y, right.y);
    const int intersectionX2 = std::min(left.x + left.width, right.x + right.width);
    const int intersectionY2 = std::min(left.y + left.height, right.y + right.height);

    const int intersectionWidth = std::max(0, intersectionX2 - intersectionX1);
    const int intersectionHeight = std::max(0, intersectionY2 - intersectionY1);
    const int intersectionArea = intersectionWidth * intersectionHeight;

    const int leftArea = left.area();
    const int rightArea = right.area();
    const int unionArea = leftArea + rightArea - intersectionArea;

    if (unionArea <= 0)
    {
        return 0.0f;
    }

    return static_cast<float>(intersectionArea) / static_cast<float>(unionArea);
}

std::optional<FaceDetection> FaceDetector_Hailo::SelectLargest(
    const std::vector<FaceDetection>& detections
) const
{
    if (detections.empty())
    {
        return std::nullopt;
    }

    const auto largestIt = std::max_element(
        detections.begin(),
        detections.end(),
        [this](const FaceDetection& left, const FaceDetection& right)
        {
            return CalculateArea(left) < CalculateArea(right);
        }
    );

    return *largestIt;
}

float FaceDetector_Hailo::CalculateArea(const FaceDetection& detection) const
{
    return static_cast<float>(detection.Box.area());
}

float FaceDetector_Hailo::ClampFloat(
    float value,
    float minValue,
    float maxValue
) const
{
    return std::max(minValue, std::min(value, maxValue));
}