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

/*! \file dataHandlerDateTimeBase.cpp
    \brief Implementation of the base class for the date/time handlers.

*/

#include "../../base/include/exception.h"
#include "../include/dataHandlerDateTimeBase.h"
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

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
// dataHandlerDateTimeBase
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
// Returns a long integer representing the date/time (UTC)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t dataHandlerDateTimeBase::getSignedLong(const std::uint32_t index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::getSignedLong");

	std::int32_t year, month, day, hour, minutes, seconds, nanoseconds, offsetHours, offsetMinutes;
	getDate(index, &year, &month, &day, &hour, &minutes, &seconds, &nanoseconds, &offsetHours, &offsetMinutes);

	tm timeStructure;
	timeStructure.tm_isdst= -1;
	timeStructure.tm_wday= 0;
	timeStructure.tm_yday= 0;
	timeStructure.tm_year = year;
	timeStructure.tm_mon = month-1;
	timeStructure.tm_mday = day;
	timeStructure.tm_hour = hour;
	timeStructure.tm_min = minutes;
	timeStructure.tm_sec = seconds;
	
	return (std::int32_t)mktime(&timeStructure);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Cast getSignedLong to unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataHandlerDateTimeBase::getUnsignedLong(const std::uint32_t index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::getUnsignedLong");

	return (std::uint32_t)getSignedLong(index);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Cast getSignedLong to double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double dataHandlerDateTimeBase::getDouble(const std::uint32_t index) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::getDouble");

	return (double)getSignedLong(index);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date as a signed long (from time_t)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerDateTimeBase::setSignedLong(const std::uint32_t index, const std::int32_t value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::setSignedLong");

    tm timeStructure;
#if defined(PUNTOEXE_WINDOWS) && !defined(__MINGW32__)
    localtime_s(&timeStructure, ((time_t*)&value));
#else
    localtime_r((time_t*)&value, &timeStructure);
#endif
    std::int32_t year = timeStructure.tm_year;
    std::int32_t month = timeStructure.tm_mon + 1;
    std::int32_t day = timeStructure.tm_mday;
    std::int32_t hour = timeStructure.tm_hour;
    std::int32_t minutes = timeStructure.tm_min;
    std::int32_t seconds = timeStructure.tm_sec;
	setDate(index, year, month, day, hour, minutes, seconds, 0, 0, 0);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date as a long (from time_t)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerDateTimeBase::setUnsignedLong(const std::uint32_t index, const std::uint32_t value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::setUnsignedLong");

	setSignedLong(index, (std::int32_t)value);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date as a double (from time_t)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerDateTimeBase::setDouble(const std::uint32_t index, const double value)
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::setDouble");

	setSignedLong(index, (std::int32_t)value);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the separator
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
wchar_t dataHandlerDateTimeBase::getSeparator() const
{
	return 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse a date string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerDateTimeBase::parseDate(
		std::wstring dateString,
		std::int32_t* pYear, 
		std::int32_t* pMonth, 
		std::int32_t* pDay) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::parseDate");

	if(dateString.size()<8)
		dateString.resize(8, L'0');

	std::wstring dateYear=dateString.substr(0, 4);
	std::wstring dateMonth=dateString.substr(4, 2);
	std::wstring dateDay=dateString.substr(6, 2);

	std::wistringstream yearStream(dateYear);
	yearStream >> (*pYear);

	std::wistringstream monthStream(dateMonth);
	monthStream >> (*pMonth);

	std::wistringstream dayStream(dateDay);
	dayStream >> (*pDay);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a date string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataHandlerDateTimeBase::buildDate(
		std::uint32_t year,
		std::uint32_t month,
		std::uint32_t day) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::buildDate");

        if((year > 9999) || (month < 1) || (month>12) || (day<1) || (day>31))
	{
		year = month = day = 0;
	}

	std::wostringstream dateStream;
	dateStream << std::setfill(L'0');
	dateStream << std::setw(4) << year;
	dateStream << std::setw(2) << month;
	dateStream << std::setw(2) << day;

	return dateStream.str();

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse a time string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerDateTimeBase::parseTime(
		std::wstring timeString,
		std::int32_t* pHour, 
		std::int32_t* pMinutes,
		std::int32_t* pSeconds,
		std::int32_t* pNanoseconds,
		std::int32_t* pOffsetHours,
		std::int32_t* pOffsetMinutes) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::parseTime");

	if(timeString.size() < 6)
	{
		timeString.resize(6, L'0');
	}
	if(timeString.size() < 7)
	{
		timeString += L'.';
	}
	if(timeString.size() < 13)
	{
		timeString.resize(13, L'0');
	}
	if(timeString.size() < 14)
	{
		timeString += L'+';
	}
	if(timeString.size() < 18)
	{
		timeString.resize(18, L'0');
	}
	
	std::wstring timeHour = timeString.substr(0, 2);
	std::wstring timeMinutes = timeString.substr(2, 2);
	std::wstring timeSeconds = timeString.substr(4, 2);
	std::wstring timeNanoseconds = timeString.substr(7, 6);
	std::wstring timeOffsetHours = timeString.substr(13, 3);
	std::wstring timeOffsetMinutes = timeString.substr(16, 2);

	std::wistringstream hourStream(timeHour);
	hourStream >> (*pHour);

	std::wistringstream minutesStream(timeMinutes);
	minutesStream >> (*pMinutes);

	std::wistringstream secondsStream(timeSeconds);
	secondsStream >> (*pSeconds);

	std::wistringstream nanosecondsStream(timeNanoseconds);
	nanosecondsStream >> (*pNanoseconds);

	std::wistringstream offsetHoursStream(timeOffsetHours);
	offsetHoursStream >> (*pOffsetHours);

	std::wistringstream offsetMinutesStream(timeOffsetMinutes);
	offsetMinutesStream >> (*pOffsetMinutes);

	if(*pOffsetHours < 0)
	{
		*pOffsetMinutes= - *pOffsetMinutes;
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build the time string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataHandlerDateTimeBase::buildTime(
		std::int32_t hour,
		std::int32_t minutes,
		std::int32_t seconds,
		std::int32_t nanoseconds,
		std::int32_t offsetHours,
		std::int32_t offsetMinutes
		) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::buildTime");

	if(
		   (hour < 0)
		|| (hour >= 24)
		|| (minutes < 0)
		|| (minutes >= 60)
		|| (seconds <0)
		|| (seconds >= 60)
		|| (nanoseconds < 0)
		|| (nanoseconds > 999999)
		|| (offsetHours < -12)
		|| (offsetHours > 12)
		|| (offsetMinutes < -59)
		|| (offsetMinutes > 59))
	{
		hour = minutes = seconds = nanoseconds = offsetHours = offsetMinutes = 0;
	}

	bool bMinus=offsetHours < 0;

	std::wostringstream timeStream;
	timeStream << std::setfill(L'0');
	timeStream << std::setw(2) << hour;
	timeStream << std::setw(2) << minutes;
	timeStream << std::setw(2) << seconds;
	timeStream << std::setw(1) << L".";
	timeStream << std::setw(6) << nanoseconds;
	timeStream << std::setw(1) << (bMinus ? L"-" : L"+");
	timeStream << std::setw(2) << labs(offsetHours);
	timeStream << std::setw(2) << labs(offsetMinutes);
	
	return timeStream.str();

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Split several parts of a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataHandlerDateTimeBase::split(const std::wstring& timeString, const std::wstring& separators, std::vector<std::wstring> *pComponents) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::split");

        if(timeString.empty())
        {
            return;
        }

	for(size_t startPos(0), sepPos(timeString.find_first_of(separators)); /* empty */; sepPos = timeString.find_first_of(separators, startPos))
	{
                if(sepPos == timeString.npos)
                {
                    pComponents->push_back(timeString.substr(startPos));
                    break;
                }
		pComponents->push_back(timeString.substr(startPos, sepPos - startPos));
		startPos = ++sepPos;
                if(startPos == timeString.size())
                {
                    pComponents->push_back(L"");
                    break;
                }
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Add the specified char to the left of a string until
//  its length reaches the desidered value
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataHandlerDateTimeBase::padLeft(const std::wstring& source, const wchar_t fillChar, const size_t length) const
{
	PUNTOEXE_FUNCTION_START(L"dataHandlerDateTimeBase::padLeft");
        
        if(source.size() >= length)
        {
            return source;
        }

        std::wstring paddedString(length - source.size(), fillChar);
        paddedString += source;

	return paddedString;

	PUNTOEXE_FUNCTION_END();
}

} // namespace handlers

} // namespace imebra

} // namespace puntoexe



