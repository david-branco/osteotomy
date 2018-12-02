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

/*! \file streamController.cpp
    \brief Implementation of the streamController class.

*/

#include "../include/streamController.h"

namespace puntoexe
{

// Used for the endian check
///////////////////////////////////////////////////////////
static const std::uint16_t m_endianCheck(0x00ff);
static std::uint8_t const * const pBytePointer((std::uint8_t*)&m_endianCheck);
static const streamController::tByteOrdering m_platformByteOrder((*pBytePointer)==0xff ? streamController::lowByteEndian : streamController::highByteEndian);

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
streamController::streamController(ptr<baseStream> pControlledStream, std::uint32_t virtualStart /* =0 */, std::uint32_t virtualLength /* =0 */):
	m_bJpegTags(false),
        m_pControlledStream(pControlledStream),
		m_dataBuffer(new std::uint8_t[IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE]),
		m_virtualStart(virtualStart),
		m_virtualLength(virtualLength),
		m_dataBufferStreamPosition(0)
{
    m_pDataBufferStart = m_pDataBufferEnd = m_pDataBufferCurrent = m_dataBuffer;
	m_pDataBufferMaxEnd = m_pDataBufferStart + IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE;
}


///////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
streamController::~streamController()
{
    delete[] m_dataBuffer;
}


///////////////////////////////////////////////////////////
//
// Retrieve the current position
//
///////////////////////////////////////////////////////////
std::uint32_t streamController::position()
{
	return m_dataBufferStreamPosition + (std::uint32_t)(m_pDataBufferCurrent - m_pDataBufferStart);
}


///////////////////////////////////////////////////////////
//
// Retrieve a pointer to the controlled stream
//
///////////////////////////////////////////////////////////
ptr<baseStream> streamController::getControlledStream()
{
	return m_pControlledStream;
}


///////////////////////////////////////////////////////////
//
// Retrieve the position without considering the virtual
//  start's position
//
///////////////////////////////////////////////////////////
std::uint32_t streamController::getControlledStreamPosition()
{
	return m_dataBufferStreamPosition + (std::uint32_t)(m_pDataBufferCurrent - m_pDataBufferStart) + m_virtualStart;
}


///////////////////////////////////////////////////////////
//
// Adjust the byte ordering of pBuffer
//
///////////////////////////////////////////////////////////
void streamController::adjustEndian(std::uint8_t* pBuffer, const std::uint32_t wordLength, const tByteOrdering endianType, const std::uint32_t words /* =1 */)
{
	if(endianType == m_platformByteOrder || wordLength<2L)
	{
		return;
	}

	switch(wordLength)
	{
	case 2:
        {
            std::uint16_t* pWord((std::uint16_t*)pBuffer);
			for(std::uint32_t scanWords = words; scanWords != 0; --scanWords)
            {
                *pWord = ((*pWord & 0x00ff) << 8) | ((*pWord & 0xff00) >> 8);
                ++pWord;
			}
		}
		return;
	case 4:
        {
            std::uint32_t* pDWord((std::uint32_t*)pBuffer);
			for(std::uint32_t scanWords = words; scanWords != 0; --scanWords)
            {
                *pDWord = ((*pDWord & 0xff000000) >> 24) | ((*pDWord & 0x00ff0000) >> 8) | ((*pDWord & 0x0000ff00) << 8) | ((*pDWord & 0x000000ff) << 24);
                ++pDWord;
			}
		}
		return;
	}
}

std::uint16_t streamController::adjustEndian(std::uint16_t buffer, const tByteOrdering endianType)
{
    if(endianType == m_platformByteOrder)
    {
        return buffer;
    }
    return ((buffer & 0xff00) >> 8) | ((buffer & 0xff) << 8);
}

std::uint32_t streamController::adjustEndian(std::uint32_t buffer, const tByteOrdering endianType)
{
    if(endianType == m_platformByteOrder)
    {
        return buffer;
    }
    return ((buffer & 0xff000000) >> 24) | ((buffer & 0x00ff0000) >> 8) | ((buffer & 0x0000ff00) << 8) | ((buffer & 0x000000ff) << 24);
}




} // namespace puntoexe
