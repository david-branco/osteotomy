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

/*! \file transform.h
    \brief Declaration of the base class used by all the transforms.
 
*/

#if !defined(imebraTransform_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_)
#define imebraTransform_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_

#include "../../base/include/baseObject.h"
#include "dataHandlerNumeric.h"
#include "image.h"


#define DEFINE_RUN_TEMPLATE_TRANSFORM \
template <typename inputType>\
void runTemplateTransform1(\
    inputType* inputData, size_t inputDataSize, std::uint32_t inputHandlerWidth, const std::wstring& inputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> inputPalette,\
    std::int32_t inputHandlerMinValue, std::uint32_t inputHighBit,\
    std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight,\
    puntoexe::ptr<puntoexe::imebra::handlers::dataHandlerNumericBase> outputHandler, std::int32_t outputHandlerWidth, const std::wstring& outputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> outputPalette,\
    std::int32_t outputHandlerMinValue, std::uint32_t outputHighBit,\
    std::int32_t outputTopLeftX, std::int32_t outputTopLeftY)\
{\
        HANDLER_CALL_TEMPLATE_FUNCTION_WITH_PARAMS(runTemplateTransform2, outputHandler, \
                        inputData, inputDataSize, inputHandlerWidth, inputHandlerColorSpace,\
			inputPalette,\
                        inputHandlerMinValue, inputHighBit,\
			inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,\
			outputHandlerWidth, outputHandlerColorSpace,\
                        outputPalette,\
                        outputHandlerMinValue, outputHighBit,\
			outputTopLeftX, outputTopLeftY);\
}\
\
void runTemplateTransform(\
    puntoexe::ptr<puntoexe::imebra::handlers::dataHandlerNumericBase> inputHandler, std::uint32_t inputHandlerWidth, const std::wstring& inputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> inputPalette,\
    std::int32_t inputHandlerMinValue, std::uint32_t inputHighBit,\
    std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight,\
    puntoexe::ptr<puntoexe::imebra::handlers::dataHandlerNumericBase> outputHandler, std::int32_t outputHandlerWidth, const std::wstring& outputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> outputPalette,\
    std::int32_t outputHandlerMinValue, std::uint32_t outputHighBit,\
    std::int32_t outputTopLeftX, std::int32_t outputTopLeftY)\
{\
        HANDLER_CALL_TEMPLATE_FUNCTION_WITH_PARAMS(runTemplateTransform1, inputHandler, \
                        inputHandlerWidth, inputHandlerColorSpace,\
                        inputPalette,\
            inputHandlerMinValue, inputHighBit,\
			inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,\
			outputHandler, outputHandlerWidth, outputHandlerColorSpace,\
                        outputPalette,\
                        outputHandlerMinValue, outputHighBit,\
			outputTopLeftX, outputTopLeftY);\
}\
\
template <typename outputType, typename inputType>\
void runTemplateTransform2(\
    outputType* outputData, size_t outputDataSize, \
    inputType* inputData, size_t inputDataSize, \
    std::uint32_t inputHandlerWidth, const std::wstring& inputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> inputPalette,\
    std::int32_t inputHandlerMinValue, std::uint32_t inputHighBit,\
    std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight,\
    std::int32_t outputHandlerWidth, const std::wstring& outputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> outputPalette,\
    std::int32_t outputHandlerMinValue, std::uint32_t outputHighBit,\
    std::int32_t outputTopLeftX, std::int32_t outputTopLeftY)\
{\
        templateTransform( \
            inputData, inputDataSize, inputHandlerWidth, inputHandlerColorSpace, \
            inputPalette, \
            inputHandlerMinValue, inputHighBit, \
            inputTopLeftX, inputTopLeftY, inputWidth, inputHeight, \
            outputData, outputDataSize, outputHandlerWidth, outputHandlerColorSpace, \
            outputPalette, \
            outputHandlerMinValue, outputHighBit, \
            outputTopLeftX, outputTopLeftY);\
}\
\
virtual void runTransformHandlers(\
    puntoexe::ptr<puntoexe::imebra::handlers::dataHandlerNumericBase> inputHandler, std::uint32_t inputHandlerWidth, const std::wstring& inputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> inputPalette,\
    std::int32_t inputHandlerMinValue, std::uint32_t inputHighBit,\
    std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight,\
    puntoexe::ptr<puntoexe::imebra::handlers::dataHandlerNumericBase> outputHandler, std::int32_t outputHandlerWidth, const std::wstring& outputHandlerColorSpace,\
    puntoexe::ptr<puntoexe::imebra::palette> outputPalette,\
    std::int32_t outputHandlerMinValue, std::uint32_t outputHighBit,\
    std::int32_t outputTopLeftX, std::int32_t outputTopLeftY)\
{\
    runTemplateTransform(inputHandler, inputHandlerWidth, inputHandlerColorSpace, inputPalette, inputHandlerMinValue, inputHighBit,\
            inputTopLeftX, inputTopLeftY, inputWidth, inputHeight,\
            outputHandler, outputHandlerWidth, outputHandlerColorSpace, outputPalette, outputHandlerMinValue, outputHighBit,\
            outputTopLeftX, outputTopLeftY);\
}

///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

namespace handlers
{
    class dataHandlerNumericBase;
}

class image;
class dataSet;
class lut;

/// \namespace transforms
/// \brief All the transforms are declared in this
///         namespace.
///
///////////////////////////////////////////////////////////
namespace transforms
{

/*! \addtogroup group_transforms Transforms
\brief The transform classes apply a transformation to
		one input image and return the result of the
		transformation into an output image.

Usually the transforms require that the input and the
 output images use the same color space, but the
 color transforms are able to copy the pixel data
 from the color space of the input image into the
 color space of the output image.

The application can call
 transforms::transform::allocateOutputImage() to
 allocate an output image that is compatible with the
 selected transform and input image.\n
For instance, once an image has been retrieved from
 a dataSet we can ask the modalityVOILUT transform
 to allocate an output image for us, and it will
 allocate an image with the right color space and
 bit depth;

\code
// loadedDataSet is a ptr<dataSet> previously loaded
// Here we get the first image in the dataSet
ptr<image> inputImage(loadedDataSet->getImage(0));

// We need to get the image's size because we have to
//  tell the transform on which area we want to apply
//  the transform (we want all the image area)
std::uint32_t width, height;
inputImage->getSize(&width, &height);

// Allocate the modality transform. The modality transform
//  gets the transformation parameters from the dataset
ptr<transforms::modalityVOILUT> modalityTransform(new transforms::modalityVOILUT(loadedDataSet));

// We ask the transform to allocate a proper output image
ptr<image> outputImage(modalityTransform->allocateOutputImage(inputImage, width, height));

// And now we run the transform
modalityTransform->runTransform(inputImage, 0, 0, width, height, outputImage, 0, 0);
\endcode

All the transforms but the modalityVOILUT can convert
 the result to the bit depth of the output image, so for
 instance the transform colorTransforms::YBRFULLToRGB
 can take a 16 bits per channel input image and
 write the result to a 8 bits per color channel output
 image.\n
modalityVOILUT cannot do this because its output has
 to conform to the value in the tag 0028,1054; the
 tag 0028,1054 specifies the units of the modality VOI-LUT
 transform. modalityVOILUT::allocateOutputImage() is able
 output image that can hold the result of the
 to allocate the modality transformation.

*/
/// @{

/// \brief This is the base class for the transforms.
///
/// A transform takes one input and one output image:
///  the output image is modified according to the
///  transform's type, input image's content and
///  transform's parameter.
///
///////////////////////////////////////////////////////////
class transform : public baseObject
{

public:
	/// \brief Returns true if the transform doesn't do
	///         anything.
	///
	/// @return false if the transform does something, or true
	///          if the transform doesn't do anything (e.g. an
	///          empty transformsChain object).
	///
	///////////////////////////////////////////////////////////
	virtual bool isEmpty();


	/// \brief Allocate an output image that is compatible with
	///         the transform given the specified input image.
	///
	/// @param pInputImage image that will be used as input
	///                     image in runTransform()
	/// @param width       the width of the output image,
	///                     in pixels
	/// @param height      the height of the output image,
	///                     in pixels
	/// @return an image suitable to be used as output image
	///          in runTransform()
	///
	///////////////////////////////////////////////////////////
	virtual ptr<image> allocateOutputImage(ptr<image> pInputImage, std::uint32_t width, std::uint32_t height) = 0;

	/// \brief Executes the transform.
	///
	/// @param inputImage    the input image for the transform
	/// @param inputTopLeftX the horizontal position of the
	///                       top left corner of the area to
	///                       process
	/// @param inputTopLeftY the vertical position of the top
	///                       left corner of the area to
	///                       process
	/// @param inputWidth    the width of the area to process
	/// @param inputHeight   the height of the area to process
	/// @param outputImage   the output image for the transform
	/// @param outputTopLeftX the horizontal position of the
	///                       top left corner of the output
	///                       area
	/// @param outputTopLeftY the vertical position of the top
	///                        left corner of the output area
	///
	///////////////////////////////////////////////////////////
	virtual void runTransform(
            const ptr<image>& inputImage,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            const ptr<image>& outputImage,
			std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) = 0;

};


/// \brief This is the base class for transforms that use
///         templates.
///
/// Transforms derived from transformHandlers
///  have the macro DEFINE_RUN_TEMPLATE_TRANSFORM in
///  their class definition and implement the template
///  function templateTransform().
///
///////////////////////////////////////////////////////////
class transformHandlers: public transform
{
public:
	/// \brief Reimplemented from transform: calls the
	///         templated function templateTransform().
	///
	/// @param inputImage    the input image for the transform
	/// @param inputTopLeftX the horizontal position of the
	///                       top left corner of the area to
	///                       process
	/// @param inputTopLeftY the vertical position of the top
	///                       left corner of the area to
	///                       process
	/// @param inputWidth    the width of the area to process
	/// @param inputHeight   the height of the area to process
	/// @param outputImage   the output image for the transform
	/// @param outputTopLeftX the horizontal position of the
	///                       top left corner of the output
	///                       area
	/// @param outputTopLeftY the vertical position of the top
	///                        left corner of the output area
	///
	///////////////////////////////////////////////////////////
	virtual void runTransform(
			const ptr<image>& inputImage,
			std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
			const ptr<image>& outputImage,
			std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY);

	/// \internal
	virtual void runTransformHandlers(
			ptr<handlers::dataHandlerNumericBase> inputHandler, std::uint32_t inputHandlerWidth, const std::wstring& inputHandlerColorSpace,
			ptr<palette> inputPalette,
            std::int32_t inputHandlerMinValue, std::uint32_t inputHighBit,
			std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight,
			ptr<handlers::dataHandlerNumericBase> outputHandler, std::int32_t outputHandlerWidth, const std::wstring& outputHandlerColorSpace,
			ptr<palette> outputPalette,
            std::int32_t outputHandlerMinValue, std::uint32_t outputHighBit,
			std::int32_t outputTopLeftX, std::int32_t outputTopLeftY) = 0;

};


/// \brief Base class for the exceptions thrown by the
///         transforms.
///
///////////////////////////////////////////////////////////
class transformException: public std::runtime_error
{
public:
	/// \brief Constructor.
	///
	/// @param message the cause of the exception
	///
	///////////////////////////////////////////////////////////
	transformException(const std::string& message): std::runtime_error(message){}
};

/// \brief Exception thrown when the image areas to be
///         processed are out of bounds.
///
///////////////////////////////////////////////////////////
class transformExceptionInvalidArea: public transformException
{
public:
    /// \brief Constructor.
    ///
    /// @param message the cause of the exception
    ///
    ///////////////////////////////////////////////////////////
    transformExceptionInvalidArea(const std::string& message): transformException(message){}
};


/// @}

} // namespace transforms

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraTransform_5DB89BFD_F105_45e7_B9D9_3756AC93C821__INCLUDED_)
