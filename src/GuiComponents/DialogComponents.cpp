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
#include <Utility.h>

using namespace std;

DialogComponents::DialogComponents(GObject* window): _window(window),
        btnOK(nullptr), btnCancel(nullptr), btnYes(nullptr), 
        btnNo(nullptr), btnSave(nullptr), btnOpen(nullptr), widgets(), btnLogIn(nullptr){
    populateMap(window);
}

void DialogComponents::populateMapName(GObject* widget){
    string id(gtk_widget_get_name(GTK_WIDGET(widget)));
    if (!id.empty()){
        widgets[Utility::ToLowerCase(id)] = GTK_WIDGET(widget);
        if (GTK_IS_BUTTON(widget)){
            if (id.find("ok") != string::npos && !btnOK)
                btnOK = GTK_WIDGET(widget);
            else if (id.find("cancel") != string::npos && !btnCancel)
                btnCancel = GTK_WIDGET(widget);
            else if (id.find("yes") != string::npos && !btnYes)
                btnYes = GTK_WIDGET(widget);
            else if (id.find("no") != string::npos && !btnNo)
                btnNo = GTK_WIDGET(widget);
            else if (id.find("save") != string::npos && !btnSave)
                btnSave = GTK_WIDGET(widget);
            else if (id.find("open") != string::npos && !btnOpen)
                btnOpen = GTK_WIDGET(widget);
            else if (id.find("login") != string::npos && !btnLogIn)
                btnLogIn = GTK_WIDGET(widget);
        }
    }
}

void DialogComponents::populateMap(GObject* widget){
    populateMapName(widget);
    if (GTK_IS_CONTAINER(widget)){
        GList* list = gtk_container_get_children(GTK_CONTAINER(widget));
		GList* children = list;
        while(children){
            populateMap((GObject*)children->data);
            children = children->next;
        }
		g_list_free(list);
    }
}

GtkWidget* DialogComponents::getWidget(string name) const {
    Utility::ToLowerCase(name);
    map<string, GtkWidget*>::const_iterator cit = widgets.find(name);
    if (cit != widgets.cend())
        return cit->second;
    return nullptr;
}

void DialogComponents::ShowDialog(){
    gint response = gtk_dialog_run(GTK_DIALOG(_window));
    //g_print("response = %d\n", response);
    gtk_widget_hide(GTK_WIDGET(_window));
}