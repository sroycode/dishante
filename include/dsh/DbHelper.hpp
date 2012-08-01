/**
* @project dishante
* @file include/dsh/DbHelper.hpp
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
* Helperfile fo DB Access
*
*/

#ifndef _DSH_DB_HELPER_HPP_
#define _DSH_DB_HELPER_HPP_
#define DSH_DB_HELPER_HPP_PROGNO 1105

#include <string>
#include <map>
#include <vector>
#include <boost/utility/value_init.hpp>

namespace dsh {
namespace db {

class Helper {
public:
	typedef std::vector<std::string> sVec;
	typedef std::map<std::string,int> siMap;
	/**
	* Constructor : default
	*
	* @param S
	*   sVec Input Vector to make map
	*
	* @return
	*   none
	*
	*/
	Helper(sVec S) {
		UMap.clear();
		for (std::size_t i=0; i<S.size(); ++i) UMap[ S[i] ] = i;
	}

	/**
	* virtual destructor
	*/
	virtual ~Helper () {}


	/**
	* Set : Add a Row
	*
	* @param row
	*   sVec Input Vector Data
	*
	* @return
	*   none
	*/
	template<class T>
	void Set(T& row) {
		if (row.size() != UMap.size())
			throw apn::GenericException(DSH_DB_HELPER_HPP_PROGNO,"Data Size Mismatch","");
		UVec.resize(row.size());
		std::copy ( row, row+UMap.size(), UVec.begin() );
	}

	/**
	* Get : get the element
	*
	* @param InData
	*   std::string Input Data
	*
	* @return
	*   T output
	*/
	template<class T>
	T Get(std::string InData) {
		siMap::const_iterator it = UMap.find(InData);
		return (it!=UMap.end()) ? UVec[std::size_t(*it)] : T();
	}


private:
	siMap UMap;
	sVec UVec;
};
} //namespace db
} //namespace dsh
#endif /* _DSH_DB_HELPER_HPP_ */
