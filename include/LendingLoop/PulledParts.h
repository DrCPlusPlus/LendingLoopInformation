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

// Data obtained from calls to pull_active_loanparts, 
// pull_delinquent_laonparts, pull_charged_off_laonparts, 
// and pull_repaid_loanparts
class PulledParts{
    uint32_t  _loanId;
    std::string _company;
    std::string _loanName;

public:
    PulledParts(uint32_t loanId, std::string const& company, std::string const& loanName):
        _loanId(loanId), _company(company), _loanName(loanName){

        }
    PulledParts(): _loanId(0), _company(), _loanName(){

    }

    uint32_t LoanId() const { return _loanId; }
    std::string Company() const { return _company; }
    std::string LoanName() const { return _loanName; }
};