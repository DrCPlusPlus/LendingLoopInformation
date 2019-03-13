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

#include <Windows/CalendarPopUp.h>

void CalendarPopUp::init() {
    _calendar = getWidget("CalendarDatePicker");
    if (_calendar){
        g_signal_connect(_calendar, "day-selected", G_CALLBACK(CalendarPopUp::DateChanged), this);
        g_signal_connect(_calendar, "day-selected-double-click", G_CALLBACK(CalendarPopUp::DoubleClickDateChanged), this);
    }

    _chkRange = getWidget("chkRange");
    if (_chkRange){
        g_signal_connect(_chkRange, "toggled", G_CALLBACK(CalendarPopUp::chkRange_Toggled), this);
    }

    if(btnOK){
        g_signal_connect(btnOK, "clicked", G_CALLBACK(CalendarPopUp::DoubleClickDateChanged), this);
    }

    GtkWidget* btnToday = getWidget("btnToday");
    if (btnToday){
        g_signal_connect(btnToday, "clicked", G_CALLBACK(CalendarPopUp::Today_Clicked), this);
    }
    
    _lblStatus = getWidget("lblStatus");
    _lblStartDate = getWidget("lblStartDate");
    _lblEndDate = getWidget("lblEndDate");

    // add lost focus event
    // in order to grab the 'focus-out-event' the window must be a 'top level' not a pop up, pop ups do not receive this signal!!
    gtk_widget_add_events(GTK_WIDGET(_window), GDK_FOCUS_CHANGE_MASK);
    g_signal_connect(_window, "focus-out-event", G_CALLBACK(CalendarPopUp::lostFocus), NULL);

}


void CalendarPopUp::DateChanged (GtkCalendar* calendar, gpointer data){
    CalendarPopUp* cpop = (CalendarPopUp*)data;

    if (!cpop->_selectingRange){
        gtk_widget_set_sensitive(cpop->btnOK, TRUE);
        return;
    }

    guint year;
    guint day;
    guint month;
    gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
    if (!day)
        return;

    
    ++month;
    DateTime* dt = &cpop->_endDate;
    GtkWidget* label = cpop->_lblEndDate;
    if (cpop->_startDate.Year() == 0) {
        label = cpop->_lblStartDate;
        dt = &cpop->_startDate;
        gtk_label_set_text(GTK_LABEL(cpop->_lblStatus), "Select End Date");
        gtk_calendar_select_day(GTK_CALENDAR(cpop->_calendar), 0);
    }

    dt->Year(year);
    dt->Month(month);
    dt->Day(day);
    
    gtk_label_set_text(GTK_LABEL(label), Utility::DateToString(*dt).c_str());
    gtk_widget_set_sensitive(cpop->btnOK, (cpop->_startDate.Year() != 0 && cpop->_endDate.Year() != 0));
}

void CalendarPopUp::DoubleClickDateChanged (GtkWidget* calendar, gpointer data){
    CalendarPopUp* cpop = (CalendarPopUp*)data;
    guint year;
    guint day;
    guint month;
    gtk_calendar_get_date(GTK_CALENDAR(cpop->_calendar), &year, &month, &day);
    
    ++month;

    cpop->_selectedDate.Year(year);
    cpop->_selectedDate.Month(month);
    cpop->_selectedDate.Day(day);
    cpop->finalize();
}

void CalendarPopUp::chkRange_Toggled(GtkToggleButton* widget, gpointer data){
    CalendarPopUp* cpop = (CalendarPopUp*)data;
    cpop->_selectingRange = gtk_toggle_button_get_active(widget);
    gtk_calendar_clear_marks(GTK_CALENDAR(cpop->_calendar));
    //g_print("Toggle!\n");
    if (cpop->_selectingRange) {
        gtk_label_set_text(GTK_LABEL(cpop->_lblStatus), "Select Start Date");
        gtk_widget_set_sensitive(cpop->btnOK, false);
        gtk_calendar_select_day(GTK_CALENDAR(cpop->_calendar), 0);
    }
    else {
        gtk_label_set_text(GTK_LABEL(cpop->_lblStatus), "");
        gtk_label_set_text(GTK_LABEL(cpop->_lblStartDate), "");
        gtk_label_set_text(GTK_LABEL(cpop->_lblEndDate), "");

        guint year;
        guint day;
        guint month;
        gtk_calendar_get_date(GTK_CALENDAR(cpop->_calendar), &year, &month, &day);

        gtk_widget_set_sensitive(cpop->btnOK, (day != 0));
        cpop->_startDate = DateTime();
        cpop->_endDate = DateTime();
    }
}

void CalendarPopUp::Today_Clicked(GtkWidget* widget, gpointer data){
    CalendarPopUp* cpop = (CalendarPopUp*)data;
    cpop->_selectedDate = DateTime::Now();
    cpop->_selectingRange = false;
    cpop->finalize();
}