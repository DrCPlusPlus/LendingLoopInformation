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

#include <Windows/PopUpSummary.h>

PopUpSummary::PopUpSummary(GObject* window): DialogComponents(window){
    GtkWidget* grid = getWidget("summaryGrid");
    if (grid){
        GtkCellRenderer* renderer;
        gint colIdx = 0;

        GtkTreeViewColumn* col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Selected");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);
        
        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Total Interest");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Total Principal");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Total Total");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Total Fee");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Interest Minus Fee");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Total Minus Fee");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, NULL);
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(grid));
        gtk_tree_selection_unselect_all(select);

    }
            // add lost focus event
    // in order to grab the 'focus-out-event' the window must be a 'top level' not a pop up, pop ups do not receive this signal!!
    gtk_widget_add_events(GTK_WIDGET(window), GDK_FOCUS_CHANGE_MASK);
    g_signal_connect(window, "focus-out-event", G_CALLBACK(PopUpSummary::lostFocus), NULL);

}

 void PopUpSummary::Display(std::vector<DisplayLineItem> const& items){
        double totalInterest = 0.0;
        double totalPrincipal = 0.0;
        double totalFee = 0.0;
        double totalTotal = 0.0;

        int quantitySelected = 0;

        for(auto const& item : items){
            if (item.isBlank())
                continue;
            LineItem line = item.GetLineItem();
            totalInterest += line.Interest();
            totalPrincipal += line.Principal();
            totalFee += line.LoopFee();
            totalTotal += line.Total();
            ++quantitySelected;
        }

        Data row;
        row.Interest = Utility::format(totalInterest);
        row.Principal = Utility::format(totalPrincipal);
        row.Total = Utility::format(totalTotal);
        row.Fee = Utility::format(totalFee);
        row.TotalMinusFee = Utility::format(totalTotal - totalFee);
        row.InterestMinusFee = Utility::format(totalInterest - totalFee);
        row.QuantitySelected = std::to_string(quantitySelected);

        GtkWidget* grid = getWidget("summaryGrid");
        if (grid){
            GtkListStore* ls;
            GtkTreeIter iter;
            ls = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, row.QuantitySelected.c_str(), 1, row.Interest.c_str(), 2, row.Principal.c_str(),
                                3, row.Total.c_str(), 4, row.Fee.c_str(), 5, row.InterestMinusFee.c_str(), 6, row.TotalMinusFee.c_str(), -1);
            gtk_tree_view_set_model(GTK_TREE_VIEW(grid), GTK_TREE_MODEL(ls));
            g_object_unref(ls);
        }

        ShowDialog();
    }
