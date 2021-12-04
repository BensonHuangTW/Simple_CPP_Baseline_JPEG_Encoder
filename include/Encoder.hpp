#ifndef ENCODER_HPP
#define ENCODER_HPP

#include <fstream>
#include <vector>
#include <utility>
#include <bitset>
#include <string>

#include "opencv2/core.hpp"
#include "Markers.hpp"
#include "Types.hpp"
#include "HuffmanTree.hpp"
#include "RLC.hpp"

namespace cppeg
{
    class Encoder
    {
    public:
        enum ResultCode
        {
            SUCCESS,
            TERMINATE,
            ERROR,
            ENCODE_INCOMPLETE,
            ENCODE_DONE
        };

        Encoder();

        Encoder(const std::string &filename);

        ~Encoder();

        /// open the input image file and output image file
        ///
        /// @param iFilename the path of the input image file name
        /// @param oFilename the path of the output compressed file name
        bool open(const std::string &iFilename, std::string oFilename = "");

        ResultCode encodeImageFile();

        void close();

    private:
        std::string m_filename;

        std::ofstream m_imageFile;

        cv::Mat m_image;

        std::vector<std::vector<UInt16>> m_QTables;

        HuffmanCodeMapper m_huffmanCodeMapper[2][2];

        HuffmanTable m_huffmanTable[2][2];

        void constructDefaultHuffmanCodeMapper();

        void constructDefaultHuffmanTables();

        /// convert each channel's run-length code into bit string
        /// and merge them together
        ///
        /// @param RLC array of run-length code for each channel
        /// @return the bit string corresponding to the run-length code
        std::string RLCToBitString(const RLCContainer &RLC);

        /// write the segment marker ,write segment data, and then calculate payload
        /// of segment and write it into the file
        ///
        /// @param marker the marker of segment
        /// @param dataWriter the class member function that write the content of segment
        void segmentWriterHandler(cppeg::Marker marker, void (Encoder::*dataWriter)());

        void writeAPP0Segment();

        void writeCOMSegment();

        void writeDQTSegment();

        void writeQTData(UInt8 precision, UInt8 tableId, const std::vector<UInt16> &QTable);

        void writeSOF0Segment();

        void writeDHTData(HuffmanTable huffmanTable, int tableType, int tableNo);

        void writeDHTSegment();

        void writeSOSSegment();

        void writeScanData();

        /// write the 2 bytes marker into the file
        void writeMarker(UInt8 markerType);

        /// write the payload of the segment based on the passed start point of segment
        /// and current end point of file
        ///
        /// @param segmentBeg start point of the segment
        /// @param includeMarker indicate if the marker itself is counted into the payload
        void writePayloadLength(std::streampos segmentBeg, bool includeMarker = false);

        // some default parameters
        UInt8 luminQTableId = 0;
        UInt8 luminQTablePrecision = 0;
        UInt8 chronminQTableId = 1;
        UInt8 chronminQTablePrecision = 0;
        UInt8 hSampFactors[3] = {1, 1, 1};
        UInt8 vSampFactors[3] = {1, 1, 1};
    };

    /// suggested quantization tables in ITU-T.81, page 143
    static const int defaultLuminQTAble[8][8] =
        {
            {16, 11, 10, 16, 24, 40, 51, 61},
            {12, 12, 14, 19, 26, 58, 60, 55},
            {14, 13, 16, 24, 40, 57, 69, 56},
            {14, 17, 22, 29, 51, 87, 80, 62},
            {18, 22, 37, 56, 68, 109, 103, 77},
            {24, 35, 55, 64, 81, 104, 113, 92},
            {49, 64, 78, 87, 103, 121, 120, 101},
            {72, 92, 95, 98, 112, 100, 103, 99}};

    static const int defaultCriominQTable[8][8] =
        {
            {17, 18, 24, 47, 99, 99, 99, 99},
            {18, 21, 26, 66, 99, 99, 99, 99},
            {24, 26, 56, 99, 99, 99, 99, 99},
            {47, 66, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
    };

    /// convert the default jpeg Huffman table arrays to HuffmanTable
    HuffmanTable huffmanTableArraysToHuffmanTable(const UInt16 bitLen[], const UInt16 symbols[]);

    /// Convert the huffman tables information arrays (bits and value arrays)
    /// into a Huffman code mapper (hash table)
    HuffmanCodeMapper huffmanTableArraysToHuffmanMapper(const UInt16 bitsLen[], const UInt16 symbols[]);

    // suggested huffman tables for DC and AC terms (ITU-T.81, page 149)

    static const UInt16 defaultBitsDCLuminanceCat[17] = {
        /* 0-base */ 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
    static const UInt16 defaultValDCLuminanceCat[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    static const UInt16 defaultBitsDCChrominanceCat[17] = {
        /* 0-base */ 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
    static const UInt16 defaultValDCChrominanceCat[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    static const UInt16 defaultBitsACLuminance[17] = {
        /* 0-base */ 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d};
    static const UInt16 defaultValACLuminance[] = {
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
        0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
        0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
        0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
        0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
        0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
        0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa};

    static const UInt16 defaultBitsACChrominance[17] = {
        /* 0-base */ 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77};
    static const UInt16 defaultValACChrominance[] = {
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
        0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
        0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
        0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
        0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
        0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
        0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa};

}
#endif