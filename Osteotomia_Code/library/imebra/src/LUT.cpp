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

/*! \file LUT.cpp
    \brief Implementation of the class lut.

*/

#include "../../base/include/exception.h"
#include "../include/LUT.h"
#include "../include/dataHandlerNumeric.h"
#include <string.h>

namespace puntoexe
{

namespace imebra
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lut::~lut()
{
	if(m_pMappedValues)
	{
        delete[] m_pMappedValues;
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Create a LUT from a data handler
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lut::setLut(ptr<handlers::dataHandler> pDescriptor, ptr<handlers::dataHandler> pData, std::wstring description)
{
	PUNTOEXE_FUNCTION_START(L"lut::setLut");

	if(pDescriptor->getSize() < 3)
	{
		PUNTOEXE_THROW(lutExceptionCorrupted, "The LUT is corrupted");
	}
	std::int32_t lutSize=pDescriptor->getSignedLong(0);
	if(lutSize == 0)
		lutSize=0x00010000;
	if(lutSize < 0)
		lutSize&=0x0000FFFF;

	std::int32_t lutFirstMapped=pDescriptor->getSignedLong(1);
	std::uint32_t lutBits=pDescriptor->getUnsignedLong(2);

	if(pData == 0 || (std::uint32_t)lutSize != pData->getSize())
	{
		PUNTOEXE_THROW(lutExceptionCorrupted, "The LUT is corrupted");
	}

	create(lutSize, lutFirstMapped, (std::uint8_t)lutBits, description);

    dynamic_cast<handlers::dataHandlerNumericBase*>(pData.get())->copyTo(m_pMappedValues, lutSize);

    m_bChecked = false; // LUT has to be checked again

	PUNTOEXE_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Create a LUT
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lut::create(std::uint32_t size, std::int32_t firstMapped, std::uint8_t bits, std::wstring description)
{
	PUNTOEXE_FUNCTION_START(L"lut::create");

	// If some values were previously allocated, then remove
	//  them
	///////////////////////////////////////////////////////////
	if(m_pMappedValues)
	{
		delete m_pMappedValues;
		m_pMappedValues= 0;
	}
	m_mappedValuesRev.clear();

	m_bChecked = false;

	m_size = 0;

	m_description = description;

    if(size != 0)
	{
        m_size = size;
        m_firstMapped = firstMapped;
        m_bits = bits;
        m_pMappedValues = new std::int32_t[m_size];
	}

	PUNTOEXE_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Fill the data handlers in a dataset with the lut
//  information
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lut::fillHandlers(ptr<handlers::dataHandler> pDescriptor, ptr<handlers::dataHandler> pData)
{
	PUNTOEXE_FUNCTION_START(L"lut::fillHandlers");

	pDescriptor->setSize(3);
	std::uint32_t lutSize = getSize();
	if(lutSize == 0x00010000)
	{
		pDescriptor->setSignedLong(0, 0);
	}
	else
	{
		pDescriptor->setUnsignedLong(0, lutSize);
	}

	std::int32_t lutFirstMapped = getFirstMapped();
	pDescriptor->setSignedLong(1, lutFirstMapped);

	std::uint8_t bits = getBits();
	pDescriptor->setUnsignedLong(2, bits);

	pData->setSize(lutSize);
	dynamic_cast<handlers::dataHandlerNumericBase*>(pData.get())->copyFrom(m_pMappedValues, lutSize);

	PUNTOEXE_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the number of values stored into the LUT
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t lut::getSize()
{
	return m_size;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Check the validity of the data in the LUT
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool lut::checkValidDataRange()
{
    if(m_bChecked)
    {
        return m_bValid;
    }
    std::int32_t maxValue(65535);
    std::int32_t minValue(-32768);
    if(m_bits == 8)
    {
        maxValue = 255;
        minValue = -128;
    }
    std::int32_t* pScanValues(m_pMappedValues);
    for(std::uint32_t checkData(0); checkData != m_size; ++checkData)
    {
        if(*pScanValues < minValue || *pScanValues > maxValue)
        {
            m_bValid = false;
            m_bChecked = true;
            return m_bValid;
        }
        ++pScanValues;
    }
    m_bValid = true;
    m_bChecked = true;
    return m_bValid;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the first mapped index
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t lut::getFirstMapped()
{
	return m_firstMapped;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert a new value into the LUT
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lut::setLutValue(std::int32_t startValue, std::int32_t lutValue)
{
	PUNTOEXE_FUNCTION_START(L"lut::setLutValue");

	if(startValue<m_firstMapped)
	{
		PUNTOEXE_THROW(lutExceptionWrongIndex, "The start index is below the first mapped index");
	}
	startValue-=m_firstMapped;
	if(startValue<(std::int32_t)m_size)
	{
		m_pMappedValues[startValue]=lutValue;
		m_mappedValuesRev.clear();
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the lut's description.
// The returned value must be removed by the client
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring lut::getDescription()
{
	return m_description;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the lut's bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint8_t lut::getBits()
{
	return m_bits;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Lookup the requested value
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t lut::mappedValue(std::int32_t id)
{
	// The LUT's size is zero, return
	///////////////////////////////////////////////////////////
	if(m_size == 0)
	{
		return 0;
	}

	// Subtract the first mapped value
	///////////////////////////////////////////////////////////
	id -= m_firstMapped;

	if(id <= 0)
	{
		return m_pMappedValues[0];
	}
	if(id < (std::int32_t)m_size)
	{
		return m_pMappedValues[id];
	}
	return m_pMappedValues[m_size-1];
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Copy the palette's data to an array of std::int32_t
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lut::copyToInt32(std::int32_t* pDestination, std::uint32_t destSize, std::int32_t* pFirstMapped)
{
	if(destSize > m_size)
	{
		destSize = m_size;
	}
	::memcpy(pDestination, m_pMappedValues, destSize*sizeof(std::int32_t));
	*pFirstMapped = m_firstMapped;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Inverse Lookup table
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t lut::mappedValueRev(std::int32_t lutValue)
{
	if(m_size == 0)
	{
		return 0;
	}

	if(m_mappedValuesRev.size() == 0)
	{
		for(std::uint32_t reverseLUT = 0; reverseLUT < m_size; ++reverseLUT)
		{
			m_mappedValuesRev[m_pMappedValues[reverseLUT]]=reverseLUT;
		}
	}

	if(lutValue<=m_mappedValuesRev.begin()->first)
		return m_mappedValuesRev.begin()->second;

	std::map<std::int32_t, std::int32_t>::iterator lutIterator;
	lutIterator=m_mappedValuesRev.end();
	--lutIterator;
	if(lutValue>=lutIterator->first)
		return lutIterator->second;

	lutIterator=m_mappedValuesRev.lower_bound(lutValue);

	if(lutIterator!=m_mappedValuesRev.end())
		return lutIterator->second;

	return 0;
}


palette::palette(ptr<lut> red, ptr<lut> green, ptr<lut> blue):
m_redLut(red), m_greenLut(green), m_blueLut(blue)
{}

void palette::setLuts(ptr<lut> red, ptr<lut> green, ptr<lut> blue)
{
	m_redLut = red;
	m_greenLut = green;
	m_blueLut = blue;
}

ptr<lut> palette::getRed()
{
	return m_redLut;
}

ptr<lut> palette::getGreen()
{
	return m_greenLut;
}

ptr<lut> palette::getBlue()
{
	return m_blueLut;
}

} // namespace imebra

} // namespace puntoexe
