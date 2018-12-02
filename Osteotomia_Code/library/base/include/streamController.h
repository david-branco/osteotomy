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

/*! \file streamController.h
    \brief Declaration of the the class used to control the streams.

*/

#if !defined(imebraStreamController_00B3C824_CD0D_4D99_8436_A41FCE9E4D6B__INCLUDED_)
#define imebraStreamController_00B3C824_CD0D_4D99_8436_A41FCE9E4D6B__INCLUDED_

#include "baseObject.h"
#include "baseStream.h"
#include <memory>

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
/// \brief This class represents a stream controller.
///        A stream controller can read or write data
///         from/to a stream.
///
/// Do not use this class directly: use
///  puntoexe::streamWriter or puntoexe::streamReader
///  instead.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class streamController : public baseObject
{

#if(!defined IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE)
	#define IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE 4096
#endif

public:
	/// \brief Construct the stream controller and connect it
	///         to a stream.
	///
	/// The stream controller can be used to control only a
	///  portion of the connected stream.
	///
	/// @param pControlledStream pointer to the controlled
	///                           stream
	/// @param virtualStart      position in the stream that
	///                           is considered as the position
	///                           0 by the stream controller
	/// @param virtualLength     the number of bytes in the
	///                           connected stream that the
	///                           controller will use.
	///                          An EOF will be issued if the
	///                           application tries to read
	///                           beyond the virtual length
	///
	///////////////////////////////////////////////////////////
	streamController(ptr<baseStream> pControlledStream, std::uint32_t virtualStart = 0, std::uint32_t virtualLength = 0);

    virtual ~streamController();

	/// \brief Get the stream's position relative to the
	///         virtual start position specified in the
	///         constructor.
	///
	/// The position is measured in bytes from the beginning
	///  of the stream.
	///
	/// The position represents the byte in the stream that
	///  the next reading or writing function will use.
	///
	/// If the virtual start position has been set in the
	///  constructor, then the returned position is relative
	///  to the virtual start position.
	///
	/// @return the stream's read position, in bytes from the
	///                  beginning of the stream or the virtual
	///                  start position set in the constructor
	///
	///////////////////////////////////////////////////////////
	std::uint32_t position();

	/// \brief Return a pointer to the controlled stream.
	///
	/// @return a pointer to the controlled stream
	///
	///////////////////////////////////////////////////////////
	ptr<baseStream> getControlledStream();

	/// \brief Return the position in bytes from the beginning
	///         of the stream.
	///
	/// It acts like the function position(), but it doesn't
	///  adjust the position to the virtual stream position
	///  specified in the constructor.
	///
	/// @return the stream's read position, in bytes from the
	///                  beginning of the stream
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getControlledStreamPosition();

	///////////////////////////////////////////////////////////
	/// \name Byte ordering
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Specify a byte ordering
	///
	///////////////////////////////////////////////////////////
	enum tByteOrdering
	{
		lowByteEndian=1,  ///< the byte ordering is low endian: the least significant bytes of a word are stored first
		highByteEndian=2  ///< the byte ordering is high endian: the most significant bytes of a word are stored first
	};

	/// \brief Adjust the pBuffer's content according to the
	///         specified byte ordering
	///
	/// If the specified byte ordering is different from the
	///  platform's byte ordering, then the content of
	///  pBuffer is modified to reflect the desidered byte
	///  ordering.
	///
	/// @param pBuffer  a pointer to the buffer that stores
	///                  the data to order
	/// @param wordLength the size, in bytes, of the elements
	///                  stored in the buffer pointed by
	///                  pBuffer.
	///                 E.g.: if the buffer stores a collection
	///                  of words, this parameter will be 2.
	///                 If the buffer stores a collection of
	///                  double words, then this parameter will
	///                  be 4.
	/// @param endianType the desidered byte ordering.
	///                 If it differs from the platform's byte
	///                  ordering, then the content of the
	///                  memory pointed by pBuffer will be
	///                  modified.
	/// @param words    The number of elements stored in the
	///                  buffer pointed by pBuffer.
	///                 This value represents the number of
	///                  element that will be byte ordered.
	///                 The total size of the buffer should be
	///                  equal to words*wordLength
	///
	///////////////////////////////////////////////////////////
	static void adjustEndian(std::uint8_t* pBuffer, const std::uint32_t wordLength, const tByteOrdering endianType, const std::uint32_t words = 1);

    static std::uint16_t adjustEndian(std::uint16_t buffer, const tByteOrdering endianType);

    static std::uint32_t adjustEndian(std::uint32_t buffer, const tByteOrdering endianType);

	//@}

public:

	/// \brief true writeByte() must write all 0xff as
        ///         0xff, 0x00 anf readByte() as to convert all
        ///         0xff,0x00 to 0xff, as in jpeg streams.
	///
	///////////////////////////////////////////////////////////
	bool m_bJpegTags;


protected:
	/// \brief Stream controlled by the stream controller.
	///
	///////////////////////////////////////////////////////////
	ptr<baseStream> m_pControlledStream;

	/// \brief Used for buffered IO
	///
	///////////////////////////////////////////////////////////
    std::uint8_t* m_dataBuffer;

	/// \brief Byte in the stream that represents the byte 0
	///         in the stream controller.
	///
	///////////////////////////////////////////////////////////
	std::uint32_t m_virtualStart;

	/// \brief Max number of bytes that the stream controller
	///         can control in the controlled stream. An EOF
	///         signal will be raised when trying to read
	///         past the maximum length.
	///
	/// If this value is 0 then there are no limits on the
	///  maximum length.
	///
	///////////////////////////////////////////////////////////
	std::uint32_t m_virtualLength;

	std::uint32_t m_dataBufferStreamPosition;
	std::uint8_t* m_pDataBufferStart;
	std::uint8_t* m_pDataBufferCurrent;
	std::uint8_t* m_pDataBufferEnd;
	std::uint8_t* m_pDataBufferMaxEnd;
};

/// \brief Exception thrown when an attempt to read past
///         the end of the file is made.
///
///////////////////////////////////////////////////////////
class streamExceptionEOF : public streamException
{
public:
	streamExceptionEOF(std::string message): streamException(message){}
};

///@}

} // namespace puntoexe

#endif // !defined(imebraStreamController_00B3C824_CD0D_4D99_8436_A41FCE9E4D6B__INCLUDED_)

