/**
 * @project apophnia++
 * @file include/apn/InThreadQueue.hpp
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
 * The InThreadQueue class is used for job queing in same thread pool or process
 *
 */

#ifndef _APN_INTHREAD_QUEUE_HPP_
#define _APN_INTHREAD_QUEUE_HPP_
#define APN_INTHREAD_QUEUE_HPP_PROGNO 1008

#include <string>
#include <queue>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/utility/value_init.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <apn/Exception.hh>

namespace apn {
/**
 * @brief: The InThreadQueue class is used for job queing in same thread pool or process
 *
 */
class InThreadQueue : private boost::noncopyable, public boost::enable_shared_from_this<InThreadQueue> {
public:
	typedef boost::shared_ptr<apn::InThreadQueue> pointer;
	typedef std::string JobElem;
	typedef boost::tuple<boost::posix_time::ptime, apn::InThreadQueue::JobElem, unsigned int> JobType;
	typedef boost::function<void(JobType)> processfunction;

	struct JobTypeComp {
		bool operator() (const JobType& x, const JobType& y) const {
			return (x.get<0>() > y.get<0>());
		}
	};

	/**
	 * create : static construction creates new first time
	 *
	 * @param thread_num
	 *   unsigned int threads
	 *
	 * @param pf
	 *   processfunction processing function for all threads
	 *
	 * @return
	 *   none
	 */
	static pointer create(unsigned int thread_num, processfunction pf) {
		return pointer(new InThreadQueue(thread_num,pf));
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
	 * Destructor
	 */
	virtual ~InThreadQueue () {
		my_io_service_work_ptr.reset();
		thr_grp.join_all();
	}

	/**
	 * Push : Push data into queue
	 *
	 * @param j
	 *   JobType job to insert
	 *
	 * @param p
	 *   (optional) Bool if on priority
	 *
	 * @return
	 *   none
	 */
	void Push(JobType const& j, bool p=false ) {
		boost::mutex::scoped_lock lock(the_mutex);
		boost::posix_time::ptime t = boost::posix_time::second_clock::local_time();
		boost::posix_time::ptime u = j.get<0>();
		if (p) {
			if (!the_queue.empty()) {
				JobType popped_value = the_queue.top();
				if ( t < popped_value.get<0>() )
					u = popped_value.get<0>() - boost::posix_time::millisec(1);
			}
		}
		the_queue.push(boost::make_tuple(u,j.get<1>(),j.get<2>()));
		new_queue_item=true;
		lock.unlock();
		the_condition_variable.notify_one();
	}

	/**
	 * Empty : return if queue is empty
	 *
	 * @return
	 *   Bool is empty
	 */
	bool Empty() const {
		boost::mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}


private:
	const unsigned int thread_num_;
	bool new_queue_item;
	processfunction pf_;
	boost::asio::io_service my_io_service;
	boost::shared_ptr<boost::asio::io_service::work> my_io_service_work_ptr;
	boost::asio::deadline_timer my_deadline_timer;
	boost::thread_group thr_grp;
	std::priority_queue<JobType,std::vector<JobType>,JobTypeComp> the_queue;
	mutable boost::mutex the_mutex;
	boost::condition_variable the_condition_variable;


	/**
	 * Constructor: function to initialize from headers
	 *
	 * @param thread_num
	 *   unsigned int threads
	 *
	 * @param pf
	 *   processfunction processing function for all threads
	 *
	 * @return
	 *   none
	 */
	InThreadQueue (unsigned int thread_num, processfunction pf) :
		thread_num_(thread_num),
		new_queue_item(false),
		pf_(pf),
		my_deadline_timer(my_io_service) {
		try {
			my_io_service_work_ptr.reset( new boost::asio::io_service::work(my_io_service) );
			for (std::size_t i = 0; i < thread_num_; ++i) {
				thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &my_io_service));
			}
			// one more thread to monitor
			thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &my_io_service));
			my_io_service.dispatch(boost::bind(&apn::InThreadQueue::PostFromQ,this));

		} catch (std::exception& e) {
			throw apn::GenericException(APN_INTHREAD_QUEUE_HPP_PROGNO,"Shutdown ",e.what());
		} catch (...) {
			throw apn::GenericException(APN_INTHREAD_QUEUE_HPP_PROGNO,"Shutdown ","unknown");
		}
	}

	/**
	 * PostFromQ : try to get an elem, wait if flag set, run in loop
	 *
	 * @return
	 *   None
	 */
	void PostFromQ() {
		while (true) {
			boost::mutex::scoped_lock lock(the_mutex);
			while((!new_queue_item) ) {
				the_condition_variable.wait(lock);
			}
			do {
				new_queue_item=false;
				boost::posix_time::ptime u = boost::posix_time::second_clock::local_time();
				JobType jt =the_queue.top();
				if (u>=jt.get<0>()) {
					the_queue.pop();
					my_io_service.post(boost::bind(pf_,jt));
				} else {
					my_deadline_timer.expires_from_now(jt.get<0>()-u);
					my_deadline_timer.async_wait(boost::bind(&apn::InThreadQueue::ResetNewItem, this,
					                             boost::asio::placeholders::error));
					break;
				}
			} while (!the_queue.empty());
			lock.unlock();
			the_condition_variable.notify_one();
		}
	}
	/**
	 * ResetNewItem : reset the new_queue_item flag
	 *
	 * @return
	 *   None
	 */
	void ResetNewItem(const boost::system::error_code& error ) {
		if (error) return;
		boost::mutex::scoped_lock lock(the_mutex);
		new_queue_item=true;
		lock.unlock();
		the_condition_variable.notify_one();
	}

};

} // namespace apn
#endif
