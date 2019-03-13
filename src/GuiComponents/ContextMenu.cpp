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

#include <GuiComponents/ContextMenu.h>
#include <defines.h>

using namespace std;


ContextMenu::ContextMenu(): _children(){
    _menu = gtk_menu_new();
}

void ContextMenu::HideChild(int childIdx){
    if ( _children.size() > 0 && childIdx < _children.size() && childIdx >= 0)
        gtk_widget_set_visible(_children[childIdx], FALSE);
    
}


void ContextMenu::AddChild(GtkWidget* child){
    _children.push_back(child);
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu), child);
}

void ContextMenu::ShowChild(int childIdx){
    if ( _children.size() > 0 && childIdx < _children.size() && childIdx >= 0)
        gtk_widget_set_visible(_children[childIdx], TRUE);
}

void ContextMenu::ShowAllChildren(){
    for(auto& child : _children)
        gtk_widget_set_visible(child, TRUE);
}