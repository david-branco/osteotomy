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

/*! \file dataHandlerNumeric.h
    \brief Declaration of the handler for the numeric tags.

*/

#if !defined(imebraDataHandlerNumeric_BD270581_5746_48d1_816E_64B700955A12__INCLUDED_)
#define imebraDataHandlerNumeric_BD270581_5746_48d1_816E_64B700955A12__INCLUDED_

#include <sstream>
#include <iomanip>
#include <type_traits>

#include "../../base/include/exception.h"
#include "dataHandler.h"


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

/// \brief This is the base class for the %data handlers
///         that manage numeric values.
///
///////////////////////////////////////////////////////////
class dataHandlerNumericBase: public dataHandler
{
    friend class buffer;

public:
	std::uint8_t* getMemoryBuffer() const
	{
		return m_pMemoryString;
	}

	size_t getMemorySize() const
	{
		return m_memorySize;
	}

	/// \brief Returns the memory object that stores the data
	///         managed by the handler.
	///
	/// @return the memory object that stores the data managed
	///          by the handler
	///
	///////////////////////////////////////////////////////////
	ptr<memory> getMemory()
	{
		return m_memory;
	}

	// Set the buffer's size, in data elements
	///////////////////////////////////////////////////////////
	virtual void setSize(const std::uint32_t elementsNumber)
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::setSize");

		m_memory->resize(elementsNumber * getUnitSize());
		m_pMemoryString = m_memory->data();
		m_memorySize = m_memory->size();

		PUNTOEXE_FUNCTION_END();
	}

	// Parse the tag's buffer and extract its content
	///////////////////////////////////////////////////////////
	virtual void parseBuffer(const ptr<memory>& memoryBuffer)
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::parseBuffer");

		m_memory = memoryBuffer;
		m_pMemoryString = m_memory->data();
		m_memorySize = m_memory->size();

		PUNTOEXE_FUNCTION_END();
	}

	// Rebuild the tag's buffer
	///////////////////////////////////////////////////////////
	virtual void buildBuffer(const ptr<memory>& memoryBuffer)
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::buildBuffer");

		memoryBuffer->transfer(m_memory);

		PUNTOEXE_FUNCTION_END();
	}

	virtual void copyFrom(ptr<dataHandlerNumericBase> pSource) = 0;

	virtual void copyFrom(std::uint8_t* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(std::int8_t* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(std::uint16_t* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(std::int16_t* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(std::uint32_t* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(std::int32_t* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(float* pMemory, size_t memorySize) = 0;
	virtual void copyFrom(double* pMemory, size_t memorySize) = 0;

	virtual void copyTo(std::uint8_t* pMemory, size_t memorySize) = 0;
	virtual void copyTo(std::int8_t* pMemory, size_t memorySize) = 0;
	virtual void copyTo(std::uint16_t* pMemory, size_t memorySize) = 0;
	virtual void copyTo(std::int16_t* pMemory, size_t memorySize) = 0;
	virtual void copyTo(std::uint32_t* pMemory, size_t memorySize) = 0;
	virtual void copyTo(std::int32_t* pMemory, size_t memorySize) = 0;
	virtual void copyTo(float* pMemory, size_t memorySize) = 0;
	virtual void copyTo(double* pMemory, size_t memorySize) = 0;

	virtual void copyFromInt32Interleaved(const std::int32_t* pSource,
										  std::uint32_t sourceReplicateX,
										  std::uint32_t sourceReplicateY,
										  std::uint32_t destStartCol,
										  std::uint32_t destStartRow,
										  std::uint32_t destEndCol,
										  std::uint32_t destEndRow,
										  std::uint32_t destStartChannel,
										  std::uint32_t destWidth,
										  std::uint32_t destHeight,
										  std::uint32_t destNumChannels) = 0;

	virtual void copyToInt32Interleaved(std::int32_t* pDest,
										std::uint32_t destSubSampleX,
										std::uint32_t destSubSampleY,
										std::uint32_t sourceStartCol,
										std::uint32_t sourceStartRow,
										std::uint32_t sourceEndCol,
										std::uint32_t sourceEndRow,
										std::uint32_t sourceStartChannel,
										std::uint32_t sourceWidth,
										std::uint32_t sourceHeight,
										std::uint32_t sourceNumChannels) const = 0;

	/// \brief Returns truen if the buffer's elements are
	///         signed, false otherwise.
	///
	/// @return true if the buffer's elements are signed,
	///          or false otherwise
	///
	///////////////////////////////////////////////////////////
	virtual bool isSigned() const = 0;

	virtual bool pointerIsValid(const std::uint32_t index) const
	{
		return index < getSize();
	}

protected:
	// Memory buffer
	///////////////////////////////////////////////////////////
	std::uint8_t* m_pMemoryString;
	size_t m_memorySize;
	ptr<memory> m_memory;
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This data handler accesses to the numeric data
///         stored in a puntoexe::imebra::buffer class.
///
/// A special definition of this class
///  (puntoexe::imebra::handlers::imageHandler) is used
///  to access to the images' raw pixels.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
template<class dataHandlerType>
class dataHandlerNumeric : public dataHandlerNumericBase
{
public:
	/// \brief Provides a direct access to the data managed by
	///         the handler.
	///
	/// @param nSubscript  the index of the numeric value that
	///                     you want to access to
	/// @return a reference to the requested value
	///
	///////////////////////////////////////////////////////////
	dataHandlerType& operator[](int nSubscript)
	{
		return ((dataHandlerType*)m_pMemoryString)[nSubscript];
	}

	/// \brief Provides a direct access to the data managed by
	///         the handler.
	///
	/// @param nSubscript  the index of the numeric value that
	///                     you want to access to
	/// @return a reference to the requested value
	///
	///////////////////////////////////////////////////////////
	dataHandlerType& at(int nSubscript)
	{
		return ((dataHandlerType*)m_pMemoryString)[nSubscript];
	}

	// Returns the size of an element managed by the
	//  handler.
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getUnitSize() const
	{
		return sizeof(dataHandlerType);
	}

	virtual bool isSigned() const
	{
		dataHandlerType firstValue((dataHandlerType) -1);
		dataHandlerType secondValue((dataHandlerType) 0);
		return firstValue < secondValue;
	}

	// Retrieve the data element as a signed long
	///////////////////////////////////////////////////////////
	virtual std::int32_t getSignedLong(const std::uint32_t index) const
	{
		return (std::int32_t) (((dataHandlerType*)m_pMemoryString)[index]);
	}

	// Retrieve the data element an unsigned long
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getUnsignedLong(const std::uint32_t index) const
	{
		return (std::uint32_t) (((dataHandlerType*)m_pMemoryString)[index]);
	}

	// Retrieve the data element as a double
	///////////////////////////////////////////////////////////
	virtual double getDouble(const std::uint32_t index) const
	{
		return (double) (((dataHandlerType*)m_pMemoryString)[index]);
	}

	// Retrieve the data element as a string
	///////////////////////////////////////////////////////////
	virtual std::string getString(const std::uint32_t index) const
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::getString");

		std::ostringstream convStream;

        if(std::is_same<dataHandlerType, std::uint8_t>::value ||
                std::is_same<dataHandlerType, std::int8_t>::value )
        {
            int tempValue = (int)(((dataHandlerType*)m_pMemoryString)[index]);
            convStream << tempValue;
        }
        else
        {
            convStream << (((dataHandlerType*)m_pMemoryString)[index]);
        }
		return convStream.str();

		PUNTOEXE_FUNCTION_END();
	}

	// Retrieve the data element as a unicode string
	///////////////////////////////////////////////////////////
	virtual std::wstring getUnicodeString(const std::uint32_t index) const
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::getUnicodeString");

        std::string ansiString = getString(index);

        std::wstring unicodeString(ansiString.size(), char(0));
        for(size_t scanAstring(0), endAstring(ansiString.size()); scanAstring != endAstring; ++scanAstring)
        {
            unicodeString[scanAstring] = (wchar_t)ansiString.at(scanAstring);
        }
        return unicodeString;

		PUNTOEXE_FUNCTION_END();
	}

	// Retrieve the buffer's size in elements
	///////////////////////////////////////////////////////////
	virtual std::uint32_t getSize() const
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::getSize");

		return m_memorySize/getUnitSize();

		PUNTOEXE_FUNCTION_END();
	}

	// Set the data element as a signed long
	///////////////////////////////////////////////////////////
	virtual void setSignedLong(const std::uint32_t index, const std::int32_t value)
	{
		((dataHandlerType*)m_pMemoryString)[index] = (dataHandlerType)value;
	}

	// Set the data element as an unsigned long
	///////////////////////////////////////////////////////////
	virtual void setUnsignedLong(const std::uint32_t index, const std::uint32_t value)
	{
		((dataHandlerType*)m_pMemoryString)[index] = (dataHandlerType)value;
	}

	// Set the data element as a double
	///////////////////////////////////////////////////////////
	virtual void setDouble(const std::uint32_t index, const double value)
	{
		((dataHandlerType*)m_pMemoryString)[index] = (dataHandlerType)value;
	}

	// Set the data element as a string
	///////////////////////////////////////////////////////////
	virtual void setString(const std::uint32_t index, const std::string& value)
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::setString");

		std::istringstream convStream(value);
        dataHandlerType tempValue;
        if(std::is_same<dataHandlerType, std::uint8_t>::value ||
                std::is_same<dataHandlerType, std::int8_t>::value)
        {
            int tempValue1;
            convStream >> tempValue1;
            tempValue = (dataHandlerType)tempValue1;
        }
        else
        {
            convStream >> tempValue;
        }

        ((dataHandlerType*)m_pMemoryString)[index] = tempValue;

		PUNTOEXE_FUNCTION_END();
	}

	// Set the data element as an unicode string
	///////////////////////////////////////////////////////////
	virtual void setUnicodeString(const std::uint32_t index, const std::wstring& value)
	{
		PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::setUnicodeString");

        std::string ansiString(value.size(), char(0));
        for(size_t scanWstring(0), endWstring(value.size()); scanWstring != endWstring; ++scanWstring)
        {
            ansiString[scanWstring] = (char)value.at(scanWstring);
        }
        setString(index, ansiString);

		PUNTOEXE_FUNCTION_END();
	}

	/// \brief Copy the values from a memory location to
	///         the buffer managed by the handler
	///
	/// @param pSource a pointer to the first byte of the
	///         memory area to copy
	/// @param sourceSize the number of values to copy
	///
	///////////////////////////////////////////////////////////
	template<class sourceHandlerType>
	void copyFromMemory(sourceHandlerType* pSource, size_t sourceSize)
	{
		setSize(sourceSize);
		if(getSize() < sourceSize)
		{
			sourceSize = getSize();
		}
		dataHandlerType* pDest((dataHandlerType*)m_pMemoryString);
		while(sourceSize-- != 0)
		{
			*(pDest++) = (dataHandlerType)*(pSource++);
		}
	}

	// Copy the data from another handler
	///////////////////////////////////////////////////////////
    virtual void copyFrom(ptr<dataHandlerNumericBase> pSource)
    {
        PUNTOEXE_FUNCTION_START(L"dataHandlerNumeric::copyFrom");

        puntoexe::imebra::handlers::dataHandlerNumericBase* pHandler(pSource.get());
        if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint8_t>) ||
            dynamic_cast<puntoexe::imebra::handlers::dataHandlerNumeric<std::uint8_t>* >(pHandler) != 0)
        {
            copyFromMemory<std::uint8_t> ((std::uint8_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int8_t>))
        {
            copyFromMemory<std::int8_t> ((std::int8_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint16_t>))
        {
            copyFromMemory<std::uint16_t> ((std::uint16_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int16_t>))
        {
            copyFromMemory<std::int16_t> ((std::int16_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint32_t>))
        {
            copyFromMemory<std::uint32_t> ((std::uint32_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int32_t>))
        {
            copyFromMemory<std::int32_t> ((std::int32_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<float>))
        {
            copyFromMemory<float> ((float*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<double>))
        {
            copyFromMemory<double> ((double*)pHandler->getMemoryBuffer(), pHandler->getSize());
        }
        else
        {
            throw std::runtime_error("Data type not valid");
        }

        PUNTOEXE_FUNCTION_END();

    }

	virtual void copyFrom(std::uint8_t* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(std::int8_t* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(std::uint16_t* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(std::int16_t* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(std::uint32_t* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(std::int32_t* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(float* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}
	virtual void copyFrom(double* pMemory, size_t memorySize)
	{
		copyFromMemory(pMemory, memorySize);
	}


	template<class destHandlerType>
	void copyToMemory(destHandlerType* pDestination, size_t destSize)
	{
		if(getSize() < destSize)
		{
			destSize = getSize();
		}
		dataHandlerType* pSource((dataHandlerType*)m_pMemoryString);
		while(destSize-- != 0)
		{
			*(pDestination++) = (destHandlerType)*(pSource++);
		}
	}

	virtual void copyTo(std::uint8_t* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(std::int8_t* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(std::uint16_t* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(std::int16_t* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(std::uint32_t* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(std::int32_t* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(float* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}
	virtual void copyTo(double* pMemory, size_t memorySize)
	{
		copyToMemory(pMemory, memorySize);
	}


	template<int subsampleX>
	inline void copyFromInt32Interleaved(
		const std::int32_t* pSource,
		std::uint32_t sourceReplicateY,
		std::uint32_t destStartCol,
		std::uint32_t destStartRow,
		std::uint32_t destEndCol,
		std::uint32_t destEndRow,
		std::uint32_t destStartChannel,
		std::uint32_t destWidth,
		std::uint32_t destHeight,
		std::uint32_t destNumChannels)
	{
		dataHandlerType *pDestRowScan = &(((dataHandlerType*)m_pMemoryString)[(destStartRow*destWidth+destStartCol)*destNumChannels+destStartChannel]);
		const std::int32_t* pSourceRowScan = pSource;

		std::uint32_t replicateYCount = sourceReplicateY;
		std::uint32_t replicateYIncrease = (destEndCol - destStartCol) / subsampleX;

		dataHandlerType *pDestColScan;
		const std::int32_t* pSourceColScan;
		const std::int32_t* pSourceEndColScan;

		if(destHeight < destEndRow)
		{
			destEndRow = destHeight;
		}
		if(destWidth < destEndCol)
		{
			destEndCol = destWidth;
		}

		std::uint32_t numColumns(destEndCol - destStartCol);

		std::uint32_t horizontalCopyOperations = numColumns / subsampleX;
		std::uint32_t horizontalFinalCopyDest = numColumns - horizontalCopyOperations * subsampleX;

		dataHandlerType copyValue;
		std::int32_t scanDest;

		std::uint32_t scanHorizontalFinalCopyDest;

		std::uint32_t destRowScanIncrease(destWidth * destNumChannels);


		for(std::uint32_t numYCopies(destEndRow - destStartRow); numYCopies != 0; --numYCopies)
		{
			pDestColScan = pDestRowScan;
			pSourceColScan = pSourceRowScan;
			pSourceEndColScan = pSourceColScan + horizontalCopyOperations;

			if(subsampleX == 1)
			{
				while(pSourceColScan != pSourceEndColScan)
				{
					*pDestColScan = (dataHandlerType)(*(pSourceColScan++));
					pDestColScan += destNumChannels;
				}
			}
			else
			{
				while(pSourceColScan != pSourceEndColScan)
				{
					copyValue = (dataHandlerType)(*(pSourceColScan++));
					for(scanDest = subsampleX; scanDest != 0; --scanDest)
					{
						*pDestColScan = copyValue;
						pDestColScan += destNumChannels;
					}
				}

				for(scanHorizontalFinalCopyDest = horizontalFinalCopyDest; scanHorizontalFinalCopyDest != 0; --scanHorizontalFinalCopyDest)
				{
					*pDestColScan = (dataHandlerType) *pSourceColScan;
					pDestColScan += destNumChannels;
				}

			}

			pDestRowScan += destRowScanIncrease;
			if(--replicateYCount == 0)
			{
				replicateYCount = sourceReplicateY;
				pSourceRowScan += replicateYIncrease;
			}
		}
	}


	/// \brief Copy the content of an array of std::int32_t values
	///         into the buffer controlled by the handler,
	///         considering that the source buffer could
	///         have subsampled data.
	///
	/// The source buffer is supposed to have the information
	///  related to a single channel.
	/// @param pSource      a pointer to the source array of
	///                      std::int32_t values
	/// @param sourceReplicateX the horizontal subsamplig
	///                      factor of the source buffer
	///                      (1=not subsampled, 2=subsampled)
	/// @param sourceReplicateY the vertical subsamplig
	///                      factor of the source buffer
	///                      (1=not subsampled, 2=subsampled)
	/// @param destStartCol the horizontal coordinate of the
	///                      top left corner of the destination
	///                      rectangle
	/// @param destStartRow the vertical coordinate of the
	///                      top left corner of the destination
	///                      rectangle
	/// @param destEndCol   the horizontal coordinate of the
	///                      bottom right corner of the
	///                      destination rectangle
	/// @param destEndRow   the vertical coordinate of the
	///                      bottom right corner of the
	///                      destination rectangle
	/// @param destStartChannel the destination channel
	/// @param destWidth    the destination buffer's width in
	///                      pixels
	/// @param destHeight   the destination buffer's height in
	///                      pixels
	/// @param destNumChannels the number of channels in the
	///                      destination buffer
	///
	///////////////////////////////////////////////////////////
	virtual void copyFromInt32Interleaved(const std::int32_t* pSource,
										  std::uint32_t sourceReplicateX,
										  std::uint32_t sourceReplicateY,
										  std::uint32_t destStartCol,
										  std::uint32_t destStartRow,
										  std::uint32_t destEndCol,
										  std::uint32_t destEndRow,
										  std::uint32_t destStartChannel,
										  std::uint32_t destWidth,
										  std::uint32_t destHeight,
										  std::uint32_t destNumChannels)
	{
		if(destStartCol >= destWidth || destStartRow >= destHeight)
		{
			return;
		}

		switch(sourceReplicateX)
		{
		case 1:
			copyFromInt32Interleaved< 1 > (
						pSource,
						sourceReplicateY,
						destStartCol,
						destStartRow,
						destEndCol,
						destEndRow,
						destStartChannel,
						destWidth,
						destHeight,
						destNumChannels);
			break;
		case 2:
			copyFromInt32Interleaved< 2 > (
						pSource,
						sourceReplicateY,
						destStartCol,
						destStartRow,
						destEndCol,
						destEndRow,
						destStartChannel,
						destWidth,
						destHeight,
						destNumChannels);
			break;
        case 4:
			copyFromInt32Interleaved< 4 > (
						pSource,
						sourceReplicateY,
						destStartCol,
						destStartRow,
						destEndCol,
						destEndRow,
						destStartChannel,
						destWidth,
						destHeight,
						destNumChannels);
			break;
        default:
            throw std::logic_error("Invalid subsampling factor");
        }
	}


	/// \brief Copy the buffer controlled by the handler into
	///         an array of std::int32_t values, considering that
	///         the destination buffer could be subsampled
	///
	/// The destination buffer is supposed to have the
	///  information related to a single channel.
	/// @param pDest        a pointer to the destination array
	///                      of std::int32_t values
	/// @param destSubSampleX the horizontal subsamplig
	///                      factor of the destination buffer
	///                      (1=not subsampled, 2=subsampled)
	/// @param destSubSampleY the vertical subsamplig
	///                      factor of the destination buffer
	///                      (1=not subsampled, 2=subsampled)
	/// @param sourceStartCol the horizontal coordinate of the
	///                      top left corner of the source
	///                      rectangle
	/// @param sourceStartRow the vertical coordinate of the
	///                      top left corner of the source
	///                      rectangle
	/// @param sourceEndCol   the horizontal coordinate of the
	///                      bottom right corner of the
	///                      source rectangle
	/// @param sourceEndRow   the vertical coordinate of the
	///                      bottom right corner of the
	///                      source rectangle
	/// @param sourceStartChannel the source channel to be
	///                      copied
	/// @param sourceWidth  the source buffer's width in
	///                      pixels
	/// @param sourceHeight the source buffer's height in
	///                      pixels
	/// @param sourceNumChannels the number of channels in the
	///                      source buffer
	///
	///////////////////////////////////////////////////////////
	virtual void copyToInt32Interleaved(std::int32_t* pDest,
										std::uint32_t destSubSampleX,
										std::uint32_t destSubSampleY,
										std::uint32_t sourceStartCol,
										std::uint32_t sourceStartRow,
										std::uint32_t sourceEndCol,
										std::uint32_t sourceEndRow,
										std::uint32_t sourceStartChannel,
										std::uint32_t sourceWidth,
										std::uint32_t sourceHeight,
										std::uint32_t sourceNumChannels) const
	{
		if(sourceStartCol >= sourceWidth || sourceStartRow >= sourceHeight)
		{
			return;
		}
		dataHandlerType *pSourceRowScan = &(((dataHandlerType*)m_pMemoryString)[(sourceStartRow*sourceWidth+sourceStartCol)*sourceNumChannels+sourceStartChannel]);
		std::int32_t* pDestRowScan = pDest;

		std::uint32_t subSampleXCount;
		std::uint32_t subSampleYCount = destSubSampleY;
		std::uint32_t subSampleYIncrease = (sourceEndCol - sourceStartCol) / destSubSampleX;

		dataHandlerType *pSourceColScan;
		std::int32_t* pDestColScan;

		std::int32_t lastValue = (std::int32_t)*pSourceRowScan;

		for(std::uint32_t scanRow = sourceStartRow; scanRow < sourceEndRow; ++scanRow)
		{
			pSourceColScan = pSourceRowScan;
			pDestColScan = pDestRowScan;
			subSampleXCount = destSubSampleX;

			for(std::uint32_t scanCol = sourceStartCol; scanCol < sourceEndCol; ++scanCol)
			{
				if(scanCol < sourceWidth)
				{
					lastValue = (std::int32_t)*pSourceColScan;
					pSourceColScan += sourceNumChannels;
				}
				*pDestColScan += (std::int32_t)lastValue;
				if(--subSampleXCount == 0)
				{
					subSampleXCount = destSubSampleX;
					++pDestColScan;
				}
			}
			if(scanRow < sourceHeight - 1)
			{
				pSourceRowScan += sourceWidth * sourceNumChannels;
			}
			if(--subSampleYCount == 0)
			{
				subSampleYCount = destSubSampleY;
				pDestRowScan += subSampleYIncrease;
			}
		}

		std::int32_t rightShift = 0;
		if(destSubSampleX == 2)
		{
			++rightShift;
		}
		if(destSubSampleY == 2)
		{
			++rightShift;
		}
		if(rightShift == 0)
		{
			return;
		}
		for(std::int32_t* scanDivide = pDest; scanDivide < pDestRowScan; ++scanDivide)
		{
			*scanDivide >>= rightShift;
		}

	}


};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief A dataHandlerRaw always "sees" the data as a
///         collection of bytes, no matter what the Dicom
///         data type is.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dataHandlerRaw: public dataHandlerNumeric<std::uint8_t>
{
};

} // namespace handlers

} // namespace imebra

} // namespace puntoexe


#define HANDLER_CALL_TEMPLATE_FUNCTION(functionName, handlerPointer)\
{\
    puntoexe::imebra::handlers::dataHandlerNumericBase* pHandler(handlerPointer.get()); \
    if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint8_t>) || \
    typeid(*pHandler)== typeid(puntoexe::imebra::handlers::dataHandlerRaw))\
{\
    functionName<std::uint8_t> ((std::uint8_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize());\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int8_t>))\
{\
    functionName<std::int8_t> ((std::int8_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize());\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint16_t>))\
{\
    functionName<std::uint16_t> ((std::uint16_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize());\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int16_t>))\
{\
    functionName<std::int16_t> ((std::int16_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize());\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint32_t>))\
{\
    functionName<std::uint32_t> ((std::uint32_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize());\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int32_t>))\
{\
    functionName<std::int32_t> ((std::int32_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize());\
    }\
    else\
{\
    throw std::runtime_error("Data type not valid");\
    }\
    }

#define HANDLER_CALL_TEMPLATE_FUNCTION_WITH_PARAMS(functionName, handlerPointer, ...)\
{\
    puntoexe::imebra::handlers::dataHandlerNumericBase* pHandler(handlerPointer.get()); \
    if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint8_t>) || \
    typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerRaw))\
{\
    functionName<std::uint8_t> ((std::uint8_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize(), __VA_ARGS__);\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int8_t>))\
{\
    functionName<std::int8_t> ((std::int8_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize(), __VA_ARGS__);\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint16_t>))\
{\
    functionName<std::uint16_t> ((std::uint16_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize(), __VA_ARGS__);\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int16_t>))\
{\
    functionName<std::int16_t> ((std::int16_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize(), __VA_ARGS__);\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::uint32_t>))\
{\
    functionName<std::uint32_t> ((std::uint32_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize(), __VA_ARGS__);\
    }\
    else if(typeid(*pHandler) == typeid(puntoexe::imebra::handlers::dataHandlerNumeric<std::int32_t>))\
{\
    functionName<std::int32_t> ((std::int32_t*)handlerPointer->getMemoryBuffer(), handlerPointer->getSize(), __VA_ARGS__);\
    }\
    else\
{\
    throw std::runtime_error("Data type not valid");\
    }\
    }


#endif // !defined(imebraDataHandlerNumeric_BD270581_5746_48d1_816E_64B700955A12__INCLUDED_)
