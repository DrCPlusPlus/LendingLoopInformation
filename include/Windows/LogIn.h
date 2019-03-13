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

#include <GuiComponents/DialogComponents.h>
#include <GuiComponents/AutoCompleteTextBox.h>
#include <GuiComponents/MessageBox.h>

#include <string>
#include <vector>

class LogIn: public DialogComponents{
    MessageBox::MessageBoxResult _result;
    std::vector<std::string> _logins;
    AutoCompleteTextBox* _actb;

    void loadLogins();
    std::string getEntryText(GtkWidget* textBox) const;
public:
    

    LogIn(GObject* window);
    ~LogIn();

    std::string getEmail() const;
    std::string getPassword() const;
    void setEmail(std::string const& email);
    void setPassword(std::string const& password);
    MessageBox::MessageBoxResult getResult() const { return _result; }

    void ShowDialog() override;
    void saveLogins(std::string newLogin);
    static void OnShow(GtkWidget* widget, gpointer data);
    static void LogIn_Clicked(GtkWidget* widget, gpointer data);

};