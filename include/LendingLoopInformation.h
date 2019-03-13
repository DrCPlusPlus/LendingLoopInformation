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
#include <string>
#include <thread>

#include <LendingLoop/LoopConnector.h>
#include <LendingLoop/LenderManipulator.h>
#include <BrowserLaunch.h>
#include <GuiComponents/MessageBox.h>
#include <GuiComponents/AutoCompleteTextBox.h>
#include <GuiComponents/ContextMenu.h>
#include <Windows/Settings.h>
#include <Windows/LogIn.h>
#include <Windows/PopUpSummary.h>
#include <Windows/CalendarPopUp.h>
#include <AllPaymentsList.h>

class LendingLoopInformation{
    Options _options;
    Settings* _settings;
    LogIn* _login;
    PopUpSummary* _popSum;
    LoopConnector* _loopConn;
    LenderManipulator* _manipulator;
    LenderManipulator* _interimManipulator;         //use this on the new thread that gets the all payments so it can be accessed in the gdk_thread_add_idle callback
    std::string _threadError;

    GObject* _openFileDialog;
    GObject* _saveFileDialog;
    GObject* _window;
    GObject* _lblStatus;
    GObject* _miSaveAs;

    GtkWidget* _tabs;
    GObject* _mainGrid;
    GtkWidget* _btnRefresh;
    GtkWidget* _btnGetAllPayments;
    GtkWidget* _btnFirstBlank;
    GtkWidget* _btnLastBlank;
    GtkWidget* _btnNextBlank;
    GtkWidget* _btnPrevBlank;
    GtkWidget* _allPaymentsGrid;

    CalendarPopUp* _calPop;
    
    ContextMenu* _cm;
    bool* _TrueVal;
    bool* _FalseVal;
    bool _suppressEvent;

    std::thread _t;

    // store the all payments lines to easily switch back
    AllPaymentsList _allPaymentsLines;
    GtkListStore* _allPaymentsModel;        //probably need to unref somewhere like when opening a new schedule

    AllPaymentsList _displayedLines;
    std::vector<int> _allPaymentsGridSelectedIndices;

    std::vector<DisplayLineItem> _displayedLinesUnpaid;
    std::vector<int> _unpaidPaymentsGridSelectedIndices;

    std::vector<DisplayLineItem>* _activeGridLines;
    std::vector<int>* _activeGridSelectedIndices;

    std::vector<GtkWidget*> _filters;

    std::vector<std::vector<DisplayLineItem>*> _loanSummaryTabActiveGridLines;
    std::vector<std::vector<int>*> _loanSummaryTabActiveGridSelectedIndices;

    void updateStatus(std::string const& status);
    void buildOnlineSummary();
    void setWidgetMargins(GtkWidget* widget, int left, int top, int right, int bottom);

    enum labelTextSize{
        xx_small,
        x_small,
        small,
        medium,
        large,
        x_large,
        xx_large
    };

    void setLabelMarkup(GtkWidget* label, bool bold = false, bool italic = false, labelTextSize textSize = labelTextSize::medium);

    void clearTabsExceptSummary();

    void populateTabs();
    void buildAllPaymentsOrig();
    void buildAllPaymentsUnpaid();
    void buildYearTab(int yearIdx);
    void buildYearlySummary();
    void buildLoanSummaryTab(LineItem item);

    // part of Refresh Button click
    void refreshTask();

    // part of Get All Payments click
    void getAllPaymentsTask();

    void buildContextMenu();

    void clearAllPaymentsGridSelection(){
        GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(_allPaymentsGrid));
        gtk_tree_selection_unselect_all(select);
        gtk_widget_set_sensitive(_btnNextBlank, FALSE);
        gtk_widget_set_sensitive(_btnPrevBlank, FALSE);
    }

    static std::vector<int> GetGridSelectedIndices(GObject* obj);

    GtkWidget* buildLoanSummaryContent(LineItem const& item);

    void destroyTab(GtkWidget* widget);

public:
    LendingLoopInformation(int argc, char*** argv);
    ~LendingLoopInformation();
    void Run(void);

    // menu item callbacks
    static void openFile(GtkWidget* widget, gpointer data);
    static void doStuff(GtkWidget* widget, gpointer data);
    static void logIn(GtkWidget* widget, gpointer data);
    static void saveAsStart(GtkWidget* widget, gpointer data);
    static void saveAs(GtkWidget* widget, gpointer data);

    //online summary button callbacks
    static void OpenDashboard_Clicked(GtkWidget* widget, gpointer data);

    // Get All Payments Button
    static void GetAllPayments_Clicked(GtkWidget* widget, gpointer data);
    static gboolean GetAllPayments_Finishing(gpointer data);


    //Refresh button
    static void Refresh_Clicked(GtkWidget* widget, gpointer data);
    static gboolean Refresh_Finishing(gpointer data);

    // All Payments blanks navigation callback
    static void BlankNavigation_Clicked(GtkWidget* widget, gpointer data);
    static void GridSelectionChanged(GtkTreeSelection* treeselection, gpointer data);
    
    // All Payments context menu 
    static gint ContextMenu_Show(GtkWidget* widget, GdkEvent* event, gpointer data);
    static void miSummaryPopUp(GtkWidget* widget, gpointer data);
    static void miTodaysDate(GtkWidget* widget, gpointer data);
    static void miCalendarPopUp(GtkWidget* widget, gpointer data);
    static void calendarCallback(DateTime dt, DateTime dt2, bool isRange, void* data);
    
    static void miLoanSummary_Click(GtkWidget* widget, gpointer data);
    static void miAllFilter_Click(GtkWidget* widget, gpointer data);
    static void miFilter_Click(GtkWidget* widget, gpointer data);

    static gint Tab_Click(GtkWidget* widget, GdkEvent* event, gpointer data);
    static void CloseButton_Click(GtkWidget* widget, gpointer data);
};