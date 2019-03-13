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

#include <Windows/Settings.h>
#include <GuiComponents/MessageBox.h>
#include <defines.h>
#include <Utility.h>

using namespace std;

Settings::Settings(Options& o, GObject* dialog) : DialogComponents(dialog), _options(o){
    loadSettings();
    GtkWidget* rdoButton = nullptr;
    switch (o.PreferredBrowser){
        case Options::Browser::Chrome:
            rdoButton = getWidget("rdoSettingsChrome");
            break;
        case Options::Browser::Firefox:
        default:
            rdoButton = getWidget("rdoSettingsFireFox");
            break;
    }

    if (rdoButton && GTK_IS_TOGGLE_BUTTON(rdoButton)){
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(rdoButton), TRUE);
    }

    _chkUsePrivateBrowsing = getWidget("chkSettingsUsePrivate");
    if (_chkUsePrivateBrowsing && GTK_IS_TOGGLE_BUTTON(_chkUsePrivateBrowsing)){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_chkUsePrivateBrowsing), o.UsePrivateBrowsing ? TRUE : FALSE);
    }

    _rdoChrome = getWidget("rdoSettingsChrome");
    _rdoFireFox = getWidget("rdoSettingsFireFox");

    if (btnCancel)
        g_signal_connect_swapped(btnCancel, "clicked", G_CALLBACK(gtk_widget_hide), dialog);
    
    if (btnOK)
        g_signal_connect(btnOK, "clicked", G_CALLBACK(Settings::OK_Clicked), this);
    
}

void Settings::loadSettings(){
    ifstream ifs(OPTIONS);
    if (ifs.is_open()){
        string line;
        if (getline(ifs, line)){
            _options.PreferredBrowser = Utility::ToLowerCase(line) == "chrome" ? Options::Browser::Chrome : Options::Browser::Firefox;
            if (getline(ifs, line))
                _options.UsePrivateBrowsing = Utility::ToLowerCase(line) == "true";
            else
                _options = Options();
        }
        ifs.close();
    }
}

void Settings::SaveSettings(GObject* parent, Options const& o, string const& filename) {
    
    ofstream ofs(filename);
    if (ofs.is_open()){
        ofs << (o.PreferredBrowser == Options::Browser::Chrome ? "Chrome" : "FireFox") << endl;
        ofs << boolalpha << o.UsePrivateBrowsing << endl;
        ofs.close();
    }
    else{
        string err(g_strerror (errno));
        MessageBox::Show(parent, MessageBox::MessageBoxType::ERROR, MessageBox::MessageBoxButtons::OK, "Could not save settings!\n" + err, "Error Saving Settings!");
    }
}

void Settings::OK_Clicked(GtkWidget* widget, gpointer data){
    
    Settings* s = (Settings*) data;
    s->_options.PreferredBrowser = Options::Browser::Firefox;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->_rdoChrome)))
        s->_options.PreferredBrowser = Options::Browser::Chrome;
        
    s->_options.UsePrivateBrowsing = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->_chkUsePrivateBrowsing));
    SaveSettings(s->_window, s->_options, OPTIONS);
    gtk_widget_hide(GTK_WIDGET(s->getWindow()));
}

void Settings::ShowDialog(){
    
}