#include <cmath>
#include <bitset>

#include "Transform.hpp"

namespace cppeg
{
    const std::pair<const int, const int> zzOrderToMatIndices( const int zzindex )
    {
        static const std::pair<const int, const int> zzorder[64] =
        {
            {0,0},
            {0,1}, {1,0},         
            {2,0}, {1,1}, {0,2},
            {0,3}, {1,2}, {2,1}, {3,0},
            {4,0}, {3,1}, {2,2}, {1,3}, {0,4},
            {0,5}, {1,4}, {2,3}, {3,2}, {4,1}, {5,0},
            {6,0}, {5,1}, {4,2}, {3,3}, {2,4}, {1,5}, {0,6},
            {0,7}, {1,6}, {2,5}, {3,4}, {4,3}, {5,2}, {6,1}, {7,0},
            {7,1}, {6,2}, {5,3}, {4,4}, {3,5}, {2,6}, {1,7},
            {2,7}, {3,6}, {4,5}, {5,4}, {6,3}, {7,2},
            {7,3}, {6,4}, {5,5}, {4,6}, {3,7},
            {4,7}, {5,6}, {6,5}, {7,4},
            {7,5}, {6,6}, {5,7},
            {6,7}, {7,6},
            {7,7}
        };
        
        return zzorder[zzindex];
    }
    
    const int matIndicesToZZOrder( const int row, const int column )
    {
        static const int matOrder[8][8] = 
        {
            {  0,  1,  5,  6, 14, 15, 27, 28 },
            {  2,  4,  7, 13, 16, 26, 29, 42 },
            {  3,  8, 12, 17, 25, 30, 41, 43 },
            {  9, 11, 18, 24, 31, 40, 44, 53 },
            { 10, 19, 23, 32, 39, 45, 52, 54 },
            { 20, 22, 33, 38, 46, 51, 55, 60 },
            { 21, 34, 37, 47, 50, 56, 59, 61 },
            { 35, 36, 48, 49, 57, 58, 62, 63 }
        };
        
        return matOrder[row][column];
    }

    std::string valuetoBitString(const Int16 value)
    {
        if (value == 0x0000)
            return "";

        std::string bitStr;
        Int16 bitsLen = getValueCategory(value);

        std::bitset<16> bits(abs(value));
        if (value < 0)
            bits.flip();

        bitStr = bits.to_string();
        return bitStr.substr(bitStr.size() - bitsLen, bitsLen);
    }

    const Int16 getValueCategory(const Int16 value)
    {
        if (value == 0x0000)
            return 0;
        return std::log2(std::abs(value)) + 1;
    }

    std::string singleRLCToBitString(const std::vector<std::pair<int, int>> &runLengthCode,
                                     const HuffmanCodeMapper &DCMapper,
                                     const HuffmanCodeMapper &ACMapper)
    {
        std::string bitString;

        // Luminance part
        // DC
        int dcAmplitude = runLengthCode[0].second;
        UInt8 dcCategory = getValueCategory(dcAmplitude);
        bitString += DCMapper.find(dcCategory)->second;
        bitString += valuetoBitString(dcAmplitude);
        // AC
        for (int i = 1; i < runLengthCode.size(); ++i)
        {
            std::pair code = runLengthCode[i];
            int acAmplitude = code.second;
            UInt8 RRRR = code.first & 0x0f, SSSS = getValueCategory(acAmplitude) & 0x0f;
            UInt8 RRRRSSSS = (RRRR << 4) | SSSS;
            bitString += ACMapper.find(RRRRSSSS)->second; // huffman code of RRRRSSSS
            bitString += valuetoBitString(acAmplitude);   // additional bits
        }
        return bitString;
    }
}
