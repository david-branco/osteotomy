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

/*! \file memoryStream.h
    \brief Declaration of the memoryStream class.

*/

#if !defined(imebraMemoryStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraMemoryStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "baseStream.h"
#include "memory.h"

///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

/// \addtogroup group_baseclasses
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class derives from the baseStream 
///         class and implements a memory stream.
///
/// This class can be used to read/write on the allocated
///  memory.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class memoryStream : public baseStream
{

public:
	/// \brief Construct a memoryStream object and attach a
	///         memory object to it.
	///
	/// The attached memory object will be resized if new data
	///  is written and its size is too small.
	///
	/// @param memoryStream the memory object to be used by
	///                      the memoryStream object.
	///
	///////////////////////////////////////////////////////////
	memoryStream(ptr<memory> memoryStream);

	///////////////////////////////////////////////////////////
	//
	// Virtual stream's functions
	//
	///////////////////////////////////////////////////////////
	virtual void write(std::uint32_t startPosition, const std::uint8_t* pBuffer, std::uint32_t bufferLength);
	virtual std::uint32_t read(std::uint32_t startPosition, std::uint8_t* pBuffer, std::uint32_t bufferLength);

protected:
	ptr<memory> m_memory;
};

///@}

} // namespace puntoexe

#endif // !defined(imebraMemoryStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
