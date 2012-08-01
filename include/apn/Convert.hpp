/**
 * @project apophnia++
 * @file include/apn/Convert.hpp
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
 * Various Converters non-string
 *
 */

#ifndef _APN_CONVERT_HPP_
#define _APN_CONVERT_HPP_
#define APN_CONVERT_HPP_PROGNO 1001

#include "Exception.hh"
#include <boost/utility/value_init.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace apn {
namespace Convert {

/**
 * AnyToAny: Get the Corresp value in a different Type
 *
 * @param InData
 *   InputType Input Data
 *
 * @param NoThrow
 *   (optional) bool get default if error, default FALSE
 *
 * @return
 *   OutputType
 */
template<typename InDataT , typename OutDataT>
OutDataT AnyToAny(InDataT InData, bool NoThrow=false)
{
	OutDataT OutData = OutDataT();

	try {
		OutData=boost::lexical_cast<OutDataT>(InData);

	} catch(std::exception& e) {
		if(! NoThrow)
			throw apn::GenericException(APN_CONVERT_HPP_PROGNO,"Convert::AnyToAny conv error ",e.what());

	} catch(...) {
		if(! NoThrow)
			throw apn::GenericException(APN_CONVERT_HPP_PROGNO,"Convert::AnyToAny conv error ","unknown");
	}

	return OutData;
}
/**
 * AnyToNum: Get the Corresp value in a numeric Type
 *
 * @param InData
 *   InputType Input Data
 *
 * @param NoThrow
 *   bool get default if error, default FALSE
 *
 * @return
 *   OutputType
 */
template<typename InDataT , typename OutDataT>
OutDataT AnyToNum(InDataT InData, bool NoThrow=false)
{
	OutDataT OutData = OutDataT();

	try {
		OutData=boost::numeric::converter<InDataT,OutDataT>::convert(InData);

	} catch(std::exception& e) {
		if(! NoThrow)
			throw apn::GenericException(APN_CONVERT_HPP_PROGNO,"Convert::AnyToNum conv error ",e.what());

	} catch(...) {
		if(! NoThrow)
			throw apn::GenericException(APN_CONVERT_HPP_PROGNO,"Convert::AnyToNum conv error ","unknown");
	}

	return OutData;
}
} // namespace Convert
} // namespace apn
#endif // _APN_CONVERT_HPP_
