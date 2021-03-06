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
#define APN_CONNSERV_HPP_PROGNO 14052

#include <boost/tuple/tuple.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "ConnHand.hpp"

namespace apn {
/**
 * @brief: The ConnServ class is used for web service
 *
 */
class ConnServ :
	private boost::noncopyable,
	public boost::enable_shared_from_this<ConnServ> {
public:
	typedef boost::shared_ptr<apn::ConnServ> pointer;
	typedef boost::function<bool(apn::WebObject::pointer)> ActionT;

	/**
	 * create : static construction creates new first time
	 *
	 * @param thread_num
	 *   unsigned int threads
	 *
	 * @param address
	 *   std::string address to bind
	 *
	 * @param port
	 *   std::string port to bind
	 *
	 * @param tref
	 *   ActionT actionable on request
	 *
	 * @return
	 *   none
	 */
	static pointer create(unsigned int thread_num, std::string address,
	                      std::string port, ActionT tref) {
		return pointer(new ConnServ(thread_num,address.c_str(),port.c_str(),tref));
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
	virtual ~ConnServ () {}

	/**
	 * Run : the actual thread runner
	 *
	 * @return
	 *   none
	 */
	void Run() {
		try {
			boost::thread_group thr_grp;
			// Create a pool of threads to run all of the io_services.
			for (std::size_t i = 0; i < thread_num_; ++i) {
				thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
			}
			thr_grp.join_all();
		} catch (std::exception& e) {
			throw apn::GenericException(APN_CONNSERV_HPP_PROGNO,"Shutdown ",e.what());
		} catch (...) {
			throw apn::GenericException(APN_CONNSERV_HPP_PROGNO,"Shutdown ","Unhandled");
		}
	}


private:
	unsigned int thread_num_;
	ActionT tref_;
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	ConnHand::pointer new_connection_;


	/**
	 * Constructor: function to initialize from headers
	 *
	 * @param thread_num
	 *   unsigned int threads
	 *
	 * @param address
	 *   CString address to bind
	 *
	 * @param port
	 *   CString port to bind
	 *
	 * @param tref
	 *   ActionT actionable on request
	 *
	 * @return
	 *   none
	 */
	ConnServ(unsigned int& thread_num, const char* address,
	         const char* port, ActionT tref) :
		thread_num_(thread_num),
		tref_(tref),
		acceptor_(io_service_),
		new_connection_(apn::ConnHand::create(io_service_,tref_)) {
		// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
		boost::asio::ip::tcp::resolver resolver(io_service_);
		boost::asio::ip::tcp::resolver::query query(address, port);
		boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen();
		acceptor_.async_accept(new_connection_->socket(),
		                       boost::bind(&ConnServ::HandleAccept, this,
		                                   boost::asio::placeholders::error));
	}

	/**
	 * HandleStop : stop the threads
	 *
	 * @return
	 *   none
	 */
	void HandleStop() {
		io_service_.stop();
	}

	/**
	 * HandleAccept: Run when new connection is accepted
	 * Operations:
	 * 1. start next conn
	 * 2. start new accept operation
	 *
	 * @param error
	 *   error_code
	 *
	 * @return
	 *   none
	 */
	void HandleAccept(const boost::system::error_code& error) {
		if (!error) {
			new_connection_->start();
			new_connection_=ConnHand::create(io_service_, tref_);
			acceptor_.async_accept(new_connection_->socket(),
			                       boost::bind(&ConnServ::HandleAccept, this,
			                                   boost::asio::placeholders::error));
		}

	}
};

} // namespace apn
#endif
