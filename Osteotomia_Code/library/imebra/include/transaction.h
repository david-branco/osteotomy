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

/*! \file transaction.h
    \brief Declaration of the classes that allow to use the transactions
            on the writing handlers (see dataHandler).

*/

#if !defined(CImbxTransaction_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
#define CImbxTransaction_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_

#include "../../base/include/configuration.h"
#include "../../base/include/baseObject.h"
#include "../../base/include/exception.h"

#include "dataHandler.h"

#include <typeinfo>
#include <map>
#include <list>
#include <thread>

namespace puntoexe
{

namespace imebra
{

class transaction;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \internal
/// \brief Manages the transactions.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class transactionsManager
{
public:
	// Constructor
	///////////////////////////////////////////////////////////
	transactionsManager(): m_lockObject(new baseObject){}

    static bool addTransaction(std::thread::id threadId, transaction* pTransaction);

    static void removeTransaction(std::thread::id threadId);

	static void addHandlerToTransaction(ptr<handlers::dataHandler> newHandler);

protected:
	// Return a pointer to the statically allocated
	//         instance of transactionsManager.
	///////////////////////////////////////////////////////////
	static transactionsManager* getTransactionsManager();

	typedef std::list<transaction*> tTransactionsStack;
    typedef std::map<std::thread::id, tTransactionsStack> tTransactionsMap;

	tTransactionsMap m_transactions;

	ptr<baseObject> m_lockObject;
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a single transaction.
///
/// When a transaction is created it is inserted into
///  the current thread's transactions stack; the 
///  transaction object on the top of the stack will be
///  used by all the writing handlers (see dataHandler)
///  created AFTER the transaction's creation.
///
/// The writing handlers that are included in a transaction
///  don't write the modified data back to the buffer
///  when they go out of scope, but when the first 
///  committing transaction goes out of scope.
///
/// The transaction writes the data back to the buffers in
///  two phases:
///  - the first phase build the data for all the buffers
///    and check for errors
///  - if there are no errors in the first phase then the
///    second phase commits all the changes and the
///    modifications are finalized.
///
/// If the transaction is aborted or an unhandled exception
///  is thrown inside a transaction block (see the 
///  transaction macros) then all the modifications are 
///  rolled back and no changes are written back to the 
///  buffers.
///
/// Transactions should be used through the macros
/// - IMEBRA_TRANSACTION_START()
/// - IMEBRA_COMMIT_TRANSACTION_START()
/// - IMEBRA_TRANSACTION_END()
/// - IMEBRA_TRANSACTION_ABORT()
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class transaction
{
	friend class transactionsManager;
public:
	/// \brief Initialize a transaction.
	///
	/// @param bCommitTransaction if this parameter is true
	///                            then the transaction will
	///                            commit the changes in the
	///                            destructor, otherwise
	///                            the changes will be 
	///                            committed by the parent
	///                            transaction.
	///                           When there are no parent
	///                            transactions, then the 
	///                            commit happens anyway
	///
	///////////////////////////////////////////////////////////
	transaction(bool bCommitTransaction = false);

	/// \brief Destroy the transaction and commit the changes
	///         if bCommitTransaction was set to true in the
	///         constructor or there are no parent 
	///         transactions.
	///
	/// If abort() was called before the destruction of the
	///  transaction then the modified data will not be copied
	///  back to the buffers.
	///
	///////////////////////////////////////////////////////////
	virtual ~transaction();

	/// \brief Rollback all the changes made to far.
	///
	/// Data handlers (see dataHandler) created AFTER the call
	///  to abort() will be committed unless another call to
	///  abort is made().
	///
	///////////////////////////////////////////////////////////
	void abort();

protected:
	/// \internal
	/// \brief Copy all the dataHandler from this transaction 
	///         to another transaction.
	///
	/// @param pDestination a pointer to the transaction where
	///                      the all the dataHandler must be
	///                      copied.
	///
	///////////////////////////////////////////////////////////
	void copyHandlersTo(transaction* pDestination);

	void addHandler(ptr<handlers::dataHandler> newHandler);

	typedef ptr<handlers::dataHandler> tDataHandlerPtr;
	typedef std::map<buffer*, tDataHandlerPtr> tHandlersList;
	tHandlersList m_transactionHandlers;

    std::thread::id m_threadId;
	bool m_bCommit;
};

/// \def IMEBRA_COMMIT_TRANSACTION_START()
/// \brief Start a committing transaction block.
///
/// The transaction block must be terminated by
///  the macro IMEBRA_TRANSACTION_END().
///
/// All the writing data handlers that are created in
///  the transaction's block will commit the modifications
///  only at the end of the transaction's block.
/// If one of the modifications cannot be committed or
///  if an uncaught exception occurs, then all the buffers
///  are rolled back to the status they had at the 
///  beginning of the transaction.
///
/// Committing transactions commit also the data of the
///  nested non-committing transactions
///
///////////////////////////////////////////////////////////
#define IMEBRA_COMMIT_TRANSACTION_START() \
	{\
		PUNTOEXE_FUNCTION_START(L"Imebra Commit Transaction");\
		puntoexe::imebra::transaction imebraNestedTransaction(true);\
		try{

/// \def IMEBRA_TRANSACTION_START()
/// \brief Start a non-committing transaction block.
///
/// The transaction block must be terminated by
///  the macro IMEBRA_TRANSACTION_END().
///
/// If the transaction is not nested into another
///  one, then a committing transaction is created
///  anyway, as in IMEBRA_COMMIT_TRANSACTION_START().
///
/// All the writing data handlers that are created in
///  the transaction's block will commit the modifications
///  only at the end first parent committing transaction.
/// If one of the modifications cannot be committed or
///  if an uncaught exception occurs, then all the buffers
///  are rolled back to the status they had at the 
///  beginning of the transaction.
///
///////////////////////////////////////////////////////////
#define IMEBRA_TRANSACTION_START() \
	{\
		PUNTOEXE_FUNCTION_START(L"Imebra Transaction");\
		puntoexe::imebra::transaction imebraNestedTransaction(false);\
		try{

/// \def IMEBRA_TRANSACTION_END()
/// \brief Terminates a transaction block initiated by
///         IMEBRA_COMMIT_TRANSACTION_START() or
///         IMEBRA_TRANSACTION_START().
///
/// If the macro terminates a committing transaction then
///  it tries to commit all the changes made through the
///  writing handlers, otherwise it passes the 
///  modifications to the parent transaction.
///
/// If an uncaught exception occurs or one of the changes 
///  cannot be committed, then all the modifications are
///  rolled back to the status they had at the beginning
///  of the transaction.
///
/// Uncaught exceptions are rethrown by this macro.
///
///////////////////////////////////////////////////////////
#define IMEBRA_TRANSACTION_END() \
		}\
		catch(...)\
		{\
			imebraNestedTransaction.abort();\
			PUNTOEXE_RETHROW("Transaction aborted");\
		}\
		PUNTOEXE_FUNCTION_END();\
	}

/// \def IMEBRA_TRANSACTION_ABORT()
/// \brief Delete the modifications made in the 
///         transaction's block before this macro is
///         called.
///
/// This macro is called by IMEBRA_TRANSACTION_END() in
///  case an uncaught exception is detected.
///
///////////////////////////////////////////////////////////
#define IMEBRA_TRANSACTION_ABORT() \
	imebraNestedTransaction.abort();

} // namespace imebra

} // namespace puntoexe

#endif // !defined(CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
