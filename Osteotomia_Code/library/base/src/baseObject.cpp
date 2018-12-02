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

/*! \file baseObject.cpp
    \brief Implementation of the baseObject class.

*/

#include "../include/baseObject.h"
#include "../include/exception.h"
#include <iostream>

namespace puntoexe
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// basePtr
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Default constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
basePtr::basePtr() : object(0)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor with initialization
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
basePtr::basePtr(baseObject* pObject): object(pObject)
{
	addRef();
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
basePtr::~basePtr()
{
	release();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Release
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void basePtr::release()
{
	if(object != 0)
	{
		object->release();
		object = 0;
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Increase reference counter
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void basePtr::addRef()
{
	if(object != 0)
	{
		object->addRef();
	}
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// baseObject
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
// Default constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
baseObject::baseObject(): m_lockCounter(0), m_bValid(true), m_pCriticalSection(new CObjectCriticalSection)
{
    m_pCriticalSection->addRef();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructs the object and set an external lock
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
baseObject::baseObject(const ptr<baseObject>& externalLock): 
	m_lockCounter(0), m_bValid(true)
{
    if(externalLock == 0)
    {
        m_pCriticalSection = new CObjectCriticalSection;
    }
    else
    {
        m_pCriticalSection = externalLock->m_pCriticalSection;
    }
    m_pCriticalSection->addRef();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return true if the object is referenced once.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool baseObject::isReferencedOnce()
{
	return m_lockCounter == 1;
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
baseObject::~baseObject()
{
    m_pCriticalSection->release();
    m_bValid = false;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Increase the references counter
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void baseObject::addRef()
{
    if(this != 0)
	{
        ++m_lockCounter;
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Decrease the references counter and delete the object
//  if the counter reaches 0
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void baseObject::release()
{
	// Calling release on a non-existing object.
	// Simply return
	///////////////////////////////////////////////////////////
	if(this == 0)
	{
		return;
	}

	// Decrease the reference counter
	///////////////////////////////////////////////////////////
    if(--m_lockCounter != 0)
    {
        return;
    }

	if(!preDelete())
	{
		return;
	}

	delete this;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// This function is called by release() just before
//  the object is deleted.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool baseObject::preDelete()
{
	return true;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Lock the object
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void baseObject::lock()
{
	if(this == 0)
	{
		return;
	}
	m_pCriticalSection->m_criticalSection.lock();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock the object
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void baseObject::unlock()
{
	if(this == 0)
	{
		return;
	}
	m_pCriticalSection->m_criticalSection.unlock();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// lockObject
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
// Lock the specified object
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lockObject::lockObject(baseObject* pObject)
{
	m_pObject = pObject;
	if(m_pObject != 0)
	{
		m_pObject->lock();
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock the specified object
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lockObject::~lockObject()
{
	unlock();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock the specified object
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lockObject::unlock()
{
	if( m_pObject != 0)
	{
		m_pObject->unlock();
		m_pObject = 0;
	}
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// lockMultipleObject
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
// Lock the specified objects
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lockMultipleObjects::lockMultipleObjects(tObjectsList* pObjectsList)
{
	tCriticalSectionsList csList;
	for(tObjectsList::iterator scanObjects = pObjectsList->begin(); scanObjects != pObjectsList->end(); ++scanObjects)
	{
		if((*scanObjects) == 0)
		{
			continue;
		}
		csList.push_back(&( (*scanObjects)->m_pCriticalSection->m_criticalSection) );
	}
	m_pLockedCS.reset(puntoexe::lockMultipleCriticalSections(&csList));
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock the locked objects
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
lockMultipleObjects::~lockMultipleObjects()
{
	unlock();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Unlock the locked objects
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void lockMultipleObjects::unlock()
{
	if(m_pLockedCS.get() == 0)
	{
		return;
	}
	puntoexe::unlockMultipleCriticalSections(m_pLockedCS.get());
        m_pLockedCS.reset();
}


} // namespace puntoexe
