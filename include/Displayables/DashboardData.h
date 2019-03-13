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
#include <Utility.h>

struct DashboardData {
	double LifetimeEarnings;
	double AvailableFunds;
	double TotalCommitments;
	double OutstandingPrincipal;
	double TotalAccountValue;
	double InvestmentLimit;

	std::string strLifetimeEarnings() {
		return Utility::format(LifetimeEarnings);
	}

	std::string strAvailableFunds() {
		return Utility::format(AvailableFunds);
	}

	std::string strTotalCommitments() {
		return Utility::format(TotalCommitments);
	}

	std::string strOutstandingPrincipal() {
		return Utility::format(OutstandingPrincipal);
	}

	std::string strTotalAccountValue() {
		return Utility::format(TotalAccountValue);
	}

	std::string strInvestmentLimit() {
		return Utility::format(InvestmentLimit);
	}

	std::string strRemainingPrincipal() {
		double investableAmount = AvailableFunds;
		if (InvestmentLimit < AvailableFunds)
			investableAmount = InvestmentLimit;
		return Utility::format(investableAmount < 0 ? 0 : investableAmount);
	}

	DashboardData() : LifetimeEarnings(0.0), AvailableFunds(0.0), TotalCommitments(0.0), OutstandingPrincipal(0.0), TotalAccountValue(0.0), InvestmentLimit(0.0) {

	}
};