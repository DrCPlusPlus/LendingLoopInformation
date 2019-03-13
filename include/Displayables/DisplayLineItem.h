#pragma once
#include <string>
#include <Displayables/LineItem.h>

class DisplayLineItem{
    std::string _payType;
    std::string _loanId;
    std::string _entityId;
    std::string _company;
    std::string _loanName;
    std::string _interestRate;
    std::string _riskBand;
    std::string _interest;
    std::string _principal;
    std::string _total;
    std::string _loopFee;
    std::string _dueDate;
    std::string _datePaid;
    std::string _status;

    LineItem _lineItem;
    bool _isBlank;

public:
    std::string PayId;
    std::string PayType() const { return _payType; }
    std::string EntityId() const { return _entityId; }
    std::string LoanId() const { return _loanId; }
    std::string Company() const { return _company; }
    std::string LoanName() const { return _loanName; }
    std::string InterestRate() const { return _interestRate; }
    std::string RiskBand() const { return _riskBand; }
    std::string Interest() const { return _interest; }
    std::string Principal() const { return _principal; }
    std::string Total() const { return _total; }
    std::string LoopFee() const { return _loopFee; }
    std::string DueDate() const { return _dueDate; }
    std::string DatePaid() const { return _datePaid; }
    std::string Status() const { return _status; }

    bool isBlank() const { return _isBlank; }

    LineItem GetLineItem() const { return _lineItem; }

    DisplayLineItem(): _isBlank(true) {

    }

    DisplayLineItem(std::string payType, std::string loanId, std::string entityId, std::string company, std::string loanName, std::string interestRate, std::string riskBand, 
			std::string interest, std::string principal, std::string total, std::string loopFee, std::string dueDate, std::string datePaid, std::string status);
    
    DisplayLineItem(LineItem const& item);
};