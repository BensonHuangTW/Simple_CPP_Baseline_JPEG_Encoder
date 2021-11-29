#ifndef RLC_HPP
#define RLC_HPP

#include <vector>
#include <array>

#include "Types.hpp"
#include "Transform.hpp"
#include "opencv2/core.hpp"

namespace cppeg
{
    /// the class used to convert the MCU block to a raw run-length code
    class RLC
    {
    public:
        /// The total number of MCUs in the image
        static int m_MCUCount;

        /// The quantization table used for quantization
        static std::vector<std::vector<UInt16>> m_QTables;

        /// Default constructor
        RLC();

        /// Parameterized constructor
        ///
        /// Initialize the RLC with 3-channel MCU
        ///
        /// @param QTables the quantization tables to be used for encoding the MCU
        RLC(const std::vector<std::vector<UInt16>> &QTables);

        /// Set the horizontal sample factors
        void setVSampFactors(int sampFactorY, int sampFactorCb, int sampFactorCr);

        /// Set the vertical sample factors
        void setHSampFactors(int sampFactorY, int sampFactorCb, int sampFactorCr);

        /// Set the Quantization table (to be implemented)
        void setQTables(const std::vector<std::vector<UInt16>> &QTables);

        /// convert the MCU into run-length code
        ///
        /// @param MCU MCU to be encoded
        /// @param curDCValues this vecoter will hold each channel's DC values of currently passed
        /// MCU based on the following rule: MCU(0, 0) = curDCValue - prevDCValue
        /// @param prevDCValues the DC values of previous encoded MCU
        RunLengthCode MCUtoRLC(const cv::Mat &MCU,
                               std::vector<int> &curDCValues,
                               const std::vector<int> &prevDCValues = std::vector<int>());

    private:
        /// horizontal sample factors for Y, Cb, Cr
        int hSampleFactors[3] = {1, 1, 1};

        /// verticals sample factors for Y, Cb, Cr
        int vSampFactors[3] = {1, 1, 1};

        /// quantization tables for Y, Cb, Cr
        static cv::Mat QTables;

        /// perform RGBtoYCbCr, shifting, forward DCT and
        /// quantization to obtained the final MCU to be encoded
        ///
        /// @param MCU single channel of MCU after RGB to YCbCr transform
        /// @param QTable quantization table used to quantize MCU elements
        /// @return the MCU after the transform described above
        cv::Mat MCUTransform(const cv::Mat &MCU, const cv::Mat &QTable);

        /// convert McU block into vector in zig-zag order
        ///
        /// @param MCU input MCU in matrix form
        /// @return MCU elements array in zig-zag order
        std::vector<int> MCUToZzorder(cv::Mat MCU);

        /// convert zig-zag order MCU data to run-length code
        ///
        /// @param zzorderData MCU elements array in zig-zag order
        /// @return the corresponding run-length code of zzorderData
        std::vector<std::pair<int, int>> zzorderDataToRLC(std::vector<int> zzorderData);
    };
}

#endif