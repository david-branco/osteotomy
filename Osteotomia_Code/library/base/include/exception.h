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

/*! \file exception.h
    \brief Defines the classes and the macros used to handle the
	        exceptions in the library.

*/

#if !defined(CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
#define CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_

#include "configuration.h"
#include "baseObject.h"

#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <thread>

namespace puntoexe
{

/// \addtogroup group_baseclasses
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class is used to store the information
///         related to the points where the exceptions are
///         being caught&thrown.
///
/// The application can use the macros
///  PUNTOEXE_FUNCTION_START(),
///  PUNTOEXE_FUNCTION_END(), 
///  PUNTOEXE_THROW() and
///  PUNTOEXE_RETHROW()
///  inside its function: the macros take care of logging
///  in the exceptions manager the source code's lines
///  travelled by an exception while it is being thrown
///  to the catch point.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class exceptionInfo
{
public:
	/// \brief Construct the exceptionInfo object.
	///
	/// @param functionName  the name of the function where
	///                       the catch&throw is happening
	/// @param fileName      the name of the file where the
	///                       catch&throw is happening
	/// @param lineNumber    the line where the catch&throw
	///                       is happening
	/// @param exceptionType the type of the exception being
	///                       rethrown
	/// @param exceptionMessage the message embedded in the
	///                       exception being rethrown
	///
	///////////////////////////////////////////////////////////
	exceptionInfo(const std::wstring& functionName, const std::string& fileName, const long lineNumber, const std::string& exceptionType, const std::string& exceptionMessage);

	exceptionInfo();
	
	exceptionInfo(const exceptionInfo& right);

	exceptionInfo& operator=(const exceptionInfo& right);

	/// \brief Return the object's content formatted in
	///         a string.
	///
	/// @return the object's content formatted in a string
	///
	///////////////////////////////////////////////////////////
	std::wstring getMessage();

	std::wstring m_functionName;   ///< The name of the function where the catch&throw is happening
	std::string m_fileName;        ///< The name of the file where the catch&throw is happening
	long m_lineNumber;             ///< The number of the line where the catch&throw is happening
	std::string m_exceptionType;   ///< The type of the exception being rethrown
	std::string m_exceptionMessage;///< The message embedded in the exception
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief One statically allocated instance of this class
///         logs the stack positions while an exception is
///         being processed.
///
/// In order to log the stack position the application
///  must use the following macros inside its functions:
/// - PUNTOEXE_FUNCTION_START()
/// - PUNTOEXE_FUNCTION_END()
/// - PUNTOEXE_THROW()
///
/// The final catch block that processes the exception and
///  doesn't rethrow it should call 
///  exceptionsManager::getMessage() in order to
///  retrieve the exception's history.
///
/// The retrieved message will contain the position of the
///  first throw statement and the positions of the
///  PUNTOEXE_FUNCTION_END() macros that rethrown the
///  exception.
///
/// All the catch blocks that don't rethrow the catched
///  exception and solve the exception's cause should call
///  exceptionsManager::getMessage() to empty the
///  messages stack.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class exceptionsManager: public baseObject
{
public:
	/// \brief Add an exceptionInfo object to the active
	///         thread's information list.
	///
	/// This function is called by PUNTOEXE_FUNCTION_END() when
	///  an uncaught exception is found.
	///
	/// @param info the info object that must be added
	///         to the active thread's information list
	///
	///////////////////////////////////////////////////////////
	static void addExceptionInfo(const exceptionInfo& info);
	
	/// \brief Returns the same information returned by
	///         getExceptionInfo(), but formatted in a text
	///         message.
	///
	/// The function also clears the information list for
	///  the current thread.
	///
	/// @return the information formatted as a text message
	///
	///////////////////////////////////////////////////////////
	static std::wstring getMessage();

	/// \brief Defines a list of exceptionInfo objects.
	///
	///////////////////////////////////////////////////////////
	typedef std::list<exceptionInfo> tExceptionInfoList;

	/// \brief Fill the list with all the collected
	///         exceptionInfo objects for the current thread.
	///
	/// The function also clears the information list of
	///  the current thread.
	///
	/// @param pList a pointer to the list that will be filled
	///               with a copy of the exceptionInfo objects
	///               collected before the call.
	///
	///////////////////////////////////////////////////////////
	static void getExceptionInfo(tExceptionInfoList* pList);

	/// \brief Clear the information list for the active
	///         thread.
	///
	///////////////////////////////////////////////////////////
	static void clearExceptionInfo();

	// Return a pointer to the statically allocated
	//         instance of exceptionsManager.
	///////////////////////////////////////////////////////////
	static ptr<exceptionsManager> getExceptionsManager();

protected:
    typedef std::map<std::thread::id, tExceptionInfoList> tInfoMap;

	tInfoMap m_information;

public:
	// Force the construction of the exceptions manager before
	//  main() starts
	///////////////////////////////////////////////////////////
	class forceExceptionsConstruction
	{
	public:
		forceExceptionsConstruction()
		{
			exceptionsManager::getExceptionsManager();
		}
	};

};




/// \def PUNTOEXE_FUNCTION_START(functionName)
///
/// \brief Initialize a try block. The try block must be
///         matched by a call to the 
///         PUNTOEXE_FUNCTION_END() macro.
///
/// This macro should be placed at the very beginning
///  of a function.
///
/// All the exceptions not catched by the body of the
///  function are catched by PUNTOEXE_FUNCTION_END() and 
///  rethrown, but before being rethrown the function's
///  name and the line number in the source file are 
///  logged.
/// All the positions of the PUNTOEXE_FUNCTION_END() that
///  catch the same exception are logged togheter.
///
/// Exceptions thrown inside the function should be thrown
///  by PUNTOEXE_THROW(), but this is not necessary.
///
/// @param functionName the name of the function in which
///         the macro is placed.
///
///////////////////////////////////////////////////////////
#define PUNTOEXE_FUNCTION_START(functionName) \
	static const wchar_t* _puntoexe_function_name = functionName;\
	try{

/// \def PUNTOEXE_FUNCTION_END()
///
/// \brief Insert a catch block that rethrows the catched
///         exception and log the function's name and
///         the position in the source file in which the
///         exception has been catched and rethrown.
///
/// This function must be placed at the end of a function
///  if the PUNTOEXE_FUNCTION_START() has been used in
///  the function.
///
///////////////////////////////////////////////////////////
#define PUNTOEXE_FUNCTION_END() \
	}\
	catch(std::exception& e)\
	{\
		puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, typeid(e).name(), e.what());\
		puntoexe::exceptionsManager::addExceptionInfo(info);\
		throw;\
	}\
	catch(...)\
	{\
		puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, "unknown", "");\
		puntoexe::exceptionsManager::addExceptionInfo(info);\
		throw;\
	}

/// \def PUNTOEXE_FUNCTION_END_MODIFY()
///
/// \brief Insert a catch block that rethrows the catched
///         exception and log the function's name and
///         the position in the source file in which the
///         exception has been catched and rethrown.
///        Exception of the type specified in the first
///         parameter are replaced by the type specified
///         in the second parameter.
///
/// @param catchType the exception type that must be
///                  caught and replace by the exception in
///                  throwType
/// @param throwType the exception type to throw instead
///                  of catchType
///
///////////////////////////////////////////////////////////
#define PUNTOEXE_FUNCTION_END_MODIFY(catchType, throwType) \
    }\
    catch(catchType& e)\
    {\
        puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, typeid(e).name(), e.what());\
        puntoexe::exceptionsManager::addExceptionInfo(info);\
        PUNTOEXE_THROW(throwType, e.what());\
    }\
    catch(std::exception& e)\
    {\
        puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, typeid(e).name(), e.what());\
        puntoexe::exceptionsManager::addExceptionInfo(info);\
        throw;\
    }\
    catch(...)\
    {\
        puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, "unknown", "");\
        puntoexe::exceptionsManager::addExceptionInfo(info);\
        throw;\
    }

/// \def PUNTOEXE_THROW(exceptionType, what)
///
/// \brief Throw an exception of the specified type and log
///         the function's name and the position in the
///         file on which the exception has been thrown.
///
/// This macro can be used only in the functions or blocks
///  that use the macros PUNTOEXE_FUNCTION_START() and
///  PUNTOEXE_FUNCTION_END().
///
/// @param exceptionType the type of exception to throw
/// @param what          a message to be associated with
///                       the exception
///
///////////////////////////////////////////////////////////
#define PUNTOEXE_THROW(exceptionType, what) \
	{\
        exceptionType puntoexeTrackException(what);\
        puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, typeid(puntoexeTrackException).name(), what);\
		puntoexe::exceptionsManager::addExceptionInfo(info);\
        throw puntoexeTrackException;\
	}

/// \def PUNTOEXE_RETHROW(what)
///
/// \brief Rethrow an exception caught by a catch block
///         and add some descriptions to it.
///
/// This macro can be used only in the functions or blocks
///  that use the macros PUNTOEXE_FUNCTION_START() and
///  PUNTOEXE_FUNCTION_END().
///
/// @param what          a message to be associated with
///                       the exception
///
///////////////////////////////////////////////////////////
#define PUNTOEXE_RETHROW(what) \
	{\
		puntoexe::exceptionInfo info(_puntoexe_function_name, __FILE__, __LINE__, "rethrowing", what);\
		puntoexe::exceptionsManager::addExceptionInfo(info);\
		throw;\
	}

///@}

} // namespace puntoexe


#endif // !defined(CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
