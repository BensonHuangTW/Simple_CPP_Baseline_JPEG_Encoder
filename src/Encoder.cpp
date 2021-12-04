#include <arpa/inet.h> // htons
#include <iomanip>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <bitset>
#include <functional>

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "Encoder.hpp"
#include "Markers.hpp"
#include "Utility.hpp"
#include "Transform.hpp"

namespace cppeg
{
    Encoder::Encoder()
    {
        // initialize the quantization table
        m_QTables = std::vector<std::vector<UInt16>>();
        m_QTables.insert(m_QTables.end(), 2, {});

        // initialize quantization table
        for (int i = 0; i < 64; ++i)
        {
            std::pair<int, int> coord = zzOrderToMatIndices(i);
            int x = coord.first, y = coord.second;
            m_QTables[luminQTableId].push_back((UInt16)defaultLuminQTAble[x][y]);
            m_QTables[chronminQTableId].push_back((UInt16)defaultCriominQTable[x][y]);
        }

        // initialize Huffman tables
        constructDefaultHuffmanTables();

        // initialize Huffman code mappers
        constructDefaultHuffmanCodeMapper();

        logFile << "Created \'Encoder object\'." << std::endl;
    }

    Encoder::~Encoder()
    {
        if (m_imageFile.is_open())
            close();
        logFile << "Destroyed \'Encoder object\'." << std::endl;
    }

    void Encoder::close()
    {
        m_imageFile.close();
        logFile << "Closed image file: \'" + m_filename + "\'" << std::endl;
    }

    bool Encoder::open(const std::string &iFilename, std::string oFilename)
    {

        m_image = cv::imread(iFilename, cv::IMREAD_COLOR);
        if (m_image.empty())
        {
            logFile << "Cannot read the input image file: \'" + iFilename + "\'" << std::endl;
            return false;
        }

        if (oFilename == "")
        {
            std::filesystem::path outputPath(iFilename);
            oFilename += outputPath.parent_path().string() + "/" + outputPath.stem().string() + "_compressed.jpg";
        }

        std::cout << "Output file path: \'" << oFilename << "\'" << std::endl;

        m_imageFile.open(oFilename, std::ios::out | std::ios::binary);

        if (!m_imageFile.is_open() || !m_imageFile.good())
        {
            logFile << "Unable to open output image: \'" + oFilename + "\'" << std::endl;
            return false;
        }

        logFile << "Opened JPEG image: \'" + oFilename + "\'" << std::endl;

        m_filename = oFilename;
        return true;
    }

    Encoder::ResultCode Encoder::encodeImageFile()
    {

        if (!m_imageFile.is_open() || !m_imageFile.good())
        {
            logFile << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return ResultCode::ERROR;
        }

        logFile << "Started encoding process..." << std::endl;

        // write SOI marker
        writeMarker(JFIF_SOI);

        segmentWriterHandler(JFIF_APP0, &Encoder::writeAPP0Segment);

        writeCOMSegment();

        writeDQTSegment();

        segmentWriterHandler(JFIF_SOF0, &Encoder::writeSOF0Segment);

        segmentWriterHandler(JFIF_DHT, &Encoder::writeDHTSegment);

        segmentWriterHandler(JFIF_SOS, &Encoder::writeSOSSegment);

        writeScanData();

        m_imageFile.close();
        UInt8 byte;
        ResultCode status = ResultCode::ENCODE_DONE;

        return status;
    }

    void Encoder::constructDefaultHuffmanCodeMapper()
    {
        logFile << "Constructing default Huffman codes mapper from the deafult table" << std::endl;

        logFile << "Luminance DC Huffman table:" << std::endl;
        m_huffmanCodeMapper[HT_DC][HT_Y] = huffmanTableArraysToHuffmanMapper(defaultBitsDCLuminanceCat, defaultValDCLuminanceCat);
        logFile << "Luminance AC Huffman table:" << std::endl;
        m_huffmanCodeMapper[HT_AC][HT_Y] = huffmanTableArraysToHuffmanMapper(defaultBitsACLuminance, defaultValACLuminance);
        logFile << "Chrominance DC Huffman table:" << std::endl;
        m_huffmanCodeMapper[HT_DC][HT_CbCr] = huffmanTableArraysToHuffmanMapper(defaultBitsDCChrominanceCat, defaultValDCChrominanceCat);
        logFile << "Chrominance AC Huffman table:" << std::endl;
        m_huffmanCodeMapper[HT_AC][HT_CbCr] = huffmanTableArraysToHuffmanMapper(defaultBitsACChrominance, defaultValACChrominance);
    }

    void Encoder::constructDefaultHuffmanTables()
    {
        logFile << "Constructing default Huffman tables from the default table" << std::endl;

        m_huffmanTable[HT_DC][HT_Y] = huffmanTableArraysToHuffmanTable(defaultBitsDCLuminanceCat, defaultValDCLuminanceCat);
        m_huffmanTable[HT_AC][HT_Y] = huffmanTableArraysToHuffmanTable(defaultBitsACLuminance, defaultValACLuminance);
        m_huffmanTable[HT_DC][HT_CbCr] = huffmanTableArraysToHuffmanTable(defaultBitsDCChrominanceCat, defaultValDCChrominanceCat);
        m_huffmanTable[HT_AC][HT_CbCr] = huffmanTableArraysToHuffmanTable(defaultBitsACChrominance, defaultValACChrominance);
    }

    std::string Encoder::RLCToBitString(const RLCContainer &RLC)
    {
#ifndef NDEBUG
        assert(RLC.size() == 3);
#endif

        std::string bitString;

        bitString += singleRLCToBitString(RLC[0], m_huffmanCodeMapper[HT_DC][HT_Y], m_huffmanCodeMapper[HT_AC][HT_Y]);
        bitString += singleRLCToBitString(RLC[1], m_huffmanCodeMapper[HT_DC][HT_CbCr], m_huffmanCodeMapper[HT_AC][HT_CbCr]);
        bitString += singleRLCToBitString(RLC[2], m_huffmanCodeMapper[HT_DC][HT_CbCr], m_huffmanCodeMapper[HT_AC][HT_CbCr]);
        return bitString;
    }

    void Encoder::writeAPP0Segment()
    {
        // write the string 'JFIF\0'
        static const char JFIF_STRING[5]{'J', 'F', 'I', 'F', '\0'};
        m_imageFile.write(JFIF_STRING, 5);

        // write JFIF version (first byte for major version and second byte for minor version)
        static const UInt8 majorVersion = 1;
        static const UInt8 minorVersion = 1;
        m_imageFile << majorVersion;
        m_imageFile << minorVersion;

        // write pixel unit density (00 for no units, 01 for pixels per inch, 02 for pixels per cm)
        static const UInt8 densityByte = 1;
        m_imageFile << densityByte;

        // write horizontal and vertical pixel density
        static const UInt16 xDensity = ntohs(72), yDensity = ntohs(72);
        m_imageFile.write(reinterpret_cast<const char *>(&xDensity), 2);
        m_imageFile.write(reinterpret_cast<const char *>(&yDensity), 2);

        // TODO: write thummbnail information
        UInt8 xThumb = 0, yThumb = 0;
        m_imageFile << xThumb << yThumb;

        logFile << "Finished writing JPEG/JFIF marker segment (APP-0) [OK]" << std::endl;
    }

    void Encoder::writeCOMSegment()
    {
        if (!m_imageFile.is_open() || !m_imageFile.good())
        {
            logFile << "Unable to write image file: \'" + m_filename + "\'" << std::endl;
            return;
        }

        logFile << "Writing comment segment )..." << std::endl;

        // write APP0 marker
        std::streampos segmentBeg = m_imageFile.tellp();
        writeMarker(JFIF_COM);

        // since the length of segment is not determined, we first
        // record the position and write an arbitrary length into the file
        UInt16 lenBtye = 0;
        m_imageFile.write(reinterpret_cast<const char *>(&lenBtye), 2);

        // write the comment
        std::string comment("This image was downloaded from WIkipedia and edited using GIMP");
        m_imageFile.write(comment.c_str(), comment.length());

        writePayloadLength(segmentBeg);

        logFile << "Finished writing comment segment [OK]" << std::endl;
    }

    void Encoder::writeDQTSegment()
    {
        if (!m_imageFile.is_open() || !m_imageFile.good())
        {
            logFile << "Unable scan image file: \'" + m_filename + "\'" << std::endl;
            return;
        }

        logFile << "Writing DQT segments..." << std::endl;

        // check if precision is valid
        if (luminQTablePrecision < 0 || luminQTablePrecision > 1)
        {
            logFile << "Invalid precision for luminance quantization table." << std::endl;
        }
        if (chronminQTablePrecision < 0 || chronminQTablePrecision > 1)
        {
            logFile << "Invalid precision for chrominance quantization table." << std::endl;
        }

        // luminance QT
        writeQTData(luminQTablePrecision, luminQTableId, m_QTables[luminQTableId]);

        // chrominance QT
        writeQTData(chronminQTablePrecision, chronminQTableId, m_QTables[chronminQTableId]);

        logFile << "Finished writing quantization table segment [OK]" << std::endl;
    }

    void Encoder::writeQTData(UInt8 precision, UInt8 tableId, const std::vector<UInt16> &QTable)
    {
        // write the marker
        std::streampos segmentBeg = m_imageFile.tellp();
        writeMarker(JFIF_DQT);

        // since the length of segment is not determined, we first
        // record the position and write an arbitrary length into the file
        UInt16 lenBtye = 0;
        m_imageFile.write(reinterpret_cast<const char *>(&lenBtye), 2);

        // write meta data of the table
        UInt8 PqTq; // first four bits: precision, last four bits: number of qauntization tables
        logFile << "Writing quantization table..." << std::endl;
        logFile << "Quantization Table Number: " << (int)tableId << std::endl;
        logFile << "Precision: " << (precision == 0 ? "8-bit" : "16-bit") << std::endl;
        PqTq = (precision == 0) ? 0 : 1 << 4;
        PqTq |= tableId & 0x0F;
        // m_imageFile.write(reinterpret_cast<const char *>(&PqTq), 1);
        m_imageFile << PqTq;

        // write the elements of quantization table
        for (int i = 0; i < 64; ++i)
        {
            if (precision == 0)
            {
                // precision = 8 bits
                UInt8 Qi = m_QTables[tableId][i];
                m_imageFile << Qi;
            }
            else if (precision == 1)
            {
                UInt16 Qi = m_QTables[tableId][i];
                Qi = ntohs(Qi);
                m_imageFile.write(reinterpret_cast<const char *>(&Qi), 2);
            }
            else
            {
                logFile << "[ FATAL ] Unrecognized precision of quantization table" << std::endl;
                return;
            }
        }

        writePayloadLength(segmentBeg);
    }

    void Encoder::writeSOF0Segment()
    {
        logFile << "Writing SOF-0 segment..." << std::endl;

        // write image precision, height, row and component counts
        UInt8 framePrecision = 8, compCount = 3;
        m_imageFile << framePrecision;
        UInt16 imgHeight = m_image.rows, imgWidth = m_image.cols;
        logFile << "Image height: " << (int)imgHeight << std::endl;
        logFile << "Image width: " << (int)imgWidth << std::endl;
        imgHeight = ntohs(imgHeight);
        imgWidth = ntohs(imgWidth);
        m_imageFile.write(reinterpret_cast<const char *>(&imgHeight), 2);
        m_imageFile.write(reinterpret_cast<const char *>(&imgWidth), 2);
        m_imageFile << compCount;

        // write the component data
        static UInt8 compIDs[3] = {1, 2, 3};
        static UInt8 QTNos[3] = {0, 1, 1};
        for (int i = 0; i < 3; ++i)
        {
            UInt8 sampFactor = (hSampFactors[i] << 4) | (vSampFactors[i] & 0x0F);
            m_imageFile << compIDs[i] << sampFactor << QTNos[i];
        }

        logFile << "Finished writing SOF-0 segment [OK]" << std::endl;
    }

    void Encoder::writeDHTData(HuffmanTable huffmanTable, int HTType, int HTNumber)
    {

        // check the table type and table no.
        if (HTType != HT_DC && HTType != HT_AC)
        {
            logFile << "The table type should be either 0 (DC) or 1(AC)!" << std::endl;
            return;
        }

        logFile << "Writing Huffman table segment..." << std::endl;

        // write the type and id of the Huffman table
        logFile << "Huffman table type: " << HTType << std::endl;
        logFile << "Huffman table #: " << HTNumber << std::endl;
        UInt8 htinfo = (HTType & 0x0f) << 4 | (HTNumber & 0x0f);
        m_imageFile << htinfo;

        // write the Huffman table data
        logFile << "Writing symbols for Huffman table (" << HTType
                << "," << HTNumber << ")..." << std::endl;
        for (int i = 0; i < 16; ++i)
        {
            UInt8 symbolCount = huffmanTable[i].first;
            m_imageFile << symbolCount;
        }
        for (int i = 0; i < 16; ++i)
        {
            std::vector<UInt8> symbols = huffmanTable[i].second;
            for (UInt8 symbol : symbols)
            {
                m_imageFile << symbol;
            }
        }

        logFile << "Finished writing Huffman table segment..." << std::endl;
    }

    void Encoder::writeDHTSegment()
    {
        writeDHTData(m_huffmanTable[HT_DC][HT_Y], HT_DC, HT_Y);
        writeDHTData(m_huffmanTable[HT_AC][HT_Y], HT_AC, HT_Y);
        writeDHTData(m_huffmanTable[HT_DC][HT_CbCr], HT_DC, HT_CbCr);
        writeDHTData(m_huffmanTable[HT_AC][HT_CbCr], HT_AC, HT_CbCr);
    }

    void Encoder::writeSOSSegment()
    {
        // write number of components
        UInt8 compCount = 3;
        m_imageFile << compCount;

        // write components data
        UInt16 compInfo;
        UInt8 cID, DCTableNum, ACTableNum;
        // Y components
        compInfo = 1;
        DCTableNum = HT_Y;
        ACTableNum = HT_Y;
        compInfo = (compInfo << 8) | (DCTableNum << 4) | (ACTableNum);
        compInfo = ntohs(compInfo);
        m_imageFile.write(reinterpret_cast<const char *>(&compInfo), 2);
        // Cb components
        compInfo = 2;
        DCTableNum = HT_CbCr;
        ACTableNum = HT_CbCr;
        compInfo = (compInfo << 8) | (DCTableNum << 4) | (ACTableNum);
        compInfo = ntohs(compInfo);
        m_imageFile.write(reinterpret_cast<const char *>(&compInfo), 2);
        // Cr components
        compInfo = 3;
        DCTableNum = HT_CbCr;
        ACTableNum = HT_CbCr;
        compInfo = (compInfo << 8) | (DCTableNum << 4) | (ACTableNum);
        compInfo = ntohs(compInfo);
        m_imageFile.write(reinterpret_cast<const char *>(&compInfo), 2);

        // Ss, Se, Ah and Al (ITU-T81, page 37)
        m_imageFile << (UInt8)0x00 << (UInt8)0x3f << (UInt8)0x00;
    }

    void Encoder::writeScanData()
    {
        // padding the image
        UInt8 MCUsize = 8;
        int padRows = m_image.rows % MCUsize ? MCUsize - (m_image.rows % MCUsize) : 0;
        int padCols = m_image.cols % MCUsize ? MCUsize - (m_image.cols % MCUsize) : 0;
        cv::Mat padImg;
        cv::copyMakeBorder(m_image, padImg, 0, padRows, 0, padCols, cv::BORDER_REPLICATE);

        // compressed image data
        int hBlcokNum = padImg.cols / MCUsize, vBlockNum = padImg.rows / MCUsize;
        std::string scanData;
        std::vector<int> prevDCValues{0, 0, 0}, curDCValues{0, 0, 0};
        for (int j = 0; j < vBlockNum; ++j)
        {
            for (int i = 0; i < hBlcokNum; ++i)
            {
                cv::Mat MCUblock = padImg(cv::Rect(i * MCUsize, j * MCUsize, MCUsize, MCUsize));
                RLC rlc;
                RLCContainer runLengthCode = rlc.MCUtoRLC(MCUblock, curDCValues, prevDCValues);
                scanData += RLCToBitString(runLengthCode);
                for (int k = 0; k < 3; ++k)
                {
                    prevDCValues[k] = curDCValues[k];
                }
            }
        }

        // byte alignment
        logFile << "Number of bits of compressed image data (before byte stuffing)" << scanData.size() << std::endl;
        scanData += std::string((8 - scanData.size()) % 8, '1');
        logFile << "Number of bits of compressed image data after alignment (before byte stuffing)" << scanData.size() << std::endl;

        // write the data (and do byte stuffing)
        size_t totalBytes = 0;
        for (size_t i = 0; i < scanData.size(); i += 8)
        {
            UInt8 byte = std::bitset<8>(scanData.substr(i, 8)).to_ulong();
            m_imageFile << byte;
            totalBytes++;
            if (byte == JFIF_BYTE_FF)
            {
                m_imageFile << (UInt8)0x00;
                totalBytes++;
            }
        }
        writeMarker(JFIF_EOI);
    }

    void Encoder::writeMarker(UInt8 markerType)
    {
        m_imageFile << JFIF_BYTE_FF;
        m_imageFile << markerType;
    }

    void Encoder::segmentWriterHandler(cppeg::Marker marker, void (Encoder::*writer)())
    {
        if (!m_imageFile.is_open() || !m_imageFile.good())
        {
            logFile << "Unable to write image file: \'" + m_filename + "\'" << std::endl;
            return;
        }

        // write marker and record its position
        std::streampos segmentBeg = m_imageFile.tellp();
        writeMarker(marker);

        // since the length of segment is not determined, we first
        // record the position and write an arbitrary length into the file
        UInt16 lenBtye = 0;
        m_imageFile.write(reinterpret_cast<const char *>(&lenBtye), 2);

        // write the segment data
        (this->*writer)();

        writePayloadLength(segmentBeg);
    }

    void Encoder::writePayloadLength(std::streampos segmentBeg, bool includeMarker)
    {
        // write the correct length of segment
        auto segmentEnd = m_imageFile.tellp();
        auto lenBtye = segmentEnd - segmentBeg - 2;
        if (includeMarker)
            lenBtye += 2;
        segmentBeg += 2; // skip over marker
        m_imageFile.seekp(segmentBeg);
        logFile << "Segment payload: " << lenBtye << " bytes" << std::endl;
        lenBtye = ntohs(lenBtye);
        m_imageFile.write(reinterpret_cast<const char *>(&lenBtye), 2);
        m_imageFile.seekp(0, m_imageFile.end);
    }

    HuffmanTable huffmanTableArraysToHuffmanTable(const UInt16 bitsLen[], const UInt16 symbols[])
    {
        HuffmanTable huffmanTable;
        int k = 0;
        for (int i = 1; i <= 16; ++i)
        {
            int numSymbols = bitsLen[i];
            logFile << "Huffman codes of length " << i
                    << " contains " << numSymbols << " symbols" << std::endl;
            huffmanTable[i - 1].first = numSymbols;
            for (int j = 0; j < numSymbols; ++j)
            {
                huffmanTable[i - 1].second.push_back(symbols[k + j]);
            }
            k += numSymbols;
        }
        return huffmanTable;
    }

    HuffmanCodeMapper huffmanTableArraysToHuffmanMapper(const UInt16 bitsLen[], const UInt16 symbols[])
    {
        // convert the default jpeg Huffman table arrays to HuffmanTable
        HuffmanTable huffmanTable = huffmanTableArraysToHuffmanTable(bitsLen, symbols);

        // build the huffman tree
        HuffmanTree huffmanTree;
        huffmanTree.constructHuffmanTree(huffmanTable);

        // use in-order traversal to build the Huffman code mapper
        HuffmanCodeMapper codeMapper;
        constructSymbolToCodeTable(huffmanTree.getTree(), codeMapper);
        return codeMapper;
    }

}