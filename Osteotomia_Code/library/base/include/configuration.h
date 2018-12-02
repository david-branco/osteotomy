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

/*! \file configuration.h
    \brief Declaration of the basic data types and of the platform flags
	        (Posix or Windows)

*/

#include <cstdint>

/// \addtogroup group_baseclasses
///
/// @{

/// @deprecated Use std::uint8_t instead
///
/// This type represents a 1 byte unsigned integer.
///
typedef std::uint8_t imbxUint8;

/// @deprecated Use std::uint16_t instead
///
/// This type represents a 2 bytes unsigned integer.
///
typedef std::uint16_t imbxUint16;

/// @deprecated Use std::uint32_t instead
///
/// This type represents a 4 bytes unsigned integer.
///
typedef std::uint32_t  imbxUint32;

/// @deprecated Use std::int8_t instead
///
/// This type represents an 1 byte signed integer.
///
typedef std::int8_t    imbxInt8;

/// @deprecated Use std::int16_t instead
///
/// This type represents a 2 bytes signed integer.
///
typedef std::int16_t   imbxInt16;

/// @deprecated Use std::int32_t instead
///
/// This type represents a 8 bytes signed integer.
///
typedef std::int32_t    imbxInt32;

typedef std::int64_t imbxInt64;

typedef std::uint64_t imbxUint64;


#if !defined(PUNTOEXE_WINDOWS) && !defined(PUNTOEXE_POSIX)

#if defined(WIN32) || defined(WIN64)
#define PUNTOEXE_WINDOWS 1
#endif

#ifndef PUNTOEXE_WINDOWS
#define PUNTOEXE_POSIX 1
#endif

#if !defined(PUNTOEXE_USE_ICONV) && !defined(PUNTOEXE_USE_ICU) && !defined(PUNTOEXE_USE_WINDOWS_CHARSET) && !defined(PUNTOEXE_USE_JAVA)

#if defined(PUNTOEXE_WINDOWS)
    #define PUNTOEXE_USE_WINDOWS_CHARSET
#else
    #define PUNTOEXE_USE_ICONV
#endif
#endif

#ifndef MAXIMUM_IMAGE_WIDTH
    #define MAXIMUM_IMAGE_WIDTH 4096
#endif
#ifndef MAXIMUM_IMAGE_HEIGHT
    #define MAXIMUM_IMAGE_HEIGHT 4096
#endif

///@}

#endif



