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

#include <Displayables/DisplayLineItem.h>
#include <Displayables/LineItem.h>
#include <string>
#include <Utility.h>

using namespace std;
    
DisplayLineItem::DisplayLineItem(LineItem const& item) {
    _payType = LineItem::PaymentTypeToString(item.PayType());
    _entityId = to_string(item.EntityId());
    _loanId = to_string(item.LoanId());
    _company = item.Company();
    _loanName = item.LoanName();
    _interestRate = Utility::format(item.InterestRate(), false) + "%";
    _riskBand = item.RiskBand();
    _interest = Utility::format(item.Interest());
    _principal = Utility::format(item.Principal());
    _total = Utility::format(item.Total());
    _loopFee = Utility::format(item.LoopFee());
    _datePaid = "";
    _status = LineItem::PaymentStatusToString(item.Status());
    _lineItem = item;
    _isBlank = false;
    _dueDate = Utility::DateToString(item.DueDate());
    if (item.DatePaid().Year() != 0){
        _datePaid = Utility::DateToString(item.DatePaid());
    }
}

DisplayLineItem::DisplayLineItem(std::string payType, std::string loanId, std::string entityId, std::string company, std::string loanName, std::string interestRate, std::string riskBand, 
			std::string interest, std::string principal, std::string total, std::string loopFee, std::string dueDate, std::string datePaid, std::string status) :
             _payType(payType), _loanId(loanId), _entityId(entityId), _company(company), _loanName(loanName), _interestRate(interestRate), _riskBand(riskBand), 
             _interest(interest), _principal(principal), _total(total), _loopFee(loopFee), _dueDate(dueDate), _datePaid(datePaid), _status(status){
    _isBlank = _payType.empty() && _loanId.empty() && _entityId.empty() && _company.empty() &&
                _loanName.empty() && _interestRate.empty() && _riskBand.empty() && _interest.empty() &&
                _principal.empty() && _total.empty() && _loopFee.empty() && _dueDate.empty() && _datePaid.empty() && _status.empty();
}
    