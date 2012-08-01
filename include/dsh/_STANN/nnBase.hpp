/*****************************************************************************/
/*                                                                           */
/*  Header: nnBase.hpp                                                       */
/*                                                                           */
/*  Accompanies STANN Version 0.70 B                                         */
/*  Oct 01, 2009                                                             */
/*                                                                           */
/*  Copyright 2007, 2008                                                     */
/*  Michael Connor and Piyush Kumar                                          */
/*  Florida State University                                                 */
/*  Tallahassee FL, 32306-4532                                               */
/*                                                                           */
/*****************************************************************************/

#ifndef __NN_BASE_CLASS__
#define __NN_BASE_CLASS__

#include <vector>

/*! \file nnBase.hpp
\brief Contains the base class for NN algorithms */

using namespace std;
template <typename Point, unsigned int Dim, typename NumType>
class nnBase {
public:
	virtual void ksearch(Point, unsigned int, vector<long unsigned int>&, float=0) {
	};
	virtual void ksearch(Point, unsigned int, vector<long unsigned int>&, vector<double>&, float=0) {
	};
	virtual ~nnBase() {};
};
#endif
