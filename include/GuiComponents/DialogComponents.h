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
#include <map>
#include <string>

class DialogComponents{
    
    std::map<std::string, GtkWidget*> widgets;
    void populateMap(GObject* widget);
    void populateMapName(GObject* widget);
protected:
    GObject* _window;
    GtkWidget* btnOK;
    GtkWidget* btnCancel;
    GtkWidget* btnYes;
    GtkWidget* btnNo;
    GtkWidget* btnSave;
    GtkWidget* btnOpen;
    GtkWidget* btnLogIn;

    GtkWidget* getWidget(std::string name) const;

    DialogComponents(GObject* window);
    virtual ~DialogComponents() { }
    
    virtual void ShowDialog();

public:
    GObject* getWindow() const { return _window; }
};
