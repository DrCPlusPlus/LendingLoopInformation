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

#include "Base64.h"
#include <iostream>
#include <iomanip>
using namespace std;

char Base64::encodeMap[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
					  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
					  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

std::string Base64::toBase64(std::string const& input) {
	string output;
	int runTo = input.size() / 3 * 3;
	int leftover = input.size() % 3;
	int padding = 3 - leftover;
	padding %= 3;

	uint32_t mapKey = 0;
	uint32_t bits24 = 0;

	unsigned char const* ptr;
	ptr = (unsigned char const*)input.data();

	for (int i = 0; i < runTo; i += 3) {
		bits24 = 0;
		for (int j = 0; j < 3; ++j)
			bits24 = bits24 << 8 | ptr[i + j];

		for (int x = 3; x >= 0; --x) {
			mapKey = bits24;
			mapKey >>= (6 * x);
			mapKey &= 0x0000003F;
			output += Base64::encodeMap[mapKey];
		}
	}
	bits24 = 0;
	for (int i = 0; i < leftover; ++i)
		bits24 = bits24 << 8 | ptr[i + runTo];

	for (int i = 0; i < (3 - leftover) && leftover; ++i)
		bits24 <<= 8;

	for (int x = 0; x < (4 - padding) && padding; ++x) {
		bits24 <<= 6;
		mapKey = bits24;
		mapKey &= 0xFF000000;
		mapKey >>= 24;
		output += Base64::encodeMap[mapKey];
		bits24 &= 0x00FFFFFF;
	}


	for (int i = 0; i < padding; ++i)
		output += '=';

	return output;
}

char Base64::decodeMap[] = {
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 62, 00,	00, 00, 63, 52, 53,
	54, 55, 56, 57,	58, 59, 60, 61, 00,	00,
	00, 00, 00, 00, 00, 00,  1,  2,  3,  4,
	5,	 6,  7,  8,  9, 10, 11, 12, 13, 14,
	15,	16, 17, 18, 19, 20, 21, 22, 23, 24,
	25,	00, 00, 00, 00, 00, 00, 26, 27, 28,
	29,	30, 31, 32, 33, 34,	35, 36, 37, 38,
	39, 40, 41, 42, 43, 44,	45, 46, 47, 48,
	49,	50, 51
};


std::string Base64::fromBase64(std::string const& input) {
	string output;
	string escaped = "";
	for (auto const& x : input){
		if (x == '\n' || x == '\r')
			continue;
		escaped += x;
	}

	int runs = escaped.size() / 4 * 4;
	int leftover = escaped.size() % 4;
	uint32_t bits;
	unsigned char val;
	for (int x = 0; x < runs; x += 4) {
		bits = 0;
		bits |= Base64::decodeMap[escaped[x]];
		bits <<= 8;

		val = Base64::decodeMap[escaped[x + 1]];
		val <<= 2;
		bits |= val;
		bits <<= 6;
		val = Base64::decodeMap[escaped[x + 2]];
		val <<= 2;
		bits |= val;
		bits <<= 6;
		val = Base64::decodeMap[escaped[x + 3]];
		val <<= 2;
		bits |= val;
		bits >>= 2;

		for (int x = 2; x >= 0; --x)
			output += (char)((bits >> (x * 8)) & 0xFF);

	}

	int padding = 0;
	string::const_reverse_iterator ri = escaped.crbegin();
	while (*ri++ == '=') ++padding;

	output.erase(output.size() - padding, padding);

	return output;
}