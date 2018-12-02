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

/*! \file streamReader.cpp
    \brief Implementation of the streamReader class.

*/

#include "../include/streamReader.h"
#include <string.h>

namespace puntoexe
{

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
streamReader::streamReader(ptr<baseStream> pControlledStream):
    streamController(pControlledStream, 0, 0),
    m_inBitsBuffer(0),
    m_inBitsNum(0)
{
}

streamReader::streamReader(ptr<baseStream> pControlledStream, std::uint32_t virtualStart, std::uint32_t virtualLength):
	streamController(pControlledStream, virtualStart, virtualLength),
	m_inBitsBuffer(0),
	m_inBitsNum(0)
{
    if(virtualLength == 0)
    {
        throw(streamExceptionEOF("Virtual stream with zero length"));
    }
}


ptr<streamReader> streamReader::getReader(std::uint32_t virtualLength)
{
    if(virtualLength == 0)
    {
        throw(streamExceptionEOF("Virtual stream with zero length"));
    }
    std::uint32_t currentPosition = position();
    if(currentPosition + virtualLength > m_virtualLength && m_virtualLength != 0)
    {
        virtualLength = m_virtualLength - currentPosition;
    }
    seek(virtualLength, true);
    return new streamReader(m_pControlledStream, currentPosition + m_virtualStart, virtualLength);
}

///////////////////////////////////////////////////////////
//
// Returns true if the last byte has been read
//
///////////////////////////////////////////////////////////
bool streamReader::endReached()
{
    return (m_pDataBufferCurrent == m_pDataBufferEnd && fillDataBuffer() == 0);
}


///////////////////////////////////////////////////////////
//
// Refill the data buffer
//
///////////////////////////////////////////////////////////
std::uint32_t streamReader::fillDataBuffer()
{
	PUNTOEXE_FUNCTION_START(L"streamReader::fillDataBuffer");

	std::uint32_t readLength = (std::uint32_t)(m_pDataBufferMaxEnd - m_pDataBufferStart);
	std::uint32_t readBytes = fillDataBuffer(m_pDataBufferStart, readLength);
	if(readBytes == 0)
	{
		m_pDataBufferCurrent = m_pDataBufferEnd = m_pDataBufferStart;
		return 0;
	}
	m_pDataBufferEnd = m_pDataBufferStart + readBytes;
	m_pDataBufferCurrent = m_pDataBufferStart;
	return readBytes;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Read data from the stream into the specified buffer
//
///////////////////////////////////////////////////////////
std::uint32_t streamReader::fillDataBuffer(std::uint8_t* pDestinationBuffer, std::uint32_t readLength)
{
    m_dataBufferStreamPosition = position();
	if(m_virtualLength != 0)
	{
        if(m_dataBufferStreamPosition >= m_virtualLength)
		{
			m_dataBufferStreamPosition = m_virtualLength;
			return 0;
		}
        if(m_dataBufferStreamPosition + readLength > m_virtualLength)
		{
            readLength = m_virtualLength - m_dataBufferStreamPosition;
		}
	}
    return m_pControlledStream->read(m_dataBufferStreamPosition + m_virtualStart, pDestinationBuffer, readLength);
}



///////////////////////////////////////////////////////////
//
// Return the specified number of bytes from the stream
//
///////////////////////////////////////////////////////////
void streamReader::read(std::uint8_t* pBuffer, std::uint32_t bufferLength)
{
	while(bufferLength != 0)
	{
		// Update the data buffer if it is empty
		///////////////////////////////////////////////////////////
		if(m_pDataBufferCurrent == m_pDataBufferEnd)
		{
			if(bufferLength >= m_pDataBufferMaxEnd - m_pDataBufferStart)
			{
				// read the data directly into the destination buffer
				///////////////////////////////////////////////////////////
				std::uint32_t readBytes(fillDataBuffer(pBuffer, bufferLength));

				m_pDataBufferCurrent = m_pDataBufferEnd = m_pDataBufferStart;
				m_dataBufferStreamPosition += readBytes;
				pBuffer += readBytes;
				bufferLength -= readBytes;
				if(readBytes == 0)
				{
					throw(streamExceptionEOF("Attempt to read past the end of the file"));
				}
				continue;
			}

			if(fillDataBuffer() == 0)
			{
				throw(streamExceptionEOF("Attempt to read past the end of the file"));
			}
		}

		// Copy the available data into the return buffer
		///////////////////////////////////////////////////////////
		std::uint32_t copySize = bufferLength;
		std::uint32_t maxSize = (std::uint32_t)(m_pDataBufferEnd - m_pDataBufferCurrent);
		if(copySize > maxSize)
		{
			copySize = maxSize;
		}
		::memcpy(pBuffer, m_pDataBufferCurrent, (size_t)copySize);
		bufferLength -= copySize;
		pBuffer += copySize;
		m_pDataBufferCurrent += copySize;
	}
}


///////////////////////////////////////////////////////////
//
// Seek the read position
//
///////////////////////////////////////////////////////////
void streamReader::seek(std::int32_t newPosition, bool bCurrent /* =false */)
{
	// Calculate the absolute position
	///////////////////////////////////////////////////////////
	std::uint32_t finalPosition = bCurrent ? (position() + newPosition) : newPosition;

	// The requested position is already in the data buffer?
	///////////////////////////////////////////////////////////
	std::uint32_t bufferEndPosition = m_dataBufferStreamPosition + (std::uint32_t)(m_pDataBufferEnd - m_pDataBufferStart);
	if(finalPosition >= m_dataBufferStreamPosition && finalPosition < bufferEndPosition)
	{
		m_pDataBufferCurrent = m_pDataBufferStart + finalPosition - m_dataBufferStreamPosition;
		return;
	}

	// The requested position is not in the data buffer
	///////////////////////////////////////////////////////////
	m_pDataBufferCurrent = m_pDataBufferEnd = m_pDataBufferStart;
	m_dataBufferStreamPosition = finalPosition;

}



} // namespace puntoexe
