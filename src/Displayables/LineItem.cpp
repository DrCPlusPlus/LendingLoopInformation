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

#include <Displayables/LineItem.h>
#include <algorithm>
#include <Utility.h>
#include <ctime>

using namespace std;

map<std::string, LineItem::PaymentStatus> LineItem::_statusMap = {
    { "paid", LineItem::PaymentStatus::Paid },
    { "Paid", LineItem::PaymentStatus::Paid }, 
    { "PAID", LineItem::PaymentStatus::Paid }, 
    { "unpaid", LineItem::PaymentStatus::Unpaid },
    { "Unpaid", LineItem::PaymentStatus::Unpaid },
    { "UNPAID", LineItem::PaymentStatus::Unpaid },
    { "scheduled", LineItem::PaymentStatus::Scheduled },
    { "Scheduled", LineItem::PaymentStatus::Scheduled },
    { "SCHEDULED", LineItem::PaymentStatus::Scheduled }
};

map<LineItem::PaymentStatus, string> LineItem::_statusMapToString = {
    { LineItem::PaymentStatus::Paid, "Paid" },
    { LineItem::PaymentStatus::Unpaid, "Unpaid" },
    { LineItem::PaymentStatus::Scheduled, "Scheduled" }

};

map<LineItem::PaymentType, string> LineItem::_payTypeMapToString = {
    { LineItem::PaymentType::isScheduled, "Scheduled" }
};

LineItem::LineItem(uint32_t loanId, uint32_t entityId, std::string const& company, std::string const& loanName, 
            double interestRate, std::string const& riskBand, double interest, 
            double principal, double total, double fee, std::string const& dueDate, 
            std::string const& payDate, std::string const& status): 
            
            _payType(PaymentType::isScheduled), _loanId(loanId), _entityId(entityId), _company(company), 
            _loanName(loanName), _interestRate(interestRate), _riskBand(riskBand), 
            _interest(interest), _principal(principal), _loopFee(fee), _total(total), _dueDate(), _datePaid(), _hasValidDatePaid(false){


    _status = LineItem::PaymentStatus::Scheduled;
    map<string, LineItem::PaymentStatus>::const_iterator cit = _statusMap.find(status);
    if (cit != _statusMap.cend())
        _status = _statusMap[status];

    vector<string> date = Utility::Split(dueDate, '-');
    _dueDate = DateTime(stoi(date[2]), stoi(date[1]), stoi(date[0]));
   /* struct tm dateDue = { 0 };
    dateDue.tm_year = stoi(date[0]) - 1900;
    dateDue.tm_mon = stoi(date[1]) - 1;
    dateDue.tm_mday = stoi(date[2]);
    _dueDate = mktime(&dateDue);*/

    if (!payDate.empty()){
        date = Utility::Split(payDate, '-');
        _datePaid = DateTime(stoi(date[2]), stoi(date[1]), stoi(date[0]));
        /*struct tm datePaid = { 0 };
        datePaid.tm_year = stoi(date[0]) - 1900;
        datePaid.tm_mon = stoi(date[1]) - 1;
        datePaid.tm_mday = stoi(date[2]);
        _datePaid = mktime(&datePaid);*/
        _hasValidDatePaid = true;
    }

}