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
#include <vector>

class ContextMenu{
    std::vector<GtkWidget*> _children;
    GtkWidget* _menu;
    
public:
    ContextMenu();

    void HideChild(int childIdx);
    void ShowChild(int childIdx);
    void ShowAllChildren();

    void AddChild(GtkWidget* child);

    GtkWidget* GetMenu() const { return _menu; }
    GtkWidget* GetChild(int childIdx) const { 
        if ( _children.size() > 0 && childIdx < _children.size() && childIdx >= 0)
            return _children[childIdx];
        return nullptr;
    }

    void Show(){
        bool allChildrenHidden = true;
        for (auto& child : _children) {
            if (gtk_widget_get_visible(child)){
                allChildrenHidden = false;
                break;
            }
        }

        if (!allChildrenHidden)
            gtk_menu_popup_at_pointer(GTK_MENU(_menu), NULL);
    }

};