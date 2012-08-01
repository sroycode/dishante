/**
 * @project apophnia++
 * @file include/apn/ConnServ.hpp
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
 * ConnServ : Class to Handle the web asio
 *
 */

#ifndef _APN_CONNSERV_HPP_
#define _APN_CONNSERV_HPP_
#define APN_CONNSERV_HPP_PROGNO 1032

#include <deque>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include "ConnHand.hpp"
#include "Exception.hh"

#define APN_CONNSERV_TIMER_FREQ 5


namespace apn {
template<class T>
class ConnServ {
public:
	/**
	 * create : static singleton construction creates new connection
	 *
	 * @param thread_num
	 *   int no of threads
	 * @param port
	 *   int listening port
	 * @param tref
	 *   T external object reference
	 *
	 * @return
	 *   none
	 */
	static void create (int thread_num, int port, T tref) {
		try {
			ios_deque io_services;
			std::deque<boost::asio::io_service::work> io_service_work;

			boost::thread_group thr_grp;
			for (int i = 0; i < thread_num; ++i) {
				io_service_ptr ios(new boost::asio::io_service);
				io_services.push_back(ios);
				io_service_work.push_back(boost::asio::io_service::work(*ios));
				thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, ios));
			}
			apn::ConnServ<T> ConnServ_(io_services,port,tref->share());
			thr_grp.join_all();
		} catch (std::exception& e) {
			throw apn::GenericException(APN_CONNSERV_HPP_PROGNO,"Shutdown ",e.what());
		}
	}


private:
	typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
	typedef std::deque<io_service_ptr> ios_deque;
	/**
	 * Constructor : private Constructor
	 *
	 * @param io_service
	 *   io_service by ref
	 * @param port
	 *   int listening port
	 * @param tref
	 *   T external object reference
	 *
	 * @return
	 *   none
	 */
	ConnServ(const ios_deque& io_services, int port, T tref)
		: io_services_(io_services),
		  acceptor_(*io_services.front(),boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
		  tref_(tref) {
		StartAccept();
	}
	/**
	 * StartAccept: start accepting conns
	 */
	void StartAccept() {
		// Round robin.
		io_services_.push_back(io_services_.front());
		io_services_.pop_front();
		apn::ConnHand::pointer new_connection = apn::ConnHand::create(*io_services_.front());

		acceptor_.async_accept(new_connection->socket(),
		                       boost::bind(&ConnServ::HandleAccept, this, new_connection,
		                                   boost::asio::placeholders::error));
	}
	/**
	 * HandleAccept: Run when new connection is accepted
	 * Operations:
	 * 1. start next conn
	 * 2. start new accept operation
	 * @param new connhand pointer, err
	 *
	 * @param new_connection
	 *   ConnHand-pointer shared
	 * @param error
	 *   error_code
	 *
	 * @return
	 *   none
	 */
	void HandleAccept(apn::ConnHand::pointer new_connection, const boost::system::error_code& error) {
		if (!error) {
			new_connection->start<T>(tref_->share());
			StartAccept();
		}
	}

	ios_deque io_services_;
	boost::asio::ip::tcp::acceptor acceptor_;
	T tref_;
};
} // namespace

#endif /* _APN_CONNSERV_HPP_ */
