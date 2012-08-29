/**
* @project dishante
* @file src/Work.cc
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
* Workhorse File
*
*/

#include <iostream>
#include <vector>
#include <set>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "Work.hpp"
#include "Dout.hpp"
#include <apn/ConvertStr.hpp>

#include <dsh/DbCsvFile.hpp>

#ifdef COMPILE_WITH_MYSQL
#include <dsh/DbMysql.hpp>
#endif

#ifdef COMPILE_WITH_PGSQL
#include <dsh/DbPgsql.hpp>
#endif


/**
* Constructor : private Constructor
*
* @param MyCFG
*   apn::CfgFileOptions ConfigFile Options
*
* @return
*   none
*/
dshn::Work::Work (apn::CfgFileOptions& MyCFG)
	: params2d(loadparams(MyCFG,false)),
	  params3d(loadparams(MyCFG,true))
{
	typedef std::pair<std::string,std::string> ssPair;
	typedef std::vector<ssPair> ssPairVec;

	sVec S = apn::Convert::StringToList<sVec>(
	             MyCFG.Find<std::string>(DSHN_DEFAULT_STRN_SYSTEM, DSHN_DEFAULT_STRN_INDEXES),
	             DSHN_DEFAULT_STRN_INDEXES_SEPARATOR);
	for (sVec::const_iterator it=S.begin(); it!=S.end(); ++it) {
		if ( MyCFG.Find<int>(*it, "active",false)==0) continue;
		bool is3d = MyCFG.Check<DSHN_DEFAULT_COORDT>(*it, DSHN_DEFAULT_STRN_Z);
		ssPairVec mvec;
		if (is3d) {
			for (sVec::const_iterator jt=params3d.begin(); jt!=params3d.end(); ++jt) {
				mvec.push_back(std::make_pair<std::string,std::string>(*jt,MyCFG.Find<std::string>(*it,*jt)));
			}
		} else {
			for (sVec::const_iterator jt=params2d.begin(); jt!=params2d.end(); ++jt) {
				mvec.push_back(std::make_pair<std::string,std::string>(*jt,MyCFG.Find<std::string>(*it,*jt)));
			}
		}
		std::string dbtype = MyCFG.Find<std::string>(*it, "dbtype");


		// Database work Begin

		if (dbtype=="csv") {
			dsh::db::CsvFile C(
			    MyCFG.Find<std::string>(*it, DSHN_DEFAULT_STRN_DELIM),
			    MyCFG.Find<std::string>(*it, DSHN_DEFAULT_STRN_FILENAME)
			);
			std::string r = MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_INDEX);
			C.Process<ssPairVec,sVec>(mvec,boost::bind(&dshn::Work::load,this,r,_1,is3d));
		} else if (dbtype=="mysql") {
#ifdef COMPILE_WITH_MYSQL
			dsh::db::Mysql C(
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBHOST),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBPORT),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBNAME),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBUSER),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBPASS),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBTABLE),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBWHERE)
			);
			std::string r = MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_INDEX);
			C.Process<ssPairVec,sVec>(mvec,boost::bind(&dshn::Work::load,this,r,_1,is3d));
#else
			throw apn::GenericException(DSHN_WORK_PROGNO,"Unimplemented datasource " , dbtype.c_str());
#endif
		} else if (dbtype=="pgsql") {
#ifdef COMPILE_WITH_PGSQL
			dsh::db::Pgsql C(
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBHOST),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBPORT),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBNAME),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBUSER),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBPASS),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBTABLE),
			    MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_DBWHERE)
			);
			std::string r = MyCFG.Find<std::string>(*it,DSHN_DEFAULT_STRN_INDEX);
			C.Process<ssPairVec,sVec>(mvec,boost::bind(&dshn::Work::load,this,r,_1,is3d));
#else
			throw apn::GenericException(DSHN_WORK_PROGNO,"Unimplemented datasource " , dbtype.c_str());
#endif
		} else {
			throw apn::GenericException(DSHN_WORK_PROGNO,"Undefined datasource: " , dbtype.c_str());
		}
		// Database work End
	}
	for(sp2Map::const_iterator jt = pdmap.begin(); jt!=pdmap.end(); ++jt) {
		jt->second->Lock();
	}
	for(sp3Map::const_iterator jt = pemap.begin(); jt!=pemap.end(); ++jt) {
		jt->second->Lock();
	}
}

/**
* loadparams: function for loading params
*
* @param MyCFG
*   apn::CfgFileOptions ConfigFile Options
*
* @param is3d
*   bool if the data is 3d
*
* @return
*   sVec
*/
dshn::Work::sVec dshn::Work::loadparams (apn::CfgFileOptions& MyCFG,bool is3d)
{
	sVec F = boost::assign::list_of<std::string>(DSHN_DEFAULT_STRN_GID)
	         (DSHN_DEFAULT_STRN_X)
	         (DSHN_DEFAULT_STRN_Y);
	if (is3d) F.push_back(DSHN_DEFAULT_STRN_Z);
	typedef std::set<std::string> sSet;
	sSet D= apn::Convert::StringToList<sSet>(
	            MyCFG.Find<std::string>(DSHN_DEFAULT_STRN_SYSTEM,DSHN_DEFAULT_STRN_FIELDS),
	            DSHN_DEFAULT_STRN_FIELDS_SEPARATOR);
	for (sVec::const_iterator it=F.begin(); it!=F.end(); ++it) {
		sSet::iterator st = D.find(*it);
		if(st!= D.end()) D.erase(st);
	}
	for (sSet::const_iterator st=D.begin(); st!=D.end(); ++st) {
		F.push_back(*st);
	}
	return F;
}

/**
* load: function for loading, this will be passed on
*
* @param index
*   std::string Index to Load
*
* @param Indata
*   sVec input data object for loading
*
* @param is3d
*   bool if the data is 3d
*
*
* @return
*   none
*/
void dshn::Work::load(std::string index, sVec Indata, bool is3d)
{
	bool e=false;
	if (is3d) {
		sp3Map::iterator it = pemap.find(index);
		if (it==pemap.end()) {
			boost::tie(it,e) = pemap.insert(std::make_pair<std::string,PointDataT3d::pointer>(index,PointDataT3d::create()));
		}
		PointDataT3d::Point P= {{
				apn::Convert::AnyToAny<std::string,DSHN_DEFAULT_COORDT>(Indata[1]),	// x
				apn::Convert::AnyToAny<std::string,DSHN_DEFAULT_COORDT>(Indata[2]),	// y
				apn::Convert::AnyToAny<std::string,DSHN_DEFAULT_COORDT>(Indata[3])	// z
			}
		};
		it->second->share()->Add(P, Indata);
	} else {
		sp2Map::iterator it = pdmap.find(index);
		if (it==pdmap.end()) {
			boost::tie(it,e) = pdmap.insert(std::make_pair<std::string,PointDataT2d::pointer>(index,PointDataT2d::create()));
		}
		PointDataT2d::Point P= {{
				apn::Convert::AnyToAny<std::string,DSHN_DEFAULT_COORDT>(Indata[1]),	// x
				apn::Convert::AnyToAny<std::string,DSHN_DEFAULT_COORDT>(Indata[2])	// y
			}
		};
		it->second->share()->Add(P, Indata);
	}
}

/**
* Work::run: mandatory function for web interface
*
* @param W
*   WebObject shared_ptr W
*
* @return
*   Bool status
*/
bool dshn::Work::run(apn::WebObject::pointer W)
{
	bool status=false;
	try {
		bool e=false;
		std::string index;
		boost::tuples::tie(e,index) = W->GetReqParam<std::string>(DSHN_DEFAULT_STRN_INDEX);
		if (!e) throw apn::GenericException(DSHN_WORK_PROGNO,"param not defined",DSHN_DEFAULT_STRN_INDEX);

		DSHN_DEFAULT_COORDT x,y,z;

		boost::tuples::tie(e,x) = W->GetReqParam<DSHN_DEFAULT_COORDT>(DSHN_DEFAULT_STRN_X);
		if (!e) throw apn::GenericException(DSHN_WORK_PROGNO,"param not found",DSHN_DEFAULT_STRN_X);

		boost::tuples::tie(e,y) = W->GetReqParam<DSHN_DEFAULT_COORDT>(DSHN_DEFAULT_STRN_Y);
		if (!e) throw apn::GenericException(DSHN_WORK_PROGNO,"param not found",DSHN_DEFAULT_STRN_X);

		bool is3d = false;
		boost::tuples::tie(e,z) = W->GetReqParam<DSHN_DEFAULT_COORDT>(DSHN_DEFAULT_STRN_Z);
		if (e) is3d=true;

		unsigned int no=1;
		boost::tuples::tie(e,no) = W->GetReqParam<unsigned int>(DSHN_DEFAULT_STRN_NO);
		if (!e) no=1;

		std::string fmt;
		boost::tuples::tie(e,fmt) = W->GetReqParam<std::string>(DSHN_DEFAULT_STRN_FMT);
		if (!e) fmt=DSHN_DEFAULT_VAL_FMT;

		std::string ctype,rstr;
		if (is3d) {
			sp3Map::iterator it = pemap.find(index);
			if (it != pemap.end()) {
				typedef std::vector<PointDataT3d::OutT> outvecT;
				PointDataT3d::Point P= {{x,y,z}};
				outvecT a = it->second->share()->GetNN<outvecT>(P,no);
				dshn::Dout<sVec,outvecT> d(params3d);
				status=d.Parse(fmt, a, ctype, rstr);
			} else {
				throw apn::GenericException(DSHN_WORK_PROGNO,"no index",index.c_str());
			}
		} else {
			sp2Map::iterator it = pdmap.find(index);
			if (it != pdmap.end()) {
				typedef std::vector<PointDataT2d::OutT> outvecT;
				PointDataT2d::Point P= {{x,y}};
				outvecT a = it->second->share()->GetNN<outvecT>(P,no);
				dshn::Dout<sVec,outvecT> d(params2d);
				status=d.Parse(fmt, a, ctype, rstr);
			} else {
				throw apn::GenericException(DSHN_WORK_PROGNO,"no index",index.c_str());
			}
		}
		if (status) {
			W->SetContentType(ctype);
			W->AddResponse(rstr.c_str(),rstr.length());
		}


	} catch (apn::GenericException& e) {
		std::cerr << e.ErrorCode_ << ":" << e.ErrorMsg_ << e.ErrorFor_ << std::endl;
	} catch (...) {
		std::cerr << "Unknown Runtime Error" << std::endl;
	}
	return status;
}
