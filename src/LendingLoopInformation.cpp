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

#include <LendingLoopInformation.h>

#include <gdk/gdk.h>
#include <LendingLoop/LoopConnector.h>
#include <LendingLoop/LenderManipulator.h>
#include <BrowserLaunch.h>
#include <GuiComponents/MessageBox.h>
#include <GuiComponents/AutoCompleteTextBox.h>
#include <Windows/Settings.h>
#include <Windows/uiAndIcon.h>
#include <defines.h>
#include <Displayables/DisplayLineItem.h>
#include <AllPaymentsList.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>

using namespace std;


LendingLoopInformation::LendingLoopInformation(int argc, char*** argv): 
        _settings(nullptr), _openFileDialog(nullptr), _saveFileDialog(nullptr), _login(nullptr), _calPop(nullptr),
        _loopConn(nullptr), _tabs(nullptr), _mainGrid(nullptr), _btnGetAllPayments(nullptr), _btnRefresh(nullptr),
        _manipulator(nullptr), _interimManipulator(nullptr), _displayedLines(),
        _allPaymentsGridSelectedIndices(), _cm(nullptr), _activeGridLines(nullptr), _activeGridSelectedIndices(nullptr),
        _displayedLinesUnpaid(), _unpaidPaymentsGridSelectedIndices(), _FalseVal(new bool(false)), _TrueVal(new bool(true)),
         _filters(), _allPaymentsLines(), _allPaymentsModel(nullptr), _suppressEvent(false), 
        _loanSummaryTabActiveGridLines(), _loanSummaryTabActiveGridSelectedIndices(), _miLogin(nullptr){
    gtk_init(&argc, argv);

    GtkBuilder* builder;
    GObject* button;
    GError* error = NULL;

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    if (gtk_builder_add_from_string(builder, uiAndIcon::GetUserInterface().c_str(), uiAndIcon::GetUserInterface().size(), &error) == 0) {
        string strError = "Error loading file: ";
        strError += error->message;
        g_clear_error(&error);
        throw strError.c_str();
    }

     /* Connect signal handlers to the constructed widgets. */
    _window = gtk_builder_get_object(builder, "window");
    gtk_window_set_icon(GTK_WINDOW(_window), uiAndIcon::GetIconPixBuf());
    //error = NULL;
    //gtk_window_set_icon_from_file(GTK_WINDOW(_window), "favicon.ico", &error);
    g_signal_connect(_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_set_size_request (GTK_WIDGET(_window), 659, 441);
    
    GObject* logInWindow = gtk_builder_get_object(builder, "login");
    _login = new LogIn(logInWindow);
    
    GObject* menuItem;
    menuItem = gtk_builder_get_object(builder, "miLogIn");
    g_signal_connect(menuItem, "activate", G_CALLBACK(LendingLoopInformation::logIn), this);
	_miLogin = GTK_WIDGET(menuItem);

    menuItem = gtk_builder_get_object(builder, "miExit");
    g_signal_connect_swapped(menuItem, "activate", G_CALLBACK(gtk_widget_hide), _window);
    g_signal_connect(menuItem, "activate", G_CALLBACK(gtk_main_quit), NULL);

    _saveFileDialog = gtk_builder_get_object(builder, "saveFileDialog");

    _miSaveAs = gtk_builder_get_object(builder, "miSaveAs");
    g_signal_connect(_miSaveAs, "activate", G_CALLBACK(LendingLoopInformation::saveAsStart), this);
    
    button = gtk_builder_get_object(builder, "btnCancelSaveFile");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_hide), _saveFileDialog);
    
    button = gtk_builder_get_object(builder, "btnSaveFile");
    g_signal_connect(button, "clicked", G_CALLBACK(LendingLoopInformation::saveAs), this);

    
    _openFileDialog = gtk_builder_get_object(builder, "openFileDialog");
    menuItem = gtk_builder_get_object(builder, "miOpen");
    g_signal_connect_swapped(menuItem, "activate", G_CALLBACK(gtk_widget_show), _openFileDialog);

    button = gtk_builder_get_object(builder, "btnCancelOpenFile");
    g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_hide), _openFileDialog);

    g_signal_connect(_openFileDialog, "file-activated", G_CALLBACK(LendingLoopInformation::openFile), this);
    button = gtk_builder_get_object(builder, "btnOpenFile");
    g_signal_connect(button, "clicked", G_CALLBACK(LendingLoopInformation::openFile), this);
    GObject* settingsDialog = gtk_builder_get_object(builder, "settings");
    
    _lblStatus = gtk_builder_get_object(builder, "lblStatus");

    _calPop = new CalendarPopUp(gtk_builder_get_object(builder, "DatePickerPopUp"), &LendingLoopInformation::calendarCallback, this);
    
    
    _options.PreferredBrowser = Options::Browser::Chrome;
    _options.UsePrivateBrowsing = false;
    _settings = new Settings(_options, settingsDialog);

    menuItem = gtk_builder_get_object(builder, "miSettings");
    g_signal_connect_swapped(menuItem, "activate", G_CALLBACK(gtk_widget_show), _settings->getWindow());

    GObject* popUpSummaryDialog = gtk_builder_get_object(builder, "popUpSummary");
    _popSum = new PopUpSummary(popUpSummaryDialog);

    _tabs = gtk_notebook_new();
    _mainGrid = gtk_builder_get_object(builder, "gridMain");
	g_object_unref(builder);
	
    buildContextMenu();
}

LendingLoopInformation::~LendingLoopInformation(){
    delete _settings;
    delete _login;
    delete _popSum;
    delete _cm;
    delete _calPop;
    delete _FalseVal;
    delete _TrueVal;

    for (auto& x : _loanSummaryTabActiveGridLines)
        delete x;

    for (auto& x : _loanSummaryTabActiveGridSelectedIndices)
        delete x;

    if (_allPaymentsModel)
        g_object_unref(_allPaymentsModel);

    if (_loopConn){
        _loopConn->Abort();
		if (_t.joinable())
        	_t.join();
        delete _loopConn;
    }
    if (_manipulator)
        delete _manipulator;

    
}

void LendingLoopInformation::buildContextMenu(){
    _cm = new ContextMenu();

    GtkWidget* miSummary = gtk_menu_item_new_with_label("Summary");
    g_signal_connect(miSummary, "activate", G_CALLBACK(LendingLoopInformation::miSummaryPopUp), this);

    GtkWidget* miLoanSummary = gtk_menu_item_new_with_label("Loan Summary");
    GtkWidget* miFilter = gtk_menu_item_new_with_label("Filter");
    GtkWidget* miAll = gtk_menu_item_new_with_label("All");
    GtkWidget* miPaid = gtk_check_menu_item_new_with_label("Paid");
    GtkWidget* miUnpaid = gtk_check_menu_item_new_with_label("Unpaid");
    GtkWidget* miScheduled = gtk_check_menu_item_new_with_label("Scheduled");

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miPaid), TRUE);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miUnpaid), TRUE);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miScheduled), TRUE);

    g_signal_connect(miPaid, "toggled", G_CALLBACK(LendingLoopInformation::miFilter_Click), this);
    g_signal_connect(miUnpaid, "toggled", G_CALLBACK(LendingLoopInformation::miFilter_Click), this);
    g_signal_connect(miScheduled, "toggled", G_CALLBACK(LendingLoopInformation::miFilter_Click), this);

    g_signal_connect(miAll, "activate", G_CALLBACK(LendingLoopInformation::miAllFilter_Click), this);
    g_signal_connect(miLoanSummary, "activate", G_CALLBACK(LendingLoopInformation::miLoanSummary_Click), this);

    _filters.push_back(miPaid);
    _filters.push_back(miUnpaid);
    _filters.push_back(miScheduled);

    g_object_set_data(G_OBJECT(miPaid), "PaymentStatus", new LineItem::PaymentStatus(LineItem::PaymentStatus::Paid));
    g_object_set_data(G_OBJECT(miUnpaid), "PaymentStatus", new LineItem::PaymentStatus(LineItem::PaymentStatus::Unpaid));
    g_object_set_data(G_OBJECT(miScheduled), "PaymentStatus", new LineItem::PaymentStatus(LineItem::PaymentStatus::Scheduled));

    GtkWidget* miDatePaid = gtk_menu_item_new_with_label("By Date Paid");
    GtkWidget* miTodaysDate = gtk_menu_item_new_with_label("Today");
	GtkWidget* miYesterdaysDate = gtk_menu_item_new_with_label("Yesterday");
    GtkWidget* miCalendar = gtk_menu_item_new_with_label("Select...");
    g_signal_connect(miCalendar, "activate", G_CALLBACK(LendingLoopInformation::miCalendarPopUp), this);
    g_signal_connect(miTodaysDate, "activate", G_CALLBACK(LendingLoopInformation::miTodaysDate), this);
	g_signal_connect(miYesterdaysDate, "activate", G_CALLBACK(LendingLoopInformation::miYesterdaysDate), this);
    
    ContextMenu datePaid;
    datePaid.AddChild(miTodaysDate);
	datePaid.AddChild(miYesterdaysDate);
    datePaid.AddChild(miCalendar);
    datePaid.ShowAllChildren();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(miDatePaid), datePaid.GetMenu());


    // GtkWidget* loanIdEntry = gtk_entry_new();
    // vector<string> ids;
    // ids.push_back("12345");
    // ids.push_back("53032");
    // ids.push_back("52684");
    // //for (auto const& x : _manipulator->GetDistinctLoanIDs())
    //    // ids.push_back(to_string(x));

    // _actb = new AutoCompleteTextBox(loanIdEntry, ids, false, true);
    // GtkWidget* miLoanFilter = gtk_menu_item_new();
    // gtk_container_add(GTK_CONTAINER(miLoanFilter), loanIdEntry);

    ContextMenu m;
    m.AddChild(miAll);
    m.AddChild(gtk_separator_menu_item_new());
    m.AddChild(miPaid);
    m.AddChild(miUnpaid);
    m.AddChild(miScheduled);
    m.AddChild(miDatePaid);
    m.ShowAllChildren();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(miFilter), m.GetMenu());

    _cm->AddChild(miSummary);
    _cm->AddChild(miLoanSummary);
    _cm->AddChild(miFilter);
}

void LendingLoopInformation::Run(void){
    gtk_main();
}

vector<int> LendingLoopInformation::GetGridSelectedIndices(GObject* obj){
    GList* list;
    GtkTreeSelection* selection;
    vector<int> selectedIndices;

    if (GTK_IS_TREE_VIEW(obj))
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(obj));
    else if (GTK_IS_TREE_SELECTION(obj))
        selection = GTK_TREE_SELECTION(obj);

    list = gtk_tree_selection_get_selected_rows(selection, NULL);
    if (list){
        int selection = 0;
         while(list){
            selection = stoi(gtk_tree_path_to_string((GtkTreePath*)list->data));
            selectedIndices.push_back(selection);
            list = list->next;
        }

        //free the list
        g_list_free_full(list, (GDestroyNotify)gtk_tree_path_free);
    }
    return selectedIndices;
}   

#pragma region Menu Item Actions
void LendingLoopInformation::openFile(GtkWidget* widget, gpointer data){
    //g_print("file-atctivated\n");
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(lli->_openFileDialog));
    //g_print("file selected = %s\n", filename);

    struct stat sb;
    if (stat(filename, &sb) == 0 && S_ISDIR(sb.st_mode)){
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(lli->_openFileDialog), filename);
    }
    else{
        gtk_widget_hide(GTK_WIDGET(lli->_openFileDialog));

        LenderManipulator* lm = nullptr;
        try{
            vector<PulledParts> pp;
        
            lm = LenderManipulator::CreateManipulator(filename, pp);
        }
        catch (string const& err){
            MessageBox::Show(lli->_window, MessageBox::MessageBoxType::ERROR, MessageBox::MessageBoxButtons::OK, err, "Error Opening File!");
            return;
        }
        if (lli->_loopConn)
            lli->_loopConn->Abort();
        if (lli->_manipulator)
            delete lli->_manipulator;
        
        for (auto& x : lli->_loanSummaryTabActiveGridLines)
            delete x;

        for (auto& x : lli->_loanSummaryTabActiveGridSelectedIndices)
            delete x;

        lli->_loanSummaryTabActiveGridLines.clear();
        lli->_loanSummaryTabActiveGridSelectedIndices.clear();
        lli->_manipulator = lm;
        gtk_widget_destroy(lli->_tabs);
        lli->_tabs = gtk_notebook_new();
        gtk_notebook_set_scrollable(GTK_NOTEBOOK(lli->_tabs), TRUE);
        gtk_widget_set_margin_top(lli->_tabs, 10);
        lli->populateTabs();
        
        gtk_widget_set_vexpand(lli->_tabs, TRUE);
        
        gtk_grid_attach(GTK_GRID(lli->_mainGrid), lli->_tabs, 0, 1, 1, 1);
        lli->updateStatus("File Opened!");
        gtk_widget_show_all(GTK_WIDGET(lli->_mainGrid));
    }
}

void LendingLoopInformation::doStuff(GtkWidget* widget, gpointer data){
    MessageBox::Show((GObject*)data, MessageBox::MessageBoxType::QUESTION, MessageBox::MessageBoxButtons::OK_CANCEL, "Error!", "Error Error!");
}

void LendingLoopInformation::logIn(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    LogIn* login = lli->_login;
    LoopConnector* loopConn = lli->_loopConn;
    bool loadData = false;

    while (true){
        if (loopConn){
            login->setEmail(loopConn->getEmail());
            login->setPassword(loopConn->getPassword());
        }

        login->ShowDialog();
        if (login->getResult() == MessageBox::MessageBoxResult::rOK){
            if (loopConn){
                loopConn->Abort();
                delete loopConn;
            }
            lli->_loopConn = new LoopConnector(login->getEmail(), login->getPassword());
            loopConn = lli->_loopConn;

            if (!loopConn->LogIn())
                MessageBox::Show(lli->_window, MessageBox::MessageBoxType::WARNING, MessageBox::MessageBoxButtons::OK, "Invalid Email or Password!", "Invalid log in");
            else{
                lli->updateStatus("Login Successful!");
                loadData = true;
                login->saveLogins(login->getEmail());
                break;
            }
        }
        else
            break;
    }

    if (loadData){
        gtk_widget_destroy(lli->_tabs);
        lli->_tabs = gtk_notebook_new();
        gtk_notebook_set_scrollable(GTK_NOTEBOOK(lli->_tabs), TRUE);
        gtk_widget_set_margin_top(lli->_tabs, 10);
        lli->buildOnlineSummary();
        gtk_widget_set_vexpand(lli->_tabs, TRUE);
        gtk_grid_attach(GTK_GRID(lli->_mainGrid), lli->_tabs, 0, 1, 1, 1);
        gtk_widget_show_all(GTK_WIDGET(lli->_mainGrid));
    }

}

void LendingLoopInformation::saveAsStart(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    
    string fileName = "all_payments" + Utility::DateToString("%Y-%m-%d_%H.%M.%S", ::time(NULL)) + ".csv";
    
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(lli->_saveFileDialog), fileName.c_str());
    gtk_widget_show(GTK_WIDGET(lli->_saveFileDialog));
}

void LendingLoopInformation::saveAs(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(lli->_saveFileDialog));
    ofstream ofs(filename);
    if (ofs.is_open()){
        ofs << lli->_manipulator->All_paymentsCsv();
        ofs.close();
    }
    gtk_widget_hide(GTK_WIDGET(lli->_saveFileDialog));
}
#pragma endregion

void LendingLoopInformation::updateStatus(std::string const& status){
    if (_lblStatus){
        gtk_label_set_text(GTK_LABEL(_lblStatus), status.c_str());
    }
}

void LendingLoopInformation::setWidgetMargins(GtkWidget* widget, int left, int top, int right, int bottom){
    gtk_widget_set_margin_start(widget, left);
    gtk_widget_set_margin_top(widget, top);
    gtk_widget_set_margin_end(widget, right);
    gtk_widget_set_margin_bottom(widget, bottom);
}

void LendingLoopInformation::setLabelMarkup(GtkWidget* label, bool bold, bool italic, labelTextSize textSize){
    string format;
    string text = gtk_label_get_text(GTK_LABEL(label));

    format = (bold ? "<b>" : "");
    format += (italic ? "<i>" : "");
    format += "<span font_size=\"";
    switch (textSize){
        case labelTextSize::xx_small:
            format += "xx-small";
            break;
        case labelTextSize::x_small:
            format += "x-small";
            break;
        case labelTextSize::small:
            format += "small";
            break;
        case labelTextSize::medium:
            format += "medium";
            break;
        case labelTextSize::large:
            format += "large";
            break;
        case labelTextSize::x_large:
            format += "x-large";
            break;
        case labelTextSize::xx_large:
            format += "xx-large";
            break;
        default:
            break;
    }

    format += "\">\%s</span>";
    format += (italic ? "</i>" : "");
    format += (bold ? "</b>" : ""); 

    char* markup = g_markup_printf_escaped(format.c_str(), text.c_str());
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
}

void LendingLoopInformation::buildOnlineSummary(){
    string str;

    
    str = "Lifetime Earnings:  " + _loopConn->Dashboard().strLifetimeEarnings();
    GtkWidget* tb = gtk_label_new(str.c_str());
    setLabelMarkup(tb, true, false, labelTextSize::xx_large);
    setWidgetMargins(tb, 10, 10, 10, 5);
    gtk_widget_set_halign(tb, GTK_ALIGN_START);
    //TODO: set font size on all 'text blocks' (tb)
    
    str = "Funds Available:  " + _loopConn->Dashboard().strAvailableFunds();
    GtkWidget* tb2 = gtk_label_new(str.c_str());
    setLabelMarkup(tb2, true, false, labelTextSize::xx_large);
    setWidgetMargins(tb2, 5, 10, 10, 5);
    gtk_widget_set_halign(tb2, GTK_ALIGN_START);
    
    str = "Available to Invest:  " + _loopConn->Dashboard().strRemainingPrincipal();
    GtkWidget* tb6 = gtk_label_new(str.c_str());
    setLabelMarkup(tb6, true, false, labelTextSize::xx_large);
    setWidgetMargins(tb6, 5, 10, 10, 5);
    gtk_widget_set_halign(tb6, GTK_ALIGN_START);

    str = "Annual Investment Limit Remaining: " + _loopConn->Dashboard().strInvestmentLimit();
    GtkWidget* tb7 = gtk_label_new(str.c_str());
    setWidgetMargins(tb7, 10, 10, 10, 10);
    gtk_widget_set_halign(tb7, GTK_ALIGN_START);

    str = "Total Commitments: " + _loopConn->Dashboard().strTotalCommitments();
    GtkWidget* tb3 = gtk_label_new(str.c_str());
    setWidgetMargins(tb3, 10, 10, 10, 10);
    gtk_widget_set_halign(tb3, GTK_ALIGN_START);
    
    str = "Outstanding Principal: " + _loopConn->Dashboard().strOutstandingPrincipal();
    GtkWidget* tb4 = gtk_label_new(str.c_str());
    setWidgetMargins(tb4, 10, 10, 10, 10);
    gtk_widget_set_halign(tb4, GTK_ALIGN_START);
    
    str = "Total Account Value: " + _loopConn->Dashboard().strTotalAccountValue();
    GtkWidget* tb5 = gtk_label_new(str.c_str());
    setWidgetMargins(tb5, 10, 10, 10, 10);
    gtk_widget_set_halign(tb5, GTK_ALIGN_START);

    GtkWidget* stackPanel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb6, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb7, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb4, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), tb5, TRUE, TRUE, 0);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), stackPanel, 0, 0, 1, 1);
    GtkWidget* lblHeader = gtk_label_new("Summary");
    gtk_widget_set_valign(grid, GTK_ALIGN_FILL);

    GtkWidget* btnOpenDashboard = gtk_button_new_with_label("Open Dashboard");
    gtk_widget_set_size_request(btnOpenDashboard, 125, 40);
    g_signal_connect(btnOpenDashboard, "clicked", G_CALLBACK(LendingLoopInformation::OpenDashboard_Clicked), this);
    
    _btnGetAllPayments = gtk_button_new_with_label("Get All Payments");
    gtk_widget_set_size_request(_btnGetAllPayments, 125, 40);
    g_signal_connect(_btnGetAllPayments, "clicked", G_CALLBACK(LendingLoopInformation::GetAllPayments_Clicked), this);
    
    _btnRefresh = gtk_button_new_with_label("Refresh");
    gtk_widget_set_size_request(_btnRefresh, 125, 40);
    g_signal_connect(_btnRefresh, "clicked", G_CALLBACK(LendingLoopInformation::Refresh_Clicked), this);

    setWidgetMargins(btnOpenDashboard, 0, 10, 40, 0);
    setWidgetMargins(_btnGetAllPayments, 0, 10, 40, 0);
    setWidgetMargins(_btnRefresh, 0, 10, 40, 0);

    GtkWidget* buttonStackPanel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), btnOpenDashboard, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), _btnGetAllPayments, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), _btnRefresh, FALSE, FALSE, 0);
    gtk_widget_set_halign(btnOpenDashboard, GTK_ALIGN_END);
    gtk_widget_set_halign(_btnGetAllPayments, GTK_ALIGN_END);
    gtk_widget_set_halign(_btnRefresh, GTK_ALIGN_END);
    gtk_widget_set_hexpand(buttonStackPanel, TRUE);

    gtk_grid_attach(GTK_GRID(grid), buttonStackPanel, 1, 0, 1, 1);
    gtk_notebook_insert_page(GTK_NOTEBOOK(_tabs), grid, lblHeader, 0);
}


void LendingLoopInformation::OpenDashboard_Clicked(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    BrowserLaunch::Launch(lli->_options, *lli->_loopConn);
}

void LendingLoopInformation::populateTabs(){
    
    buildAllPaymentsOrig();
    buildAllPaymentsUnpaid();
    
    for (int x = 0; x < _manipulator->YearCount(); ++x)
        buildYearTab(x);
    
    buildYearlySummary();
}

void LendingLoopInformation::buildYearlySummary(){

    
    vector<DisplayData> lines = _manipulator->GetYearlySummary();

    GtkListStore* ls;
    GtkTreeIter iter;
    ls  = gtk_list_store_new(11, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    for (auto& x : lines){
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter, 0, x.Month.c_str(), 1, x.Principal().c_str(), 
                2, x.Principal_Paid().c_str(), 3, x.Total_Due().c_str(),
                4, x.Total_Due_Paid().c_str(), 5, x.Interest().c_str(),
                6, x.Interest_Paid().c_str(), 7, x.Fee().c_str(),
                8, x.Fee_Paid().c_str(), 9, x.Total_Interest().c_str(),
                10, x.Total_Interest_Paid().c_str(), -1);
    }
    
    GtkWidget* grid = gtk_tree_view_new();
    GtkCellRenderer* render;
    gint colIdx = 0;

    GtkTreeViewColumn* col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Year");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Due");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Due Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Fee");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Fee Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, render, TRUE);
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Interest Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, render, TRUE);
    g_object_set(render, "xalign", 0.5, NULL);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);

    gtk_tree_view_set_model(GTK_TREE_VIEW(grid), GTK_TREE_MODEL(ls));
    g_object_unref(ls);
    gtk_widget_set_vexpand(grid, TRUE);

    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(grid), GTK_TREE_VIEW_GRID_LINES_BOTH);

    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrollWindow), grid);
    gtk_widget_set_vexpand(scrollWindow, TRUE);
    
    GtkWidget* tabLabel = gtk_label_new("Payment Summary");
    gtk_notebook_append_page(GTK_NOTEBOOK(_tabs), scrollWindow, tabLabel);
    
}

void LendingLoopInformation::buildYearTab(int yearIdx){

    vector<DisplayData> lines = _manipulator->GetDataForYear(_manipulator->GetYear(yearIdx));

    GtkListStore* ls;
    GtkTreeIter iter;
    ls  = gtk_list_store_new(11, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    for (auto& x : lines){
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter, 0, x.Month.c_str(), 1, x.Principal().c_str(), 
                2, x.Principal_Paid().c_str(), 3, x.Total_Due().c_str(),
                4, x.Total_Due_Paid().c_str(), 5, x.Interest().c_str(),
                6, x.Interest_Paid().c_str(), 7, x.Fee().c_str(),
                8, x.Fee_Paid().c_str(), 9, x.Total_Interest().c_str(),
                10, x.Total_Interest_Paid().c_str(), -1);
    }

    GtkWidget* grid = gtk_tree_view_new();
    GtkCellRenderer* render;
    gint colIdx = 0;

    GtkTreeViewColumn* col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Month");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Due");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Due Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Fee");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Fee Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total Interest Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    render = gtk_cell_renderer_text_new();
    g_object_set(render, "xalign", 0.5, NULL);
    gtk_tree_view_column_pack_start(col, render, TRUE);
    gtk_tree_view_column_add_attribute(col, render, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);

    gtk_tree_view_set_model(GTK_TREE_VIEW(grid), GTK_TREE_MODEL(ls));
    g_object_unref(ls);
    gtk_widget_set_vexpand(grid, TRUE);

    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(grid), GTK_TREE_VIEW_GRID_LINES_BOTH);

    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrollWindow), grid);
    gtk_widget_set_vexpand(scrollWindow, TRUE);

    int idx = Utility::Month(::time(NULL)) - 1;

    string strPath = to_string(idx);
    GtkTreePath* path = gtk_tree_path_new_from_string(strPath.c_str());

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(grid));
    gtk_tree_selection_unselect_all(selection);
    gtk_tree_selection_select_path(selection, path);
    gtk_tree_path_free(path);


    string header = to_string(_manipulator->GetYear(yearIdx));
    GtkWidget* tabLabel = gtk_label_new(header.c_str());
    gtk_notebook_append_page(GTK_NOTEBOOK(_tabs), scrollWindow, tabLabel);
}

void LendingLoopInformation::buildAllPaymentsOrig(){
    GtkListStore* ls;
    GtkTreeIter iter;
    
    _displayedLines.clear();
    for (auto const& l : _manipulator->Lines()){
        _displayedLines.push_back(DisplayLineItem(l));
        _displayedLines.push_back(DisplayLineItem());
    }
    _allPaymentsLines = _displayedLines;

    if (_allPaymentsModel){
        g_object_unref(_allPaymentsModel);
        _allPaymentsModel = nullptr;
    }

    //https://en.wikibooks.org/wiki/GTK%2B_By_Example/Tree_View/Tree_Models
    if (_manipulator->isCompanyType()){
        ls  = gtk_list_store_new(13, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : _displayedLines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.LoanId().c_str(),
                        2, x.Company().c_str(), 3, x.LoanName().c_str(),
                        4, x.InterestRate().c_str(), 5, x.RiskBand().c_str(),
                        6, x.Interest().c_str(), 7, x.Principal().c_str(),
                        8, x.Total().c_str(), 9, x.LoopFee().c_str(),
                        10, x.DueDate().c_str(), 11, x.DatePaid().c_str(),
                        12, x.Status().c_str(), -1);

        }
    }
    else if (_manipulator->isEntityType()){
        ls  = gtk_list_store_new(12, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : _displayedLines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.EntityId().c_str(),
                        2, x.LoanId().c_str(), 3, x.InterestRate().c_str(), 4, x.RiskBand().c_str(),
                        5, x.Interest().c_str(), 6, x.Principal().c_str(),
                        7, x.Total().c_str(), 8, x.LoopFee().c_str(),
                        9, x.DueDate().c_str(), 10, x.DatePaid().c_str(),
                        11, x.Status().c_str(), -1);

        }
    }
    else {
        // ??
    }
    
    GtkWidget* grid = gtk_tree_view_new();
    gtk_widget_set_tooltip_text(grid, "To search by Loan ID, set focus and start typing ID or set focus and press CTRL + F");
    GtkCellRenderer* renderer;

    gint colIdx = 0;

    GtkTreeViewColumn* col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Pay Type");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    if (_manipulator->isEntityType()){
        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Entity Id");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);
    }

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Loan ID");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    if (_manipulator->isCompanyType()){
        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Company");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Loan Name");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);
    }

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest Rate");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Risk Band");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Loop Fee");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Due Date");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Date Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Status");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);



    gtk_tree_view_set_model(GTK_TREE_VIEW(grid), GTK_TREE_MODEL(ls));
    //g_object_unref(ls);
    _allPaymentsModel = ls;
    gtk_widget_set_vexpand(grid, TRUE);

    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrollWindow), grid);
    gtk_widget_set_vexpand(scrollWindow, TRUE);
    
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(grid), GTK_TREE_VIEW_GRID_LINES_BOTH);

    // context menu trigger
    g_signal_connect(grid, "button_press_event", G_CALLBACK(LendingLoopInformation::ContextMenu_Show), this);
    

    // blanks navigation
    _btnFirstBlank = gtk_button_new_with_label("First Blank Payment");
    gtk_widget_set_name(_btnFirstBlank, "first");
    setWidgetMargins(_btnFirstBlank, 0, 5, 5, 5);
    g_signal_connect(_btnFirstBlank, "clicked", G_CALLBACK(LendingLoopInformation::BlankNavigation_Clicked), this);

    _btnLastBlank = gtk_button_new_with_label("Last Blank Payment");
    gtk_widget_set_name(_btnLastBlank, "last");
    setWidgetMargins(_btnLastBlank, 0, 5, 5, 5);
    g_signal_connect(_btnLastBlank, "clicked", G_CALLBACK(LendingLoopInformation::BlankNavigation_Clicked), this);

    _btnNextBlank = gtk_button_new_with_label("Next >");
    gtk_widget_set_name(_btnNextBlank, "next");
    setWidgetMargins(_btnNextBlank, 0, 5, 5, 5);
    g_signal_connect(_btnNextBlank, "clicked", G_CALLBACK(LendingLoopInformation::BlankNavigation_Clicked), this);
    gtk_widget_set_sensitive(_btnNextBlank, FALSE);

    _btnPrevBlank = gtk_button_new_with_label("< Previous");
    gtk_widget_set_name(_btnPrevBlank, "prev");
    setWidgetMargins(_btnPrevBlank, 0, 5, 5, 5);
    g_signal_connect(_btnPrevBlank, "clicked", G_CALLBACK(LendingLoopInformation::BlankNavigation_Clicked), this);
    gtk_widget_set_sensitive(_btnPrevBlank, FALSE);

    GtkWidget* buttonStackPanel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), _btnFirstBlank, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), _btnPrevBlank, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), _btnNextBlank, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttonStackPanel), _btnLastBlank, FALSE, FALSE, 0);
    gtk_widget_set_halign(buttonStackPanel, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(buttonStackPanel, TRUE);
    
    _allPaymentsGrid = grid;
    GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(grid));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_MULTIPLE);
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(grid), TRUE);
    g_signal_connect(select, "changed", G_CALLBACK(LendingLoopInformation::GridSelectionChanged), this);
    

    GtkWidget* allPayGrid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(allPayGrid), buttonStackPanel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(allPayGrid), scrollWindow, 0, 1, 1, 1);
    gtk_widget_set_hexpand(allPayGrid, true);
    GtkWidget* tabLabel = gtk_label_new("All");
    gtk_notebook_append_page(GTK_NOTEBOOK(_tabs), allPayGrid, tabLabel);

    g_object_set_data(G_OBJECT(grid), "displayLines", &_displayedLines);
    g_object_set_data(G_OBJECT(grid), "selectedIndices", &_allPaymentsGridSelectedIndices);
    g_object_set_data(G_OBJECT(grid), "useFilterMenu", _TrueVal);
    g_object_set_data(G_OBJECT(grid), "useLoanSummaryMenu", _TrueVal);
    gint column = 1;
    if (_manipulator->isEntityType())
        column = 2;
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(grid), column);
    
}

void LendingLoopInformation::buildAllPaymentsUnpaid(){
    GtkListStore* ls;
    GtkTreeIter iter;
    
    vector<LineItem> lineItems = _manipulator->FilterResults(LineItem::PaymentStatus::Unpaid);
    vector<DisplayLineItem>& lines = _displayedLinesUnpaid;
    for (auto const& l : lineItems){
        lines.push_back(DisplayLineItem(l));
        lines.push_back(DisplayLineItem());
    }

    //https://en.wikibooks.org/wiki/GTK%2B_By_Example/Tree_View/Tree_Models
    if (_manipulator->isCompanyType()){
        ls  = gtk_list_store_new(13, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : lines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.LoanId().c_str(),
                        2, x.Company().c_str(), 3, x.LoanName().c_str(),
                        4, x.InterestRate().c_str(), 5, x.RiskBand().c_str(),
                        6, x.Interest().c_str(), 7, x.Principal().c_str(),
                        8, x.Total().c_str(), 9, x.LoopFee().c_str(),
                        10, x.DueDate().c_str(), 11, x.DatePaid().c_str(),
                        12, x.Status().c_str(), -1);

        }
    }
    else if (_manipulator->isEntityType()){
        ls  = gtk_list_store_new(12, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : lines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.EntityId().c_str(),
                        2, x.LoanId().c_str(), 3, x.InterestRate().c_str(), 4, x.RiskBand().c_str(),
                        5, x.Interest().c_str(), 6, x.Principal().c_str(),
                        7, x.Total().c_str(), 8, x.LoopFee().c_str(),
                        9, x.DueDate().c_str(), 10, x.DatePaid().c_str(),
                        11, x.Status().c_str(), -1);

        }
    }
    else {
        // ??
    }
    
    GtkWidget* grid = gtk_tree_view_new();
    GtkCellRenderer* renderer;

    gint colIdx = 0;

    GtkTreeViewColumn* col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Pay Type");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    if (_manipulator->isEntityType()){
        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Entity Id");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);
    }

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Loan ID");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    if (_manipulator->isCompanyType()){
        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Company");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

        col = gtk_tree_view_column_new();
        gtk_tree_view_column_set_title(col, "Loan Name");
        gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
        renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(col, renderer, TRUE);
        gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);
    }

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest Rate");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Risk Band");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Loop Fee");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 1.0, NULL);
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Due Date");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Date Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Status");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);



    gtk_tree_view_set_model(GTK_TREE_VIEW(grid), GTK_TREE_MODEL(ls));
    g_object_unref(ls);
    gtk_widget_set_vexpand(grid, TRUE);

    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrollWindow), grid);
    gtk_widget_set_vexpand(scrollWindow, TRUE);
    
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(grid), GTK_TREE_VIEW_GRID_LINES_BOTH);

    // context menu trigger
    g_signal_connect(grid, "button_press_event", G_CALLBACK(LendingLoopInformation::ContextMenu_Show), this);
    
 
    GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(grid));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_MULTIPLE);
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(grid), TRUE);
    g_signal_connect(select, "changed", G_CALLBACK(LendingLoopInformation::GridSelectionChanged), this);

    string header = "Unpaid: " + to_string(lineItems.size());
    GtkWidget* tabLabel = gtk_label_new(header.c_str());
    gtk_notebook_append_page(GTK_NOTEBOOK(_tabs), scrollWindow, tabLabel);

    g_object_set_data(G_OBJECT(grid), "displayLines", &_displayedLinesUnpaid);
    g_object_set_data(G_OBJECT(grid), "selectedIndices", &_unpaidPaymentsGridSelectedIndices);
    g_object_set_data(G_OBJECT(grid), "useFilterMenu", _FalseVal);
    g_object_set_data(G_OBJECT(grid), "useLoanSummaryMenu", _TrueVal);
    
}

void LendingLoopInformation::buildLoanSummaryTab(LineItem item){
    
    GtkWidget* stackPanel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    string tabHeader = to_string(item.LoanId()) + string(" Summary");
    GtkWidget* label = gtk_label_new(tabHeader.c_str());
    GtkWidget* button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_widget_set_focus_on_click(button, FALSE);

    GtkWidget* image = gtk_image_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);
    gtk_container_add(GTK_CONTAINER(button), image);

    GtkWidget* eventBox = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(eventBox), label);

    gtk_box_pack_start(GTK_BOX(stackPanel), eventBox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(stackPanel), button, FALSE, FALSE, 0);
    
    g_signal_connect(stackPanel, "button_press_event", G_CALLBACK(LendingLoopInformation::Tab_Click), this);
    g_signal_connect(eventBox, "button_press_event", G_CALLBACK(LendingLoopInformation::Tab_Click), this);
    g_signal_connect(button, "clicked", G_CALLBACK(LendingLoopInformation::CloseButton_Click), this);
    gtk_widget_show_all(stackPanel);
    GtkWidget* content = buildLoanSummaryContent(item);
    gint position = gtk_notebook_append_page(GTK_NOTEBOOK(_tabs), content, stackPanel);
    gtk_widget_show_all(_tabs);
    g_object_set_data(G_OBJECT(button), "tab", content);
    g_object_set_data(G_OBJECT(eventBox), "tab", content);
    g_object_set_data(G_OBJECT(stackPanel), "tab", content);
    gtk_notebook_set_current_page(GTK_NOTEBOOK(_tabs), position);

}

GtkWidget* LendingLoopInformation::buildLoanSummaryContent(LineItem const& item){
    GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    vector<LineItem> loanDetails = Utility::Where(_manipulator->Lines(), 
        [item](LineItem const& li)->bool{
            return li.LoanId() == item.LoanId();
    });

    GtkWidget* loanSummaryGrid = gtk_grid_new();

    GtkWidget* lblLoanId = gtk_label_new("Loan ID: ");
    GtkWidget* lblRisk = gtk_label_new("Risk Band: ");
    GtkWidget* lblIntRate = gtk_label_new("Interest Rate: ");
    GtkWidget* lblLoanTerm = gtk_label_new("Loan Term: ");
    GtkWidget* lblCompany = gtk_label_new("Company: ");
    GtkWidget* lblLoanName = gtk_label_new("Loan Name: ");

    GtkWidget* lblLoanId_ = gtk_label_new(to_string(item.LoanId()).c_str());
    setLabelMarkup(lblLoanId_, true);

    GtkWidget* lblRisk_ = gtk_label_new(item.RiskBand().c_str());
    setLabelMarkup(lblRisk_, true);
    string intRate = Utility::format(item.InterestRate(), false);
    if (intRate.back() == '0')
        intRate.replace(intRate.size() - 1, 1, "");
    intRate += "%";
    GtkWidget* lblIntRate_ = gtk_label_new(intRate.c_str());
    setLabelMarkup(lblIntRate_, true);

    GtkWidget* lblLoanTerm_ = gtk_label_new((to_string(loanDetails.size()) + string(" Months")).c_str());
    setLabelMarkup(lblLoanTerm_, true);
    GtkWidget* lblCompany_ = gtk_label_new(item.Company().c_str());
    GtkWidget* lblLoanName_ = gtk_label_new(item.LoanName().c_str());

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(box), lblLoanId, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblLoanId_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblRisk, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblRisk_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblIntRate, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblIntRate_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblLoanTerm, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), lblLoanTerm_, FALSE, FALSE, 0);

    GtkWidget* box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(box2), lblCompany, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box2), lblCompany_, FALSE, FALSE, 0);

    GtkWidget* box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(box3), lblLoanName, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box3), lblLoanName_, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(loanSummaryGrid), box, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(loanSummaryGrid), box2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(loanSummaryGrid), box3, 0, 2, 1, 1);
    
    gtk_widget_set_halign(lblRisk, GTK_ALIGN_START);
    gtk_widget_set_halign(lblIntRate, GTK_ALIGN_START);
    gtk_widget_set_halign(lblLoanTerm, GTK_ALIGN_START);
    gtk_widget_set_halign(lblCompany, GTK_ALIGN_START);
    gtk_widget_set_halign(lblLoanName, GTK_ALIGN_START);
    gtk_widget_set_halign(lblLoanId, GTK_ALIGN_START);
    gtk_widget_set_halign(lblLoanId_, GTK_ALIGN_START);
    gtk_widget_set_halign(lblRisk_, GTK_ALIGN_START);
    gtk_widget_set_halign(lblIntRate_, GTK_ALIGN_START);
    gtk_widget_set_halign(lblLoanTerm_, GTK_ALIGN_START);
    gtk_widget_set_halign(lblCompany_, GTK_ALIGN_START);
    gtk_widget_set_halign(lblLoanName_, GTK_ALIGN_START);

    setWidgetMargins(lblLoanId, 10, 5, 0, 0);
    setWidgetMargins(lblCompany, 10, 5, 0, 0);
    setWidgetMargins(lblLoanName, 10, 5, 0, 0);
    setWidgetMargins(lblRisk, 20, 5, 0, 0);
    setWidgetMargins(lblIntRate, 20, 5, 0, 0);
    setWidgetMargins(lblLoanTerm, 20, 5, 0, 0);
    setWidgetMargins(lblLoanId_, 0, 5, 0, 0);
    setWidgetMargins(lblRisk_, 0, 5, 0, 0);
    setWidgetMargins(lblIntRate_, 0, 5, 0, 0);
    setWidgetMargins(lblLoanTerm_, 0, 5, 0, 0);
    setWidgetMargins(lblCompany_, 0, 5, 0, 0);
    setWidgetMargins(lblLoanName_, 0, 5, 0, 0);

    GtkWidget* summary = gtk_grid_new();
    GtkWidget* label1 = gtk_label_new("Totals");
    GtkWidget* label2 = gtk_label_new("Interest");
    GtkWidget* label3 = gtk_label_new("Principal");
    GtkWidget* label4 = gtk_label_new("Total");
    GtkWidget* label5 = gtk_label_new("Fee");
    GtkWidget* label6 = gtk_label_new("Interest Minus Fee");
    GtkWidget* label7 = gtk_label_new("Outstanding Principal");

    double interest = 0.0;
    double principal = 0.0;
    double total = 0.0;
    double fee = 0.0;
    double intMinusFee = 0.0;
    double principalPaid = 0.0;

    unsigned idx = 1;
    vector<DisplayLineItem> lines;
    for (auto const& l : loanDetails){
        lines.push_back(DisplayLineItem(l));
        lines.back().PayId = to_string(idx++);
        lines.push_back(DisplayLineItem());
        fee += l.LoopFee();
        interest += l.Interest();
        principal += l.Principal();
        total += l.Total();
        if (l.HasValidDatePaid())
            principalPaid += l.Principal();
    }

    intMinusFee = interest - fee;

    GtkWidget* label8 = gtk_label_new(Utility::format(interest).c_str());
    GtkWidget* label9 = gtk_label_new(Utility::format(principal).c_str());
    GtkWidget* label10 = gtk_label_new(Utility::format(total).c_str());
    GtkWidget* label11 = gtk_label_new(Utility::format(fee).c_str());
    GtkWidget* label12 = gtk_label_new(Utility::format(intMinusFee).c_str());
    GtkWidget* label13 = gtk_label_new(Utility::format(principal - principalPaid).c_str());

    setWidgetMargins(label1, 2, 0, 8, 0);
    setWidgetMargins(label2, 8, 0, 8, 0);
    setWidgetMargins(label3, 8, 0, 8, 0);
    setWidgetMargins(label4, 8, 0, 8, 0);
    setWidgetMargins(label5, 8, 0, 8, 0);
    setWidgetMargins(label6, 8, 0, 8, 0);
    setWidgetMargins(label7, 8, 0, 8, 0);
    setWidgetMargins(label8, 8, 0, 8, 2);
    setWidgetMargins(label9, 8, 0, 8, 2);
    setWidgetMargins(label10, 8, 0, 8, 2);
    setWidgetMargins(label11, 8, 0, 8, 2);
    setWidgetMargins(label12, 8, 0, 8, 2);
    setWidgetMargins(label13, 8, 0, 8, 2);

    setLabelMarkup(label8, true);
    setLabelMarkup(label9, true);
    setLabelMarkup(label10, true);
    setLabelMarkup(label11, true);
    setLabelMarkup(label12, true);
    setLabelMarkup(label13, true);
    
    GtkWidget* frame1 = gtk_frame_new(NULL);
    GtkWidget* frame2 = gtk_frame_new(NULL);
    GtkWidget* frame3 = gtk_frame_new(NULL);
    GtkWidget* frame4 = gtk_frame_new(NULL);
    GtkWidget* frame5 = gtk_frame_new(NULL);
    GtkWidget* frame6 = gtk_frame_new(NULL);
    GtkWidget* frame7 = gtk_frame_new(NULL);
    GtkWidget* frame8 = gtk_frame_new(NULL);
    GtkWidget* frame9 = gtk_frame_new(NULL);
    GtkWidget* frame10 = gtk_frame_new(NULL);
    GtkWidget* frame11 = gtk_frame_new(NULL);
    GtkWidget* frame12 = gtk_frame_new(NULL);
    GtkWidget* frame13 = gtk_frame_new(NULL);
    GtkWidget* frame14 = gtk_frame_new(NULL);

    gtk_container_add(GTK_CONTAINER(frame1), label1);
    gtk_container_add(GTK_CONTAINER(frame2), label2);
    gtk_container_add(GTK_CONTAINER(frame3), label3);
    gtk_container_add(GTK_CONTAINER(frame4), label4);
    gtk_container_add(GTK_CONTAINER(frame5), label5);
    gtk_container_add(GTK_CONTAINER(frame6), label6);
    gtk_container_add(GTK_CONTAINER(frame7), label7);
    gtk_container_add(GTK_CONTAINER(frame9), label8);
    gtk_container_add(GTK_CONTAINER(frame10), label9);
    gtk_container_add(GTK_CONTAINER(frame11), label10);
    gtk_container_add(GTK_CONTAINER(frame12), label11);
    gtk_container_add(GTK_CONTAINER(frame13), label12);
    gtk_container_add(GTK_CONTAINER(frame14), label13);

    // all this is needed to draw the border around each cell in the grid
    // https://stackoverflow.com/questions/13001990/why-is-there-no-a-border-on-this-gtkframe
    GtkCssProvider *provider;
    GdkScreen *screen;
    GdkDisplay *display;

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider),GTK_STYLE_PROVIDER_PRIORITY_USER);
    GError *error = NULL;
    string css = "frame#all {border-style:solid; border-width: 1px; border-color: black;}";
    css += " frame#threeSides {border-style:solid; border-width: 1px 1px 1px 0px; border-color: black;}";
    css += " frame#bottomThreeSides {border-style:solid; border-width: 0px 1px 1px 1px; border-color: black;}";
    css += " frame#twoSides {border-style:solid; border-width: 0px 1px 1px 0px; border-color: black;}";
    gtk_css_provider_load_from_data(provider, css.c_str(), -1, &error);
    if (error)
        cout << "Error! " << error->message << endl;
    g_object_unref(provider);

    gtk_widget_set_name(frame1, "all");
    gtk_widget_set_name(frame2, "threeSides");
    gtk_widget_set_name(frame3, "threeSides");
    gtk_widget_set_name(frame4, "threeSides");
    gtk_widget_set_name(frame5, "threeSides");
    gtk_widget_set_name(frame6, "threeSides");
    gtk_widget_set_name(frame7, "threeSides");
    gtk_widget_set_name(frame8, "bottomThreeSides");
    gtk_widget_set_name(frame9, "twoSides");
    gtk_widget_set_name(frame10, "twoSides");
    gtk_widget_set_name(frame11, "twoSides");
    gtk_widget_set_name(frame12, "twoSides");
    gtk_widget_set_name(frame13, "twoSides");
    gtk_widget_set_name(frame14, "twoSides");



    gtk_grid_attach(GTK_GRID(summary), frame1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame4, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame5, 4, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame6, 5, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame7, 6, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame8, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame9, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame10, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame11, 3, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame12, 4, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame13, 5, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(summary), frame14, 6, 1, 1, 1);
    
    setWidgetMargins(summary, 10, 10, 0, 0);
    gtk_grid_attach(GTK_GRID(loanSummaryGrid), summary, 0, 3, 1, 1);

    gtk_grid_set_column_homogeneous(GTK_GRID(loanSummaryGrid), FALSE);

    gtk_box_pack_start(GTK_BOX(mainBox), loanSummaryGrid, FALSE, FALSE, 0);


    GtkListStore* ls;
    GtkTreeIter iter;

    ls  = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    for (auto& x : lines){
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter, 0, x.PayId.c_str(), 1, x.Interest().c_str(),
                    2, x.Principal().c_str(), 3, x.Total().c_str(),
                    4, x.LoopFee().c_str(), 5, x.DueDate().c_str(),
                    6, x.DatePaid().c_str(), 7, x.Status().c_str(), -1);
    }
    
    GtkWidget* grid = gtk_tree_view_new();
    GtkCellRenderer* renderer;

    gint colIdx = 0;

    GtkTreeViewColumn* col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Pay Id");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Interest");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Principal");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Total");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Loop Fee");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Due Date");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Date Paid");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Status");
    gtk_tree_view_append_column(GTK_TREE_VIEW(grid), col);
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_add_attribute(col, renderer, "text", colIdx++);

    gtk_tree_view_set_model(GTK_TREE_VIEW(grid), GTK_TREE_MODEL(ls));
    g_object_unref(ls);
    
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(grid));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
    gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(grid), TRUE);
    g_signal_connect(selection, "changed", G_CALLBACK(LendingLoopInformation::GridSelectionChanged), this);

    vector<DisplayLineItem>* ptrLines = new vector<DisplayLineItem>();
    *ptrLines = lines;
    _loanSummaryTabActiveGridLines.push_back(ptrLines);
    _loanSummaryTabActiveGridSelectedIndices.push_back(new vector<int>());
    g_object_set_data(G_OBJECT(grid), "displayLines", ptrLines);
    g_object_set_data(G_OBJECT(grid), "selectedIndices", _loanSummaryTabActiveGridSelectedIndices.back());
    g_object_set_data(G_OBJECT(grid), "useFilterMenu", _FalseVal);
    g_object_set_data(G_OBJECT(grid), "useLoanSummaryMenu", _FalseVal);

    // context menu trigger
    g_signal_connect(grid, "button_press_event", G_CALLBACK(LendingLoopInformation::ContextMenu_Show), this);
    setWidgetMargins(grid, 10, 0, 0, 0);

    GtkWidget* scrollWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrollWindow), grid);
    gtk_widget_set_vexpand(scrollWindow, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);
    gtk_widget_set_hexpand(scrollWindow, FALSE);
    gtk_widget_set_hexpand(grid, FALSE);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(grid), GTK_TREE_VIEW_GRID_LINES_BOTH);
    
    gtk_box_pack_start(GTK_BOX(mainBox), scrollWindow, TRUE, TRUE, 10);
    gtk_widget_show_all(mainBox);

    g_object_set_data(G_OBJECT(mainBox), "treeView", grid);
    return mainBox;

}

void LendingLoopInformation::clearTabsExceptSummary(){
    // summary tab must always be page 0
    while (gtk_notebook_get_n_pages(GTK_NOTEBOOK(_tabs)) > 1)
        gtk_notebook_remove_page(GTK_NOTEBOOK(_tabs), -1);

    for (auto& x : _loanSummaryTabActiveGridLines)
        delete x;

    for (auto& x : _loanSummaryTabActiveGridSelectedIndices)
        delete x;

    _loanSummaryTabActiveGridLines.clear();
    _loanSummaryTabActiveGridSelectedIndices.clear();
}

void LendingLoopInformation::BlankNavigation_Clicked(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    string btnName(gtk_widget_get_name(widget));

    if (lli->_displayedLines.size() == 0)
        return;

    int idx = lli->_displayedLines.size() - 2;
    int idxOfFirst = lli->_displayedLines.IndexOfFirstBlankPayment();
    int idxOfLast = lli->_displayedLines.IndexOfLastBlankPayment();
    vector<int> selectedIndices = LendingLoopInformation::GetGridSelectedIndices(G_OBJECT(lli->_allPaymentsGrid));
    GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(lli->_allPaymentsGrid));
    int selection = (selectedIndices.size() == 0 ? -1 : selectedIndices[0]);
    
 
    if (btnName == "first"){
        idx = idxOfFirst;
    }
    else if (btnName == "next"){
        idx = lli->_displayedLines.IndexOfNextBlankPayment(selection, idxOfLast);
    }
    else if (btnName == "prev"){
        if (selection > idxOfLast)
            idx = idxOfLast;
        else
            idx = lli->_displayedLines.IndexOfPreviousBlankPayment(selection, idxOfFirst);
    }
    else if (btnName == "last"){
        idx = idxOfLast;
    }
    else{
        // ?
    }

    if (idx == -1)
        idx = 0;
    
    string strPath = to_string(idx);
    GtkTreePath* path = gtk_tree_path_new_from_string(strPath.c_str());

    // determine if blank is already in view, scroll to if not
    GtkTreePath* startPath;
    GtkTreePath* endPath;
    gtk_tree_view_get_visible_range(GTK_TREE_VIEW(lli->_allPaymentsGrid), &startPath, &endPath);

    gint startDepth = gtk_tree_path_get_depth(startPath);
    gint endDepth = gtk_tree_path_get_depth(endPath);
    gint startIndex = -1;
    gint endIndex = -1;

    if (startDepth > 0)
        startIndex = gtk_tree_path_get_indices(startPath)[0];
    if (endDepth > 0)
        endIndex = gtk_tree_path_get_indices(endPath)[0];

    if (idx < startIndex || idx > endIndex)
        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(lli->_allPaymentsGrid), path, NULL, TRUE, 0.5, 0.5);

    

    gtk_tree_path_free(startPath);
    gtk_tree_path_free(endPath);

    
    gtk_tree_selection_unselect_all(select);
    gtk_tree_selection_select_path(select, path);
    gtk_tree_path_free(path);
}


void LendingLoopInformation::GridSelectionChanged(GtkTreeSelection* treeselection, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    GtkWidget* grid = (GtkWidget*)gtk_tree_selection_get_tree_view(treeselection);
    vector<int>* indices = (vector<int>*)g_object_get_data(G_OBJECT(grid), "selectedIndices");
    
    vector<int> selectedIndices = LendingLoopInformation::GetGridSelectedIndices(G_OBJECT(treeselection));
    
    int selection = -1;
    if (selectedIndices.size()){
        *indices = selectedIndices;
        selection = selectedIndices.back();
    }
    
    if (selection == -1)    // no change
        return;
    
    //lli->_allPaymentsGridSelectedIndex = selection;
    if (lli->_allPaymentsGridSelectedIndices.size() > 0){
        gtk_widget_set_sensitive(lli->_btnNextBlank, lli->_displayedLines.HasNextBlank(lli->_allPaymentsGridSelectedIndices[0]));
        gtk_widget_set_sensitive(lli->_btnPrevBlank, lli->_displayedLines.HasPreviousBlank(lli->_allPaymentsGridSelectedIndices[0]));
    }
}


#pragma region Get All Payments Button Click
void LendingLoopInformation::getAllPaymentsTask(){
    try{
        _threadError = "";
        vector<PulledParts> parts = _loopConn->GetPullData();
        _interimManipulator = LenderManipulator::CreateManipulator(_loopConn->GetAllPayments(), parts);
    }
    catch(char const* msg){
        _threadError = msg;
    }
    catch(...){
        _threadError = "Unknown error occurred during retrieve.";
    }
    
    gdk_threads_add_idle(LendingLoopInformation::GetAllPayments_Finishing, this);
}

void LendingLoopInformation::GetAllPayments_Clicked(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    try{
        lli->updateStatus("Retrieving data...");
        gtk_widget_set_sensitive(lli->_btnGetAllPayments, FALSE);
        gtk_widget_set_sensitive(lli->_btnRefresh, FALSE);
		gtk_widget_set_sensitive(lli->_miLogin, FALSE);
        if (lli->_t.joinable())
            lli->_t.join();
        lli->_t = thread(&LendingLoopInformation::getAllPaymentsTask, lli);
    }
    catch(char const* msg){
        MessageBox::Show(lli->_window, MessageBox::MessageBoxType::WARNING, MessageBox::MessageBoxButtons::OK, msg, "Error");
    }
}   

gboolean LendingLoopInformation::GetAllPayments_Finishing(gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;

    gtk_widget_set_sensitive(lli->_btnGetAllPayments, TRUE);
    gtk_widget_set_sensitive(lli->_btnRefresh, TRUE);
	gtk_widget_set_sensitive(lli->_miLogin, TRUE);
    if (!lli->_threadError.empty()){
        MessageBox::Show(lli->_window, MessageBox::MessageBoxType::WARNING, MessageBox::MessageBoxButtons::OK, lli->_threadError, "Error");
        lli->updateStatus("Retrieve error!");
        return G_SOURCE_REMOVE;
    }

    if (lli->_manipulator)
        delete lli->_manipulator;
    lli->_manipulator = lli->_interimManipulator;
    lli->clearTabsExceptSummary();
    lli->populateTabs();
    

    gtk_widget_set_sensitive(GTK_WIDGET(lli->_miSaveAs), TRUE);
    lli->updateStatus("Loaded!");
    gtk_widget_show_all(GTK_WIDGET(lli->_mainGrid));
    return G_SOURCE_REMOVE;

}
#pragma endregion Get All Payments Button Click

#pragma region Refresh Button Click
void LendingLoopInformation::refreshTask(){
    try{
        _threadError = "";
        _loopConn->Refresh();
    }
    catch(char const* msg){
        _threadError = msg;
    }
    catch(...){
        _threadError = "Unknown error occurred during refresh.";
    }
    
    gdk_threads_add_idle(LendingLoopInformation::Refresh_Finishing, this);
}

gboolean LendingLoopInformation::Refresh_Finishing(gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    
    if (!lli->_threadError.empty()){
        MessageBox::Show(lli->_window, MessageBox::MessageBoxType::WARNING, MessageBox::MessageBoxButtons::OK, lli->_threadError, "Error");
        lli->updateStatus("Refresh error!");
        return G_SOURCE_REMOVE;
    }

    gtk_notebook_remove_page(GTK_NOTEBOOK(lli->_tabs), 0);
    lli->buildOnlineSummary();
    gtk_widget_show_all(GTK_WIDGET(lli->_mainGrid));
    lli->updateStatus("Refreshed!");
    return G_SOURCE_REMOVE;
}

void LendingLoopInformation::Refresh_Clicked(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    try{
        lli->updateStatus("Reloading...");
        gtk_widget_set_sensitive(lli->_btnGetAllPayments, FALSE);
        gtk_widget_set_sensitive(lli->_btnRefresh, FALSE);
        if (lli->_t.joinable())
            lli->_t.join();
        lli->_t = thread(&LendingLoopInformation::refreshTask, lli);
    }
    catch(const char* msg){
        MessageBox::Show(lli->_window, MessageBox::MessageBoxType::WARNING, MessageBox::MessageBoxButtons::OK, msg, "Error");
    }
}
#pragma endregion Refresh Button Click

#pragma region All Payments Context Menu

gint LendingLoopInformation::ContextMenu_Show(GtkWidget* widget, GdkEvent* event, gpointer data){
    GdkEventButton* event_button;
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    vector<int> indices = LendingLoopInformation::GetGridSelectedIndices(G_OBJECT(widget));
    
    int selection = (indices.size() == 0 ? -1 : indices[0]);
    
    if (event->type == GDK_BUTTON_PRESS){
        event_button = (GdkEventButton*) event;
        if (event_button->button == GDK_BUTTON_SECONDARY){
            lli->_activeGridLines = (vector<DisplayLineItem>*)g_object_get_data(G_OBJECT(widget), "displayLines");
            lli->_activeGridSelectedIndices = (vector<int>*)g_object_get_data(G_OBJECT(widget), "selectedIndices");
            lli->_cm->ShowAllChildren();
            if (selection == -1 || !(*(bool*)g_object_get_data(G_OBJECT(widget), "useLoanSummaryMenu")) || lli->_activeGridSelectedIndices->size() > 1 || (lli->_activeGridSelectedIndices->size() == 1 && (*lli->_activeGridLines)[(*lli->_activeGridSelectedIndices)[0]].isBlank()))
                lli->_cm->HideChild(1);
            if (selection == -1 || (lli->_activeGridSelectedIndices->size() == 1 && (*lli->_activeGridLines)[(*lli->_activeGridSelectedIndices)[0]].isBlank()))
                lli->_cm->HideChild(0);
            if (!(*(bool*)g_object_get_data(G_OBJECT(widget), "useFilterMenu")))
                lli->_cm->HideChild(2);
            lli->_cm->Show();
            return TRUE;
        }
    }

    return FALSE;
}

void LendingLoopInformation::miSummaryPopUp(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    vector<DisplayLineItem> lines;
    for (int line : *lli->_activeGridSelectedIndices){
        lines.push_back((*lli->_activeGridLines)[line]);
    }
    lli->_popSum->Display(lines);
}

void LendingLoopInformation::miTodaysDate(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    DateTime dt = DateTime::Now();
    LendingLoopInformation::calendarCallback(dt, DateTime(), false, data);
}

void LendingLoopInformation::miYesterdaysDate(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    DateTime dt = --DateTime::Now();
    LendingLoopInformation::calendarCallback(dt, DateTime(), false, data);
}

void LendingLoopInformation::miCalendarPopUp(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    lli->_calPop->ShowDialog();
}

void LendingLoopInformation::calendarCallback(DateTime dt, DateTime dt2, bool isRange, void* data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;

    //create date vector
    vector<DateTime> dates;
    if (isRange)
        dates = DateTime::GetDateRange(dt, dt2);
    else
        dates.push_back(dt);

    // get items from manipulator
    vector<LineItem> lines = lli->_manipulator->FilterResults(dates);
    
    // for (auto& d : dates){
    //     g_print("date = %s\n", Utility::DateToString(d).c_str());
    // }
    // display
    GtkListStore* ls;
    GtkTreeIter iter;
    
    lli->_displayedLines.clear();
    for (auto const& l : lines){
        lli->_displayedLines.push_back(DisplayLineItem(l));
        lli->_displayedLines.push_back(DisplayLineItem());
    }
    lli->_allPaymentsGridSelectedIndices.clear();
    //g_print("lines.size() = %d\n", lines.size());
    //https://en.wikibooks.org/wiki/GTK%2B_By_Example/Tree_View/Tree_Models
    if (lli->_manipulator->isCompanyType()){
        ls  = gtk_list_store_new(13, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : lli->_displayedLines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.LoanId().c_str(),
                        2, x.Company().c_str(), 3, x.LoanName().c_str(),
                        4, x.InterestRate().c_str(), 5, x.RiskBand().c_str(),
                        6, x.Interest().c_str(), 7, x.Principal().c_str(),
                        8, x.Total().c_str(), 9, x.LoopFee().c_str(),
                        10, x.DueDate().c_str(), 11, x.DatePaid().c_str(),
                        12, x.Status().c_str(), -1);

        }
    }
    else if (lli->_manipulator->isEntityType()){
        ls  = gtk_list_store_new(12, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : lli->_displayedLines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.EntityId().c_str(),
                        2, x.LoanId().c_str(), 3, x.InterestRate().c_str(), 4, x.RiskBand().c_str(),
                        5, x.Interest().c_str(), 6, x.Principal().c_str(),
                        7, x.Total().c_str(), 8, x.LoopFee().c_str(),
                        9, x.DueDate().c_str(), 10, x.DatePaid().c_str(),
                        11, x.Status().c_str(), -1);

        }
    }
    else {
        // ??
    }

    lli->clearAllPaymentsGridSelection();

    gtk_tree_view_set_model(GTK_TREE_VIEW(lli->_allPaymentsGrid), GTK_TREE_MODEL(ls));
    gint column = 1;
    if (lli->_manipulator->isEntityType())
        column = 2;
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(lli->_allPaymentsGrid), column);
    g_object_unref(ls);
    // ensure all filter options are unchecked
    lli->_suppressEvent = true;
    for (auto x : lli->_filters)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(x), FALSE);
    lli->_suppressEvent = false;
}

void LendingLoopInformation::miAllFilter_Click(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;

    lli->_suppressEvent = true;
    for (auto x : lli->_filters)
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(x), TRUE);
    lli->_suppressEvent = false;
    
    lli->_displayedLines = lli->_allPaymentsLines;

    lli->clearAllPaymentsGridSelection();

    gtk_tree_view_set_model(GTK_TREE_VIEW(lli->_allPaymentsGrid), GTK_TREE_MODEL(lli->_allPaymentsModel));
    gint column = 1;
    if (lli->_manipulator->isEntityType())
        column = 2;
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(lli->_allPaymentsGrid), column);

}

void LendingLoopInformation::miFilter_Click(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    
    if (lli->_suppressEvent)
        return;

    vector<LineItem::PaymentStatus> statuses;

    for (auto x : lli->_filters){
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(x)))
            statuses.push_back((*(LineItem::PaymentStatus*)g_object_get_data(G_OBJECT(x), "PaymentStatus")));
    }

    // get items from manipulator
    vector<LineItem> lines = lli->_manipulator->FilterResults(statuses);
    
    // display
    GtkListStore* ls;
    GtkTreeIter iter;
    
    lli->_displayedLines.clear();
    for (auto const& l : lines){
        lli->_displayedLines.push_back(DisplayLineItem(l));
        lli->_displayedLines.push_back(DisplayLineItem());
    }
    lli->_allPaymentsGridSelectedIndices.clear();
    
    //https://en.wikibooks.org/wiki/GTK%2B_By_Example/Tree_View/Tree_Models
    if (lli->_manipulator->isCompanyType()){
        ls  = gtk_list_store_new(13, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : lli->_displayedLines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.LoanId().c_str(),
                        2, x.Company().c_str(), 3, x.LoanName().c_str(),
                        4, x.InterestRate().c_str(), 5, x.RiskBand().c_str(),
                        6, x.Interest().c_str(), 7, x.Principal().c_str(),
                        8, x.Total().c_str(), 9, x.LoopFee().c_str(),
                        10, x.DueDate().c_str(), 11, x.DatePaid().c_str(),
                        12, x.Status().c_str(), -1);

        }
    }
    else if (lli->_manipulator->isEntityType()){
        ls  = gtk_list_store_new(12, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        for (auto& x : lli->_displayedLines){
            gtk_list_store_append(ls, &iter);
            gtk_list_store_set(ls, &iter, 0, x.PayType().c_str(), 1, x.EntityId().c_str(),
                        2, x.LoanId().c_str(), 3, x.InterestRate().c_str(), 4, x.RiskBand().c_str(),
                        5, x.Interest().c_str(), 6, x.Principal().c_str(),
                        7, x.Total().c_str(), 8, x.LoopFee().c_str(),
                        9, x.DueDate().c_str(), 10, x.DatePaid().c_str(),
                        11, x.Status().c_str(), -1);

        }
    }
    else {
        // ??
    }

    lli->clearAllPaymentsGridSelection();

    gtk_tree_view_set_model(GTK_TREE_VIEW(lli->_allPaymentsGrid), GTK_TREE_MODEL(ls));
    gint column = 1;
    if (lli->_manipulator->isEntityType())
        column = 2;
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(lli->_allPaymentsGrid), column);
    g_object_unref(ls);
}

void LendingLoopInformation::miLoanSummary_Click(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    lli->buildLoanSummaryTab((*lli->_activeGridLines)[(*lli->_activeGridSelectedIndices)[0]].GetLineItem());
}

#pragma endregion All Payments Context Menu

gint LendingLoopInformation::Tab_Click(GtkWidget* widget, GdkEvent* event, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    GdkEventButton* event_button;

    if (event->type == GDK_BUTTON_PRESS){
        event_button = (GdkEventButton*) event;
        if (event_button->button == GDK_BUTTON_MIDDLE){
            lli->destroyTab(widget);
            return TRUE;
        }
    }

    return FALSE;
}

void LendingLoopInformation::CloseButton_Click(GtkWidget* widget, gpointer data){
    LendingLoopInformation* lli = (LendingLoopInformation*)data;
    lli->destroyTab(widget);
}

void LendingLoopInformation::destroyTab(GtkWidget* widget){
    GtkWidget* tab = (GtkWidget*)g_object_get_data(G_OBJECT(widget), "tab");

    GtkWidget* treeView = (GtkWidget*)g_object_get_data(G_OBJECT(tab), "treeView");

    vector<DisplayLineItem>* treeViewLines = (vector<DisplayLineItem>*)g_object_get_data(G_OBJECT(treeView), "displayLines");
    vector<int>* treeViewIndices = (vector<int>*)g_object_get_data(G_OBJECT(treeView), "selectedIndices");

    vector<vector<DisplayLineItem>*>::iterator it = find(_loanSummaryTabActiveGridLines.begin(), _loanSummaryTabActiveGridLines.end(), treeViewLines);
    if (it != _loanSummaryTabActiveGridLines.end())
        _loanSummaryTabActiveGridLines.erase(it);
    
    vector<vector<int>*>::iterator it2 = find(_loanSummaryTabActiveGridSelectedIndices.begin(), _loanSummaryTabActiveGridSelectedIndices.end(), treeViewIndices);
    if (it2 != _loanSummaryTabActiveGridSelectedIndices.end())
        _loanSummaryTabActiveGridSelectedIndices.erase(it2);

    delete treeViewLines;
    delete treeViewIndices;

    gint position = gtk_notebook_page_num(GTK_NOTEBOOK(_tabs), tab);
    gtk_notebook_remove_page(GTK_NOTEBOOK(_tabs), position);
}