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

#include <gtk/gtk.h>
#include <ctime>

#define OPTIONS "options.dat"
#define LOGINS "Logins.dat"

#define DEBUG_MARKER g_print("%d %s\n", __LINE__, __PRETTY_FUNCTION__)

#define START_TIMER time_t time1234 = ::time(NULL);
#define END_TIMER time_t endTime = ::time(NULL); \
                time_t ellapsed = endTime - time1234; \
                ofstream ofs("timings.txt", ios_base::app); \
                if (ofs.is_open()){ \
                    ofs << __LINE__ << " " << __PRETTY_FUNCTION__ << " ellapsed: " << ellapsed << "S" << endl; \
                }

#define START_CLOCK clock_t clock1234 = ::clock();
#define END_CLOCK clock_t endClock = ::clock(); \
                clock_t ellapsed = endClock - clock1234; \
                double dElapses = (double)ellapsed / CLOCKS_PER_SEC; \
                ofstream ofs("timings.txt", ios_base::app); \
                if (ofs.is_open()){ \
                    ofs << __LINE__ << " " << __PRETTY_FUNCTION__ << " ellapsed: " << ellapsed << " ticks" << endl; \
                    ofs << __LINE__ << " " << __PRETTY_FUNCTION__ << " ellapsed time: " << dElapses << "S" << endl; \
                }