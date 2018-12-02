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

/*! \file criticalSection.cpp
    \brief Implementation of the critical sections.

*/

#include "../include/criticalSection.h"
#include "../include/exception.h"
#include <map>
#include <memory>

#ifdef PUNTOEXE_WINDOWS
#include <process.h>
#endif

#ifdef PUNTOEXE_POSIX
#include <sched.h>
#include <errno.h>
#endif

namespace puntoexe
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Global functions
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
// Lock several critical sections
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tCriticalSectionsList* lockMultipleCriticalSections(tCriticalSectionsList* pList)
{
	PUNTOEXE_FUNCTION_START(L"lockMultipleCriticalSections");

	// Build a map of the involved critical sections.
	// The map is ordered by the pointer's value
	///////////////////////////////////////////////////////////
	typedef std::map<criticalSection*, bool> tCriticalSectionsMap;
	tCriticalSectionsMap CSmap;
	for(tCriticalSectionsList::iterator copyCS = pList->begin(); copyCS != pList->end(); ++copyCS)
	{
		CSmap[*copyCS] = true;
	}

	// Build a list that lists all the locked critical sections
	///////////////////////////////////////////////////////////
	std::unique_ptr<tCriticalSectionsList> pLockedList(new tCriticalSectionsList);

	// Use the normal lockCriticalSection if the list contains
	//  only one critical section
	///////////////////////////////////////////////////////////
	if(CSmap.size() == 1)
	{
		CSmap.begin()->first->lock();
		pLockedList->push_back(CSmap.begin()->first);
		return pLockedList.release();
	}

	// Try to lock all the critical sections. Give way to
	//  other threads if one lock fails, then retry.
	///////////////////////////////////////////////////////////
	for(bool bOK = false; !bOK; /* empty */)
	{
		pLockedList->clear();
		bOK = true;
		for(tCriticalSectionsMap::iterator lockCS = CSmap.begin(); lockCS != CSmap.end(); ++lockCS)
		{
			if(lockCS->first->tryLock())
			{
				pLockedList->push_back(lockCS->first);
				continue;
			}

			bOK = false;
			unlockMultipleCriticalSections(pLockedList.get());

#ifdef PUNTOEXE_WINDOWS // WINDOWS

#if(_WIN32_WINNT>=0x0400)
			SwitchToThread();
#else
			Sleep(0);
#endif

#else // POSIX
			sched_yield();
#endif
			break;
		}
	}

	return pLockedList.release();

	PUNTOEXE_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock several critical sections
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void unlockMultipleCriticalSections(tCriticalSectionsList* pList)
{
	for(tCriticalSectionsList::reverse_iterator unlockCS = pList->rbegin(); unlockCS != pList->rend(); ++unlockCS)
	{
		(*unlockCS)->unlock();
	}
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// lockCriticalSection
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
// Constructor: Lock a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lockCriticalSection::lockCriticalSection(criticalSection* pCriticalSection): m_pCriticalSection(pCriticalSection)
{
	m_pCriticalSection->lock();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor: unlock a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lockCriticalSection::~lockCriticalSection()
{
	m_pCriticalSection->unlock();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// criticalSection
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
// Initialize a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
criticalSection::criticalSection()
{
#ifdef PUNTOEXE_WINDOWS
	InitializeCriticalSection(&m_criticalSection);
#else
	pthread_mutexattr_t criticalSectionAttribute;

	pthread_mutexattr_init(&criticalSectionAttribute );
	pthread_mutexattr_settype(&criticalSectionAttribute, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_criticalSection, &criticalSectionAttribute );
	pthread_mutexattr_destroy(&criticalSectionAttribute );
#endif
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Uninitialize a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
criticalSection::~criticalSection()
{
#ifdef PUNTOEXE_WINDOWS
	DeleteCriticalSection(&m_criticalSection);
#else
	pthread_mutex_destroy(&m_criticalSection);
#endif
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Lock a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void criticalSection::lock()
{
#ifdef PUNTOEXE_WINDOWS
	EnterCriticalSection(&m_criticalSection);
#else
	pthread_mutex_lock(&m_criticalSection);
#endif
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void criticalSection::unlock()
{
#ifdef PUNTOEXE_WINDOWS
	LeaveCriticalSection(&m_criticalSection);
#else
	pthread_mutex_unlock(&m_criticalSection);
#endif
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Try to lock a critical section
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool criticalSection::tryLock()
{

#ifdef PUNTOEXE_WINDOWS // WINDOWS
	return TryEnterCriticalSection(&m_criticalSection) != 0;

#else // POSIX

	PUNTOEXE_FUNCTION_START(L"criticalSection::tryLock");

	int tryLockResult = pthread_mutex_trylock(&m_criticalSection);
	if(tryLockResult == 0)
	{
		return true;
	}
	if(tryLockResult == EBUSY)
	{
		return false;
	}
	PUNTOEXE_THROW(posixMutexException, "A mutex is in an error state");

	PUNTOEXE_FUNCTION_END();
#endif

}

} // namespace puntoexe
