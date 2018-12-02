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

/*! \file dicomDir.cpp
    \brief Implementation of the classes dicomDir and directoryRecord.

*/

#include "../include/dicomDir.h"
#include "../include/dataSet.h"
#include "../include/dicomCodec.h"
#include "../include/dataHandlerNumeric.h"
#include "../../base/include/nullStream.h"
#include "../../base/include/streamWriter.h"
#include <map>
#include <string>

namespace puntoexe
{

namespace imebra
{

	struct tDirectoryRecordTypeDef
	{
		std::wstring m_name;
		directoryRecord::tDirectoryRecordType m_type;
	};
	
	static const tDirectoryRecordTypeDef typesList[] =
	{
		{L"PATIENT", directoryRecord::patient},
		{L"STUDY", directoryRecord::study},
		{L"SERIES", directoryRecord::series},
		{L"IMAGE", directoryRecord::image},
		{L"OVERLAY", directoryRecord::overlay},
		{L"MODALITY LUT", directoryRecord::modality_lut},
		{L"VOI LUT", directoryRecord::voi_lut},
		{L"CURVE", directoryRecord::curve},
		{L"TOPIC", directoryRecord::topic},
		{L"VISIT", directoryRecord::visit},
		{L"RESULTS", directoryRecord::results},
		{L"INTERPRETATION", directoryRecord::interpretation},
		{L"STUDY COMPONENT", directoryRecord::study_component},
		{L"STORED PRINT", directoryRecord::stored_print},
		{L"RT DOSE", directoryRecord::rt_dose},
		{L"RT STRUCTURE SET", directoryRecord::rt_structure_set},
		{L"RT PLAN", directoryRecord::rt_plan},
		{L"RT TREAT RECORD", directoryRecord::rt_treat_record},
		{L"PRESENTATION", directoryRecord::presentation},
		{L"WAVEFORM", directoryRecord::waveform},
		{L"SR DOCUMENT", directoryRecord::sr_document},
		{L"KEY OBJECT DOC", directoryRecord::key_object_doc},
		{L"SPECTROSCOPY", directoryRecord::spectroscopy},
		{L"RAW DATA", directoryRecord::raw_data},
		{L"REGISTRATION", directoryRecord::registration},
		{L"FIDUCIAL", directoryRecord::fiducial},
		{L"MRDR", directoryRecord::mrdr},
		{L"", directoryRecord::endOfDirectoryRecordTypes}
	};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// directoryRecord
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
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
directoryRecord::directoryRecord(ptr<dataSet> pDataSet):
	m_pDataSet(pDataSet)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the record's dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<dataSet> directoryRecord::getRecordDataSet()
{
	return m_pDataSet;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the next sibling record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<directoryRecord> directoryRecord::getNextRecord()
{
	return m_pNextRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the first child record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<directoryRecord> directoryRecord::getFirstChildRecord()
{
	return m_pFirstChildRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the referenced record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<directoryRecord> directoryRecord::getReferencedRecord()
{
	return m_pReferencedRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Sets the next sibling record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setNextRecord(ptr<directoryRecord> pNextRecord)
{
	if(pNextRecord != 0)
	{
		pNextRecord->checkCircularReference(this);
	}
	m_pNextRecord = pNextRecord;
}
	

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the first child record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setFirstChildRecord(ptr<directoryRecord> pFirstChildRecord)
{
	if(pFirstChildRecord != 0)
	{
		pFirstChildRecord->checkCircularReference(this);
	}
	m_pFirstChildRecord = pFirstChildRecord;
}
	

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Sets the referenced record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setReferencedRecord(ptr<directoryRecord> pReferencedRecord)
{
	if(pReferencedRecord != 0)
	{
		pReferencedRecord->checkCircularReference(this);
	}
	m_pReferencedRecord = pReferencedRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns a part of the full file path
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring directoryRecord::getFilePart(std::uint32_t part)
{
	return getRecordDataSet()->getUnicodeString(0x0004, 0, 0x1500, part);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a part of the full file path
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setFilePart(std::uint32_t part, const std::wstring partName)
{
    getRecordDataSet()->setUnicodeString(0x0004, 0, 0x1500, part, partName);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the record's type (enum)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
directoryRecord::tDirectoryRecordType directoryRecord::getType()
{
	std::wstring typeString(getTypeString());

	for(size_t scanTypes(0); typesList[scanTypes].m_type != endOfDirectoryRecordTypes; ++scanTypes)
	{
		if(typesList[scanTypes].m_name == typeString)
		{
			return typesList[scanTypes].m_type;
		}
	}

	// Invalid value found . Throw an exception
	///////////////////////////////////////////////////////////
	throw dicomDirUnknownDirectoryRecordType("Unknown directory record type");
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the record's type (string)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring directoryRecord::getTypeString()
{
	return getRecordDataSet()->getUnicodeString(0x0004, 0, 0x1430, 0);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the item's type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setType(tDirectoryRecordType recordType)
{
	for(size_t scanTypes(0); typesList[scanTypes].m_type != endOfDirectoryRecordTypes; ++scanTypes)
	{
		if(typesList[scanTypes].m_type == recordType)
		{
			getRecordDataSet()->setUnicodeString(0x0004, 0, 0x1430, 0, typesList[scanTypes].m_name);
			return;
		}
	}

	// Trying to set an invalid type. Throw an exception
	///////////////////////////////////////////////////////////
	throw dicomDirUnknownDirectoryRecordType("Unknown directory record type");
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the item's type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setTypeString(std::wstring recordType)
{
	getRecordDataSet()->setUnicodeString(0x0004, 0, 0x1430, 0, recordType);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Update the dataSet's offsets. 
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::updateOffsets()
{
	// Update offset for the next record
	///////////////////////////////////////////////////////////
	if(m_pNextRecord == 0)
	{
		getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1400, 0, 0);
	}
	else
	{
		getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1400, 0, m_pNextRecord->getRecordDataSet()->getItemOffset());
		m_pNextRecord->updateOffsets();
	}

	// Update offset for the first child record
	///////////////////////////////////////////////////////////
	if(m_pFirstChildRecord == 0)
	{
		getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1420, 0, 0);
	}
	else
	{
		getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1420, 0, m_pFirstChildRecord->getRecordDataSet()->getItemOffset());
		m_pFirstChildRecord->updateOffsets();
	}

	// Update offset for the referenced record
	///////////////////////////////////////////////////////////
	if(m_pReferencedRecord == 0)
	{
		getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1504, 0, 0);
	}
	else
	{
		getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1504, 0, m_pReferencedRecord->getRecordDataSet()->getItemOffset());
		m_pReferencedRecord->updateOffsets();
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Check circular (recursive) references between the
//  dicomdir's items
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::checkCircularReference(directoryRecord* pStartRecord)
{
	if(this == pStartRecord)
	{
		throw dicomDirCircularReferenceException("Circular reference detected");
	}

	if(m_pNextRecord != 0)
	{
		m_pNextRecord->checkCircularReference(pStartRecord);
	}

	if(m_pFirstChildRecord != 0)
	{
		m_pFirstChildRecord->checkCircularReference(pStartRecord);
	}

	if(m_pReferencedRecord != 0)
	{
		m_pReferencedRecord->checkCircularReference(pStartRecord);
	}
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dicomDir class
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
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
dicomDir::dicomDir(ptr<dataSet> pDataSet):
	m_pDataSet(pDataSet)
{
	if(m_pDataSet == 0)
	{
		m_pDataSet =new dataSet;
	}

	// Parse the dataset
	///////////////////////////////////////////////////////////
	lockObject lockDataSet(pDataSet);

	// Get the DICOMDIR sequence
	///////////////////////////////////////////////////////////
	typedef std::map<std::uint32_t, ptr<directoryRecord> > tOffsetsToRecords;
	tOffsetsToRecords offsetsToRecords;
	for(std::uint32_t scanItems(0); ; ++scanItems)
	{
		ptr<dataSet> pDataSet(m_pDataSet->getSequenceItem(0x0004, 0, 0x1220, scanItems));
		if(pDataSet == 0)
		{
			break;
		}
		ptr<directoryRecord> newRecord(new directoryRecord(pDataSet));
		offsetsToRecords[pDataSet->getItemOffset()] = newRecord;
		m_recordsList.push_back(newRecord);
	}

	// Scan all the records and update the pointers
	///////////////////////////////////////////////////////////
	for(tOffsetsToRecords::iterator scanRecords(offsetsToRecords.begin()); scanRecords != offsetsToRecords.end(); ++scanRecords)
	{
		std::uint32_t nextRecordOffset(scanRecords->second->getRecordDataSet()->getUnsignedLong(0x0004, 0, 0x1400, 0));
		std::uint32_t childRecordOffset(scanRecords->second->getRecordDataSet()->getUnsignedLong(0x0004, 0, 0x1420, 0));
		std::uint32_t referencedRecordOffset(scanRecords->second->getRecordDataSet()->getUnsignedLong(0x0004, 0, 0x1504, 0));

		tOffsetsToRecords::iterator findNextRecord(offsetsToRecords.find(nextRecordOffset));
		if(findNextRecord != offsetsToRecords.end())
		{
			scanRecords->second->setNextRecord(findNextRecord->second);
		}

		tOffsetsToRecords::iterator findChildRecord(offsetsToRecords.find(childRecordOffset));
		if(findChildRecord != offsetsToRecords.end())
		{
			scanRecords->second->setFirstChildRecord(findChildRecord->second);
		}

		tOffsetsToRecords::iterator findReferencedRecord(offsetsToRecords.find(referencedRecordOffset));
		if(findReferencedRecord != offsetsToRecords.end())
		{
			scanRecords->second->setReferencedRecord(findReferencedRecord->second);
		}
	}

	// Get the position of the first record
	///////////////////////////////////////////////////////////
	tOffsetsToRecords::iterator findRecord(offsetsToRecords.find(m_pDataSet->getUnsignedLong(0x0004, 0, 0x1200, 0)));
	if(findRecord == offsetsToRecords.end())
	{
		return;
	}
	setFirstRootRecord(findRecord->second);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor, make sure that the records are released
//  after the root record.
// This resolves a stack overflow when a large number
//  sibling records is present
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
dicomDir::~dicomDir()
{
	m_pFirstRootRecord.release();
	m_recordsList.clear();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the dataSet
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<dataSet> dicomDir::getDirectoryDataSet()
{
	return m_pDataSet;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns a new record that can be inserted in the
//  folder
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<directoryRecord> dicomDir::getNewRecord()
{
	ptr<data> recordsTag(m_pDataSet->getTag(0x0004, 0, 0x1220, true));
	ptr<dataSet> recordDataSet(new dataSet);
	recordsTag->appendDataSet(recordDataSet);

	ptr<directoryRecord> newRecord(new directoryRecord(recordDataSet));
	m_recordsList.push_back(newRecord);

	return newRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build the a dicom dataset that contain the directory's
//  information.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<dataSet> dicomDir::buildDataSet()
{
	// Adjust the transfer syntax if it isn't already set
	///////////////////////////////////////////////////////////
	if(m_pDataSet->getString(0x2, 0, 0x10, 0) == "")
	{
		m_pDataSet->setString(0x2, 0, 0x10, 0, "1.2.840.10008.1.2.1");
	}

	// Adjust the version if it isn't already set
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandlerRaw> versionHandler(m_pDataSet->getDataHandlerRaw(0x2, 0, 0x1, 0, true, "OB"));
	if(versionHandler->getSize() != 2)
	{
		versionHandler->setSize(2);
		versionHandler->setUnsignedLong(0, 0);
		versionHandler->setUnsignedLong(1, 1);
	}
	versionHandler.release();

	// Adjust the SOP class UID if it isn't already set
	///////////////////////////////////////////////////////////
	if(m_pDataSet->getString(0x2, 0, 0x2, 0) == "")
	{
		m_pDataSet->setString(0x2, 0, 0x2, 0, "1.2.840.10008.1.3.10");
	}

	// Allocate offset fields
	///////////////////////////////////////////////////////////
	if(m_pFirstRootRecord != 0)
	{
		m_pFirstRootRecord->updateOffsets();
	}
	m_pDataSet->setUnsignedLong(0x0004, 0, 0x1200, 0, 0);


	// Save to a null stream in order to update the offsets
	///////////////////////////////////////////////////////////
	ptr<nullStream> saveStream(new nullStream);
	ptr<streamWriter> writer(new streamWriter(saveStream));
	ptr<codecs::dicomCodec> writerCodec(new codecs::dicomCodec);
	writerCodec->write(writer, m_pDataSet);

	// Scan all the records and update the pointers
	///////////////////////////////////////////////////////////
	if(m_pFirstRootRecord != 0)
	{
		m_pFirstRootRecord->updateOffsets();
		m_pDataSet->setUnsignedLong(0x0004, 0, 0x1200, 0, m_pFirstRootRecord->getRecordDataSet()->getItemOffset());
	}

	return m_pDataSet;

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the first root record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
ptr<directoryRecord> dicomDir::getFirstRootRecord()
{
	return m_pFirstRootRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the first root record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomDir::setFirstRootRecord(ptr<directoryRecord> pFirstRootRecord)
{
	m_pFirstRootRecord = pFirstRootRecord;
}



} // namespace imebra

} // namespace puntoexe
