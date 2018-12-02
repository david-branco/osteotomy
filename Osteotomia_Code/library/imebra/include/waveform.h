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

/*! \file waveform.h
    \brief Declaration of the class waveform.

*/

#if !defined(imebraWaveform_A807A3CA_FA04_44f4_85D2_C7AA2FE103C4__INCLUDED_)
#define imebraWaveform_A807A3CA_FA04_44f4_85D2_C7AA2FE103C4__INCLUDED_

#include "../../base/include/baseObject.h"


///////////////////////////////////////////////////////////
//
// Everything is in the namespace puntoexe::imebra
//
///////////////////////////////////////////////////////////
namespace puntoexe
{

namespace imebra
{

class dataSet;
namespace handlers
{
	class dataHandler;
}

/// \addtogroup group_waveform Waveforms
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a single waveform of a dicom dataset. 
///
/// Waveforms are embedded into the dicom structures
///  (represented by the dataSet class), stored in sequence
///  items (one waveform per item).
///
/// Use dataSet::getWaveform() to retrieve a waveform from
///  a dataSet.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class waveform : public baseObject
{
public:
	/// \brief Constructor. Initializes the class and connects
	///         it to the sequence item containing the waveform
	///         data.
	///
	/// @param pDataSet   the sequence item containing the
	///                    waveform. 
	///                   Use dataSet::getSequenceItem() to
	///                    retrieve the sequence item 
	///                    containing the waveform or
	///                    construct the class with
	///                    dataSet::getWaveform()
	///
	///////////////////////////////////////////////////////////
	waveform(ptr<dataSet> pDataSet);

	/// \brief Retrieve the number of channels (tag 003A,0005).
	///
	/// @return the number of channels
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getChannels();

	/// \brief Retrieve the number of samples per channel
	///         (tag 003A,0010).
	///
	/// @return the number of samples per channel
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getSamples();

	/// \brief Retrieve the number of bit stored 
	///         (tag 003A,021A).
	///
	/// @return the number of bits stored
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getBitsStored();

	/// \brief Retrieve the number of bit allocated
	///         (tag 5400,1004).
	///
	/// @return the number of bits allocated
	///
	///////////////////////////////////////////////////////////
	std::uint32_t getBitsAllocated();

	/// \brief Return the data interpretation string 
	///         (tag 5400,1006).
	///
	/// @return the interpretation string. Possible values are:
	/// - "SB": signed 8 bit linear
	/// - "UB": unsigned 8 bit linear
	/// - "MB": 8 bit u-law 
	/// - "AB": 8 bit a-law
	/// - "SS": signed 16 bit
	/// - "US": unsigned 16 bit
	///
	///////////////////////////////////////////////////////////
	std::string getInterpretation();

	/// \brief Retrieve the decompressed waveform data.
	///
	/// Retrieve the requested channel's data and decompress
	///  it into signed long values. 8 bits uLaw and aLaw data 
	///  are decompressed into normalized 16 bits values.
	///
	/// This function takes into account the value in the 
	///  interpretation tag and returns an handler to ready to
	///  use data.
	///
	/// @param channel      the channel for which the data is
	///                      required
	/// @param paddingValue the value that the function must
	///                      write in the returned data in
	///                      place of the original padding
	///                      value. Specify a number
	///                      outside the range -32768..65535
	/// @return a data handler attached to ready to use data.
	///        Use handlers::dataHandler->GetSignedLong() or 
	///        handlers::dataHandler->GetSignedLongIncPointer()
	///        to retrieve the data
	///
	///////////////////////////////////////////////////////////
	ptr<handlers::dataHandler> getIntegerData(std::uint32_t channel, std::int32_t paddingValue = 0x7fffffff);

	/// \brief Return the sequence item used by the waveform.
	///
	/// @return the sequence item used by the waveform
	///
	///////////////////////////////////////////////////////////
	ptr<dataSet> GetWaveformItem();
	
private:
	ptr<dataSet> m_pDataSet;
};

/// @}

} // namespace imebra

} // namespace puntoexe

#endif // !defined(imebraWaveform_A807A3CA_FA04_44f4_85D2_C7AA2FE103C4__INCLUDED_)
