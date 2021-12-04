/// Transform helper module

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <string>
#include <utility>

#include "Types.hpp"

namespace cppeg
{
    /// Convert a zig-zag order index to its corresponding matrix indices
    ///
    /// @param zzIndex the index in the zig-zag order
    /// @return the matrix indices corresponding to the zig-zag order
    const std::pair<const int, const int> zzOrderToMatIndices(const int zzindex);

    /// Convert matrix indices to its corresponding zig-zag order index
    ///
    /// @param the matrix indices
    /// @return the zig-zag index corresponding to the matrix indices
    const int matIndicesToZZOrder(const int row, const int column);

    /// Convert a value to it's corresponding bit string
    ///
    /// @param value value of the number
    /// @return the bit string corresponding to the value
    std::string valuetoBitString(const Int16 value);

    /// Get the category of a value
    ///
    /// @param value the whose category has to be determined
    /// @return the category of the specified value
    const Int16 getValueCategory(const Int16 value);

    /// Convert a single channel run-length code into bit string
    /// (based on passed Huffman table of DC and AC coefficient)
    ///
    /// @param runLengthCode run-length code of single channel
    /// @param DCMapper huffman code mapper of DC coefficient
    /// @param ACMapper huffman code mapper of AC coefficient
    /// @return the bit string corresponding to the run-length code
    std::string singleRLCToBitString(const ChannelRLC &runLengthCode,
                                     const HuffmanCodeMapper &DCMapper,
                                     const HuffmanCodeMapper &ACMapper);

}

#endif // TRANSFORM_HPP
