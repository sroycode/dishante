/**
 * @project apophnia++
 * @file include/apn/ConvertStr.hpp
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
 * Various Converters  - string and list
 *
 */

#ifndef _APN_CONVERTSTR_HPP_
#define _APN_CONVERTSTR_HPP_
#define APN_CONVERTSTR_HPP_PROGNO 1002

#include "Convert.hpp"
#include <string>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


namespace apn {
namespace Convert {

/**
 * StringToList: Make List from string using delimiter(s)
 *
 * @param InString
 *   String Input Data
 *
 * @param Sep
 *   String Separators
 *
 * @return
 *   OutputType
 */
template<typename OutDataT>
OutDataT StringToList(const std::string InString, const char* Sep)
{
	OutDataT OutData;

	try {
		boost::char_separator<char> sepA(Sep);
		boost::tokenizer<boost::char_separator<char> > tokensA(InString, sepA);
		BOOST_FOREACH(std::string tokA, tokensA) {
			typename OutDataT::value_type ctok = AnyToAny<std::string,typename OutDataT::value_type>(tokA);
			OutData.insert(OutData.end(),ctok);
		}

	} catch(...) {
		throw apn::GenericException(APN_CONVERTSTR_HPP_PROGNO,"Convert::StringToList error","generic");
	}

	return OutData;
}
/**
 * ListToString: Make String from List apply delims
 *
 * @param InList
 *   ListT Input List
 *
 * @param Sep
 *   String Separator
 *
 * @return
 *   String
 */
template<typename ListT>
std::string ListToString(ListT& InList, const char* Sep = " ")
{
	std::string Ret;
	try {
		BOOST_FOREACH(std::string s, InList) Ret += s + std::string(Sep);
	} catch(...) {
		throw apn::GenericException(APN_CONVERTSTR_HPP_PROGNO,"Convert::ListToString error","generic");
	}
	return Ret;
}

/**
 * StringToMap: Make Map from string using delimiter(s)
 *
 * @param InString
 *   String Input Data
 *
 * @param SepA
 *   CString Separators for first
 *
 * @param SepB
 *   CString Separators second
 *
 * @return
 *   OutputType
 */
template<typename OutDataT>
OutDataT StringToMap(const std::string InString, const char* SepA, const char* SepB)
{
	typedef boost::tokenizer< boost::char_separator<char> > Tokenizer;
	typedef typename OutDataT::key_type key_type;
	typedef typename OutDataT::mapped_type data_type;
	OutDataT OutData;

	try {
		boost::char_separator<char> csepA(SepA),csepB(SepB);
		Tokenizer tokensA(InString, csepA);
		BOOST_FOREACH(std::string tokA, tokensA) {
			std::size_t i=0;
			Tokenizer tokensB(tokA, csepB);
			key_type k;
			data_type v;
			for (Tokenizer::const_iterator it=tokensB.begin(); it!=tokensB.end(); ++it) {
				if (i==0) k = AnyToAny<std::string,key_type>(*it);
				else v = AnyToAny<std::string,data_type>(*it);
				++i;
			}
			OutData.insert(OutData.end(), std::make_pair<key_type,data_type>(k,v));
		}

	} catch(...) {
		throw apn::GenericException(APN_CONVERTSTR_HPP_PROGNO,"Convert::StringToMap error","generic");
	}

	return OutData;
}


/**
 * Trim: Remove unwanted white spaces at begin and end and return
 *
 * @param InString
 *   String Input Data
 *
 * @param Delims
 *   CString Delimitors
 *
 * @return
 *   String
 */
inline std::string Trim(const std::string InString, char const* Delims = " \t\r\n")
{
	std::string WkString(InString);
	std::string::size_type index = WkString.find_last_not_of(Delims);

	if(index != std::string::npos)
		WkString.erase(++index);

	index = WkString.find_first_not_of(Delims);

	if(index != std::string::npos)
		WkString.erase(0, index);

	else
		WkString.erase();

	return WkString;
}
/**
 * InPlaceTrim: Remove unwanted white spaces at begin and end for string
 *
 * @param InString
 *   String Input Data
 *
 * @param Delims
 *   CString Delimitors
 *
 * @return
 *   none
 */
inline void InPlaceTrim(std::string& InString, char const* Delims = " \t\r\n")
{
	std::string::size_type index = InString.find_last_not_of(Delims);

	if(index != std::string::npos)
		InString.erase(++index);

	index = InString.find_first_not_of(Delims);

	if(index != std::string::npos)
		InString.erase(0, index);

	else
		InString.erase();
}

/**
 * MapToList: Copy Element of a map to a list
 *
 * @param UseMap
 *   MapT Input Map
 *
 * @param UseKey
 *   bool Populate Map key if true
 *
 * @return
 *   ListT
 */
template<typename MapT , typename ListT>
ListT MapToList(MapT& UseMap, bool UseKey=true)
{
	ListT UseList;

	for(typename MapT::const_iterator it = UseMap.begin(); it!= UseMap.end() ; ++it) {
		UseList.insert(UseList.end(), typename ListT::value_type((UseKey) ? it->first : it->second));
	}

	return UseList;
}

/**
 * ListToMap: Copy Element of a list to map key
 *
 * @param UseList
 *   ListT Input List
 *
 * @param UseKey
 *   bool Populate List key if true
 *
 * @return
 *   MapT
 */
template<typename ListT , typename MapT>
MapT ListToMap(ListT& UseList)
{
	MapT UseMap;

	for(typename ListT::const_iterator it = UseList.begin(); it!= UseList.end() ; ++it) {
		UseMap.insert(UseMap.end(), typename MapT::value_type(
		                  typename MapT::value_type::first_type(*it),
		                  typename MapT::value_type::second_type()
		              ));
	}

	return UseMap;
}

} // namespace Convert
} // namespace apn
#endif // _APN_CONVERTSTR_HPP_
