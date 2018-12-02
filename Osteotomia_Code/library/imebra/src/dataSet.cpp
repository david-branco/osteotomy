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

/*! \file dataSet.cpp
    \brief Implementation of the class dataSet.

*/

#include "../../base/include/exception.h"
#include "../../base/include/streamReader.h"
#include "../../base/include/streamWriter.h"
#include "../../base/include/memoryStream.h"
#include "../include/dataSet.h"
#include "../include/dataGroup.h"
#include "../include/dataHandlerNumeric.h"
#include "../include/dicomDict.h"
#include "../include/codecFactory.h"
#include "../include/codec.h"
#include "../include/image.h"
#include "../include/LUT.h"
#include "../include/waveform.h"
#include "../include/colorTransformsFactory.h"
#include "../include/transformsChain.h"
#include "../include/transformHighBit.h"
#include "../include/transaction.h"
#include "../include/modalityVOILUT.h"
#include <iostream>
#include <string.h>


namespace puntoexe
{

namespace imebra
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataSet
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<data> dataSet::getTag(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, bool bCreate /* =false */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getTag");

	lockObject lockAccess(this);

	ptr<data> pData;

	ptr<dataGroup>	group=getGroup(groupId, order, bCreate);
	if(group != 0)
	{
		pData=group->getTag(tagId, bCreate);
	}

	return pData;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested group
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<dataGroup> dataSet::getGroup(std::uint16_t groupId, std::uint16_t order, bool bCreate /* =false */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getGroup");

	lockObject lockAccess(this);

	ptr<dataGroup> pData=getData(groupId, order);

	if(pData == 0 && bCreate)
	{
		pData = new dataGroup(this);
		setGroup(groupId, order, pData);
	}

	return pData;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested group
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setGroup(std::uint16_t groupId, std::uint16_t order, ptr<dataGroup> pGroup)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setGroup");

	setData(groupId, order, pGroup);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the image from the structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<image> dataSet::getImage(std::uint32_t frameNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getImage");

	// Lock this object
	///////////////////////////////////////////////////////////
	lockObject lockAccess(this);

	// Retrieve the transfer syntax
	///////////////////////////////////////////////////////////
	std::wstring transferSyntax=getUnicodeString(0x0002, 0x0, 0x0010, 0x0);

	// Get the right codec
	///////////////////////////////////////////////////////////
	ptr<codecs::codec> pCodec=codecs::codecFactory::getCodec(transferSyntax);

	// Return if the codec has not been found
	///////////////////////////////////////////////////////////
	if(pCodec == 0)
	{
		PUNTOEXE_THROW(dataSetExceptionUnknownTransferSyntax, "None of the codecs support the specified transfer syntax");
	}

	ptr<imebra::data> imageTag = getTag(0x7fe0, 0x0, 0x0010, false);
	if(imageTag == 0)
	{
		PUNTOEXE_THROW(dataSetImageDoesntExist, "The requested image doesn't exist");
	}
	std::string imageStreamDataType = imageTag->getDataType();

	// Get the number of frames
	///////////////////////////////////////////////////////////
	std::uint32_t numberOfFrames = 1;
	if(!getDataType(0x0028, 0, 0x0008).empty())
	{
		numberOfFrames = getUnsignedLong(0x0028, 0, 0x0008, 0);
	}

	if(frameNumber >= numberOfFrames)
	{
		PUNTOEXE_THROW(dataSetImageDoesntExist, "The requested image doesn't exist");
	}

	// Placeholder for the stream containing the image
	///////////////////////////////////////////////////////////
	ptr<streamReader> imageStream;

	// Retrieve the second item in the image's tag.
	// If the second item is present, then a multiframe
	//  image is present.
	///////////////////////////////////////////////////////////
	bool bDontNeedImagesPositions = false;
	{
		if(imageTag->getBufferSize(1) != 0)
		{
			std::uint32_t firstBufferId(0), endBufferId(0), totalLength(0);
			if(imageTag->getBufferSize(0) == 0 && numberOfFrames + 1 == imageTag->getBuffersCount())
			{
				firstBufferId = frameNumber + 1;
				endBufferId = firstBufferId + 1;
				totalLength = imageTag->getBufferSize(firstBufferId);
			}
			else
			{
				totalLength = getFrameBufferIds(frameNumber, &firstBufferId, &endBufferId);
			}
			if(firstBufferId == endBufferId - 1)
			{
				imageStream = imageTag->getStreamReader(firstBufferId);
				if(imageStream == 0)
				{
					PUNTOEXE_THROW(dataSetImageDoesntExist, "The requested image doesn't exist");
				}
			}
			else
			{
				ptr<memory> temporaryMemory(memoryPool::getMemoryPool()->getMemory(totalLength));
				const std::uint8_t* pDest = temporaryMemory->data();
				for(std::uint32_t scanBuffers = firstBufferId; scanBuffers != endBufferId; ++scanBuffers)
				{
					ptr<handlers::dataHandlerRaw> bufferHandler = imageTag->getDataHandlerRaw(scanBuffers, false, "");
					std::uint8_t* pSource = bufferHandler->getMemoryBuffer();
					::memcpy((void*)pDest, (void*)pSource, bufferHandler->getSize());
					pDest += bufferHandler->getSize();
				}
				ptr<baseStream> compositeStream(new memoryStream(temporaryMemory));
				imageStream = ptr<streamReader>(new streamReader(compositeStream));
			}
			bDontNeedImagesPositions = true;
		}
	}

	// If the image cannot be found, then probably we are
	//  handling an old dicom format.
	// Then try to read the image from the next group with
	//  id=0x7fe
	///////////////////////////////////////////////////////////
	if(imageStream == 0)
	{
		imageStream = getStreamReader(0x7fe0, (std::uint16_t)frameNumber, 0x0010, 0x0);
		bDontNeedImagesPositions = true;
	}

	// We are dealing with an old dicom format that doesn't
	//  include the image offsets and stores all the images
	//  in one buffer
	///////////////////////////////////////////////////////////
	if(imageStream == 0)
	{
		imageStream = imageTag->getStreamReader(0x0);
		if(imageStream == 0)
		{
			PUNTOEXE_THROW(dataSetImageDoesntExist, "The requested image doesn't exist");
		}

		// Reset an internal array that keeps track of the
		//  images position
		///////////////////////////////////////////////////////////
		if(m_imagesPositions.size() != numberOfFrames)
		{
			m_imagesPositions.resize(numberOfFrames);

			for(std::uint32_t resetImagesPositions = 0; resetImagesPositions < numberOfFrames; m_imagesPositions[resetImagesPositions++] = 0)
			{}// empty loop

		}

		// Read all the images before the desidered one so we set
		//  reading position in the stream
		///////////////////////////////////////////////////////////
		for(std::uint32_t readImages = 0; readImages < frameNumber; readImages++)
		{
			std::uint32_t offsetPosition = m_imagesPositions[readImages];
			if(offsetPosition == 0)
			{
				ptr<image> tempImage = pCodec->getImage(this, imageStream, imageStreamDataType);
				m_imagesPositions[readImages] = imageStream->position();
				continue;
			}
			if((m_imagesPositions[readImages + 1] == 0) || (readImages == (frameNumber - 1)))
			{
				imageStream->seek(offsetPosition);
			}
		}
	}

	double pixelDistanceX=getDouble(0x0028, 0x0, 0x0030, 0);
	double pixelDistanceY=getDouble(0x0028, 0x0, 0x0030, 1);
	if(bDontNeedImagesPositions)
	{
		lockAccess.unlock();
	}

	ptr<image> pImage;
	pImage = pCodec->getImage(this, imageStream, imageStreamDataType);

	if(!bDontNeedImagesPositions && m_imagesPositions.size() > frameNumber)
	{
		m_imagesPositions[frameNumber] = imageStream->position();
	}

	// If the image has been returned correctly, then set
	//  the image's size
	///////////////////////////////////////////////////////////
	if(pImage != 0)
	{
		std::uint32_t sizeX, sizeY;
		pImage->getSize(&sizeX, &sizeY);
		pImage->setSizeMm(pixelDistanceX*(double)sizeX, pixelDistanceY*(double)sizeY);
	}

	if(pImage->getColorSpace() == L"PALETTE COLOR")
	{
		ptr<lut> red(new lut), green(new lut), blue(new lut);
		red->setLut(getDataHandler(0x0028, 0x0, 0x1101, 0, false), getDataHandler(0x0028, 0x0, 0x1201, 0, false), L"");
		green->setLut(getDataHandler(0x0028, 0x0, 0x1102, 0, false), getDataHandler(0x0028, 0x0, 0x1202, 0, false), L"");
		blue->setLut(getDataHandler(0x0028, 0x0, 0x1103, 0, false), getDataHandler(0x0028, 0x0, 0x1203, 0, false), L"");
		ptr<palette> imagePalette(new palette(red, green, blue));
		pImage->setPalette(imagePalette);
	}

	return pImage;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get an image from the dataset and apply the modality
//  transform.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<image> dataSet::getModalityImage(std::uint32_t frameNumber)
{
    PUNTOEXE_FUNCTION_START(L"dataSet::getImage");

    ptr<image> originalImage = getImage(frameNumber);

    ptr<transforms::colorTransforms::colorTransformsFactory> colorFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
    if(originalImage == 0 || !colorFactory->isMonochrome(originalImage->getColorSpace()))
    {
        return originalImage;
    }

    ptr<transforms::modalityVOILUT> modalityVOILUT(new transforms::modalityVOILUT(this));

    // Convert to MONOCHROME2 if a modality transform is not present
    ////////////////////////////////////////////////////////////////
    if(modalityVOILUT->isEmpty())
    {
        ptr<transforms::colorTransforms::colorTransform> monochromeColorTransform(colorFactory->getTransform(originalImage->getColorSpace(), L"MONOCHROME2"));
        if(monochromeColorTransform != 0)
        {
            std::uint32_t width, height;
            originalImage->getSize(&width, &height);
            ptr<image> outputImage = monochromeColorTransform->allocateOutputImage(originalImage, width, height);
            monochromeColorTransform->runTransform(originalImage, 0, 0, width, height, outputImage, 0, 0);
            return outputImage;
        }

        return originalImage;
    }

    // Apply the modality VOI/LUT transform
    ///////////////////////////////////////
    std::uint32_t width, height;
    originalImage->getSize(&width, &height);
    ptr<image> outputImage = modalityVOILUT->allocateOutputImage(originalImage, width, height);
    modalityVOILUT->runTransform(originalImage, 0, 0, width, height, outputImage, 0, 0);
    return outputImage;

    PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert an image into the dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setImage(std::uint32_t frameNumber, ptr<image> pImage, std::wstring transferSyntax, codecs::codec::quality quality)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setImage");

	// Lock the entire dataset
	///////////////////////////////////////////////////////////
	lockObject lockAccess(this);

	// All the commit are in one transaction
	///////////////////////////////////////////////////////////
	transaction localTransaction(true);

	// The group, order, tag and buffer where the image must
	//  be stored
	///////////////////////////////////////////////////////////
	std::uint16_t groupId(0x7fe0), orderId(0), tagId(0x0010);
    std::uint32_t firstBufferId(0);

	// bDontChangeAttributes is true if some images already
	//  exist in the dataset and we must save the new image
	//  using the attributes already stored
	///////////////////////////////////////////////////////////
	std::uint32_t numberOfFrames = getUnsignedLong(0x0028, 0, 0x0008, 0);
	if(frameNumber != numberOfFrames)
	{
		PUNTOEXE_THROW(dataSetExceptionWrongFrame, "The frames must be inserted in sequence");
	}
	bool bDontChangeAttributes = (numberOfFrames != 0);
	if(bDontChangeAttributes)
	{
		transferSyntax = getUnicodeString(0x0002, 0x0, 0x0010, 0x0);
	}

	// Select the right codec
	///////////////////////////////////////////////////////////
	ptr<codecs::codec> saveCodec(codecs::codecFactory::getCodec(transferSyntax));
	if(saveCodec == 0L)
	{
		PUNTOEXE_THROW(dataSetExceptionUnknownTransferSyntax, "None of the codec support the requested transfer syntax");
	}

	// Do we have to save the basic offset table?
	///////////////////////////////////////////////////////////
	bool bEncapsulated = saveCodec->encapsulated(transferSyntax) ||
		                 (getDataHandlerRaw(groupId, 0x0, tagId, 0x1, false) != 0);

	// Check if we are dealing with an old Dicom format...
	///////////////////////////////////////////////////////////
	std::string dataHandlerType = getDataType(0x7fe0, 0x1, 0x0010);
	if(!dataHandlerType.empty())
	{
		orderId = (std::uint16_t)frameNumber;
		bEncapsulated = false;
	}


	// Set the subsampling flags
	///////////////////////////////////////////////////////////
	bool bSubSampledX = quality > codecs::codec::high;
	bool bSubSampledY = quality > codecs::codec::medium;
	if( !transforms::colorTransforms::colorTransformsFactory::canSubsample(pImage->getColorSpace()) )
	{
		bSubSampledX = bSubSampledY = false;
	}

    image::bitDepth depth = pImage->getDepth();
    bool b2complement = (depth == image::depthS32 || depth == image::depthS16 || depth == image::depthS8);
    std::uint32_t channelsNumber = pImage->getChannelsNumber();
    std::uint8_t allocatedBits = (std::uint8_t)(saveCodec->suggestAllocatedBits(transferSyntax, pImage->getHighBit()));
    bool bInterleaved(false);
    if(getDataType(0x0028, 0, 0x0006).empty())
    {
        if(channelsNumber > 1)
        {
            bInterleaved = true;
            setUnsignedLong(0x0028, 0, 0x0006, 0, 0, "US");
        }
    }
    else
    {
        bInterleaved = (getUnsignedLong(0x0028, 0x0, 0x0006, 0x0) == 0x0);
    }

	// If the attributes cannot be changed, then check the
	//  attributes already stored in the dataset
	///////////////////////////////////////////////////////////
	if(bDontChangeAttributes)
	{
		pImage = convertImageForDataSet(pImage);
		std::wstring currentColorSpace = getUnicodeString(0x0028, 0x0, 0x0004, 0x0);
		bSubSampledX = transforms::colorTransforms::colorTransformsFactory::isSubsampledX(currentColorSpace);
		bSubSampledY = transforms::colorTransforms::colorTransformsFactory::isSubsampledY(currentColorSpace);
		b2complement = (getUnsignedLong(0x0028, 0, 0x0103, 0) != 0);
		allocatedBits = (std::uint8_t)getUnsignedLong(0x0028, 0x0, 0x0100, 0x0);
		channelsNumber = getUnsignedLong(0x0028, 0x0, 0x0002, 0x0);
	}

	// Select the data type OB if not already set in the
	//  dataset
	///////////////////////////////////////////////////////////
	if(dataHandlerType.empty())
	{
		if(transferSyntax == L"1.2.840.10008.1.2")
		{
			dataHandlerType = getDefaultDataType(0x7FE0, 0x0010);
		}
		else
		{
			dataHandlerType = (bEncapsulated || allocatedBits <= 8) ? "OB" : "OW";
		}
	}

	// Encapsulated mode. Check if we have the offsets table
	///////////////////////////////////////////////////////////
	if(bEncapsulated)
	{
		// We have to add the offsets buffer
		///////////////////////////////////////////////////////////
		ptr<handlers::dataHandlerRaw> imageHandler0 = getDataHandlerRaw(groupId, 0x0, tagId, 0x0, false);
		ptr<handlers::dataHandlerRaw> imageHandler1 = getDataHandlerRaw(groupId, 0x0, tagId, 0x1, false);
		if(imageHandler0 != 0L && imageHandler0->getSize() != 0 && imageHandler1 == 0L)
		{
			// The first image must be moved forward, in order to
			//  make some room for the offset table
			///////////////////////////////////////////////////////////
			dataHandlerType = imageHandler0->getDataType();
			ptr<handlers::dataHandlerRaw> moveFirstImage = getDataHandlerRaw(groupId, 0x0, tagId, 0x1, true, dataHandlerType);

			if(moveFirstImage == 0L)
			{
				PUNTOEXE_THROW(dataSetExceptionOldFormat, "Cannot move the first image");
			}
			std::uint32_t bufferSize=imageHandler0->getSize();
			moveFirstImage->setSize(bufferSize);
			::memcpy(moveFirstImage->getMemoryBuffer(), imageHandler0->getMemoryBuffer(), bufferSize);
		}

		// An image in the first buffer already exists.
		///////////////////////////////////////////////////////////
		if(imageHandler1 != 0)
		{
			dataHandlerType = imageHandler1->getDataType();
		}

		firstBufferId = getFirstAvailFrameBufferId();
	}

	// Get a stream to save the image
	///////////////////////////////////////////////////////////
	ptr<streamWriter> outputStream;
	ptr<memory> uncompressedImage(new memory);
	if(bEncapsulated || frameNumber == 0)
	{
		outputStream = getStreamWriter(groupId, orderId, tagId, firstBufferId, dataHandlerType);
	}
	else
	{
		ptr<puntoexe::memoryStream> memStream(new memoryStream(uncompressedImage));
		outputStream = new streamWriter(memStream);
	}

	// Save the image in the stream
	///////////////////////////////////////////////////////////
	saveCodec->setImage(
		outputStream,
		pImage,
		transferSyntax,
		quality,
		dataHandlerType,
		allocatedBits,
		bSubSampledX, bSubSampledY,
		bInterleaved,
		b2complement);
	outputStream->flushDataBuffer();

	if(!bEncapsulated && frameNumber != 0)
	{
		ptr<handlers::dataHandlerRaw> copyUncompressed(getDataHandlerRaw(groupId, orderId, tagId, firstBufferId, true));
		copyUncompressed->setSize((frameNumber + 1) * uncompressedImage->size());
		std::uint8_t* pSource = uncompressedImage->data();
		std::uint8_t* pDest = copyUncompressed->getMemoryBuffer() + (frameNumber * uncompressedImage->size());
		::memcpy(pDest, pSource, uncompressedImage->size());
	}

	// The images' positions calculated by getImage are not
	//  valid now. They must be recalculated.
	///////////////////////////////////////////////////////////
	m_imagesPositions.clear();

	// Write the attributes in the dataset
	///////////////////////////////////////////////////////////
	if(!bDontChangeAttributes)
	{
		ptr<handlers::dataHandler> dataHandlerTransferSyntax = getDataHandler(0x0002, 0x0, 0x0010, 0x0, true);
		dataHandlerTransferSyntax->setUnicodeString(0, transferSyntax);

		std::wstring colorSpace = pImage->getColorSpace();
		setUnicodeString(0x0028, 0x0, 0x0004, 0x0, transforms::colorTransforms::colorTransformsFactory::makeSubsampled(colorSpace, bSubSampledX, bSubSampledY));
		setUnsignedLong(0x0028, 0x0, 0x0006, 0x0, bInterleaved ? 0 : 1);
		setUnsignedLong(0x0028, 0x0, 0x0100, 0x0, allocatedBits);            // allocated bits
		setUnsignedLong(0x0028, 0x0, 0x0101, 0x0, pImage->getHighBit() + 1); // stored bits
		setUnsignedLong(0x0028, 0x0, 0x0102, 0x0, pImage->getHighBit());     // high bit
		setUnsignedLong(0x0028, 0x0, 0x0103, 0x0, b2complement ? 1 : 0);
		setUnsignedLong(0x0028, 0x0, 0x0002, 0x0, channelsNumber);
		std::uint32_t imageSizeX, imageSizeY;
		pImage->getSize(&imageSizeX, &imageSizeY);
		setUnsignedLong(0x0028, 0x0, 0x0011, 0x0, imageSizeX);
		setUnsignedLong(0x0028, 0x0, 0x0010, 0x0, imageSizeY);

		if(colorSpace == L"PALETTECOLOR")
		{
			ptr<palette> imagePalette(pImage->getPalette());
			if(imagePalette != 0)
			{
				imagePalette->getRed()->fillHandlers(getDataHandler(0x0028, 0x0, 0x1101, 0, true), getDataHandler(0x0028, 0x0, 0x1201, 0, true));
				imagePalette->getGreen()->fillHandlers(getDataHandler(0x0028, 0x0, 0x1102, 0, true), getDataHandler(0x0028, 0x0, 0x1202, 0, true));
				imagePalette->getBlue()->fillHandlers(getDataHandler(0x0028, 0x0, 0x1103, 0, true), getDataHandler(0x0028, 0x0, 0x1203, 0, true));
			}

		}

		double imageSizeMmX, imageSizeMmY;
		pImage->getSizeMm(&imageSizeMmX, &imageSizeMmY);

	}

	// Update the number of frames
	///////////////////////////////////////////////////////////
	numberOfFrames = frameNumber + 1;
	setUnsignedLong(0x0028, 0, 0x0008, 0, numberOfFrames );

	// Update the offsets tag with the image's offsets
	///////////////////////////////////////////////////////////
	if(!bEncapsulated)
	{
		return;
	}

	std::uint32_t calculatePosition(0);
	ptr<data> tag(getTag(groupId, 0, tagId, true));
	for(std::uint32_t scanBuffers = 1; scanBuffers != firstBufferId; ++scanBuffers)
	{
		calculatePosition += tag->getBufferSize(scanBuffers);
		calculatePosition += 8;
	}
	ptr<handlers::dataHandlerRaw> offsetHandler(getDataHandlerRaw(groupId, 0, tagId, 0, true, dataHandlerType));
	offsetHandler->setSize(4 * (frameNumber + 1));
	std::uint8_t* pOffsetFrame(offsetHandler->getMemoryBuffer() + (frameNumber * 4));
	*( (std::uint32_t*)pOffsetFrame  ) = calculatePosition;
	streamController::adjustEndian(pOffsetFrame, 4, streamController::lowByteEndian, 1);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Get the offset, in bytes, of the specified frame
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameOffset(std::uint32_t frameNumber)
{
	// Retrieve the buffer containing the offsets
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerRaw> framesPointer = getDataHandlerRaw(0x7fe0, 0x0, 0x0010, 0, false);
	if(framesPointer == 0)
	{
		return 0xffffffff;
	}

	// Get the offset table's size, in number of offsets
	///////////////////////////////////////////////////////////
	std::uint32_t offsetsCount = framesPointer->getSize() / sizeof(std::uint32_t);

	// If the requested frame doesn't exist then return
	//  0xffffffff (the maximum value)
	///////////////////////////////////////////////////////////
	if(frameNumber >= offsetsCount && frameNumber != 0)
	{
		return 0xffffffff;
	}

	// Return the requested offset. If the requested frame is
	//  the first and is offset is not specified, then return
	//  0 (the first position)
	///////////////////////////////////////////////////////////
    if(frameNumber < offsetsCount)
    {
        std::uint32_t* pOffsets = (std::uint32_t*)(framesPointer->getMemoryBuffer());
        std::uint32_t returnOffset(pOffsets[frameNumber]);
        streamController::adjustEndian((std::uint8_t*)&returnOffset, 4, streamController::lowByteEndian);
        return returnOffset;
	}
	return 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Return the buffer that starts at the specified offset
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameBufferId(std::uint32_t offset, std::uint32_t* pLengthToBuffer)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getFrameBufferId");

	*pLengthToBuffer = 0;

	ptr<data> imageTag = getTag(0x7fe0, 0, 0x0010, false);
	if(imageTag == 0)
	{
		return 0;
	}

	// Start from the buffer n. 1 (the buffer 0 contains
	//  the offset table
	///////////////////////////////////////////////////////////
	std::uint32_t scanBuffers(1);

	if(offset == 0xffffffff)
	{
		while(imageTag->bufferExists(scanBuffers))
		{
			++scanBuffers;
		}
		return scanBuffers;
	}

	while(offset != 0)
	{
		// If the handler isn't connected to any buffer, then
		//  the buffer doesn't exist: return
		///////////////////////////////////////////////////////////
		if(!imageTag->bufferExists(scanBuffers))
		{
			break;
		}

		// Calculate the total size of the buffer, including
		//  its descriptor (tag group and id and length)
		///////////////////////////////////////////////////////////
		std::uint32_t bufferSize = imageTag->getBufferSize(scanBuffers);;
		(*pLengthToBuffer) += bufferSize; // Increase the total size
		bufferSize += 4; // one WORD for the group id, one WORD for the tag id
		bufferSize += 4; // one DWORD for the tag length
		if(bufferSize > offset)
		{
			PUNTOEXE_THROW(dataSetImageDoesntExist, "Image not in the offset table");
		}
		offset -= bufferSize;
		++scanBuffers;
	}

	if(offset != 0)
	{
		PUNTOEXE_THROW(dataSetCorruptedOffsetTable, "The basic offset table is corrupted");
	}

	return scanBuffers;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the first buffer and the end buffer occupied by an
//  image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameBufferIds(std::uint32_t frameNumber, std::uint32_t* pFirstBuffer, std::uint32_t* pEndBuffer)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getFrameBufferIds");

	std::uint32_t startOffset = getFrameOffset(frameNumber);
	std::uint32_t endOffset = getFrameOffset(frameNumber + 1);

	if(startOffset == 0xffffffff)
	{
		PUNTOEXE_THROW(dataSetImageDoesntExist, "Image not in the table offset");
	}

	std::uint32_t startLength, endLength;
	*pFirstBuffer = getFrameBufferId(startOffset, &startLength);
	*pEndBuffer = getFrameBufferId(endOffset, &endLength);

	ptr<data> imageTag = getTag(0x7fe0, 0, 0x0010, false);
	if(imageTag == 0)
	{
		return 0;
	}
	std::uint32_t totalSize(0);
	for(std::uint32_t scanBuffers(*pFirstBuffer); scanBuffers != *pEndBuffer; ++scanBuffers)
	{
		totalSize += imageTag->getBufferSize(scanBuffers);
	}
	return totalSize;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the id of the first available buffer that can
//  be used to store a new frame
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFirstAvailFrameBufferId()
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getFirstAvailFrameBufferId");

	ptr<data> imageTag = getTag(0x7fe0, 0, 0x0010, false);
	if(imageTag == 0)
	{
		return 1;
	}

	std::uint32_t availableId(1);
	while(imageTag->bufferExists(availableId))
	{
		++availableId;
	}

	return availableId;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Converts an image using the attributes specified in
//  the dataset.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<image> dataSet::convertImageForDataSet(ptr<image> sourceImage)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::convertImageForDataSet");

	std::uint32_t imageWidth, imageHeight;
	sourceImage->getSize(&imageWidth, &imageHeight);

	std::wstring colorSpace = sourceImage->getColorSpace();
	std::uint32_t highBit = sourceImage->getHighBit();

	std::uint32_t currentWidth  = getUnsignedLong(0x0028, 0x0, 0x0011, 0x0);
	std::uint32_t currentHeight = getUnsignedLong(0x0028, 0x0, 0x0010, 0x0);
	std::uint32_t currentHighBit = getUnsignedLong(0x0028, 0x0, 0x0102, 0x0);





	std::wstring currentColorSpace = transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(getUnicodeString(0x0028, 0x0, 0x0004, 0x0));

	if(currentWidth != imageWidth || currentHeight != imageHeight)
	{
		PUNTOEXE_THROW(dataSetExceptionDifferentFormat, "The dataset already contains an image with a different size");
	}

	if(currentHighBit < highBit)
	{
		PUNTOEXE_THROW(dataSetExceptionDifferentFormat, "The high bit in the dataset is smaller than the requested one");
	}

	if( !transforms::colorTransforms::colorTransformsFactory::isMonochrome(colorSpace) && colorSpace != currentColorSpace)
	{
		PUNTOEXE_THROW(dataSetExceptionDifferentFormat, "The requested color space doesn't match the one already stored in the dataset");
	}

	ptr<transforms::transformsChain> chain(new transforms::transformsChain);
	if(colorSpace != currentColorSpace)
	{
		ptr<transforms::colorTransforms::colorTransformsFactory> pColorFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
		ptr<transforms::transform> colorChain = pColorFactory->getTransform(colorSpace, currentColorSpace);
		if(colorChain->isEmpty())
		{
			PUNTOEXE_THROW(dataSetExceptionDifferentFormat, "The image color space cannot be converted to the dataset color space");
		}
		chain->addTransform(colorChain);
	}

	if(currentHighBit != highBit)
	{
		chain->addTransform(new transforms::transformHighBit);
	}

	if(chain->isEmpty())
	{
		return sourceImage;
	}

	ptr<image> destImage(new image);
	bool b2Complement(getUnsignedLong(0x0028, 0x0, 0x0103, 0x0)!=0x0);
	image::bitDepth depth;
	if(b2Complement)
		depth=highBit>=8 ? image::depthS16 : image::depthS8;
	else
		depth=highBit>=8 ? image::depthU16 : image::depthU8;
	destImage->create(currentWidth, currentHeight, depth, currentColorSpace, currentHighBit);

	chain->runTransform(sourceImage, 0, 0, imageWidth, imageHeight, destImage, 0, 0);

	return destImage;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a sequence item as a dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<dataSet> dataSet::getSequenceItem(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t itemId)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getSequenceItem");

	ptr<data> tag=getTag(groupId, order, tagId, false);
	ptr<dataSet> pDataSet;
	if(tag != 0)
	{
		pDataSet = tag->getDataSet(itemId);
	}

	return pDataSet;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a LUT from the data set
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<lut> dataSet::getLut(std::uint16_t groupId, std::uint16_t tagId, std::uint32_t lutId)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getLut");

	lockObject lockAccess(this);

	ptr<lut> pLUT;
	ptr<dataSet> embeddedLUT=getSequenceItem(groupId, 0, tagId, lutId);
	std::string tagType = getDataType(groupId, 0, tagId);
	if(embeddedLUT != 0)
	{
		ptr<lut> tempLut(new lut);
		pLUT = tempLut;
		ptr<handlers::dataHandler> descriptorHandle=embeddedLUT->getDataHandler(0x0028, 0x0, 0x3002, 0x0, false);
		ptr<handlers::dataHandler> dataHandle=embeddedLUT->getDataHandler(0x0028, 0x0, 0x3006, 0x0, false);

		pLUT->setLut(
			descriptorHandle,
			dataHandle,
			embeddedLUT->getUnicodeString(0x0028, 0x0, 0x3003, 0x0));
	}
	return pLUT;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a waveform
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<waveform> dataSet::getWaveform(std::uint32_t waveformId)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getWaveform");

	lockObject lockAccess(this);

	ptr<dataSet> embeddedWaveform(getSequenceItem(0x5400, 0, 0x0100, waveformId));
	if(embeddedWaveform == 0)
	{
		return 0;
	}

	return new waveform(embeddedWaveform);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a tag as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t dataSet::getSignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getSignedLong");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0, false);
	if(dataHandler == 0)
	{
		return 0;
	}

	return dataHandler->pointerIsValid(elementNumber) ? dataHandler->getSignedLong(elementNumber) : 0;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a tag as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setSignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::int32_t newValue, std::string defaultType /* = "" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setSignedLong");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0, true, defaultType);
	if(dataHandler != 0)
	{
		if(dataHandler->getSize() <= elementNumber)
		{
			dataHandler->setSize(elementNumber + 1);
		}
		dataHandler->setSignedLong(elementNumber, newValue);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getUnsignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getUnignedLong");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0, false);
	if(dataHandler == 0)
	{
		return 0;
	}

	return dataHandler->pointerIsValid(elementNumber) ? dataHandler->getUnsignedLong(elementNumber) : 0;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setUnsignedLong(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::uint32_t newValue, std::string defaultType /* = "" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setUnsignedLong");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0, true, defaultType);
	if(dataHandler != 0)
	{
		if(dataHandler->getSize() <= elementNumber)
		{
			dataHandler->setSize(elementNumber + 1);
		}
		dataHandler->setUnsignedLong(elementNumber, newValue);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double dataSet::getDouble(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getDouble");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0, false);
	if(dataHandler == 0)
	{
		return 0.0;
	}

	return dataHandler->pointerIsValid(elementNumber) ? dataHandler->getDouble(elementNumber) : 0.0;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setDouble(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, double newValue, std::string defaultType /* = "" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setDouble");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0, true, defaultType);
	if(dataHandler != 0)
	{
		if(dataHandler->getSize() <= elementNumber)
		{
			dataHandler->setSize(elementNumber + 1);
		}
		dataHandler->setDouble(elementNumber, newValue);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataSet::getString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getString");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0L, false);
	std::string returnValue;
	if(dataHandler != 0)
	{
		if(dataHandler->pointerIsValid(elementNumber))
		{
			returnValue = dataHandler->getString(elementNumber);
		}
	}

	return returnValue;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as an unicode string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataSet::getUnicodeString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getUnicodeString");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0L, false);
	std::wstring returnValue;
	if(dataHandler != 0)
	{
		if(dataHandler->pointerIsValid(elementNumber))
		{
			returnValue = dataHandler->getUnicodeString(elementNumber);
		}
	}

	return returnValue;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::string newString, std::string defaultType /* = "" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setString");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0L, true, defaultType);
	if(dataHandler != 0)
	{
		if(dataHandler->getSize() <= elementNumber)
		{
			dataHandler->setSize(elementNumber + 1);
		}
		dataHandler->setString(elementNumber, newString);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setUnicodeString(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t elementNumber, std::wstring newString, std::string defaultType /* = "" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::setUnicodeString");

	ptr<handlers::dataHandler> dataHandler=getDataHandler(groupId, order, tagId, 0L, true, defaultType);
	if(dataHandler != 0)
	{
		if(dataHandler->getSize() <= elementNumber)
		{
			dataHandler->setSize(elementNumber + 1);
		}
		dataHandler->setUnicodeString(elementNumber, newString);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a data handler for the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<handlers::dataHandler> dataSet::getDataHandler(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId, bool bWrite, std::string defaultType /* ="" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getDataHandler");

	lockObject lockAccess(this);

	ptr<dataGroup>	group=getGroup(groupId, order, bWrite);

	ptr<handlers::dataHandler> pDataHandler;

	if(group == 0)
	{
		return pDataHandler;
	}

	if(defaultType.length()!=2L)
	{
		defaultType=getDefaultDataType(groupId, tagId);
	}

	pDataHandler = group->getDataHandler(tagId, bufferId, bWrite, defaultType);

	return pDataHandler;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a stream reader that works on the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<streamReader> dataSet::getStreamReader(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getStream");

	lockObject lockAccess(this);

	ptr<dataGroup>	group=getGroup(groupId, order, false);

	ptr<streamReader> returnStream;

	if(group != 0)
	{
		returnStream = group->getStreamReader(tagId, bufferId);
	}

	return returnStream;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a stream writer for the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<streamWriter> dataSet::getStreamWriter(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId, std::string dataType /* = "" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getStream");

	lockObject lockAccess(this);

	ptr<dataGroup>	group=getGroup(groupId, order, true);

	ptr<streamWriter> returnStream;

	if(group != 0)
	{
		returnStream = group->getStreamWriter(tagId, bufferId, dataType);
	}

	return returnStream;

	PUNTOEXE_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a raw data handler for the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<handlers::dataHandlerRaw> dataSet::getDataHandlerRaw(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId, std::uint32_t bufferId, bool bWrite, std::string defaultType /* ="" */)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getDataHandlerRaw");

	lockObject lockAccess(this);

	ptr<dataGroup>	group=getGroup(groupId, order, bWrite);

	if(group == 0)
	{
		ptr<handlers::dataHandlerRaw> emptyDataHandler;
		return emptyDataHandler;
	}

	if(defaultType.length()!=2)
	{
		defaultType=getDefaultDataType(groupId, tagId);
	}

	return group->getDataHandlerRaw(tagId, bufferId, bWrite, defaultType);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the requested tag's default data type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataSet::getDefaultDataType(std::uint16_t groupId, std::uint16_t tagId)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getDefaultDataType");

	return dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the data type of a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataSet::getDataType(std::uint16_t groupId, std::uint16_t order, std::uint16_t tagId)
{
	PUNTOEXE_FUNCTION_START(L"dataSet::getDataType");

	std::string bufferType;

	ptr<data> tag = getTag(groupId, order, tagId, false);
	if(tag != 0)
	{
		bufferType = tag->getDataType();
	}
	return bufferType;

	PUNTOEXE_FUNCTION_END();
}

void dataSet::updateCharsetTag()
{
	charsetsList::tCharsetsList charsets;
	getCharsetsList(&charsets);
	ptr<handlers::dataHandler> charsetHandler(getDataHandler(0x0008, 0, 0x0005, 0, true));
	charsetHandler->setSize((std::uint32_t)(charsets.size()));
	std::uint32_t pointer(0);
	for(charsetsList::tCharsetsList::iterator scanCharsets = charsets.begin(); scanCharsets != charsets.end(); ++scanCharsets)
	{
		charsetHandler->setUnicodeString(pointer++, *scanCharsets);
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Update the list of the used charsets
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::updateTagsCharset()
{
	charsetsList::tCharsetsList charsets;
	ptr<handlers::dataHandler> charsetHandler(getDataHandler(0x0008, 0, 0x0005, 0, false));
	if(charsetHandler != 0)
	{
		for(std::uint32_t pointer(0); charsetHandler->pointerIsValid(pointer); ++pointer)
		{
			charsets.push_back(charsetHandler->getUnicodeString(pointer));
		}
	}
	setCharsetsList(&charsets);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the item's position in the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setItemOffset(std::uint32_t offset)
{
	m_itemOffset = offset;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the item's position in the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getItemOffset()
{
	return m_itemOffset;
}


} // namespace imebra

} // namespace puntoexe
