/**
* @project dishante
* @file src/Work.hpp
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
* Workhorse Headers
*
*/

#ifndef _DSHN_WORK_HPP_
#define _DSHN_WORK_HPP_
#define DSHN_WORK_PROGNO 2002

#include <string>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Default.hh"

#include <apn/CfgFileOptions.hpp>
#include <apn/WebObject.hpp>
#include <dsh/PointData.hpp>



namespace dshn {
class Work : private boost::noncopyable, public boost::enable_shared_from_this<Work> {
public:
	typedef std::vector<std::string> sVec;
	typedef std::vector<std::string> AttrT;

	typedef dsh::PointData<DSHN_DEFAULT_COORDT,AttrT,2> PointDataT2d;
	typedef dsh::PointData<DSHN_DEFAULT_COORDT,AttrT,3> PointDataT3d;

	typedef std::map<std::string,PointDataT2d::pointer> sp2Map;
	typedef std::map<std::string,PointDataT3d::pointer> sp3Map;
	typedef boost::shared_ptr<Work> pointer;
	/**
	* create : static construction creates new first time
	*
	* @param MyCFG
	*   apn::CfgFileOptions ConfigFile Options
	*
	* @return
	*   none
	*/
	static pointer create(apn::CfgFileOptions& MyCFG) {
		return pointer(new Work(MyCFG));
	}
	/**
	* share : return instance
	*
	* @return
	*   none
	*/
	pointer share() {
		return shared_from_this();
	}
	/**
	* virtual destructor
	*/
	virtual ~Work () {}
	/**
	* run: mandatory function for web interface
	*
	* @param W
	*   WebObject W
	*
	* @return
	*   Bool status
	*/
	bool run(apn::WebObject::pointer W);
private:
	sp2Map pdmap;
	sp3Map pemap;
	sVec params2d;
	sVec params3d;
	/**
	* Constructor : private Constructor
	*
	* @param MyCFG
	*   apn::CfgFileOptions ConfigFile Options
	*
	* @return
	*   none
	*/
	Work(apn::CfgFileOptions& MyCFG);

	/**
	* loadparams: function for loading params
	*
	* @param MyCFG
	*   apn::CfgFileOptions ConfigFile Options
	*
	* @param is3d
	*   bool flag to see if data is 3d, is used to create the two vecs
	*
	* @return
	*   sVec
	*/

	sVec loadparams(apn::CfgFileOptions& MyCFG, bool is3d);

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
	*   bool is it 3d
	*
	* @return
	*   none
	*/
	void load(std::string index, sVec Indata, bool is3d);
};
}
#endif /* _DSHN_WORK_HPP_ */
