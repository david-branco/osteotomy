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

/*! \file memory.h
    \brief Declaration of the memory manager and the memory class.

*/

#if !defined(imebraMemory_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
#define imebraMemory_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_

#include "baseObject.h"
#include "criticalSection.h"
#include <list>
#include <map>
#include <memory>
#include <array>

///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

/// \addtogroup group_baseclasses
///
/// @{

typedef std::basic_string<std::uint8_t> stringUint8;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class holds an allocated array of bytes.
///
/// New instances of this class should be obtained through
///  the class \ref memoryPool; 
///  call \ref memoryPool::getMemory() in order to
///  obtain a new instance of memory.
///
/// This class is usually used by \ref imebra::buffer 
///  objects to store the tags values.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class memory : public baseObject
{
public:
    /// \brief Construct an empty memory object
    ///
    ///////////////////////////////////////////////////////////
	memory();

    /// \brief Construct a memory object using the passed
    ///        buffer.
    ///
    /// Takes ownership of the buffer and deletes it in
    ///  the destructor.
    ///
    /// @param pBuffer buffer containing the data. The
    ///                memory object will take ownership
    ///                of it.
    ///
    ///////////////////////////////////////////////////////////
    memory(stringUint8* pBuffer);

    /// \brief Constructs the memory object and allocate
	///         the requested amount of memory
	///
	/// @param initialSize the initial size of the allocated
	///                      memory, in bytes
	///
	///////////////////////////////////////////////////////////
	memory(std::uint32_t initialSize);

    /// \brief Destruct the memory object.
    ///
    /// The owned buffer is passed to the memoryPool for
    ///  possible reuse: memoryPool will decide if to keep
    ///  the buffer or to delete it.
    ///
    ///////////////////////////////////////////////////////////
    ~memory();

	/// \brief Transfer the content from another memory object.
	///
	/// The source memory object will transfer the ownership
	///  of the managed memory to this object and then will
	///  reference an empty memory area (size = 0)
	///
	/// @param transferFrom the object from which the memory
	///                      must be transferred
	///
	///////////////////////////////////////////////////////////
	void transfer(const ptr<memory>& transferFrom);

	/// \brief Copy the content of the memory managed
	///         by another memory object into the memory 
	///         managed by this object.
	///
	/// @param sourceMemory a pointer to the memory object from
	///                      which the data has to be copied
	///
	///////////////////////////////////////////////////////////
	void copyFrom(const ptr<memory>& sourceMemory);

	/// \brief Clear the content of the memory object and
	///         set its size to 0 bytes.
	///
	///////////////////////////////////////////////////////////
	void clear();

	/// \brief Resize the memory buffer.
	///
	/// @param newSize  the new size of the buffer, in bytes
	///
	///////////////////////////////////////////////////////////
	void resize(std::uint32_t newSize);

	/// \brief Reserve the specified quantity of bytes for
	///         the memory object. This doesn't modify the
	///         actual size of the memory object.
	///
	/// @param reserveSize   the number of bytes to reserve for
	///                       the memory object.
	///
	///////////////////////////////////////////////////////////
	void reserve(std::uint32_t reserveSize);

	/// \brief Return the size of the managed
	///         memory in bytes.
	///
	/// @return the size of the managed memory, in bytes
	///
	///////////////////////////////////////////////////////////
	std::uint32_t size();

	/// \brief Return a pointer to the memory managed by the
	///         object.
	///
	/// @return a pointer to the data managed by the object
	///
	///////////////////////////////////////////////////////////
	std::uint8_t* data();

	/// \brief Return true if the size of the managed memory
	///         is 0.
	///
	/// @return true if the managed memory's size is 0 or
	///          false otherwise
	///
	///////////////////////////////////////////////////////////
	bool empty();

	/// \brief Copy the specified array of bytes into the
	///         managed memory.
	///
	/// @param pSource      a pointer to the buffer containing
	///                      the bytes to be copied
	/// @param sourceLength the number of bytes stored in the
	///                      buffer pSource and to be copied
	///                      into the managed memory
	///
	///////////////////////////////////////////////////////////
	void assign(const std::uint8_t* pSource, const std::uint32_t sourceLength);

protected:
    std::unique_ptr<stringUint8> m_pMemoryBuffer;
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Stores unused memory objects (see 
///         \ref puntoexe::memory) so they can be reused 
///         when needed.
///
/// One instance of this class is statically allocated
///  by the library. Don't allocate new instance of this
///  class.
///
/// To obtain a pointer to the statically allocated 
///  instance of memoryPool call the static function
///  memoryPool::getMemoryPool().
///
/// When the reference counter of a \ref memory object
///  reaches 0, the memory object may not be deleted 
///  immediatly; infact, if the memory managed by the 
///  memory pool matches some predefined requisites, the
///  memory object is temporarily stored in the memory
///  pool and reused when a request for a \ref memory
///  object is received.
///
/// The memory pool tries to reuse the \ref memory
///  object that manages an amount of memory similar to
///  the amount of memory requested through getMemory().
///
/// When a memory object is not used for a while then it
///  is deleted permanently.
///
///////////////////////////////////////////////////////////
class memoryPool
{
	friend class memory;
protected:
#if(!defined IMEBRA_MEMORY_POOL_SLOTS)
	#define IMEBRA_MEMORY_POOL_SLOTS 256
#endif
#if(!defined IMEBRA_MEMORY_POOL_MAX_SIZE)
	#define IMEBRA_MEMORY_POOL_MAX_SIZE 20000000
#endif
#if(!defined IMEBRA_MEMORY_POOL_MIN_SIZE)
	#define IMEBRA_MEMORY_POOL_MIN_SIZE 1024
#endif

    std::array<std::uint32_t, IMEBRA_MEMORY_POOL_SLOTS> m_memorySize;
    std::array<stringUint8*, IMEBRA_MEMORY_POOL_SLOTS>  m_memoryPointer;
	std::uint32_t m_firstUsedCell;
	std::uint32_t m_firstFreeCell;

	std::uint32_t m_actualSize;

public:
	memoryPool(): m_actualSize(0){}

	virtual ~memoryPool();

	/// \brief Retrieve a new or reused 
	///         \ref puntoexe::memory object.
	///
	/// The function look for an unused \ref memory object
	///  that has a managed string with the same size of the
	///  specified one and tries to reuse it.
	///
	/// If none of the unused objects has the requested
	///  size, then a new memory object is created and 
	///  returned.
	///
	/// @param requestedSize the size that the string managed
	///                       by the returned memory object
	///                       must have
	/// @return              a pointer to the reused or new
	///                       memory object: in any case the
	///                       reference counter of the returned
	///                       object will be 1
	///
	///////////////////////////////////////////////////////////
	memory* getMemory(std::uint32_t requestedSize);

	/// \brief Discard all the currently unused memory.
	///
	///////////////////////////////////////////////////////////
	void flush();

	/// \brief Get a pointer to the statically allocated 
	///         instance of memoryPool.
	///
	/// @return a pointer to the statically allocated instance
	///          of memoryPool
	///
	///////////////////////////////////////////////////////////
	static memoryPool* getMemoryPool();

protected:
	/// \internal
	/// \brief Called by \ref memory before the object
	///         is deleted.
	///
	/// This function returns true if the memory object will
	///  be reused and shouldn't be deleted.
	///
	/// @param pMemoryToReuse a pointer to the memory object
	///                        that call this function
	/// @return               true if the memory object has
	///                        been saved in the memory pool
	///                        and will be reused, false
	///                        otherwise
	///
	///////////////////////////////////////////////////////////
    bool reuseMemory(stringUint8* pMemoryToReuse);

	criticalSection m_criticalSection;

};

///@}

} // namespace puntoexe

#endif // !defined(imebraMemory_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
