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
#include <vector>
#include <list>
#include <map>
#include <Displayables/LineItem.h>
#include <Displayables/DisplayData.h>
#include <LendingLoop/PulledParts.h>
#include <Displayables/DateTime.h>
#include <fstream>
#include <sstream>

class LenderManipulator{
    static std::string MONTHS[];
    std::vector<int> _distinctYears;
    std::vector<LineItem> _lines;
    std::vector<std::string> _paymentsHeaders;
    std::map <int, std::vector<DisplayData>> _yearlyCache;
    double _contractTotalInterest;
    double _contractTotalFees;
    double _contractTotalPrincipal;
    double _contractTotalTotalDue;
    double _contractTotalInterestPaid;
    double _contractTotalFeesPaid;
    double _contractTotalPrincipalPaid;
    double _contractTotalTotalDuePaid;
    int _yearCount;
    std::string _all_paymentsCsv;

    LenderManipulator() { }

public:
    ~LenderManipulator() { }

    std::vector<LineItem> const& Lines() const { return _lines; }
    double ContractTotalInterest() const { return _contractTotalInterest; }
    double ContractTotalFees() const { return _contractTotalFees; }
    double ContractTotalPrincipal() const { return _contractTotalPrincipal; }
    double ContractTotalTotalDue() const { return _contractTotalTotalDue; }
    double ContractTotalInterestPaid() const { return _contractTotalInterestPaid; }
    double ContractTotalFeesPaid() const { return _contractTotalFeesPaid; }
    double ContractTotalPrincipalPaid() const { return _contractTotalPrincipalPaid; }
    double ContractTotalTotalDuePaid() const { return _contractTotalTotalDuePaid; }
    int YearCount() const { return _yearCount; }

    double ContractTotal() const { return _contractTotalInterest - _contractTotalFees; }
    std::string const& All_paymentsCsv() const { return _all_paymentsCsv; }
    
    int GetYear(int idx) const { return _distinctYears[idx]; }
    std::vector<DisplayData> GetDataForYear(int year);
    std::vector<DisplayData> GetYearlySummary();
    std::vector<LineItem> FilterResults (std::vector<LineItem::PaymentStatus> const& statuses) const;
    std::vector<LineItem> FilterResults(LineItem::PaymentStatus status) const;
    std::vector<LineItem> FilterResults(std::vector<DateTime> const& dates) const;
    std::vector<uint32_t> GetDistinctLoanIDs() const;

    bool isEntityType() const;
    static bool isEntityType(std::string header);

    bool isCompanyType() const;
    static bool isCompanyType(std::string header);

private:
    static LenderManipulator* CreateManipulator(std::stringstream& reader, std::vector<PulledParts> const& pulledParts);
public:
    static LenderManipulator* CreateManipulator(std::string const& fileOrDoc, std::vector<PulledParts> const& parts);
};