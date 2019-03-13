#ifndef __UTILITY_HG__
#define __UTILITY_HG__
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
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctype.h>
#include <set>
#include <ctime>

#include <Displayables/DateTime.h>

struct Utility {
private:
	static std::ostringstream oss;

public:
	Utility(){
		oss.imbue(std::locale("en_US.UTF-8"));
	}

	static std::string& Replace(std::string& str, std::string const& replace, std::string const& replaceWith) {
		std::string::size_type idx = str.find(replace);
		while (idx != std::string::npos) {
			str.replace(idx, replace.size(), replaceWith);
			idx = str.find(replace);
		}
		return str;
	}

	static std::vector<std::string> Split(std::string const& input, char splitChar) {
		std::vector<std::string> result;
		std::string::size_type pos;
		std::string token;
		std::string s = input;
		while ((pos = s.find(splitChar)) != std::string::npos) {
			token = s.substr(0, pos);
			result.push_back(token);
			s.erase(0, pos + 1);
		}
		result.push_back(s);

		return result;
	}

	template <typename U, typename T>
	static std::vector<typename U::value_type> Where(U const& list, T&& lambda){
    	std::vector<typename U::value_type> data;
		for(typename U::const_iterator cit = list.cbegin(); cit != list.cend(); ++cit){
			if (lambda(*cit))
				data.push_back(*cit);
		}
		return data;
	}

	template <typename U, typename T>
	static std::vector<typename U::const_pointer> WherePtr(U const& list, T&& lambda){
    	std::vector<typename U::const_pointer> data;
		for(typename U::const_iterator cit = list.cbegin(); cit != list.cend(); ++cit){
			if (lambda(*cit))
				data.push_back(&(*cit));
		}
		return data;
	}

	template<typename T>
	static std::set<typename T::value_type> Distinct(T const& list){
		std::set<typename T::value_type> data;
		for (typename T::const_iterator cit = list.cbegin(); cit != list.cend(); ++cit)
			data.insert(*cit);
		return data;
	}
  
	//splits a string based on ',' but ignores them if the string is delimited by '"'
	static std::vector<std::string> Separate(std::string const& data){
		std::vector<std::string> finalStrings;

		bool delimited = false;
		std::string sb;
		for (std::string::size_type x = 0; x < data.size(); ++x){
			if (data[x] == '"')
				delimited = !delimited;
			
			if (delimited)
				sb += data[x];
			else{
				if (data[x] == ','){
					finalStrings.push_back(sb);
					sb = "";
				}
				else
					sb += data[x];
			}
		}
		if (sb.size() > 0)
			finalStrings.push_back(sb);

		return finalStrings;
	}
	/*static std::vector<std::string> Separate(std::string const& data){
		std::vector<std::string> strings = Split(data, ',');

		std::vector<std::string> finalStrings;

		for (int x = 0; x < strings.size(); ++x){
			std::string s = strings[x];

			if (!s.empty() && s[0] == '"' && s.back() != '"'){
				std::string sb = s;
				do{
					sb += ",";
					sb += strings[++x];
				} while (strings[x].back() != '"' && x < strings.size());
				s = Replace(sb, "\"", "");
			}
			finalStrings.push_back(s);
		}
		return finalStrings;
	}*/

	static std::string format(double amount, bool includeDollarSign = true, int precision = 2){
		// static std::ostringstream oss;
		// static bool initialized = false;
		// if(!initialized){
		// 	oss.imbue(std::locale("en_US.UTF-8"));
		// 	initialized = true;
		// }
		oss.str("");
        oss << (includeDollarSign ? "$" : "") << std::fixed << std::setprecision(precision) << amount;
        return oss.str();
    }

	static std::string DateToString(std::string const& format, time_t stamp){
		char buffer[80] = {0};
		struct tm* timeinfo = ::localtime(&stamp);
		::strftime(buffer, 80, format.c_str(), timeinfo);
		return std::string(buffer);
	}

	static std::string DateToString(time_t stamp){
		return DateToString("%B %d %Y", stamp);
	}

	static std::string DateToString(DateTime const& date){
		static std::string MONTHS[] = { "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
		std::ostringstream oss;
		oss << MONTHS[date.Month()] << " " << date.Day() << " " << date.Year();
		return oss.str();
	}

	static int Year(time_t t){
        struct tm* tmYear  = ::localtime(&t);
        return (tmYear->tm_year + 1900);
    }

    static int Month(time_t t){
        struct tm* tmMonth  = ::localtime(&t);
        return (tmMonth->tm_mon + 1);
    }

    static int Day(time_t t){
        struct tm* tmDay  = ::localtime(&t);
        return tmDay->tm_mday;
    }

	static std::string& ToLowerCase(std::string& str){
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str;
	}

	static std::string& ToUpperCase(std::string& str){
		std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		return str;
	}
};

#endif 