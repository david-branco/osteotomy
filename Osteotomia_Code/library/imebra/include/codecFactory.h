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

/*! \file codecFactory.h
    \brief Declaration of the class used to retrieve the codec able to
	        handle the requested transfer syntax.

*/

#if !defined(imebraCodecFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraCodecFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include "../../base/include/baseObject.h"
#include "dataSet.h"



#include <list>


///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

	class streamReader;

namespace imebra
{

// Classes used in the declaration
class dataSet;

namespace codecs
{

/// \addtogroup group_codecs
///
/// @{

class codec;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class maintains a list of the available
///        codecs.
///        
/// It is used to retrieve the right codec when the 
///  transfer syntax is known, or to automatically select
///  the right codec that can parse the specified stream
///  of data.
///
/// An instance of this class is automatically allocated
///  by the library and can be retrieved using the
///  static function codecFactory::getCodecFactory().
///
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class codecFactory: public baseObject
{
protected:
    codecFactory();

public:

	/// \brief Register a new codec.
	///
	/// This function is called by the framework during the
	///  library's startup, in order to register all the imebra
	///  codecs.
	/// The codecs distributed with the imebra library are
	///  automatically registered.
	///
	/// @param pCodec a pointer to the codec to register
	///
	///////////////////////////////////////////////////////////
	void registerCodec(ptr<codec> pCodec);

	/// \brief Get a pointer to the codec that can handle
	///        the requested transfer syntax.
	///
	/// All the registered codecs are queried until the codec
	///  that can handle the specified transfer syntax replies.
	///
	/// A new instance of the requested codec is allocated and
	///  its pointer is returned.
	///
	/// @param transferSyntax a string with the requested
	///         transfer syntax.
	/// @return a pointer to a imebra codec that can handle the
	///        requested transfer syntax, or 0 if the function
	///         didn't find any valid codec.
	///        The returned pointer references a new instance
	///         of the codec, not the instance registered using
	///         registerCodec()
	///
	///////////////////////////////////////////////////////////
	static ptr<codec> getCodec(std::wstring transferSyntax);

	/// \brief Retrieve the only reference to the codecFactory
	///         instance.
	///
	/// An instance of codecFactory class is statically
	///  allocated by the imebra framework.
	///
	/// The applications should use only the statically 
	///  allocated instance of codecFactory.
	///
	/// @return a pointer to the only instance of the
	///          codecFactory class.
	///
	///////////////////////////////////////////////////////////
	static ptr<codecFactory> getCodecFactory();

	/// \brief Build a dataSet structure from the specified
	///         stream of data.
	///
	/// The function selects automatically the codec that can
	///  read the specified stream.
	///
	/// @param pStream the stream that contain the data to be
	///                 parsed
	/// @param maxSizeBufferLoad if a loaded buffer exceedes
	///                 the size in the parameter then it is
	///                 not loaded immediatly but it will be
	///                 loaded on demand. Some codecs may 
	///                 ignore this parameter.
	///                Set to 0xffffffff to load all the 
	///                 buffers immediatly
	/// @return a pointer to the dataSet containing the parsed
	///          data
	///
	///////////////////////////////////////////////////////////
	ptr<dataSet> load(ptr<streamReader> pStream, std::uint32_t maxSizeBufferLoad = 0xffffffff);

    /// \brief Set the maximum size of the images created by
    ///         the codec::getImage() function.
    ///
    /// @param maximumWidth   the maximum with of the images
    ///                        created by codec::getImage(), in
    ///                        pixels
    /// @param maximumHeight the maximum height of the images
    ///                        created by codec::getImage(), in
    ///                        pixels
    ///
    ///////////////////////////////////////////////////////////
    void setMaximumImageSize(const std::uint32_t maximumWidth, const std::uint32_t maximumHeight);

    /// \brief Get the maximum width of the images created
    ///         by codec::getImage()
    ///
    /// @return the maximum width, in pixels, of the images
    ///          created by codec::getImage()
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getMaximumImageWidth();

    /// \brief Get the maximum height of the images created
    ///         by codec::getImage()
    ///
    /// @return the maximum height, in pixels, of the images
    ///          created by codec::getImage()
    ///
    ///////////////////////////////////////////////////////////
    std::uint32_t getMaximumImageHeight();

protected:
	// The list of the registered codecs
	///////////////////////////////////////////////////////////
	std::list<ptr<codec> > m_codecsList;

    // Maximum allowed image size
    ///////////////////////////////////////////////////////////
    std::uint32_t m_maximumImageWidth;
    std::uint32_t m_maximumImageHeight;


public:
	// Force the creation of the codec factory before main()
	//  starts
	///////////////////////////////////////////////////////////
	class forceCodecFactoryCreation
	{
	public:
		forceCodecFactoryCreation()
		{
			codecFactory::getCodecFactory();
		}
	};
};

/// @}

} // namespace codecs

} // namespace imebra

} // namespace puntoexe


#endif // !defined(imebraCodecFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
