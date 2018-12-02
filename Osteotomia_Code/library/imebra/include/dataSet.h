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

/*! \file dataSet.h
    \brief Declaration of the class dataSet.

*/

#if !defined(imebraDataSet_93F684BF_0024_4bf3_89BA_D98E82A1F44C__INCLUDED_)
#define imebraDataSet_93F684BF_0024_4bf3_89BA_D98E82A1F44C__INCLUDED_

#include "dataCollection.h"
#include "dataGroup.h"
#include "../../base/include/exception.h"
#include "codec.h"

#include <vector>


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

//The following classes are used in this declaration file
class image;
class lut;
class waveform;

/// \addtogroup group_dataset Dicom data
/// \brief The Dicom dataset is represented by the
///         class dataSet.
///
/// @{



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief A data set is a collection of groups of tags
///        (see dataGroup).
///
/// The dataSet is usually built from a dicom stream by
///  using the codec codecs::dicomCodec.
///
/// Also the tags with the data type SQ (sequence) contains
///  one or more embedded dataSets that can be retrieved
///  by using data::getDataSet().
///
/// If your application creates a new dataset then it can
///  set the default dataSet's charset by calling 
///  setCharsetsList(). See \ref imebra_unicode for
///  more information related to Imebra and the Dicom
///   charsets.
///
/// The dataSet and its components (all the dataGroup, 
///  and data) share a common lock object:
///  this means that a lock on one of the dataSet's
///  component will lock the entire dataSet and all
///  its components.
///
/// For an introduction to the dataSet, read 
///  \ref quick_tour_dataSet.
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSet : public dataCollection<dataGroup>
{
public:
	// Costructor
	///////////////////////////////////////////////////////////
	dataSet(): dataCollection<dataGroup>(ptr<baseObject>(new baseObject)), m_itemOffset(0) {}

	///////////////////////////////////////////////////////////
	/// \name Get/set groups/tags
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Retrieve a tag object.
	///
	/// Tag object is represented by the \ref data class.
	///
	/// If the tag doesn't exist and the parameter bCreate is
	///  set to false, then the function returns a null
	///  pointer.
	/// If the tag doesn't exist and the parameter bCreate is
	///  set to true, then an empty tag will be created and
	///  inserted into the dataset.
	///
	/// @param groupId The group to which the tag belongs.
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify which group
	///                 must be retrieved.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero.
	/// @param tagId   The id of the tag to retrieve.
	/// @param bCreate When bCreate is set to true and the
	///                 requested tag doesn't exist,
	///                 then a new one is created and inserted
	///                 into the dataset.
	///                When bCreate is set to false and the
	///                 requested tag doesn't exist, then
	///                 a null pointer is returned.
	/// @return        A pointer to the retrieved tag.
	///                If the requested tag doesn't exist then
	///                 the returned value depend on the value
	///                 of the bCreate parameter: when bCreate
	///                 is false then a value of zero is
	///                 returned, otherwise a pointer to the
	///                 just created tag is returned.
	///
	///////////////////////////////////////////////////////////
	ptr<data> getTag(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, bool bCreate=false);
	
	/// \brief Retrieve a group object.
	///
	/// A Group object is represented by the \ref dataGroup
	/// class.
	///
	/// If the group doesn't exist and the parameter bCreate
	///  is set to false, then the function returns a null
	///  pointer.
	/// If the group doesn't exist and the parameter bCreate
	///  is set to true, then an empty group will be created
	///  and inserted into the dataset.
	///
	/// @param groupId The group to retrieve.
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify which group
	///                 must be retrieved.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero.
	/// @param bCreate When bCreate is set to true and the
	///                 requested group doesn't exist,
	///                 then a new one is created and inserted
	///                 into the dataset.
	///                When bCreate is set to false and the
	///                 requested group doesn't exist, then
	///                 a null pointer is returned.
	/// @return        A pointer to the retrieved group.
	///                The group should be released as soon as
	///                 possible using the function Release().
	///                If the requested group doesn't exist
	///                 then the returned value depend on the
	///                 value of the bCreate parameter: when
	///                 bCreate is false then a value of zero
	///                 is returned, otherwise a pointer to the
	///                 just created group is returned.
	///
	///////////////////////////////////////////////////////////
	ptr<dataGroup> getGroup(std::uint16_t groupId, std::uint16_t order, bool bCreate=false);
	
	/// \brief Insert the specified group into the dataset.
	///
	/// A Group object is represented by the \ref dataGroup
	///  class.
	///
	/// If a group with the same id and order is already
	///  present into the data set, then it is removed to
	///  leave space to the new group.
	///
	/// @param groupId The id of the group to insert into
	///                 the data set.
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero.
	/// @param pGroup  A pointer to the group to insert into
	///                 the data set.
	///
	///////////////////////////////////////////////////////////
	void setGroup(std::uint16_t groupId, std::uint16_t order, ptr<dataGroup> pGroup);

	//@}


	///////////////////////////////////////////////////////////
	/// \name Get/set the image
	///
	///////////////////////////////////////////////////////////
	//@{
	
	/// \brief Retrieve an image from the dataset.
	///
	/// The right codec will be automatically used to
	///  decode the image embedded into the dataset.
	/// If multiple frames are available, then the
	///  calling application can decide the frame to
	///  retrieve.
	///
	/// The function throw an exception if the requested
	///  image doesn't exist or the image's tag is
	///  corrupted.
	///
	/// The retrieved image should then be processed by the
	///  transforms::modalityVOILUT transform in order to 
	///  convert the pixels value to a meaningful space.
	/// Infact, the dicom image's pixel values saved by
	///  other application have a meaningful value only for
	///  the application that generated them, while the
	///  modality VOI/LUT transformation will convert those
	///  values to a more portable unit (e.g.: optical
	///  density).
	///
	/// Further transformations are applied by 
	///  the transforms::VOILUT transform, in order to 
	///  adjust the image's contrast for displaying purposes.
	///  
	/// @param frameNumber The frame number to retrieve.
	///                    The first frame's id is 0
	/// @return            A pointer to the retrieved
	///                     image
	///
	///////////////////////////////////////////////////////////
	ptr<image> getImage(std::uint32_t frameNumber);

    /// \brief Retrieve an image from the dataset and apply the
    ///        modality transform if it is specified in the
    ///        dataset.
    ///
    /// If the dataSet does not contain any modality transform
    ///  then returns the same image returned by getImage().
    ///
    /// @param frameNumber The frame number to retrieve.
    ///                    The first frame's id is 0
    /// @return            A pointer to the retrieved
    ///                     image to which the modality
    ///                     transform has been applied
    ///
    ///////////////////////////////////////////////////////////
    ptr<image> getModalityImage(std::uint32_t frameNumber);
	
	/// \brief Insert an image into the data set.
	///
	/// The specified transfer syntax will be used to choose
	///  the right codec for the image.
	///
	/// @param frameNumber The frame number where the image
	///                     must be stored.
	///                    The first frame's id is 0.
	/// @param pImage      A pointer to the image object to
	///                     be stored into the data set.
	/// @param transferSyntax the transfer syntax that
	///                     specifies the codec and the 
	///                     parameters to use for the encoding
	/// @param quality     an enumeration that set the 
	///                     compression quality
	///
	///////////////////////////////////////////////////////////
	void setImage(std::uint32_t frameNumber, ptr<image> pImage, std::wstring transferSyntax, codecs::codec::quality quality);

	/// \brief Get a frame's offset from the offset table.
	///
	/// @param frameNumber the number of the frame for which
	///                     the offset is requested
	/// @return the offset for the specified frame
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getFrameOffset(std::uint32_t frameNumber);

	/// \brief Get the id of the buffer that starts at the
	///         specified offset.
	///
	/// @param offset   one offset retrieved from the frames
	///                  offset table: see getFrameOffset()
	/// @param pLengthToBuffer a pointer to a variable that
	///                  will store the total lenght of
	///                  the buffers that preceed the one
	///                  being returned (doesn't include
	///                  the tag descriptors)
	/// @return         the id of the buffer that starts at
	///                  the specified offset
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getFrameBufferId(std::uint32_t offset, std::uint32_t* pLengthToBuffer);

	/// \brief Retrieve the first and the last buffers used
	///         to store the image.
	///
	/// This function works only with the new Dicom3 streams,
	///  not with the old NEMA format.
	///
	/// This function is used by setImage() and getImage().
	///
	/// @param frameNumber the frame for which the buffers 
	///                     have to be retrieved
	/// @param pFirstBuffer a pointer to a variable that will
	///                     contain the id of the first buffer
	///                     used to store the image
	/// @param pEndBuffer  a pointer to a variable that will
	///                     contain the id of the first buffer
	///                     next to the last one used to store 
	///                     the image
	/// @return the total length of the buffers that contain
	///          the image
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getFrameBufferIds(std::uint32_t frameNumber, std::uint32_t* pFirstBuffer, std::uint32_t* pEndBuffer);
	
	/// \brief Return the first buffer's id available where
	///         a new frame can be saved.
	///
	/// @return the id of the first buffer available to store
	///          a new frame
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getFirstAvailFrameBufferId();

	//@}


	///////////////////////////////////////////////////////////
	/// \name Get/set a sequence item
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Retrieve a data set embedded into a sequence
	///        tag.
	///
	/// Sequence tags store several binary data which can be 
	///  individually parsed as a normal dicom file 
	///  (without the preamble of 128 bytes and the DICM 
	///   signature).
	///
	/// When using sequences an application can store several
	///  nested dicom structures.
	///
	/// This function parse a single item of a sequence tag
	///  and return a data set object (represented by a 
	///  this class) which stores the retrieved
	///  tags.
	///
	/// If the requested tag's type is not a sequence or the
	///  requested item in the sequence is missed, then a null
	///  pointer will be returned.
	///
	/// @param groupId The group to which the sequence 
	///                 tag to be parsed belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify which group
	///                 must be retrieved.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to parse
	/// @param itemId  The id of the tag's item to parse
	///                (zero based)
	/// @return        A pointer to the retrieved data set.
	///                If the requested group, tag or buffer
	///                 (sequence item) doesn't exist, or if
	///                 the tag's type is not a sequence (SQ),
	///                 then a null pointer is returned,
	///                 otherwise a pointer to the retrieved
	///                 dataset is returned
	///
	///////////////////////////////////////////////////////////
	ptr<dataSet> getSequenceItem(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t itemId);

	/// \brief Retrieve a LUT.
	///
	/// LUT are encoded into sequences.
	/// This function retrieve the sequence and build
	///  a \ref lut object describing the LUT.
	///
	/// @param groupId The group to which the LUT sequence 
	///                 belongs
	/// @param tagId   The id of the tag to containing the
	///                 LUT
	/// @param lutId   The id of the lut inside the tag (0
	///                 based)
	/// @return        A pointer to the retrieved LUT.
	///                If the requested group, tag or buffer
	///                 (sequence item) doesn't exist, or if
	///                 the tag's type is not a sequence (SQ),
	///                 then a null pointer is returned,
	///                 otherwise a pointer to the retrieved
	///                 LUT is returned
	///
	///////////////////////////////////////////////////////////
	ptr<lut> getLut(std::uint16_t groupId, std::uint16_t tagId, std::uint32_t lutId);

	/// \brief Retrieve a waveform from the dataSet.
	///
	/// Each waveforms is stored in a sequence item;
	/// the function retrieves the proper sequence item and
	///  connects it to the class waveform which can be used
	///  to retrieve the waveform data.
	///
	/// @param waveformId   the zero based index of the 
	///                      waveform to retrieve
	/// @return an object waveform that can be used to read
	///          the waveform data, or a null pointer if
	///          the requested waveform doesn't exist
	///
	///////////////////////////////////////////////////////////
	ptr<waveform> getWaveform(std::uint32_t waveformId);

	//@}


	///////////////////////////////////////////////////////////
	/// \name Get/set the tags' values
	///
	///////////////////////////////////////////////////////////
	//@{
public:	
	/// \brief Read the value of the requested tag and return
	///         it as a signed long.
	///
	/// @param groupId The group to which the tag to be read
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to retrieve
	/// @param elementNumber The element's number to retrieve.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be retrieved.
	///                The first element's number is 0
	/// @return        The tag's content, as a signed long
	///
	///////////////////////////////////////////////////////////
	std::int32_t getSignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber);

	/// \brief Set a tag's value as a signed long.
	///
	/// If the specified tag doesn't exist, then a new one
	///  will be created and inserted into the dataset.
	///
	/// @param groupId The group to which the tag to be write
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to set
	/// @param elementNumber The element's number to set.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be set.
	///                The first element's number is 0
	/// @param newValue the value to be written into the tag
	/// @param defaultType if the specified tag doesn't exist
	///                 then the function will create a new
	///                 tag with the data type specified in
	///                 this parameter
	///
	///////////////////////////////////////////////////////////
	void setSignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::int32_t newValue, std::string defaultType = "");

	/// \brief Retrieve a tag's value as an unsigned long.
	///
	/// Read the value of the requested tag and return it as
	///  an unsigned long.
	///
	/// @param groupId The group to which the tag to be read
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to retrieve
	/// @param elementNumber The element's number to retrieve.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be retrieved.
	///                The first element's number is 0
	/// @return        The tag's content, as an unsigned long
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getUnsignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber);

	/// \brief Set a tag's value as an unsigned long.
	///
	/// If the specified tag doesn't exist, then a new one
	///  will be created and inserted into the dataset.
	///
	/// @param groupId The group to which the tag to be write
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to set
	/// @param elementNumber The element's number to set.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be set.
	///                The first element's number is 0
	/// @param newValue the value to be written into the tag
	/// @param defaultType if the specified tag doesn't exist
	///                 then the function will create a new
	///                 tag with the data type specified in
	///                 this parameter
	///
	///////////////////////////////////////////////////////////
	void setUnsignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::uint32_t newValue, std::string defaultType = "");

	/// \brief Retrieve a tag's value as a double.
	///
	/// Read the value of the requested tag and return it as
	///  a double.
	///
	/// @param groupId The group to which the tag to be read
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to retrieve
	/// @param elementNumber The element's number to retrieve.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be retrieved.
	///                The first element's number is 0
	/// @return        The tag's content, as a double
	///
	///////////////////////////////////////////////////////////
	double getDouble(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber);
	
	/// \brief Set a tag's value as a double.
	///
	/// If the specified tag doesn't exist, then a new one
	///  will be created and inserted into the dataset.
	///
	/// @param groupId The group to which the tag to be write
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to set
	/// @param elementNumber The element's number to set.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be set.
	///                The first element's number is 0
	/// @param newValue the value to be written into the tag
	/// @param defaultType if the specified tag doesn't exist
	///                 then the function will create a new
	///                 tag with the data type specified in
	///                 this parameter
	///
	///////////////////////////////////////////////////////////
	void setDouble(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, double newValue, std::string defaultType = "");

	/// \brief Retrieve a tag's value as a string.
	///        getUnicodeString() is preferred over this
	///         method.
	///
	/// Read the value of the requested tag and return it as
	///  a string.
	///
	/// @param groupId The group to which the tag to be read
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to retrieve
	/// @param elementNumber The element's number to retrieve.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be retrieved.
	///                The first element's number is 0
	/// @return        The tag's content, as a string
	///
	///////////////////////////////////////////////////////////
	std::string getString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber);

	/// \brief Retrieve a tag's value as an unicode string.
	///
	/// Read the value of the requested tag and return it as
	///  an unicode string.
	///
	/// @param groupId The group to which the tag to be read
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to retrieve
	/// @param elementNumber The element's number to retrieve.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be retrieved.
	///                The first element's number is 0
	/// @return        The tag's content, as an unicode string
	///
	///////////////////////////////////////////////////////////
	std::wstring getUnicodeString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber);

	/// \brief Set a tag's value as a string.
	///        setUnicodeString() is preferred over this
	///         method.
	///
	/// If the specified tag doesn't exist, then a new one
	///  will be created and inserted into the dataset.
	///
	/// @param groupId The group to which the tag to be write
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to set
	/// @param elementNumber The element's number to set.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be set.
	///                The first element's number is 0
	/// @param newString the value to be written into the tag
	/// @param defaultType if the specified tag doesn't exist
	///                 then the function will create a new
	///                 tag with the data type specified in
	///                 this parameter
	///
	///////////////////////////////////////////////////////////
	void setString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::string newString, std::string defaultType = "");

	/// \brief Set a tag's value as an unicode string.
	///
	/// If the specified tag doesn't exist, then a new one
	///  will be created and inserted into the dataset.
	///
	/// @param groupId The group to which the tag to be write
	///                 belongs
	/// @param order   If the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the group belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   The id of the tag to set
	/// @param elementNumber The element's number to set.
	///                A buffer can store several elements:
	///                 this parameter specifies which element
	///                 must be set.
	///                The first element's number is 0
	/// @param newString the value to be written into the tag
	/// @param defaultType if the specified tag doesn't exist
	///                 then the function will create a new
	///                 tag with the data type specified in
	///                 this parameter
	///
	///////////////////////////////////////////////////////////
	void setUnicodeString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::wstring newString, std::string defaultType = "");

	//@}


	///////////////////////////////////////////////////////////
	/// \name Data handlers
	///
	///////////////////////////////////////////////////////////
	//@{
public:
	/// \brief Return the default data type for the specified
	///        tag's id.
	///
	/// The default data type is retrieved from an internal
	///  dictionary which stores the default properties of
	///  each dicom's tag.
	///
	/// @param groupId    The group to which the tag
	///                   belongs
	/// @param tagId      The id of the tag.
	/// @return           the tag's default type.
	///                   The returned string is a constant.
	///////////////////////////////////////////////////////////
	std::string getDefaultDataType(std::uint16_t groupId, std::uint16_t tagId);

	/// \brief Return the data type of a tag
	///
	/// @param groupId    The group to which the tag belongs
	/// @param order      When multiple groups with the same
	///                    it are present, then use this
	///                    parameter to specify which group
	///                    must be used. The first group as
	///                    an order of 0.
	/// @param tagId      The id of the tag for which the type
	///                    must be retrieved.
	/// @return           a string with the tag's type.
	///
	///////////////////////////////////////////////////////////
	std::string getDataType(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId);

	/// \brief Return a data handler for the specified tag's
	///         buffer.
	///
	/// The data handler allows the application to read, write
	///  and resize the tag's buffer.
	///
	/// A tag can store several buffers, then the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param groupId the group to which the tag belongs
	/// @param order   if the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the tag belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   the tag's id
	/// @param bufferId the buffer's id (zero based)
	/// @param bWrite  true if the application wants to write
	///                 into the buffer
	/// @param defaultType a string with the dicom data type 
	///                 to use if the buffer doesn't exist.
	///                If none is specified, then a default
	///                 data type will be used
	/// @return a pointer to the data handler.
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandler> getDataHandler(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId, bool bWrite, std::string defaultType="");

	/// \brief Return a raw data handler for the specified 
	///         tag's buffer.
	///
	/// A raw data handler always sees the buffer as a 
	///  collection of bytes, no matter what the tag's data
	///  type is.
	///
	/// A tag can store several buffers, then the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param groupId the group to which the tag belongs
	/// @param order   if the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the tag belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   the tag's id
	/// @param bufferId the buffer's id (zero based)
	/// @param bWrite  true if the application wants to write
	///                 into the buffer
	/// @param defaultType a string with the dicom data type 
	///                 to use if the buffer doesn't exist.
	///                If none is specified, then a default
	///                 data type will be used
	/// @return a pointer to the data handler.
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerRaw> getDataHandlerRaw(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId, bool bWrite, std::string defaultType="");

	/// \brief Return a streamReader connected to the specified
	///         tag's buffer's memory.
	///
	/// A tag can store several buffers: the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param groupId the group to which the tag belongs
	/// @param order   if the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the tag belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   the tag's id
	/// @param bufferId the buffer's id (zero based)
	/// @return a pointer to the streamReader
	///
	///////////////////////////////////////////////////////////
	ptr<streamReader> getStreamReader(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId);

	/// \brief Return a streamWriter connected to the specified
	///         tag's buffer's memory.
	///
	/// A tag can store several buffers, then the application
	///  must specify the buffer's id it wants to deal with.
	///
	/// @param groupId the group to which the tag belongs
	/// @param order   if the group is recurring in the file
	///                 (it appears several times), then use
	///                 this parameter to specify to which
	///                 group the tag belongs.
	///                This parameter is used to deal with
	///                 old DICOM files, since the new one
	///                 should use the sequence items to
	///                 achieve the same result.
	///                It should be set to zero
	/// @param tagId   the tag's id
	/// @param bufferId the buffer's id (zero based)
	/// @param dataType the datatype used to create the 
	///                 buffer if it doesn't exist already
	/// @return a pointer to the streamWriter
	///
	///////////////////////////////////////////////////////////
	ptr<streamWriter> getStreamWriter(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId, std::string dataType = "");

	//@}


	///////////////////////////////////////////////////////////
	/// \name Syncronize the charset tag (0008,0005)
	///
	///////////////////////////////////////////////////////////
	//@{
	
	/// \brief Collect all the charsets used in the dataSet's
	///         tags and then update the content of the tag
	///         0008,0005.
	///
	/// This function is called by the codecs before the dicom
	///  stream is saved, therefore the application doesn't
	///  need to call the function before saving the stream.
	///
	///////////////////////////////////////////////////////////
	void updateCharsetTag();

	/// \brief Update all the dataSet's tags with the charsets
	///         specified in the tag 0008,0005.
	///
	/// This function is called by the codecs after the stream
	///  has been loaded, therefore the application doesn't
	///  need to call the function after the stream has been
	///  loaded.
	///
	///////////////////////////////////////////////////////////
	void updateTagsCharset();

	//@}


	///////////////////////////////////////////////////////////
	/// \name Set/get the item offset.
	///
	///////////////////////////////////////////////////////////
	//@{
	
	/// \brief Called by codecs::dicomCodec when the dataset
	///         is written into a stream.
	///        Tells the dataSet the position at which it has
	///         been written into the stream
	///
	/// @param offset   the position at which the dataSet has
	///                  been written into the stream
	///
	///////////////////////////////////////////////////////////
	void setItemOffset(std::uint32_t offset);

	/// \brief Retrieve the offset at which the dataSet is
	///         located in the dicom stream.
	///
	/// @return the position at which the dataSet is located
	///          in the dicom stream
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getItemOffset();

	//@}

protected:
	// Convert an image using the attributes specified in the
	//  the dataset
	///////////////////////////////////////////////////////////
	ptr<image> convertImageForDataSet(ptr<image> sourceImage);

	std::vector<std::uint32_t> m_imagesPositions;

	// Position of the sequence item in the stream. Used to
	//  parse DICOMDIR items
	///////////////////////////////////////////////////////////
	std::uint32_t m_itemOffset;
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This is the base class for the exceptions thrown
///         by the dataSet.
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetException: public std::runtime_error
{
public:
	dataSetException(const std::string& message): std::runtime_error(message){}
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the application is
///         trying to store an image in the dataSet but
///         the dataSet already stores other images that
///         have different attributes.
///
/// The exception is usually thrown by dataSet::setImage().
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetExceptionDifferentFormat: public dataSetException
{
public:
	dataSetExceptionDifferentFormat(const std::string& message): dataSetException(message){}
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This exception is thrown when an unknown 
///         transfer syntax is being used while reading or
///         writing a stream.
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetExceptionUnknownTransferSyntax: public dataSetException
{
public:
	dataSetExceptionUnknownTransferSyntax(const std::string& message): dataSetException(message){}
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the application
///         is storing several images in the dataSet but
///         doesn't store them in the right order.
///
/// The application must store the images following the 
///  frame order, without skipping frames.
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetExceptionWrongFrame: public dataSetException
{
public:
	dataSetExceptionWrongFrame(const std::string& message): dataSetException(message){}
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the application
///         is trying to store an image in an old Dicom
///         format.
///
/// The application cannot store images in old Dicom 
///  formats (before Dicom3).
/// 
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetExceptionOldFormat: public dataSetException
{
public:
	dataSetExceptionOldFormat(const std::string& message): dataSetException(message){}
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the an image that
///         doesn't exist is requested.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetImageDoesntExist: public dataSetException
{
public:
	/// \brief Build a dataSetImageDoesntExist exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	dataSetImageDoesntExist(const std::string& message): dataSetException(message){}
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the basic offset
///         table is corrupted.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataSetCorruptedOffsetTable: public dataSetException
{
public:
	/// \brief Build a dataSetImageDoesntExist exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	dataSetCorruptedOffsetTable(const std::string& message): dataSetException(message){}
};

/// @}



} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraDataSet_93F684BF_0024_4bf3_89BA_D98E82A1F44C__INCLUDED_)
