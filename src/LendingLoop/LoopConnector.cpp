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

#include <LendingLoop/LoopConnector.h>
#include <Utility.h>
#include <exception>
#include <thread>
#include <algorithm>

//!!!!!!!!!
#include <iostream>

using namespace std;

void LoopConnector::processDashboard() {
	
	ResponseData data = getWebData("https://my.lendingloop.ca/investor-profile");
	string strToFind = "<span>Annual Investment Limit Remaining</span>";
	string limit = data.HTMLData.substr(data.HTMLData.find(strToFind) + strToFind.size());
	strToFind = "<span>";
	limit = limit.substr(limit.find(strToFind) + strToFind.size());
	limit = limit.substr(0, limit.find("</span>"));
	Utility::Replace(limit, "$", "");
	Utility::Replace(limit, ",", "");
	_dashboard.InvestmentLimit = stod(limit);

	string strValue;
	string partial = _dashboardDataRaw.substr(_dashboardDataRaw.find("<span class=\"btn btn-lg btn-icon btn-soft-primary rounded-circle mr-4\">"));
	strToFind = "<span class=\"d-block font-size-2\">";
	partial = partial.substr(partial.find(strToFind) + strToFind.size());

	strValue = partial.substr(0, partial.find("</span>"));
	Utility::Replace(strValue, "$", "");
	Utility::Replace(strValue, ",", "");
	
	try {
		_dashboard.LifetimeEarnings = stod(strValue);
	}
	catch (...) { }

	strToFind = "<span class=\"btn btn-lg btn-icon btn-soft-warning rounded-circle mr-4\">";
	partial = partial.substr(partial.find(strToFind) + strToFind.size());
	strToFind = "<span class=\"d-block font-size-2\">";
	partial = partial.substr(partial.find(strToFind) + strToFind.size());
	strValue = partial.substr(0, partial.find("</span>"));
	Utility::Replace(strValue, "$", "");
	Utility::Replace(strValue, ",", "");

	try {
		_dashboard.AvailableFunds = stod(strValue);
	}
	catch (...) {}

	partial = partial.substr(partial.find("<span class=\"tooltips color-purple\" data-toggle=\"tooltip\" data-original-title="
		 "\"The amount you have pledged to loans on the marketplace that have not yet closed\" data-placement=\"right\">Total Commitments</span>"));

	partial = partial.substr(partial.find("$"));
	strValue = partial.substr(0, partial.find("</td>"));
	Utility::Replace(strValue, "$", "");
	Utility::Replace(strValue, ",", "");

	try {
		_dashboard.TotalCommitments = stod(strValue);
	}
	catch (...) {}

	partial = partial.substr(partial.find("<td class=\"text-right\">") + 23);
	strValue = partial.substr(0, partial.find("</td>"));
	Utility::Replace(strValue, "$", "");
	Utility::Replace(strValue, ",", "");

	try {
		_dashboard.OutstandingPrincipal = stod(strValue);
	}
	catch (...) {}

	partial = partial.substr(partial.find("<td class=\"text-right\">") + 26);
	strValue = partial.substr(0, partial.find("</b>"));
	
	Utility::Replace(strValue, "$", "");
	Utility::Replace(strValue, ",", "");

	try {
		_dashboard.TotalAccountValue = stod(strValue);
	}
	catch (...) {}

}

void LoopConnector::Abort(){
	_dieThread = true;
	_cv.notify_all();
}

string LoopConnector::GetAllPayments() {
	_dieThread = false;
	if (!_loggedIn) {
		if (!LogIn())
			throw "Invalid Email and Password";
	}
	
	return getPayments();
}

string LoopConnector::getPayments() {
	ResponseData returnData = getWebData("https://my.lendingloop.ca/payment_schedule_polling");
	string uuid = returnData.HTMLData.substr(returnData.HTMLData.find("\"uuid\":\"") + 8);
	uuid = uuid.substr(0, uuid.find("\""));
	//!!!!!!
	//cout << "returnData = " << returnData.HTMLData << endl;
	//cout << "uuid = " << uuid << endl;

	while (!_dieThread) {
		//this_thread::sleep_for(chrono::seconds(5));
		unique_lock<mutex> lk(_m);
		_cv.wait_for(lk, chrono::seconds(5));
		if (_dieThread)
			return "";

		returnData = getWebData("https://my.lendingloop.ca/polling/" + uuid);
		if (returnData.StatusCode != WebConnector::HTTPStatusCode::NoContent)
			break;
	}

	if (_dieThread)
		return "";

	string paymentsData = "";

	for (int i = 0; i < 3; ++i) {
		try {
			ResponseData rd = getWebData("https://my.lendingloop.ca/payment_schedule/download/" + uuid + ".csv");
			if (rd.StatusCode == WebConnector::HTTPStatusCode::OK) {
				paymentsData = rd.HTMLData;
				break;
			}
		}
		catch (exception const& ex) {

		}
	}

	if (paymentsData.empty())
		throw "Several attempts were made to retrieve the data but all failed!";

	return paymentsData;
}

bool LoopConnector::LogIn() {
	_loggedIn = false;
	try {
		ResponseData data = getWebData("https://my.lendingloop.ca/users/sign_in");
		//!!!!!!!!!!!!!!!!!!!!!!!
		//cout << __PRETTY_FUNCTION__ << " response = " << data.HTMLData << endl;

		string sb = "utf8=%E2%9C%93&authenticity_token=";
		sb += WebConnector::EscapeDataString(getAuthenticityToken(data.HTMLData));
		sb += "&user%5Bemail%5D=";
		sb += WebConnector::EscapeDataString(_email);
		sb += "&user%5Bpassword%5D=";
		sb += WebConnector::EscapeDataString(_password);
		sb += "&user%5Bremember_me%5D=0&commit=Log+In";

		data = getWebData("https://my.lendingloop.ca/users/sign_in", sb);

		while (data.IsRedirect)
			data = getWebData(data.NewURL);

		if (data.HTMLData.find("<div id=\"flash_alert\">Invalid email or password.</div>") != string::npos)
			return _loggedIn;

		//we now have a logged in cookie! And the dashboard
		_dashboardDataRaw = data.HTMLData;
		processDashboard();
		_loggedIn = true;
	}
	catch(...) {
		//!!!!!!!!!!!!!!!!
		 cout << "Exception logging in!" << endl; 
	}

	return _loggedIn;
}

void LoopConnector::Refresh() {
	if (!_loggedIn) {
		LogIn();
		return;
	}

	ResponseData data = getWebData("https://my.lendingloop.ca/dashboard");
	while (data.StatusCode == WebConnector::HTTPStatusCode::Redirect)
		data = getWebData(data.NewURL);

	//we now have a logged in cookie! And the dashboard
	_dashboardDataRaw = data.HTMLData;
	processDashboard();
}

vector<PulledParts> LoopConnector::GetPullData(){
	if (!_loggedIn){
		if (!LogIn())
			throw "Invalid Email and Password";
	}

	return getPullData();
}

vector<PulledParts> LoopConnector::getPullData(){
	vector<PulledParts> list;
	try{
		
		ResponseData active = getWebData("https://my.lendingloop.ca/pull_active_loanparts");
        ResponseData repaid = getWebData("https://my.lendingloop.ca/pull_repaid_loanparts");
        ResponseData late = getWebData("https://my.lendingloop.ca/pull_late_loanparts");
		ResponseData defaultLoans = getWebData("https://my.lendingloop.ca/pull_default_loanparts");
        ResponseData chargedOff = getWebData("https://my.lendingloop.ca/pull_charged_off_loanparts");

		vector<PulledParts> l = pullApartPulledParts(active.HTMLData);
		list.insert(list.end(), l.begin(), l.end());

		l = pullApartPulledParts(repaid.HTMLData);
		list.insert(list.end(), l.begin(), l.end());

		l = pullApartPulledParts(late.HTMLData);
		list.insert(list.end(), l.begin(), l.end());

		l = pullApartPulledParts(defaultLoans.HTMLData);
		list.insert(list.end(), l.begin(), l.end());

		l = pullApartPulledParts(chargedOff.HTMLData, "loan_rails_id");
		list.insert(list.end(), l.begin(), l.end());
	}
	catch(...){
		throw "An exception happened while retrieving / parsing pull data!";
	}

	return list;
}

vector<PulledParts> LoopConnector::pullApartPulledParts(string const& pulledParts, string alternateLoanId) const{
	vector<PulledParts> list;
	string::size_type idx = 0;

	vector<string> allParts = Utility::Separate(pulledParts);

	while (idx < allParts.size()){
		while (idx < allParts.size() && allParts[idx].find(alternateLoanId) == string::npos)
			++idx;

		if (idx >= allParts.size())
			break;
		
		string str = allParts[idx].substr(allParts[idx].find("\"" + alternateLoanId + "\":"));
		str = str.substr(str.find(":\"") + 2);
		str = str.substr(0, str.find('"'));
		string loanId = str;

		if (++idx >= allParts.size())
			break;

		str = allParts[idx].substr(allParts[idx].find(':'));
		str = str.substr(str.find('"') + 1);
		str = str.substr(0, str.find('"'));
		string company = str;

		if (++idx >= allParts.size())
			break;
		
		str = allParts[idx].substr(allParts[idx].find(':'));
		str = str.substr(str.find('"') + 1);
		str = str.substr(0, str.find('"'));
		string loanName = str;

		list.push_back(PulledParts(stoul(loanId), company, loanName));
	}

	return list;
}

string LoopConnector::getAuthenticityToken(string const& data) {
	string form = data.substr(data.find("<form"));

	form = form.substr(form.find("authenticity_token\""));
	form = form.substr(form.find("value=") + 7);
	form = form.substr(0, form.find("\""));

	return form;
}

void log(string const& msg){
	cout << __PRETTY_FUNCTION__ << msg << endl;
}

ResponseData LoopConnector::getWebData(string const& url, string const& requestBody, bool useLastCookie) {
	ResponseData rd;

	WebConnector wc(log);
	wc.AddHeader("Accept-Encoding", "identity");

	if (useLastCookie && !_lastCookie.empty())
		wc.SetCookie(_lastCookie);

	
	rd.HTMLData = wc.performHTTPRequest(url, requestBody);
	
	map<string, string> const& m = wc.ResponseHeaders();
	if (useLastCookie){
		map<string, string>::const_iterator cit = m.find("Set-Cookie");
		if (cit == m.cend())
			cit = m.find("set-cookie");
		if (cit != m.cend())
			_lastCookie = cit->second;
		
	}

	map<string, string>::const_iterator cit = m.find("Location");
	if (cit == m.cend())
		cit = m.find("location");
	
	if (cit != m.cend()){
		rd.IsRedirect = true;
		rd.NewURL = cit->second;
	}
	
	rd.StatusCode = wc.RepsonseCode();
	return rd;
}