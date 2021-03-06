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

#include <Options.h>
#include <fstream>
#include <string>
#include <gtk/gtk.h>

class Settings: public DialogComponents{
    Options& _options;
    GtkWidget* _chkUsePrivateBrowsing;
    GtkWidget* _rdoChrome;
    GtkWidget* _rdoFireFox;
    void loadSettings();

public:
    Settings(Options& o, GObject* dialog);
    static void SaveSettings(GObject* parent, Options const& o, std::string const& fileName);
    static void OK_Clicked(GtkWidget* widget, gpointer data);
    void ShowDialog() override;
};
