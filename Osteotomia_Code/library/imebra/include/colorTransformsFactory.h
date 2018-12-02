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

/*! \file colorTransformsFactory.h
    \brief Declaration of the class used to retrieve a color transform able to
	        handle the requested color spaces.

*/

#if !defined(imebraColorTransformsFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_)
#define imebraColorTransformsFactory_82307D4A_6490_4202_BF86_93399D32721E__INCLUDED_

#include <list>

#include "colorTransform.h"

namespace puntoexe
{

namespace imebra
{

namespace transforms
{

namespace colorTransforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class maintains a list of all the available
///         colorTransform classes and retrieve the
///         most appropriate transform class (or classes)
///         when a color space conversion is needed.
///
/// One instance of this class is statically allocated
///  by the library; the application does NOT have to
///  allocate its own instance of colorTransformsFactory.
///
/// A pointer to the statically allocated 
///  colorTransformsFactory class can be obtained by 
///  calling that static function 
///  colorTransformsFactory::getColorTransformsFactory().
///
/// The class can also retrieve more information
///  from a name of a color space (in dicom standard).
/// For instance, both the Dicom color space 
///  "YBR_FULL_422" and "YBR_FULL" describe the color
///  space YBR, but the first indicates that the image
///  is subsampled both horizontally and vertically.
///
/// The colorTransformsFactory can normalize the color
///  space name (e.g.: convert "YBR_FULL_422" to
///  "YBR_FULL") and can retrieve the subsampling
///  parameters.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class colorTransformsFactory: public baseObject
{
public:
	/// \internal
	/// \brief Register a color transform in the 
	///         colorTransformsFactory class.
	///
	/// @parameter newColorTransform the color transform to
	///                               be registered
	///
	///////////////////////////////////////////////////////////
	void registerTransform(ptr<colorTransform> newColorTransform);

	///////////////////////////////////////////////////////////
	/// \name Static instance
	///       
	/// Static functions that return a pointer to the
	///  statically allocated instance of 
	///  colorTransformsFactory
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Retrieve a pointer to the unique statically 
	///         allocated instance of the colorTransforsFactory
	///         class.
	///         
	/// The application must use the colorTransformsFactory
	///  referenced by this function.
	///
	/// @return a pointer to the unique instance of the
	///          colorTransformsFactory
	///
	///////////////////////////////////////////////////////////
	static ptr<colorTransformsFactory> getColorTransformsFactory();

	
	///////////////////////////////////////////////////////////
	/// \name Static functions
	///       
	/// Static functions that operate on the color space name
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Normalize a color space name.
	///
	/// The function converts all the chars to uppercase and
	///  remove additional information from the color space.
	///
	/// For instance, the color space "ybr_full_420" is
	///  converted to "YBR_FULL".
	///
	/// @param colorSpace the color space name to be normalized
	/// @return the normalized color space name
	///
	///////////////////////////////////////////////////////////
	static std::wstring normalizeColorSpace(std::wstring colorSpace);

	/// \brief Returns true if the color space name specified
	///         in the parameter has only one color channel and
	///         is monochrome (it doesn't have a lookup color
	///         table).
	///
	/// At the moment, only the color space names "MONOCHROME1" 
	///  and "MONOCHROME2" indicate a monochrome color space.
	///
	/// @param colorSpace the name of the color space to be
	///                    tested
	/// @return true if the color space indicated in the 
	///                    parameter is monochrome, or false
	///                    otherwise
	///
	///////////////////////////////////////////////////////////
	static bool isMonochrome(std::wstring colorSpace);

	/// \brief Returns true if the name of the color space
	///         specified in the parameter indicates that
	///         the chrominance channels are subsampled
	///         horizontally.
	///
	/// @param colorSpace the name of the color space to be
	///                    tested
	/// @return true if the name of the color space in the
	///                    parameter colorSpace has the
	///                    chrominance channels subsampled
	///                    horizontally
	///
	///////////////////////////////////////////////////////////
	static bool isSubsampledX(std::wstring colorSpace);

	/// \brief Returns true if the name of the color space
	///         specified in the parameter indicates that
	///         the chrominance channels are subsampled
	///         vertically.
	///
	/// @param colorSpace the name of the color space to be
	///                    tested
	/// @return true if the name of the color space in the
	///                    parameter colorSpace has the
	///                    chrominance channels subsampled
	///                    vertically
	///
	///////////////////////////////////////////////////////////
	static bool isSubsampledY(std::wstring colorSpace);

	/// \brief Returns true if the color space specified 
	///         in the parameter can be subsampled.
	///
	/// For instance, the color spaces "YBR_FULL" and 
	///  "YBR_PARTIAL" can be subsampled, but the color
	///  space "RGB" cannot be subsampled.
	///
	/// @param colorSpace the name of the color space to
	///                    be tested
	/// @return true if the name of the color space in the
	///                    parameter colorSpace can be
	///                    subsampled
	///
	///////////////////////////////////////////////////////////
	static bool canSubsample(std::wstring colorSpace);

	/// \brief Add the subsamplig information to a color space
	///         name.
	///
	/// Only the color spaces for which canSubsample() returns
	///  true can have the subsampling information.
	///
	/// @param colorSpace the name of the color space to which
	///                    the subsampling information should
	///                    be added
	/// @param bSubsampleX if true, then the function will make
	///                    the color space subsampled 
	///                    horizontally. The color space will
	///                    also be subsampled vertically
	/// @param bSubsampleY if true, then the function will make
	///                    the color space subsampled 
	///                    vertically
	/// @return the color space name subsampled as specified
	///
	///////////////////////////////////////////////////////////
	static std::wstring makeSubsampled(std::wstring colorSpace, bool bSubsampleX, bool bSubsampleY);

	/// \brief Returns the number of channels used by the
	///         specified color space.
	///
	/// For instance, the color space "RGB" has 3 color 
	///  channels, while the "MONOCHROME2" color space has
	///  1 color channel.
	///
	/// @param colorSpace the name of the color space for
	///                    which the number of channels
	///                    must be returned
	/// @return the number of color channels in the 
	///                    specified color channel
	///
	///////////////////////////////////////////////////////////
	static std::uint32_t getNumberOfChannels(std::wstring colorSpace);
	
	//@}
	

	///////////////////////////////////////////////////////////
	/// \name Color space conversion
	/// 
	/// Return the transform that convert one color space into
	///  another
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Returns a transform that can convert the
	///         pixels from one color space to another color
	///         space.
	///
	/// If no conversion is needed then the function
	///  returns 0.
	/// If the function cannot find any suitable transform 
	///  then a colorTransformsFactoryExceptionNoTransform
	///  is thrown.
	///
	/// @param startColorSpace the color space from which the
	///                         conversion has to take
	///                         place
	/// @param endColorSpace   the color space resulting 
	///                         from the conversion
	/// @return the transform that can convert the 
	///          startColorSpace into endColorSpace, or 0 if
	///          startColorSpace and endColorSpace have the
	///          same value
	///
	///////////////////////////////////////////////////////////
	ptr<colorTransform> getTransform(std::wstring startColorSpace, std::wstring endColorSpace);

	//@}

protected:
	typedef std::list<ptr<colorTransform> > tTransformsList;
	tTransformsList m_transformsList;

public:
	// Force the construction of the factory before main()
	//  starts
	///////////////////////////////////////////////////////////
	class forceColorTransformsFactoryConstruction
	{
	public:
		forceColorTransformsFactoryConstruction()
		{
			colorTransformsFactory::getColorTransformsFactory();
		}
	};


};

///////////////////////////////////////////////////////////
/// \brief This is the base class for the exceptions
///         thrown by colorTransformsFactory.
///
///////////////////////////////////////////////////////////
class colorTransformsFactoryException: public transformException
{
public:
	colorTransformsFactoryException(const std::string& message): transformException(message){}
};

///////////////////////////////////////////////////////////
/// \brief This exception is thrown by the function
///         colorTransformsFactory::getTransform() when
///         it cannot find any transform that can convert
///         the specified color spaces.
///
///////////////////////////////////////////////////////////
class colorTransformsFactoryExceptionNoTransform: public colorTransformsFactoryException
{
public:
	colorTransformsFactoryExceptionNoTransform(const std::string& message): colorTransformsFactoryException(message){}
};

/// @}

} // namespace colorTransforms

} // namespace transforms

} // namespace imebra

} // namespace puntoexe


#endif // !defined imebraColorTransformsFactory
