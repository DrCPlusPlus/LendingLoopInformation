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

#include <vector>
#include <Displayables/DisplayLineItem.h>

class AllPaymentsList: public std::vector<DisplayLineItem>{
    bool _firstBlankIdxDetermined;
    bool _lastBlankIdxDetermined;
    int _firstBlank;
    int _lastBlank;

    void determineIndexOfFirstBlankPayment();
    void determineIndexOfLastBlankPayment();

public:
    AllPaymentsList():std::vector<DisplayLineItem>(), _firstBlankIdxDetermined(false), _lastBlankIdxDetermined(false){

    }
    
    int IndexOfFirstBlankPayment();
    int IndexOfLastBlankPayment();
    int IndexOfNextBlankPayment(int startIdx, int idxOfLast);
    int IndexOfPreviousBlankPayment(int startIdx, int idxOfFirst);
    bool HasNextBlank(int startIdx, int idxOfLast = -1);
    bool HasPreviousBlank(int startIdx, int idxOfFirst = -1);

    void clear();

};