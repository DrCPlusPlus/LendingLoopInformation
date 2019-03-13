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

#include <GuiComponents/MessageBox.h>


using namespace std;

MessageBox::MessageBoxResult MessageBox::Show(GObject* parent, MessageBoxType type, MessageBoxButtons buttons, string const& message, string const& title){
    
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(parent), flags, (GtkMessageType)type, (GtkButtonsType)buttons, "%s", message.c_str());
    if (!title.empty())
        gtk_window_set_title(GTK_WINDOW(dialog), title.c_str());

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    switch(result){
        case GTK_RESPONSE_OK:
            return MessageBox::MessageBoxResult::rOK;
        case GTK_RESPONSE_NO:
            return MessageBox::MessageBoxResult::rNO;
        case GTK_RESPONSE_YES:
            return MessageBox::MessageBoxResult::rYES;
        default:
            return MessageBox::MessageBoxResult::rCANCEL;
    }
}