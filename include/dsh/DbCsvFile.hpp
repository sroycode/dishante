/**
* @project dishante
* @file include/dsh/DbCsvFile.hpp
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
* DB Access file for CSV format
*
*/

#ifndef _DSH_DB_CSVFILE_HPP_
#define _DSH_DB_CSVFILE_HPP_
#define DSH_DB_CSVFILE_HPP_PROGNO 1101
#include <string>
#include <fstream>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <apn/ConvertStr.hpp>

namespace dsh {
namespace db {
class CsvFile {
public:
	typedef std::vector<std::string> sVec;
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

	/**
	* Constructor : default for Csv Files
	*
	* @param Delim
	*   std::string Default for Delim
	*
	* @param FileNam
	*   std::string Default for FileNam
	*
	* @return
	*   none
	*
	*/
	CsvFile(
	    std::string Delim,
	    std::string FileNam
	) : Delim_(Delim),
		FileNam_(FileNam)
	{}

	/**
	* virtual destructor
	*/
	virtual ~CsvFile () {}

	/**
	* Process: Process the result
	*
	* @param InVars
	*   T Input Variable and transform List
	*
	* @param CallBack
	*   boost::function nullary callback function
	*
	*
	* @return
	*   none
	*/
	template<class T,class U>
	void Process(T InVars, boost::function <void (U)> Callback) {
		typedef std::vector<unsigned long int> uVec;
		typedef std::pair<std::string,std::string> ssPair;

		std::size_t header_size=InVars.size();
		std::string InFile = FileNam_;
		std::ifstream file(InFile.c_str());
		if (!file.is_open()) throw apn::GenericException(DSH_DB_CSVFILE_HPP_PROGNO,"Cannot Open " ,InFile.c_str());
		std::string line;
		boost::char_separator<char> sepA(Delim_.c_str());
		std::size_t lctr=0;
		uVec horder;
		while (std::getline(file,line)) {
			if (! line.length()) continue;
			tokenizer tokensA(line, sepA);
			if (lctr==0) {
				sVec hdata;
				BOOST_FOREACH(std::string tokA, tokensA) {
					hdata.push_back(apn::Convert::Trim(tokA));
				}
				sVec header;
				BOOST_FOREACH(ssPair sp, InVars) {
					for (std::size_t i=0; i<hdata.size(); ++i) {
						if (sp.second == hdata[i])
							horder.push_back(i);
					}
				}
				if (horder.size()!=header_size)
					throw apn::GenericException(DSH_DB_CSVFILE_HPP_PROGNO,"Mismatched no. of headers in " ,InFile.c_str());
			} else {
				U data(header_size);
				std::size_t mctr=0;
				BOOST_FOREACH(std::string tokA, tokensA) {
					if (mctr>=header_size)
						throw apn::GenericException(DSH_DB_CSVFILE_HPP_PROGNO,"More fields in " ,InFile.c_str());
					std::size_t pos = horder[mctr];
					data[pos]=apn::Convert::Trim(tokA);
					++mctr;
				}
				if (mctr!=header_size)
					throw apn::GenericException(DSH_DB_CSVFILE_HPP_PROGNO,"Mismatched no. of datapoints in " ,InFile.c_str());
				Callback(data);
			}
			++lctr;
		}
	}

private:
	/** Csv Params */
	std::string Delim_;
	std::string FileNam_;
};
} // namespace db
} // namespace dsh
#endif /* _DSH_DB_CSVFILE_HPP_ */
