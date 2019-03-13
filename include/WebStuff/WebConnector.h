#ifndef __WEB_CONNECTOR_HG_
#define __WEB_CONNECTOR_HG_
/***************************************************************************** 
*	Lending Loop Information - A program for manipulating and interpreting the payment schedule from Lending Loop
*   Copyright (C) 2019  Jon Fowler
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335  USA
*	
*	Authors: Jon Fowler <jfowler84@hotmail.com>
*
*********************************************************************************/

#include <curl/curl.h>
#include <string>
#include <sstream>
#include <mutex>
#include <map>

class WebConnector {
public:
	typedef void(*DataLogger)(std::string const& str);

	enum HTTPStatusCode {
		NoCode = 0,
		OK = 200,
		NoContent = 204,
		Redirect = 301,
		Unauthorized = 403,
		NotFound = 404
	};

private:
	std::stringstream result;
	static bool curlInitialized;
	static std::mutex _mutex;
	static int refCount;
	DataLogger dataLogger;
	HTTPStatusCode responseCode;
	std::map<std::string, std::string> _requestHeaders;
	std::map<std::string, std::string> _responseHeaders;
	std::string _cookie;
	std::string _requestBody;
	
	//make non copyable
	WebConnector(WebConnector const& wc) = delete;
	WebConnector& operator = (WebConnector const& rhs) = delete;
	
public:
	WebConnector(DataLogger logger = NULL);
	~WebConnector();
	std::string performHTTPRequest(std::string const& url, std::string const& requestBody = "");
	
	static size_t CurlCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
	static size_t CurlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);

	HTTPStatusCode RepsonseCode() { return responseCode; }
	static std::string EscapeDataString(std::string const& str);
	std::map<std::string, std::string> const& ResponseHeaders() const { return _responseHeaders; }

	void AddHeader(std::string const& header, std::string const& val) {
		_requestHeaders[header] = val;
	}
	void ClearRequestHeaders() { _requestHeaders.clear(); }
	void SetCookie(std::string const& cookie) { _cookie = cookie; }
	void ClearCookie() { _cookie = ""; }
};

#endif