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
#include <sstream>
#include <iomanip>
#include <Utility.h>

class DisplayData{
    bool _isBlank;

    double _interestScheduled;
    double _feeDueScheduled;
    double _principalScheduled;
    double _totalDueScheduled;

    //represents payments received
    double _interestPaid;
    double _feeDuePaid;
    double _principalPaid;
    double _totalDuePaid;

public:
    std::string Month;

    std::string Principal(){
        return _isBlank ? "" : Utility::format(_principalScheduled);
    }

    std::string Principal_Paid(){
        return _isBlank ? "" : Utility::format(_principalPaid);
    }

    std::string Total_Due() {
        return _isBlank ? "" : Utility::format(_totalDueScheduled);
    }

    std::string Total_Due_Paid() {
        return _isBlank ? "" : Utility::format(_totalDuePaid);

    }

    std::string Interest() {
        return _isBlank ? "" : Utility::format(_interestScheduled);

    }

    std::string Interest_Paid() {
        return _isBlank ? "" : Utility::format(_interestPaid);

    }

    std::string Fee() {
        return _isBlank ? "" : Utility::format(_feeDueScheduled);

    }

    std::string Fee_Paid(){
        return _isBlank ? "" : Utility::format(_feeDuePaid);
    }

    std::string Total_Interest() {
        return _isBlank ? "" : Utility::format(_interestScheduled - _feeDueScheduled);

    }

    std::string Total_Interest_Paid() {
        return _isBlank ? "" : Utility::format(_interestPaid - _feeDuePaid);
    }

    double GetInterestScheduled() const { return _interestScheduled; }
    double GetFeesScheduled() const { return _feeDueScheduled; }
    double GetPrincipalScheduled() const { return _principalScheduled; }
    double GetTotalDueScheduled() const { return _totalDueScheduled; }
    double GetInterestActual() const { return _interestPaid; }
    double GetFeeActual() const { return _feeDuePaid; }
    double GetPrincipalActual() { return _principalPaid; }
    double GetTotalDueActual() { return _totalDuePaid; }

    DisplayData (std::string const& month, double interest, double fee, double principal, 
            double total, double interestActual = 0.0, double feeActual = 0.0, double principalActual = 0.0, 
            double totalActual = 0.0, bool blank = false) : 
                Month(month), _interestScheduled(interest), _feeDueScheduled(fee), 
                _isBlank(blank), _principalScheduled(principal), _totalDueScheduled(total), 
                _interestPaid(interestActual), _feeDuePaid(feeActual), _principalPaid(principalActual), 
                _totalDuePaid(totalActual){

    }

};