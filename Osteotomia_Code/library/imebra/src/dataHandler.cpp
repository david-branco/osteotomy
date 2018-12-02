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

/*! \file dataHandler.cpp
    \brief Implementation of the base class for the data handlers.

*/

#include "../../base/include/exception.h"
#include "../include/dataHandler.h"

namespace puntoexe
{

namespace imebra
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// imebraDataHandler
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
// Disconnect the handler
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool dataHandler::preDelete()
{
	PUNTOEXE_FUNCTION_START(L"dataHandler::preDelete");

	if(!m_bCommitted)
	{
		lockObject lockAccess(m_buffer.get());

		copyBack();
		commit();
	}
	return true;

	PUNTOEXE_FUNCTION_END();
}


void dataHandler::copyBack()
{
	PUNTOEXE_FUNCTION_START(L"dataHandler::copyBack");

	if(m_buffer == 0)
	{
		return;
	}
	m_buffer->copyBack(this);

	PUNTOEXE_FUNCTION_END();
}

void dataHandler::commit()
{
	PUNTOEXE_FUNCTION_START(L"dataHandler::copyBack");

	if(m_buffer == 0)
	{
		return;
	}

	m_buffer->commit();
	m_bCommitted = true;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Discard all the changes made on a writing handler
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandler::abort()
{
	m_buffer.release();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve an element's size
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataHandler::getUnitSize() const
{
	return 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the padding byte
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint8_t dataHandler::getPaddingByte() const
{
	return 0;
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the data 's type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataHandler::getDataType() const
{
	return m_bufferType;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse the buffer's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandler::parseBuffer(const std::uint8_t* pBuffer, const std::uint32_t bufferLength)
{
	PUNTOEXE_FUNCTION_START(L"dataHandler::parseBuffer");

	ptr<memory> tempMemory(memoryPool::getMemoryPool()->getMemory(bufferLength));
	if(pBuffer && bufferLength)
	{
		tempMemory->assign(pBuffer, bufferLength);
	}
	parseBuffer(tempMemory);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandler::getDate(const std::uint32_t /* index */,
		std::int32_t* pYear, 
		std::int32_t* pMonth, 
		std::int32_t* pDay, 
		std::int32_t* pHour, 
		std::int32_t* pMinutes,
		std::int32_t* pSeconds,
		std::int32_t* pNanoseconds,
		std::int32_t* pOffsetHours,
		std::int32_t* pOffsetMinutes) const
{
	*pYear = 0;
	*pMonth = 0;
	*pDay = 0;
	*pHour = 0;
	*pMinutes = 0;
	*pSeconds = 0;
	*pNanoseconds = 0;
	*pOffsetHours = 0;
	*pOffsetMinutes = 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandler::setDate(const std::uint32_t /* index */,
		std::int32_t /* year */, 
		std::int32_t /* month */, 
		std::int32_t /* day */, 
		std::int32_t /* hour */, 
		std::int32_t /*minutes */,
		std::int32_t /*seconds */,
		std::int32_t /*nanoseconds */,
		std::int32_t /*offsetHours */,
		std::int32_t /*offsetMinutes */)
{
	return;
}

void dataHandler::setCharsetsList(charsetsList::tCharsetsList* /* pCharsetsList */)
{
	// Intentionally empty
}

void dataHandler::getCharsetsList(charsetsList::tCharsetsList* /* pCharsetsList */) const
{
	// Intentionally empty
}


} // namespace handlers

} // namespace imebra

} // namespace puntoexe
