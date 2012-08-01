/*****************************************************************************/
/*                                                                           */
/*  Header: sfcdata_work.hpp                                                        */
/*                                                                           */
/*  Accompanies STANN Version 0.70 B                                         */
/*  Oct 01, 2009                                                             */
/*                                                                           */
/*  Copyright 2007, 2008                                                     */
/*  Michael Connor and Piyush Kumar                                          */
/*  Florida State University                                                 */
/*  Tallahassee FL, 32306-4532                                               */
/*                                                                           */
/*  (modified by Shreos Roychowdhury)                                        */
/*                                                                           */
/*****************************************************************************/


#ifndef __SFCDATA_WORK___
#define __SFCDATA_WORK___

#include <cstdlib>
#include <cmath>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>

#include "compute_bounding_box.hpp"
#include "nnBase.hpp"
#include "pair_iter.hpp"
#include "qknn.hpp"
#include "zorder_lt.hpp"
#include "bsearch.hpp"

/*!
	\mainpage STANN Doxygen Index Page

	This Doxygen API is intended for users interested in modifying
	the STANN library.  For instructions on using STANN, see the
	STANN webpage at http://www.compgeom.com/~stann or the README.txt
	file located in the main directory of the STANN distribution.

	The API is still being updated.  For requests/comments/errors,
	please see our issues list (powered by google) at
	http://code.google.com/p/zorderlibrary/issues/list
*/

/*!
  \file
  \brief Space filling curve nearest neighbor search
  This file contains the implementation of a space filling curve
  nearest neighbor search data structure
*/


/*!
  \brief A space filling curve nearest neighbor class.

  This is the workhorse class for the sfcnn class.
  The Space Filling Curve Nearest Neighbor (SFCNN) algorithm sorts the
  input data set into 2-order Morton ordering. Nearest neighbors are then
  calculated based on that curve. The algorithm has a runtime of   O(ln(N)),
  a construction time of O(Nlog(N)), and a space requirement of
  O(N). The query functions of the algorithm are thread-safe.
*/
template <typename Point, typename Ptype=typename Point::__NumType>
class sfcdata_work {
public:
	sfcdata_work() {};
	~sfcdata_work() {};
	void ksearch(Point q, unsigned int k, std::vector<long unsigned int> &nn_idx, float Eps) {
		long unsigned int query_point_index;
		qknn que;
		query_point_index = BinarySearch(points, q, lt);
		ksearch_common(q, k, query_point_index, que, Eps);
		que.answer(nn_idx);
	}

	/*!
	  \brief Nearest Neighbor search function
	  Searches for the k nearest neighbors to the point q.  The answer
	  vector returned will contain the indexes to the answer points
	  This function is thread-safe.
	  \param q The query point
	  \param k The number of neighbors to return
	  \param nn_idx Answer vector
	  \param dist Distance Vector
	  \param eps Error tolerence, default of 0.0.
	*/
	void ksearch(Point q, unsigned int k, std::vector<long unsigned int> &nn_idx, std::vector<double> &dist, float Eps) {
		long unsigned int query_point_index;
		qknn que;
		query_point_index = BinarySearch(points, q, lt);
		ksearch_common(q, k, query_point_index, que, Eps);
		que.answer(nn_idx, dist);
	}
	/*!
	  \brief Initialize the sfc data structure
	  \param PointAr Array of Pints to use
	  \return bool status
	*/
	template <typename ArrT>
	bool sfcnn_do_init(ArrT& PointArr) {
		std::size_t N = PointArr.size();
		if (N==0) return false; // logic change  do it
		std::size_t Dim = PointArr[0].size();
		points.resize(N);
		pointers.resize(N);
		for (std::size_t i=0; i < N; ++i) {
			pointers[i] = (i);
			for (std::size_t j=0; j < Dim; ++j) {
				points[i][j] = PointArr[i][j];
			}
		}
		return sfcdata_work_init();
	}

private:
	typedef std::vector<Point> pVec;
	pVec points;
	typedef std::vector<long unsigned int> lVec;
	lVec pointers;
	zorder_lt<Point> lt;
	float eps;
	typename Point::__NumType max, min;

	void compute_bounding_box(Point q, Point &q1, Point &q2, double R) {
		cbb_work<Point, Ptype>::eval(q, q1, q2, R, max, min);
	}
	/*!
	  \brief Initialize the sfc data structure
	  \return bool status
	*/
	bool sfcdata_work_init() {
		max = (std::numeric_limits<typename Point::__NumType>::max)();
		min = (std::numeric_limits<typename Point::__NumType>::min)();

		if (points.size() == 0) {
			return false;
		}
		pair_iter<typename pVec::iterator, typename lVec::iterator>
		a(points.begin(), pointers.begin()),
		b(points.end(), pointers.end());
		std::sort(a,b,lt);
		return true;
	}

	void ksearch_common(Point q, unsigned int k, long unsigned int query_point_index, qknn &que, float Eps) {
		Point bound_box_lower_corner, bound_box_upper_corner;
		Point low, high;

		que.set_size(k);
		eps=(float) 1.0+Eps;
		if (query_point_index >= (k)) query_point_index -= (k);
		else query_point_index=0;

		long unsigned int initial_scan_upper_range=query_point_index+2*k+1;
		if (initial_scan_upper_range > (long unsigned int)points.size())
			initial_scan_upper_range = (long unsigned int)points.size();

		low = points[query_point_index];
		high = points[initial_scan_upper_range-1];
		for (long unsigned int i=query_point_index; i<initial_scan_upper_range; ++i) {
			que.update(points[i].sqr_dist(q), pointers[i]);
		}
		compute_bounding_box(q, bound_box_lower_corner, bound_box_upper_corner, sqrt(que.topdist()));

		if (lt(bound_box_upper_corner, high) && lt(low,bound_box_lower_corner)) {
			return;
		}

		//Recurse through the entire set
		recurse(0, points.size(), q, que, bound_box_lower_corner, bound_box_upper_corner, query_point_index, initial_scan_upper_range);
	}

	inline void recurse(long unsigned int s,     // Starting index
	                    long unsigned int n,     // Number of points
	                    Point q,  // Query point
	                    qknn &ans, // Answer que
	                    Point &bound_box_lower_corner,
	                    Point &bound_box_upper_corner,
	                    long unsigned int initial_scan_lower_range,
	                    long unsigned int initial_scan_upper_range) {
		if (n < 4) {
			if (n == 0) return;

			bool update=false;
			for (long unsigned int i=0; i < n; ++i) {
				if ((s+i >= initial_scan_lower_range)
				        && (s+i < initial_scan_upper_range))
					continue;
				update = ans.update(points[s+i].sqr_dist(q), pointers[s+i]) || update;
			}
			if (update)
				compute_bounding_box(q, bound_box_lower_corner, bound_box_upper_corner, sqrt(ans.topdist()));
			return;
		}

		if ((s+n/2 >= initial_scan_lower_range) && (s+n/2 < initial_scan_upper_range)) {
		} else if (ans.update(points[s+n/2].sqr_dist(q), pointers[s+n/2]))
			compute_bounding_box(q, bound_box_lower_corner, bound_box_upper_corner, sqrt(ans.topdist()));

		double dsqb = lt.dist_sq_to_quad_box(q,points[s], points[s+n-1]);

		if (dsqb > ans.topdist()) return;
		if (lt(q,points[s+n/2])) {
			recurse(s, n/2, q, ans, bound_box_lower_corner, bound_box_upper_corner, initial_scan_lower_range, initial_scan_upper_range);
			if (lt(points[s+n/2],bound_box_upper_corner))
				recurse(s+n/2+1,n-n/2-1, q, ans, bound_box_lower_corner, bound_box_upper_corner, initial_scan_lower_range, initial_scan_upper_range);
		} else {
			recurse(s+n/2+1, n-n/2-1, q, ans, bound_box_lower_corner, bound_box_upper_corner, initial_scan_lower_range, initial_scan_upper_range);
			if (lt(bound_box_lower_corner,points[s+n/2]))
				recurse(s, n/2, q, ans, bound_box_lower_corner, bound_box_upper_corner, initial_scan_lower_range, initial_scan_upper_range);
		}
	}
};
#endif // __SFCDATA_WORK___
