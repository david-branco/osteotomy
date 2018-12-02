/*

Imebra community build 20151130-002

Imebra: a C++ Dicom library

Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
by Paolo Brandoli/Binarno s.p.

All rights reserved.

This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2 as published by
 the Free Software Foundation.

This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

-------------------

If you want to use Imebra commercially then you have to buy the commercial
 license available at http://imebra.com

After you buy the commercial license then you can use Imebra according
 to the terms described in the Imebra Commercial License Version 2.
A copy of the Imebra Commercial License Version 2 is available in the
 documentation pages.

Imebra is available at http://imebra.com

The author can be contacted by email at info@binarno.com or by mail at
 the following address:
 Binarno s.p., Paolo Brandoli
 Rakuseva 14
 1000 Ljubljana
 Slovenia



*/

/*! \file jpegCodec.cpp
    \brief Implementation of the class jpegCodec.

*/

#include "../../base/include/exception.h"
#include "../../base/include/streamReader.h"
#include "../../base/include/streamWriter.h"
#include "../../base/include/huffmanTable.h"
#include "../include/jpegCodec.h"
#include "../include/dataSet.h"
#include "../include/image.h"
#include "../include/dataHandlerNumeric.h"
#include "../include/codecFactory.h"
#include <vector>
#include <stdlib.h>
#include <string.h>

namespace puntoexe
{

namespace imebra
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default luminance or RGB quantization table
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegStdLuminanceQuantTbl[] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default chrominance quantization table
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegStdChrominanceQuantTbl[] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default scale factors for FDCT/IDCT calculation
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const float JpegDctScaleFactor[]=
{
    (float)1.0,
    (float)1.387039845,
    (float)1.306562965,
    (float)1.175875602,
    (float)1.0,
    (float)0.785694958,
    (float)0.541196100,
    (float)0.275899379
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of luminance channel
// (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsDcLuminance[]=
{ 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of luminance channel
// (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValDcLuminance[]=
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of chrominance
//  channel (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsDcChrominance[]=
{ 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of chrominance
//  channel (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValDcChrominance[]=
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for AC values of luminance channel
// (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsAcLuminance[]=
{ 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for AC values of luminance channel
// (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValAcLuminance[]=
{
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
    0xf9, 0xfa
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for AC values of chrominance
//  channel (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsAcChrominance[] =
{ 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Default Huffman table for AC values of chrominance
//  channel (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValAcChrominance[] =
{
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
    0xf9, 0xfa
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Translate zig-zag order in 8x8 blocks to raw order
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegDeZigZagOrder[]=
{
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

#define JPEG_DECOMPRESSION_BITS_PRECISION 14

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodec
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
jpegCodec::jpegCodec()
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::jpegCodec");

    // Resets the channels list
    ///////////////////////////////////////////////////////////
    memset(m_channelsList, 0, sizeof(m_channelsList));

    // Allocate the huffman tables
    ///////////////////////////////////////////////////////////
    for(int resetHuffmanTables = 0; resetHuffmanTables<16; ++resetHuffmanTables)
    {
        ptr<huffmanTable> huffmanDC(new huffmanTable(9));
        m_pHuffmanTableDC[resetHuffmanTables]=huffmanDC;

        ptr<huffmanTable> huffmanAC(new huffmanTable(9));
        m_pHuffmanTableAC[resetHuffmanTables]=huffmanAC;
    }

    // Register all the tag classes
    ///////////////////////////////////////////////////////////

    // Unknown tag must be registered
    ///////////////////////////////////////////////////////////
    registerTag(unknown, ptr<jpeg::tag>(new jpeg::tagUnknown));

    // Register SOF
    ///////////////////////////////////////////////////////////
    registerTag(sof0, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof1, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof2, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof3, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof5, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof6, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof7, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sof9, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sofA, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sofB, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sofD, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sofE, ptr<jpeg::tag>(new jpeg::tagSOF));
    registerTag(sofF, ptr<jpeg::tag>(new jpeg::tagSOF));

    // Register DHT
    ///////////////////////////////////////////////////////////
    registerTag(dht, ptr<jpeg::tag>(new jpeg::tagDHT));

    // Register DQT
    ///////////////////////////////////////////////////////////
    registerTag(dqt, ptr<jpeg::tag>(new jpeg::tagDQT));

    // Register SOS
    ///////////////////////////////////////////////////////////
    registerTag(sos, ptr<jpeg::tag>(new jpeg::tagSOS));

    // Register EOI
    ///////////////////////////////////////////////////////////
    registerTag(eoi, ptr<jpeg::tag>(new jpeg::tagEOI));

    // Register RST
    ///////////////////////////////////////////////////////////
    registerTag(rst0, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst1, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst2, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst3, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst4, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst5, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst6, ptr<jpeg::tag>(new jpeg::tagRST));
    registerTag(rst7, ptr<jpeg::tag>(new jpeg::tagRST));

    // Register DRI
    ///////////////////////////////////////////////////////////
    registerTag(dri, ptr<jpeg::tag>(new jpeg::tagDRI));

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
jpegCodec::~jpegCodec()
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Create another JPEG codec
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<codec> jpegCodec::createCodec()
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::createCodec");

    return ptr<codec>(new jpegCodec);

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::registerTag(tTagId tagId, ptr<jpeg::tag> pTag)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::registerTag");

    ptr<jpeg::tag> test = pTag;
    m_tagsMap[(std::uint8_t)tagId]=pTag;

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a jpeg stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::writeStream(ptr<streamWriter> pStream, ptr<dataSet> pDataSet)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::writeStream");

    lockObject lockDataSet(pDataSet.get());

    // Retrieve the transfer syntax
    ///////////////////////////////////////////////////////////
    std::wstring transferSyntax = pDataSet->getUnicodeString(0x0002, 0x0, 0x0010, 0x0);

    // The buffer can be written as it is
    ///////////////////////////////////////////////////////////
    if(canHandleTransferSyntax(transferSyntax))
    {
        ptr<data> imageData = pDataSet->getTag(0x7fe0, 0, 0x0010, false);
        if(imageData == 0 || !imageData->bufferExists(0))
        {
            PUNTOEXE_THROW(dataSetImageDoesntExist, "The requested image doesn't exist");
        }
        std::uint32_t firstBufferId(0);
        std::uint32_t endBufferId(1);
        if(imageData->bufferExists(1))
        {
            pDataSet->getFrameBufferIds(0, &firstBufferId, &endBufferId);
        }
        for(std::uint32_t scanBuffers = firstBufferId; scanBuffers != endBufferId; ++scanBuffers)
        {
            ptr<handlers::dataHandlerRaw> readHandler = imageData->getDataHandlerRaw(scanBuffers, false, "");
            std::uint8_t* readBuffer = readHandler->getMemoryBuffer();
            pStream->write(readBuffer, readHandler->getSize());
        }
        return;

    }

    // Get the image then write it
    ///////////////////////////////////////////////////////////
    ptr<image> decodedImage = pDataSet->getImage(0);
    std::wstring defaultTransferSyntax(L"1.2.840.10008.1.2.4.50"); // baseline (8 bits lossy)
    setImage(pStream, decodedImage, defaultTransferSyntax, codecs::codec::high, "OB", 8, true, true, false, false);

    PUNTOEXE_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Erase all the allocated channels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::eraseChannels()
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::eraseChannels");

    m_channelsMap.clear();
    memset(m_channelsList, 0, sizeof(m_channelsList));

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Reset all the internal variables
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::resetInternal(bool bCompression, quality compQuality)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::resetInternal");

    // Factor used to calculate the quantization tables used
    //  for the compression
    ///////////////////////////////////////////////////////////
    float compQuantization = (float)compQuality / (float)medium;

    eraseChannels();

    m_imageSizeX = m_imageSizeY = 0;

    m_precision = 8;
    m_valuesMask = ((std::int32_t)1 << m_precision)-1;

    m_process = 0;

    m_mcuPerRestartInterval = 0;

    m_mcuLastRestart = 0;

    m_spectralIndexStart = 0;
    m_spectralIndexEnd = 63;
    m_bitHigh = 0;
    m_bitLow = 0;

    m_bLossless = false;

    // The number of MCUs (horizontal, vertical, total)
    ///////////////////////////////////////////////////////////
    m_mcuNumberX = 0;
    m_mcuNumberY = 0;
    m_mcuNumberTotal = 0;

    m_maxSamplingFactorX = 0;
    m_maxSamplingFactorY = 0;

    m_mcuProcessed = 0;
    m_mcuProcessedX = 0;
    m_mcuProcessedY = 0;
    m_eobRun = 0;

    m_jpegImageSizeX = 0;
    m_jpegImageSizeY = 0;

    // Reset the QT tables
    ///////////////////////////////////////////////////////////
    for(int resetQT = 0; resetQT<16; ++resetQT)
    {
        const std::uint32_t* pSourceTable = (resetQT == 0) ? JpegStdLuminanceQuantTbl : JpegStdChrominanceQuantTbl;

        std::uint8_t tableIndex = 0;
        for(std::uint8_t row = 0; row < 8; ++row)
        {
            for(std::uint8_t col = 0; col < 8; ++col)
            {
                if(bCompression)
                {
                    std::uint32_t quant = (std::uint32_t) ((float)(pSourceTable[tableIndex]) * compQuantization);
                    if(quant < 1)
                    {
                        quant = 1;
                    }
                    if(quant > 255)
                    {
                        quant = 255;
                    }
                    m_quantizationTable[resetQT][tableIndex++] = quant;
                    continue;
                }
                m_quantizationTable[resetQT][tableIndex] = pSourceTable[tableIndex];
                ++tableIndex;
            }
        }
        recalculateQuantizationTables(resetQT);
    }

    // Reset the huffman tables
    ///////////////////////////////////////////////////////////
    for(int DcAc = 0; DcAc < 2; ++DcAc)
    {
        for(int resetHT=0; resetHT < 16; ++resetHT)
        {
            ptr<huffmanTable> pHuffman;
            const std::uint32_t* pLengthTable;
            const std::uint32_t* pValuesTable;
            if(DcAc == 0)
            {
                pHuffman=m_pHuffmanTableDC[resetHT];
                if(resetHT == 0)
                {
                    pLengthTable=JpegBitsDcLuminance;
                    pValuesTable=JpegValDcLuminance;
                }
                else
                {
                    pLengthTable=JpegBitsDcChrominance;
                    pValuesTable=JpegValDcChrominance;
                }
            }
            else
            {
                pHuffman=m_pHuffmanTableAC[resetHT];
                if(resetHT == 0)
                {
                    pLengthTable=JpegBitsAcLuminance;
                    pValuesTable=JpegValAcLuminance;
                }
                else
                {
                    pLengthTable=JpegBitsAcChrominance;
                    pValuesTable=JpegValAcChrominance;
                }
            }

            pHuffman->reset();
            if(bCompression)
            {
                continue;
            }

            // Read the number of codes per length
            /////////////////////////////////////////////////////////////////
            std::uint32_t valueIndex = 0;
            for(int scanLength = 0; scanLength<16; ++scanLength)
            {
                pHuffman->setValuesPerLength(scanLength + 1, (std::uint32_t)pLengthTable[scanLength]);
                for(std::uint32_t scanValues(0); scanValues < pLengthTable[scanLength]; ++scanValues)
                {
                    pHuffman->addOrderedValue(valueIndex, pValuesTable[valueIndex]);
                    ++valueIndex;
                }
            }
            pHuffman->calcHuffmanTables();
        }
    }

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate the channels. This function is called when a
//  SOF tag is found
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::allocChannels()
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::allocChannels");

    m_maxSamplingFactorX=1L;
    m_maxSamplingFactorY=1L;

    m_valuesMask = ((std::int32_t)1 << m_precision)-1;

    // Find the maximum sampling factor
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator channelsIterator0=m_channelsMap.begin(); channelsIterator0!=m_channelsMap.end(); ++channelsIterator0)
    {
        ptr<jpeg::jpegChannel> pChannel=channelsIterator0->second;

        if(pChannel->m_samplingFactorX>m_maxSamplingFactorX)
            m_maxSamplingFactorX=pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY>m_maxSamplingFactorY)
            m_maxSamplingFactorY=pChannel->m_samplingFactorY;
    }

    if(m_bLossless)
    {
        m_jpegImageSizeX=(m_imageSizeX+(m_maxSamplingFactorX-1))/m_maxSamplingFactorX;
        m_jpegImageSizeX*=m_maxSamplingFactorX;
        m_jpegImageSizeY=(m_imageSizeY+(m_maxSamplingFactorY-1))/m_maxSamplingFactorY;
        m_jpegImageSizeY*=m_maxSamplingFactorY;
    }
    else
    {
        m_jpegImageSizeX=(m_imageSizeX+((m_maxSamplingFactorX<<3)-1))/(m_maxSamplingFactorX<<3);
        m_jpegImageSizeX*=(m_maxSamplingFactorX<<3);
        m_jpegImageSizeY=(m_imageSizeY+((m_maxSamplingFactorY<<3)-1))/(m_maxSamplingFactorY<<3);
        m_jpegImageSizeY*=(m_maxSamplingFactorY<<3);
    }

    // Allocate the channels' buffers
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator channelsIterator1=m_channelsMap.begin(); channelsIterator1 != m_channelsMap.end(); ++channelsIterator1)
    {
        ptr<jpeg::jpegChannel> pChannel=channelsIterator1->second;
        pChannel->m_defaultDCValue = m_bLossless ? ((std::int32_t)1<<(m_precision - 1)) : 0;
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        pChannel->allocate(
                    m_jpegImageSizeX*(std::uint32_t)pChannel->m_samplingFactorX/m_maxSamplingFactorX,
                    m_jpegImageSizeY*(std::uint32_t)pChannel->m_samplingFactorY/m_maxSamplingFactorY);
        pChannel->m_valuesMask = m_valuesMask;
    }

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Find the MCU's size
// This function is called when a SOS tag is found
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::findMcuSize()
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::findMcuSize");

    // Find the maximum sampling factor for all the channels
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorChannelsX=1;
    std::uint32_t maxSamplingFactorChannelsY=1;
    for(tChannelsMap::iterator allChannelsIterator=m_channelsMap.begin(); allChannelsIterator!=m_channelsMap.end(); ++allChannelsIterator)
    {
        ptr<jpeg::jpegChannel> pChannel = allChannelsIterator->second;

        if(pChannel->m_samplingFactorX > maxSamplingFactorChannelsX)
            maxSamplingFactorChannelsX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY > maxSamplingFactorChannelsY)
            maxSamplingFactorChannelsY = pChannel->m_samplingFactorY;
    }


    // Find the minimum and maximum sampling factor in the scan
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorX=1;
    std::uint32_t maxSamplingFactorY=1;
    std::uint32_t minSamplingFactorX=256;
    std::uint32_t minSamplingFactorY=256;

    jpeg::jpegChannel* pChannel; // Used in the lòops
    for(jpeg::jpegChannel** channelsIterator = m_channelsList; *channelsIterator != 0; ++channelsIterator)
    {
        pChannel = *channelsIterator;

        if(pChannel->m_samplingFactorX > maxSamplingFactorX)
            maxSamplingFactorX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY > maxSamplingFactorY)
            maxSamplingFactorY = pChannel->m_samplingFactorY;
        if(pChannel->m_samplingFactorX < minSamplingFactorX)
            minSamplingFactorX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY < minSamplingFactorY)
            minSamplingFactorY = pChannel->m_samplingFactorY;
    }

    // Find the number of blocks per MCU per channel
    ///////////////////////////////////////////////////////////
    for(jpeg::jpegChannel** channelsIterator = m_channelsList; *channelsIterator != 0; ++channelsIterator)
    {
        pChannel=*channelsIterator;

        pChannel->m_blockMcuX=pChannel->m_samplingFactorX/minSamplingFactorX;
        pChannel->m_blockMcuY=pChannel->m_samplingFactorY/minSamplingFactorY;
        pChannel->m_blockMcuXY = pChannel->m_blockMcuX * pChannel->m_blockMcuY;
        pChannel->m_losslessPositionX = 0;
        pChannel->m_losslessPositionY = 0;
        pChannel->m_unprocessedAmplitudesCount = 0;
        pChannel->m_unprocessedAmplitudesPredictor = 0;
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;
    }

    // Find the MCU size, in image's pixels
    ///////////////////////////////////////////////////////////
    if(m_bLossless)
    {
        m_mcuNumberX = m_jpegImageSizeX * minSamplingFactorX / maxSamplingFactorChannelsX;
        m_mcuNumberY = m_jpegImageSizeY * minSamplingFactorY / maxSamplingFactorChannelsY;
    }
    else
    {
        std::uint32_t xBoundary = 8 * maxSamplingFactorChannelsX / minSamplingFactorX;
        std::uint32_t yBoundary = 8 * maxSamplingFactorChannelsY / minSamplingFactorY;

        m_mcuNumberX = (m_imageSizeX + xBoundary - 1) / xBoundary;
        m_mcuNumberY = (m_imageSizeY + yBoundary - 1) / yBoundary;
    }
    m_mcuNumberTotal = m_mcuNumberX*m_mcuNumberY;
    m_mcuProcessed = 0;
    m_mcuProcessedX = 0;
    m_mcuProcessedY = 0;


    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a DICOM dataset from a jpeg file
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodec::readStream(ptr<streamReader> pSourceStream, ptr<dataSet> pDataSet, std::uint32_t /* maxSizeBufferLoad = 0xffffffff */)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::readStream");

    streamReader* pStream = pSourceStream.get();

    // Reset the internal variables
    ///////////////////////////////////////////////////////////
    resetInternal(false, medium);

    // Store the stream's position.
    // This will be used later, in order to reread all the
    //  stream's content and store it into the dataset
    ///////////////////////////////////////////////////////////
    std::uint32_t startPosition=pStream->position();

    try
    {
        // Read the Jpeg signature
        ///////////////////////////////////////////////////////////
        std::uint8_t jpegSignature[2];
        try
        {
            pStream->read(jpegSignature, 2);
        }
        catch(streamExceptionEOF&)
        {
            PUNTOEXE_THROW(codecExceptionWrongFormat, "readStream detected a wrong format");
        }

        // If the jpeg signature is wrong, then return an error
        //  condition
        ///////////////////////////////////////////////////////////
        std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
        if(::memcmp(jpegSignature, checkSignature, 2) != 0)
        {
            PUNTOEXE_THROW(codecExceptionWrongFormat, "detected a wrong format");
        }
    }
    catch(streamExceptionEOF&)
    {
        PUNTOEXE_THROW(codecExceptionWrongFormat, "detected a wrong format");
    }

    // Used to read discharged chars
    ///////////////////////////////////////////////////////////
    std::uint8_t entryByte;

    // Read all the tags in the stream
    ///////////////////////////////////////////////////////////
    for(m_bEndOfImage=false; !m_bEndOfImage; /* empty */)
    {
        // If a tag has been found, then parse it
        ///////////////////////////////////////////////////////////
        pStream->read(&entryByte, 1);
        if(entryByte != 0xff)
        {
            continue;
        }
        do
        {
            pStream->read(&entryByte, 1);
        } while(entryByte == 0xff);

        if(entryByte != 0)
        {
            ptr<jpeg::tag> pTag;
            tTagsMap::iterator findTag = m_tagsMap.find(entryByte);
            if(findTag != m_tagsMap.end())
                pTag = findTag->second;
            else
                pTag=m_tagsMap[0xff];

            // Parse the tag
            ///////////////////////////////////////////////////////////
            pTag->readTag(pStream, this, entryByte);
        }
    }

    //
    // Build the dataset
    //
    ///////////////////////////////////////////////////////////

    // Color space
    ///////////////////////////////////////////////////////////
    if(m_channelsMap.size()==1)
        pDataSet->setUnicodeString(0x0028, 0, 0x0004, 0, L"MONOCHROME2");
    else
        pDataSet->setUnicodeString(0x0028, 0, 0x0004, 0, L"YBR_FULL");

    // Transfer syntax
    ///////////////////////////////////////////////////////////
    switch(m_process)
    {
    case 0x00:
        pDataSet->setUnicodeString(0x0002, 0, 0x0010, 0, L"1.2.840.10008.1.2.4.50");
        break;
    case 0x01:
        pDataSet->setUnicodeString(0x0002, 0, 0x0010, 0, L"1.2.840.10008.1.2.4.51");
        break;
    case 0x03:
        pDataSet->setUnicodeString(0x0002, 0, 0x0010, 0, L"1.2.840.10008.1.2.4.57");
        break;
    case 0x07:
        pDataSet->setUnicodeString(0x0002, 0, 0x0010, 0, L"1.2.840.10008.1.2.4.57");
        break;
    default:
        throw jpegCodecCannotHandleSyntax("Jpeg SOF not supported");
    }

    // Number of planes
    ///////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0002, 0, (std::uint32_t)m_channelsMap.size());

    // Image's width
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0011, 0, m_imageSizeX);

    // Image's height
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0010, 0, m_imageSizeY);

    // Number of frames
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0008, 0, 1);

    // Pixel representation
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0103, 0x0, 0);

    // Allocated, stored bits and high bit
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0100, 0x0, m_precision);
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0101, 0x0, m_precision);
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0102, 0x0, m_precision - 1);

    // Interleaved (more than 1 channel in the channels list)
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0006, 0x0, m_channelsList[0] != 0 && m_channelsList[1] != 0);

    // Insert the basic offset table
    ////////////////////////////////////////////////////////////////
    ptr<handlers::dataHandlerRaw> offsetHandler=pDataSet->getDataHandlerRaw(0x7fe0, 0, 0x0010, 0, true, "OB");
    offsetHandler->setSize(4);
    ::memset(offsetHandler->getMemoryBuffer(), 0, offsetHandler->getSize());

    // Reread all the stream's content and write it into the dataset
    ////////////////////////////////////////////////////////////////
    std::uint32_t finalPosition=pStream->position();
    std::uint32_t streamLength=(std::uint32_t)(finalPosition-startPosition);
    pStream->seek((std::int32_t)startPosition);

    ptr<handlers::dataHandlerRaw> imageHandler=pDataSet->getDataHandlerRaw(0x7fe0, 0, 0x0010, 1, true, "OB");
    if(imageHandler != 0 && streamLength != 0)
    {
        imageHandler->setSize(streamLength);
        pStream->read(imageHandler->getMemoryBuffer(), streamLength);
    }

    PUNTOEXE_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the specified transfer
//  syntax
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
bool jpegCodec::canHandleTransferSyntax(std::wstring transferSyntax)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::canHandleTransferSyntax");

    return (
                transferSyntax == L"1.2.840.10008.1.2.4.50" ||  // baseline (8 bits lossy)
                transferSyntax == L"1.2.840.10008.1.2.4.51" ||  // extended (12 bits lossy)
                transferSyntax == L"1.2.840.10008.1.2.4.57" ||  // lossless NH
                transferSyntax == L"1.2.840.10008.1.2.4.70");   // lossless NH first order prediction

    PUNTOEXE_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the transfer syntax has to be
//  encapsulated
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
bool jpegCodec::encapsulated(std::wstring transferSyntax)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::canHandleTransferSyntax");

    if(!canHandleTransferSyntax(transferSyntax))
    {
        PUNTOEXE_THROW(codecExceptionWrongTransferSyntax, "Cannot handle the transfer syntax");
    }
    return true;

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Return the highest bit that the transfer syntax can
//  handle
//
//
/////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t jpegCodec::getMaxHighBit(std::string transferSyntax)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::getMaxHighBit");

    if(transferSyntax == "1.2.840.10008.1.2.4.50")
    {
        return 7;
    }
    if(transferSyntax == "1.2.840.10008.1.2.4.51")
    {
        return 11;
    }
    return 15;

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the suggested allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t jpegCodec::suggestAllocatedBits(std::wstring transferSyntax, std::uint32_t highBit)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::suggestAllocatedBits");

    if(transferSyntax == L"1.2.840.10008.1.2.4.50")
    {
        return 8;
    }
    if(transferSyntax == L"1.2.840.10008.1.2.4.51")
    {
        return 12;
    }
    return (highBit + 8) & 0xfffffff8;

    PUNTOEXE_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Get a jpeg image from a Dicom dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
ptr<image> jpegCodec::getImage(ptr<dataSet> sourceDataSet, ptr<streamReader> pStream, std::string /* dataType not used */)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::getImage");

    streamReader* pSourceStream = pStream.get();

    // Reset the internal variables
    ////////////////////////////////////////////////////////////////
    resetInternal(false, medium);

    // Activate the tags in the stream
    ///////////////////////////////////////////////////////////
    pSourceStream->m_bJpegTags = true;

    // Read the Jpeg signature
    ///////////////////////////////////////////////////////////
    std::uint8_t jpegSignature[2];

    try
    {
        pSourceStream->read(jpegSignature, 2);
    }
    catch(streamExceptionEOF&)
    {
        PUNTOEXE_THROW(codecExceptionWrongFormat, "Jpeg signature not present");
    }

    // If the jpeg signature is wrong, then return an error
    //  condition
    ///////////////////////////////////////////////////////////
    static std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
    if(::memcmp(jpegSignature, checkSignature, 2) != 0)
    {
        PUNTOEXE_THROW(codecExceptionWrongFormat, "Jpeg signature not valid");
    }

    //
    // Preallocate the variables used in the loop
    //
    ///////////////////////////////////////////////////////////

    int scanBlock;              // scan lossless blocks
    int scanBlockX, scanBlockY; // scan lossy blocks
    std::uint32_t amplitudeLength; // lossless amplitude's length
    std::int32_t amplitude;        // lossless amplitude

    // Used to read the channels' content
    ///////////////////////////////////////////////////////////
    std::uint32_t bufferPointer = 0;

    // Read until the end of the image is reached
    ///////////////////////////////////////////////////////////
    for(m_bEndOfImage=false; !m_bEndOfImage; pSourceStream->resetInBitsBuffer())
    {
        std::uint32_t nextMcuStop = m_mcuNumberTotal;
        if(m_mcuPerRestartInterval != 0)
        {
            nextMcuStop = m_mcuLastRestart + m_mcuPerRestartInterval;
            if(nextMcuStop > m_mcuNumberTotal)
            {
                nextMcuStop = m_mcuNumberTotal;
            }
        }

        if(nextMcuStop <= m_mcuProcessed)
        {
            // Look for a tag. Skip all the FF bytes
            std::uint8_t tagId(0xff);

            try
            {
                pSourceStream->read(&tagId, 1);
                if(tagId != 0xff)
                {
                    continue;
                }

                while(tagId == 0xff)
                {
                    pSourceStream->read(&tagId, 1);
                }


                // An entry has been found. Process it
                ///////////////////////////////////////////////////////////
                ptr<jpeg::tag> pTag;
                if(m_tagsMap.find(tagId)!=m_tagsMap.end())
                    pTag=m_tagsMap[tagId];
                else
                    pTag=m_tagsMap[0xff];

                pTag->readTag(pSourceStream, this, tagId);
            }
            catch(const streamExceptionEOF& e)
            {
                if(m_mcuProcessed == m_mcuNumberTotal && m_mcuNumberTotal != 0)
                {
                    m_bEndOfImage = true;
                }
                else
                {
                    throw;
                }
            }
            continue;

        }

        jpeg::jpegChannel* pChannel; // Used in the loops
        while(m_mcuProcessed < nextMcuStop && !pSourceStream->endReached())
        {
            // Read an MCU
            ///////////////////////////////////////////////////////////

            // Scan all components
            ///////////////////////////////////////////////////////////
            for(jpeg::jpegChannel** channelsIterator = m_channelsList; *channelsIterator != 0; ++channelsIterator)
            {
                pChannel = *channelsIterator;

                // Read a lossless pixel
                ///////////////////////////////////////////////////////////
                if(m_bLossless)
                {
                    for(
                        scanBlock = 0;
                        scanBlock != pChannel->m_blockMcuXY;
                        ++scanBlock)
                    {
                        amplitudeLength = pChannel->m_pActiveHuffmanTableDC->readHuffmanCode(pSourceStream);
                        if(amplitudeLength)
                        {
                            amplitude = pSourceStream->readBits(amplitudeLength);
                            if(amplitude < ((std::int32_t)1<<(amplitudeLength-1)))
                            {
                                amplitude -= ((std::int32_t)1<<amplitudeLength)-1;
                            }
                        }
                        else
                        {
                            amplitude = 0;
                        }

                        pChannel->addUnprocessedAmplitude(amplitude, m_spectralIndexStart, m_mcuLastRestart == m_mcuProcessed && scanBlock == 0);
                    }

                    continue;
                }

                // Read a lossy MCU
                ///////////////////////////////////////////////////////////
                bufferPointer = (m_mcuProcessedY * pChannel->m_blockMcuY * ((m_jpegImageSizeX * pChannel->m_samplingFactorX / m_maxSamplingFactorX) >> 3) + m_mcuProcessedX * pChannel->m_blockMcuX) * 64;
                for(scanBlockY = pChannel->m_blockMcuY; (scanBlockY != 0); --scanBlockY)
                {
                    for(scanBlockX = pChannel->m_blockMcuX; scanBlockX != 0; --scanBlockX)
                    {
                        readBlock(pSourceStream, &(pChannel->m_pBuffer[bufferPointer]), pChannel);

                        if(m_spectralIndexEnd>=63 && m_bitLow==0)
                        {
                            IDCT(
                                        &(pChannel->m_pBuffer[bufferPointer]),
                                        m_decompressionQuantizationTable[pChannel->m_quantTable]
                                    );
                        }
                        bufferPointer += 64;
                    }
                    bufferPointer += (m_mcuNumberX -1) * pChannel->m_blockMcuX * 64;
                }
            }

            ++m_mcuProcessed;
            if(++m_mcuProcessedX == m_mcuNumberX)
            {
                m_mcuProcessedX = 0;
                ++m_mcuProcessedY;
            }
        }
    }

    // Process unprocessed lossless amplitudes
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator processLosslessIterator = m_channelsMap.begin();
        processLosslessIterator != m_channelsMap.end();
        ++processLosslessIterator)
    {
        processLosslessIterator->second->processUnprocessedAmplitudes();
    }


    // Check for 2's complement
    ///////////////////////////////////////////////////////////
    bool b2complement = sourceDataSet->getUnsignedLong(0x0028, 0, 0x0103, 0) != 0;
    std::wstring colorSpace = sourceDataSet->getUnicodeString(0x0028, 0, 0x0004, 0);

    // If the compression is jpeg baseline or jpeg extended
    //  then the color space cannot be "RGB"
    ///////////////////////////////////////////////////////////
    if(colorSpace == L"RGB")
    {
        std::wstring transferSyntax(sourceDataSet->getUnicodeString(0x0002, 0, 0x0010, 0));
        if(transferSyntax == L"1.2.840.10008.1.2.4.50" ||  // baseline (8 bits lossy)
                transferSyntax == L"1.2.840.10008.1.2.4.51")    // extended (12 bits lossy)
        {
            colorSpace = L"YBR_FULL";
        }
    }

    ptr<image> returnImage(new image());
    copyJpegChannelsToImage(returnImage, b2complement, colorSpace);

    return returnImage;

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Copy the loaded image into a class image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::copyJpegChannelsToImage(ptr<image> destImage, bool b2complement, std::wstring colorSpace)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::copyJpegChannelsToImage");

    image::bitDepth depth;
    if(b2complement)
        depth = (m_precision==8) ? image::depthS8 : image::depthS16;
    else
        depth = (m_precision==8) ? image::depthU8 : image::depthU16;

    ptr<handlers::dataHandlerNumericBase> handler = destImage->create(m_imageSizeX, m_imageSizeY, depth, colorSpace, (std::uint8_t)(m_precision-1));

    std::int32_t offsetValue=(std::int32_t)1<<(m_precision-1);
    std::int32_t maxClipValue=((std::int32_t)1<<m_precision)-1;
    std::int32_t minClipValue = 0;
    if(b2complement)
    {
        maxClipValue-=offsetValue;
        minClipValue-=offsetValue;
    }

    if(handler == 0)
    {
        return;
    }

    // Copy the jpeg channels into the new image
    ///////////////////////////////////////////////////////////
    std::uint32_t destChannelNumber = 0;
    for(tChannelsMap::iterator copyChannelsIterator=m_channelsMap.begin();
        copyChannelsIterator!=m_channelsMap.end();
        ++copyChannelsIterator)
    {
        ptr<jpeg::jpegChannel> pChannel = copyChannelsIterator->second;

        // Adjust 2complement
        ///////////////////////////////////////////////////////////
        std::int32_t* pChannelBuffer = pChannel->m_pBuffer;
        if(!m_bLossless && !b2complement)
        {
            for(std::uint32_t adjust2complement = pChannel->m_bufferSize; adjust2complement != 0; --adjust2complement, ++pChannelBuffer)
            {
                *pChannelBuffer += offsetValue;
                if(*pChannelBuffer < minClipValue)
                {
                    *pChannelBuffer = minClipValue;
                }
                else if(*pChannelBuffer > maxClipValue)
                {
                    *pChannelBuffer = maxClipValue;
                }
            }
        }
        else if(m_bLossless && b2complement)
        {
            for(std::uint32_t adjust2complement = pChannel->m_bufferSize; adjust2complement; --adjust2complement)
            {
                if(*pChannelBuffer & offsetValue)
                {
                    *pChannelBuffer |= ((std::int32_t)-1) << m_precision;
                }
                if(*pChannelBuffer < minClipValue)
                {
                    *pChannelBuffer = minClipValue;
                }
                else if(*pChannelBuffer > maxClipValue)
                {
                    *pChannelBuffer = maxClipValue;
                }
                ++pChannelBuffer;
            }
        }

        // If only one channel is present, then use the fast copy
        ///////////////////////////////////////////////////////////
        if(m_bLossless && m_channelsMap.size() == 1)
        {
            handler->copyFrom(pChannel->m_pBuffer, pChannel->m_bufferSize);
            return;
        }

        // Lossless interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t runX = m_maxSamplingFactorX / pChannel->m_samplingFactorX;
        std::uint32_t runY = m_maxSamplingFactorY / pChannel->m_samplingFactorY;
        if(m_bLossless)
        {
            handler->copyFromInt32Interleaved(
                        pChannel->m_pBuffer,
                        runX, runY,
                        0, 0, pChannel->m_sizeX * runX, pChannel->m_sizeY * runY,
                        destChannelNumber++,
                        m_imageSizeX, m_imageSizeY,
                        (std::uint32_t)m_channelsMap.size());

            continue;
        }

        // Lossy interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t totalBlocksY(pChannel->m_sizeY >> 3);
        std::uint32_t totalBlocksX(pChannel->m_sizeX >> 3);

        std::int32_t* pSourceBuffer(pChannel->m_pBuffer);

        std::uint32_t startRow(0);
        for(std::uint32_t scanBlockY = 0; scanBlockY < totalBlocksY; ++scanBlockY)
        {
            std::uint32_t startCol(0);
            std::uint32_t endRow(startRow + (runY << 3));

            for(std::uint32_t scanBlockX = 0; scanBlockX < totalBlocksX; ++scanBlockX)
            {
                std::uint32_t endCol = startCol + (runX << 3);
                handler->copyFromInt32Interleaved(
                            pSourceBuffer,
                            runX, runY,
                            startCol,
                            startRow,
                            endCol,
                            endRow,
                            destChannelNumber,
                            m_imageSizeX, m_imageSizeY,
                            (std::uint32_t)m_channelsMap.size());

                pSourceBuffer += 64;
                startCol = endCol;
            }
            startRow = endRow;
        }
        ++destChannelNumber;
    }

    PUNTOEXE_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Copy an image into the internal channels
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void jpegCodec::copyImageToJpegChannels(
        ptr<image> sourceImage,
        bool b2complement,
        std::uint8_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::copyImageToJpegChannels");

    std::wstring colorSpace = sourceImage->getColorSpace();
    sourceImage->getSize(&m_imageSizeX, &m_imageSizeY);
    m_precision = allocatedBits;

    // Create the channels
    ////////////////////////////////////////////////////////////////
    std::uint32_t rowSize, channelSize, channelsNumber;
    ptr<handlers::dataHandlerNumericBase>imageDataHandler = sourceImage->getDataHandler(false, &rowSize, &channelSize, &channelsNumber);

    for(std::uint8_t channelId = 0; channelId < (std::uint8_t)channelsNumber; ++channelId)
    {
        ptr<jpeg::jpegChannel> pChannel(new jpeg::jpegChannel);
        m_channelsMap[channelId] = pChannel;

        pChannel->m_huffmanTableAC = 0;
        pChannel->m_pActiveHuffmanTableAC = m_pHuffmanTableAC[0].get();
        pChannel->m_huffmanTableDC = 0;
        pChannel->m_pActiveHuffmanTableDC = m_pHuffmanTableDC[0].get();

        if(channelId == 0)
        {
            if(bSubSampledX)
            {
                ++(pChannel->m_samplingFactorX);
            }
            if(bSubSampledY)
            {
                ++(pChannel->m_samplingFactorY);
            }
            continue;
        }
        if(colorSpace != L"YBR_FULL" && colorSpace != L"YBR_PARTIAL")
        {
            continue;
        }
        pChannel->m_quantTable = 1;
        pChannel->m_huffmanTableDC = 1;
        pChannel->m_huffmanTableAC = 1;
        pChannel->m_pActiveHuffmanTableAC = m_pHuffmanTableAC[1].get();
        pChannel->m_pActiveHuffmanTableDC = m_pHuffmanTableDC[1].get();
    }
    allocChannels();

    std::int32_t offsetValue=(std::int32_t)1<<(m_precision-1);
    std::int32_t maxClipValue=((std::int32_t)1<<m_precision)-1;
    std::int32_t minClipValue = 0;
    if(b2complement)
    {
        maxClipValue-=offsetValue;
        minClipValue-=offsetValue;
    }

    // Copy the image into the jpeg channels
    ///////////////////////////////////////////////////////////
    std::uint32_t sourceChannelNumber = 0;
    for(tChannelsMap::iterator copyChannelsIterator=m_channelsMap.begin();
        copyChannelsIterator!=m_channelsMap.end();
        ++copyChannelsIterator)
    {
        ptr<jpeg::jpegChannel> pChannel = copyChannelsIterator->second;

        // If only one channel is present, then use the fast copy
        ///////////////////////////////////////////////////////////
        if(m_bLossless && m_channelsMap.size() == 1)
        {
            imageDataHandler->copyTo(pChannel->m_pBuffer, pChannel->m_bufferSize);
            continue;
        }

        // Lossless interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t runX = m_maxSamplingFactorX / pChannel->m_samplingFactorX;
        std::uint32_t runY = m_maxSamplingFactorY / pChannel->m_samplingFactorY;
        if(m_bLossless)
        {
            imageDataHandler->copyToInt32Interleaved(
                        pChannel->m_pBuffer,
                        runX, runY,
                        0, 0, pChannel->m_sizeX * runX, pChannel->m_sizeY * runY,
                        sourceChannelNumber++,
                        m_imageSizeX, m_imageSizeY,
                        (std::uint32_t)m_channelsMap.size());

            continue;
        }

        // Lossy interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t totalBlocksY = (pChannel->m_sizeY >> 3);
        std::uint32_t totalBlocksX = (pChannel->m_sizeX >> 3);

        std::int32_t* pDestBuffer = pChannel->m_pBuffer;

        std::uint32_t startRow = 0;
        for(std::uint32_t scanBlockY = 0; scanBlockY < totalBlocksY; ++scanBlockY)
        {
            std::uint32_t startCol = 0;
            std::uint32_t endRow = startRow + (runY << 3);

            for(std::uint32_t scanBlockX = 0; scanBlockX < totalBlocksX; ++scanBlockX)
            {
                std::uint32_t endCol = startCol + (runX << 3);
                imageDataHandler->copyToInt32Interleaved(
                            pDestBuffer,
                            runX, runY,
                            startCol,
                            startRow,
                            endCol,
                            endRow,
                            sourceChannelNumber,
                            m_imageSizeX, m_imageSizeY,
                            (std::uint32_t)m_channelsMap.size());

                pDestBuffer += 64;
                startCol = endCol;
            }
            startRow = endRow;
        }
        ++sourceChannelNumber;
    }


    for(tChannelsMap::iterator clipChannelsIterator = m_channelsMap.begin();
        clipChannelsIterator != m_channelsMap.end();
        ++clipChannelsIterator)
    {
        ptr<jpeg::jpegChannel> pChannel = clipChannelsIterator->second;

        // Clip the values
        ///////////////////////////////////////////////////////////
        std::int32_t* pChannelBuffer = pChannel->m_pBuffer;
        for(std::uint32_t clipValues = pChannel->m_bufferSize; clipValues; --clipValues)
        {
            if(*pChannelBuffer < minClipValue)
            {
                *pChannelBuffer = minClipValue;
            }
            if(*pChannelBuffer > maxClipValue)
            {
                *pChannelBuffer = maxClipValue;
            }
            ++pChannelBuffer;
        }

        // Adjust 2complement
        ///////////////////////////////////////////////////////////
        if(!m_bLossless && !b2complement)
        {
            pChannelBuffer = pChannel->m_pBuffer;
            for(std::uint32_t adjust2complement = pChannel->m_bufferSize; adjust2complement; --adjust2complement)
            {
                *(pChannelBuffer++) -= offsetValue;
            }
        }

        pChannelBuffer = pChannel->m_pBuffer;
        std::int32_t orValue   = ((std::int32_t) - 1) << m_precision;
        for(std::uint32_t adjustHighBits = pChannel->m_bufferSize; adjustHighBits != 0; --adjustHighBits)
        {
            if((*pChannelBuffer & offsetValue) != 0)
            {
                *pChannelBuffer |= orValue;
            }
            ++pChannelBuffer;
        }
    }

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write an image into the dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodec::setImage(
        ptr<streamWriter> pDestStream,
        ptr<image> pImage,
        std::wstring transferSyntax,
        quality imageQuality,
        std::string /* dataType */,
        std::uint8_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool b2Complement)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::setImage");

    streamWriter* pDestinationStream = pDestStream.get();

    // Activate the tags in the stream
    ///////////////////////////////////////////////////////////
    pDestinationStream->m_bJpegTags = true;

    // Reset the internal variables
    ////////////////////////////////////////////////////////////////
    resetInternal(true, imageQuality);

    m_bLossless = transferSyntax == L"1.2.840.10008.1.2.4.57" ||  // lossless NH
            transferSyntax == L"1.2.840.10008.1.2.4.70";    // lossless NH first order prediction

    copyImageToJpegChannels(pImage, b2Complement, allocatedBits, bSubSampledX, bSubSampledY);

    // Now write the jpeg stream
    ////////////////////////////////////////////////////////////////
    static std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
    pDestinationStream->write(checkSignature, 2);

    // Write the SOF tag
    ////////////////////////////////////////////////////////////////
    writeTag(pDestinationStream, m_bLossless ? sof3 : (m_precision <= 8 ? sof0 : sof1));

    // Write the quantization tables
    ////////////////////////////////////////////////////////////////
    writeTag(pDestinationStream, dqt);

    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase == 1)
        {
            // Write the huffman tables
            ////////////////////////////////////////////////////////////////
            writeTag(pDestinationStream, dht);
        }

        // Write the scans
        ////////////////////////////////////////////////////////////////
        memset(m_channelsList, 0, sizeof(m_channelsList));
        if(bInterleaved)
        {
            size_t scanChannels(0);
            for(tChannelsMap::iterator channelsIterator=m_channelsMap.begin();
                channelsIterator!=m_channelsMap.end();
                ++channelsIterator)
            {
                m_channelsList[scanChannels++] = channelsIterator->second.get();
            }
            writeScan(pDestinationStream, phase == 0);
        }
        else
        {
            for(tChannelsMap::iterator channelsIterator=m_channelsMap.begin();
                channelsIterator!=m_channelsMap.end();
                ++channelsIterator)
            {
                m_channelsList[0] = channelsIterator->second.get();
                writeScan(pDestinationStream, phase == 0);
            }
        }
    }

    writeTag(pDestinationStream, eoi);

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a single scan (SOS tag + channels)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodec::writeScan(streamWriter* pDestinationStream, bool bCalcHuffman)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::writeScan");

    findMcuSize();

    if(m_bLossless)
    {
        m_spectralIndexStart = 1;
        m_spectralIndexEnd = 0;
    }
    if(!bCalcHuffman)
    {
        writeTag(pDestinationStream, sos);
    }

    jpeg::jpegChannel* pChannel; // Used in the loops
    while(m_mcuProcessed < m_mcuNumberTotal)
    {
        // Write an MCU
        ///////////////////////////////////////////////////////////

        // Scan all components
        ///////////////////////////////////////////////////////////
        for(jpeg::jpegChannel** channelsIterator = m_channelsList; *channelsIterator != 0; ++channelsIterator)
        {
            pChannel = *channelsIterator;

            // Write a lossless pixel
            ///////////////////////////////////////////////////////////
            if(m_bLossless)
            {
                std::int32_t lastValue = pChannel->m_lastDCValue;
                std::int32_t* pBuffer = pChannel->m_pBuffer + pChannel->m_losslessPositionY * pChannel->m_sizeX + pChannel->m_losslessPositionX;

                for(int scanBlock = pChannel->m_blockMcuXY; scanBlock != 0; --scanBlock)
                {
                    std::int32_t value(*pBuffer);
                    if(pChannel->m_losslessPositionX == 0 && pChannel->m_losslessPositionY != 0)
                    {
                        lastValue = *(pBuffer - pChannel->m_sizeX);
                    }
                    ++pBuffer;
                    std::int32_t diff = value - lastValue;
                    std::int32_t diff1 = value + ((std::int32_t)1 << m_precision) - lastValue;
                    std::int32_t diff2 = value - ((std::int32_t)1 << m_precision) - lastValue;
                    if(labs(diff1) < labs(diff))
                    {
                        diff = diff1;
                    }
                    if(labs(diff2) < labs(diff))
                    {
                        diff = diff2;
                    }

                    // Calculate amplitude and build the huffman table
                    std::uint32_t amplitudeLength = 0;
                    std::uint32_t amplitude = 0;
                    if(diff != 0)
                    {
                        amplitude = diff > 0 ? diff : -diff;
                        for(amplitudeLength = 32; (amplitude & ((std::uint32_t)1 << (amplitudeLength -1))) == 0; --amplitudeLength){};

                        if(diff < 0)
                        {
                            amplitude = ((std::uint32_t)1 << amplitudeLength) + diff - 1;
                        }

                    }

                    pChannel->m_lastDCValue = value;
                    if(++(pChannel->m_losslessPositionX) == pChannel->m_sizeX)
                    {
                        ++(pChannel->m_losslessPositionY);
                        pChannel->m_losslessPositionX = 0;
                    }

                    if(bCalcHuffman)
                    {
                        pChannel->m_pActiveHuffmanTableDC->incValueFreq(amplitudeLength);
                        continue;
                    }
                    pChannel->m_pActiveHuffmanTableDC->writeHuffmanCode(amplitudeLength, pDestinationStream);
                    pDestinationStream->writeBits(amplitude, amplitudeLength);
                }

                continue;
            }

            // write a lossy MCU
            ///////////////////////////////////////////////////////////
            std::uint32_t bufferPointer = (m_mcuProcessedY * pChannel->m_blockMcuY * ((m_jpegImageSizeX * pChannel->m_samplingFactorX / m_maxSamplingFactorX) >> 3) + m_mcuProcessedX * pChannel->m_blockMcuX) * 64;

            for(int scanBlockY = 0; scanBlockY < pChannel->m_blockMcuY; ++scanBlockY)
            {
                for(int scanBlockX = 0; scanBlockX < pChannel->m_blockMcuX; ++scanBlockX)
                {
                    writeBlock(pDestinationStream, &(pChannel->m_pBuffer[bufferPointer]), pChannel, bCalcHuffman);
                    bufferPointer += 64;
                }
                bufferPointer += (m_mcuNumberX -1) * pChannel->m_blockMcuX * 64;
            }
        }

        ++m_mcuProcessed;
        if(++m_mcuProcessedX == m_mcuNumberX)
        {
            m_mcuProcessedX = 0;
            ++m_mcuProcessedY;
        }
    }

    if(!bCalcHuffman)
    {
        pDestinationStream->resetOutBitsBuffer();
    }

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write a single jpeg tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodec::writeTag(streamWriter* pDestinationStream, tTagId tagId)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::writeTag");

    ptr<jpeg::tag> pTag;
    tTagsMap::iterator findTag = m_tagsMap.find((std::uint8_t)tagId);
    if(findTag == m_tagsMap.end())
    {
        return;
    }
    static std::uint8_t ff(0xff);
    std::uint8_t byteTagId(tagId);
    pDestinationStream->write(&ff, 1);
    pDestinationStream->write(&byteTagId, 1);
    findTag->second->writeTag(pDestinationStream, this);

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read a single MCU's block.
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
inline void jpegCodec::readBlock(streamReader* pStream, std::int32_t* pBuffer, jpeg::jpegChannel* pChannel)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::readBlock");

    // Scan all the requested spectral values
    /////////////////////////////////////////////////////////////////
    std::uint32_t spectralIndex(m_spectralIndexStart);

    // If an eob-run is defined, then don't read the DC value
    /////////////////////////////////////////////////////////////////
    if(m_eobRun && spectralIndex == 0)
    {
        ++spectralIndex;
    }

    std::uint32_t amplitude;
    std::uint32_t hufCode;
    std::int32_t value = 0;
    std::int32_t oldValue;
    std::uint8_t amplitudeLength;
    int runLength;
    std::uint32_t tempEobRun;
    std::int32_t positiveBitLow((std::int32_t)1 << m_bitLow);
    std::int32_t negativeBitLow((std::int32_t)-1 << m_bitLow);

    // Scan the specified spectral values
    /////////////////////////////////////////////////////////////////
    for(; spectralIndex <= m_spectralIndexEnd; ++spectralIndex)
    {
        // Read AC progressive bits for non-zero coefficients
        /////////////////////////////////////////////////////////////////
        if(m_eobRun != 0)
        {
            if(m_bitHigh == 0)
            {
                break;
            }
            oldValue = pBuffer[JpegDeZigZagOrder[spectralIndex]];
            if(oldValue == 0)
            {
                continue;
            }

            amplitude = pStream->readBit();

            if(amplitude != 0 && (oldValue & positiveBitLow)==0)
            {
                oldValue += (oldValue>0 ? positiveBitLow : negativeBitLow);
                pBuffer[JpegDeZigZagOrder[spectralIndex]] = oldValue;
            }
            continue;
        }

        //
        // AC/DC pass
        //
        /////////////////////////////////////////////////////////////////
        if(spectralIndex != 0)
        {
            hufCode = pChannel->m_pActiveHuffmanTableAC->readHuffmanCode(pStream);

            // End of block reached
            /////////////////////////////////////////////////////////////////
            if(hufCode == 0)
            {
                ++m_eobRun;
                --spectralIndex;
                continue;
            }
        }
        else
        {
            // First pass
            /////////////////////////////////////////////////////////////////
            if(m_bitHigh)
            {
                hufCode = pStream->readBit();
                value=(int)hufCode;
                hufCode = 0;
            }
            else
            {
                hufCode = pChannel->m_pActiveHuffmanTableDC->readHuffmanCode(pStream);
            }
        }


        //
        // Get AC or DC amplitude or zero run
        //
        /////////////////////////////////////////////////////////////////

        // Find bit coded coeff. amplitude
        /////////////////////////////////////////////////////////////////
        amplitudeLength=(std::uint8_t)(hufCode & 0xf);

        // Find zero run length
        /////////////////////////////////////////////////////////////////
        runLength=(int)(hufCode>>4);

        // First DC or AC pass or refine AC pass but not EOB run
        /////////////////////////////////////////////////////////////////
        if(spectralIndex == 0 || amplitudeLength != 0 || runLength == 0xf)
        {
            //
            // First DC pass and all the AC passes are similar,
            //  then use the same algorithm
            //
            /////////////////////////////////////////////////////////////////
            if(m_bitHigh == 0 || spectralIndex != 0)
            {
                // Read coeff
                /////////////////////////////////////////////////////////////////
                if(amplitudeLength != 0)
                {
                    value = (std::int32_t)(pStream->readBits(amplitudeLength));
                    if(value < ((std::int32_t)1 << (amplitudeLength-1)) )
                    {
                        value -= ((std::int32_t)1 << amplitudeLength) - 1;
                    }
                }
                else
                {
                    value = 0;
                }

                // Move spectral index forward by zero run length
                /////////////////////////////////////////////////////////////////
                if(m_bitHigh && spectralIndex)
                {
                    // Read the correction bits
                    /////////////////////////////////////////////////////////////////
                    for(/* none */; spectralIndex <= m_spectralIndexEnd; ++spectralIndex)
                    {
                        oldValue = pBuffer[JpegDeZigZagOrder[spectralIndex]];
                        if(oldValue != 0)
                        {
                            amplitude = pStream->readBit();
                            if(amplitude != 0 && (oldValue & positiveBitLow) == 0)
                            {
                                oldValue += (oldValue>0 ? positiveBitLow : negativeBitLow);
                                pBuffer[JpegDeZigZagOrder[spectralIndex]] = oldValue;
                            }
                            continue;
                        }
                        if(runLength == 0)
                        {
                            break;
                        }
                        --runLength;
                    }
                }
                else
                {
                    spectralIndex += runLength;
                    runLength = 0;
                }
            }

            // Store coeff.
            /////////////////////////////////////////////////////////////////
            if(spectralIndex<=m_spectralIndexEnd)
            {
                oldValue = value<<m_bitLow;
                if(m_bitHigh)
                    oldValue |= pBuffer[JpegDeZigZagOrder[spectralIndex]];

                // DC coeff added to the previous value.
                /////////////////////////////////////////////////////////////////
                if(spectralIndex == 0 && m_bitHigh == 0)
                {
                    oldValue += pChannel->m_lastDCValue;
                    pChannel->m_lastDCValue=oldValue;
                }
                pBuffer[JpegDeZigZagOrder[spectralIndex]]=oldValue;
            }
        } // ----- End of first DC or AC pass or refine AC pass but not EOB run

        // EOB run found
        /////////////////////////////////////////////////////////////////
        else
        {
            tempEobRun = pStream->readBits(runLength);
            m_eobRun+=(std::uint32_t)1<<runLength;
            m_eobRun+=tempEobRun;
            --spectralIndex;
        }
    }

    //
    // EOB run processor
    //
    /////////////////////////////////////////////////////////////////
    if(m_eobRun != 0)
        m_eobRun--;

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write a single MCU's block.
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
inline void jpegCodec::writeBlock(streamWriter* pStream, std::int32_t* pBuffer, jpeg::jpegChannel* pChannel, bool bCalcHuffman)
{
    PUNTOEXE_FUNCTION_START(L"jpegCodec::writeBlock");

    if(bCalcHuffman)
    {
        FDCT(pBuffer, m_compressionQuantizationTable[pChannel->m_quantTable]);
    }

    // Scan the specified spectral values
    /////////////////////////////////////////////////////////////////
    std::uint8_t zeroRun = 0;
    std::int32_t value;
    const std::uint32_t* pJpegDeZigZagOrder(&(JpegDeZigZagOrder[m_spectralIndexStart]));
    huffmanTable* pActiveHuffmanTable;

    for(std::uint32_t spectralIndex=m_spectralIndexStart; spectralIndex <= m_spectralIndexEnd; ++spectralIndex)
    {
        value = pBuffer[*(pJpegDeZigZagOrder++)];

        if(value > 32767)
        {
            value = 32767;
        }
        else if(value < -32767)
        {
            value = -32767;
        }
        if(spectralIndex == 0)
        {
            value -= pChannel->m_lastDCValue;
            pChannel->m_lastDCValue += value;
            pActiveHuffmanTable = pChannel->m_pActiveHuffmanTableDC;
        }
        else
        {
            pActiveHuffmanTable = pChannel->m_pActiveHuffmanTableAC;
            if(value == 0)
            {
                ++zeroRun;
                continue;
            }
        }

        //Write out the zero runs
        /////////////////////////////////////////////////////////////////
        while(zeroRun >= 16)
        {
            zeroRun -= 16;
            static std::uint32_t zeroRunCode = 0xf0;
            if(bCalcHuffman)
            {
                pActiveHuffmanTable->incValueFreq(zeroRunCode);
                continue;
            }
            pActiveHuffmanTable->writeHuffmanCode(zeroRunCode, pStream);
        }

        std::uint32_t hufCode = (zeroRun << 4);
        zeroRun = 0;

        // Write out the value
        /////////////////////////////////////////////////////////////////
        std::uint8_t amplitudeLength = 0;
        std::uint32_t amplitude = 0;
        if(value != 0)
        {
            amplitude = (value > 0) ? value : -value;
            for(amplitudeLength = 15; (amplitude & ((std::uint32_t)1 << (amplitudeLength -1))) == 0; --amplitudeLength){};

            if(value < 0)
            {
                amplitude = ((std::uint32_t)1 << amplitudeLength) + value -1;
            }
            hufCode |= amplitudeLength;
        }

        if(bCalcHuffman)
        {
            pActiveHuffmanTable->incValueFreq(hufCode);
            continue;
        }
        pActiveHuffmanTable->writeHuffmanCode(hufCode, pStream);
        if(amplitudeLength != 0)
        {
            pStream->writeBits(amplitude, amplitudeLength);
        }
    }

    if(zeroRun == 0)
    {
        return;
    }

    static std::uint32_t zero = 0;
    if(bCalcHuffman)
    {
        pChannel->m_pActiveHuffmanTableAC->incValueFreq(zero);
        return;
    }
    pChannel->m_pActiveHuffmanTableAC->writeHuffmanCode(zero, pStream);

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the quantization tables with the correction
//  factor needed by the IDCT/FDCT
//
//
///////////////////////////////////////////////////////////;
///////////////////////////////////////////////////////////
void jpegCodec::recalculateQuantizationTables(int table)
{
    // Adjust the tables for compression/decompression
    ///////////////////////////////////////////////////////////
    std::uint8_t tableIndex = 0;
    for(std::uint8_t row = 0; row<8; ++row)
    {
        for(std::uint8_t col = 0; col<8; ++col)
        {
            m_decompressionQuantizationTable[table][tableIndex]=(long long)((float)((m_quantizationTable[table][tableIndex])<<JPEG_DECOMPRESSION_BITS_PRECISION)*JpegDctScaleFactor[col]*JpegDctScaleFactor[row]);
            m_compressionQuantizationTable[table][tableIndex]=1.0f/((float)((m_quantizationTable[table][tableIndex])<<3)*JpegDctScaleFactor[col]*JpegDctScaleFactor[row]);
            ++tableIndex;
        }
    }
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Calc FDCT on MCU
// This routine comes from the IJG software version 6b
//
// Values must be Zero centered (-x...0...+x)
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodec::FDCT(std::int32_t* pIOMatrix, float* pDescaleFactors)
{
    // Temporary values
    /////////////////////////////////////////////////////////////////
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z1, z2, z3, z4, z5, z11, z13;

    // Rows FDCT
    /////////////////////////////////////////////////////////////////
    std::int32_t *pointerOperator0, *pointerOperator1;
    for(int scanBlockY=0; scanBlockY<64; scanBlockY+=8)
    {
        pointerOperator0 = &(pIOMatrix[scanBlockY]);
        pointerOperator1 = &(pIOMatrix[scanBlockY + 7]);
        tmp0 = (float)(*pointerOperator0 + *pointerOperator1);
        tmp7 = (float)(*pointerOperator0 - *pointerOperator1);

        tmp1 = (float)(*(++pointerOperator0) + *(--pointerOperator1));
        tmp6 = (float)(*pointerOperator0 - *pointerOperator1);

        tmp2 = (float)(*(++pointerOperator0) + *(--pointerOperator1));
        tmp5 = (float)(*pointerOperator0 - *pointerOperator1);

        tmp3 = (float)(*(++pointerOperator0) + *(--pointerOperator1));
        tmp4 = (float)(*pointerOperator0 - *pointerOperator1);

        // Phase 2
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        // Phase 3
        m_fdctTempMatrix[scanBlockY]   = tmp10 + tmp11;
        m_fdctTempMatrix[scanBlockY+4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13)*0.707106781f;     // c4

        // Phase 5
        m_fdctTempMatrix[scanBlockY+2] = tmp13 + z1;
        m_fdctTempMatrix[scanBlockY+6] = tmp13 - z1;

        // Odd part
        // Phase 2
        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        // The rotator is modified from fig 4-8 to avoid extra negations.
        z5 =  (tmp10 - tmp12)*0.382683433f;    // c6
        z2 =  tmp10*0.541196100f + z5;         // c2-c6
        z4 =  tmp12*1.306562965f + z5;         // c2+c6
        z3 =  tmp11*0.707106781f;              // c4

        // Phase 5
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;

        // Phase 6
        m_fdctTempMatrix[scanBlockY+5] = z13 + z2;
        m_fdctTempMatrix[scanBlockY+3] = z13 - z2;
        m_fdctTempMatrix[scanBlockY+1] = z11 + z4;
        m_fdctTempMatrix[scanBlockY+7] = z11 - z4;
    }

    // Columns FDCT
    /////////////////////////////////////////////////////////////////
    float *pointerOperatorFloat0, *pointerOperatorFloat1;
    for(int scanBlockX = 0; scanBlockX < 8; ++scanBlockX)
    {
        pointerOperatorFloat0 = &(m_fdctTempMatrix[scanBlockX]);
        pointerOperatorFloat1 = &(m_fdctTempMatrix[scanBlockX + 56]);

        tmp0 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp7 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        pointerOperatorFloat0 += 8;
        pointerOperatorFloat1 -= 8;
        tmp1 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp6 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        pointerOperatorFloat0 += 8;
        pointerOperatorFloat1 -= 8;
        tmp2 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp5 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        pointerOperatorFloat0 += 8;
        pointerOperatorFloat1 -= 8;
        tmp3 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp4 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        // Even part
        // Phase 2
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        // Phase 3
        m_fdctTempMatrix[scanBlockX   ] = tmp10 + tmp11;
        m_fdctTempMatrix[scanBlockX+32] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13)*0.707106781f;     // c4

        // Phase 5
        m_fdctTempMatrix[scanBlockX+16] = (tmp13 + z1);
        m_fdctTempMatrix[scanBlockX+48] = (tmp13 - z1);

        // Odd part
        // Phase 2
        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        // Avoid extra negations.
        z5 =  (tmp10 - tmp12)*0.382683433f;    // c6
        z2 =  tmp10*0.541196100f + z5;         // c2-c6
        z4 =  tmp12*1.306562965f + z5;         // c2+c6
        z3 =  tmp11*0.707106781f;              // c4

        // Phase 5
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;

        // Phase 6
        m_fdctTempMatrix[scanBlockX+40] = (z13 + z2);
        m_fdctTempMatrix[scanBlockX+24] = (z13 - z2);
        m_fdctTempMatrix[scanBlockX+ 8] = (z11 + z4);
        m_fdctTempMatrix[scanBlockX+56] = (z11 - z4);
    }

    // Descale FDCT results
    /////////////////////////////////////////////////////////////////
    for(int descale = 0; descale < 64; ++descale)
        pIOMatrix[descale]=(std::int32_t)(m_fdctTempMatrix[descale]*pDescaleFactors[descale]+.5f);

}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Calc IDCT on MCU
// This routine comes from the IJG software version 6b
//
// Values must be Zero centered (-x...0...+x)
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodec::IDCT(std::int32_t* pIOMatrix, long long* pScaleFactors)
{
    static const double multiplier((float)((long long)1 << JPEG_DECOMPRESSION_BITS_PRECISION));
    static const long long multiplier_1_414213562f((long long)(multiplier * 1.414213562f + .5f));
    static const long long multiplier_1_847759065f((long long)(multiplier * 1.847759065f + .5f));
    static const long long multiplier_1_0823922f((long long)(multiplier * 1.0823922f + .5f));
    static const long long multiplier_2_61312593f((long long)(multiplier * 2.61312593f + .5f));
    static const long long zero_point_five((long long)1 << (JPEG_DECOMPRESSION_BITS_PRECISION - 1));
    static const long long zero_point_five_by_8((std::int32_t)zero_point_five << 3);


    // Temporary values
    /////////////////////////////////////////////////////////////////
    long long tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    long long tmp10, tmp11, tmp12, tmp13;
    long long z5, z10, z11, z12, z13;

    //
    // Rows IDCT
    //
    /////////////////////////////////////////////////////////////////
    std::int32_t* pMatrix(pIOMatrix);
    std::int32_t* pCheckMatrix(pIOMatrix);

    std::int32_t checkZero;
    long long* pTempMatrix(m_idctTempMatrix);

    for(int scanBlockY(8); scanBlockY != 0; --scanBlockY)
    {
        checkZero = *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        ++pCheckMatrix; // Point pCheckMatrix to the next row

        // Check for AC coefficients value.
        // If they are all NULL, then apply the DC value to all
        /////////////////////////////////////////////////////////////////
        if(checkZero == 0)
        {
            tmp0 = (long long)(*pMatrix) * (*pScaleFactors);
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            pMatrix = pCheckMatrix;
            pScaleFactors += 8;
            continue;
        }

        tmp0 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp4 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp1 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp5 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp2 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp6 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp3 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp7 = (long long)*pMatrix++ * (*pScaleFactors++);

        // Phase 3
        tmp10 = tmp0 + tmp2;
        tmp11 = tmp0 - tmp2;

        // Phases 5-3
        tmp13 = tmp1 + tmp3;
        tmp12 = (((tmp1 - tmp3) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp13; // 2*c4

        // Phase 2
        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        // Phase 6
        z13 = tmp6 + tmp5;
        z10 = tmp6 - tmp5;
        z11 = tmp4 + tmp7;
        z12 = tmp4 - tmp7;

        // Phase 5
        tmp7 = z11 + z13;
        z5 = ((z10 + z12) * multiplier_1_847759065f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION;    // 2*c2

        // Phase 2
        tmp6 = z5 - ((z10 *multiplier_2_61312593f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp7;
        tmp5 = (((z11 - z13) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp6;
        tmp4 = ((z12 * multiplier_1_0823922f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - z5 + tmp5;

        *(pTempMatrix++) = tmp0 + tmp7;
        *(pTempMatrix++) = tmp1 + tmp6;
        *(pTempMatrix++) = tmp2 + tmp5;
        *(pTempMatrix++) = tmp3 - tmp4;
        *(pTempMatrix++) = tmp3 + tmp4;
        *(pTempMatrix++) = tmp2 - tmp5;
        *(pTempMatrix++) = tmp1 - tmp6;
        *(pTempMatrix++) = tmp0 - tmp7;
    }

    //
    // Columns IDCT
    //
    /////////////////////////////////////////////////////////////////
    pMatrix = pIOMatrix;
    pTempMatrix = m_idctTempMatrix;
    for(int scanBlockX(8); scanBlockX != 0; --scanBlockX)
    {
        tmp0 = *pTempMatrix;
        pTempMatrix += 8;
        tmp4 = *pTempMatrix;
        pTempMatrix += 8;
        tmp1 = *pTempMatrix;
        pTempMatrix += 8;
        tmp5 = *pTempMatrix;
        pTempMatrix += 8;
        tmp2 = *pTempMatrix;
        pTempMatrix += 8;
        tmp6 = *pTempMatrix;
        pTempMatrix += 8;
        tmp3 = *pTempMatrix;
        pTempMatrix += 8;
        tmp7 = *pTempMatrix;
        pTempMatrix -= 55;

        // Phase 3
        tmp10 = tmp0 + tmp2;
        tmp11 = tmp0 - tmp2;

        // Phases 5-3
        tmp13 = tmp1 + tmp3;
        tmp12 = (((tmp1 - tmp3) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp13; // 2*c4

        // Phase 2
        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        // Phase 6
        z13 = tmp6 + tmp5;
        z10 = tmp6 - tmp5;
        z11 = tmp4 + tmp7;
        z12 = tmp4 - tmp7;

        // Phase 5
        tmp7 = z11 + z13;
        tmp11 = ((z11 - z13) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION; // 2*c4

        z5 = ((z10 + z12) * multiplier_1_847759065f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION;    // 2*c2
        tmp10 = ((z12 * multiplier_1_0823922f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - z5;    // 2*(c2-c6)
        tmp12 = z5 - ((z10 *multiplier_2_61312593f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION);      // -2*(c2+c6)

        // Phase 2
        tmp6 = tmp12 - tmp7;
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 + tmp5;

        // Final output stage: scale down by a factor of 8 (+JPEG_DECOMPRESSION_BITS_PRECISION bits)
        *pMatrix = (std::int32_t)((tmp0 + tmp7 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp1 + tmp6 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp2 + tmp5 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp3 - tmp4 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp3 + tmp4 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp2 - tmp5 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp1 - tmp6 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp0 - tmp7 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix -= 55;
    }
}


namespace jpeg
{
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegChannel
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void jpegChannel::processUnprocessedAmplitudes()
{
    if(m_unprocessedAmplitudesCount == 0)
    {
        return;
    }

    std::int32_t* pDest = m_pBuffer + (m_losslessPositionY * m_sizeX + m_losslessPositionX);
    std::int32_t* pSource = m_unprocessedAmplitudesBuffer;

    // Find missing pixels
    std::int32_t missingPixels = (std::int32_t)m_sizeX - (std::int32_t)m_losslessPositionX + (std::int32_t)m_sizeX * ((std::int32_t)m_sizeY - (std::int32_t)m_losslessPositionY - 1);
    if(missingPixels < (std::int32_t)m_unprocessedAmplitudesCount)
    {
        throw codecExceptionCorruptedFile("Excess data in the lossless jpeg stream");
    }

    if(m_unprocessedAmplitudesPredictor == 0)
    {
        while(m_unprocessedAmplitudesCount != 0)
        {
            --m_unprocessedAmplitudesCount;
            *(pDest++) = *(pSource++) & m_valuesMask;
            if(++m_losslessPositionX == m_sizeX)
            {
                m_losslessPositionX = 0;
                ++m_losslessPositionY;
            }
        }
        m_lastDCValue = *(pDest - 1);
        return;
    }

    int applyPrediction;
    std::int32_t* pPreviousLine = pDest - m_sizeX;
    std::int32_t* pPreviousLineColumn = pDest - m_sizeX - 1;
    while(m_unprocessedAmplitudesCount != 0)
    {
        --m_unprocessedAmplitudesCount;
        applyPrediction = (int)m_unprocessedAmplitudesPredictor;
        if(m_losslessPositionY == 0)
        {
            applyPrediction = 1;
        }
        else if(m_losslessPositionX == 0)
        {
            applyPrediction = 2;
        }
        switch(applyPrediction)
        {
        case 1:
            m_lastDCValue += *(pSource++);
            break;
        case 2:
            m_lastDCValue = *(pSource++) + *pPreviousLine;
            break;
        case 3:
            m_lastDCValue = *(pSource++) + *pPreviousLineColumn;
            break;
        case 4:
            m_lastDCValue += *(pSource++) + *pPreviousLine - *pPreviousLineColumn;
            break;
        case 5:
            m_lastDCValue += *(pSource++) + ((*pPreviousLine - *pPreviousLineColumn)>>1);
            break;
        case 6:
            m_lastDCValue -= *pPreviousLineColumn;
            m_lastDCValue >>= 1;
            m_lastDCValue += *(pSource++) + *pPreviousLine;
            break;
        case 7:
            m_lastDCValue += *pPreviousLine;
            m_lastDCValue >>= 1;
            m_lastDCValue += *(pSource++);
            break;
        default:
            throw codecExceptionCorruptedFile("Wrong predictor index in lossless jpeg stream");
        }

        m_lastDCValue &= m_valuesMask;
        *pDest++ = m_lastDCValue;

        ++pPreviousLine;
        ++pPreviousLineColumn;
        if(++m_losslessPositionX == m_sizeX)
        {
            ++m_losslessPositionY;
            m_losslessPositionX = 0;
        }
    }
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTag
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tag::writeLength(streamWriter* pStream, std::uint16_t length)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tag::writeLength");

    length+=sizeof(length);
    pStream->adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
    pStream->write((std::uint8_t*)&length, sizeof(length));

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t tag::readLength(streamReader* pStream)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tag::readLength");

    std::uint16_t length = 0;
    pStream->read((std::uint8_t*)&length, sizeof(length));
    pStream->adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
    if(length > 1)
        length -= 2;
    return (std::int32_t)((std::uint32_t)length);

    PUNTOEXE_FUNCTION_END();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagUnknown
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagUnknown::writeTag(streamWriter* pStream, jpegCodec* /* pCodec */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagUnknown::writeTag");

    writeLength(pStream, 0);

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagUnknown::readTag(streamReader* pStream, jpegCodec* /* pCodec */, std::uint8_t /* tagEntry */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagUnknown::readTag");

    std::int32_t tagLength=readLength(pStream);
    pStream->seek(tagLength, true);

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagSOF
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagSOF::writeTag(streamWriter* pStream, jpegCodec* pCodec)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagSOF::writeTag");

    // Calculate the components number
    ///////////////////////////////////////////////////////////
    std::uint8_t componentsNumber = (std::uint8_t)(pCodec->m_channelsMap.size());

    // Write the tag's length
    ///////////////////////////////////////////////////////////
    writeLength(pStream, 6+componentsNumber*3);

    // Write the precision, in bits
    ///////////////////////////////////////////////////////////
    std::uint8_t precisionBits=(std::uint8_t)pCodec->m_precision;
    pStream->write(&precisionBits, 1);

    // Write the image's size, in pixels
    ///////////////////////////////////////////////////////////
    std::uint16_t imageSizeX=(std::uint16_t)pCodec->m_imageSizeX;
    std::uint16_t imageSizeY=(std::uint16_t)pCodec->m_imageSizeY;
    pStream->adjustEndian((std::uint8_t*)&imageSizeY, 2, streamController::highByteEndian);
    pStream->adjustEndian((std::uint8_t*)&imageSizeX, 2, streamController::highByteEndian);
    pStream->write((std::uint8_t*)&imageSizeY, 2);
    pStream->write((std::uint8_t*)&imageSizeX, 2);

    // write the components number
    ///////////////////////////////////////////////////////////
    pStream->write((std::uint8_t*)&componentsNumber, 1);

    // Write all the components specifications
    ///////////////////////////////////////////////////////////
    std::uint8_t componentId;
    std::uint8_t componentSamplingFactor;
    std::uint8_t componentQuantTable;

    for(jpegCodec::tChannelsMap::iterator channelsIterator=pCodec->m_channelsMap.begin(); channelsIterator!=pCodec->m_channelsMap.end(); ++channelsIterator)
    {
        ptrChannel pChannel=channelsIterator->second;

        componentId=channelsIterator->first;
        componentSamplingFactor=((std::uint8_t)pChannel->m_samplingFactorX<<4) | ((std::uint8_t)pChannel->m_samplingFactorY);
        componentQuantTable=(std::uint8_t)pChannel->m_quantTable;

        pStream->write((std::uint8_t*)&componentId, 1);
        pStream->write((std::uint8_t*)&componentSamplingFactor, 1);
        pStream->write((std::uint8_t*)&componentQuantTable, 1);
    }

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the tag's content
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOF::readTag(streamReader* pStream, jpegCodec* pCodec, std::uint8_t tagEntry)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagSOF::readTag");

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes
    //////////////////////////////////////////////////////////
    const std::int32_t tagLength = readLength(pStream);
    ptr<streamReader> tagReader(pStream->getReader(tagLength));

    pCodec->m_bLossless = (tagEntry==0xc3) || (tagEntry==0xc7);
    pCodec->m_process = tagEntry - 0xc0;

    // Read the precision, in bits
    ///////////////////////////////////////////////////////////
    std::uint8_t precisionBits;
    tagReader->read(&precisionBits, 1);
    pCodec->m_precision = (int)precisionBits;

    // Read the image's size, in pixels
    ///////////////////////////////////////////////////////////
    std::uint16_t imageSizeX, imageSizeY;
    tagReader->read((std::uint8_t*)&imageSizeY, 2);
    tagReader->read((std::uint8_t*)&imageSizeX, 2);
    tagReader->adjustEndian((std::uint8_t*)&imageSizeY, 2, streamController::highByteEndian);
    tagReader->adjustEndian((std::uint8_t*)&imageSizeX, 2, streamController::highByteEndian);

    if(
            precisionBits < 8 ||
            precisionBits > 16 ||
            imageSizeX > codecFactory::getCodecFactory()->getMaximumImageWidth() ||
            imageSizeY > codecFactory::getCodecFactory()->getMaximumImageHeight())
    {
        PUNTOEXE_THROW(codecExceptionImageTooBig, "The factory settings prevented the loading of this image. Consider using codecFactory::setMaximumImageSize() to modify the settings");
    }

    pCodec->m_imageSizeX=(int)imageSizeX;
    pCodec->m_imageSizeY=(int)imageSizeY;

    // Read the components number
    ///////////////////////////////////////////////////////////
    pCodec->eraseChannels();
    std::uint8_t componentsNumber;
    tagReader->read(&componentsNumber, 1);

    // Get all the components specifications
    ///////////////////////////////////////////////////////////
    std::uint8_t componentId;
    std::uint8_t componentSamplingFactor;
    std::uint8_t componentQuantTable;
    for(std::uint8_t scanComponents(0); scanComponents<componentsNumber; ++scanComponents)
    {
        tagReader->read(&componentId, 1);
        tagReader->read(&componentSamplingFactor, 1);
        tagReader->read(&componentQuantTable, 1);

        ptrChannel pChannel(new jpeg::jpegChannel);
        pChannel->m_quantTable = (int)componentQuantTable;
        if(pChannel->m_quantTable >= 16)
        {
            PUNTOEXE_THROW(codecExceptionCorruptedFile, "Corrupted quantization table index in SOF tag");
        }
        pChannel->m_samplingFactorX = (int)(componentSamplingFactor>>4);
        pChannel->m_samplingFactorY = (int)(componentSamplingFactor & 0x0f);
        if(
                (pChannel->m_samplingFactorX != 1 &&
                 pChannel->m_samplingFactorX != 2 &&
                 pChannel->m_samplingFactorX != 4) ||
                (pChannel->m_samplingFactorY != 1 &&
                 pChannel->m_samplingFactorY != 2 &&
                 pChannel->m_samplingFactorY != 4)
                )
        {
            PUNTOEXE_THROW(codecExceptionCorruptedFile, "Wrong sampling factor in SOF tag");
        }
        pCodec->m_channelsMap[componentId] = pChannel;
    }

    // Recalculate the MCUs' attributes
    ///////////////////////////////////////////////////////////
    pCodec->allocChannels();

    PUNTOEXE_FUNCTION_END_MODIFY(streamExceptionEOF, codecExceptionCorruptedFile);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDHT
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write the DHT entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDHT::writeTag(streamWriter* pStream, jpegCodec* pCodec)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagDHT::writeTag");

    // Used to write bytes into the stream
    /////////////////////////////////////////////////////////////////
    std::uint8_t byte;

    // There are two phases:
    //  the first phase calculates the tag's length, the second one
    //  write the tables' definitions
    /////////////////////////////////////////////////////////////////
    std::uint16_t tagLength = 0;
    for(int phase = 0; phase < 2; ++phase)
    {
        // Write the tag's length
        /////////////////////////////////////////////////////////////////
        if(phase == 1)
            writeLength(pStream, tagLength);

        // Scan all the used tables
        /////////////////////////////////////////////////////////////////
        for(int tableNum = 0; tableNum < 16; ++tableNum)
        {
            // Scan for DC and AC tables
            /////////////////////////////////////////////////////////////////
            for(int DcAc = 0; DcAc < 2; ++DcAc)
            {
                // bAdd is true if the huffman table is used by a channel
                /////////////////////////////////////////////////////////////////
                bool bAdd=false;

                for(jpegCodec::tChannelsMap::iterator channelsIterator = pCodec->m_channelsMap.begin(); !bAdd && channelsIterator!=pCodec->m_channelsMap.end(); ++channelsIterator)
                {
                    ptrChannel pChannel=channelsIterator->second;
                    bAdd= DcAc==0 ? (tableNum == pChannel->m_huffmanTableDC) : (tableNum == pChannel->m_huffmanTableAC);
                }

                // If the table is used by at least one channel, then write
                //  its definition
                /////////////////////////////////////////////////////////////////
                if(!bAdd)
                {
                    continue;
                }
                ptr<huffmanTable> pHuffman;

                if(DcAc==0)
                {
                    pHuffman=pCodec->m_pHuffmanTableDC[tableNum];
                }
                else
                {
                    pHuffman=pCodec->m_pHuffmanTableAC[tableNum];
                }

                // Calculate the tag's length
                /////////////////////////////////////////////////////////////////
                if(phase == 0)
                {
                    pHuffman->incValueFreq(0x100);
                    pHuffman->calcHuffmanCodesLength(16);
                    // Remove the value 0x100 now
                    pHuffman->removeLastCode();

                    pHuffman->calcHuffmanTables();
                    tagLength+=17;
                    for(int scanLength = 0; scanLength < 16;)
                    {
                        tagLength += (std::uint16_t)(pHuffman->getValuesPerLength(++scanLength));
                    }
                    continue;
                }

                // Write the huffman table
                /////////////////////////////////////////////////////////////////

                // Write the table ID
                /////////////////////////////////////////////////////////////////
                std::uint8_t tableID=(std::uint8_t)((DcAc<<4) | tableNum);
                pStream->write(&tableID, 1);

                // Write the values per length.
                /////////////////////////////////////////////////////////////////
                int scanLength;
                for(scanLength=0; scanLength<16;)
                {
                    byte=(std::uint8_t)(pHuffman->getValuesPerLength(++scanLength));
                    pStream->write(&byte, 1);
                }

                // Write the table values
                /////////////////////////////////////////////////////////////////
                std::uint32_t valueIndex = 0;
                for(scanLength = 0; scanLength < 16; ++scanLength)
                {
                    for(std::uint32_t scanValues = 0; scanValues < pHuffman->getValuesPerLength(scanLength+1); ++scanValues)
                    {
                        byte=(std::uint8_t)(pHuffman->getOrderedValue(valueIndex++));
                        pStream->write(&byte, 1);
                    }
                }
            } // DcAc
        } // tableNum
    } // phase

    PUNTOEXE_FUNCTION_END();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DHT entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDHT::readTag(streamReader* pStream, jpegCodec* pCodec, std::uint8_t /* tagEntry */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagDHT::readTag");

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::int32_t tagLength = readLength(pStream);
    ptr<streamReader> tagReader(pStream->getReader(tagLength));

    // Used to read bytes from the stream
    /////////////////////////////////////////////////////////////////
    std::uint8_t byte;

    try
    {
        // Read all the defined tables
        /////////////////////////////////////////////////////////////////
        while(!tagReader->endReached())
        {
            // Read the table's ID
            /////////////////////////////////////////////////////////////////
            tagReader->read(&byte, 1);

            // Get a pointer to the right table
            /////////////////////////////////////////////////////////////////
            ptr<huffmanTable> pHuffman;
            if((byte & 0xf0) == 0)
                pHuffman=pCodec->m_pHuffmanTableDC[byte & 0xf];
            else
                pHuffman=pCodec->m_pHuffmanTableAC[byte & 0xf];

            // Reset the table
            /////////////////////////////////////////////////////////////////
            pHuffman->reset();

            // Used to scan all the codes lengths
            /////////////////////////////////////////////////////////////////
            int scanLength;

            // Read the number of codes per length
            /////////////////////////////////////////////////////////////////
            for(scanLength=0; scanLength<16L; )
            {
                tagReader->read(&byte, 1);
                pHuffman->setValuesPerLength(++scanLength, (std::uint32_t)byte);
            }

            // Used to store the values into the table
            /////////////////////////////////////////////////////////////////
            std::uint32_t valueIndex = 0;

            // Read all the values and store them into the huffman table
            /////////////////////////////////////////////////////////////////
            for(scanLength = 0; scanLength < 16; ++scanLength)
            {
                for(std::uint32_t scanValues = 0; scanValues < pHuffman->getValuesPerLength(scanLength+1); ++scanValues)
                {
                    tagReader->read(&byte, 1);
                    pHuffman->addOrderedValue(valueIndex++, (std::uint32_t)byte);
                }
            }

            // Calculate the huffman tables
            /////////////////////////////////////////////////////////////////
            pHuffman->calcHuffmanTables();
        }
    }
    catch(const huffmanExceptionCreateTable& e)
    {
        PUNTOEXE_THROW(codecs::codecExceptionCorruptedFile, e.what());
    }

    PUNTOEXE_FUNCTION_END_MODIFY(streamExceptionEOF, codecExceptionCorruptedFile);

}




/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagSOS
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the SOS entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOS::writeTag(streamWriter* pStream, jpegCodec* pCodec)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagSOS::writeTag");

    // Calculate the components number
    /////////////////////////////////////////////////////////////////
    std::uint8_t componentsNumber(0);
    while(pCodec->m_channelsList[componentsNumber] != 0)
    {
        ++componentsNumber;
    }

    // Write the tag's length
    /////////////////////////////////////////////////////////////////
    writeLength(pStream, 4+2*componentsNumber);

    // Write the component's number
    /////////////////////////////////////////////////////////////////
    pStream->write(&componentsNumber, 1);

    // Scan all the channels in the current scan
    /////////////////////////////////////////////////////////////////
    jpeg::jpegChannel* pChannel; // used in the loop
    for(jpeg::jpegChannel** listIterator = pCodec->m_channelsList; *listIterator != 0; ++listIterator)
    {
        pChannel = *listIterator;

        std::uint8_t channelId(0);

        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        // Find the channel's ID
        /////////////////////////////////////////////////////////////////
        for(jpegCodec::tChannelsMap::iterator mapIterator=pCodec->m_channelsMap.begin(); mapIterator != pCodec->m_channelsMap.end(); ++mapIterator)
        {
            if(mapIterator->second.get() == pChannel)
            {
                channelId=mapIterator->first;
                break;
            }
        }

        // Write the channel's ID
        /////////////////////////////////////////////////////////////////
        pStream->write(&channelId, 1);

        // Write the ac/dc tables ID
        /////////////////////////////////////////////////////////////////
        std::uint8_t acdc=(std::uint8_t)((pChannel->m_huffmanTableDC & 0xf)<<4);
        acdc |= (std::uint8_t)(pChannel->m_huffmanTableAC & 0xf);

        pStream->write(&acdc, 1);
    }

    std::uint8_t byte;

    // Write the spectral index start
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)pCodec->m_spectralIndexStart;
    pStream->write(&byte, 1);

    // Write the spectral index end
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)pCodec->m_spectralIndexEnd;
    pStream->write(&byte, 1);

    // Write the hi/lo bit
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)(pCodec->m_bitHigh & 0xf)<<4;
    byte|=(std::uint8_t)(pCodec->m_bitLow & 0xf);
    pStream->write(&byte, 1);

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the SOS entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOS::readTag(streamReader* pStream, jpegCodec* pCodec, std::uint8_t /* tagEntry */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagSOS::readTag");

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::int32_t tagLength = readLength(pStream);
    ptr<streamReader> tagReader(pStream->getReader(tagLength));

    pCodec->m_eobRun = 0;
    memset(pCodec->m_channelsList, 0, sizeof(pCodec->m_channelsList));

    std::uint8_t componentsNumber;
    tagReader->read(&componentsNumber, 1);

    std::uint8_t byte;
    for(std::uint8_t scanComponents = 0; scanComponents != componentsNumber; ++scanComponents)
    {
        tagReader->read(&byte, 1);

        jpegCodec::tChannelsMap::const_iterator findChannel = pCodec->m_channelsMap.find(byte);
        if(findChannel == pCodec->m_channelsMap.end())
        {
            PUNTOEXE_THROW(codecExceptionCorruptedFile, "Corrupted SOS tag found");
        }
        ptrChannel pChannel = findChannel->second;

        pChannel->processUnprocessedAmplitudes();

        tagReader->read(&byte, 1);

        pChannel->m_huffmanTableDC=byte>>4;
        pChannel->m_huffmanTableAC=byte & 0xf;
        pChannel->m_pActiveHuffmanTableDC = pCodec->m_pHuffmanTableDC[pChannel->m_huffmanTableDC].get();
        pChannel->m_pActiveHuffmanTableAC = pCodec->m_pHuffmanTableAC[pChannel->m_huffmanTableAC].get();

        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        pCodec->m_channelsList[scanComponents] = pChannel.get();

    }

    tagReader->read(&byte, 1);
    pCodec->m_spectralIndexStart=(int)byte;

    tagReader->read(&byte, 1);
    pCodec->m_spectralIndexEnd=(int)byte;

    tagReader->read(&byte, 1);
    pCodec->m_bitHigh=(int)(byte>>4);
    pCodec->m_bitLow=(int)(byte & 0xf);

    pCodec->findMcuSize();

    PUNTOEXE_FUNCTION_END_MODIFY(streamExceptionEOF, codecExceptionCorruptedFile);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDQT
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the DQT tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDQT::writeTag(streamWriter* pStream, jpegCodec* pCodec)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagDQT::writeTag");

    // Read the tag's length
    /////////////////////////////////////////////////////////////////
    std::int16_t tagLength = 0;

    std::uint8_t  tablePrecision;
    std::uint8_t  tableValue8;
    std::uint16_t tableValue16;

    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase != 0)
        {
            writeLength(pStream, tagLength);
        }
        for(std::uint8_t tableId = 0; tableId < 16; ++tableId)
        {
            // bAdd is true if the huffman table is used by a channel
            /////////////////////////////////////////////////////////////////
            bool bAdd=false;

            for(jpegCodec::tChannelsMap::iterator channelsIterator=pCodec->m_channelsMap.begin(); !bAdd && channelsIterator!=pCodec->m_channelsMap.end(); ++channelsIterator)
            {
                ptrChannel pChannel=channelsIterator->second;
                bAdd=pChannel->m_quantTable==tableId;
            }

            if(!bAdd)
            {
                continue;
            }
            // Calculate the table's precision
            bool b16Bits = pCodec->m_precision > 8;
            for(int tableIndex = 0; !b16Bits && (tableIndex < 64); ++tableIndex)
            {
                if(pCodec->m_quantizationTable[tableId][tableIndex] >= 256)
                {
                    b16Bits=true;
                }
            }

            if(phase == 0)
            {
                tagLength += 1+(b16Bits ? 128 : 64);
            }
            else
            {
                tablePrecision = tableId | (b16Bits ? 0x10 : 0);
                pStream->write(&tablePrecision, 1);
                if(b16Bits)
                {
                    for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
                    {
                        tableValue16 = (std::uint16_t)pCodec->m_quantizationTable[tableId][JpegDeZigZagOrder[tableIndex]];
                        pStream->adjustEndian((std::uint8_t*)&tableValue16, 2, streamController::highByteEndian);
                        pStream->write((std::uint8_t*)&tableValue16, 2);
                    }
                }
                else
                {
                    for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
                    {
                        tableValue8=(std::uint8_t)pCodec->m_quantizationTable[tableId][JpegDeZigZagOrder[tableIndex]];
                        pStream->write(&tableValue8, 1);
                    }
                }
            }

            pCodec->recalculateQuantizationTables(tableId);
        }
    }

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DQT tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDQT::readTag(streamReader* pStream, jpegCodec* pCodec, std::uint8_t /* tagEntry */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagDQT::readTag");

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::int32_t tagLength = readLength(pStream);
    ptr<streamReader> tagReader(pStream->getReader(tagLength));

    std::uint8_t  tablePrecision;
    std::uint8_t  tableValue8;
    std::uint16_t tableValue16;
    while(!tagReader->endReached())
    {
        tagReader->read(&tablePrecision, 1);

        // Read a DQT table
        /////////////////////////////////////////////////////////////////
        for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
        {
            // 16 bits precision
            /////////////////////////////////////////////////////////////////
            if((tablePrecision & 0xf0) != 0)
            {
                tagReader->read((std::uint8_t*)&tableValue16, 2);
                tagReader->adjustEndian((std::uint8_t*)&tableValue16, 2, streamController::highByteEndian);
                pCodec->m_quantizationTable[tablePrecision & 0x0f][JpegDeZigZagOrder[tableIndex]]=tableValue16;
            }

            // 8 bits precision
            /////////////////////////////////////////////////////////////////
            else
            {
                tagReader->read(&tableValue8, 1);
                pCodec->m_quantizationTable[tablePrecision & 0x0f][JpegDeZigZagOrder[tableIndex]]=tableValue8;
            }

        } // ----- End of table reading

        pCodec->recalculateQuantizationTables(tablePrecision & 0x0f);
    }

    PUNTOEXE_FUNCTION_END_MODIFY(streamExceptionEOF, codecExceptionCorruptedFile);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDRI
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the DRI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDRI::writeTag(streamWriter* pStream, jpegCodec* pCodec)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagDRI::writeTag");

    // Write the tag's length
    /////////////////////////////////////////////////////////////////
    writeLength(pStream, 2);

    // Write the MCU per restart interval
    /////////////////////////////////////////////////////////////////
    std::uint16_t unitsPerRestartInterval=pCodec->m_mcuPerRestartInterval;
    pStream->adjustEndian((std::uint8_t*)&unitsPerRestartInterval, 2, streamController::highByteEndian);
    pStream->write((std::uint8_t*)&unitsPerRestartInterval, 2);

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DRI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDRI::readTag(streamReader* pStream, jpegCodec* pCodec, std::uint8_t /* tagEntry */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagDRI::readTag");

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::int32_t tagLength = readLength(pStream);
    ptr<streamReader> tagReader(pStream->getReader(tagLength));

    std::uint16_t unitsPerRestartInterval;
    tagReader->read((std::uint8_t*)&unitsPerRestartInterval, 2);
    tagReader->adjustEndian((std::uint8_t*)&unitsPerRestartInterval, 2, streamController::highByteEndian);
    pCodec->m_mcuPerRestartInterval=unitsPerRestartInterval;

    PUNTOEXE_FUNCTION_END_MODIFY(streamExceptionEOF, codecExceptionCorruptedFile);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagRST
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the RST tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagRST::writeTag(streamWriter* /* pStream */, jpegCodec* /* pCodec */)
{
    return;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the RST tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagRST::readTag(streamReader* /* pStream */, jpegCodec* pCodec, std::uint8_t tagEntry)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagRST::readTag");

    // Reset the channels last dc value
    /////////////////////////////////////////////////////////////////
    for(jpeg::jpegChannel** channelsIterator = pCodec->m_channelsList; *channelsIterator != 0; ++channelsIterator)
    {
        (*channelsIterator)->processUnprocessedAmplitudes();
        (*channelsIterator)->m_lastDCValue = (*channelsIterator)->m_defaultDCValue;
    }

    // Calculate the mcu processed counter
    /////////////////////////////////////////////////////////////////
    if(pCodec->m_mcuPerRestartInterval > 0)
    {
        std::uint32_t doneRestartInterval=(pCodec->m_mcuProcessed+pCodec->m_mcuPerRestartInterval-1)/pCodec->m_mcuPerRestartInterval-1;
        std::uint8_t doneRestartIntervalID=(std::uint8_t)(doneRestartInterval & 0x7);
        std::uint8_t foundRestartIntervalID=tagEntry & 0x7;
        if(foundRestartIntervalID<doneRestartIntervalID)
            doneRestartInterval+=8L;
        doneRestartInterval-=doneRestartIntervalID;
        doneRestartInterval+=foundRestartIntervalID;
        pCodec->m_mcuProcessed=(doneRestartInterval+1)*pCodec->m_mcuPerRestartInterval;
        pCodec->m_mcuProcessedY = pCodec->m_mcuProcessed / pCodec->m_mcuNumberX;
        pCodec->m_mcuProcessedX = pCodec->m_mcuProcessed - (pCodec->m_mcuProcessedY * pCodec->m_mcuNumberX);
        pCodec->m_mcuLastRestart = pCodec->m_mcuProcessed;

        // Update the lossless pixel's counter in the channels
        /////////////////////////////////////////////////////////////////
        for(jpeg::jpegChannel** channelsIterator = pCodec->m_channelsList; *channelsIterator != 0; ++channelsIterator)
        {
            jpeg::jpegChannel* pChannel(*channelsIterator);
            pChannel->m_losslessPositionX = pCodec->m_mcuProcessedX / pChannel->m_blockMcuX;
            pChannel->m_losslessPositionY = pCodec->m_mcuProcessedY / pChannel->m_blockMcuY;
        }
    }

    pCodec->m_eobRun = 0;

    PUNTOEXE_FUNCTION_END();
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagEOI
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the EOI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagEOI::writeTag(streamWriter* pStream, jpegCodec* /* pCodec */)
{
    PUNTOEXE_FUNCTION_START(L"jpeg::tagEOI::writeTag");

    writeLength(pStream, 0);

    PUNTOEXE_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the EOI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagEOI::readTag(streamReader* /* pStream */, jpegCodec* pCodec, std::uint8_t /* tagEntry */)
{
    pCodec->m_bEndOfImage=true;
}

} // namespace jpeg

} // namespace codecs

} // namespace imebra

} // namespace puntoexe



