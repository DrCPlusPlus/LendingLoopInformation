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
#include <Displayables/DisplayLineItem.h>
#include <gtk/gtk.h>
#include <string>
#include <defines.h>
#include <vector>
#include <Utility.h>

class PopUpSummary: public DialogComponents {
    
    struct Data{
    public:
        std::string QuantitySelected;
        std::string Interest;
        std::string Principal;
        std::string Total;
        std::string Fee;
        std::string TotalMinusFee;
        std::string InterestMinusFee;
    };


    void ShowDialog() override {
        gtk_widget_show_all(GTK_WIDGET(_window));
        // in order for the window to grab focus this way, I had to set the window hint to none (having it on pop up menu prevented 'grab focus' from working :S)
        gtk_widget_grab_focus(GTK_WIDGET(_window));
     }


public:
    PopUpSummary(GObject* window);


    void Display(std::vector<DisplayLineItem> const& items);


    static gboolean lostFocus(GtkWidget* widget, GdkEvent* event, gpointer data){
        // in order to grab the 'focus-out-event' the window must be a 'top level' not a pop up, pop ups do not receive this signal!!
        gtk_widget_hide(GTK_WIDGET(widget));
        return TRUE;
    }

};