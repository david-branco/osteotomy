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

/*! \file VOILUT.h
    \brief Declaration of the class VOILUT.

*/

#if !defined(imebraVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
#define imebraVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_

#include "image.h"
#include "LUT.h"
#include "transform.h"



///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

class lut;

namespace transforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
/// \brief Transforms the value of the input image's pixels
///         by using the presentation VOI/LUT defined in
///         the dataSet.
///
/// The dataSet could define more than one VOI/LUT: by
///  default the transform uses the first VOI or LUT
///  defined in the dataset.
///
/// To control which VOI/LUT is used for the
///  transformation the application must use the functions
///  getVOILUTId(), getVOILUTDescription(), setVOILUT() or
///  set the VOI or the LUT directly by calling
///  setCenterWidth() or setLUT().
///
///////////////////////////////////////////////////////////
class VOILUT: public transforms::transformHandlers
{
public:
    /// \brief Constructor.
    ///
    /// @param pDataSet the dataset from which the input image
    ///        comes from
    ///
    ///////////////////////////////////////////////////////////
	VOILUT(ptr<dataSet> pDataSet): m_pDataSet(pDataSet), m_windowCenter(0), m_windowWidth(0){}

	/// \brief Retrieve an ID for a VOI or a LUT.
	///
	/// The returned ID can be used with the functions
	///  getVOILUTDescription() and setVOILUT().
	///
	/// The function returns 0 when the requested VOI/LUT
	///  doesn't exist.
	///
	/// The parameter VOILUTNumber is a zero based index used
	///  to scan all the available VOIs first and then all the
	///  LUTs.
	/// For instance, if VOILUTNumber is 3 and the dataSet
	///  contains 2 VOIs and 3 LUTs, then the function will
	///  return the ID for the second LUT.
	///
	/// @param VOILUTNumber  a number that identifies the
	///                       VOI or the LUT for which the
	///                       ID is requested.
	///                      The value 0 refers to the first
	///                       VOI in the dataSet or to the
	///                       first LUT if there isn't any
	///                       defined VOI. Bigger values refer
	///                       to the following VOIs or LUTs
	///                       when all the VOIs have been
	///                       scanned
	/// @return an ID that can be used with
	///          getVOILUTDescription() and setVOILUT(), or 0
	///                       if the requested VOI/LUT doesn't
	///                       exist
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getVOILUTId(std::uint32_t VOILUTNumber);

    /// \brief List of ids of the VOIs and LUTs defined in
    ///        the dataset.
    ///
    /// The list is returned by getVOILUTIds() and each id can
    ///  be used with getVOILUTDescription() and setVOILUT().
    ///
    ///////////////////////////////////////////////////////////
    typedef std::list<std::uint32_t> voilutIds_t;

    /// \brief Return a list of ids of the VOIs and LUTs
    ///        defined in the database.
    ///
    /// @return a list of ids of the available VOI/LUTs
    ///
    ///////////////////////////////////////////////////////////
    voilutIds_t getVOILUTIds();

	/// \brief Return a description for the VOI or LUT with
	///         the specified ID.
	///
	/// The VOI/LUT ID can be obtained by calling
	///  getVOILUTId().
	///
	/// @param VOILUTId the id of the VOI/LUT for which the
	///                  description is required
	/// @return         the VOI/LUT description
	///
	///////////////////////////////////////////////////////////
	std::wstring getVOILUTDescription(std::uint32_t VOILUTId);

	/// \brief Define the VOI/LUT to use for the
	///         transformation.
	///
	/// The VOI/LUT ID can be obtained by calling
	///  getVOILUTId().
	///
	/// Disable the VOI/LUT transform if the parameter is 0.
	///
	/// @param VOILUTId the ID of the VOI/LUT to use for the
	///                  transformation, or 0 to disable the
	///                  transformation
	///
	///////////////////////////////////////////////////////////
	void setVOILUT(std::uint32_t VOILUTId);

	/// \brief Define the LUT to use for the transformation.
	///
	/// @param pLut the lut that will be used for the
	///              transformation
	///
	///////////////////////////////////////////////////////////
	void setLUT(ptr<lut> pLut);

	/// \brief Define the VOI width/center to use for the
	///         transformation.
	///
	/// @param center   the center value of the VOI
	/// @param width    the width value of the VOI
	///
	///////////////////////////////////////////////////////////
	void setCenterWidth(std::int32_t center, std::int32_t width);

	/// \brief Returns the VOI width/center used for the
	///         transformation.
	///
	/// @param pCenter  pointer to the recipient for the VOI
	///                  center
	/// @param pWidth    pointer to the recipient for the VOI
	///                  width
	///
	///////////////////////////////////////////////////////////
	void getCenterWidth(std::int32_t* pCenter, std::int32_t* pWidth);

    /// \brief Finds and apply the optimal VOI values.
    ///
    /// @param inputImage    the image for which the optimal
    ///                      VOI must be found
    /// @param inputTopLeftX the horizontal coordinate of the
    ///                       top-left corner of the area for
    ///                       which the optimal VOI must be
    ///                       found
    /// @param inputTopLeftY the vertical coordinate of the
    ///                       top-left corner of the area for
    ///                       which the optimal VOI must be
    ///                       found
    /// @param inputWidth    the width of the area for which
    ///                       the optimal VOI must be found
    /// @param inputHeight   the height of the area for which
    ///                       the optimal VOI must be found
    ///
    ///////////////////////////////////////////////////////////
    void applyOptimalVOI(const ptr<puntoexe::imebra::image>& inputImage, std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight);


	DEFINE_RUN_TEMPLATE_TRANSFORM;

    // The actual transformation is done here
    //
    ///////////////////////////////////////////////////////////
	template <class inputType, class outputType>
			void templateTransform(
					inputType* inputHandlerData, size_t /* inputHandlerSize */, std::uint32_t inputHandlerWidth, const std::wstring& /* inputHandlerColorSpace */,
					ptr<palette> /* inputPalette */,
                    std::int32_t inputHandlerMinValue, std::uint32_t inputHighBit,
					std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight,
					outputType* outputHandlerData, size_t /* outputHandlerSize */, std::int32_t outputHandlerWidth, const std::wstring& /* outputHandlerColorSpace */,
					ptr<palette> /* outputPalette */,
                    std::int32_t outputHandlerMinValue, std::uint32_t outputHighBit,
					std::int32_t outputTopLeftX, std::int32_t outputTopLeftY)

	{
		inputType* pInputMemory(inputHandlerData);
		outputType* pOutputMemory(outputHandlerData);

		pInputMemory += inputTopLeftY * inputHandlerWidth + inputTopLeftX;
		pOutputMemory += outputTopLeftY * outputHandlerWidth + outputTopLeftX;

		//
		// LUT found
		//
		///////////////////////////////////////////////////////////
		if(m_pLUT != 0 && m_pLUT->getSize() != 0)
		{
			lut* pLUT = m_pLUT.get();
            inputHighBit = pLUT->getBits();
			inputHandlerMinValue = 0;

            if(inputHighBit > outputHighBit)
			{
                std::int32_t rightShift = inputHighBit - outputHighBit;
                for(; inputHeight != 0; --inputHeight)
                {
                    for(int scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                    {
                        *(pOutputMemory++) = (outputType)( outputHandlerMinValue + (pLUT->mappedValue((std::int32_t) (*(pInputMemory++))) >> rightShift ));
                    }
                    pInputMemory += (inputHandlerWidth - inputWidth);
                    pOutputMemory += (outputHandlerWidth - inputWidth);
                }
            }
			else
			{
                std::int32_t leftShift = outputHighBit - inputHighBit;
				for(; inputHeight != 0; --inputHeight)
				{
					for(int scanPixels(inputWidth); scanPixels != 0; --scanPixels)
					{
                        *(pOutputMemory++) = (outputType)( outputHandlerMinValue + (pLUT->mappedValue((std::int32_t) (*(pInputMemory++))) << leftShift ));
					}
					pInputMemory += (inputHandlerWidth - inputWidth);
					pOutputMemory += (outputHandlerWidth - inputWidth);
				}
			}

			return;
		}

		//
		// LUT not found.
		// Use the window's center/width
		//
        ///////////////////////////////////////////////////////////
        std::int64_t inputHandlerNumValues = (std::int64_t)1 << (inputHighBit + 1);
        std::int64_t outputHandlerNumValues = (std::int64_t)1 << (outputHighBit + 1);
        std::int64_t minValue = (std::int64_t)m_windowCenter- (std::int64_t)(m_windowWidth/2);
        std::int64_t maxValue = (std::int64_t)m_windowCenter+ (std::int64_t)(m_windowWidth/2);
		if(m_windowWidth <= 1)
		{
			minValue = inputHandlerMinValue ;
			maxValue = inputHandlerMinValue + inputHandlerNumValues;
		}
		else
		{
			inputHandlerNumValues = maxValue - minValue;
		}


        std::int64_t value;

        if(inputHandlerNumValues > outputHandlerNumValues)
        {
            std::int32_t ratio = (std::int32_t)(inputHandlerNumValues / outputHandlerNumValues);
            for(; inputHeight != 0; --inputHeight)
            {

                for(int scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                {
                    value = (std::int64_t) *(pInputMemory++);
                    if(value <= minValue)
                    {
                        *(pOutputMemory++) = (outputType)outputHandlerMinValue;
                        continue;
                    }
                    if(value >= maxValue)
                    {
                        *(pOutputMemory++) = (outputType)( outputHandlerMinValue + outputHandlerNumValues - 1 );
                        continue;
                    }
                    *(pOutputMemory++) = (outputType)( (value - minValue) / ratio + outputHandlerMinValue );
                }
                pInputMemory += (inputHandlerWidth - inputWidth);
                pOutputMemory += (outputHandlerWidth - inputWidth);
            }
        }
        else
        {
            std::int32_t ratio = (std::int32_t)(outputHandlerNumValues / inputHandlerNumValues);
            for(; inputHeight != 0; --inputHeight)
            {

                for(int scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                {
                    value = (std::int64_t) *(pInputMemory++);
                    if(value <= minValue)
                    {
                        *(pOutputMemory++) = (outputType)outputHandlerMinValue;
                        continue;
                    }
                    if(value >= maxValue)
                    {
                        *(pOutputMemory++) = (outputType)( outputHandlerMinValue + outputHandlerNumValues - 1 );
                        continue;
                    }
                    *(pOutputMemory++) = (outputType)( (value - minValue) * ratio + outputHandlerMinValue );
                }
                pInputMemory += (inputHandlerWidth - inputWidth);
                pOutputMemory += (outputHandlerWidth - inputWidth);
            }
        }
	}


	virtual bool isEmpty();

	virtual ptr<image> allocateOutputImage(ptr<image> pInputImage, std::uint32_t width, std::uint32_t height);

protected:

    // Find the optimal VOI
    //
    ///////////////////////////////////////////////////////////
    template <class inputType>
            void templateFindOptimalVOI(
                    inputType* inputHandlerData, size_t /* inputHandlerSize */, std::uint32_t inputHandlerWidth,
                    std::int32_t inputTopLeftX, std::int32_t inputTopLeftY, std::int32_t inputWidth, std::int32_t inputHeight)
    {
        inputType* pInputMemory(inputHandlerData + inputHandlerWidth * inputTopLeftY + inputTopLeftX);
        std::int32_t minValue(*pInputMemory);
        std::int32_t maxValue(minValue);
        std::int32_t value;
        for(std::int32_t scanY(inputHeight); scanY != 0; --scanY)
        {
            for(std::int32_t scanX(inputWidth); scanX != 0; --scanX)
            {
                value = *(pInputMemory++);
                if(value < minValue)
                {
                    minValue = value;
                }
                else if(value > maxValue)
                {
                    maxValue = value;
                }
            }
            pInputMemory += inputHandlerWidth - inputWidth;
        }
        std::int32_t center = (maxValue - minValue) / 2 + minValue;
        std::int32_t width = maxValue - minValue;
        setCenterWidth(center, width);
    }

    ptr<dataSet> m_pDataSet;
	ptr<lut> m_pLUT;
	std::int32_t m_windowCenter;
	std::int32_t m_windowWidth;
};

/// @}

} // namespace transforms

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
