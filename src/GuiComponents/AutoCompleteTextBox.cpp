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

#include <GuiComponents/AutoCompleteTextBox.h>

using namespace std;

AutoCompleteTextBox::AutoCompleteTextBox(GObject* txtBox, std::vector<std::string> const& list, bool inlineCompletion, bool popupCompletion): 
            _textBox(GTK_WIDGET(txtBox)), _ls(nullptr), _completion(nullptr), _inlineCompletion(inlineCompletion), _popupCompletion(popupCompletion) {
    init(GTK_WIDGET(txtBox), list);
}

AutoCompleteTextBox::AutoCompleteTextBox(GtkWidget* txtBox, std::vector<std::string> const& list, bool inlineCompletion, bool popupCompletion):
             _textBox(txtBox), _ls(nullptr), _completion(nullptr), _inlineCompletion(inlineCompletion), _popupCompletion(popupCompletion) {
    init(txtBox, list);
}

void AutoCompleteTextBox::init(GtkWidget* txtBox, vector<string> const& list){
    _list = list;
    GtkTreeIter iter;
	_ls = gtk_list_store_new(1, G_TYPE_STRING);
	_completion = gtk_entry_completion_new();

    for (auto const& x : list){
        gtk_list_store_append(_ls, &iter);
	    gtk_list_store_set(_ls, &iter, 0, x.c_str(), -1);
    }

    gtk_entry_completion_set_model(_completion, GTK_TREE_MODEL(_ls));
	gtk_entry_set_completion(GTK_ENTRY(txtBox), _completion);
	gtk_entry_completion_set_text_column(_completion, 0);
    gtk_entry_completion_set_inline_completion(_completion, _inlineCompletion ? TRUE : FALSE);
    gtk_entry_completion_set_popup_completion(_completion, _popupCompletion ? TRUE : FALSE);
}

string AutoCompleteTextBox::getText() const {
    gchar const* buff = gtk_entry_get_text(GTK_ENTRY(_textBox));
    return string (buff);
}

void AutoCompleteTextBox::addCompletionOption(std::string const& option){
    for (auto const& x : _list){
        if (x == option)
            return;
    }
    GtkTreeIter iter;
    gtk_list_store_append(_ls, &iter);
	gtk_list_store_set(_ls, &iter, 0, option.c_str(), -1);
    gtk_entry_completion_set_model(_completion, GTK_TREE_MODEL(_ls));
}

void AutoCompleteTextBox::setInlineCompletion(bool inlineCompletion){
    _inlineCompletion = inlineCompletion;
    gtk_entry_completion_set_inline_completion(_completion, _inlineCompletion ? TRUE : FALSE);
}

void AutoCompleteTextBox::setPopupCompletion(bool popupCompletion){
    _popupCompletion = popupCompletion;
    gtk_entry_completion_set_popup_completion(_completion, popupCompletion ? TRUE : FALSE);
}