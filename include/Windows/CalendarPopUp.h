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
#include <Displayables/DateTime.h>
#include <gtk/gtk.h>
#include <string>
#include <defines.h>
#include <vector>
#include <Utility.h>


class CalendarPopUp: public DialogComponents {
public:
    typedef void (*DateSelectionChanged)(DateTime dt1, DateTime dt2, bool isRange, void* data);
private:
    DateSelectionChanged _dateSelectedCallback;
    DateTime _selectedDate;
    DateTime _startDate;
    DateTime _endDate;
    GtkWidget* _calendar;
    GtkWidget* _lblStatus;
    GtkWidget* _chkRange;
    GtkWidget* _lblStartDate;
    GtkWidget* _lblEndDate;
    bool _selectingRange;
    void* _userData;

    void finalize(){
        gtk_widget_hide(GTK_WIDGET(_window));
        if (_dateSelectedCallback){
            if (_selectingRange)
                _dateSelectedCallback(_startDate, _endDate, true, _userData);
            else
                _dateSelectedCallback(_selectedDate, DateTime(), false, _userData);
        }
    }
    void init();

public:

    CalendarPopUp(GObject* window, DateSelectionChanged callback, void* data): DialogComponents(window), _selectedDate(), 
                _startDate(), _endDate(), _dateSelectedCallback(callback), _userData(data), _selectingRange(false) {
        init();
    }

    void ShowDialog() override {
        gtk_label_set_text(GTK_LABEL(_lblStatus), "");
        gtk_label_set_text(GTK_LABEL(_lblStartDate), "");
        gtk_label_set_text(GTK_LABEL(_lblEndDate), "");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_chkRange), FALSE);
        DateTime t = DateTime::Now();
        gtk_calendar_select_month(GTK_CALENDAR(_calendar), t.Month() - 1, t.Year());
        gtk_calendar_select_day(GTK_CALENDAR(_calendar), t.Day());
        gtk_widget_show_all(GTK_WIDGET(_window));
        // in order for the window to grab focus this way, I had to set the window hint to none (having it on pop up menu prevented 'grab focus' from working :S)
        gtk_widget_grab_focus(GTK_WIDGET(_window));
     }

    static gboolean lostFocus(GtkWidget* widget, GdkEvent* event, gpointer data){
        // in order to grab the 'focus-out-event' the window must be a 'top level' not a pop up, pop ups do not receive this signal!!
        gtk_widget_hide(GTK_WIDGET(widget));
        return TRUE;
    }

    static void DateChanged (GtkCalendar* calendar, gpointer data);
    static void DoubleClickDateChanged (GtkWidget* calendar, gpointer data);
    
    static void chkRange_Toggled(GtkToggleButton* widget, gpointer data);
    static void Today_Clicked(GtkWidget* widget, gpointer data);

};