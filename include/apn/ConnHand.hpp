/**
 * @project apophnia++
 * @file include/apn/ConnHand.hpp
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
 * ConnHand : Class to Handle the actual connnections
 *
 */

#ifndef _APN_CONNHAND_HPP_
#define _APN_CONNHAND_HPP_
#define APN_CONNHAND_HPP_PROGNO 1031

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "ConvertStr.hpp"
#include "WebObject.hpp"

#define APN_CONNHAND_CTRLF "\r\n"
#define APN_CONNHAND_CTRLFTWO "\r\n\r\n"

namespace apn {

class ConnHand : public boost::enable_shared_from_this<ConnHand> {
	/**
	 * @brief: ConnHand : Class to Handle the actual connnections
	 */
public:
	typedef boost::shared_ptr<ConnHand> pointer;

	/**
	 * create : static singleton construction creates new connection
	 *
	 * @param io_service
	 *   io_service
	 *
	 * @return
	 *   pointer shared_ptr to newly allocated object
	 */
	static pointer create(boost::asio::io_service& io_service) {
		return pointer(new ConnHand(io_service));
	}

	/**
	 * socket: Return browser socket, associated with this ConnHand. This socket used in accept operation
	 *
	 * @return
	 *   socket by reference
	 */
	boost::asio::ip::tcp::socket& socket() {
		return bsocket_;
	}

	/**
	 * start: start receiving a request (0)
	 *
	 * @param tref
	 *   T external object reference
	 *
	 * @return
	 *   none
	 */
	template<class T>
	void start(T tref) {
		fHeaders.clear();
		boost::asio::async_read(bsocket_, boost::asio::buffer(bbuffer), boost::asio::transfer_at_least(1),
		                        strand_.wrap(boost::bind(&ConnHand::HandleReadInputHeaders<T>,
		                                     shared_from_this(),
		                                     tref,
		                                     boost::asio::placeholders::error,
		                                     boost::asio::placeholders::bytes_transferred)));
	}

private:
	/**
	 * Constructor : private Constructor
	 *
	 * @param io_service
	 *   io_service by ref
	 *
	 * @return
	 *   none
	 */
	ConnHand(boost::asio::io_service& io_service) :
		io_service_(io_service),
		bsocket_(io_service),
		strand_(io_service)
	{}

	/**
	 * HandleReadInputHeaders: read headers from incoming request
	 *
	 * @param tref
	 *   T external object reference
	 * @param err
	 *   error_code
	 * @param len
	 *   size_t length
	 *
	 * @return
	 *   none
	 */
	template<class T>
	void HandleReadInputHeaders(T tref, const boost::system::error_code& err, std::size_t len) {
		if (!err) {
			if (fHeaders.empty())
				fHeaders=std::string(bbuffer.data(),len);
			else
				fHeaders+=std::string(bbuffer.data(),len);
			if (fHeaders.find(APN_CONNHAND_CTRLFTWO) == std::string::npos) { // going to read rest of headers
				boost::asio::async_read(bsocket_, boost::asio::buffer(bbuffer), boost::asio::transfer_at_least(1),
				                        strand_.wrap(boost::bind(&ConnHand::HandleReadInputHeaders<T>,
				                                     shared_from_this(),
				                                     tref,
				                                     boost::asio::placeholders::error,
				                                     boost::asio::placeholders::bytes_transferred)));
			} else {
				// pass control to local req processor
				ProcessLocal(tref);
			}
		} else {
			shutdown();
		}
	}


	/**
	 * ProcessLocal: start processing local part of request using tref,
	 *    should have tref->run()
	 *
	 * @param tref
	 *   T external object reference
	 *
	 * @return
	 *   none
	 */
	template<class T>
	void ProcessLocal(T tref) {
		apn::WebObject::pointer W = apn::WebObject::create(fHeaders);
		bool status = tref->run(W->share());
		if (status) {
			rbuffer = W->GetReply();
		} else {
			std::string NotFoundReply="HTTP/1.0 404 NOT FOUND" APN_CONNHAND_CTRLF;
			// modified for boost 
			rbuffer.push_back(boost::asio::const_buffers_1(NotFoundReply.c_str(),NotFoundReply.size()));
		}
		/**
				boost::asio::async_write(bsocket_,
				                         rbuffer,
				                         strand_.wrap(
				                             boost::bind(&ConnHand::graceful, shared_from_this(),
				                                     boost::asio::placeholders::error)));
		*/
		boost::system::error_code error;
		boost::asio::write(bsocket_,
		                   rbuffer,
		                   boost::asio::transfer_all(),
		                   error);
	}

	/**
	 * graceful : connclosure
	 *
	void graceful(const boost::system::error_code& e) {
		if (!e) {
			// Initiate graceful connection closure.
			boost::system::error_code ignored_ec;
			bsocket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		}
	}
	 */

	/**
	 * shutdown: stop all
	 */
	void shutdown() {
		bsocket_.close();
	}

	/* variables */
	boost::asio::io_service& io_service_;
	boost::asio::ip::tcp::socket bsocket_;
	boost::asio::io_service::strand strand_;

	int32_t RespLen;

	boost::array<char, 8192> bbuffer;

	std::string fHeaders;
	apn::WebObject::BufferType rbuffer;

};
} // namespace apn
#endif /* _APN_CONNHAND_HPP_ */
