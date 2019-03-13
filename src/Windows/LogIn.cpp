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

#include <Windows/LogIn.h>
#include <defines.h>
#include <Utility.h>

#include <fstream>

using namespace std;

LogIn::LogIn(GObject* window): DialogComponents(window), _logins(), _actb(nullptr){
    
    if (btnLogIn)
        g_signal_connect(btnLogIn, "clicked", G_CALLBACK(LogIn::LogIn_Clicked), this);

    if (btnCancel)
        g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_widget_hide), window);

    g_signal_connect(window, "show", G_CALLBACK(LogIn::OnShow), this);

    GtkWidget* txtPassword = getWidget("txtPassword");
    if (txtPassword)
        g_signal_connect(txtPassword, "activate", G_CALLBACK(LogIn::LogIn_Clicked), this);

    loadLogins();
}

string LogIn::getEntryText(GtkWidget* textBox) const{
    if (textBox)
        return string(gtk_entry_get_text(GTK_ENTRY(textBox)));
    return "";
}

string LogIn::getEmail() const{
    return getEntryText(getWidget("txtEmail"));
}

string LogIn::getPassword() const{
    return getEntryText(getWidget("txtPassword"));
}

void LogIn::loadLogins(){
    ifstream ifs(LOGINS);
    if (ifs.is_open()){
        _logins.clear();
        string line;
        while (getline(ifs, line))
            _logins.push_back(line);
        ifs.close();

        GtkWidget* txtEmail = getWidget("txtEmail");
        if(txtEmail)
            _actb = new AutoCompleteTextBox(txtEmail, _logins, true, false);
    }
}

void LogIn::saveLogins(string newLogin){
    Utility::ToLowerCase(newLogin);
    if(!newLogin.empty())
        _logins.push_back(newLogin);
    
    if (_actb)
        _actb->addCompletionOption(newLogin);
    else{
        vector<string> logins;
        logins.push_back(newLogin);
        GtkWidget* txtEmail = getWidget("txtEmail");
        if(txtEmail)
            _actb = new AutoCompleteTextBox(txtEmail, logins);
    }

    ofstream ofs(LOGINS);
    if (ofs.is_open()){
        set<string> distinct = Utility::Distinct(_logins);
        for (auto const& x : distinct)
            ofs << x << endl;
        ofs.close();
    }
}

LogIn::~LogIn(){
    if (_actb)
        delete _actb;
}

void LogIn::setEmail(string const& email){
    GtkWidget* txtEmail = getWidget("txtEmail");
    if (txtEmail)
        gtk_entry_set_text(GTK_ENTRY(txtEmail), email.c_str());
}

void LogIn::setPassword(string const& password){
    GtkWidget* txtPassword = getWidget("txtPassword");
    if (txtPassword)
        gtk_entry_set_text(GTK_ENTRY(txtPassword), password.c_str());
}

void LogIn::ShowDialog(){
    DialogComponents::ShowDialog();
}

void LogIn::LogIn_Clicked(GtkWidget* widget, gpointer data){
    LogIn* li = (LogIn*)data;

    string email = li->getEmail();
    string password = li->getPassword();

    if (email.empty() || password.empty()){
        MessageBox::Show(li->_window, MessageBox::MessageBoxType::ERROR, MessageBox::MessageBoxButtons::OK, "Please enter Email and Password", "Blank Field");
        return;
    }

    li->_result = MessageBox::MessageBoxResult::rOK;
    gtk_widget_hide(GTK_WIDGET(li->_window));
    gtk_dialog_response(GTK_DIALOG(li->_window), 1);
}

void LogIn::OnShow(GtkWidget* widget, gpointer data){
    LogIn* li = (LogIn*)data;
    li->_result = MessageBox::MessageBoxResult::rCANCEL;
}