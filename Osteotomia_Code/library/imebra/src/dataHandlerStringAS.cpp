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

/*! \file dataHandlerStringAS.cpp
    \brief Implementation of the class dataHandlerStringAS.

*/

#include <sstream>
#include <iomanip>

#include "../../base/include/exception.h"
#include "../include/dataHandlerStringAS.h"

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
// dataHandlerStringAS
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
// Set the age
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerStringAS::setAge(const std::uint32_t index, const std::uint32_t age, const tAgeUnit unit)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::setAge");

	std::wostringstream ageStream;
	ageStream << std::setfill(L'0');
	ageStream << std::setw(3) << age;
	ageStream << std::setw(1) << (wchar_t)unit;

	setUnicodeString(index, ageStream.str());

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataHandlerStringAS::getAge(const std::uint32_t index, tAgeUnit* pUnit) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::getAge");

	std::wstring ageString = getUnicodeString(index);
	std::wistringstream ageStream(ageString);
	std::uint32_t age;
	ageStream >> age;
    char unit = ageString[ageString.size() - 1];
    if(unit == days || unit == weeks || unit == months || unit == years)
    {
        *pUnit = (tAgeUnit)unit;
    }
    else
    {
        *pUnit = years;
    }

	return age;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age in years as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t dataHandlerStringAS::getSignedLong(const std::uint32_t index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::getSignedLong");

	return (std::int32_t)getDouble(index);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age in years as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataHandlerStringAS::getUnsignedLong(const std::uint32_t index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::getUnsignedLong");

	return (std::int32_t)getDouble(index);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age in years as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double dataHandlerStringAS::getDouble(const std::uint32_t index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::getDouble");

	tAgeUnit ageUnit;
	double age = (double)getAge(index, &ageUnit);

	if(ageUnit == days)
	{
		return age / (double)365;
	}
	if(ageUnit == weeks)
	{
		return age / 52.14;
	}
	if(ageUnit == months)
	{
		return age / (double)12;
	}
	return age;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age in years as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerStringAS::setSignedLong(const std::uint32_t index, const std::int32_t value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::setSignedLong");

	setDouble(index, (double)value);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age in years as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerStringAS::setUnsignedLong(const std::uint32_t index, const std::uint32_t value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::setUnsignedLong");

	setDouble(index, (double)value);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age in years as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerStringAS::setDouble(const std::uint32_t index, const double value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerStringAS::setDouble");

	if(value < 0)
	{
		setAge(index, 0, days);
	}
	if(value < 0.08)
	{
		setAge(index, (std::uint32_t)(value * 365), days);
		return;
	}
	if(value < 0.5)
	{
		setAge(index, (std::uint32_t)(value * 52.14), weeks);
		return;
	}
	if(value < 2)
	{
		setAge(index, (std::uint32_t)(value * 12), months);
		return;
	}
	setAge(index, (std::uint32_t)value, years);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the padding byte
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint8_t dataHandlerStringAS::getPaddingByte() const
{
	return 0x20;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the element's size
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataHandlerStringAS::getUnitSize() const
{
	return 4L;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the maximum size
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataHandlerStringAS::maxSize() const
{
	return 4L;
}

} // namespace handlers

} // namespace imebra

} // namespace puntoexe
