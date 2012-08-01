/**
 * @project apophnia++
 * @file include/apn/CfgFileOptions.hpp
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
 * CfgFileOptions: Class for reading the configuration files
 *
 */

#if !defined(NDEBUG)
#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif
#ifndef _APN_CFGFILEOPTIONS_HPP_
#define _APN_CFGFILEOPTIONS_HPP_
#define APN_CFGFILEOPTIONS_HPP_PROGNO 1012

#include <fstream>
#include <map>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include "ConvertStr.hpp"

#define APN_CFGFILE_COLON ':'
#define APN_CFGFILE_SEMICOLON ';'
#define APN_CFGFILE_SQB_OPEN '['
#define APN_CFGFILE_SQB_CLOSE ']'
#define APN_CFGFILE_HASH '#'
#define APN_CFGFILE_EQUALTO '='
#define APN_CFGFILE_MIDLINE_TRIM " \t\r\n"
#define APN_CFGFILE_ENDLINE_TRIM " \t\r\n;"
#define APN_CFGFILE_BLANK_STR ""
#define APN_CFGFILE_DELIM_STR "::"

namespace apn {



class CfgFileOptions {
public:
	/**
	 * Constructor : the default constructor
	 */
	CfgFileOptions() {}
	/**
	 * Constructor : the used constructor
	 *
	 * @param InputFile
	 *   String Input File Name
	 *
	 */
	CfgFileOptions(const std::string InputFile) {
		std::ifstream file(InputFile.c_str());

		if(!file.is_open())
			throw apn::GenericException(APN_CFGFILEOPTIONS_HPP_PROGNO,
			                            "Cannot Open Configfile",InputFile.c_str());
		std::string line, inSection;
		while(std::getline(file,line)) {
			line=apn::Convert::Trim(line,APN_CFGFILE_ENDLINE_TRIM);
			if(! line.length()) continue;
			if(line[0] == APN_CFGFILE_HASH) continue;
			if(line[0] == APN_CFGFILE_SEMICOLON) continue;
			if(line[0] == APN_CFGFILE_SQB_OPEN) {
				if(!line.find(APN_CFGFILE_SQB_CLOSE))
					throw apn::GenericException(APN_CFGFILEOPTIONS_HPP_PROGNO,
					                            "Configfile Syntax Error ",line.c_str());

				/** section name */
				inSection=apn::Convert::Trim(line.substr(1,line.find(APN_CFGFILE_SQB_CLOSE)-1),
				                             APN_CFGFILE_ENDLINE_TRIM);
				std::string inHerits = APN_CFGFILE_BLANK_STR;
				ssMap::const_iterator it = SectionInheritMap.find(inSection);

				if(it!=SectionInheritMap.end())
					throw apn::GenericException(APN_CFGFILEOPTIONS_HPP_PROGNO,
					                            "Duplicate Section : ",inSection.c_str());

				/** inherits name if exists */
				if(line.length() > line.find_last_of(APN_CFGFILE_COLON)) {
					inHerits = apn::Convert::Trim(line.substr(line.find_last_of(APN_CFGFILE_COLON)+1),
					                              APN_CFGFILE_ENDLINE_TRIM);
					it = SectionInheritMap.find(inHerits);

					if(it==SectionInheritMap.end())
						throw apn::GenericException(APN_CFGFILEOPTIONS_HPP_PROGNO,
						                            "Invalid Inherit : ", inSection.c_str());
				}
				SectionInheritMap.insert(ssMap::value_type(inSection,inHerits));
				continue;
			}

			size_t EqPos = line.find_first_of(APN_CFGFILE_EQUALTO); /** position */

			if(EqPos==std::string::npos)
				throw apn::GenericException(APN_CFGFILEOPTIONS_HPP_PROGNO,
				                            "Invalid Line : ", line.c_str());

			std::string myname = apn::Convert::Trim(line.substr(0,EqPos),APN_CFGFILE_MIDLINE_TRIM);
			std::string myval = apn::Convert::Trim(line.substr(EqPos+1),APN_CFGFILE_ENDLINE_TRIM);
			Update(inSection,myname,myval);
		}

		file.close();
	}
	~CfgFileOptions() {}
	/**
	 * Update: Update or Add an entry
	 *
	 * @param Section
	 *   String Section
	 * @param Name
	 *   String Name
	 * @param Value
	 *   String Value
	 *
	 * @return
	 *   none
	 */
	void Update(const std::string Section, const std::string Name, std::string Value) {
		ces_index_by_id::iterator it = CStore.find(Section + APN_CFGFILE_DELIM_STR + Name);
		if (it==CStore.end())
			CStore.insert(CfgElem(Section,Name,Value));
		else {
			CStore.replace(it,CfgElem(Section,Name,Value));
		}
	}
	/**
	 * SectionToMap: Get a Map of all in section
	 *
	 * @param SectionName
	 *   String SectionName
	 *
	 * @return
	 *   Type
	 */
	template<typename T>
	T SectionToMap(std::string SectionName) {
		typedef typename T::key_type key_type;
		typedef typename T::mapped_type data_type;
		T t;
		std::pair<ces_index_by_section::iterator, ces_index_by_section::iterator> itp;
		itp.first = CStore.get<section>().lower_bound(SectionName);
		itp.second = CStore.get<section>().upper_bound(SectionName);
		for (ces_index_by_section::iterator it = itp.first; it != itp.second; ++it) {
			CfgElem e=*it;
			key_type k = apn::Convert::AnyToAny<std::string,key_type>(e.name_);
			data_type v = apn::Convert::AnyToAny<std::string,data_type>(e.value_);
			t.insert(t.end(), std::make_pair<key_type,data_type>(k,v));
		}
		return t;
	}

	/**
	 * Check : Check if Value Exists
	 *
	 * @param Section
	 *   String Section to find it in
	 *
	 * @param Name
	 *   String Name to find
	 *
	 * @return
	 *   bool
	 */
	template<typename T>
	bool Check(const std::string Section, const std::string Name) {
		std::string Section_ = Section;
		size_t sisz = SectionInheritMap.size();

		do {
			ces_index_by_id::iterator it = CStore.get<id>().find(Section_ + APN_CFGFILE_DELIM_STR + Name);
			if (it != CStore.get<id>().end()) {
				CfgElem c = *it;
				Convert::AnyToAny<std::string,T>(c.value_);
				return true;
			} else {
				/** else look for inherited section */
				ssMap::const_iterator jt = SectionInheritMap.find(Section_);
				if(jt!=SectionInheritMap.end()) Section_= jt->second;
				else break;
			}
		} while(--sisz>0);  /** logic: one less than total */
		return false;
	}
	/**
	 * Find: Find an entry
	 *
	 * @param Section
	 *   String Section to find it in
	 *
	 * @param Name
	 *   String Name to find
	 *
	 * @param NoThrow
	 *   Bool get default-val if true, defult false.
	 *
	 * @return
	 *   Type
	 */
	template<typename T>
	T Find(const std::string Section, const std::string Name, bool NoThrow=false) {
		std::string Section_ = Section;
		size_t sisz = SectionInheritMap.size();

		do {
			ces_index_by_id::iterator it = CStore.get<id>().find(Section_ + APN_CFGFILE_DELIM_STR + Name);
			if (it != CStore.get<id>().end()) {
				CfgElem c = *it;
				return Convert::AnyToAny<std::string,T>(c.value_,NoThrow);
			}

			/** else look for inherited section */
			ssMap::const_iterator jt = SectionInheritMap.find(Section_);

			if(jt!=SectionInheritMap.end()) Section_= jt->second;

			else break;
		} while(--sisz>0);  /** logic: one less than total */

		if(!NoThrow)
			throw apn::GenericException(APN_CFGFILEOPTIONS_HPP_PROGNO,"Invalid Config Param : ", Name.c_str());

		return T();
	}
	/**
	 * Show: Show all entries
	 *
	 * @return
	 *   none
	 */
	void Show() {
		const ces_index_by_id& i = CStore.get<id>();
		std::copy(i.begin(),i.end(),std::ostream_iterator<CfgElem>(std::cout));
	}
private:
	typedef std::map<std::string,std::string> ssMap;
	/* an CfgElem record holds its section name value */
	struct CfgElem {
		std::string section_;
		std::string name_;
		std::string value_;

		CfgElem(std::string section,std::string name,std::string value):
			section_(section), name_(name), value_(value) {}
		std::string id()const {
			return section_ + APN_CFGFILE_DELIM_STR + name_;
		}
		friend std::ostream& operator<<(std::ostream& os,const CfgElem& e) {
			os<<e.section_<<" :: "<<e.name_<<" = "<<e.value_<<std::endl;
			return os;
		}

	};

	/* tags for accessing the corresponding indices of CfgElem_Set */
	struct id {};
	struct section {};

	/* Define a multi_index_container of CfgElems with following indices:
	 *   - a unique index sorted by CfgElem::id,
	 *   - a non-unique index sorted by CfgElem::section.
	 */

	typedef boost::multi_index::multi_index_container<
	CfgElem,
	boost::multi_index::indexed_by<
	boost::multi_index::ordered_unique<
	boost::multi_index::tag<id>,  BOOST_MULTI_INDEX_CONST_MEM_FUN(CfgElem,std::string,id)>,
	boost::multi_index::ordered_non_unique<
	boost::multi_index::tag<section>, BOOST_MULTI_INDEX_MEMBER(CfgElem,std::string,section_)> >
	> CfgElemSet;
	CfgElemSet CStore;
	typedef CfgElemSet::index<id>::type ces_index_by_id;
	typedef CfgElemSet::index<section>::type ces_index_by_section;
	ssMap SectionInheritMap; /** Section and inherits */
};
} // namespace
#endif
