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

/*! \file dataHandler.h
    \brief Declaration of the base class used by all the data handlers.

*/

#if !defined(imebraDataHandler_6F85D344_DEF8_468d_BF73_AC5BB17FD22A__INCLUDED_)
#define imebraDataHandler_6F85D344_DEF8_468d_BF73_AC5BB17FD22A__INCLUDED_

#include "../../base/include/baseObject.h"
#include "buffer.h"
#include "charsetsList.h"


///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

	class transaction; // is a friend

/// \namespace handlers
/// \brief All the data handlers returned by the class
///         buffer are defined in this namespace
///
///////////////////////////////////////////////////////////
namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This is the base class for all the imebra data
///         handlers.
///        A data handler allows to read/write the data
///         stored in a \ref puntoexe::imebra::buffer 
///         object without worrying about the %data format.
///
/// Data handlers work on a local copy of the buffer
///  so they don't need to worry about multithreading
///  accesses.
///
/// Also, once a dataHandler has been obtained from
///  a \ref buffer, it cannot be shared between threads
///  and it doesn't provide any multithread-safe mechanism,
///  except for its destructor which copies the local
///  buffer back to the original one (only for the writable
///  handlers).
///
/// Data handlers are also used to access to the
///  decompressed image's pixels (see image and 
///  handlers::dataHandlerNumericBase).
///
/// To obtain a data handler your application has to
///  call buffer::getDataHandler() or 
///  image::getDataHandler().
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataHandler : public baseObject
{
	// buffer & transaction are friend of this class
	///////////////////////////////////////////////////////////
	friend class puntoexe::imebra::buffer;
	friend class puntoexe::imebra::transaction;

public:
	// Contructor
	///////////////////////////////////////////////////////////
	dataHandler(): m_bCommitted(false){}

	// The data handler is disconnected
	///////////////////////////////////////////////////////////
	virtual bool preDelete();

	/// \brief In a writing handler copies back the modified
	///         data to the buffer.
	///
	/// Is not necessary to call this function directly because
	///  it is called by the handler's destructor, which copy
	///  any modification back to the buffer and finalize it.
	///
	/// The operation must be finalized by a call to commit(),
	///  or will be finalized by the destructor unless a call
	///  to abort() happen.
	///
	///////////////////////////////////////////////////////////
	void copyBack();
	
	/// \brief Finalize the copy of the data from the handler
	///         to the buffer.
	///
	/// Is not necessary to call this function directly because
	///  it is called by the handler's destructor, which copy
	///  any modification back to the buffer and then finalize
	///  it.
	///
	///////////////////////////////////////////////////////////
	void commit();

	/// \brief Discard all the changes made on a writing
	///         handler.
	///
	/// The function also switches the write flag to false,
	///  so it also prevent further changes from being
	///  committed into the buffer.
	///
	///////////////////////////////////////////////////////////
	void abort();

public:
	///////////////////////////////////////////////////////////
	/// \name Data pointer
	///
	/// The following functions set the value of the internal
	///  pointer that references the %data element read/written
	///  by the reading/writing functions.
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Returns true if the specified index points to
	///        a valid element in the buffer.
	///
	/// If the index is out of the valid range, then this
	///  method returns false.
	///
	/// @param index the index to be tested
	/// @return true if the index is valid, false if it is out
	///          of range
	///
	///////////////////////////////////////////////////////////
	virtual bool pointerIsValid(const std::uint32_t index) const=0;

	//@}


	///////////////////////////////////////////////////////////
	/// \name Buffer and elements size
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Set the buffer's size, in elements.
	///
	/// The function resize the data handler's local buffer
	///  to the requested number of elements.
	///
	/// @param elementsNumber the requested buffer's size,
	///                        in data elements
	///
	///////////////////////////////////////////////////////////
	virtual void setSize(const std::uint32_t elementsNumber) =0;

	/// \brief Retrieve the data handler's local buffer buffer
	///         size (in elements).
	///
	/// @return the buffer's size in elements
	///
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getSize() const=0;

	/// \brief Returns a single element's size in bytes.
	///
	/// If the element doesn't have a fixed size, then
	///  this function return 0.
	///
	/// @return the element's size in bytes, or zero if the
	///         element doesn't have a fixed size
	///
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getUnitSize() const=0;

	//@}
	

	///////////////////////////////////////////////////////////
	/// \internal
	/// \name Copy the local buffer from/to the buffer
	///
	///////////////////////////////////////////////////////////
	
	/// \internal
	/// \brief This function copies the %data from the 
	///         \ref buffer into the local %buffer
	///
	/// @param memoryBuffer the \ref memory managed by the 
	///                      \ref buffer
	///
	///////////////////////////////////////////////////////////
	virtual void parseBuffer(const ptr<memory>& memoryBuffer)=0;
	
	/// \internal
	/// \brief This function copies the %data from the 
	///         \ref buffer into the local %buffer
	///
	/// @param pBuffer a pointer to the %memory that stores the
	///                 data to be copied
	/// @param bufferLength the number of bytes stored in
	///                 pBuffer
	///
	///////////////////////////////////////////////////////////
	void parseBuffer(const std::uint8_t* pBuffer, const std::uint32_t bufferLength);

	/// \internal
	/// \brief Copies the local %buffer into the original
	///         \ref buffer object.
	///
	/// @param memoryBuffer the \ref memory managed by the 
	///                      \ref buffer
	///
	///////////////////////////////////////////////////////////
	virtual void buildBuffer(const ptr<memory>& memoryBuffer)=0;

	/// \internal
	/// \brief Defines the charsets used in the string
	///
	/// @param pCharsetsList a list of dicom charsets
	///
	///////////////////////////////////////////////////////////
	virtual void setCharsetsList(charsetsList::tCharsetsList* pCharsetsList);

	/// \internal
	/// \brief Retrieve the charsets used in the string.
	///
	/// @param pCharsetsList a list that will be filled with the
	///                      dicom charsets used in the string
	///
	///////////////////////////////////////////////////////////
	virtual void getCharsetsList(charsetsList::tCharsetsList* pCharsetsList) const;

	//@}


	///////////////////////////////////////////////////////////
	/// \name Attributes
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Get the dicom data type managed by this handler.
	///
	/// The dicom data type is formed by 2 uppercase chars,
	///  as described by the dicom standard.
	/// See \ref buffer for further information.
	///
	/// @return the data handler's dicom data type
	///
	///////////////////////////////////////////////////////////
	std::string getDataType() const;

	/// \brief Return the byte that this handler uses to fill
	///         its content to make its size even.
	///
	/// @return the byte used to make the content's size even
	///
	///////////////////////////////////////////////////////////
	virtual std::uint8_t getPaddingByte() const;

	//@}


	///////////////////////////////////////////////////////////
	/// \name Reading/writing functions
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Retrieve the buffer's element referenced by the
	///         zero-based index specified in the parameter and
	///         returns it as a signed long value.
	///
	/// Returns 0 if the specified index is out of range.
	/// You can check the validity of the index by using the
	///  function pointerIsValid().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to retrieve
	/// @return the value of the data element referenced by
	///          the index, transformed into a signed long, or
	///          0 if the index is out of range
	///
	///////////////////////////////////////////////////////////
	virtual std::int32_t getSignedLong(const std::uint32_t index) const=0;

	/// \brief Retrieve the buffer's element referenced by the
	///         zero-based index specified in the parameter and
	///         returns it as an unsigned long value.
	///
	/// Returns 0 if the specified index is out of range.
	/// You can check the validity of the index by using the
	///  function pointerIsValid().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to retrieve
	/// @return the value of the data element referenced by
	///          the index, transformed into an unsigned long,
	///          or 0 if the index is out of range
	///
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getUnsignedLong(const std::uint32_t index) const =0;

	/// \brief Retrieve the buffer's element referenced by the
	///         zero-based index specified in the parameter and
	///         returns it as a double floating point value.
	///
	/// Returns 0 if the specified index is out of range.
	/// You can check the validity of the index by using the
	///  function pointerIsValid().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to retrieve
	/// @return the value of the data element referenced by
	///          the index, transformed into a double floating
	///          point, or 0 if the index is out of range
	///
	///////////////////////////////////////////////////////////
	virtual double getDouble(const std::uint32_t index) const=0;

	/// \brief Retrieve the buffer's element referenced by the
	///         zero-based index specified in the parameter and
	///         returns it as a string value.
	///
	/// Returns 0 if the specified index is out of range.
	/// You can check the validity of the index by using the
	///  function pointerIsValid().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to retrieve
	/// @return the value of the data element referenced by
	///          the index, transformed into a string, or
	///          0 if the index is out of range
	///
	///////////////////////////////////////////////////////////
	virtual std::string getString(const std::uint32_t index) const= 0;

	/// \brief Retrieve the buffer's element referenced by the
	///         zero-based index specified in the parameter and
	///         returns it as an unicode string value.
	///
	/// Returns 0 if the specified index is out of range.
	/// You can check the validity of the index by using the
	///  function pointerIsValid().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to retrieve
	/// @return the value of the data element referenced by
	///          the index, transformed into an unicode string,
	///          or 0 if the index is out of range
	///
	///////////////////////////////////////////////////////////
	virtual std::wstring getUnicodeString(const std::uint32_t index) const = 0;

	/// \brief Retrieve the buffer's element referenced by the
	///         zero-based index specified in the parameter and
	///         returns it as a date/time value.
	///
	/// Returns all zeros if the specified index is out of
	///  range.
	/// You can check the validity of the index by using the
	///  function pointerIsValid().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to retrieve
	/// @param pYear   a pointer to a value that will be filled
	///                 with the UTC date's year
	/// @param pMonth  a pointer to a value that will be filled
	///                 with the UTC date's month
	/// @param pDay    a pointer to a value that will be filled
	///                 with the UTC date's day of the month
	/// @param pHour   a pointer to a value that will be filled
	///                 with the UTC hour
	/// @param pMinutes a pointer to a value that will be 
	///                 filled with the UTC minutes
	/// @param pSeconds a pointer to a value that will be 
	///                 filled with the UTC seconds
	/// @param pNanoseconds a pointer to a value that will be 
	///                 filled with the UTC nanosecods
	/// @param pOffsetHours a pointer to a value that will be 
	///                 filled with the difference between the
	///                 date time zone and the UTC time zone
	/// @param pOffsetMinutes a pointer to a value that will be 
	///                 filled with the difference between the
	///                 date time zone and the UTC time zone
	///
	///////////////////////////////////////////////////////////
	virtual void getDate(const std::uint32_t index,
		std::int32_t* pYear, 
		std::int32_t* pMonth, 
		std::int32_t* pDay, 
		std::int32_t* pHour, 
		std::int32_t* pMinutes,
		std::int32_t* pSeconds,
		std::int32_t* pNanoseconds,
		std::int32_t* pOffsetHours,
		std::int32_t* pOffsetMinutes) const;

	/// \brief Set the buffer's element referenced by the
	///         zero-based index specified in the parameter
	///         to a date/time value.
	///
	/// Does nothing if the specified index is out of range
	/// You can check the validity of the index by using the
	///  function pointerIsValid(), you can resize the buffer
	///  by using the function setSize().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to be set
	/// @param year   the UTC date's year
	/// @param month  the UTC date's month
	/// @param day    the UTC date's day of the month
	/// @param hour   the UTC hour
	/// @param minutes the UTC minutes
	/// @param seconds the UTC seconds
	/// @param nanoseconds the UTC nanosecods
	/// @param offsetHours the difference between the date time 
	///                zone and the UTC time zone
	/// @param offsetMinutes the difference between the date
	///                time zone and the UTC time zone
	///
	///////////////////////////////////////////////////////////
	virtual void setDate(const std::uint32_t index,
		std::int32_t year, 
		std::int32_t month, 
		std::int32_t day, 
		std::int32_t hour, 
		std::int32_t minutes,
		std::int32_t seconds,
		std::int32_t nanoseconds,
		std::int32_t offsetHours,
		std::int32_t offsetMinutes);

	/// \brief Set the buffer's element referenced by the
	///         zero-based index specified in the parameter
	///         to a signed long value.
	///
	/// Does nothing if the specified index is out of range
	/// You can check the validity of the index by using the
	///  function pointerIsValid(), you can resize the buffer
	///  by using the function setSize().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to be set
	/// @param value the value to write into the
	///				  %data element.
	///
	///////////////////////////////////////////////////////////
	virtual void setSignedLong(const std::uint32_t index, const std::int32_t value) =0;

	/// \brief Set the buffer's element referenced by the
	///         zero-based index specified in the parameter
	///         to an unsigned long value.
	///
	/// Does nothing if the specified index is out of range
	/// You can check the validity of the index by using the
	///  function pointerIsValid(), you can resize the buffer
	///  by using the function setSize().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to be set
	/// @param value the value to write into the
	///				  %data element.
	///
	///////////////////////////////////////////////////////////
	virtual void setUnsignedLong(const std::uint32_t index, const std::uint32_t value) =0;

	/// \brief Set the buffer's element referenced by the
	///         zero-based index specified in the parameter
	///         to a double floating point value.
	///
	/// Does nothing if the specified index is out of range
	/// You can check the validity of the index by using the
	///  function pointerIsValid(), you can resize the buffer
	///  by using the function setSize().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to be set
	/// @param value the value to write into the
	///				  %data element.
	///
	///////////////////////////////////////////////////////////
	virtual void setDouble(const std::uint32_t index, const double value) =0;

	/// \brief Set the buffer's element referenced by the
	///         zero-based index specified in the parameter
	///         to a string value. See also setUnicodeString().
	///
	/// Does nothing if the specified index is out of range
	/// You can check the validity of the index by using the
	///  function pointerIsValid(), you can resize the buffer
	///  by using the function setSize().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to be set
	/// @param value the value to write into the
	///				  %data element.
	///////////////////////////////////////////////////////////
	virtual void setString(const std::uint32_t index, const std::string& value) =0;

	/// \brief Set the buffer's element referenced by the
	///         zero-based index specified in the parameter
	///         to a string value. See also setString().
	///
	/// Does nothing if the specified index is out of range
	/// You can check the validity of the index by using the
	///  function pointerIsValid(), you can resize the buffer
	///  by using the function setSize().
	///
	/// @param index   the zero base index of the buffer's
	///                 element to be set
	/// @param value the value to write into the %data
	///				  element.
	///////////////////////////////////////////////////////////
	virtual void setUnicodeString(const std::uint32_t index, const std::wstring& value) =0;

	//@}


protected:
	// true if the buffer has been committed
	///////////////////////////////////////////////////////////
	bool m_bCommitted;

	// Pointer to the connected buffer
	///////////////////////////////////////////////////////////
	ptr<buffer> m_buffer;

	std::string m_bufferType;

	charsetsList::tCharsetsList m_charsetsList;
};

} // namespace handlers

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraDataHandler_6F85D344_DEF8_468d_BF73_AC5BB17FD22A__INCLUDED_)
