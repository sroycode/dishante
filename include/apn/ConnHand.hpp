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
#define APN_CONNHAND_HPP_PROGNO 14050

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <map>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include "Exception.hh"
#include "Convert.hpp"
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
	typedef boost::function<bool(apn::WebObject::pointer)> ActionT;
	typedef std::vector<boost::asio::const_buffer> BufferType;

	/**
	 * create : static singleton construction creates new connection
	 *
	 * @param io_service
	 *   io_service
	 *
	 * @param tref
	 *   ActionT ext function reference
	 *
	 * @return
	 *   pointer shared_ptr to newly allocated object
	 */
	static pointer create(boost::asio::io_service& io_service, ActionT tref) {
		return pointer(new ConnHand(io_service,tref));
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
	 * @return
	 *   none
	 */
	void start() {
		data_len=0;
		fHeaders.clear();
		boost::asio::async_read(bsocket_, boost::asio::buffer(bbuffer), boost::asio::transfer_at_least(1),
		                        strand_.wrap(boost::bind(&ConnHand::HandleReadInputHeaders,
		                                     shared_from_this(),
		                                     boost::asio::placeholders::error,
		                                     boost::asio::placeholders::bytes_transferred)));
	}

private:
	/* variables */
	boost::asio::io_service& io_service_;
	boost::asio::ip::tcp::socket bsocket_;
	boost::asio::io_service::strand strand_;
	ActionT tref_;


	int32_t RespLen;

	boost::array<char, 8192> bbuffer;

	std::string fHeaders;
	BufferType rbuffer;
	uint64_t data_len;

	/**
	 * Constructor : private Constructor
	 *
	 * @param io_service
	 *   io_service by ref
	 *
	 * @param tref
	 *   ActionT ext function reference
	 *
	 * @return
	 *   none
	 */
	ConnHand(boost::asio::io_service& io_service,ActionT tref) :
		io_service_(io_service),
		bsocket_(io_service),
		strand_(io_service),
		tref_(tref),
		data_len(0)
	{}

	/**
	 * HandleReadInputHeaders: read headers from incoming request
	 *
	 * @param err
	 *   error_code
	 *
	 * @param len
	 *   size_t length
	 *
	 * @return
	 *   none
	 */
	void HandleReadInputHeaders(const boost::system::error_code& err, std::size_t len) {
		if (!err) {
			if (fHeaders.empty())
				fHeaders=std::string(bbuffer.data(),len);
			else
				fHeaders+=std::string(bbuffer.data(),len);

			std::string::size_type he_end =fHeaders.find(APN_CONNHAND_CTRLFTWO);
			if (he_end == std::string::npos) { // going to read rest of headers
				boost::asio::async_read(bsocket_, boost::asio::buffer(bbuffer), boost::asio::transfer_at_least(1),
				                        strand_.wrap(boost::bind(&ConnHand::HandleReadInputHeaders,
				                                     shared_from_this(),
				                                     boost::asio::placeholders::error,
				                                     boost::asio::placeholders::bytes_transferred)));
			} else {
				// SHREOS
				data_len = he_end+4;
				std::string::size_type cl_start = fHeaders.find("Content-Length: ");
				if ((cl_start != std::string::npos)) {
					if (cl_start+16 >= he_end) shutdown();
					std::string cl_str;
					for (std::string::size_type i=cl_start+16; std::isdigit(fHeaders[i]); ++i) {
						cl_str.push_back(fHeaders[i]);
					}
					try {
						data_len += boost::lexical_cast<std::size_t>(cl_str);
					} catch (...) {
						shutdown();
					}
					if (fHeaders.length()>=data_len) {
						ProcessLocal();
					} else {
						boost::asio::async_read(bsocket_, boost::asio::buffer(bbuffer), boost::asio::transfer_at_least(1),
						                        strand_.wrap(boost::bind(&ConnHand::HandleReadInputData,
						                                     shared_from_this(),
						                                     boost::asio::placeholders::error,
						                                     boost::asio::placeholders::bytes_transferred)));

					}
				} else {
					ProcessLocal();
				}
			}
		} else {
			shutdown();
		}
	}

	/**
	 * HandleReadInputData: read data from incoming request
	 *
	 * @param err
	 *   error_code
	 *
	 * @param len
	 *   size_t length
	 *
	 * @return
	 *   none
	 */
	void HandleReadInputData(const boost::system::error_code& err, std::size_t len) {
		if (!err) {
			if (fHeaders.empty())
				fHeaders=std::string(bbuffer.data(),len);
			else
				fHeaders+=std::string(bbuffer.data(),len);

			// reading complete when read upto data_len
			if (fHeaders.length()<data_len) {
				boost::asio::async_read(bsocket_, boost::asio::buffer(bbuffer), boost::asio::transfer_at_least(1),
				                        strand_.wrap(boost::bind(&ConnHand::HandleReadInputData,
				                                     shared_from_this(),
				                                     boost::asio::placeholders::error,
				                                     boost::asio::placeholders::bytes_transferred)));
			} else {
				// pass control to local req processor
				ProcessLocal();
			}
		} else {
			shutdown();
		}
	}


	/**
	 * ProcessLocal: start processing local part of request using tref,
	 *
	 * @return
	 *   none
	 */
	void ProcessLocal() {
		apn::WebObject::pointer W = apn::WebObject::create(fHeaders);
		bool status = tref_(W->share());
		if (status) {
			rbuffer = W->GetReply();
		} else {
			std::string NotFoundReply="HTTP/1.0 404 NOT FOUND" APN_CONNHAND_CTRLF;
			// modified for boost
			rbuffer.push_back(boost::asio::const_buffers_1(NotFoundReply.c_str(),NotFoundReply.size()));
		}
		boost::asio::async_write(bsocket_, rbuffer,
		                         strand_.wrap(boost::bind(&ConnHand::graceful, shared_from_this(),
		                                      boost::asio::placeholders::error)));
	}

	/**
	 * graceful : connclosure
	 *
	 */
	void graceful(const boost::system::error_code& e) {
		if (!e) {
			// Initiate graceful connection closure.
			boost::system::error_code ignored_ec;
			bsocket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		} else
			shutdown();
	}

	/**
	 * shutdown: stop all
	 */
	void shutdown() {
		bsocket_.close();
	}

};
} // namespace apn
#endif /* _APN_CONNHAND_HPP_ */
