#pragma once
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

#include <string>
#include <Displayables/DashboardData.h>
#include <WebStuff/ResponseData.h>
#include <mutex>
#include <condition_variable>
#include <LendingLoop/PulledParts.h>
#include <map>

class LoopConnector {
	std::string _email;
	std::string _password;
	std::string _lastCookie;
	std::string _dashboardDataRaw;

	bool _dieThread;
	std::mutex _m;
	std::condition_variable _cv;


	DashboardData _dashboard;
	bool _loggedIn;


	void processDashboard();
	std::string getPayments();
	std::vector<PulledParts> getPullData();
	std::vector<PulledParts> pullApartPulledParts(std::string const& pulledParts, std::string alternateLoanId = "loan_id") const;

public:
	
	LoopConnector(std::string const& email, std::string const& password) : _email(email), _password(password), _lastCookie(), 
				_dashboardDataRaw(), _dashboard(), _loggedIn(false), _dieThread(false), _cv(), _m() {

	}
	~LoopConnector() {

	}

	std::string getEmail() { return _email; }
	std::string getPassword() { return _password; }
	bool LoggedIn() { return _loggedIn; }
	DashboardData Dashboard() { return _dashboard; }
	
	// must be called from a separate thread
	std::string GetAllPayments();
	void Abort();

	std::vector<PulledParts> GetPullData();

	bool LogIn();
	void Refresh();
	
	// Gets a valid idToken and constructs the url to redirect to
	std::string GetAuthenitcatedURL();

	ResponseData getWebData(std::string const& url, std::map<std::string, std::string> const& headers, std::string const& requestBody = "", bool useLastCookie = true);

};