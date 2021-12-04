/// Types module
///
/// Contains definition of standard types

#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <array>
#include <unordered_map>
#include <utility>
#include <memory>

namespace cppeg
{
    /// Standard, cross-platform word sizes for channels
    ///
    /// The channels we deal with here are Red, Green and Blue channels
    /// in the RGB color model  and the Y, Cb and Cr channels in the
    /// Y-Cb-Cr color model.

    /// Standard unsigned integral types
    typedef unsigned char UInt8;
    typedef unsigned short UInt16;

    /// Standard signed integral types
    typedef char Int8;
    typedef short Int16;

    /// Aliases for commonly used types

    /// Huffman table, each element is a pair that denotes (counts, array of symbols)
    typedef std::array<std::pair<int, std::vector<UInt8>>, 16> HuffmanTable;

    /// Hash table whose key is symbol and value is the corresponding Huffman code
    typedef std::unordered_map<UInt16, std::string> HuffmanCodeMapper;

    /// Run-length code for single channel
    typedef std::vector<std::pair<int, int>> ChannelRLC;

    /// Run-length codes container (each component corresponds to RLC of a channel)
    typedef std::vector<ChannelRLC> RLCContainer;

    /// Identifiers used to access a Huffman table based on the class and ID
    /// E.g., To access the Huffman table for the DC coefficients of the
    /// CbCr component, we use `huff_table[HT_DC][HT_CbCr]`.
    const int HT_DC = 0;
    const int HT_AC = 1;
    const int HT_Y = 0;
    const int HT_CbCr = 1;
}

#endif // TYPES_HPP
