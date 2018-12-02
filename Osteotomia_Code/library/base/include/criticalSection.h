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

/*! \file criticalSection.h
    \brief Declaration of the functions that handle the the mutexes.

*/

#if !defined(CImbxCriticalSection_5092DA6B_EF16_4EF9_A1CF_DC8651AA7873__INCLUDED_)
#define CImbxCriticalSection_5092DA6B_EF16_4EF9_A1CF_DC8651AA7873__INCLUDED_

#include "configuration.h"

#ifdef PUNTOEXE_WINDOWS
#include "windows.h"
#else
#include <pthread.h> // this must be the first included file
#endif

#include <exception>
#include <stdexcept>
#include <list>

#include <typeinfo>

namespace puntoexe
{

/// \addtogroup group_baseclasses
///
/// @{

#ifdef PUNTOEXE_WINDOWS
	typedef CRITICAL_SECTION tCriticalSection;
#else
	typedef pthread_mutex_t tCriticalSection;
#endif


/// \brief This class represents a critical section.
///
/// Critical sections can be used by different threads to
///  lock the access to shared resources.
///
/// Use the class lockCriticalSection to safely lock
///  a critical section; lockCriticalSection unlocks the
///  locked critical section in its destructor, therefore
///  it is safe to use it in code that throw exceptions.
///
///////////////////////////////////////////////////////////
class criticalSection
{
public:
	/// \brief Construct and initializes the critical section.
	///
	/// The critical section is initially not locked. It can be
	///  locked by calling lock() or tryLock() or by using the 
	///  class lockCriticalSection (preferred).
	///
	///////////////////////////////////////////////////////////
	criticalSection();

	/// \brief Deallocates the critical section.
	///
	/// A system crash is likely to happens in the case a
	///  critical section is destroyed while it is in a locked 
	///  state. For this reason the class lockCriticalSection
	///  should be used instead of calling directly lock(),
	///  unlock() or tryLock().
	///
	///////////////////////////////////////////////////////////
	virtual ~criticalSection();

	/// \brief Lock the critical section.
	///
	/// If the critical section is already locked by another
	///  thread then the function waits for the critical 
	///  section to become available and then locks it and set
	///  its reference conter to 1.
	///
	/// If the critical section is unlocked then the fuction
	///  locks it and set its reference counter to 1.
	///
	/// If the critical section is already locked by the same
	///  thread that called lock(), then the function simply
	///  increases the reference counter by one.
	///
	/// Use unlock() to unlock a locked critical section.
	/// 
	///////////////////////////////////////////////////////////
	void lock();

	/// \brief Decreases the reference counter of the critical
	///         section and unlocks it if the counter reaches 
	///         0.
	///
	/// Use lock() to lock the critical section.
	///
	///////////////////////////////////////////////////////////
	void unlock();

	/// \brief Try to lock a critical section.
	///
	/// If the critical section is already locked in another
	///  thread then the function returns immediatly the value
	///  false.
	///
	/// If the critical section is not locked, then the 
	///  function locks the critical section, set the reference
	///  counter to 1 and returns true.
	///
	/// If the critical section is locked by the same thread
	///  that called tryLock() then the reference counter is
	///  increased by one and the function returns true.
	///
	/// @return true if the section has been succesfully locked
	///          or false otherwise
	///
	///////////////////////////////////////////////////////////
	bool tryLock();

private:
	tCriticalSection m_criticalSection;
};


/// \brief This class locks a critical section in the
///         constructor and unlocks it in the destructor.
///
/// This helps to correctly release a critical section in
///  case of exceptions or premature exit from a function
///  that uses the critical section.
///
///////////////////////////////////////////////////////////
class lockCriticalSection
{
public:
	/// \brief Creates the object lockCriticalSection and
	///         lock the specified criticalSection.
	///
	/// @param pCriticalSection a pointer to the 
	///                          criticalSection that has to
	///                          be locked
	///
	///////////////////////////////////////////////////////////
	lockCriticalSection(criticalSection* pCriticalSection);
	
	/// \brief Destroy the object lockCriticalSection and
	///         unlock the previously locked criticalSection.
	///
	///////////////////////////////////////////////////////////
	virtual ~lockCriticalSection();
private:
	criticalSection* m_pCriticalSection;
};


/// \internal
/// \brief Represents a list of critical sections.
///
/// It is used by lockMultipleCriticalSections() and
///  unlockMultipleCriticalSections().
///
///////////////////////////////////////////////////////////
typedef std::list<criticalSection*> tCriticalSectionsList;

/// \internal
/// \brief Lock a collection of critical sections.
///
/// The list can contain several pointers to the critical
///  sections that must be locked; the function tries to
///  lock all the critical sections in the list until it
///  succeedes.
///
/// The critical sections locked with 
///  lockMultipleCriticalSections() should be unlocked by
///  unlockMultipleCriticalSections().
///
/// @param pList a list of the critical sections that must
///               be locked
/// @return      a pointer to a list that must be passed
///               to unlockMultipleCriticalSections()
///
///////////////////////////////////////////////////////////
tCriticalSectionsList* lockMultipleCriticalSections(tCriticalSectionsList* pList);

/// \internal
/// \brief Unlock a collection of critical sections
///         locked by lockMultipleCriticalSections().
///
/// @param pList a pointer the list returned by
///               lockMultipleCriticalSections().
///
///////////////////////////////////////////////////////////
void unlockMultipleCriticalSections(tCriticalSectionsList* pList);

/// \internal
/// \brief Exception thrown when a posix mutex is in an
///         error state.
///
///////////////////////////////////////////////////////////
class posixMutexException: public std::runtime_error
{
public:
	posixMutexException(const std::string& message): std::runtime_error(message){}
};

///@}

} // namespace puntoexe

#endif // !defined(CImbxCriticalSection_5092DA6B_EF16_4EF9_A1CF_DC8651AA7873__INCLUDED_)
