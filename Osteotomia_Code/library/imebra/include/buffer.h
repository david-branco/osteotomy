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

/*! \file buffer.h
    \brief Declaration of the buffer class.

*/

#if !defined(imebraBuffer_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
#define imebraBuffer_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_

#include "../../base/include/baseObject.h"
#include "../../base/include/streamController.h"
#include "../../base/include/memory.h"

#include "charsetsList.h"

///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{
	class streamReader;
	class streamWriter;


namespace imebra
{

namespace handlers
{
	class dataHandler;
	class dataHandlerRaw;
}

/// \addtogroup group_dataset
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class manages a memory area containing
///         data in dicom format.
///
/// A buffer also knows the data type of the
///  elements it stores.
/// The data type is in Dicom format (two upper case
///  chars).
///
/// The memory can be accessed through a 
///  \ref handlers::dataHandler derived object
///  obtained by calling the function getDataHandler().
///  
/// Data handlers work on a copy of the buffer, so most
///  of the problem related to the multithreading
///  enviroments are avoided.
///
/// The data handlers supply several functions that
///  allow to access to the data in several formats
///  (strings, numeric, time, and so on).
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class buffer : public baseObject
{

public:
	///////////////////////////////////////////////////////////
	/// \name Constructor
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Constructor. Initialize the buffer object and
	///        set the default data type.
	///
	/// If no data type is specified, then the Dicom data
	///  type "OB" is used.
	///
	/// @param externalLock a pointer to an object to be
	///                      used to lock this one
	///                      (see baseObject).
	///                     If set to zero then a local
	///                      locker will be used
	/// @param defaultType  a string with the buffer's type.
	///                     The buffer's type must be one of
	///                      the Dicom data types.
	///                     A dicom's data type is formed by
	///                      two uppercase chars
	///
	///////////////////////////////////////////////////////////
	buffer(const ptr<baseObject>& externalLock, const std::string& defaultType="");

	/// \brief Constructor. Initialize the buffer object and
	///         declare the buffer's content on demand.
	///
	/// On demand content is loaded from the original stream
	///  when the application requires the access to the
	///  buffer.
	///
	/// @param externalLock a pointer to an object to be
	///                      used to lock this one
	///                      (see baseObject).
	///                     If set to zero then a local
	///                      locker will be used
	/// @param defaultType  a string with the buffer's type.
	///                     The buffer's type must be one of
	///                      the Dicom data types.
	///                     A dicom's data type is formed by
	///                      two uppercase chars
	/// @param originalStream the stream from which the content
	///                      can be read
	/// @param bufferPosition the first stream's byte that 
	///                      contains the buffer's content
	/// @param bufferLength the buffer's content length, in
	///                      bytes
	/// @param wordLength   the size of a buffer's element,
	///                      in bytes
	/// @param endianType   the stream's endian type
	///
	///////////////////////////////////////////////////////////
	buffer(const ptr<baseObject>& externalLock,
		const std::string& defaultType,
		const ptr<baseStream>& originalStream,
		std::uint32_t bufferPosition,
		std::uint32_t bufferLength,
		std::uint32_t wordLength,
		streamController::tByteOrdering endianType);

	//@}

	///////////////////////////////////////////////////////////
	/// \name Data handlers
	///
	///////////////////////////////////////////////////////////
	//@{
public:
	/// \brief Retrieve a data handler that can be used to
	///         read, write and resize the memory controlled by 
	///         the buffer.
	///
	/// The data handler will have access to a local copy
	///  of the buffer, then it will not have to worry about
	///  multithreading related problems.
	/// If a writing handler is requested, then the handler's
	///  local buffer will be copied back into the buffer when
	///  the handler will be destroyed.
	///
	/// @param bWrite set to true if you want to write into
	///                the buffer
	/// @param size   this parameter is used only when the
	///                parameter bWrite is set to true and the
	///                buffer is empty: in this case, the
	///                returned buffer will be resized to the
	///                number of elements (NOT bytes) declared
	///                in this parameter
	/// @return a pointer to a dataHandler object
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandler> getDataHandler(bool bWrite, std::uint32_t size = 0);

	/// \brief Retrieve a raw data handler that can be used to
	///         read, write and resize the memory controlled by 
	///         the buffer.
	///
	/// Raw data handlers always see a collection of bytes,
	///  regardless of the original buffer's type.
	///
	/// The data handler will have access to a local copy
	///  of the buffer, then it will not have to worry about
	///  multithreading related problems.
	/// If a writing handler is requested, then the handler's
	///  local buffer will be copied back into the buffer when
	///  the handler will be destroyed.
	///
	/// @param bWrite set to true if you want to write into
	///                the buffer
	/// @param size   this parameter is used only when the
	///                parameter bWrite is set to true and the
	///                buffer is empty: in this case, the
	///                returned buffer will be resized to the
	///                number of bytes declared in this
	///                parameter
	/// @return a pointer to a dataHandler object
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerRaw> getDataHandlerRaw(bool bWrite, std::uint32_t size = 0);

	//@}


	///////////////////////////////////////////////////////////
	/// \name Stream
	///
	///////////////////////////////////////////////////////////
	//@{
	
	/// \brief Return the current buffer's size in bytes
	///
	/// If the buffer is currently loaded then return the
	///  memory's size, otherwise return the size that the
	///  buffer would have when it is loaded.
	///
	/// @return the buffer's size, in bytes
	///////////////////////////////////////////////////////////
	std::uint32_t getBufferSizeBytes();

	//@}


	///////////////////////////////////////////////////////////
	/// \name Stream
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Return a stream reader connected to the 
	///         buffer's content.
	///
	/// The stream works on a local copy of the buffer's data,
	///  then it doesn't have to worry about multithreading
	///  related problems.
	///
	/// @return          a pointer to a stream reader
	///
	///////////////////////////////////////////////////////////
	ptr<streamReader> getStreamReader();

	/// \brief Return a stream writer connected to the 
	///         buffer's content.
	///
	/// The stream works on a local copy of the buffer's data,
	///  then it doesn't have to worry about multithreading
	///  related problems.
	///
	/// @return          a pointer to a stream writer
	///
	///////////////////////////////////////////////////////////
	ptr<streamWriter> getStreamWriter();

	//@}


	///////////////////////////////////////////////////////////
	/// \name Buffer's data type
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Returns the buffer's data type.
	///
	/// Return a string with the buffer's data type.
	///
	/// @return a string with the buffer's data type in Dicom
	///          format.
	//
	///////////////////////////////////////////////////////////
	std::string getDataType();

	//@}

	// Disconnect a data handler.
	// Called by the handler during its destruction
	///////////////////////////////////////////////////////////
	void copyBack(handlers::dataHandler* pDisconnectHandler);
	void commit();

	///////////////////////////////////////////////////////////
	/// \name Charsets
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Defines the charsets that should be used by
	///         the object.
	///
	/// The valid charsets are:
	/// - ""
    /// - "ISO_IR 6"
	/// - "ISO_IR 100"
	/// - "ISO_IR 101"
	/// - "ISO_IR 109"
	/// - "ISO_IR 110"
	/// - "ISO_IR 144"
	/// - "ISO_IR 127"
	/// - "ISO_IR 126"
	/// - "ISO_IR 138"
	/// - "ISO_IR 148"
	/// - "ISO_IR 13"
	/// - "ISO_IR 166"
	/// - "ISO 2022 IR 6"
	/// - "ISO 2022 IR 100"
	/// - "ISO 2022 IR 101"
	/// - "ISO 2022 IR 109"
	/// - "ISO 2022 IR 110"
	/// - "ISO 2022 IR 144"
	/// - "ISO 2022 IR 127"
	/// - "ISO 2022 IR 126"
	/// - "ISO 2022 IR 138"
	/// - "ISO 2022 IR 148"
	/// - "ISO 2022 IR 13"
	/// - "ISO 2022 IR 166"
	/// - "ISO 2022 IR 87"
	/// - "ISO 2022 IR 159"
	/// - "ISO 2022 IR 149"
	/// - "ISO_IR 192" (UTF-8)
	/// - "GB18030"
	///
	/// @param pCharsetsList  a list of charsets that can be
	///                        used by the dicom object.
	///                       The default charsets must be 
	///                        the first item in the list
	///
	///////////////////////////////////////////////////////////
	virtual void setCharsetsList(charsetsList::tCharsetsList* pCharsetsList);
	
	/// \brief Retrieve the charsets used by the dicom object.
	///
	/// If during the operation an error is detected (diffetent
	///  objects use different default charsets) then
	///  the exception charsetListExceptionDiffDefault is 
	///  thrown.
	///
	/// @param pCharsetsList  a pointer to a list that will
	///                        be filled with the used 
	///                        charsets
	///
	///////////////////////////////////////////////////////////
	virtual void getCharsetsList(charsetsList::tCharsetsList* pCharsetsList);

	//@}

protected:
	// Return a data handler.
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandler> getDataHandler(bool bWrite, bool bRaw, std::uint32_t size);

	//
	// Attributes
	//
	///////////////////////////////////////////////////////////
private:
	// The memory buffer
	///////////////////////////////////////////////////////////
	ptr<memory> m_memory;

	// Temporary memory buffer and charsets list, used for
	//  the two phase operation copyBack/commit
	///////////////////////////////////////////////////////////
	ptr<memory> m_temporaryMemory;
	charsetsList::tCharsetsList m_temporaryCharsets;
	std::string m_temporaryBufferType;
	
protected:
	// The buffer's type, in Dicom standard
	///////////////////////////////////////////////////////////
	std::string m_bufferType;

protected:
	// The following variables are used to reread the buffer
	//  from the stream.
	///////////////////////////////////////////////////////////
	ptr<baseStream> m_originalStream;    // < Original stream
	std::uint32_t m_originalBufferPosition; // < Original buffer's position
	std::uint32_t m_originalBufferLength;   // < Original buffer's length
	std::uint32_t m_originalWordLength;     // < Original word's length (for low/high endian adjustment)
	streamController::tByteOrdering m_originalEndianType; // < Original endian type
	
private:
	// Charset list
	///////////////////////////////////////////////////////////
	charsetsList::tCharsetsList m_charsetsList;

	// Buffer's version
	///////////////////////////////////////////////////////////
	std::uint32_t m_version;
	
};

///////////////////////////////////////////////////////////
/// \brief This is the base class for the exceptions thrown
///         by the buffer class.
///
///////////////////////////////////////////////////////////
class bufferException: public std::runtime_error
{
public:
	/// \brief Build a buffer exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	bufferException(const std::string& message): std::runtime_error(message){}
};


///////////////////////////////////////////////////////////
/// \brief This exception is throw by the buffer when an
///         handler for an unknown data type is asked.
///
///////////////////////////////////////////////////////////
class bufferExceptionUnknownType: public bufferException
{
public:
	/// \brief Build a wrong data type exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	bufferExceptionUnknownType(const std::string& message): bufferException(message){}
};

/// @}

} // End of namespace imebra

} // End of namespace puntoexe

#endif // !defined(imebraBuffer_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
