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

/*! \file drawBitmap.h
    \brief Declaration of the a class that draw an image into a bitmap.

This file is not included automatically by imebra.h

*/

#if !defined(imebraDrawBitmap_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraDrawBitmap_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "../../base/include/memory.h"
#include "transformsChain.h"
#include <memory>
#include <string.h>

namespace puntoexe
{

	namespace imebra
	{

		/// \addtogroup group_helpers Helpers
		///
		/// @{

		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		/// \brief Base class used for the exceptions thrown by
		///         drawBitmap.
		///
		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		class drawBitmapException: public std::runtime_error
		{
		public:
			drawBitmapException(const std::string& message): std::runtime_error(message){}
		};

		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		/// \brief This exception is thrown by
		///         declareBitmapType() if the image's area that
		///         has to be generated is not valid.
		///
		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		class drawBitmapExceptionInvalidArea: public drawBitmapException
		{
		public:
			drawBitmapExceptionInvalidArea(const std::string& message): drawBitmapException(message){}
		};


        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////
        /// \brief Defines the output type of
        ///         getBitmap().
        ///
        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////
        enum tDrawBitmapType
		{
            drawBitmapRGB  = 0, ///< Generates a BMP image where each pixel contains 3 bytes (R, G and B)
            drawBitmapBGR  = 1, ///< Generates a BMP image where each pixel contains 3 bytes (B, G and R)
            drawBitmapRGBA = 2, ///< Generates a BMP image where each pixel contains 4 bytes (R, G, B and A)
            drawBitmapBGRA = 3  ///< Generates a BMP image where each pixel contains 4 bytes (B, G, R and A)
		};

		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		/// \brief This class takes an image as an input and
		///         returns an 8 bit RGB bitmap of the requested
		///         image's area.
		///
		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
        class drawBitmap: public baseObject
		{
		public:
			/// \brief Constructor.
			///
			/// @param sourceImage  the input image that has to be
			///                      rendered
			/// @param transformsChain the list of transforms to be
			///                      applied to the image before it
			///                      is rendered. Can be null.
			///                     The transformation to RGB and
			///                      high bit shift are applied
			///                      automatically by this class
			///
			///////////////////////////////////////////////////////////
			drawBitmap(ptr<image> sourceImage, ptr<transforms::transformsChain> transformsChain);

			/// \brief Renders the image specified in the constructor
			///         into an RGB or BGR buffer.
			///
			/// The caller can pass to the function a memory object
			///  that will be used to store the RGB/BGR buffer,
			///  otherwise the function will allocate a new one.
			///
			/// Each row of pixels in the RGB/BGR buffer will be
			///  aligned according to the template parameter
			///  rowAlignBytes
			///
			/// \image html drawbitmap.png "drawbitmap"
			/// \image latex drawbitmap.png "drawbitmap" width=15cm
			///
			/// The figure illustrates how the getBitmap() method
			///  works:
			/// -# the image is resized according to the parameters
			///        totalWidthPixels and totalHeightPixels
			/// -# the area specified by visibleTopLeftX,
			///    visibleTopLeftY - visibleBottomRightX,
			///    visibleBottomRightY is rendered into the buffer
			///
			/// Please note that the rendering algorithm achieves the
			///  described results without actually resizing the image.
			///
			/// @tparam drawBitmapType The RGB order. Must be
			///                         drawBitmapBGR for BMP images
			/// @tparam rowAlignBytes  the boundary alignment of each
			///                         row. Must be 4 for BMP images
			/// @param totalWidthPixels the width of the magnified or
			///                          shrunken image in pixels
			///                          (magnified width on the
			///                          figure "drawbitmap")
			/// @param totalHeightPixels the height of the magnified
			///                           or shrunken image in pixels
			///                           (magnified height on the
			///                           figure "drawbitmap")
			/// @param visibleTopLeftX  the X coordinate of the top
			///                          left corner of image area that
			///                          has to be rendered
			///                          (visible top left magnified X
			///                           on the figure "drawbitmap")
			/// @param visibleTopLeftY  the Y coordinate of the top
			///                          left corner of image area that
			///                          has to be rendered
			///                          (visible top left magnified Y
			///                           on the figure "drawbitmap")
			/// @param visibleBottomRightX the X coordinate of the
			///                          bottom right corner of image
			///                          area that has to be rendered
			///                          (visible bottom right
			///                           magnified X on the figure
			///                           "drawbitmap")
			/// @param visibleBottomRightY the Y coordinate of the
			///                          bottom right corner of image
			///                          area that has to be rendered
			///                          (visible bottom right
			///                           magnified Y on the figure
			///                           "drawbitmap")
			/// @param reuseMemory      a pointer to a memory object
			///                          that must be used to store the
			///                          output buffer. Can be null
			/// @return the memory object in which the output buffer
			///          is stored. Is the same object specified in
			///          reuseMemory or a new object if reuseMemory
			///          is null
			///
			///////////////////////////////////////////////////////////
			template <tDrawBitmapType drawBitmapType, int rowAlignBytes>
					ptr<memory> getBitmap(std::int32_t totalWidthPixels, std::int32_t totalHeightPixels,
										  std::int32_t visibleTopLeftX, std::int32_t visibleTopLeftY, std::int32_t visibleBottomRightX, std::int32_t visibleBottomRightY,
										  ptr<memory> reuseMemory)
			{
				PUNTOEXE_FUNCTION_START(L"drawBitmap::getBitmap");

				// Just return if there is nothing to show
				///////////////////////////////////////////////////////////
				if(visibleTopLeftX == visibleBottomRightX || visibleTopLeftY == visibleBottomRightY)
				{
					if(reuseMemory != 0)
					{
						reuseMemory->resize(0);
					}
					return reuseMemory;
				}

				// Check if the image is visible in the specified area
				///////////////////////////////////////////////////////////
				if(
						visibleBottomRightX > totalWidthPixels ||
						visibleBottomRightY > totalHeightPixels ||
						visibleTopLeftX < 0 ||
						visibleTopLeftY < 0 ||
						visibleTopLeftX > visibleBottomRightX ||
						visibleTopLeftY > visibleBottomRightY
						)
				{
					PUNTOEXE_THROW(drawBitmapExceptionInvalidArea, "Destination area not valid");
				}

                size_t memorySize(getBitmap<drawBitmapType, rowAlignBytes>(totalWidthPixels, totalHeightPixels, visibleTopLeftX, visibleTopLeftY, visibleBottomRightX, visibleBottomRightY, 0, 0));

				if(reuseMemory == 0)
				{
					reuseMemory = memoryPool::getMemoryPool()->getMemory(memorySize);
				}
				else
				{
					reuseMemory->resize(memorySize);
				}

				// Retrieve the final bitmap's buffer
				///////////////////////////////////////////////////////////
				std::uint8_t* pFinalBuffer = (std::uint8_t*)(reuseMemory->data());

				getBitmap<drawBitmapType, rowAlignBytes>(totalWidthPixels, totalHeightPixels, visibleTopLeftX, visibleTopLeftY, visibleBottomRightX, visibleBottomRightY, pFinalBuffer, memorySize);

				return reuseMemory;

				PUNTOEXE_FUNCTION_END();

			}

			template <tDrawBitmapType drawBitmapType, int rowAlignBytes>
					size_t getBitmap(std::int32_t totalWidthPixels, std::int32_t totalHeightPixels,
										  std::int32_t visibleTopLeftX, std::int32_t visibleTopLeftY, std::int32_t visibleBottomRightX, std::int32_t visibleBottomRightY,
										  std::uint8_t* pBuffer, size_t bufferSize)
			{
				PUNTOEXE_FUNCTION_START(L"drawBitmap::getBitmapRaw");

				// Just return if there is nothing to show
				///////////////////////////////////////////////////////////
				if(visibleTopLeftX == visibleBottomRightX || visibleTopLeftY == visibleBottomRightY)
				{
					return 0;
				}

				// Check if the image is visible in the specified area
				///////////////////////////////////////////////////////////
				if(
						visibleBottomRightX > totalWidthPixels ||
						visibleBottomRightY > totalHeightPixels ||
						visibleTopLeftX < 0 ||
						visibleTopLeftY < 0 ||
						visibleTopLeftX > visibleBottomRightX ||
						visibleTopLeftY > visibleBottomRightY
						)
				{
					PUNTOEXE_THROW(drawBitmapExceptionInvalidArea, "Destination area not valid");
				}

                std::uint32_t destPixelSize((drawBitmapType == drawBitmapRGBA || drawBitmapType == drawBitmapBGRA) ? 4 : 3);

                // Calculate the row' size, in bytes
				///////////////////////////////////////////////////////////
                std::uint32_t rowSizeBytes = ((visibleBottomRightX - visibleTopLeftX) * destPixelSize + rowAlignBytes - 1) / rowAlignBytes;
				rowSizeBytes *= rowAlignBytes;


				// Allocate the memory for the final bitmap
				///////////////////////////////////////////////////////////
				std::uint32_t memorySize(rowSizeBytes * (visibleBottomRightY - visibleTopLeftY));
				if(memorySize > bufferSize)
				{
				    return memorySize;
				}

				// Find the multiplier that make the image bigger than
				//  the rendering area
				///////////////////////////////////////////////////////////
				std::uint32_t imageSizeX, imageSizeY;
				m_image->getSize(&imageSizeX, &imageSizeY);

				std::uint8_t leftShiftX(0), leftShiftY(0);
				std::uint32_t maskX(0), maskY(0);
				while( (imageSizeX << leftShiftX) < (std::uint32_t)totalWidthPixels)
				{
					++leftShiftX;
					maskX <<= 1;
					++maskX;
				}
				while( (imageSizeY << leftShiftY) < (std::uint32_t)totalHeightPixels)
				{
					++leftShiftY;
					maskY <<= 1;
					++maskY;
				}

				// Allocate an horizontal buffer that stores the pixels
				//  average colors and a buffer that indicates the pixels
				//  in the source image mapped to the final bitmap
				///////////////////////////////////////////////////////////
				std::uint32_t destBitmapWidth(visibleBottomRightX - visibleTopLeftX);

				ptr<memory> averagePixelsMemory(memoryPool::getMemoryPool()->getMemory(destBitmapWidth * 4 * sizeof(std::int32_t)));
				ptr<memory> sourcePixelIndexMemory(memoryPool::getMemoryPool()->getMemory((destBitmapWidth + 1) * sizeof(std::uint32_t)));
				std::int32_t* averagePixels = (std::int32_t*)averagePixelsMemory->data();
				std::uint32_t* sourcePixelIndex = (std::uint32_t*)sourcePixelIndexMemory->data();
				for(std::int32_t scanPixelsX = visibleTopLeftX; scanPixelsX != visibleBottomRightX + 1; ++scanPixelsX)
				{
					sourcePixelIndex[scanPixelsX - visibleTopLeftX] = scanPixelsX * (imageSizeX << leftShiftX) / totalWidthPixels;
				}

				// Get the index of the first and last+1 pixel to be
				//  displayed
				///////////////////////////////////////////////////////////
				std::int32_t firstPixelX(*sourcePixelIndex);
				std::int32_t lastPixelX(sourcePixelIndex[visibleBottomRightX - visibleTopLeftX]);

				// If a transform chain is active then allocate a temporary
				//  output image
				///////////////////////////////////////////////////////////
				std::uint32_t rowSize, channelSize, channelsNumber;
				ptr<image> sourceImage(m_image);

				// Retrieve the final bitmap's buffer
				///////////////////////////////////////////////////////////
                std::int32_t nextRowGap = rowSizeBytes - destBitmapWidth * destPixelSize;

				// First Y pixel not transformed by the transforms chain
				///////////////////////////////////////////////////////////
				std::int32_t transformChainStartY(0), transformChainNextY(0);

				std::uint32_t sourceHeight;
				std::uint32_t sourceWidth;
				if(m_transformsChain->isEmpty())
				{
					sourceHeight = imageSizeY;
					sourceWidth = imageSizeX;
				}
				else
				{

					sourceWidth = (lastPixelX >> leftShiftX) - (firstPixelX >> leftShiftX) + 1;
					if((firstPixelX >> leftShiftX) + sourceWidth > imageSizeX)
					{
						sourceWidth = imageSizeX - (firstPixelX >> leftShiftX);
					}
                    sourceHeight = 65536 / (sourceWidth * 3);
					if(sourceHeight < 1)
					{
						sourceHeight = 1;
					}
					if(sourceHeight > imageSizeY)
					{
						sourceHeight = imageSizeY;
					}
					sourceImage = new image;
					sourceImage->create(sourceWidth, sourceHeight, image::depthU8, L"RGB", 7);
				}


				// Scan all the final bitmap's rows
				///////////////////////////////////////////////////////////
				for(std::int32_t scanY = visibleTopLeftY; scanY != visibleBottomRightY; ++scanY)
				{
					::memset(averagePixels, 0, destBitmapWidth * 4 * sizeof(std::int32_t));

					// Scan all the image's rows that go in the bitmap's row
					///////////////////////////////////////////////////////////
					std::int32_t firstPixelY = scanY * (imageSizeY << leftShiftY) / totalHeightPixels;
					std::int32_t lastPixelY = (scanY + 1) * (imageSizeY << leftShiftY) / totalHeightPixels;

					for(std::int32_t scanImageY = firstPixelY; scanImageY != lastPixelY; /* increased in the loop */)
					{
						std::int32_t currentImageY = (scanImageY >> leftShiftY);
						std::int32_t* pAveragePointer = averagePixels;
						std::uint32_t* pNextSourceXIndex = sourcePixelIndex;

						std::uint8_t* pImagePointer(0);
						std::uint8_t* imageMemory(0);

						ptr<handlers::dataHandlerNumericBase> imageHandler;
						if(m_transformsChain->isEmpty())
						{
							imageHandler = sourceImage->getDataHandler(false, &rowSize, &channelSize, &channelsNumber);
							imageMemory = imageHandler->getMemoryBuffer();
							pImagePointer = &(imageMemory[currentImageY * imageSizeX * 3 + ((*pNextSourceXIndex) >> leftShiftX) * 3]);
						}
						else
						{
							if(currentImageY >= transformChainNextY)
							{
								transformChainNextY = currentImageY + sourceHeight;
								if(transformChainNextY > (std::int32_t)imageSizeY)
								{
									transformChainNextY = imageSizeY;
								}
								m_transformsChain->runTransform(m_image, firstPixelX >> leftShiftX, currentImageY, sourceWidth, transformChainNextY - currentImageY, sourceImage, 0, 0);
								transformChainStartY = currentImageY;
							}
							imageHandler = sourceImage->getDataHandler(false, &rowSize, &channelSize, &channelsNumber);
							imageMemory = imageHandler->getMemoryBuffer();

							pImagePointer = &(imageMemory[(currentImageY - transformChainStartY) * sourceWidth * 3]);
						}

						std::int32_t scanYBlock ( (scanImageY & (~maskY)) + ((std::int32_t)1 << leftShiftY) );
						if(scanYBlock > lastPixelY)
						{
							scanYBlock = lastPixelY;
						}
						std::int32_t numRows(scanYBlock - scanImageY);
						scanImageY += numRows;

						if(numRows == 1)
						{
							for(std::int32_t scanX (destBitmapWidth); scanX != 0; --scanX)
							{
								for(std::uint32_t scanImageX = *(pNextSourceXIndex++); scanImageX != *pNextSourceXIndex; ++scanImageX)
								{
									++(*pAveragePointer);
									*(++pAveragePointer) += *pImagePointer;
									*(++pAveragePointer) += *(++pImagePointer);
									*(++pAveragePointer) += *(++pImagePointer);
									pAveragePointer -= 3;
									if( (scanImageX & maskX) != 0)
									{
										pImagePointer -= 2;
										continue;
									}
									++pImagePointer;
								}
								pAveragePointer += 4;
							}
						}
						else
						{
							for(std::int32_t scanX (destBitmapWidth); scanX != 0; --scanX)
							{
								for(std::uint32_t scanImageX = *(pNextSourceXIndex++); scanImageX != *pNextSourceXIndex; ++scanImageX)
								{
									*pAveragePointer += numRows;
									*(++pAveragePointer) += *pImagePointer  * numRows;
									*(++pAveragePointer) += *(++pImagePointer)  * numRows;
									*(++pAveragePointer) += *(++pImagePointer)  * numRows;
									pAveragePointer -= 3;
									if( (scanImageX & maskX) != 0)
									{
										pImagePointer -= 2;
										continue;
									}
									++pImagePointer;
								}
								pAveragePointer += 4;
							}
						}
					}

					// Copy the average to the bitmap
					std::int32_t* pAveragePointer = averagePixels;
					std::uint32_t counter;

                    if(drawBitmapType == drawBitmapRGBA)
                    {
                        for(std::int32_t scanX (destBitmapWidth); scanX != 0; --scanX)
                        {
                            counter = (std::uint32_t)*(pAveragePointer++);
                            *(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
                            *(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
                            *(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
                            *(pBuffer++) = 0xff;
                        }
                    }
                    else if(drawBitmapType == drawBitmapBGRA)
                    {
					    std::uint32_t r, g;
                        for(std::int32_t scanX (destBitmapWidth); scanX != 0; --scanX)
                        {
                            counter = (std::uint32_t)*(pAveragePointer++);
							r = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							g = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							*(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							*(pBuffer++) = (std::uint8_t)g;
							*(pBuffer++) = (std::uint8_t)r;
                            *(pBuffer++) = 0xff;
                        }
                    }
                    else if(drawBitmapType == drawBitmapRGB)
					{
						for(std::int32_t scanX (destBitmapWidth); scanX != 0; --scanX)
						{
							counter = (std::uint32_t)*(pAveragePointer++);
							*(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							*(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							*(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
						}
					}
					else
					{
					    std::uint32_t r, g;
						for(std::int32_t scanX (destBitmapWidth); scanX != 0; --scanX)
						{
							counter = (std::uint32_t)*(pAveragePointer++);
							r = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							g = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							*(pBuffer++) = (std::uint8_t) (((std::uint32_t)*(pAveragePointer++) / counter) & 0xff);
							*(pBuffer++) = (std::uint8_t)g;
							*(pBuffer++) = (std::uint8_t)r;
						}
					}

					pBuffer += nextRowGap;
				}

				return memorySize;

				PUNTOEXE_FUNCTION_END();
			}

		protected:
			ptr<image> m_image;

			ptr<image> m_finalImage;

			// Transform that calculates an 8 bit per channel RGB image
			ptr<transforms::transformsChain> m_transformsChain;
		};

		/// @}

	} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraDrawBitmap_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
