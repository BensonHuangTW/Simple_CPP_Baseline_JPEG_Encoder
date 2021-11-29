#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "Types.hpp"
#include "RLC.hpp"
#include "Encoder.hpp"

namespace cppeg
{
    cv::Mat RLC::QTables = cv::Mat::zeros(8, 8, CV_32FC3);

    RLC::RLC()
    {
        // use the default quantization tables
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                QTables.at<cv::Vec3f>(i, j)[0] = defaultLuminQTAble[i][j];
                QTables.at<cv::Vec3f>(i, j)[1] = defaultCriominQTable[i][j];
                QTables.at<cv::Vec3f>(i, j)[2] = defaultCriominQTable[i][j];
            }
        }
    }

    void RLC::setHSampFactors(int sampFactorY, int sampFactorCb, int sampFactorCr)
    {
        hSampleFactors[0] = sampFactorY;
        hSampleFactors[1] = sampFactorCb;
        hSampleFactors[2] = sampFactorCr;
    }

    void RLC::setVSampFactors(int sampFactorY, int sampFactorCb, int sampFactorCr)
    {
        hSampleFactors[0] = sampFactorY;
        hSampleFactors[1] = sampFactorCb;
        hSampleFactors[2] = sampFactorCr;
    }

    RunLengthCode RLC::MCUtoRLC(const cv::Mat &MCU,
                                std::vector<int> &curDCValues,
                                const std::vector<int> &prevDCValues)
    {
        // output run-length codes
        std::vector<std::vector<std::pair<int, int>>> outputRLC;

        // convert color space: RGB to YCbCr
        cv::Mat fpMCU(8, 8, CV_32FC3);
        cv::cvtColor(MCU, fpMCU, cv::COLOR_BGR2YCrCb);

        // perform forward DCT for each channel
        std::vector<cv::Mat> channels(3), QTableArr(3);
        cv::split(fpMCU, channels);
        std::iter_swap(channels.begin() + 1, channels.begin() + 2); // CrCb to CbCr
        cv::split(QTables, QTableArr);
        for (int c = 0; c < 3; ++c)
        {
            channels[c] = MCUTransform(channels[c], QTableArr[c]);
            if (prevDCValues.size() != 0)
            {
                curDCValues[c] = channels[c].at<float>(0, 0);
                channels[c].at<float>(0, 0) -= prevDCValues[c];
            }
            std::vector<int> zzorderMCUData = MCUToZzorder(channels[c]);
            outputRLC.push_back(zzorderDataToRLC(zzorderMCUData));
        }

        return outputRLC;
    }

    cv::Mat RLC::MCUTransform(const cv::Mat &MCU, const cv::Mat &QTable)
    {
        assert(MCU.rows == 8 && MCU.cols == 8);

        // create a block to excute float point operation
        cv::Mat fpMCU;
        MCU.convertTo(fpMCU, CV_32F);

        // sfhit the pixel value by -128
        fpMCU -= cv::Scalar::all(128);

        // perform forward DCT
        cv::dct(fpMCU, fpMCU);

        // quantization
        cv::divide(fpMCU, QTable, fpMCU);
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                fpMCU.at<float>(i, j) = roundf(fpMCU.at<float>(i, j));
            }
        }
        return fpMCU;
    }

    std::vector<int> RLC::MCUToZzorder(cv::Mat MCU)
    {
        std::vector<int> runLenCode;
        for (int i = 0; i < 64; ++i)
        {
            std::pair<const int, const int> matIdx = zzOrderToMatIndices(i);
            runLenCode.push_back(MCU.at<float>(matIdx.first, matIdx.second));
        }
        return runLenCode;
    }

    std::vector<std::pair<int, int>> RLC::zzorderDataToRLC(std::vector<int> zzorderData)
    {
        int zeroCount = 0, posEOB = zzorderData.size();
        std::vector<std::pair<int, int>> outputRLC;

        // first find the position of EOB
        for (int i = posEOB - 1; i >= 0; --i)
        {
            if (zzorderData[i] == 0)
                posEOB = i;
            else
                break;
        }

        // generate run-length code
        outputRLC.push_back(std::make_pair(0, zzorderData[0])); // DC component
        for (auto i = 1; i < posEOB; ++i)
        {
            if (zzorderData[i] == 0)
            {
                zeroCount++;
                if (zeroCount == 15)
                {
                    // length of zero coefficients exceed 15
                    outputRLC.push_back(std::make_pair(15, 0));
                    zeroCount = 0;
                }
            }
            else
            {
                outputRLC.push_back(std::make_pair(zeroCount, zzorderData[i]));
                zeroCount = 0;
            }
        }

        // append EOB
        outputRLC.push_back(std::make_pair(0, 0));

        return outputRLC;
    }
}