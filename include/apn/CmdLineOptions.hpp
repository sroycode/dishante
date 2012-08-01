/**
 * @project apophnia++
 * @file include/apn/CmdLineOptions.hpp
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
 * Class for reading the command line options
 *
 */

#ifndef _APN_CMDLINEOPTIONS_HPP_
#define _APN_CMDLINEOPTIONS_HPP_
#define APN_CMDLINEOPTIONS_HPP_PROGNO 1011

#include <list>
#include <vector>
#include <map>
#include <boost/tuple/tuple.hpp>
#include "ConvertStr.hpp"

#define APN_CMDLINE_LONG_DASH "--"
#define APN_CMDLINE_SHORT_DASH "-"
#define APN_CMDLINE_TABSP "   "
#define APN_CMDLINE_EOL "\n"

namespace apn {
class CmdLineOptions {
public:
	/**
	 * @brief: Element: Tuple for Holding Commandline option elems
	 * 	0 - param long
	 * 	1 - param short
	 * 	2 - description
	 * 	3 - hasparams
	 */
	typedef boost::tuple<std::string,std::string,std::string,bool> Element;
	typedef std::list<Element> eList;

	/**
	 * Constructor : default Constructor
	 *
	 */
	CmdLineOptions() {}
	/**
	 * Constructor : used Constructor
	 *
	 * @param argc
	 *   int number of parameters supplied (from command line)
	 * @param argv
	 *   char*[] parameters supplied (from command line)
	 * @param AddOptions
	 *   List_of_Elements of Valid Options
	 *
	 * @return
	 *   none
	 */
	CmdLineOptions(int argc, char* argv[], eList AddOptions = eList()) {
		ssMap NameMap;
		typedef std::map<std::string,bool> sbMap;
		sbMap HasOptMap;
		HelpLine = std::string("Allowed Options" APN_CMDLINE_EOL);
		AddOptions.insert(AddOptions.begin(),boost::make_tuple("help","h","print help line",false));
		BOOST_FOREACH(Element El, AddOptions) {
			/** short form */
			std::string ShortForm = APN_CMDLINE_SHORT_DASH + std::string(boost::get<1>(El));
			ssMap::const_iterator it = NameMap.find(ShortForm);

			if (it==NameMap.end())
				NameMap.insert(ssMap::value_type(ShortForm,boost::get<0>(El)));
			else
				throw apn::GenericException(APN_CMDLINEOPTIONS_HPP_PROGNO,"Repeated Param Shortform ",ShortForm.c_str());

			HelpLine+= std::string(APN_CMDLINE_TABSP + ShortForm);
			/** long form */
			std::string LongForm = APN_CMDLINE_LONG_DASH + std::string(boost::get<0>(El));
			it = NameMap.find(LongForm);

			if(it==NameMap.end())
				NameMap.insert(ssMap::value_type(LongForm,boost::get<0>(El)));

			else
				throw apn::GenericException(APN_CMDLINEOPTIONS_HPP_PROGNO,"Repeated Param Longform ",LongForm.c_str());

			HelpLine+= std::string(APN_CMDLINE_TABSP "[" + LongForm + "]");
			/** explanation */
			HasOptMap.insert(sbMap::value_type(boost::get<0>(El),boost::get<3>(El)));
			HelpLine+= std::string(APN_CMDLINE_TABSP + boost::get<2>(El) +  APN_CMDLINE_EOL);
		}
		std::string TokenString;

		for(int i=0; i<argc; ++i) TokenString+=std::string((i==0)?"":" ")+std::string(argv[i]);

		std::vector<std::string> TokenVec = apn::Convert::StringToList<std::vector<std::string> >(TokenString," =\n\t");

		for(size_t i=1; i<TokenVec.size(); ++i) {
			ssMap::const_iterator it = NameMap.find(TokenVec.at(i));

			if(it==NameMap.end()) throw apn::GenericException(APN_CMDLINEOPTIONS_HPP_PROGNO,"Unknown Option ",TokenVec.at(i).c_str());

			if(HasOptMap[it->second]) {
				if(++i>=TokenVec.size())  throw apn::GenericException(APN_CMDLINEOPTIONS_HPP_PROGNO,"No Value for Option ",it->second.c_str());

				OptionsMap.insert(ssMap::value_type(it->second,TokenVec.at(i)));

			} else {
				OptionsMap.insert(ssMap::value_type(it->second,std::string()));
			}
		}
	}
	~CmdLineOptions() {}
	/**
	 * GetHelpLine :  Get Help Line
	 *
	 * @return
	 *   String
	 */
	std::string GetHelpLine() {
		return HelpLine;
	}
	/**
	 * Check : Check if Value Exists
	 *
	 * @param ParamName
	 *   String Name of Param to retrieve
	 *
	 * @return
	 *   bool
	 */
	bool Check(std::string ParamName) {
		ssMap::const_iterator it = OptionsMap.find(ParamName);
		return (it!=OptionsMap.end());
	}
	/**
	 * Find : Find Value
	 *
	 * @param ParamName
	 *   String Name of Param to retrieve
	 * @param NoThrow
	 *   bool Find default if absent
	 *
	 * @return
	 *   Param
	 */
	template<class T>
	T Find(std::string ParamName, bool NoThrow=false) {
		ssMap::const_iterator it = OptionsMap.find(ParamName);

		if(it==OptionsMap.end()) {
			if(!NoThrow) throw apn::GenericException(APN_CMDLINEOPTIONS_HPP_PROGNO,"Bad Commandline parameter",ParamName.c_str());

			else return T();
		}

		try {
			return apn::Convert::AnyToAny<std::string,T>(it->second);

		} catch(apn::GenericException& e) {
			if(!NoThrow) throw apn::GenericException(APN_CMDLINEOPTIONS_HPP_PROGNO,"Missing Commandline parameter ",ParamName.c_str());

			else return T();
		}

		return T();
	}
private:
	typedef std::map<std::string,std::string> ssMap;
	ssMap OptionsMap; /** param name val */
	std::string HelpLine;
};
} // namespace
#endif
