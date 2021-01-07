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

#include <ctime>
#include <vector>

class DateTime{
    int _year;
    int _month;     // 1 - 12
    int _day;       // 1 - 31

    time_t convertToTimeStamp() const {
        struct tm thisTime = { 0 };

        thisTime.tm_year = _year - 1900;
        thisTime.tm_mon = _month - 1;
        thisTime.tm_mday = _day;

        return ::mktime(&thisTime);
    }

public:
    DateTime(int day, int month, int year): _year(year), _month(month), _day(day){

    }
    DateTime(): _year(0), _month(0), _day(0){

    }

    int Year() const { return _year; }
    int Month() const { return _month; }
    int Day() const { return _day; }

    void Year(int year) { _year = year; }
    void Month(int month) { _month = month; }
    void Day(int day) { _day = day; }

    static DateTime Now(){
        struct tm today;
        time_t t = ::time(NULL);
        today = *::localtime(&t);

        return DateTime(today.tm_mday, today.tm_mon + 1, (today.tm_year + 1900));
    }

    void AddDays(int nDays){
        int days = 86400 * nDays;
        time_t thisTimeAsSeconds = convertToTimeStamp();
        thisTimeAsSeconds += days;
        struct tm newDay = { 0 };
        newDay = *::localtime(&thisTimeAsSeconds);
        _year = newDay.tm_year + 1900;
        _day = newDay.tm_mday;
        _month = newDay.tm_mon + 1;
    }

    void AddDay(){
        AddDays(1);
    }

	void SubtractDays(int nDays){
        int days = 86400 * nDays;
        time_t thisTimeAsSeconds = convertToTimeStamp();
        thisTimeAsSeconds -= days;
        struct tm newDay = { 0 };
        newDay = *::localtime(&thisTimeAsSeconds);
        _year = newDay.tm_year + 1900;
        _day = newDay.tm_mday;
        _month = newDay.tm_mon + 1;
    }

    void SubtractDay(){
        SubtractDays(1);
    }

    bool operator < (DateTime const& rhs) const {
        return this->convertToTimeStamp() < rhs.convertToTimeStamp();
    }

    bool operator == (DateTime const& rhs) const {
        return (_year == rhs._year && _month == rhs._month && _day == rhs._day);
    }

    bool operator != (DateTime const& rhs) const {
        return !(*this == rhs);
    }

    bool operator > (DateTime const& rhs) const {
        return (*this != rhs && !(*this < rhs));
    }

    bool operator <= (DateTime const& rhs) const {
        return (*this == rhs || *this < rhs);
    }

    bool operator >= (DateTime const& rhs) const {
        return ( *this == rhs || *this > rhs);
    }

	DateTime operator --(int) {
		DateTime dt = *this;
		this->SubtractDay();
		return dt;
	}

	DateTime& operator --() {
		this->SubtractDay();
		return *this;
	}

    static std::vector<DateTime> GetDateRange(DateTime const& dt, DateTime const& dt2){
        std::vector<DateTime> dates;
        if (dt == dt2)
            dates.push_back(dt);
        else{
            DateTime dtFirst = (dt > dt2 ? dt2 : dt);
            DateTime dtLast = (dt > dt2 ? dt : dt2);
            
            while (dtFirst <= dtLast){
                dates.push_back(dtFirst);
                dtFirst.AddDay();    
            }
        }
        return dates;
    }
    
};