/**
 * @project apophnia++
 * @file include/apn/Exception.hh
 * @author  S Roychowdhury <sroycode AT gmail DOT com>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * standard Exceptions to throw
 */

#ifndef _APN_EXCEPTION_HH_
#define _APN_EXCEPTION_HH_
#include <exception>

namespace apn {
/**
 * @brief NullException: null Exception to throw and catch
 */
struct NullException : public std::exception {
	NullException() throw() {}
};
/**
 * @brief GenericException: Exception to throw while initializing
 */
struct GenericException : public std::exception {
	/**
	 * Constructor: Default
	 *
	 * @param ErrorCode
	 *   int Error Code
	 * @param ErrorMsg
	 *   CString representing the error message
	 * @param ErrorFor
	 *   CString representing the error variable
	 *
	 * @return
	 *   none
	 */
	GenericException(const int ErrorCode, const char* ErrorMsg, const char* ErrorFor="") :
		ErrorCode_(ErrorCode),
		ErrorMsg_(ErrorMsg),
		ErrorFor_(ErrorFor)
	{}
	const int ErrorCode_;
	const char* ErrorMsg_;
	const char* ErrorFor_;
};
} // namespace apn
#endif
