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

#include <BrowserLaunch.h>
#include <Base64.h>
#include <unistd.h>
#include <iostream>

using namespace std;


void BrowserLaunch::Launch(Options& o, LoopConnector& loopConnector){
    // remember data urls...

    try{
        string browser = "firefox";
        if (o.PreferredBrowser == Options::Browser::Chrome)
            browser = "google-chrome";
        
        string arg1 =  (o.UsePrivateBrowsing ? (o.PreferredBrowser == Options::Browser::Chrome ? "-incognito" : "-private-window") : "");
        string arg2 = loopConnector.GetAuthenitcatedURL();

        pid_t cpid;

        cpid = fork();
        if (cpid == -1)
             throw "fork error";

        if (cpid == 0){
            //child process
            if (execlp(browser.c_str(), browser.c_str(), arg1.c_str(), arg2.c_str(), (char*)NULL) == -1){
                if (errno == 2){
                    // browser of choice doesn't exist, try the other one
                    string browser = "firefox";
                    if (o.PreferredBrowser == Options::Browser::Firefox){
                        browser = "google-chrome";
                        o.PreferredBrowser = Options::Browser::Chrome;
                    }
                    else
                        o.PreferredBrowser = Options::Browser::Firefox;
                    
                    string arg1 =  (o.UsePrivateBrowsing ? (o.PreferredBrowser == Options::Browser::Chrome ? "-incognito" : "-private-window") : "");
                    
                    if (execlp(browser.c_str(), browser.c_str(), arg1.c_str(), arg2.c_str(), (char*)NULL) == -1){
                        // out of options!
                        exit(EXIT_FAILURE);
                    }
                }
                exit(EXIT_FAILURE);
            }
        }
    }
    catch (char const* msg){
        cout << __LINE__ << " " << __PRETTY_FUNCTION__ << " msg = " << msg << endl;
    }
    catch (...){
        cout << __LINE__ << " " << __PRETTY_FUNCTION__ << "EXCEPTION THROWN!" << endl;
    }
}
