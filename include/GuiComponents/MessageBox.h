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
#include <string>

class MessageBox{
public:
    enum MessageBoxResult {
        rOK,
        rYES,
        rCANCEL,
        rNO,
        rFAIL
    };

    enum MessageBoxType {
        INFO = GTK_MESSAGE_INFO,
        WARNING = GTK_MESSAGE_WARNING,
        QUESTION = GTK_MESSAGE_QUESTION,
        ERROR = GTK_MESSAGE_ERROR,
        OTHER = GTK_MESSAGE_OTHER
    };

    enum MessageBoxButtons {
        NONE = GTK_BUTTONS_NONE,
        OK = GTK_BUTTONS_OK,
        CLOSE = GTK_BUTTONS_CLOSE,
        CANCEL = GTK_BUTTONS_CANCEL,
        YES_NO = GTK_BUTTONS_YES_NO,
        OK_CANCEL = GTK_BUTTONS_OK_CANCEL
    };

    static MessageBoxResult Show(GObject* parent, MessageBoxType type, MessageBoxButtons buttons, std::string const& message, std::string const& title);
};