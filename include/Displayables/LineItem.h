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

#include <stdint.h>
#include <string>
#include <map>
#include <Displayables/DateTime.h>

class LineItem{
public:
    enum PaymentType{
        isScheduled
    };
    
    enum PaymentStatus{
        Paid,
        Unpaid,
        Scheduled
    };

private:
    PaymentType _payType;
    uint32_t _loanId;
    uint32_t _entityId;
    std::string _company;
    std::string _loanName;

    double _interestRate;
    std::string _riskBand;
    double _interest;
    double _principal;
    double _total;
    double _loopFee;
    DateTime _dueDate;
    DateTime _datePaid;
    bool _hasValidDatePaid;
    PaymentStatus _status;
    static std::map<std::string, PaymentStatus> _statusMap;
    static std::map<PaymentStatus, std::string> _statusMapToString;

    static std::map<PaymentType, std::string> _payTypeMapToString;
public:
    PaymentType PayType() const { return _payType; }
    uint32_t LoanId() const { return _loanId; }
    uint32_t EntityId() const { return _entityId; }
    std::string Company() const { return _company; }
    std::string LoanName() const { return _loanName; }
    double InterestRate() const { return _interestRate; }
    std::string RiskBand() const { return _riskBand; }
    double Interest() const { return _interest; }
    double Principal() const { return _principal; }
    double Total() const { return _total; }
    double LoopFee() const { return _loopFee; }
    DateTime const& DueDate() const { return _dueDate; }
    DateTime const& DatePaid() const { return _datePaid; }
    bool HasValidDatePaid() const { return _hasValidDatePaid; }
    PaymentStatus Status() const { return _status; }

    LineItem(uint32_t loanId, uint32_t entityId, std::string const& company, std::string const& loanName, 
            double interestRate, std::string const& riskBand, double interest, 
            double principal, double total, double fee, std::string const& dueDate, 
            std::string const& payDate, std::string const& status);

    LineItem(): _payType(PaymentType::isScheduled), _loanId(0), _entityId(0), _company(), _loanName(), _interestRate(0.0), _riskBand(), 
            _interest(0.0), _principal(0.0), _total(0.0), _loopFee(0.0), _dueDate(), 
            _datePaid(), _hasValidDatePaid(false){

    }
    
    static std::string PaymentStatusToString(PaymentStatus status){
        return _statusMapToString[status];
    }

    static std::string PaymentTypeToString(PaymentType type){
        return _payTypeMapToString[type];
    }
};