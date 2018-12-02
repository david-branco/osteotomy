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

/*! \file dicomDict.h
    \brief Declaration of the class dicomDict.

*/


#if !defined(imebraDicomDict_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
#define imebraDicomDict_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_

#include "../../base/include/baseObject.h"
#include <map>


///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

/// \addtogroup group_dictionary Dicom dictionary
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief The Dicom Dictionary.
///
/// This class can be used to retrieve the tags' default
///  data types and descriptions.
///
/// An instance of this class is automatically allocated
///  by the library: your application should use the
///  static function getDicomDictionary() in order to
///  get the only valid instance of this class.
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dicomDictionary
{
	struct imageDataDictionaryElement
	{
		std::wstring m_tagName;
		std::string m_tagType;
	};

	struct validDataTypesStruct
	{
		bool  m_longLength;       // true if the tag has a 4 bytes length descriptor
		std::uint32_t m_wordLength;       // Word's length, used for byte reversing in hi/lo endian conversion
		std::uint32_t m_maxLength;        // The maximum length for the tag. An exception will be trown while reading a tag which exceedes this size 
	};

public:
	dicomDictionary();

	void registerTag(std::uint32_t tagId, const wchar_t* tagName, const char* tagType);
	void registerVR(std::string vr, bool bLongLength, std::uint32_t wordSize, std::uint32_t maxLength);

	/// \brief Retrieve a tag's description.
	///
	/// @param groupId   The group which the tag belongs to
	/// @param tagId     The tag's id
	/// @return          The tag's description
	///
	///////////////////////////////////////////////////////////
	std::wstring getTagName(std::uint16_t groupId, std::uint16_t tagId) const;

	/// \brief Retrieve a tag's default data type.
	///
	/// @param groupId   The group which the tag belongs to
	/// @param tagId     The tag's id
	/// @return          The tag's data type
	///
	///////////////////////////////////////////////////////////
	std::string getTagType(std::uint16_t groupId, std::uint16_t tagId) const;

	/// \brief Retrieve the only valid instance of this class.
	///
	/// @return a pointer to the dicom dictionary
	///
	///////////////////////////////////////////////////////////
	static dicomDictionary* getDicomDictionary();

	/// \brief Return true if the specified string represents
	///         a valid dicom data type.
	///
	/// @param dataType the string to be checked
	/// @return         true if the specified string is a valid
	///                  dicom data type
	///
	///////////////////////////////////////////////////////////
	bool isDataTypeValid(std::string dataType) const;

	/// \brief Return true if the tag's length in the dicom 
	///         stream must be written using a DWORD
	///
	/// @param dataType the data type for which the information
	///                  is required
	/// @return         true if the specified data type's 
	///                  length must be written using a DWORD
	///
	///////////////////////////////////////////////////////////
	bool getLongLength(std::string dataType) const ;
	
	/// \brief Return the size of the data type's elements
	///
	/// @param dataType the data type for which the information
	///                  is required
	/// @return the size of a single element
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getWordSize(std::string dataType) const;
	
	/// \brief Return the maximum size of the tags with
	///         the specified data type.
	///
	/// @param dataType the data type for which the information
	///                  is required
	/// @return         the maximum tag's size in bytes 
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getMaxSize(std::string dataType) const;

protected:
	typedef std::map<std::uint32_t, imageDataDictionaryElement> tDicomDictionary;
	tDicomDictionary m_dicomDict;

	typedef std::map<std::string, validDataTypesStruct> tVRDictionary;
	tVRDictionary m_vrDict;

};

/// @}

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraDicomDict_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
