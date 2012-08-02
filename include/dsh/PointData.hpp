/**
* @project dishante
* @file include/dsh/PointData.hpp
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
* PointData type encapsulating the SFC
*
*/

#ifndef _DSH_POINT_DATA_HPP_
#define _DSH_POINT_DATA_HPP_
#define DSH_POINT_DATA_HPP_PROGNO 1112
#ifndef DSH_POINT_DATA_MAX_POINTS
#define DSH_POINT_DATA_MAX_POINTS 10
#endif

#include <vector>
#include <string>
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <apn/Convert.hpp>
#include "SfcData.hpp"


namespace dsh {
template <class CoordT, class AttrT>
class PointData : private boost::noncopyable, public boost::enable_shared_from_this<PointData<CoordT,AttrT> > {
public:
	typedef typename boost::shared_ptr< PointData<CoordT,AttrT> > pointer;
	typedef typename boost::array<CoordT, 2> Point;
	typedef typename std::vector<Point> pVec;
	typedef typename dsh::SfcData<pVec, 2, CoordT> SfcT;
	typedef typename boost::tuple<long unsigned int,double,AttrT> OutT;
	typedef typename std::vector<AttrT> aVec;


	/**
	* create : static construction creates new first time
	*
	* @return
	*   none
	*/
	static pointer create() {
		return pointer(new PointData());
	}

	/**
	* share : return instance
	*
	* @return
	*   none
	*/
	pointer share() {
		return this->shared_from_this();
	}

	/**
	* virtual destructor
	*/
	virtual ~PointData() {}

	/**
	* Add: Add to Array
	*
	* @param x
	*   CoordT elem x
	*
	* @param y
	*   CoordT elem y
	*
	* @param a
	*   AttrT elem a
	*
	* @return
	*   none
	*/
	void Add(CoordT x,CoordT y, AttrT a) {

		if (PointDataSize!=0)
			throw apn::GenericException(DSH_POINT_DATA_HPP_PROGNO,"PointDataSize exists"," while addition");
		Point Q= {{x,y}};
		PointDataVec.push_back(Q);
		AttrDataVec.push_back(a);
	}

	/**
	* Lock : make ready to search by populating SFC since that will only populate from fixed array
	*
	* @return
	*   none
	*/
	void Lock() {
		if (PointDataSize!=0)
			throw apn::GenericException(DSH_POINT_DATA_HPP_PROGNO,"PointDataSize exists"," when locking");
		PointDataSfc = SfcT(PointDataVec);
		PointDataSize=PointDataVec.size();
		std::cerr << "PointDataSize " << std::endl;
	}

	/**
	* GetNN: find nearest point
	*
	* @param x
	*   CoordT elem x
	*
	* @param y
	*   CoordT elem y
	*
	* @param nores
	*   unsigned long no of results
	*
	* @return
	*   T output point and distance list
	*/
	template<class T>
	T GetNN(CoordT& x, CoordT& y,unsigned int nores) {
		if (PointDataSize==0)
			throw apn::GenericException(DSH_POINT_DATA_HPP_PROGNO,"PointDataSize is zero"," when searching");
		std::vector<OutT> aout;
		typename SfcT::lVec answer;
		typename SfcT::dVec distance;
		Point Q= {{x,y}};
		if (nores>PointDataSize) nores=PointDataSize;
		PointDataSfc.ksearch(Q, (unsigned long)nores, answer,distance,0);
		for (std::size_t i=0; i<answer.size(); ++i) {
			aout.push_back(boost::make_tuple(answer[i],ceil(sqrt(distance[i])), AttrDataVec[std::size_t(answer[i])]));
		}
		return aout;
	}

private:
	/* data */
	pVec PointDataVec;
	SfcT PointDataSfc;
	aVec AttrDataVec;
	unsigned long int PointDataSize;

	/**
	* Constructor : private Constructor
	*
	* @param progname
	*   String executable name
	*
	* @param MyCFG
	*   apn::CfgFileOptions ConfigFile Options
	*
	* @return
	*   none
	*/
	PointData() : PointDataSize(0) {}

};
} //namespace dsh
#endif /* _DSH_POINT_DATA_HPP_ */
