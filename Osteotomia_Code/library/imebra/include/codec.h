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

/*! \file codec.h
    \brief Declaration of the base class used by the codecs.

*/

#if !defined(imebraCodec_299706D7_4761_44a1_9F2D_8C38A7BD7AD5__INCLUDED_)
#define imebraCodec_299706D7_4761_44a1_9F2D_8C38A7BD7AD5__INCLUDED_

#include <stdexcept>

#include "../../base/include/baseObject.h"
#include "../../base/include/memory.h"

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


// Classes used in the declaration
class dataSet;
class image;

/// \namespace codecs
/// \brief This namespace is used to define the classes
///         that implement a codec and their helper
///         classes
///
///////////////////////////////////////////////////////////
namespace codecs
{

/// \addtogroup group_codecs Codecs
/// \brief The codecs can generate a dataSet structure
///         or an image from a stream or can write the
///         dataSet structure or an image into a stream.
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This is the base class for all the imebra 
///         codecs.
///
/// When the imebra codecs read the content of a file, they
///  don't return a decompressed image: instead they build
///  a DICOM structure in memory, with the image(s)
///  embedded in it.
///
/// A call to the dataSet::getImage() method will
///  return the decompressed image embedded into the dicom
///  structure.
///
/// E.G:
///  A call to jpegCodec::read() will build a
///   dataSet object with a jpeg image embedded in
///   it.
///  Your application should call 
///   dataSet::getImage()in order to get the 
///   decompressed image.
///
/// The same concept is used when your application must
///  generate a file with a compressed image in it.
/// Your application should:
///  - allocate a dicom structure (see dataSet)
///  - insert an image into the structure by calling
///     dataSet::setImage()
///  - generate the final file using the preferred codec.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class codec : public baseObject
{
public:
	///////////////////////////////////////////////////////////
	/// \name Streams read/write
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Read a stream and build an in-memory dicom 
	///        structure.
	///
	/// The specified stream is parsed and its content is
	///  saved into a dataSet object.
	///
	/// The type of codec to use depends on the stream's
	///  type (the jpeg codec jpegCodec will parse a
	///  jpeg stream, the DICOM codec dicomCodec will
	///  parse a DICOM stream).
	/// 
	/// If you want to autodetect the file type, just
	///  try to parse the stream with several codecs until
	///  one of them return a valid result.
	/// This task is easier if you use the class 
	///  codecFactory, which automatically scans all the
	///  imebra codecs until one can decompress the stream.
	///
	/// Each codec will rewind the stream's position if
	///  an error occurs.
	///
	/// If the codec parses the stream correctly (no error
	///  occurs) then the resulting dataSet object will
	///  contain a valid dicom structure.
	///
	/// @param pSourceStream Stream a pointer to the baseStream
	///                 object to parse.
	///                If the codec cannot parse the stream's
	///                 content, then the stream is rewinded to
	///                 its initial position.
	/// @param maxSizeBufferLoad if a loaded buffer exceedes
	///                 the size in the parameter then it is
	///                 not loaded immediatly but it will be
	///                 loaded on demand. Some codecs may 
	///                 ignore this parameter.
	///                Set to -1 to load all the buffers 
	///                 immediatly
	/// @return        a pointer to the loaded dataSet
	///
	///////////////////////////////////////////////////////////
	ptr<dataSet> read(ptr<streamReader> pSourceStream, std::uint32_t maxSizeBufferLoad = 0xffffffff);

	/// \brief Write a dicom structure into a stream.
	///
	/// The specified dataSet object is transformed into
	///  the desidered kind of stream (the jpeg codec
	///  jpegCodec will produce a jpeg stream, the DICOM
	///  codec dicomCodec will produce a DICOM stream).
	///
	/// @param pDestStream a pointer to the stream to use for
	///                     writing.
	/// @param pSourceDataSet a pointer to the Dicom structure 
	///                     to write into the stream
	///
	///////////////////////////////////////////////////////////
	void write(ptr<streamWriter> pDestStream, ptr<dataSet> pSourceDataSet);

	//@}


	///////////////////////////////////////////////////////////
	/// \name Set/get the image in the dicom structure
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Get a decompressed image from a dicom structure.
	///
	/// This function is usually called by 
	///  dataSet::getImage(), which also manages the 
	///  codec and the frame selection.
	///
	/// The decompressed image will be stored in a image
	///  object.
	/// Your application can choose the frame to decompress if
	///  a multiframe Dicom structure is available.
	///
	/// We suggest to use the dataSet::getImage()
	///  function instead of calling this function directly.
	/// dataSet::getImage() takes care of selecting
	///  the right tag's group and buffer's id. Infact,
	///  some dicom file formats span images in several groups,
	///  while others use sequence when saving multiple frames.
	///
	/// @param pSourceDataSet a pointer to the Dicom structure 
	///              where the requested image is embedded into
	/// @param pSourceStream a pointer to a stream containing
	///              the data to be parsed
	/// @param dataType the data type of the buffer from which
	///               the stream pSourceStream has been 
	///               obtained. The data type must be in DICOM
	///               format
	/// @return a pointer to the loaded image
	///
	///////////////////////////////////////////////////////////
	virtual ptr<image> getImage(ptr<dataSet> pSourceDataSet, ptr<streamReader> pSourceStream, std::string dataType) = 0;
	
	/// \brief This enumeration is used by setImage() in order
	///         to setup the compression parameters.
	///
	///////////////////////////////////////////////////////////
	enum quality
	{
		veryHigh = 0,      ///< the image is saved with very high quality. No subsampling is performed and no quantization
		high = 100,        ///< the image is saved with high quality. No subsampling is performed. Quantization ratios are low
		aboveMedium = 200, ///< the image is saved in medium quality. Horizontal subsampling is applied. Quantization ratios are low
		medium = 300,      ///< the image is saved in medium quality. Horizontal subsampling is applied. Quantization ratios are medium
		belowMedium = 400, ///< the image is saved in medium quality. Horizontal and vertical subsampling are applied. Quantization ratios are medium
		low = 500,         ///< the image is saved in low quality. Horizontal and vertical subsampling are applied. Quantization ratios are higher than the ratios used in the belowMedium quality
		veryLow = 600	   ///< the image is saved in low quality. Horizontal and vertical subsampling are applied. Quantization ratios are high
	};

	/// \brief Stores an image into stream.
	///
	/// The image is compressed using the specified transfer
	///  syntax and quality.
	///
	/// The application should call dataSet::setImage()
	///  instead of calling this function directly.
	///
	/// @param pDestStream the stream where the compressed 
	///                     image must be saved
	/// @param pSourceImage the image to be saved into the
	///                     stream
	/// @param transferSyntax the transfer syntax to use for
	///                     the compression
	/// @param imageQuality the quality to use for the 
	///                     compression. Please note that the
	///                     parameters bSubSampledX and
	///                     bSubSampledY override the settings
	///                     specified by this parameter
	/// @param dataType    the data type of the tag that will
	///                     contain the generated stream
	/// @param allocatedBits the number of bits per color
	///                     channel
	/// @param bSubSampledX true if the chrominance channels
	///                     must be subsampled horizontally,
	///                     false otherwise
	/// @param bSubSampledY true if the chrominance channels
	///                     must be subsampled vertically,
	///                     false otherwise
	/// @param bInterleaved true if the channels' information
	///                      must be interleaved, false if the
	///                      channels' information must be
	///                      flat (not interleaved)
	/// @param b2Complement true if the image contains 
	///                     2-complement data, false otherwise
	///
	///////////////////////////////////////////////////////////
	virtual void setImage(
		ptr<streamWriter> pDestStream,
		ptr<image> pSourceImage, 
		std::wstring transferSyntax, 
		quality imageQuality,
		std::string dataType,
		std::uint8_t allocatedBits,
		bool bSubSampledX,
		bool bSubSampledY,
		bool bInterleaved,
		bool b2Complement)=0;

	//@}


	///////////////////////////////////////////////////////////
	/// \name Selection of the codec from a transfer syntax
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Create another codec of the same type of the
	///         codec where the function is being called.
	///
	/// @return a pointer to a codec of the same type of the
	///          codec where the function is being called
	///
	///////////////////////////////////////////////////////////
	virtual ptr<codec> createCodec()=0;

	/// \brief This function returns true if the codec can
	///        handle the requested DICOM transfer syntax.
	///
	/// @param transferSyntax the transfer syntax to check
	///                         for
	/// @return true if the transfer syntax specified in
	///         transferSyntax can be handled by the
	///         codec, false otherwise.
	///
	///////////////////////////////////////////////////////////
	virtual bool canHandleTransferSyntax(std::wstring transferSyntax)=0;

	/// \brief This function returns true if the codec 
	///         transfer syntax handled by the code has to be
	///         encapsulated
	///
	/// @param transferSyntax the transfer syntax to check
	///                         for
	/// @return true if the transfer syntax specified in
	///         transferSyntax has to be encapsulated
	///
	///////////////////////////////////////////////////////////
	virtual bool encapsulated(std::wstring transferSyntax)=0;

	//@}


	///////////////////////////////////////////////////////////
	/// \name Image's attributes from the transfer syntax.
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief This function returns the highest bit per
	///        channel supported by the specified transfer
	///        syntax.
	///
	/// @param transferSyntax the transfer syntax for which
	///         the information is requested.
	/// @return the highest bit supported by the transfer
	///         syntax
	///
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getMaxHighBit(std::string transferSyntax)=0;

	/// \brief Suggest an optimal number of allocated bits for
	///        the specified transfer syntax and high bit.
	///
	/// @param transferSyntax the transfer syntax to use
	/// @param highBit        the high bit to use for the
	///                        suggestion
	/// @return the suggested number of allocated bits for the
	///          specified transfer syntax and high bit.
	///
	///////////////////////////////////////////////////////////
	virtual std::uint32_t suggestAllocatedBits(std::wstring transferSyntax, std::uint32_t highBit)=0;

	//@}


protected:
	virtual void readStream(ptr<streamReader> pInputStream, ptr<dataSet> pDestDataSet, std::uint32_t maxSizeBufferLoad = 0xffffffff) =0;
	virtual void writeStream(ptr<streamWriter> pDestStream, ptr<dataSet> pSourceDataSet) =0;
};


class channel : public baseObject
{
public:
	// Constructor
	///////////////////////////////////////////////////////////
	channel(): 
		m_samplingFactorX(1), 
		m_samplingFactorY(1),
		m_sizeX(0),
		m_sizeY(0),
		m_pBuffer(0),
		m_bufferSize(0){}

	// Allocate the channel
	///////////////////////////////////////////////////////////
	void allocate(std::uint32_t sizeX, std::uint32_t sizeY);

	// Sampling factor
	///////////////////////////////////////////////////////////
	std::uint32_t m_samplingFactorX;
	std::uint32_t m_samplingFactorY;

	// Channel's size in pixels
	///////////////////////////////////////////////////////////
	std::uint32_t m_sizeX;
	std::uint32_t m_sizeY;

	// Channel's buffer & size
	///////////////////////////////////////////////////////////
	std::int32_t* m_pBuffer;
	std::uint32_t m_bufferSize;

	ptr<memory> m_memory;
};


///////////////////////////////////////////////////////////
/// \brief This is the base class for the exceptions thrown
///         by the codec derived classes.
///
///////////////////////////////////////////////////////////
class codecException: public std::runtime_error
{
public:
	/// \brief Build a codec exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	codecException(const std::string& message): std::runtime_error(message){}
};


///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the file being
///         parsed is not valid for the codec.
///
///////////////////////////////////////////////////////////
class codecExceptionWrongFormat: public codecException
{
public:
	/// \brief Build a codecExceptionWrongFormat exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	codecExceptionWrongFormat(const std::string& message): codecException(message){}
};


///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the file being
///         parsed is corrupted.
///
///////////////////////////////////////////////////////////
class codecExceptionCorruptedFile: public codecException
{
public:
	/// \brief Build a codecExceptionCorruptedFile exception
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	codecExceptionCorruptedFile(const std::string& message): codecException(message){}
};


///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the transfer
///         syntax is not recognized by the codec.
///
///////////////////////////////////////////////////////////
class codecExceptionWrongTransferSyntax: public codecException
{
public:
	/// \brief Build a codecExceptionWrongTransferSyntax
    ///         exception.
	///
	/// @param message the message to store into the exception
	///
	///////////////////////////////////////////////////////////
	codecExceptionWrongTransferSyntax(const std::string& message): codecException(message){}
};


///////////////////////////////////////////////////////////
/// \brief This exception is thrown when the codec reads
///         a file that contains an image that is too big
///         according to the codecFactory settings.
///
/// Consider using codecFactory::setMaximumImageSize()
///  to change the allowed maximum image size.
///
///////////////////////////////////////////////////////////
class codecExceptionImageTooBig: public codecException
{
public:
    /// \brief Build a codecExceptionImageTooBig
    ///         exception.
    ///
    /// @param message the message to store into the exception
    ///
    ///////////////////////////////////////////////////////////
    codecExceptionImageTooBig(const std::string& message): codecException(message){}
};


/// @}

} // namespace codecs

} // namespace imebra

} // namespace puntoexe


#endif // !defined(imebraCodec_299706D7_4761_44a1_9F2D_8C38A7BD7AD5__INCLUDED_)
