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

/*! \file dataGroup.h
    \brief Declaration of the class used to store a single dicom group.

*/

#if !defined(imebraDataGroup_E2A96B14_8398_4a3c_BB3B_A850F27F88AC__INCLUDED_)
#define imebraDataGroup_E2A96B14_8398_4a3c_BB3B_A850F27F88AC__INCLUDED_

#include "dataCollection.h"
#include "dataHandler.h"



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

/// \addtogroup group_dataset
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a Dicom group which stores a group 
///         of Dicom tags.
///
/// The Dicom tags are organized into groups: this class
///  stores all the tags that belong to the same group.
///
/// Groups and tags (represented by the class \ref data)
///  are identified by an ID.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataGroup : public dataCollection<data>
{
public:
	dataGroup(ptr<baseObject> externalLock): dataCollection<data>(externalLock) {}

	/// \brief Get the requested tag (class \ref data).
	///
	/// @param tagId the tag's id (without the group id).
	/// @param bCreate if true and the tag doesn't exist then
	///              a new tag will be created.
	/// @return a pointer to the tag object
	///
	///////////////////////////////////////////////////////////
	ptr<data> getTag(std::uint16_t tagId, bool bCreate=false);
	
	/// \brief Get a handlers::dataHandler object for the 
	///         requested tag's buffer.
	///
	/// A tag is represented by the class \ref data.
	///
	/// The data handler (handlers::dataHandler) allows the 
	///  application to read, write and resize the tag's 
	///  buffer.
	///
	/// A tag can store several buffers: the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param tagId    the tag's id, without the group's id.
	/// @param bufferId the buffer's id (zero based). See
	///                  data::getDataHandler() for more 
	///                  informantion regarding this parameter
	/// @param bWrite   true if the application wants to write
	///                  into the buffer
	/// @param defaultType a string with the dicom data type
	///                  to use if the buffer doesn't exist.
	///                 If none is specified, then a default
	///                  data type will be used.
	///                 This parameter is ignored if bWrite is
	///                  set to false
	/// @return a pointer to the handlers::dataHandler
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandler> getDataHandler(std::uint16_t tagId, std::uint32_t bufferId, bool bWrite, std::string defaultType="");

	/// \brief Get a handlers::dataHandlerRaw object for the 
	///         requested tag's buffer.
	///
	/// A raw data handler always sees the buffer as a 
	///  collection of bytes, no matter what its real data type
	///  is.
	///
	/// A tag is represented by the class \ref data.
	///
	/// The raw data handler (handlers::dataHandlerRaw) 
	///  allows the application to read, write and resize the 
	///  tag's buffer.
	/// The data handler works on a local copy of the data so
	///  the application doesn't have to worry about 
	///  multithreading accesses to the buffer's data.
	///
	/// A tag can store several buffers: the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param tagId    the tag's id, without the group's id.
	/// @param bufferId the buffer's id (zero based). See
	///                  data::getDataHandlerRaw() for more 
	///                  informantion regarding this parameter
	/// @param bWrite   true if the application wants to write
	///                  into the buffer
	/// @param defaultType a string with the dicom data type
	///                  to use if the buffer doesn't exist.
	///                 If none is specified, then a default
	///                  data type will be used.
	///                 This parameter is ignored if bWrite is
	///                  set to false
	/// @return a pointer to the handlers::dataHandlerRaw
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerRaw> getDataHandlerRaw(std::uint16_t tagId, std::uint32_t bufferId, bool bWrite, std::string defaultType="");

	/// \brief Get a streamReader connected to the requested
	///         tag's buffer.
	///
	/// A tag is represented by the class \ref data.
	///
	/// The streamReader allows the application to read the
	///  data from the tag's buffer.
	/// The stream works on a local copy of the data so
	///  the application doesn't have to worry about 
	///  multithreading accesses to the buffer's data.
	///
	/// A tag can store several buffers: the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param tagId    the tag's id, without the group's id.
	/// @param bufferId the buffer's id (zero based). See
	///                  data::getDataHandler() for more 
	///                  informantion regarding this parameter
	/// @return a pointer to the streamReader
	///
	///////////////////////////////////////////////////////////
	ptr<streamReader> getStreamReader(std::uint16_t tagId, std::uint32_t bufferId);

	/// \brief Get a streamWriter connected to the requested
	///         tag's buffer.
	///
	/// A tag is represented by the class \ref data.
	///
	/// The streamWriter allows the application to write
	///  the data into the tag's buffer.
	/// The stream works on a local copy of the data so
	///  the application doesn't have to worry about 
	///  multithreading accesses to the buffer's data.
	///
	/// A tag can store several buffers: the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param tagId    the tag's id, without the group's id.
	/// @param bufferId the buffer's id (zero based). See
	///                  data::getDataHandler() for more 
	///                  informantion regarding this parameter
	/// @param dataType the datatype used to create the 
	///                  buffer if it doesn't exist already
	/// @return a pointer to the streamWriter
	///
	///////////////////////////////////////////////////////////
	ptr<streamWriter> getStreamWriter(std::uint16_t tagId, std::uint32_t bufferId, std::string dataType = "");

	/// \brief Return the data type of the specified tag, in
	///         dicom format.
	///
	/// A dicom data type is composed by 2 uppercase chars.
	///
	/// If the specified tag doesn't exist, then the function
	///  returns an empty string.
	///
	/// @param  tagId the tag's id, without the group's id
	/// @return the tag's data type in dicom format, or an
	///          empty string if the tag doesn't exist in
	///          the group
	///
	///////////////////////////////////////////////////////////
	std::string getDataType(std::uint16_t tagId);

};

/// @}

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraDataGroup_E2A96B14_8398_4a3c_BB3B_A850F27F88AC__INCLUDED_)
