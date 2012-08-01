/**
 * @project apophnia++
 * @file include/apn/WebObject.hpp
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
 * The WebObject class is used for exchanging Web Request and response
 *
 */

#ifndef _APN_WEBOBJECT_HPP_
#define _APN_WEBOBJECT_HPP_
#define APN_WEBOBJECT_HPP_PROGNO 1006

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/value_init.hpp>
#include <boost/asio.hpp>
#include "Exception.hh"
#include "ConvertStr.hpp"

#define APN_WEBOBJ_STRN_AMPERSAND "&"
#define APN_WEBOBJ_STRN_EQUALTO "="
#define APN_WEBOBJ_STRN_SPACE " "
#define APN_WEBOBJ_STRN_COLON ":"
#define APN_WEBOBJ_STRN_BLANK ""
#define APN_WEBOBJ_STRN_PLUS "+"
#define APN_WEBOBJ_STRN_SLASH "/"
#define APN_WEBOBJ_STRN_QMARK "?"
#define APN_WEBOBJ_STRN_COLONSPACE ": "

#define APN_WEBOBJ_CHAR_PERCENT '%'
#define APN_WEBOBJ_CHAR_PLUS '+'
#define APN_WEBOBJ_CHAR_SPACE ' '

#define APN_WEBOBJ_CONTENT_LENGTH_STR "Content-Length"
#define APN_WEBOBJ_CONTENT_TYPE_STR "Content-Type"
#define APN_WEBOBJ_CTRLF "\r\n"
#define APN_WEBOBJ_CTRLFTWO "\r\n\r\n"


namespace apn {
/**
 * @brief: WebObject: Class for storing Web Objects
 */
class WebObject : private boost::noncopyable, public boost::enable_shared_from_this<WebObject> {
public:
	typedef boost::shared_ptr<apn::WebObject> pointer;
	typedef std::vector<boost::asio::const_buffer> BufferType;

	/**
	 * create : static construction creates new first time
	 *
	 * @param hdrs
	 *   String Headers value
	 *
	 * @return
	 *   none
	 */
	static pointer create(std::string hdrs) {
		return pointer(new WebObject(hdrs));
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
	virtual ~WebObject() {}

	/**
	 * GetStatus: get status
	 *
	 * @return
	 *   Bool Status
	 */
	bool GetStatus() {
		return Status;
	}

	/**
	 * GetReply: make the reply
	 *
	 * @return
	 *   Type buffer vector
	 */
	BufferType GetReply() {
		fResponse.insert(fResponse.end(),'\0');
		fReply="HTTP/1.0 200 OK" APN_WEBOBJ_CTRLF;
		addHeaders(APN_WEBOBJ_CONTENT_LENGTH_STR,boost::lexical_cast<std::string>(fResponse.size()),respHeaders);
		addHeaders(APN_WEBOBJ_CONTENT_TYPE_STR,fContentType,respHeaders);
		fReply+=getHeaders(respHeaders);
		fReply+=APN_WEBOBJ_CTRLF ;
		BufferType t;
		t.push_back(boost::asio::buffer(fReply));
		t.push_back(boost::asio::buffer(fResponse));
		return t;
	}

	/**
	 * GetOrigURL: function to get URL
	 *
	 * @return
	 *   String URL value
	 */
	std::string GetOrigURL() {
		return fOrigURL ;
	}

	/**
	 * GetHeaders: function to get URL
	 *
	 * @return
	 *   String Headers value
	 */
	std::string GetHeaders() {
		return fHeaders ;
	}

	/**
	 * GetMethod: function to get URL Method
	 *
	 * @return
	 *   String Method value
	 */
	std::string GetMethod() {
		return fMethod ;
	}

	/**
	 * GetReqVersion: function to get URL ReqVersion
	 *
	 * @return
	 *   String ReqVersion value
	 */
	std::string GetReqVersion() {
		return fReqVersion ;
	}

	/**
	 * AddResponse: function to set the Content incrementally
	 *
	 * @param buffer
	 *   char * buffer
	 *
	 * @param bufsize
	 *   char * buffer size
	 *
	 * @return
	 *   none
	 */
	void AddResponse(const char* buffer, std::size_t sz) {
		cVec r (&buffer[0],&buffer[sz]);
		fResponse.insert(fResponse.end(),r.begin(),r.end());
	}

	/**
	 * SetContentType: function to get URL
	 *
	 * @param val
	 *   String fContentType value
	 *
	 * @return
	 *   none
	 */
	void SetContentType(std::string val) {
		fContentType = val ;
	}
	/**
	 * GetContentType: function to get URL ContentType
	 *
	 * @return
	 *   String ContentType value
	 */
	std::string GetContentType() {
		return fContentType ;
	}

	/**
	 * GetURLPartCount: function to get no of URL parts
	 *
	 * @return
	 *   size_t value
	 */
	std::size_t GetURLPartCount() {
		return reqPath.size() ;
	}

	/**
	 * GetURLPart: function to get URL parts
	 *
	 * @param partno
	 *   size_t value of that part
	 *
	 * @return
	 *   String value
	 */
	std::string GetURLPart(std::size_t partno) {
		return ( partno<reqPath.size() ) ? reqPath.at(partno) : std::string(APN_WEBOBJ_STRN_BLANK);
	}

	/**
	 * GetReqParam: function to get Request Params
	 *
	 * @param p
	 *   String param
	 *
	 * @return
	 *   Bool Status
	 *   Type value
	 */
	template<typename T>
	std::pair<bool,T> GetReqParam(std::string p) {
		return getElem<T>(p, reqParams);
	}

	/**
	 * GetReqHeader: function to get Request Header
	 *
	 * @param p
	 *   String param
	 *
	 * @return
	 *   Bool Status
	 *   Type value
	 */
	template<typename T>
	std::pair<bool,T> GetReqHeader(std::string p) {
		return getElem<T>(p, reqHeaders);
	}

private:
	std::string fOrigURL;
	std::string fNewURL;
	std::string fHeaders;
	std::string fMethod;
	std::string fReqVersion;
	std::string fContentType;

	typedef std::vector<std::string> sVec;
	sVec reqPath;
	typedef std::map<std::string,std::string> ssMap;
	ssMap reqHeaders, respHeaders;
	ssMap reqParams;
	bool Status;

	typedef std::vector<char> cVec;
	cVec fResponse;
	std::string fReply;

	/**
	 * Constructor: function to initialize from headers
	 *
	 * @param fH
	 *   String Headers provided
	 *
	 * @return
	 *   none
	 */
	WebObject(std::string fH) : Status(true) {
		SetContentType("text/plain");
		try {
			fHeaders = fH;
			std::string::size_type idx=fHeaders.find(APN_WEBOBJ_CTRLF);
			std::string reqString=fHeaders.substr(0,idx);
			fHeaders.erase(0,idx+2);

			idx=reqString.find(APN_WEBOBJ_STRN_SPACE);
			if (idx == std::string::npos) {
				throw apn::GenericException(APN_WEBOBJECT_HPP_PROGNO,"conversion error","");
			}

			fMethod=reqString.substr(0,idx);
			reqString=reqString.substr(idx+1);
			idx=reqString.find(APN_WEBOBJ_STRN_SPACE);
			if (idx == std::string::npos) {
				throw apn::GenericException(APN_WEBOBJECT_HPP_PROGNO,"conversion error","");
			}
			fOrigURL=reqString.substr(0,idx);
			fReqVersion=reqString.substr(idx+1);
			idx=fReqVersion.find(APN_WEBOBJ_STRN_SLASH);
			if (idx == std::string::npos) {
				throw apn::GenericException(APN_WEBOBJECT_HPP_PROGNO,"conversion error","");
			}
			fReqVersion=fReqVersion.substr(idx+1);

			// rest of headers, etc
			parseHeaders(fHeaders,reqHeaders);
			if ( ! UrlDecode(fOrigURL, fNewURL) ) {
				throw apn::GenericException(APN_WEBOBJECT_HPP_PROGNO,"conversion error","");
			}
			sVec tp = Convert::StringToList<sVec>(fNewURL,APN_WEBOBJ_STRN_QMARK);
			if (tp.size()==0) {
				throw apn::GenericException(APN_WEBOBJECT_HPP_PROGNO,"conversion error","");
			} else
				reqPath = Convert::StringToList<sVec>(tp[0],APN_WEBOBJ_STRN_SLASH);
			if (tp.size()>1)
				reqParams = apn::Convert::StringToMap<ssMap>(tp[1], APN_WEBOBJ_STRN_AMPERSAND,APN_WEBOBJ_STRN_EQUALTO);
		} catch (apn::GenericException e) {
			Status=false;
		} catch (...) {
			Status=false;
		}
	}
	/**
	 * Constructor : the unused constructor
	 */
	WebObject() {}
	/**
	 * parseHeaders: function to parse HTTP headers
	 *
	 * @param h
	 *   String header string
	 * @param hm
	 *   ssMap header map by ref
	 *
	 * @return
	 *   none
	 */
	void parseHeaders(const std::string& h, ssMap& hm) {
		std::string str(h);
		std::string::size_type idx;
		std::string t;
		while ((idx=str.find(APN_WEBOBJ_CTRLF)) != std::string::npos) {
			t=str.substr(0,idx);
			str.erase(0,idx+2);
			if (t == APN_WEBOBJ_STRN_BLANK)
				break;
			idx=t.find(APN_WEBOBJ_STRN_COLONSPACE);
			if (idx == std::string::npos) {
				break;
			}
			hm.insert(ssMap::value_type(t.substr(0,idx),t.substr(idx+2)));
		}
	}
	/**
	 * addHeaders: function to add to HTTP headers
	 *
	 * @param h
	 *   String header name
	 * @param v
	 *   String header value
	 * @param hm
	 *   ssMap header map by ref
	 *
	 * @return
	 *   none
	 */
	void addHeaders(const std::string& h, const std::string& v, ssMap& hm) {
		hm[h]=v;
	}
	/**
	 * getHeaders: function to get the HTTP headers from a map
	 *
	 * @param hm
	 *   ssMap header map by ref
	 *
	 * @return
	 *   String
	 */
	std::string getHeaders(ssMap& hm) {
		std::string s;
		for (ssMap::const_iterator it = hm.begin(); it!=hm.end(); ++it) {
			s+= it->first + APN_WEBOBJ_STRN_COLONSPACE + it->second + APN_WEBOBJ_CTRLF;
		}
		return s;
	}

	/**
	 * getElem: function to get the Elems from a map
	 *
	 * @param needle
	 *   String item to find
	 *
	 * @param hm
	 *   ssMap header map by ref
	 *
	 * @return
	 *   Bool Status
	 *   Type value
	 */
	template<typename T>
	std::pair<bool,T> getElem(std::string needle, ssMap& hm) {
		bool stat=false;
		T item=T();
		try {
			ssMap::const_iterator it = hm.find(needle);
			if (it!=hm.end()) {
				item=Convert::AnyToAny<std::string,T>(it->second);
				stat=true;
			}
		} catch (...) {}
		return std::make_pair<bool,T>(stat,item);
	}


	/**
	 * UrlDecode: Get url-decoded string
	 *
	 * @param inS
	 *   String Input String
	 *
	 * @param outS
	 *   String Output String
	 *
	 * @return
	 *   Bool status
	 */
	bool UrlDecode(const std::string& inS, std::string& outS) {
		outS.clear();
		std::size_t insize = inS.size();
		outS.reserve(insize);
		try {
			for (std::size_t i = 0; i < insize; ++i) {
				switch (inS[i]) {
				case APN_WEBOBJ_CHAR_PERCENT:
					if (i + 2 >= insize) return false;
					else {
						int value = 0;
						std::istringstream is(inS.substr(i + 1, 2));
						if (is >> std::hex >> value) {
							outS += static_cast<char>(value);
							i += 2;
						} else {
							return false;
						}
					}
					break;
				case APN_WEBOBJ_CHAR_PLUS:
					outS += APN_WEBOBJ_CHAR_SPACE;
					break;
				default:
					outS += inS[i];
					break;
				}
			}
		} catch (...) {
			return false;
		}
		return true;
	}

};
} // namespace apn
#endif
