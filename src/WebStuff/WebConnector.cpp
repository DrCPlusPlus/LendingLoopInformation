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

#include <WebStuff/WebConnector.h>
#include <Utility.h>
#include <iostream>

using namespace std;

bool WebConnector::curlInitialized = false;
mutex WebConnector::_mutex;
int WebConnector::refCount = 0;

size_t WebConnector::CurlCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	WebConnector* wc = (WebConnector*)userdata;
	for (size_t i = 0; i < nmemb; ++i)
		wc->result << ptr[i];

	return nmemb;
}

size_t WebConnector::CurlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
	WebConnector* wc = (WebConnector*)userdata;
	string data(buffer, size * nitems);

	string::size_type idx = data.find(":");
	if (idx != string::npos) {
		string& value = wc->_responseHeaders[data.substr(0, idx)];
		value = "";
		if (idx != data.size() - 1) {
			value = data.substr(idx + 1);
			if (value[0] == ' ')
				value.erase(0, 1);
			Utility::Replace(value, "\r\n", "");
		}
	}
	return size * nitems;
}

string WebConnector::performHTTPRequest(string const& url, string const& requestBody){
	CURL* _curl = curl_easy_init();
	
	if (_curl) {
		this->result.str("");
		curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WebConnector::CurlCallback);
		curl_easy_setopt(_curl, CURLOPT_HEADERDATA, this);
		curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, WebConnector::CurlHeaderCallback);
		//!!!!!!!!!!1
		//curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);

		_responseHeaders.clear();

		struct curl_slist* list = NULL;

		if (_requestHeaders.size() > 0) {
			for (map<string, string>::iterator it = _requestHeaders.begin(); it != _requestHeaders.end(); ++it)
				list = curl_slist_append(list, string(it->first + ": " + it->second).c_str());
			curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, list);
		}

		if (!_cookie.empty())
			curl_easy_setopt(_curl, CURLOPT_COOKIE, _cookie.c_str());

		if (!requestBody.empty()){
			curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, requestBody.size());
			curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, requestBody.data());
		}

		CURLcode res = curl_easy_perform(_curl);
		if (res != CURLE_OK && dataLogger) {
			string msg = "curl_easy_perform() failed: " + url + " ";
			msg += curl_easy_strerror(res);
			dataLogger(msg);
		}
		else if (res == CURLE_OK) {
			long response_code;
			curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &response_code);
			responseCode = (HTTPStatusCode)response_code;
		}
		if (list)
			curl_slist_free_all(list);
		curl_easy_cleanup(_curl);
	}

	return this->result.str();
}

WebConnector::WebConnector(DataLogger logger) : dataLogger(logger), responseCode(HTTPStatusCode::NoCode), _requestHeaders(), _responseHeaders() {
	std::lock_guard<std::mutex> lk(_mutex);
	if (!curlInitialized) {
		curl_global_init(CURL_GLOBAL_DEFAULT);
		curlInitialized = true;
	}
	++refCount;
}

WebConnector::~WebConnector() {
	std::lock_guard<std::mutex> lk(_mutex);
	--refCount;

	if (refCount == 0) {
		curl_global_cleanup();
	}
}

string WebConnector::EscapeDataString(std::string const& str) {
	CURL* curl = curl_easy_init();
	string escaped = "";

	if (curl) {
		char* output = curl_easy_escape(curl, str.c_str(), str.size());
		if (output) {
			escaped = string(output);
			curl_free(output);
		}
	}
	return escaped;
}
