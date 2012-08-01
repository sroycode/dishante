/**
* @project dishante
* @file src/Dout.hpp
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
* Dout is data output of results in various formats
*
*/

#ifndef _DSHN_DOUT_HPP_
#define _DSHN_DOUT_HPP_
#define DSHN_DOUT_PROGNO 2003

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/tuple/tuple.hpp>

namespace dshn {
struct mime_type_mapping {
	const char* type;
	const char* ctype;
	unsigned long ofmt;
} mime_type_mappings[] = {
	{ "json", "application/json", 1 },
	{ "csv", "text/csv", 2 },
	{ 0, 0, 0 } // Marks end of list.
};

template<class T,class R>
class Dout  {
public:
	typedef std::vector<std::string> sVec;
	/**
	* Constructor : Constructor
	*
	* @param invec
	*   T input vector
	*
	* @return
	*   none
	*/
	Dout(T& invec) : invec_(invec) {}

	/**
	* virtual destructor
	*/
	virtual ~Dout () {}

	/**
	* Parse : parse and populate values
	*
	* @param format
	*   std::string input format
	*
	* @param inres
	*   R result in container format ( specific to this work )
	*   	each result will be tuple of id, dist, and a list of fields ins same order as invec
	*
	* @param content_type
	*   std::string content type by address
	*
	* @param result
	*   std::string result by address
	*
	* @return
	*   bool status
	*/
	bool Parse(std::string format, R& res, std::string& content_type, std::string& result) {
		bool status=false;

		unsigned int fcode = 0;
		std::transform(format.begin(), format.end(), format.begin(), ::tolower);
		for (mime_type_mapping* m = mime_type_mappings; m->type; ++m) {
			if (m->type == format) {
				content_type=std::string(m->ctype);
				fcode=m->ofmt;
				break;
			}
		}
		// starts
		switch (fcode) {
		case 1: {
			std::stringstream ss;
			ss << "{result:";
			for (std::size_t i=0; i<res.size(); ++i) {
				if (i>0) ss << ",";
				ss << "[id:" << res[i].template get<0>() << ",dist:" << res[i].template get<1>() << ",{" ;
				for (std::size_t j=0; j<res[i].template get<2>().size(); ++j) {
					if (j>0) ss <<",";
					ss << invec_[j] << ":\"" << res[i].template get<2>()[j] << "\"" ;
				}
				ss << "]}";
			}
			ss << "}";
			status=true;
			result=ss.str();
		}
		break;
		case 2:
			break;
		default:
			break;
		}
		return status;
	}

private:
	sVec invec_;
};
}
#endif /* _DSHN_DOUT_HPP_ */
