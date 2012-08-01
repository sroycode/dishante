/**
* @project dishante
* @file include/dsh/SfcData.hpp
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
* SFC Data Objects : the wrapper class acts as ext interface for Nearest Neighbour search class
*
*/

#ifndef _DSH_SFCDATA_HPP_
#define _DSH_SFCDATA_HPP_
#define DSH_SFCDATA_HPP_PROGNO 1111

#include <vector>
#include "_STANN/sfcdata_work.hpp"
#include <apn/Exception.hh>

namespace dsh {
template <typename ArrT, unsigned int Dim, typename NumType>
class SfcData : public nnBase<typename ArrT::value_type, Dim, NumType> {
public:
	typedef std::vector<long unsigned int> lVec;
	typedef std::vector<double> dVec;
	SfcData() {};
	/**
	* Constructor : the used constructor
	*
	* @param PointArr
	*   ArrT Array of input Points
	*
	*/
	SfcData(ArrT& PointArr) : max(PointArr.size()) {
		if (sizeof(NumType) != sizeof(typename ArrT::value_type::value_type))
			throw apn::GenericException(DSH_SFCDATA_HPP_PROGNO,"sfcnn Numeric Type Mismatch","");
		if (! NN.sfcnn_do_init(PointArr))
			throw apn::GenericException(DSH_SFCDATA_HPP_PROGNO,"Cannot init Sfc Data","");
	};

	/**
	* virtual destructor
	*/
	virtual ~SfcData() {};

	/**
	* ksearch: thread safe search for k NN points
	*
	* @param q
	*   T Query Point
	* @param k
	*   unsigned int No of results to retrieve
	* @param nn_idx
	*   lVec Vector of Point Ids to be populated
	* @param eps
	*   float optional Error tolerence, default of 0.0.
	*
	* @return
	*   none
	*/

	template <typename T>
	void ksearch(T q, unsigned int k, lVec &nn_idx, float eps = 0) {
		k=(k>max)?max:k;
		reviver::dpoint<NumType, Dim> qry;
		for (unsigned int j=0; j < Dim; ++j) {
			qry[j]=q[j];
		}
		NN.ksearch(qry,k,nn_idx,eps);
	}

	/**
	* ksearch: thread safe search for k NN points
	*
	* @param q
	*   T Query Point
	* @param k
	*   unsigned int No of results to retrieve
	* @param nn_idx
	*   lVec Vector of Point Ids to be populated
	* @param dist
	*   dVec Vector of distances corresp. to the above Point Ids to be populated
	* @param eps
	*   float optional Error tolerence, default of 0.0.
	*
	* @return
	*   none
	*/
	template <typename T>
	void ksearch(T q, unsigned int k, lVec &nn_idx, dVec &dist, float eps=0) {
		k=(k>max)?max:k;
		reviver::dpoint<NumType, Dim> qry;
		for (unsigned int j=0; j < Dim; ++j) {
			qry[j]=q[j];
		}
		NN.ksearch(qry,k,nn_idx,dist,eps);
	}

private:
	sfcdata_work<reviver::dpoint<NumType, Dim> > NN;
	unsigned long int max;
};

} //namespace dsh
#endif // _DSH_SFCDATA_HPP_
