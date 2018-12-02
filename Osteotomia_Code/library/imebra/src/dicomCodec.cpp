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

/*! \file dicomCodec.cpp
    \brief Implementation of the class dicomCodec.

*/

#include <list>
#include <vector>
#include <string.h>
#include "../../base/include/exception.h"
#include "../../base/include/streamReader.h"
#include "../../base/include/streamWriter.h"
#include "../../base/include/memory.h"
#include "../include/dicomCodec.h"
#include "../include/dataSet.h"
#include "../include/dicomDict.h"
#include "../include/image.h"
#include "../include/colorTransformsFactory.h"
#include "../include/codecFactory.h"


namespace puntoexe
{

namespace imebra
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dicomCodec
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
// Create another DICOM codec
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<codec> dicomCodec::createCodec()
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::createCodec");

	return ptr<codec>(new dicomCodec);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a Dicom stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writeStream(ptr<streamWriter> pStream, ptr<dataSet> pDataSet)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::writeStream");

	// Retrieve the transfer syntax
	///////////////////////////////////////////////////////////
	std::wstring transferSyntax=pDataSet->getUnicodeString(0x0002, 0, 0x0010, 0);

	// Adjust the flags
	///////////////////////////////////////////////////////////
	bool bExplicitDataType = (transferSyntax != L"1.2.840.10008.1.2");        // Implicit VR little endian

	// Explicit VR big endian
	///////////////////////////////////////////////////////////
	streamController::tByteOrdering endianType = (transferSyntax == L"1.2.840.10008.1.2.2") ? streamController::highByteEndian : streamController::lowByteEndian;

	// Write the dicom header
	///////////////////////////////////////////////////////////
	std::uint8_t zeroBuffer[128];
	::memset(zeroBuffer, 0L, 128L);
	pStream->write(zeroBuffer, 128);

	// Write the DICM signature
	///////////////////////////////////////////////////////////
	pStream->write((std::uint8_t*)"DICM", 4);

	// Build the stream
	///////////////////////////////////////////////////////////
	buildStream(pStream, pDataSet, bExplicitDataType, endianType);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a dicom stream, without header and DICM signature
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::buildStream(ptr<streamWriter> pStream, ptr<dataSet> pDataSet, bool bExplicitDataType, streamController::tByteOrdering endianType)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::buildStream");

	for(
		ptr<dataCollectionIterator<dataGroup> > groupsIterator = pDataSet->getDataIterator();
		groupsIterator->isValid();
		groupsIterator->incIterator())
	{
		ptr<dataGroup> pGroup = groupsIterator->getData();
		std::uint16_t groupId = groupsIterator->getId();
		writeGroup(pStream, pGroup, groupId, bExplicitDataType, endianType);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a single data group
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writeGroup(ptr<streamWriter> pDestStream, ptr<dataGroup> pGroup, std::uint16_t groupId, bool bExplicitDataType, streamController::tByteOrdering endianType)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::writeGroup");

	if(groupId == 2)
	{
		bExplicitDataType = true;
		endianType = streamController::lowByteEndian;
	}

	// Calculate the group's length
	///////////////////////////////////////////////////////////
	std::uint32_t groupLength = getGroupLength(pGroup, bExplicitDataType);

	// Write the group's length
	///////////////////////////////////////////////////////////
	static char lengthDataType[] = "UL";

    std::uint16_t adjustedGroupId = pDestStream->adjustEndian(groupId, endianType);;

	std::uint16_t tagId = 0;
	pDestStream->write((std::uint8_t*)&adjustedGroupId, 2);
	pDestStream->write((std::uint8_t*)&tagId, 2);

	if(bExplicitDataType)
	{
		pDestStream->write((std::uint8_t*)&lengthDataType, 2);
        std::uint16_t tagLengthWord = pDestStream->adjustEndian((std::uint16_t)4, endianType);;
        pDestStream->write((std::uint8_t*)&tagLengthWord, 2);
	}
	else
	{
        std::uint32_t tagLengthDword = pDestStream->adjustEndian((std::uint32_t)4, endianType);
        pDestStream->write((std::uint8_t*)&tagLengthDword, 4);
	}

	pDestStream->adjustEndian((std::uint8_t*)&groupLength, 4, endianType);
	pDestStream->write((std::uint8_t*)&groupLength, 4);

	// Write all the tags
	///////////////////////////////////////////////////////////
	for(
		ptr<dataCollectionIterator<data> > pIterator = pGroup->getDataIterator();
		pIterator->isValid();
		pIterator->incIterator())
	{
		std::uint16_t tagId = pIterator->getId();
		if(tagId == 0)
		{
			continue;
		}
		ptr<data> pData = pIterator->getData();
		pDestStream->write((std::uint8_t*)&adjustedGroupId, 2);
		writeTag(pDestStream, pData, tagId, bExplicitDataType, endianType);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a single tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writeTag(ptr<streamWriter> pDestStream, ptr<data> pData, std::uint16_t tagId, bool bExplicitDataType, streamController::tByteOrdering endianType)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::writeTag");

	// Calculate the tag's length
	///////////////////////////////////////////////////////////
	bool bSequence;
	std::uint32_t tagHeader;
	std::uint32_t tagLength = getTagLength(pData, bExplicitDataType, &tagHeader, &bSequence);

	// Prepare the identifiers for the sequence (adjust the
	//  endian)
	///////////////////////////////////////////////////////////
	std::uint16_t sequenceItemGroup = 0xfffe;
	std::uint16_t sequenceItemDelimiter = 0xe000;
	std::uint16_t sequenceItemEnd = 0xe0dd;
	pDestStream->adjustEndian((std::uint8_t*)&sequenceItemGroup, 2, endianType);
	pDestStream->adjustEndian((std::uint8_t*)&sequenceItemDelimiter, 2, endianType);
	pDestStream->adjustEndian((std::uint8_t*)&sequenceItemEnd, 2, endianType);

	// Check the data type
	///////////////////////////////////////////////////////////
	std::string dataType = pData->getDataType();
	if(!(dicomDictionary::getDicomDictionary()->isDataTypeValid(dataType)))
	{
		if(pData->getDataSet(0) != 0)
		{
			dataType = "SQ";
		}
		else
		{
			dataType = "OB";
		}
	}

	// Adjust the tag id endian and write it
	///////////////////////////////////////////////////////////
	std::uint16_t adjustedTagId = tagId;
	pDestStream->adjustEndian((std::uint8_t*)&adjustedTagId, 2, endianType);
	pDestStream->write((std::uint8_t*)&adjustedTagId, 2);

	// Write the data type if it is explicit
	///////////////////////////////////////////////////////////
	if(bExplicitDataType)
	{
		pDestStream->write((std::uint8_t*)(dataType.c_str()), 2);

		std::uint16_t tagLengthWord = (std::uint16_t)tagLength;

		if(bSequence | dicomDictionary::getDicomDictionary()->getLongLength(dataType))
		{
			std::uint32_t tagLengthDWord = bSequence ? 0xffffffff : tagLength;
			tagLengthWord = 0;
			pDestStream->adjustEndian((std::uint8_t*)&tagLengthDWord, 4, endianType);
			pDestStream->write((std::uint8_t*)&tagLengthWord, 2);
			pDestStream->write((std::uint8_t*)&tagLengthDWord, 4);
		}
		else
		{
			pDestStream->adjustEndian((std::uint8_t*)&tagLengthWord, 2, endianType);
			pDestStream->write((std::uint8_t*)&tagLengthWord, 2);
		}
	}
	else
	{
		std::uint32_t tagLengthDword = bSequence ? 0xffffffff : tagLength;
		pDestStream->adjustEndian((std::uint8_t*)&tagLengthDword, 4, endianType);
		pDestStream->write((std::uint8_t*)&tagLengthDword, 4);
	}

	// Write all the buffers or datasets
	///////////////////////////////////////////////////////////
	for(std::uint32_t scanBuffers = 0; ; ++scanBuffers)
	{
		ptr<handlers::dataHandlerRaw> pDataHandlerRaw = pData->getDataHandlerRaw(scanBuffers, false, "");
		if(pDataHandlerRaw != 0)
		{
			std::uint32_t wordSize = dicomDictionary::getDicomDictionary()->getWordSize(dataType);
			std::uint32_t bufferSize = pDataHandlerRaw->getSize();

			// write the sequence item header
			///////////////////////////////////////////////////////////
			if(bSequence)
			{
				pDestStream->write((std::uint8_t*)&sequenceItemGroup, 2);
				pDestStream->write((std::uint8_t*)&sequenceItemDelimiter, 2);
				std::uint32_t sequenceItemLength = bufferSize;
				pDestStream->adjustEndian((std::uint8_t*)&sequenceItemLength, 4, endianType);
				pDestStream->write((std::uint8_t*)&sequenceItemLength, 4);
			}

			if(bufferSize == 0)
			{
				continue;
			}

			// Adjust the buffer's endian
			///////////////////////////////////////////////////////////
			if(wordSize > 1)
			{
                std::vector<std::uint8_t> tempBuffer((size_t)bufferSize);
                ::memcpy(tempBuffer.data(), pDataHandlerRaw->getMemoryBuffer(), pDataHandlerRaw->getSize());
                streamController::adjustEndian(tempBuffer.data(), wordSize, endianType, bufferSize / wordSize);
                pDestStream->write(tempBuffer.data(), bufferSize);
				continue;
			}

			pDestStream->write((std::uint8_t*)pDataHandlerRaw->getMemoryBuffer(), bufferSize);
			continue;
		}

		// Write a nested dataset
		///////////////////////////////////////////////////////////
		ptr<dataSet> pDataSet = pData->getDataSet(scanBuffers);
		if(pDataSet == 0)
		{
			break;
		}

		// Remember the position at which the item has been written
		///////////////////////////////////////////////////////////
		pDataSet->setItemOffset(pDestStream->getControlledStreamPosition());

		// write the sequence item header
		///////////////////////////////////////////////////////////
		pDestStream->write((std::uint8_t*)&sequenceItemGroup, 2);
		pDestStream->write((std::uint8_t*)&sequenceItemDelimiter, 2);
		std::uint32_t sequenceItemLength = getDataSetLength(pDataSet, bExplicitDataType);
		pDestStream->adjustEndian((std::uint8_t*)&sequenceItemLength, 4, endianType);
		pDestStream->write((std::uint8_t*)&sequenceItemLength, 4);

		// write the dataset
		///////////////////////////////////////////////////////////
		buildStream(pDestStream, pDataSet, bExplicitDataType, endianType);
	}

	// write the sequence item end marker
	///////////////////////////////////////////////////////////
	if(bSequence)
	{
		pDestStream->write((std::uint8_t*)&sequenceItemGroup, 2);
		pDestStream->write((std::uint8_t*)&sequenceItemEnd, 2);
		std::uint32_t sequenceItemLength = 0;
		pDestStream->write((std::uint8_t*)&sequenceItemLength, 4);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomCodec::getTagLength(ptr<data> pData, bool bExplicitDataType, std::uint32_t* pHeaderLength, bool *pbSequence)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::getTagLength");

	std::string dataType = pData->getDataType();
	*pbSequence = (dataType == "SQ");
	std::uint32_t numberOfElements = 0;
	std::uint32_t totalLength = 0;
	for(std::uint32_t scanBuffers = 0; ; ++scanBuffers, ++numberOfElements)
	{
		ptr<dataSet> pDataSet = pData->getDataSet(scanBuffers);
		if(pDataSet != 0)
		{
			totalLength += getDataSetLength(pDataSet, bExplicitDataType);
			totalLength += 8; // item tag and item length
			*pbSequence = true;
			continue;
		}
		if(!pData->bufferExists(scanBuffers))
		{
			break;
		}
		totalLength += pData->getBufferSize(scanBuffers);
	}

	(*pbSequence) |= (numberOfElements > 1);

	// Find the tag type
	bool bLongLength = dicomDictionary::getDicomDictionary()->getLongLength(dataType);

	*pHeaderLength = 8;
	if((bLongLength || (*pbSequence)) && bExplicitDataType)
	{
		(*pHeaderLength) +=4;
	}

	if(*pbSequence)
	{
		totalLength += (numberOfElements+1) * 8;
	}

	return totalLength;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the group's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomCodec::getGroupLength(ptr<dataGroup> pDataGroup, bool bExplicitDataType)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::getGroupLength");

	ptr<dataCollectionIterator<data> > pIterator = pDataGroup->getDataIterator();

	std::uint32_t totalLength = 0;

	for(;pIterator->isValid(); pIterator->incIterator())
	{
		if(pIterator->getId() == 0)
		{
			continue;
		}

		std::uint32_t tagHeaderLength;
		bool bSequence;
		totalLength += getTagLength(pIterator->getData(), bExplicitDataType, &tagHeaderLength, &bSequence);
		totalLength += tagHeaderLength;
	}

	return totalLength;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the dataset's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomCodec::getDataSetLength(ptr<dataSet> pDataSet, bool bExplicitDataType)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::getDataSetLength");

	ptr<dataCollectionIterator<dataGroup> > pIterator = pDataSet->getDataIterator();

	std::uint32_t totalLength = 0;

	while(pIterator->isValid())
	{
		totalLength += getGroupLength(pIterator->getData(), bExplicitDataType);
		totalLength += 4; // Add space for the tag 0
		if(bExplicitDataType) // Add space for the data type
		{
			totalLength += 2;
		}
		totalLength += 2; // Add space for the tag's length
		totalLength += 4; // Add space for the group's length

		pIterator->incIterator();
	}

	return totalLength;

	PUNTOEXE_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a DICOM stream and fill the dataset with the
//  DICOM's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::readStream(ptr<streamReader> pStream, ptr<dataSet> pDataSet, std::uint32_t maxSizeBufferLoad /* = 0xffffffff */)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::readStream");

	// Save the starting position
	///////////////////////////////////////////////////////////
	std::uint32_t position=pStream->position();

	// This flag signals a failure
	///////////////////////////////////////////////////////////
	bool bFailed=false;

	// Read the old dicom signature (NEMA)
	///////////////////////////////////////////////////////////
	std::uint8_t oldDicomSignature[8];

	try
	{
		pStream->read(oldDicomSignature, 8);
	}
	catch(streamExceptionEOF&)
	{
		PUNTOEXE_THROW(codecExceptionWrongFormat, "detected a wrong format");
	}

	// Skip the first 128 bytes (8 already skipped)
	///////////////////////////////////////////////////////////
	pStream->seek(120, true);

	// Read the DICOM signature (DICM)
	///////////////////////////////////////////////////////////
	std::uint8_t dicomSignature[4];
	pStream->read(dicomSignature, 4);
	// Check the DICM signature
	///////////////////////////////////////////////////////////
	const char* checkSignature="DICM";
	if(::memcmp(dicomSignature, checkSignature, 4) != 0)
	{
		bFailed=true;
	}

	bool bExplicitDataType = true;
	streamController::tByteOrdering endianType=streamController::lowByteEndian;
	if(bFailed)
	{
		// Tags 0x8 and 0x2 are accepted in the begin of the file
		///////////////////////////////////////////////////////////
		if(
			(oldDicomSignature[0]!=0x8 && oldDicomSignature[0]!=0x2) ||
			oldDicomSignature[1]!=0x0 ||
			oldDicomSignature[3]!=0x0)
		{
			PUNTOEXE_THROW(codecExceptionWrongFormat, "detected a wrong format (checked old NEMA signature)");
		}

		// Go back to the beginning of the file
		///////////////////////////////////////////////////////////
		pStream->seek((std::int32_t)position);

		// Set "explicit data type" to true if a valid data type
                //  is found
		///////////////////////////////////////////////////////////
                std::string firstDataType;
                firstDataType.push_back(oldDicomSignature[4]);
                firstDataType.push_back(oldDicomSignature[5]);
                bExplicitDataType = dicomDictionary::getDicomDictionary()->isDataTypeValid(firstDataType);
	}

	// Signature OK. Now scan all the tags.
	///////////////////////////////////////////////////////////
	parseStream(pStream, pDataSet, bExplicitDataType, endianType, maxSizeBufferLoad);

	PUNTOEXE_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse a Dicom stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::parseStream(ptr<streamReader> pStream,
							 ptr<dataSet> pDataSet,
							 bool bExplicitDataType,
							 streamController::tByteOrdering endianType,
							 std::uint32_t maxSizeBufferLoad /* = 0xffffffff */,
							 std::uint32_t subItemLength /* = 0xffffffff */,
							 std::uint32_t* pReadSubItemLength /* = 0 */,
							 std::uint32_t depth)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::parseStream");

	if(depth > IMEBRA_DATASET_MAX_DEPTH)
	{
		PUNTOEXE_THROW(dicomCodecExceptionDepthLimitReached, "Depth for embedded dataset reached");
	}

	std::uint16_t tagId;
	std::uint16_t tagSubId;
	std::uint16_t tagLengthWord;
	std::uint32_t tagLengthDWord;

	// Used to calculate the group order
	///////////////////////////////////////////////////////////
	std::uint16_t order = 0;
	std::uint16_t lastGroupId = 0;
	std::uint16_t lastTagId = 0;

	std::uint32_t tempReadSubItemLength = 0; // used when the last parameter is not defined
	char       tagType[3];
	bool       bStopped = false;
	bool       bFirstTag = (pReadSubItemLength == 0);
	bool       bCheckTransferSyntax = bFirstTag;
	short      wordSize;

	tagType[2] = 0;

	if(pReadSubItemLength == 0)
	{
		pReadSubItemLength = &tempReadSubItemLength;
	}
	*pReadSubItemLength = 0;

	///////////////////////////////////////////////////////////
	//
	// Read all the tags
	//
	///////////////////////////////////////////////////////////
	while(!bStopped && !pStream->endReached() && (*pReadSubItemLength < subItemLength))
	{
		// Get the tag's ID
		///////////////////////////////////////////////////////////
		pStream->read((std::uint8_t*)&tagId, sizeof(tagId));
		pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);
		(*pReadSubItemLength) += sizeof(tagId);

		// Check for EOF
		///////////////////////////////////////////////////////////
		if(pStream->endReached())
		{
			break;
		}

		// Check the byte order
		///////////////////////////////////////////////////////////
		if(bFirstTag && tagId==0x0200)
		{
			// Reverse the last adjust
			pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);

			// Fix the byte adjustment
            endianType=streamController::highByteEndian;

			// Redo the byte adjustment
			pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);
		}

		// If this tag's id is not 0x0002, then load the
		//  transfer syntax and set the byte endian.
		///////////////////////////////////////////////////////////
		if(tagId!=0x0002 && bCheckTransferSyntax)
		{
			// Reverse the last adjust
			pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);

			std::wstring transferSyntax=pDataSet->getUnicodeString(0x0002, 0x0, 0x0010, 0x0);

			if(transferSyntax == L"1.2.840.10008.1.2.2")
				endianType=streamController::highByteEndian;
			if(transferSyntax == L"1.2.840.10008.1.2")
				bExplicitDataType=false;

			bCheckTransferSyntax=false;

			// Redo the byte adjustment
			pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);
		}

		// The first tag's ID has been read
		///////////////////////////////////////////////////////////
		bFirstTag=false;

		// Set the word's length to the default value
		///////////////////////////////////////////////////////////
		wordSize = 1;

		// Get the tag's sub ID
		///////////////////////////////////////////////////////////
		pStream->read((std::uint8_t*)&tagSubId, sizeof(tagSubId));
		pStream->adjustEndian((std::uint8_t*)&tagSubId, sizeof(tagSubId), endianType);
		(*pReadSubItemLength) += sizeof(tagSubId);

		// Check for the end of the dataset
		///////////////////////////////////////////////////////////
		if(tagId==0xfffe && tagSubId==0xe00d)
		{
			// skip the tag's length and exit
			std::uint32_t dummyDWord;
			pStream->read((std::uint8_t*)&dummyDWord, 4);
            (*pReadSubItemLength) += 4;
			break;
		}

		//
		// Explicit data type
		//
		///////////////////////////////////////////////////////////
		if(bExplicitDataType && tagId!=0xfffe)
		{
			// Get the tag's type
			///////////////////////////////////////////////////////////
			pStream->read((std::uint8_t*)tagType, 2);
			(*pReadSubItemLength) += 2;

			// Get the tag's length
			///////////////////////////////////////////////////////////
			pStream->read((std::uint8_t*)&tagLengthWord, sizeof(tagLengthWord));
			pStream->adjustEndian((std::uint8_t*)&tagLengthWord, sizeof(tagLengthWord), endianType);
			(*pReadSubItemLength) += sizeof(tagLengthWord);

			// The data type is valid
			///////////////////////////////////////////////////////////
			if(dicomDictionary::getDicomDictionary()->isDataTypeValid(tagType))
			{
				tagLengthDWord=(std::uint32_t)tagLengthWord;
				wordSize = (short)dicomDictionary::getDicomDictionary()->getWordSize(tagType);
				if(dicomDictionary::getDicomDictionary()->getLongLength(tagType))
				{
					pStream->read((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord));
					pStream->adjustEndian((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord), endianType);
					(*pReadSubItemLength) += sizeof(tagLengthDWord);
				}
			}

			// The data type is not valid. Switch to implicit data type
			///////////////////////////////////////////////////////////
			else
			{
				if(endianType == streamController::lowByteEndian)
					tagLengthDWord=(((std::uint32_t)tagLengthWord)<<16) | ((std::uint32_t)tagType[0]) | (((std::uint32_t)tagType[1])<<8);
				else
					tagLengthDWord=(std::uint32_t)tagLengthWord | (((std::uint32_t)tagType[0])<<24) | (((std::uint32_t)tagType[1])<<16);
			}


		} // End of the explicit data type read block


		///////////////////////////////////////////////////////////
		//
		// Implicit data type
		//
		///////////////////////////////////////////////////////////
		else
		{
			// Get the tag's length
			///////////////////////////////////////////////////////////
			pStream->read((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord));
			pStream->adjustEndian((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord), endianType);
			(*pReadSubItemLength) += sizeof(tagLengthDWord);
		} // End of the implicit data type read block


		///////////////////////////////////////////////////////////
		//
		// Find the default data type and the tag's word's size
		//
		///////////////////////////////////////////////////////////
		if((!bExplicitDataType || tagId==0xfffe))
		{
			// Group length. Data type is always UL
			///////////////////////////////////////////////////////////
			if(tagSubId == 0)
			{
				tagType[0]='U';
				tagType[1]='L';
			}
			else
			{
				tagType[0]=tagType[1] = 0;
				std::string defaultType=pDataSet->getDefaultDataType(tagId, tagSubId);
				if(defaultType.length()==2L)
				{
					tagType[0]=defaultType[0];
					tagType[1]=defaultType[1];

					wordSize = (short)dicomDictionary::getDicomDictionary()->getWordSize(tagType);
				}
			}
		}

		// Check for the end of a sequence
		///////////////////////////////////////////////////////////
		if(tagId==0xfffe && tagSubId==0xe0dd)
		{
			break;
		}

		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////
		//
		//
		// Read the tag's buffer
		//
		//
		///////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
		//
		// Adjust the order when multiple groups with the same
		//  id are present.
		//
		///////////////////////////////////////////////////////////
		if(tagId<=lastGroupId && tagSubId<=lastTagId)
		{
			++order;
		}
		else
		{
			if(tagId>lastGroupId)
			{
				order = 0;
			}
		}
		lastGroupId=tagId;
		lastTagId=tagSubId;

		if(tagLengthDWord != 0xffffffff && ::memcmp(tagType, "SQ", 2) != 0)
		{
			(*pReadSubItemLength) += readTag(pStream, pDataSet, tagLengthDWord, tagId, order, tagSubId, tagType, endianType, wordSize, 0, maxSizeBufferLoad);
			continue;
		}

		///////////////////////////////////////////////////////////
		//
		// We are within an undefined-length tag or a sequence
		//
		///////////////////////////////////////////////////////////

		// Parse all the sequence's items
		///////////////////////////////////////////////////////////
		std::uint16_t subItemGroupId;
		std::uint16_t subItemTagId;
		std::uint32_t sequenceItemLength;
		std::uint32_t bufferId = 0;
		while(tagLengthDWord && !pStream->endReached())
		{
			// Remember the item's position (used by DICOMDIR
			//  structures)
			///////////////////////////////////////////////////////////
			std::uint32_t itemOffset(pStream->getControlledStreamPosition());

			// Read the sequence item's group
			///////////////////////////////////////////////////////////
			pStream->read((std::uint8_t*)&subItemGroupId, sizeof(subItemGroupId));
			pStream->adjustEndian((std::uint8_t*)&subItemGroupId, sizeof(subItemGroupId), endianType);
			(*pReadSubItemLength) += sizeof(subItemGroupId);

			// Read the sequence item's id
			///////////////////////////////////////////////////////////
			pStream->read((std::uint8_t*)&subItemTagId, sizeof(subItemTagId));
			pStream->adjustEndian((std::uint8_t*)&subItemTagId, sizeof(subItemTagId), endianType);
			(*pReadSubItemLength) += sizeof(subItemTagId);

			// Read the sequence item's length
			///////////////////////////////////////////////////////////
			pStream->read((std::uint8_t*)&sequenceItemLength, sizeof(sequenceItemLength));
			pStream->adjustEndian((std::uint8_t*)&sequenceItemLength, sizeof(sequenceItemLength), endianType);
			(*pReadSubItemLength) += sizeof(sequenceItemLength);

			if(tagLengthDWord!=0xffffffff)
			{
				tagLengthDWord-=8;
			}

			// check for the end of the undefined length sequence
			///////////////////////////////////////////////////////////
			if(subItemGroupId==0xfffe && subItemTagId==0xe0dd)
			{
				break;
			}

			///////////////////////////////////////////////////////////
			// Parse a sub element
			///////////////////////////////////////////////////////////
			if((sequenceItemLength == 0xffffffff) || (::memcmp(tagType, "SQ", 2) == 0))
			{
				ptr<dataSet> sequenceDataSet(new dataSet);
				sequenceDataSet->setItemOffset(itemOffset);
				std::uint32_t effectiveLength(0);
				parseStream(pStream, sequenceDataSet, bExplicitDataType, endianType, maxSizeBufferLoad, sequenceItemLength, &effectiveLength, depth + 1);
				(*pReadSubItemLength) += effectiveLength;
				if(tagLengthDWord!=0xffffffff)
					tagLengthDWord-=effectiveLength;
				ptr<data> sequenceTag=pDataSet->getTag(tagId, 0x0, tagSubId, true);
				sequenceTag->setDataSet(bufferId, sequenceDataSet);
				++bufferId;

				continue;
			}

			///////////////////////////////////////////////////////////
			// Read a buffer's element
			///////////////////////////////////////////////////////////
			sequenceItemLength=readTag(pStream, pDataSet, sequenceItemLength, tagId, order, tagSubId, tagType, endianType, wordSize, bufferId++, maxSizeBufferLoad);
			(*pReadSubItemLength) += sequenceItemLength;
			if(tagLengthDWord!=0xffffffff)
			{
				tagLengthDWord -= sequenceItemLength;
			}
		}

	} // End of the tags-read block

	PUNTOEXE_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a DICOM raw or RLE image from a dicom structure
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
ptr<image> dicomCodec::getImage(ptr<dataSet> pData, ptr<streamReader> pStream, std::string dataType)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::getImage");

	streamReader* pSourceStream = pStream.get();

	// Check for RLE compression
	///////////////////////////////////////////////////////////
	std::wstring transferSyntax = pData->getUnicodeString(0x0002, 0x0, 0x0010, 0x0);
	bool bRleCompressed = (transferSyntax == L"1.2.840.10008.1.2.5");

	// Check for color space and subsampled channels
	///////////////////////////////////////////////////////////
	std::wstring colorSpace=pData->getUnicodeString(0x0028, 0x0, 0x0004, 0x0);

	// Retrieve the number of planes
	///////////////////////////////////////////////////////////
	std::uint8_t channelsNumber=(std::uint8_t)pData->getUnsignedLong(0x0028, 0x0, 0x0002, 0x0);

	// Adjust the colorspace and the channels number for old
	//  NEMA files that don't specify those data
	///////////////////////////////////////////////////////////
	if(colorSpace.empty() && (channelsNumber == 0 || channelsNumber == 1))
	{
		colorSpace = L"MONOCHROME2";
		channelsNumber = 1;
	}

	if(colorSpace.empty() && channelsNumber == 3)
	{
		colorSpace = L"RGB";
	}

	// Retrieve the image's size
	///////////////////////////////////////////////////////////
	std::uint32_t imageSizeX=pData->getUnsignedLong(0x0028, 0x0, 0x0011, 0x0);
	std::uint32_t imageSizeY=pData->getUnsignedLong(0x0028, 0x0, 0x0010, 0x0);

    if(
            imageSizeX > codecFactory::getCodecFactory()->getMaximumImageWidth() ||
            imageSizeY > codecFactory::getCodecFactory()->getMaximumImageHeight())
    {
        PUNTOEXE_THROW(codecExceptionImageTooBig, "The factory settings prevented the loading of this image. Consider using codecFactory::setMaximumImageSize() to modify the settings");
    }

    if((imageSizeX == 0) || (imageSizeY == 0))
	{
		PUNTOEXE_THROW(codecExceptionCorruptedFile, "The size tags are not available");
	}

	// Check for interleaved planes.
	///////////////////////////////////////////////////////////
	bool bInterleaved(pData->getUnsignedLong(0x0028, 0x0, 0x0006, 0x0)==0x0);

	// Check for 2's complement
	///////////////////////////////////////////////////////////
	bool b2Complement=pData->getUnsignedLong(0x0028, 0x0, 0x0103, 0x0)!=0x0;

	// Retrieve the allocated/stored/high bits
	///////////////////////////////////////////////////////////
	std::uint8_t allocatedBits=(std::uint8_t)pData->getUnsignedLong(0x0028, 0x0, 0x0100, 0x0);
	std::uint8_t storedBits=(std::uint8_t)pData->getUnsignedLong(0x0028, 0x0, 0x0101, 0x0);
	std::uint8_t highBit=(std::uint8_t)pData->getUnsignedLong(0x0028, 0x0, 0x0102, 0x0);
	if(highBit<storedBits-1)
		highBit=storedBits-1;


	// If the chrominance channels are subsampled, then find
	//  the right image's size
	///////////////////////////////////////////////////////////
	bool bSubSampledY=channelsNumber>0x1 && transforms::colorTransforms::colorTransformsFactory::isSubsampledY(colorSpace);
	bool bSubSampledX=channelsNumber>0x1 && transforms::colorTransforms::colorTransformsFactory::isSubsampledX(colorSpace);

	// Create an image
	///////////////////////////////////////////////////////////
	image::bitDepth depth;
	if(b2Complement)
	{
		if(highBit >= 16)
		{
			depth = image::depthS32;
		}
		else if(highBit >= 8)
		{
			depth = image::depthS16;
		}
		else
		{
			depth = image::depthS8;
		}
	}
	else
	{
		if(highBit >= 16)
		{
			depth = image::depthU32;
		}
		else if(highBit >= 8)
		{
			depth = image::depthU16;
		}
		else
		{
			depth = image::depthU8;
		}
	}

	ptr<image> pImage(new image);
	ptr<handlers::dataHandlerNumericBase> handler = pImage->create(imageSizeX, imageSizeY, depth, colorSpace, highBit);
	std::uint32_t tempChannelsNumber = pImage->getChannelsNumber();

	if(handler == 0 || tempChannelsNumber != channelsNumber)
	{
		PUNTOEXE_THROW(codecExceptionCorruptedFile, "Cannot allocate the image's buffer");
	}

	// Allocate the dicom channels
	///////////////////////////////////////////////////////////
	allocChannels(channelsNumber, imageSizeX, imageSizeY, bSubSampledX, bSubSampledY);

	std::uint32_t mask( (std::uint32_t)0x1 << highBit );
	mask <<= 1;
	--mask;
	mask-=((std::uint32_t)0x1<<(highBit+1-storedBits))-1;

	//
	// The image is not compressed
	//
	///////////////////////////////////////////////////////////
	if(!bRleCompressed)
	{
		std::uint8_t wordSizeBytes= (dataType=="OW") ? 2 : 1;

		// The planes are interleaved
		///////////////////////////////////////////////////////////
		if(bInterleaved && channelsNumber != 1)
		{
			readUncompressedInterleaved(
				channelsNumber,
				bSubSampledX,
				bSubSampledY,
				pSourceStream,
				wordSizeBytes,
				allocatedBits,
				mask);
		}
		else
		{
			readUncompressedNotInterleaved(
				channelsNumber,
				pSourceStream,
				wordSizeBytes,
				allocatedBits,
				mask);
		}
	}

	//
	// The image is RLE compressed
	//
	///////////////////////////////////////////////////////////
	else
	{
		if(bSubSampledX || bSubSampledY)
		{
			PUNTOEXE_THROW(codecExceptionCorruptedFile, "Cannot read subsampled RLE images");
		}

		readRLECompressed(imageSizeX, imageSizeY, channelsNumber, pSourceStream, allocatedBits, mask, bInterleaved);

	} // ...End of RLE decoding

	// Adjust b2complement buffers
	///////////////////////////////////////////////////////////
	if(b2Complement)
	{
		std::int32_t checkSign = (std::int32_t)0x1<<highBit;
		std::int32_t orMask = ((std::int32_t)-1)<<highBit;

		for(size_t adjChannels = 0; adjChannels < m_channels.size(); ++adjChannels)
		{
			std::int32_t* pAdjBuffer = m_channels[adjChannels]->m_pBuffer;
			std::uint32_t adjSize = m_channels[adjChannels]->m_bufferSize;
			while(adjSize != 0)
			{
				if(*pAdjBuffer & checkSign)
				{
					*pAdjBuffer |= orMask;
				}
				++pAdjBuffer;
				--adjSize;
			}
		}
	}


	// Copy the dicom channels into the image
	///////////////////////////////////////////////////////////
	std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
	std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;
	for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
	{
		ptrChannel dicomChannel = m_channels[copyChannels];
		handler->copyFromInt32Interleaved(
			dicomChannel->m_pBuffer,
			maxSamplingFactorX /dicomChannel->m_samplingFactorX,
			maxSamplingFactorY /dicomChannel->m_samplingFactorY,
			0, 0,
			dicomChannel->m_sizeX * maxSamplingFactorX / dicomChannel->m_samplingFactorX,
			dicomChannel->m_sizeY * maxSamplingFactorY / dicomChannel->m_samplingFactorY,
			copyChannels,
			imageSizeX,
			imageSizeY,
			channelsNumber);
	}

	// Return OK
	///////////////////////////////////////////////////////////
	return pImage;

	PUNTOEXE_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate the channels used to read/write an image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::allocChannels(std::uint32_t channelsNumber, std::uint32_t sizeX, std::uint32_t sizeY, bool bSubSampledX, bool bSubSampledY)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::allocChannels");

	if(bSubSampledX && (sizeX & 0x1) != 0)
	{
		++sizeX;
	}

	if(bSubSampledY && (sizeY & 0x1) != 0)
	{
		++sizeY;
	}

	m_channels.resize(channelsNumber);
	for(std::uint32_t channelNum = 0; channelNum < channelsNumber; ++channelNum)
	{
		ptrChannel newChannel(new channel);
		std::uint32_t channelSizeX = sizeX;
		std::uint32_t channelSizeY = sizeY;
		std::uint32_t samplingFactorX = 1;
		std::uint32_t samplingFactorY = 1;
		if(channelNum != 0)
		{
			if(bSubSampledX)
			{
				channelSizeX >>= 1;
			}
			if(bSubSampledY)
			{
				channelSizeY >>= 1;
			}
		}
		else
		{
			if(bSubSampledX)
			{
				++samplingFactorX;
			}
			if(bSubSampledY)
			{
				++samplingFactorY;
			}
		}
		newChannel->allocate(channelSizeX, channelSizeY);

		if(channelNum == 0)
		newChannel->m_samplingFactorX = samplingFactorX;
		newChannel->m_samplingFactorY = samplingFactorY;

		m_channels[channelNum] = newChannel;
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read an uncompressed interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::readUncompressedInterleaved(
	std::uint32_t channelsNumber,
	bool bSubSampledX,
	bool bSubSampledY,
	streamReader* pSourceStream,
	std::uint8_t wordSizeBytes,
	std::uint8_t allocatedBits,
	std::uint32_t mask
	)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::readUncompressedInterleaved");

	std::uint8_t  bitPointer=0x0;

    std::vector<std::int32_t*> channelsMemory(m_channels.size());
	for(size_t copyChannelsPntr = 0; copyChannelsPntr < m_channels.size(); ++copyChannelsPntr)
	{
		channelsMemory[copyChannelsPntr] = m_channels[copyChannelsPntr]->m_pBuffer;
	}

	// No subsampling here
	///////////////////////////////////////////////////////////
	if(!bSubSampledX && !bSubSampledY)
	{
		std::uint8_t readBuffer[4];
		for(std::uint32_t totalSize = m_channels[0]->m_bufferSize; totalSize != 0; --totalSize)
		{
			for(std::uint32_t scanChannels = 0; scanChannels != channelsNumber; ++scanChannels)
			{
                            readPixel(pSourceStream, channelsMemory[scanChannels]++, 1, &bitPointer, readBuffer, wordSizeBytes, allocatedBits, mask);
			}
		}
		return;
	}

        std::uint32_t numValuesPerBlock(channelsNumber);
        if(bSubSampledX)
        {
            ++numValuesPerBlock;
        }
        if(bSubSampledY)
        {
            numValuesPerBlock += 2;
        }
        std::vector<std::int32_t> readBlockValues((size_t)numValuesPerBlock);

        // Read the subsampled channels.
	// Find the number of blocks to read
	///////////////////////////////////////////////////////////
	std::uint32_t adjSizeX = m_channels[0]->m_sizeX;
	std::uint32_t adjSizeY = m_channels[0]->m_sizeY;

	std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
	std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;

	ptr<memory> readBuffer(memoryPool::getMemoryPool()->getMemory(numValuesPerBlock * ((7+allocatedBits) >> 3)));

	// Read all the blocks
	///////////////////////////////////////////////////////////
	for(
		std::uint32_t numBlocks = (adjSizeX / maxSamplingFactorX) * (adjSizeY / maxSamplingFactorY);
		numBlocks != 0;
		--numBlocks)
	{
        std::int32_t* readBlockValuesPtr(&(readBlockValues[0]));
        readPixel(pSourceStream, readBlockValuesPtr, numValuesPerBlock, &bitPointer, readBuffer->data(), wordSizeBytes, allocatedBits, mask);

		// Read channel 0 (not subsampled)
		///////////////////////////////////////////////////////////
		*(channelsMemory[0]++) = *readBlockValuesPtr++;
		if(bSubSampledX)
		{
			*(channelsMemory[0]++) = *readBlockValuesPtr++;
		}
		if(bSubSampledY)
		{
			*(channelsMemory[0]+adjSizeX-2) = *readBlockValuesPtr++;
			*(channelsMemory[0]+adjSizeX-1) = *readBlockValuesPtr++;
		}
		// Read channels 1... (subsampled)
		///////////////////////////////////////////////////////////
		for(std::uint32_t scanSubSampled = 1; scanSubSampled < channelsNumber; ++scanSubSampled)
		{
			*(channelsMemory[scanSubSampled]++) = *readBlockValuesPtr++;
		}
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write an uncompressed interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writeUncompressedInterleaved(
	std::uint32_t channelsNumber,
	bool bSubSampledX,
	bool bSubSampledY,
	streamWriter* pDestStream,
	std::uint8_t wordSizeBytes,
	std::uint8_t allocatedBits,
	std::uint32_t mask
	)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::writeUncompressedInterleaved");

	std::uint8_t  bitPointer=0x0;

    std::vector<std::int32_t*> channelsMemory(m_channels.size());
	for(size_t copyChannelsPntr = 0; copyChannelsPntr < m_channels.size(); ++copyChannelsPntr)
	{
		channelsMemory[copyChannelsPntr] = m_channels[copyChannelsPntr]->m_pBuffer;
	}

	// No subsampling here
	///////////////////////////////////////////////////////////
	if(!bSubSampledX && !bSubSampledY)
	{
		for(std::uint32_t totalSize = m_channels[0]->m_bufferSize; totalSize != 0; --totalSize)
		{
			for(std::uint32_t scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)
			{
				writePixel(pDestStream, *(channelsMemory[scanChannels]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
			}
		}
		flushUnwrittenPixels(pDestStream, &bitPointer, wordSizeBytes);
		return;
	}

	// Write the subsampled channels.
	// Find the number of blocks to write
	///////////////////////////////////////////////////////////
	std::uint32_t adjSizeX = m_channels[0]->m_sizeX;
	std::uint32_t adjSizeY = m_channels[0]->m_sizeY;

	std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
	std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;

	// Write all the blocks
	///////////////////////////////////////////////////////////
	for(
		std::uint32_t numBlocks = (adjSizeX / maxSamplingFactorX) * (adjSizeY / maxSamplingFactorY);
		numBlocks != 0;
		--numBlocks)
	{
		// Write channel 0 (not subsampled)
		///////////////////////////////////////////////////////////
		writePixel(pDestStream, *(channelsMemory[0]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
		if(bSubSampledX)
		{
			writePixel(pDestStream, *(channelsMemory[0]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
		}
		if(bSubSampledY)
		{
			writePixel(pDestStream, *(channelsMemory[0]+adjSizeX-2), &bitPointer, wordSizeBytes, allocatedBits, mask);
			writePixel(pDestStream, *(channelsMemory[0]+adjSizeX-1), &bitPointer, wordSizeBytes, allocatedBits, mask);
		}
		// Write channels 1... (subsampled)
		///////////////////////////////////////////////////////////
		for(std::uint32_t scanSubSampled = 1; scanSubSampled < channelsNumber; ++scanSubSampled)
		{
			writePixel(pDestStream, *(channelsMemory[scanSubSampled]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
		}
	}

	flushUnwrittenPixels(pDestStream, &bitPointer, wordSizeBytes);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read an uncompressed not interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::readUncompressedNotInterleaved(
	std::uint32_t channelsNumber,
	streamReader* pSourceStream,
	std::uint8_t wordSizeBytes,
	std::uint8_t allocatedBits,
	std::uint32_t mask
	)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::readUncompressedNotInterleaved");

	std::uint8_t  bitPointer=0x0;

	ptr<memory> readBuffer;
	std::uint32_t lastBufferSize(0);

	// Read all the pixels
	///////////////////////////////////////////////////////////
	for(std::uint32_t channel = 0; channel < channelsNumber; ++channel)
	{
		if(m_channels[channel]->m_bufferSize != lastBufferSize)
		{
			lastBufferSize = m_channels[channel]->m_bufferSize;
			readBuffer = memoryPool::getMemoryPool()->getMemory(lastBufferSize * ((7+allocatedBits) >> 3));
		}
		std::int32_t* pMemoryDest = m_channels[channel]->m_pBuffer;
        readPixel(pSourceStream, pMemoryDest, m_channels[channel]->m_bufferSize, &bitPointer, readBuffer->data(), wordSizeBytes, allocatedBits, mask);
	}

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write an uncompressed not interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writeUncompressedNotInterleaved(
	std::uint32_t channelsNumber,
	streamWriter* pDestStream,
	std::uint8_t wordSizeBytes,
	std::uint8_t allocatedBits,
	std::uint32_t mask
	)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::writeUncompressedNotInterleaved");

	std::uint8_t  bitPointer=0x0;

	// Write all the pixels
	///////////////////////////////////////////////////////////
	for(std::uint32_t channel = 0; channel < channelsNumber; ++channel)
	{
		std::int32_t* pMemoryDest = m_channels[channel]->m_pBuffer;
		for(std::uint32_t scanPixels = m_channels[channel]->m_bufferSize; scanPixels != 0; --scanPixels)
		{
			writePixel(pDestStream, *pMemoryDest++, &bitPointer, wordSizeBytes, allocatedBits, mask);
		}
	}
	flushUnwrittenPixels(pDestStream, &bitPointer, wordSizeBytes);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a RLE compressed image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writeRLECompressed(
	std::uint32_t imageSizeX,
	std::uint32_t imageSizeY,
	std::uint32_t channelsNumber,
	streamWriter* pDestStream,
	std::uint8_t allocatedBits,
	std::uint32_t mask
	)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::writeRLECompressed");

	std::uint32_t segmentsOffset[16];
	::memset(segmentsOffset, 0, sizeof(segmentsOffset));

	// The first phase fills the segmentsOffset pointers, the
	//  second phase writes to the stream.
	///////////////////////////////////////////////////////////
	for(int phase = 0; phase < 2; ++phase)
	{
		if(phase == 1)
		{
			pDestStream->adjustEndian((std::uint8_t*)segmentsOffset, 4, streamController::lowByteEndian, sizeof(segmentsOffset) / sizeof(segmentsOffset[0]));
			pDestStream->write((std::uint8_t*)segmentsOffset, sizeof(segmentsOffset));
		}

		std::uint32_t segmentNumber = 0;
		std::uint32_t offset = 64;

		for(std::uint32_t scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)
		{
            std::vector<std::uint8_t> rowBytes(imageSizeX);

            std::vector<std::uint8_t> differentBytes;
            differentBytes.reserve(imageSizeX);

			for(std::int32_t rightShift = ((allocatedBits + 7) & 0xfffffff8) -8; rightShift >= 0; rightShift -= 8)
			{
				std::int32_t* pPixel = m_channels[scanChannels]->m_pBuffer;

				if(phase == 0)
				{
					segmentsOffset[++segmentNumber] = offset;
					segmentsOffset[0] = segmentNumber;
				}
				else
				{
					offset = segmentsOffset[++segmentNumber];
				}

				for(std::uint32_t scanY = imageSizeY; scanY != 0; --scanY)
				{
                    std::uint8_t* rowBytesPointer = &(rowBytes[0]);

					for(std::uint32_t scanX = imageSizeX; scanX != 0; --scanX)
					{
						*(rowBytesPointer++) = (std::uint8_t)((*pPixel & mask) >> rightShift);
						++pPixel;
					}

					for(std::uint32_t scanBytes = 0; scanBytes < imageSizeX; /* left empty */)
					{
                        std::uint8_t currentByte = rowBytes[scanBytes];

                        // Calculate the run-length
                        ///////////////////////////
                        size_t runLength(1);
                        for(; ((scanBytes + runLength) != imageSizeX) && rowBytes[scanBytes + runLength] == currentByte; ++runLength)
                        {
                        }

                        // Write the runlength
                        //////////////////////
                        if(runLength > 3)
                        {
                            if(!differentBytes.empty())
                            {
                                offset += writeRLEDifferentBytes(&differentBytes, pDestStream, phase == 1);
                            }
                            if(runLength > 128)
                            {
                                runLength = 128;
                            }
                            offset += 2;
                            scanBytes += runLength;
                            if(phase == 1)
                            {
                                std::uint8_t lengthByte = 1 - runLength;
                                pDestStream->write(&lengthByte, 1);
                                pDestStream->write(&currentByte, 1);
                            }
                            continue;
                        }

                        // Remmember sequence of different bytes
                        ////////////////////////////////////////
                        differentBytes.push_back(currentByte);
                        ++scanBytes;
                    } // for(std::uint32_t scanBytes = 0; scanBytes < imageSizeX; )

                    offset += writeRLEDifferentBytes(&differentBytes, pDestStream, phase == 1);

				} // for(std::uint32_t scanY = imageSizeY; scanY != 0; --scanY)

				if(offset & 0x00000001)
				{
					++offset;
					if(phase == 1)
					{
                        static const std::uint8_t command = 0x80;
						pDestStream->write(&command, 1);
					}
				}

			} // for(std::int32_t rightShift = ((allocatedBits + 7) & 0xfffffff8) -8; rightShift >= 0; rightShift -= 8)

		} // for(int scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)

	} // for(int phase = 0; phase < 2; ++phase)

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write RLE sequence of different bytes
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t dicomCodec::writeRLEDifferentBytes(std::vector<std::uint8_t>* pDifferentBytes, streamWriter* pDestStream, bool bWrite)
{
    size_t writtenLength = 0;
    for(size_t offset(0); offset != pDifferentBytes->size();)
    {
        size_t writeSize = pDifferentBytes->size() - offset;
        if(writeSize > 128)
        {
            writeSize = 128;
        }
        writtenLength += writeSize + 1;
        if(bWrite)
        {
            const std::uint8_t writeLength((std::uint8_t)(writeSize - 1));
            pDestStream->write(&writeLength, 1);
            pDestStream->write(&(pDifferentBytes->at(offset)), writeSize);
        }
        offset += writeSize;
    }
    pDifferentBytes->clear();

    // return number of written bytes
    /////////////////////////////////
    return writtenLength;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a RLE compressed image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::readRLECompressed(
	std::uint32_t imageSizeX,
	std::uint32_t imageSizeY,
	std::uint32_t channelsNumber,
	streamReader* pSourceStream,
	std::uint8_t allocatedBits,
	std::uint32_t mask,
	bool /*bInterleaved*/)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::readRLECompressed");

	// Copy the RLE header into the segmentsOffset array
	//  and adjust the byte endian to the machine architecture
	///////////////////////////////////////////////////////////
	std::uint32_t segmentsOffset[16];
	::memset(segmentsOffset, 0, sizeof(segmentsOffset));
	pSourceStream->read((std::uint8_t*)segmentsOffset, 64);
    pSourceStream->adjustEndian((std::uint8_t*)segmentsOffset, 4, streamController::lowByteEndian, sizeof(segmentsOffset) / sizeof(segmentsOffset[0]));

	//
	// Scan all the RLE segments
	//
	///////////////////////////////////////////////////////////
	std::uint32_t loopsNumber = channelsNumber;
	std::uint32_t loopSize = imageSizeX * imageSizeY;

	std::uint32_t currentSegmentOffset = sizeof(segmentsOffset);
	std::uint8_t segmentNumber = 0;
	for(std::uint32_t channel = 0; channel<loopsNumber; ++channel)
	{
		for(std::int32_t leftShift = ((allocatedBits + 7) & 0xfffffff8) -8; leftShift >= 0; leftShift -= 8)
		{
			// Prepare to scan all the RLE segment
			///////////////////////////////////////////////////////////
			std::uint32_t segmentOffset=segmentsOffset[++segmentNumber]; // Get the offset
			pSourceStream->seek(segmentOffset - currentSegmentOffset, true);
			currentSegmentOffset = segmentOffset;

			std::uint8_t  rleByte = 0;         // RLE code
			std::uint8_t  copyBytes = 0;       // Number of bytes to copy
			std::uint8_t  runByte = 0;         // Byte to use in run-lengths
			std::uint8_t  runLength = 0;       // Number of bytes with the same information (runByte)
			std::uint8_t  copyBytesBuffer[0x81];

			std::int32_t* pChannelMemory = m_channels[channel]->m_pBuffer;
			std::uint32_t channelSize = loopSize;
			std::uint8_t* pScanCopyBytes;

			// Read the RLE segment
			///////////////////////////////////////////////////////////
			pSourceStream->read(&rleByte, 1);
			++currentSegmentOffset;
			while(channelSize != 0)
			{
				if(rleByte==0x80)
				{
				    pSourceStream->read(&rleByte, 1);
					++currentSegmentOffset;
					continue;
				}

				// Copy the specified number of bytes
				///////////////////////////////////////////////////////////
				if(rleByte<0x80)
				{
					copyBytes = ++rleByte;
					if(copyBytes < channelSize)
					{
					    pSourceStream->read(copyBytesBuffer, copyBytes + 1);
						currentSegmentOffset += copyBytes + 1;
						rleByte = copyBytesBuffer[copyBytes];
					}
					else
					{
					    pSourceStream->read(copyBytesBuffer, copyBytes);
						currentSegmentOffset += copyBytes;
					}
					pScanCopyBytes = copyBytesBuffer;
					while(copyBytes-- && channelSize != 0)
					{
						*pChannelMemory |= ((*pScanCopyBytes++) << leftShift) & mask;
						++pChannelMemory;
						--channelSize;
					}
					continue;
				}

				// Copy the same byte several times
				///////////////////////////////////////////////////////////
				runLength = (std::uint8_t)0x1-rleByte;
				if(runLength < channelSize)
				{
				    pSourceStream->read(copyBytesBuffer, 2);
					currentSegmentOffset += 2;
					runByte = copyBytesBuffer[0];
					rleByte = copyBytesBuffer[1];
				}
				else
				{
				    pSourceStream->read(&runByte, 1);
					++currentSegmentOffset;
				}
				while(runLength-- && channelSize != 0)
				{
					*pChannelMemory |= (runByte << leftShift) & mask;
					++pChannelMemory;
					--channelSize;
				}

			} // ...End of the segment scanning loop

		} // ...End of the leftshift calculation

	} // ...Channels scanning loop

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a single component from a DICOM raw image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::readPixel(
					streamReader* pSourceStream,
					std::int32_t* pDest,
					std::uint32_t numPixels,
					std::uint8_t* pBitPointer,
					std::uint8_t* pReadBuffer,
					const std::uint8_t wordSizeBytes,
					const std::uint8_t allocatedBits,
					const std::uint32_t mask)
{
	if(allocatedBits == 8 || allocatedBits == 16 || allocatedBits == 32)
	{
		pSourceStream->read(pReadBuffer, numPixels * (allocatedBits >> 3));
		if(allocatedBits == 8)
		{
			std::uint8_t* pSource(pReadBuffer);
			while(numPixels-- != 0)
			{
				*pDest++ = (std::uint32_t)(*pSource++) & mask;
            }
			return;
		}
		pSourceStream->adjustEndian(pReadBuffer, allocatedBits >> 3, streamController::lowByteEndian, numPixels);
		if(allocatedBits == 16)
		{
			std::uint16_t* pSource((std::uint16_t*)(pReadBuffer));
			while(numPixels-- != 0)
			{
				*pDest++ = (std::uint32_t)(*pSource++) & mask;
			}
			return;
		}
		std::uint32_t* pSource((std::uint32_t*)(pReadBuffer));
		while(numPixels-- != 0)
		{
			*pDest++ = (*pSource++) & mask;
		}
		return;

	}


        while(numPixels-- != 0)
        {
            *pDest = 0;
            for(std::uint8_t bitsToRead = allocatedBits; bitsToRead != 0;)
            {
                if(*pBitPointer == 0)
                {
                    if(wordSizeBytes==0x2)
                    {
                        pSourceStream->read((std::uint8_t*)&m_ioWord, sizeof(m_ioWord));
                        *pBitPointer = 16;
                    }
                    else
                    {
                        pSourceStream->read(&m_ioByte, 1);
                        m_ioWord = (std::uint16_t)m_ioByte;
                        *pBitPointer = 8;
                    }
                }

                if(*pBitPointer <= bitsToRead)
                {
                    *pDest |= m_ioWord << (allocatedBits - bitsToRead);
                    bitsToRead -= *pBitPointer;
                    *pBitPointer = 0;
                    continue;
                }

                *pDest |= (m_ioWord & (((std::uint16_t)1<<bitsToRead) - 1)) << (allocatedBits - bitsToRead);
                m_ioWord >>= bitsToRead;
                *pBitPointer -= bitsToRead;
                bitsToRead = 0;
            }
            *pDest++ &= mask;
        }
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a single component from a DICOM raw image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::writePixel(
					streamWriter* pDestStream,
					std::int32_t pixelValue,
					std::uint8_t*  pBitPointer,
					std::uint8_t wordSizeBytes,
					std::uint8_t allocatedBits,
					std::uint32_t mask)
{
	pixelValue &= mask;

	if(allocatedBits == 8)
	{
        m_ioByte = (std::uint8_t)pixelValue;
        pDestStream->write(&m_ioByte, sizeof(m_ioByte));
		return;
	}

	if(allocatedBits == 16)
    {
        if(wordSizeBytes == 1)
        {
            m_ioWord = pDestStream->adjustEndian((std::uint16_t)pixelValue, streamController::lowByteEndian);
        }
        else
        {
            m_ioWord = (std::uint16_t)pixelValue;
        }
        pDestStream->write((std::uint8_t*)&m_ioWord, sizeof(m_ioWord));
        return;
	}

	if(allocatedBits == 32)
	{
		if(wordSizeBytes == 1)
        {
            m_ioDWord = pDestStream->adjustEndian((std::uint32_t)m_ioDWord, streamController::lowByteEndian);
        }
        else
        {
            m_ioDWord = (std::uint32_t)pixelValue;
        }
        pDestStream->write((std::uint8_t*)&m_ioDWord, sizeof(m_ioDWord));
		return;
	}

    std::uint8_t maxBits = (std::uint8_t)(wordSizeBytes << 3);

	for(std::uint8_t writeBits = allocatedBits; writeBits != 0;)
	{
		std::uint8_t freeBits = maxBits - *pBitPointer;
		if(freeBits == maxBits)
		{
            m_ioWord = 0;
		}
		if( freeBits <= writeBits )
		{
            m_ioWord |= (pixelValue & (((std::int32_t)1 << freeBits) -1 ))<< (*pBitPointer);
			*pBitPointer = maxBits;
			writeBits -= freeBits;
			pixelValue >>= freeBits;
		}
		else
		{
            m_ioWord |= (pixelValue & (((std::int32_t)1 << writeBits) -1 ))<< (*pBitPointer);
			(*pBitPointer) += writeBits;
			writeBits = 0;
		}

		if(*pBitPointer == maxBits)
		{
			if(wordSizeBytes == 2)
			{
                pDestStream->write((std::uint8_t*)&m_ioWord, 2);
			}
			else
			{
                m_ioByte = (std::uint8_t)m_ioWord;
                pDestStream->write(&m_ioByte, 1);
			}
			*pBitPointer = 0;
		}
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Used by the writing routines to commit the unwritten
//  bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::flushUnwrittenPixels(streamWriter* pDestStream, std::uint8_t* pBitPointer, std::uint8_t wordSizeBytes)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::flushUnwrittenPixels");

	if(*pBitPointer == 0)
	{
		return;
	}
	if(wordSizeBytes == 2)
	{
		pDestStream->write((std::uint8_t*)&m_ioWord, 2);
	}
	else if(wordSizeBytes == 4)
	{
		pDestStream->write((std::uint8_t*)&m_ioDWord, 4);
	}
	else
	{
		m_ioByte = (std::uint8_t)m_ioWord;
		pDestStream->write(&m_ioByte, 1);
	}
	*pBitPointer = 0;

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert an image into a Dicom structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomCodec::setImage(
		ptr<streamWriter> pDestStream,
		ptr<image> pImage,
		std::wstring transferSyntax,
		quality /*imageQuality*/,
		std::string dataType,
		std::uint8_t allocatedBits,
		bool bSubSampledX,
		bool bSubSampledY,
		bool bInterleaved,
		bool /*b2Complement*/)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::setImage");

	// First calculate the attributes we want to use.
	// Return an exception if they are different from the
	//  old ones and bDontChangeAttributes is true
	///////////////////////////////////////////////////////////
	std::uint32_t imageWidth, imageHeight;
	pImage->getSize(&imageWidth, &imageHeight);

	std::wstring colorSpace = pImage->getColorSpace();
	std::uint32_t highBit = pImage->getHighBit();
	bool bRleCompressed = (transferSyntax == L"1.2.840.10008.1.2.5");

	std::uint32_t rowSize, channelPixelSize, channelsNumber;
	ptr<handlers::dataHandlerNumericBase> imageHandler = pImage->getDataHandler(false, &rowSize, &channelPixelSize, &channelsNumber);

	// Copy the image into the dicom channels
	///////////////////////////////////////////////////////////
	allocChannels(channelsNumber, imageWidth, imageHeight, bSubSampledX, bSubSampledY);
	std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
	std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;
	for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
	{
		ptrChannel dicomChannel = m_channels[copyChannels];
		imageHandler->copyToInt32Interleaved(
			dicomChannel->m_pBuffer,
			maxSamplingFactorX /dicomChannel->m_samplingFactorX,
			maxSamplingFactorY /dicomChannel->m_samplingFactorY,
			0, 0,
			dicomChannel->m_sizeX * maxSamplingFactorX / dicomChannel->m_samplingFactorX,
			dicomChannel->m_sizeY * maxSamplingFactorY / dicomChannel->m_samplingFactorY,
			copyChannels,
			imageWidth,
			imageHeight,
			channelsNumber);
	}

	std::uint32_t mask = ((std::uint32_t)1 << (highBit + 1)) - 1;

	if(bRleCompressed)
	{
		writeRLECompressed(
			imageWidth,
			imageHeight,
			channelsNumber,
			pDestStream.get(),
			allocatedBits,
			mask);
		return;
	}

	std::uint8_t wordSizeBytes = ((dataType == "OW") || (dataType == "SS") || (dataType == "US")) ? 2 : 1;

	if(bInterleaved || channelsNumber == 1)
	{
		writeUncompressedInterleaved(
			channelsNumber,
			bSubSampledX, bSubSampledY,
			pDestStream.get(),
			wordSizeBytes,
			allocatedBits,
			mask);
		return;
	}

	writeUncompressedNotInterleaved(
		channelsNumber,
		pDestStream.get(),
		wordSizeBytes,
		allocatedBits,
		mask);

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the transfer
//  syntax
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool dicomCodec::canHandleTransferSyntax(std::wstring transferSyntax)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::canHandleTransferSyntax");

	return(
		transferSyntax == L"" ||
		transferSyntax == L"1.2.840.10008.1.2" ||      // Implicit VR little endian
		transferSyntax == L"1.2.840.10008.1.2.1" ||    // Explicit VR little endian
		// transferSyntax==L"1.2.840.10008.1.2.1.99" || // Deflated explicit VR little endian
		transferSyntax == L"1.2.840.10008.1.2.2" ||    // Explicit VR big endian
		transferSyntax == L"1.2.840.10008.1.2.5");     // RLE compression

	PUNTOEXE_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the transfer syntax has to be
//  encapsulated
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
bool dicomCodec::encapsulated(std::wstring transferSyntax)
{
	PUNTOEXE_FUNCTION_START(L"jpegCodec::canHandleTransferSyntax");

	if(!canHandleTransferSyntax(transferSyntax))
	{
		PUNTOEXE_THROW(codecExceptionWrongTransferSyntax, "Cannot handle the transfer syntax");
	}
	return (transferSyntax == L"1.2.840.10008.1.2.5");

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the highest bit number that can be handled
//  by the codec
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomCodec::getMaxHighBit(std::string /* transferSyntax */)
{
	return 15;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Suggest the number of allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomCodec::suggestAllocatedBits(std::wstring transferSyntax, std::uint32_t highBit)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::suggestAllocatedBits");

	if(transferSyntax == L"1.2.840.10008.1.2.5")
	{
		return (highBit + 8) & 0xfffffff8;
	}

	return highBit + 1;

	PUNTOEXE_FUNCTION_END();

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a single tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomCodec::readTag(
	ptr<streamReader> pStream,
	ptr<dataSet> pDataSet,
	std::uint32_t tagLengthDWord,
	std::uint16_t tagId,
	std::uint16_t order,
	std::uint16_t tagSubId,
	std::string tagType,
	streamController::tByteOrdering endianType,
	short wordSize,
	std::uint32_t bufferId,
	std::uint32_t maxSizeBufferLoad /* = 0xffffffff */
	)
{
	PUNTOEXE_FUNCTION_START(L"dicomCodec::readTag");

	// If the tag's size is bigger than the maximum loadable
	//  size then just specify in which file it resides
	///////////////////////////////////////////////////////////
	if(tagLengthDWord > maxSizeBufferLoad)
	{
		std::uint32_t bufferPosition(pStream->position());
		std::uint32_t streamPosition(pStream->getControlledStreamPosition());
		pStream->seek(tagLengthDWord, true);
		std::uint32_t bufferLength(pStream->position() - bufferPosition);

		if(bufferLength != tagLengthDWord)
		{
			PUNTOEXE_THROW(codecExceptionCorruptedFile, "dicomCodec::readTag detected a corrupted tag");
		}

		ptr<dataGroup> writeGroup(pDataSet->getGroup(tagId, order, true));
		ptr<data>      writeData (writeGroup->getTag(tagSubId, true));
		ptr<buffer> newBuffer(
			new buffer(
				writeData,
				tagType,
				pStream->getControlledStream(),
				streamPosition,
				bufferLength,
				wordSize,
				endianType));

		writeData->setBuffer(bufferId, newBuffer);

		return bufferLength;
	}

	// Allocate the tag's buffer
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerRaw> handler(pDataSet->getDataHandlerRaw(tagId, order, tagSubId, bufferId, true, tagType));

	// Do nothing if the tag's size is 0
	///////////////////////////////////////////////////////////
	if(tagLengthDWord == 0)
	{
		return 0;
	}

	// In order to deal with damaged tags asking for an
	//  incredible amount of memory, this function reads the
	//  tag using a lot of small buffers (32768 bytes max)
	//  and then the tag's buffer is rebuilt at the end of the
	//  function.
	// This method saves a lot of time if a huge amount of
	//  memory is asked by a damaged tag, since only the amount
	//  of memory actually stored in the source file is
	//  allocated
	///////////////////////////////////////////////////////////

	// If the buffer size is bigger than the following const
	//  variable, then read the buffer in small chunks
	///////////////////////////////////////////////////////////
	static const std::uint32_t smallBuffersSize(32768);

	if(tagLengthDWord <= smallBuffersSize) // Read in one go
	{
		handler->setSize(tagLengthDWord);
		pStream->read(handler->getMemoryBuffer(), tagLengthDWord);
	}
	else // Read in small chunks
	{
		std::list<std::vector<std::uint8_t> > buffers;

		// Used to keep track of the read bytes
		///////////////////////////////////////////////////////////
		std::uint32_t remainingBytes(tagLengthDWord);

		// Fill all the small buffers
		///////////////////////////////////////////////////////////
		while(remainingBytes != 0)
		{
			// Calculate the small buffer's size and allocate it
			///////////////////////////////////////////////////////////
			std::uint32_t thisBufferSize( (remainingBytes > smallBuffersSize) ? smallBuffersSize : remainingBytes);
			buffers.push_back(std::vector<std::uint8_t>());
			buffers.back().resize(thisBufferSize);

			// Fill the buffer
			///////////////////////////////////////////////////////////
			pStream->read(&buffers.back()[0], thisBufferSize);

			// Decrease the number of the remaining bytes
			///////////////////////////////////////////////////////////
			remainingBytes -= thisBufferSize;
		}

		// Copy the small buffers into the tag object
		///////////////////////////////////////////////////////////
		handler->setSize(tagLengthDWord);
		std::uint8_t* pHandlerBuffer(handler->getMemoryBuffer());

		// Scan all the small buffers and copy their content into
		//  the final buffer
		///////////////////////////////////////////////////////////
		std::list<std::vector<std::uint8_t> >::iterator smallBuffersIterator;
		remainingBytes = tagLengthDWord;
		for(smallBuffersIterator=buffers.begin(); smallBuffersIterator != buffers.end(); ++smallBuffersIterator)
		{
			std::uint32_t copySize=(remainingBytes>smallBuffersSize) ? smallBuffersSize : remainingBytes;
			::memcpy(pHandlerBuffer, &(*smallBuffersIterator)[0], copySize);
			pHandlerBuffer += copySize;
			remainingBytes -= copySize;
		}
	} // end of reading from stream

	// All the bytes have been read, now rebuild the tag's
	//  buffer. Don't rebuild the tag if it is 0xfffc,0xfffc
	//  (end of the stream)
	///////////////////////////////////////////////////////////
	if(tagId == 0xfffc && tagSubId == 0xfffc)
	{
		return (std::uint32_t)tagLengthDWord;
	}

	// Adjust the buffer's byte endian
	///////////////////////////////////////////////////////////
	if(wordSize != 0)
	{
		pStream->adjustEndian(handler->getMemoryBuffer(), wordSize, endianType, tagLengthDWord / wordSize);
	}

	// Return the tag's length in bytes
	///////////////////////////////////////////////////////////
	return (std::uint32_t)tagLengthDWord;

	PUNTOEXE_FUNCTION_END();
}

} // namespace codecs

} // namespace imebra

} // namespace puntoexe

