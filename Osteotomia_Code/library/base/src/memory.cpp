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

/*! \file memory.cpp
    \brief Implementation of the memory manager and the memory class.

*/

#include "../include/memory.h"

namespace puntoexe
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// memory
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
// Constructors
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
memory::memory():
	m_pMemoryBuffer(new stringUint8)
{
}

memory::memory(stringUint8* pBuffer):
    m_pMemoryBuffer(pBuffer)
{
}

memory::memory(std::uint32_t initialSize):
    m_pMemoryBuffer(new stringUint8((size_t)initialSize, 0))
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
memory::~memory()
{
    memoryPool::getMemoryPool()->reuseMemory(m_pMemoryBuffer.release());
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Detach a managed string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memory::transfer(const ptr<memory>& transferFrom)
{
	m_pMemoryBuffer.reset(transferFrom->m_pMemoryBuffer.release());
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Copy the content from another memory object
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memory::copyFrom(const ptr<memory>& sourceMemory)
{
	if(m_pMemoryBuffer.get() == 0)
	{
		m_pMemoryBuffer.reset(new stringUint8);
	}
	m_pMemoryBuffer->assign(sourceMemory->data(), sourceMemory->size());
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Clear the managed string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memory::clear()
{
	if(m_pMemoryBuffer.get() != 0)
	{
		m_pMemoryBuffer->clear();
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Resize the memory buffer
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memory::resize(std::uint32_t newSize)
{
	if(m_pMemoryBuffer.get() == 0)
	{
		m_pMemoryBuffer.reset(new stringUint8((size_t)newSize, (std::uint8_t)0));
	}
	else
	{
	    m_pMemoryBuffer->resize((size_t)newSize, (std::uint8_t)0);
	}

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Reserve memory
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memory::reserve(std::uint32_t reserveSize)
{
	if(m_pMemoryBuffer.get() == 0)
	{
		m_pMemoryBuffer.reset(new stringUint8);
	}
	m_pMemoryBuffer->reserve(reserveSize);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the size of the managed string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t memory::size()
{
	if(m_pMemoryBuffer.get() == 0)
	{
		return 0;
	}
	return (std::uint32_t)(m_pMemoryBuffer->size());
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return a pointer to the data
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint8_t* memory::data()
{
	if(m_pMemoryBuffer.get() == 0 || m_pMemoryBuffer->empty())
	{
		return 0;
	}
	return &( ( (*m_pMemoryBuffer.get()))[0]);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return true if the managed string is empty
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool memory::empty()
{
	return m_pMemoryBuffer.get() == 0 || m_pMemoryBuffer->empty();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Copy the content of a buffer in the managed string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memory::assign(const std::uint8_t* pSource, const std::uint32_t sourceLength)
{
	if(m_pMemoryBuffer.get() == 0)
	{
		m_pMemoryBuffer.reset(new stringUint8);
	}
	m_pMemoryBuffer->assign(pSource, sourceLength);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// memoryPool
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
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
memoryPool::~memoryPool()
{
	while(m_actualSize != 0)
	{
		m_actualSize -= m_memorySize[m_firstUsedCell];
		delete m_memoryPointer[m_firstUsedCell++];
		if(m_firstUsedCell == IMEBRA_MEMORY_POOL_SLOTS)
		{
			m_firstUsedCell = 0;
		}
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Save a memory object to reuse it
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool memoryPool::reuseMemory(stringUint8* pString)
{
    if(pString == 0)
    {
        return false;
    }
    std::unique_ptr<stringUint8> pBuffer(pString);

	// Check for the memory size. Don't reuse it if the memory
	//  doesn't match the requested parameters
	///////////////////////////////////////////////////////////
    std::uint32_t memorySize = pBuffer->size();
	if(memorySize == 0 || memorySize < IMEBRA_MEMORY_POOL_MIN_SIZE || memorySize > IMEBRA_MEMORY_POOL_MAX_SIZE)
	{
		return false;
	}

	// Ok to reuse
	///////////////////////////////////////////////////////////
	lockCriticalSection lockThis(&m_criticalSection);

	// Store the memory object in the pool
	///////////////////////////////////////////////////////////
	m_memorySize[m_firstFreeCell] = memorySize;
    m_memoryPointer[m_firstFreeCell] = pBuffer.release();
	m_actualSize += memorySize;
    if(++m_firstFreeCell >= m_memorySize.size())
	{
		m_firstFreeCell = 0;
	}

	// Remove old unused memory objects
	///////////////////////////////////////////////////////////
	if(m_firstFreeCell == m_firstUsedCell)
	{
		m_actualSize -= m_memorySize[m_firstUsedCell];
		delete m_memoryPointer[m_firstUsedCell];
        if(++m_firstUsedCell >= m_memorySize.size())
		{
			m_firstUsedCell = 0;
		}
	}

	// Remove old unused memory objects if the total unused
	//  memory is bigger than the specified parameters
	///////////////////////////////////////////////////////////
	while(m_actualSize != 0 && m_actualSize > IMEBRA_MEMORY_POOL_MAX_SIZE)
	{
		m_actualSize -= m_memorySize[m_firstUsedCell];
		delete m_memoryPointer[m_firstUsedCell];
		if(++m_firstUsedCell >= IMEBRA_MEMORY_POOL_SLOTS)
		{
			m_firstUsedCell = 0;
		}
	}

	return true;

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Discard the currently unused memory
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memoryPool::flush()
{
	lockCriticalSection lockThis(&m_criticalSection);

	while(m_firstUsedCell != m_firstFreeCell)
	{
		delete m_memoryPointer[m_firstUsedCell];
		m_actualSize -= m_memorySize[m_firstUsedCell];
        if(++m_firstUsedCell >= m_memorySize.size())
		{
			m_firstUsedCell = 0;
		}
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a pointer to the unique instance of the memoryPool
//  class
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
memoryPool* memoryPool::getMemoryPool()
{
	static memoryPool m_staticMemoryPool;

	return &m_staticMemoryPool;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Look for a memory object to reuse or allocate a new one
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
memory* memoryPool::getMemory(std::uint32_t requestedSize)
{
	lockCriticalSection lockThis(&m_criticalSection);

	// Look for an object to reuse
	///////////////////////////////////////////////////////////
	for(std::uint32_t findCell = m_firstUsedCell; findCell != m_firstFreeCell;)
	{
		if(m_memorySize[findCell] != requestedSize)
		{
            if(++findCell >= m_memorySize.size())
			{
				findCell = 0;
			}
			continue;
		}

		// Memory found
		///////////////////////////////////////////////////////////
        memory* pMemory = new memory(m_memoryPointer[findCell]);
		m_actualSize -= m_memorySize[findCell];
		if(findCell == m_firstUsedCell)
		{
			if(++m_firstUsedCell >= IMEBRA_MEMORY_POOL_SLOTS)
			{
				m_firstUsedCell = 0;
			}
			return pMemory;
		}

		std::uint32_t lastUsedCell = m_firstFreeCell == 0 ? (IMEBRA_MEMORY_POOL_SLOTS - 1) : (m_firstFreeCell - 1);
		if(findCell == lastUsedCell)
		{
			m_firstFreeCell = lastUsedCell;
			return pMemory;
		}

		m_memorySize[findCell] = m_memorySize[m_firstUsedCell];
		m_memoryPointer[findCell] = m_memoryPointer[m_firstUsedCell];
		if(++m_firstUsedCell >= IMEBRA_MEMORY_POOL_SLOTS)
		{
			m_firstUsedCell = 0;
		}
		return pMemory;
	}

    try
    {
        return new memory(requestedSize);
    }
    catch(const std::bad_alloc& e)
    {
        // If an allocation error occurred, then free the cached memory and try again
        /////////////////////////////////////////////////////////////////////////////
        flush();
        return new memory(requestedSize);
    }

}

} // namespace puntoexe
