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

#include <LendingLoop/LenderManipulator.h>
#include <Utility.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

std::string LenderManipulator::MONTHS[] = { "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };



vector<DisplayData> LenderManipulator::GetDataForYear(int year) {
    if (_yearlyCache.find(year) != _yearlyCache.end())
        return _yearlyCache[year];

    vector<DisplayData> data;
    vector<LineItem const*> itemsForYear = Utility::WherePtr(_lines, [year](LineItem const& li)->bool
            { 
                //return LineItem::Year(li.DueDate()) == year;
                return li.DueDate().Year() == year;
            });
    vector<LineItem const*> itemsForYearPaid = Utility::WherePtr(_lines, [year](LineItem const& li)->bool
            { 
                //return LineItem::Year(li.DatePaid()) == year && li.Status() == LineItem::PaymentStatus::Paid;
                return li.DatePaid().Year() == year && li.Status() == LineItem::PaymentStatus::Paid;
            });

    double interestForYear = 0.0;
    double feesForYear = 0.0;
    double principalForYear = 0.0;
    double totalDueForYear = 0.0;

    double interestForYearPaid = 0.0;
    double feesForYearPaid = 0.0;
    double principalForYearPaid = 0.0;
    double totalDueForYearPaid = 0.0;

    for (int x = 1; x <= 12; ++x){

        vector<LineItem const*> items = Utility::Where(itemsForYear, [x](LineItem const* li)->bool
            { 
                return li->DueDate().Month() == x;
            });
        
        vector<LineItem const*> itemsPaid = Utility::Where(itemsForYearPaid, [x](LineItem const* li)->bool
            { 
                return li->DatePaid().Month() == x;
            });

        double fees = 0.0;
        double interest = 0.0;
        double principal = 0.0;
        double totalDue = 0.0;

        double feesPaid = 0.0;
        double interestPaid = 0.0;
        double principalPaid = 0.0;
        double totalDuePaid = 0.0;

        for(auto const& j : items){
            fees += j->LoopFee();
            interest += j->Interest();
            principal += j->Principal();
            totalDue += j->Total();
        }

        for(auto const& j : itemsPaid){
            feesPaid += j->LoopFee();
            interestPaid += j->Interest();
            principalPaid += j->Principal();
            totalDuePaid += j->Total();
        }

        interestForYear += interest;
        feesForYear += fees;
        principalForYear += principal;
        totalDueForYear += totalDue;

        interestForYearPaid += interestPaid;
        feesForYearPaid += feesPaid;
        principalForYearPaid += principalPaid;
        totalDueForYearPaid += totalDuePaid;

        data.push_back(DisplayData(MONTHS[x], interest, fees, principal, totalDue, interestPaid, feesPaid, principalPaid, totalDuePaid));
    }

    data.push_back(DisplayData("", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true));
    data.push_back(DisplayData("Total:", interestForYear, feesForYear, principalForYear, totalDueForYear, interestForYearPaid, feesForYearPaid, principalForYearPaid, totalDueForYearPaid));
    _yearlyCache[year] = data;
    return data;
}
vector<DisplayData> LenderManipulator::GetYearlySummary() {
    vector<DisplayData> data;

    for (int x = 0; x < _yearCount; ++x){
        DisplayData& yearData = GetDataForYear(GetYear(x)).back();
        data.push_back(DisplayData(to_string(GetYear(x)), yearData.GetInterestScheduled(),
                yearData.GetFeesScheduled(), yearData.GetPrincipalScheduled(), 
                yearData.GetTotalDueScheduled(), yearData.GetInterestActual(),
                yearData.GetFeeActual(), yearData.GetPrincipalActual(),
                yearData.GetTotalDueActual()));
    }

    data.push_back(DisplayData("", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, true));
    data.push_back(DisplayData("Total:", ContractTotalInterest(), ContractTotalFees(),
            ContractTotalPrincipal(), ContractTotalTotalDue(), ContractTotalInterestPaid(), 
            ContractTotalFeesPaid(), ContractTotalPrincipalPaid(), ContractTotalTotalDuePaid()));

    return data;
}
vector<LineItem> LenderManipulator::FilterResults (vector<LineItem::PaymentStatus> const& statuses) const{
    return Utility::Where(_lines, [&statuses](LineItem const& li)->bool{ return find(statuses.cbegin(), statuses.cend(), li.Status()) != statuses.cend();});
}
vector<LineItem> LenderManipulator::FilterResults(LineItem::PaymentStatus status) const{
    return Utility::Where(_lines, [status](LineItem const& li)->bool{ return li.Status() ==  status;});
}

vector<LineItem> LenderManipulator::FilterResults(vector<DateTime> const& dates) const{
    return Utility::Where(_lines, [&dates](LineItem const& li)->bool{ return find(dates.cbegin(), dates.cend(), li.DatePaid()) != dates.cend(); });
}

vector<uint32_t> LenderManipulator::GetDistinctLoanIDs() const{
    set<uint32_t> uniqueIds;
    for (auto const& x : _lines)
        uniqueIds.insert(x.LoanId());
    
    vector<uint32_t> ids(uniqueIds.size());
    int idx = 0;
    for (auto const& x : uniqueIds)
        ids[idx++] = x;
        
    return ids;
}
    
bool LenderManipulator::isEntityType() const {
    return Utility::Where(_paymentsHeaders, [](string const& s)->bool{ 
                return s.find("Entity Id") != string::npos;}).size() > 0;
}

bool LenderManipulator::isEntityType(string header){
    return header.find("Entity Id") != string::npos;
}

bool LenderManipulator::isCompanyType() const {
    return Utility::Where(_paymentsHeaders, [](string const& s)->bool{ 
                return s.find("Company") != string::npos;}).size() > 0;;
}

bool LenderManipulator::isCompanyType(string header){
    return header.find("Company") != string::npos;
}

LenderManipulator* LenderManipulator::CreateManipulator(stringstream& reader, vector<PulledParts> const& pulledParts){
    LenderManipulator* manipulator = new LenderManipulator();
    string sb;
    bool transformPaymentFormat = pulledParts.size() > 0;
    
    string line;
    getline(reader, line);      //consume the header

    bool isEntity = isEntityType(line);
    bool isCompany = isCompanyType(line);
    if (isEntity && transformPaymentFormat)
        Utility::Replace(line, "Entity Id", "Company,Loan Name");

    sb += line;
    sb += "\n";

    try{
        manipulator->_paymentsHeaders = Utility::Separate(line);
        LineItem item;
        while (getline(reader, line)){
            if (!line.empty()){
                vector<string> parts = Utility::Separate(line);
                if (isCompany){
                    item = LineItem(stoul(parts[1]), 0, parts[2], parts[3], stod(parts[4]), 
                        parts[5], stod(parts[6]), stod(parts[7]), stod(parts[8]), 
                        stod(parts[15]), parts[16], parts[17], Utility::Replace(parts[18], "\r", ""));
                }
                else if (isEntity){
                    string company = "";
                    string loanName = "";
                    if (transformPaymentFormat){
                        uint32_t lId = stoul(parts[1]);
                        vector<PulledParts> pps = Utility::Where(pulledParts, [lId](PulledParts const& pp)->bool{ return pp.LoanId() == lId; });
                        if (pps.size() > 0){
                            PulledParts& pp = pps[0];
                            company = pp.Company();
                            loanName = pp.LoanName();
                            line = parts[0];
                            line += ',';
                            line += parts[1];
                            line += ',';
                            if (company.find(',') != string::npos)
                                line += "\"" + company + "\"";
                            else
                                line += company;
                            line += ',';
                            if (loanName.find(',') != string::npos)
                                line += "\"" + loanName + "\"";
                            else
                                line += loanName;
                            for (vector<string>::size_type idx = 3; idx < parts.size(); ++idx)
                                line += "," + parts[idx];
                        }
                    }
                    item = LineItem(stoul(parts[1]), stoul(parts[2]), company, loanName, stod(parts[3]), 
                        parts[4], stod(parts[5]), stod(parts[6]), stod(parts[7]), 
                        stod(parts[14]), parts[15], parts[16], Utility::Replace(parts[17], "\r", ""));
                }
                else{
                    // nothing to do in this case for now
                }

                manipulator->_lines.push_back(item);
            }
            sb += line;
            sb += "\n";
        }
    }
    catch (...){
        string err = "Error parsing data - ";
        err += line; 
        throw err.c_str();
    }

    manipulator->_all_paymentsCsv = sb;

    manipulator->_contractTotalFees = 0.0;
    manipulator->_contractTotalPrincipal = 0.0;
    manipulator->_contractTotalTotalDue = 0.0;
    manipulator->_contractTotalInterest = 0.0;

    manipulator->_contractTotalFeesPaid = 0.0;
    manipulator->_contractTotalInterestPaid = 0.0;
    manipulator->_contractTotalPrincipalPaid = 0.0;
    manipulator->_contractTotalTotalDuePaid = 0.0;

    for (auto const& item : manipulator->_lines){
        manipulator->_contractTotalFees += item.LoopFee();
        manipulator->_contractTotalInterest += item.Interest();
        manipulator->_contractTotalPrincipal += item.Principal();
        manipulator->_contractTotalTotalDue += item.Total();

         if (item.HasValidDatePaid() /*&& item.DatePaid() < ::time(NULL)*/ && item.Status() == LineItem::PaymentStatus::Paid){
            manipulator->_contractTotalFeesPaid += item.LoopFee();
            manipulator->_contractTotalInterestPaid += item.Interest();
            manipulator->_contractTotalPrincipalPaid += item.Principal();
            manipulator->_contractTotalTotalDuePaid += item.Total();
         }
    }

    set<int> years;
    for (auto const& x : manipulator->_lines){
        years.insert(x.DueDate().Year());
    }
    manipulator->_yearCount = years.size();
    manipulator->_distinctYears = vector<int>(years.begin(), years.end());
    
    return manipulator;

}

LenderManipulator* LenderManipulator::CreateManipulator(string const& fileOrDoc, vector<PulledParts> const& parts){
    stringstream ss;

    struct stat statBuf;
    if (!::stat(fileOrDoc.c_str(), &statBuf)){
        ifstream ifs(fileOrDoc);
        if (ifs.is_open()){
            ss << ifs.rdbuf();
            ifs.close();
        }
    }
    else
        ss << fileOrDoc;
    
    return CreateManipulator(ss, parts);
}
