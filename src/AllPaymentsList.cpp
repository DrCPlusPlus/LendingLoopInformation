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

#include <AllPaymentsList.h>
#include <defines.h>
using namespace std;

void AllPaymentsList::determineIndexOfFirstBlankPayment(){
    for (size_type i = 0; i < size(); ++i){
        if (!(*this)[i].isBlank() && (*this)[i].DatePaid().size() == 0){
            _firstBlank = i;
            return;
        }
    }
    _firstBlank = -1;
}

void AllPaymentsList::determineIndexOfLastBlankPayment(){
    int previousIdx = size() - 1;
    for (int i = size() - 1; i >= 0; --i){
        if (!(*this)[i].isBlank() && (*this)[i].DatePaid().size() > 0){
            _lastBlank = previousIdx;
            return;
        }
        else if (!(*this)[i].isBlank())
            previousIdx = i;
        else{
            //don't care
        }
    }
    _lastBlank = -1;
}

int AllPaymentsList::IndexOfFirstBlankPayment(){
    if (!_firstBlankIdxDetermined){
        determineIndexOfFirstBlankPayment();
        _firstBlankIdxDetermined = true;
    }
    return _firstBlank;
}


int AllPaymentsList::IndexOfLastBlankPayment(){
    if (!_lastBlankIdxDetermined){
        determineIndexOfLastBlankPayment();
        _lastBlankIdxDetermined = true;
    }
    return _lastBlank;
}

int AllPaymentsList::IndexOfNextBlankPayment(int startIdx, int idxOfLast){
    for (int x = startIdx + 1; x <= idxOfLast; ++x){
        if (!(*this)[x].isBlank() && (*this)[x].DatePaid().size() == 0)
            return x;
    }
    return -1;
}

int AllPaymentsList::IndexOfPreviousBlankPayment(int startIdx, int idxOfFirst){
    for (int x = startIdx - 1; x >= idxOfFirst && x >= 0; --x){
        if (!(*this)[x].isBlank() && (*this)[x].DatePaid().size() == 0)
            return x;
    }
    return -1;
}

bool AllPaymentsList::HasNextBlank(int startIdx, int idxOfLast){
    if (idxOfLast == -1)
        idxOfLast = this->IndexOfLastBlankPayment();
    return this->IndexOfNextBlankPayment(startIdx, idxOfLast) != -1;
}

bool AllPaymentsList::HasPreviousBlank(int startIdx, int idxOfFirst){
    if (idxOfFirst == -1)
        idxOfFirst = this->IndexOfFirstBlankPayment();
    return this->IndexOfPreviousBlankPayment(startIdx, idxOfFirst) != -1;
}


void AllPaymentsList::clear() {
    _firstBlankIdxDetermined = false;
    _lastBlankIdxDetermined = false;

    vector<DisplayLineItem>::clear();
}