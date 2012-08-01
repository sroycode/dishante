#ifndef _APN_ASYNC_HTTP_GET_HPP_
#define _APN_ASYNC_HTTP_GET_HPP_

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <apn/Convert.hpp>


namespace apn {

// using boost::asio::ip::tcp;

class AsyncHttpGet {
public:
	/**
	 * Constructor: function to initialize  and start the process
	 *
	 * @param io_service
	 *   boost::asio::io_service supplied io_service
	 *
	 * @param server
	 *   String http server
	 *
	 * @param path
	 *   String URL path
	 *
	 * @return
	 *   none
	 */
	AsyncHttpGet(boost::asio::io_service& io_service,
	             const std::string& server, const std::string& path)
		: resolver_(io_service),
		  socket_(io_service),
		  http_status_code(0),
		  Error(false) {
		// Form the request. We specify the "Connection: close" header so that the
		// server will close the socket after transmitting the response. This will
		// allow us to treat all data up until the EOF as the content.
		std::ostream request_stream(&request_);
		request_stream << "GET " << path << " HTTP/1.0\r\n";
		request_stream << "Host: " << server << "\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";

		// Start an asynchronous resolve to translate the server and service names
		// into a list of endpoints.
		boost::asio::ip::tcp::resolver::query query(server, "http");
		resolver_.async_resolve(query,
		                        boost::bind(&AsyncHttpGet::HandleResolve, this,
		                                    boost::asio::placeholders::error,
		                                    boost::asio::placeholders::iterator));
	}

	/**
	 * GetError : get error value
	 *
	 * @return
	 *   Bool error status
	 */
	bool GetError() {
		return Error;
	}

	/**
	 * GetHead : get result headers
	 *
	 * @return
	 *   String message headers
	 */
	std::string GetHead() {
		return fHeaders;
	}

	/**
	 * GetBody : get result body
	 *
	 * @return
	 *   String message body
	 */
	std::string GetBody() {
		return fResponse;
	}

	/**
	 * GetStatusCode : get HTTP status code
	 *
	 * @return
	 *   unsigned int http_status_code
	 */
	unsigned int GetStatusCode() {
		return http_status_code;
	}



private:
	/**
	 * HandleResolve: handle resolve
	 *
	 * @param err
	 *   error_code
	 *
	 * @param endpoint_iterator
	 *   tcp::resolver::iterator endpoint iterator
	 *
	 * @return
	 *   none
	 */
	void HandleResolve(const boost::system::error_code& err,
	                   boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
		if (!err) {
			// Attempt a connection to the first endpoint in the list. Each endpoint
			// will be tried until we successfully establish a connection.
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			socket_.async_connect(endpoint,
			                      boost::bind(&AsyncHttpGet::HandleConnect, this,
			                                  boost::asio::placeholders::error, ++endpoint_iterator));
		} else {
			Error=true;
		}
	}

	/**
	 * HandleConnect: handle the connection
	 *
	 * @param err
	 *   error_code
	 *
	 * @param endpoint_iterator
	 *   tcp::resolver::iterator endpoint iterator
	 *
	 * @return
	 *   none
	 */
	void HandleConnect(const boost::system::error_code& err,
	                   boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
		if (!err) {
			// The connection was successful. Send the request.
			boost::asio::async_write(socket_, request_,
			                         boost::bind(&AsyncHttpGet::HandleWrite, this,
			                                     boost::asio::placeholders::error));
		} else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()) {
			// The connection failed. Try the next endpoint in the list.
			socket_.close();
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			socket_.async_connect(endpoint,
			                      boost::bind(&AsyncHttpGet::HandleConnect, this,
			                                  boost::asio::placeholders::error, ++endpoint_iterator));
		} else {
			Error=true;
		}
	}


	/**
	 * HandleWrite: handle the write request
	 *
	 * @param err
	 *   error_code
	 *
	 * @return
	 *   none
	 */
	void HandleWrite(const boost::system::error_code& err) {
		if (!err) {
			// Read the response status line.
			boost::asio::async_read_until(socket_, response_, "\r\n",
			                              boost::bind(&AsyncHttpGet::HandleReadStatus, this,
			                                      boost::asio::placeholders::error));
		} else {
			Error=true;
		}
	}

	/**
	 * HandleReadStatus: handle the connection status line
	 *
	 * @param err
	 *   error_code
	 *
	 * @return
	 *   none
	 */
	void HandleReadStatus(const boost::system::error_code& err) {
		if (!err) {
			// Check that response is OK.
			std::istream response_stream(&response_);
			std::string hello;
			std::getline(response_stream, hello);
			boost::xpressive::sregex rex = boost::xpressive::sregex::compile(
			                                   "HTTP[\\/]([0-9\\.]+)[[:space:]]+(\\d+)[[:space:]]+(\\w+)\\W*"
			                               );
			boost::xpressive::smatch what;
			if( boost::xpressive::regex_match( hello, what, rex ) ) {

				http_status_code = apn::Convert::AnyToAny<std::string,unsigned int>(what[2],true);
				if (http_status_code != 200) Error=true;
			} else {
				Error=true;
			}
			// Read the response headers, which are terminated by a blank line.
			boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
			                              boost::bind(&AsyncHttpGet::HandleReadHeaders, this,
			                                      boost::asio::placeholders::error));
		} else {
			Error=true;
		}
	}

	/**
	 * HandleReadHeaders: handle the reading of restr of headers
	 *
	 * @param err
	 *   error_code
	 *
	 * @return
	 *   none
	 */
	void HandleReadHeaders(const boost::system::error_code& err) {
		if (!err) {
			// Process the response headers.
			std::istream response_stream(&response_);
			std::string header;
			while (std::getline(response_stream, header) && header != "\r")
				fHeaders += header + "\n";
			fHeaders += "\n";

			// Write whatever content we already have to output.
			if (response_.size() > 0) {
				// std::cout << &response_;
				const char* r=boost::asio::buffer_cast<const char*>(response_.data());
				fResponse+= std::string(r);
			}

			// Start reading remaining data until EOF.
			boost::asio::async_read(socket_, response_,
			                        boost::asio::transfer_at_least(1),
			                        boost::bind(&AsyncHttpGet::HandleReadContent, this,
			                                    boost::asio::placeholders::error));
		} else {
			Error=true;
		}
	}

	/**
	 * HandleReadContent: handle the reading of content
	 *
	 * @param err
	 *   error_code
	 *
	 * @return
	 *   none
	 */
	void HandleReadContent(const boost::system::error_code& err) {
		if (!err) {
			// Write all of the data that has been read so far.
			// std::cout << &response_;
			const char* r=boost::asio::buffer_cast<const char*>(response_.data());
			fResponse+= std::string(r);

			// Continue reading remaining data until EOF.
			boost::asio::async_read(socket_, response_,
			                        boost::asio::transfer_at_least(1),
			                        boost::bind(&AsyncHttpGet::HandleReadContent, this,
			                                    boost::asio::placeholders::error));
		} else if (err != boost::asio::error::eof) {
			Error=true;
		}
	}

	boost::asio::ip::tcp::resolver resolver_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;
	std::string fHeaders;
	std::string fResponse;
	unsigned int http_status_code;
	bool Error;
};

} // namespace apn
#endif /* _APN_ASYNC_HTTP_GET_HPP_ */
