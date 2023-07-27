/**
 ******************************************************************
 *
 * Module Name : IVcurve.cpp
 *
 * Author/Date : C.B. Lirakis / 22-Nov-18
 *
 * Description : Run and plot the IV curves. This is mainly motivated
 *               by choosing the best possible 1N34 diode for a 
 *               crystal radio! ;-)
 *
 * 24-Jul-23   CBL   Modified to include external setup parameters 
 *                   (TENV)
 *                   Starting to setup as a true voltage or current
 *                   source. 
 *                   Also store the resistor used. If this is zero
 *                   then we are using a voltage source
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
// System includes.
#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <csignal>
#include <list>
#include <fstream>

/// Root Includes
#include <TROOT.h>
#include <TGWindow.h>
#include <TGToolBar.h>
#include <TGStatusBar.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TAxis.h>
#include <TGraph.h>
#include <TGButton.h>
#include <TGMenu.h>
#include <TGFileDialog.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TFile.h>
#include <TRootHelpDialog.h>
#include <TSystem.h>
#include <TGLabel.h>
#include <TColor.h>
#include <TF1.h>
#include <TLatex.h>
#include <TEnv.h>
#include <TTimer.h>

// Local Includes
#include "debug.h"
#include "IVcurve.hh"
#include "Instruments.hh"
#include "CLogger.hh"
#include "ParamDialog.hh"
#include "CommentDialog.hh"

// Setup print queues. Way out of date!
const char *PrintPrg[]  = {"/usr/bin/lpr","/usr/bin/lp"};
const char *printName[] = {"zevonlaser","elvis"};

// File types supported for save and load. 
const char *filetypes[] = { 
    "CSV files",               "*.csv",
    "tab files",               "*.tsv",
    "space delimited files",   "*.txt",
    "All files",     "*",
    "ROOT files",    "*.root",
    0,               0 };

// Save as file types
static const char *SPSaveTypes[] = {
    "PostScript",   "*.ps",
    "Encapsulated PostScript", "*.eps",
    "PDF",          "*.pdf",
    "PNG",          "*.png",
    "JPG",          "*.jpg",
    0,              0 };

/*
 * Enumerations for menu and other buttons. 
 */
enum SVPCommandIdentifiers {
   M_FILE_EXIT=100,
   M_FILE_LOAD,
   M_FILE_SAVE,
   M_FILE_SAVEAS,
   M_FILE_PRINT,
   M_EDIT_PARAMETERS,
   M_HELP_ABOUT,
   M_ZOOM_PLUS,
   M_ZOOM_MINUS,
   M_ZOOM_SELECTED,
   M_START,
   M_STOP,
   M_INST_K196,
   M_INST_K230,
   M_INST_FIT,
   M_INST_COMMENT,
};

/*
 * Toolbar icons to be used. 
 * FIXME
 */
const char *dialog_xpm_names[] = {
    "stop.png",
    "replay.png",
    "bld_save.png",
    "bld_plus.png",
    "bld_new.png",
    "selection_t.xpm",
    0
};


/* 
 * ToolBarData fields.
 * pixmap, TipText, stay_down, ID, TGButton
 *
 */
ToolBarData_t toolbar_data[] = {
  { "",          "Stop",    kFALSE,          M_STOP, NULL },
  { "",           "Run",    kFALSE,         M_START, NULL },
  { "",          "Save",     kTRUE,     M_FILE_SAVE, NULL },
  { "",          "Zoom",     kTRUE,     M_ZOOM_PLUS, NULL },
  { "",        "UnZoom",         0,    M_ZOOM_MINUS, NULL },
  { "", "Zoom Selected",         0, M_ZOOM_SELECTED, NULL },
  { "",            NULL,         0,               0, NULL }
};


static const char *HelpText1 = 
    "This dialog allows plotting of the traces from running an IV-curve\n"\
    "A Keithly 196 multimeter and 230 voltage source are used.\n"\
    "This can be run in at least two configurations.\n"\
    "1) a direct voltage source (R=0) or\n"\
    "using the voltage source with R>0 to make a current source. \n"\
    " In that case, R should be set to the measured value of the device.\n";


/**
 ******************************************************************
 *
 * Function Name : IVCurve
 *
 * Description : Create a cern root frame from which everything hangs. 
 *
 * Inputs : 
 *         p - parent frame/window
 *         w - width in pixels
 *         h - height in pixels
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
IVCurve::IVCurve(const TGWindow *p, UInt_t w, UInt_t h) : 
    TGMainFrame( p, w, h,  kVerticalFrame)
{
    SET_DEBUG_STACK;
    ReadConfiguration();

    Connect("CloseWindow()", "IVCurve" , this, "CloseWindow()");

    CreateMenuBar();
    CreateToolBar();
    AddEmbeddedCanvas(w, h);
    AddStatusPane();
    CreateStatusBar();

    MapSubwindows();
    Resize();
    SetWindowName("Raw Display");
    MapWindow();
    Move(10,10);
    PlotState    = PLOT_STATE_NORMAL;
    X            = Y = 0.0;
    fCurrentFile = 0;
    fLastDir     = new TString(".");

    fGraph       = NULL;
    fZoomLevel   = 2;
    fTakeData    = kFALSE;

    CreateGraphObjects();

    CreateFitFunction();

    // Check instrument status
    CheckInstrumentStatus();
    // Last thing setup a timeout to run the process. 
    fTimer = new TTimer();
    // Set it up to call PlotTimeoutProcedure once per second.
    fTimer->Connect("Timeout()", "IVCurve", this, "TimeoutProc()");
    CLogger::GetThis()->LogData("# IVcurve Timeout started.\n");
    //fTimer->Start(500, kFALSE);

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : IVCurve Destructor
 *
 * Description : clean up any resources we have allocated. 
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
* 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
IVCurve::~IVCurve(void)
{
    SET_DEBUG_STACK;
    CleanUp();
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : CleanUp
 *
 * Description : clean up any resources we have allocated. 
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
* 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CleanUp(void)
{
    SET_DEBUG_STACK;
    WriteConfiguration();
    if (fCurrentFile)
    {
	delete fCurrentFile;
	fCurrentFile = 0;
    }
    delete fLastDir;
    fLastDir = 0;
    delete fInstruments;
    fInstruments = 0;
    delete fGraph;
    fGraph = 0;
    delete fComment;
    fComment = 0;

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : AddEmbeddedCanvas
 *
 * Description : 
 *
 * Inputs : w - width of canvas
 *          h - height of canvas
 *
 * Returns : none
 *
 * Error Conditions :
 * 
 * Unit Tested on: 24-Apr-08
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::AddEmbeddedCanvas(UInt_t w, UInt_t h)
{
    SET_DEBUG_STACK;
    /*
     * The remaining space I want to be a frame. 
     * Inside this frame I will embed a canvas for
     * drawing the profile. 
     */

    TGHorizontalFrame *fGraphicsFrame = new TGHorizontalFrame(this, 600, 600);

    fEmbeddedCanvas = new TRootEmbeddedCanvas("ec1", fGraphicsFrame, w, h);

    TCanvas *c1 = fEmbeddedCanvas->GetCanvas();
    c1->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject *)", 
		"IVCurve", this,
		"ProcessedEvent(Int_t, Int_t, Int_t, TObject *)");
			     

    fGraphicsFrame->SetBackgroundColor(0);
    fGraphicsFrame->AddFrame( fEmbeddedCanvas, 
			      new TGLayoutHints(kLHintsExpandX |
						kLHintsExpandY, 2, 2, 2, 0));
    this->AddFrame( fGraphicsFrame, 
		    new TGLayoutHints(kLHintsExpandX |
				      kLHintsExpandY, 2, 2, 2, 0));
    // Finally put a graph to plot on in this frame.
    
    c1->SetBorderMode(0);
#if 0
    /* 
     * Set NoContextMenu iff 
     *  a) You don't want the user to fool with the axis 
     *     styles, un-zoom...
     *  b) You don't want to be able to edit the points on the graph.
     * This is a property of TObject
     */
    c1->SetBit(kNoContextMenu);
#endif
    c1->SetFillColor(0);
    c1->SetGrid();
    //c1->SetEditable(kFALSE);
    //c1->SetCrosshair(1);
    c1->cd();
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : AddStatusPane
 *
 * Description : create a status pane at the bottom to show if the 
 *               equipment is actually communicating. 
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::AddStatusPane(void)
{
    SET_DEBUG_STACK;
    TGHorizontalFrame* StatusPane = new TGHorizontalFrame(this, 10, 10);
    TGLayoutHints*     L1 = new TGLayoutHints(kLHintsTop|kLHintsLeft|
					      kLHintsExpandX,
					      2, 2, 2, 2);

    // Create the colors for the buttons
    fRedColor = gROOT->GetColor(2);
    fGreenColor = gROOT->GetColor(3);


    fMultimeter = new TGLabel( StatusPane, TGHotString("Keithley 196"));
    fMultimeter->SetTextColor(fRedColor);
    StatusPane->AddFrame(fMultimeter, L1);

    fVoltageSource = new TGLabel( StatusPane, TGHotString("Keithly 230"));
    fVoltageSource->SetTextColor(fRedColor);
    StatusPane->AddFrame(fVoltageSource, L1);

    this->AddFrame(StatusPane, L1);
    
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : CreateMenuBar
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 13-Jul-05
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CreateMenuBar()
{
    SET_DEBUG_STACK;
    TGPopupMenu *MenuFile, *MenuHelp, *MenuEdit;

    // Layout menu here. 
    // Create menubar and popup menus. The hint objects are used to place
    // and group the different menu widgets with respect to eachother.

    // First menu, named file, and only has exit associated with it. 
    // Standard File Menu
    // ---------------------------------------------------------
    MenuFile = new TGPopupMenu(gClient->GetRoot());

    MenuFile->AddEntry("L&oad"  , M_FILE_LOAD);
    MenuFile->AddEntry("S&ave"  , M_FILE_SAVE);
    MenuFile->AddEntry("SaveA&s", M_FILE_SAVEAS);
    MenuFile->AddEntry("P&rint" , M_FILE_PRINT);

    MenuFile->AddSeparator();
    MenuFile->AddEntry("E&xit"  , M_FILE_EXIT);

    /*
     * Disable buttons that are currently non-functional
     */
    MenuFile->DisableEntry( M_FILE_PRINT);

    // Edit menu -------------------------------------------
    MenuEdit = new TGPopupMenu(gClient->GetRoot());

    MenuEdit->AddEntry("P&arameters"  , M_EDIT_PARAMETERS);

    // Instrument Menu ------------------------------------
    
    fMenuInstrument = new TGPopupMenu(gClient->GetRoot());
    fMenuInstrument->AddEntry("Keithley 196", M_INST_K196);
    fMenuInstrument->AddEntry("Keithley 230", M_INST_K230);
    fMenuInstrument->AddSeparator();
    fMenuInstrument->AddEntry("Fit",          M_INST_FIT);
    fMenuInstrument->AddEntry("Comment",      M_INST_COMMENT);

    // Help menu -------------------------------------------
    MenuHelp = new TGPopupMenu(gClient->GetRoot());
    MenuHelp->AddSeparator();
    MenuHelp->AddEntry("&About"   , M_HELP_ABOUT);


    TGMenuBar *MenuBar = new TGMenuBar( this, 1, 1, kHorizontalFrame);

    MenuBar->AddPopup("&File", MenuFile, new 
		       TGLayoutHints(kLHintsTop | kLHintsLeft, 
				     0, 4, 0, 0));

    MenuBar->AddPopup("&Edit", MenuEdit, new 
		       TGLayoutHints(kLHintsTop | kLHintsLeft, 
				     0, 4, 0, 0));

    MenuBar->AddPopup("&Instruments", fMenuInstrument, new 
		       TGLayoutHints(kLHintsTop | kLHintsLeft, 
				     0, 4, 0, 0));

    MenuBar->AddPopup("&Help", MenuHelp, 
		      new TGLayoutHints(kLHintsTop | kLHintsRight) );


    // Connect the callbacks for the menu items. 
    MenuFile->Connect("Activated(Int_t)", "IVCurve", this,
		       "HandleMenu(Int_t)");

    MenuEdit->Connect("Activated(Int_t)", "IVCurve", this,
		       "HandleMenu(Int_t)");

    fMenuInstrument->Connect("Activated(Int_t)", "IVCurve", this,
			    "HandleMenu(Int_t)");

    MenuHelp->Connect("Activated(Int_t)", "IVCurve", this,
		       "HandleMenu(Int_t)");

    AddFrame(MenuBar, 
	     new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,
			       0, 0, 1, 1));
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : CreateToolBar
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on: 13-Jul-05
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CreateToolBar()
{
    SET_DEBUG_STACK;
    TString Path;

    if(gSystem->Getenv("ROOTSYS") != NULL)
    {
        Path = gSystem->Getenv("ROOTSYS");
    }
    else
    {
        Path = "./";
    }

    Path+="/icons/";
    int spacing = 8;
    fToolBar = new TGToolBar(this, 60, 20, kHorizontalFrame | kRaisedFrame);

    for (int i = 0; dialog_xpm_names[i]; i++) 
    {
        TString iconname(Path);
        iconname += dialog_xpm_names[i];
        toolbar_data[i].fPixmap = iconname.Data();
	fToolBar->AddButton(this, &toolbar_data[i], spacing);
    }
    AddFrame(fToolBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0,
					 0, 0));
    fToolBar->Connect("Pressed(Int_t)", "IVCurve", this, 
		      "HandleToolBar(Int_t)");
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : HandleToolBar
 *
 * Description : Handle toolbar items for this dialog.
 *
 * Inputs : ID for menu event.
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::HandleToolBar(Int_t id)
{
    SET_DEBUG_STACK;
    TGButton *tb;
    TCanvas *c1;

    switch (id) 
    {
    case M_FILE_LOAD:
	FileDialog(kTRUE);
	tb = fToolBar->GetButton(M_FILE_LOAD);
        tb->SetState(kButtonUp);
	break;
    case M_START:
	tb = fToolBar->GetButton(M_START);
        tb->SetState(kButtonUp);
	fTimer->Start(500, kFALSE);
	fInstruments->Reset();
	fInstruments->Setup(false);  // set to voltage source, FIXME
	CreateGraphObjects();
	fTakeData = kTRUE;
	break;
    case M_STOP:
	tb = fToolBar->GetButton(M_STOP);
        tb->SetState(kButtonUp);
	fTakeData = kFALSE;
	fTimer->Stop();
	break;
    case M_ZOOM_PLUS:
	Zoom();
	tb = fToolBar->GetButton(M_ZOOM_PLUS);
        tb->SetState(kButtonUp);
	break;
    case M_ZOOM_MINUS:
	UnZoom();
	tb = fToolBar->GetButton(M_ZOOM_MINUS);
        tb->SetState(kButtonUp);
	break;
    case M_ZOOM_SELECTED:
	c1 = fEmbeddedCanvas->GetCanvas();
	c1->SetEditable(kFALSE);
	PlotState = PLOT_STATE_ZOOM;
	break;
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : CreateStatusBar
 *
 * Description : Create a status bar that shows the loaded file, 
 *               running in test mode or real data. 
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions :
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CreateStatusBar(void)
{
    SET_DEBUG_STACK;
    /*
     * Finally add a status bar at the bottom. 
     * parts is the breakup of the 3 subdivisions of the
     * status bar.
     */
    Int_t parts[] = {40, 40, 20};
    fStatusBar = new TGStatusBar( this, 10, 10, kHorizontalFrame);
    fStatusBar->SetParts(parts, 3);
    this->AddFrame( fStatusBar,  new 
		    TGLayoutHints( kLHintsExpandX , 2, 2, 2, 2));
    fStatusBar->SetText("Please Select Data to Display.",0);
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : HandleMenu
 *
 * Description : Handle menu items for this dialog. 
 *
 * Inputs : ID for menu event. 
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 30-Jul-04
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::HandleMenu(Int_t id)
{
    SET_DEBUG_STACK;
    TRootHelpDialog *trh;
    //TCanvas *c1;

    // Handle menu items.
    switch (id) 
    {
    case M_FILE_EXIT:
	SendCloseMessage();
	break;

    case M_FILE_LOAD:
	FileDialog(kTRUE);
	break;

    case M_FILE_SAVE:
	FileDialog(kFALSE);
	break;

    case M_FILE_SAVEAS:
	DoSaveAs();
	break;
    case M_INST_FIT:
	FitData();
	break;
    case M_INST_COMMENT:
    {
	TString temp;
	new CommentDlg(this, &temp);
	if (temp.CompareTo("NONE")!=0)
	{
	    delete fComment;
	    fComment = new TString(temp);
	    CLogger::GetThis()->Log("# Comment: %s\n", fComment->Data());
	}
    }
    break;
    case M_FILE_PRINT:
#if 0
        new TGPrintDialog 
	    ( gClient->GetRoot(), this, 400,300,  printName, PrintPrg, &rc);
	if (rc == kTRUE)
	{
	    // Print!
	    printf("All is well!\n");
	}
        // Only saves to PS file, not quite what I want. 
	//	gStyle->SetPaperSize(kUSLetter);
	c1 = fEmbeddedCanvas->GetCanvas();
	c1->Print();
#endif
	break;

    case M_EDIT_PARAMETERS:
	new ParamDlg(this);
	break;
    case M_INST_K196:
	// Check the status of the menu and do the appropriate action. 
	if (fMenuInstrument->IsEntryChecked(M_INST_K196))
	{
	    // If the instrument is off, turn it on. 
	    cout << "Checked" << endl;
	    fMenuInstrument->UnCheckEntry(M_INST_K196);
	}
	else
	{
	    cout << "Unchecked." << endl;
	    fMenuInstrument->CheckEntry(M_INST_K196);
	}
	break;
    case M_INST_K230:
	if (fMenuInstrument->IsEntryChecked(M_INST_K230))
	{
	    cout << "Checked" << endl;
	    fMenuInstrument->UnCheckEntry(M_INST_K230);
	}
	else
	{
	    cout << "Unchecked." << endl;
	    fMenuInstrument->CheckEntry(M_INST_K230);
	}
	break;
    case M_HELP_ABOUT:
	trh = new TRootHelpDialog ( this,"IVCurve Help", 600, 400);
	trh->SetText(HelpText1);
	trh->Popup();
	break;

    default:
	printf("Menu item %d selected\n", id);
	break;
   }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : SetCurrentFileName
 *
 * Description : set the current file name loaded and put it on the status 
 *               bar
 *
 * Inputs : loaded file
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::SetCurrentFileName(const char *File)
{
    SET_DEBUG_STACK;
    /* Delete current file name */
    if (fCurrentFile)
    {
	delete fCurrentFile;
    }

    // Add in logging for this. 
    fCurrentFile = new TString(File);
    fStatusBar->SetText( fCurrentFile->Data(), 0);
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : FileDialog
 *
 * Description : Handle the Load/Save menu option by bringing up a file
 *               selection dialog.  This is strictly for data load and save
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 23-Oct-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::FileDialog(bool LoadOrSave)
{
    SET_DEBUG_STACK;
    TGFileInfo      fi;
    EFileDialogMode mode;

    if (LoadOrSave)
    {
	mode = kFDOpen;
    }
    else
    {
	mode = kFDSave;
    }

    fi.fFileTypes = filetypes;
    fi.fIniDir    = StrDup(fLastDir->Data());

    new TGFileDialog( gClient->GetRoot(), 0, mode, &fi);

    /* Looked at the code. if cancel is hit, this is NULL */
    if (fi.fFilename != NULL)
    {
	/*
	 * Store the name of the selected directory such that 
	 * the next time we open a file it will start in the 
	 * same directory. 
	 */
	delete fLastDir;
	fLastDir = new TString(fi.fIniDir);

	if (LoadOrSave)
	{
	    Load(fi.fFilename);
	}
	else
	{
	    Save(fi.fFilename);
	}
    }

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : PlotMe
 *
 * Description : Setup the actual plot
 *
 * Inputs : Index - not used
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::PlotMe(Int_t Index)
{
    SET_DEBUG_STACK;

    gPad->Clear();
    fGraph->SetMarkerSize(0.75);
    fGraph->SetMarkerStyle(kPlus);
    fGraph->Draw("ACP");

    // SetTitle(char) FIXME - Add in a dialog to get info on what is under test
    //
    TH1 *f = fGraph->GetHistogram();

    switch(fMode)
    {
    case 0:
    case 1:
	f->SetXTitle("Set Voltage");
	f->SetYTitle("Measured Voltage");
	break;
    case 2:
	f->SetXTitle("Set Current(A)");
	f->SetYTitle("Measured Voltage");
	break;
    case 3:
	f->SetXTitle("Set Voltage");
	f->SetYTitle("Measured Current (A)");
	break;
    }
    f->SetLabelSize(0.03, "X");
    f->SetLabelSize(0.03, "Y");

    gPad->Update();
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : CloseWindow
 *
 * Description : Registered callback for closing window. 
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 23-Oct-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CloseWindow(void)
{
    SET_DEBUG_STACK;
    if (fTimer)
    {
        fTimer->Stop();
        fTimer->Disconnect("Timeout()");
        delete fTimer;
        fTimer = 0;
    }
    // Got close message for this MainFrame. Terminates the application.
    CleanUp();
    gApplication->Terminate(0);
}

/**
 ******************************************************************
 *
 * Function Name : ProcessedEvent
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::ProcessedEvent(Int_t event, Int_t px, Int_t py, 
			     TObject *selected)
{
    SET_DEBUG_STACK;
    // Statics for rubber band zoom. 
    static Double_t x0, y0, x1, y1;
    static Int_t    pxold, pyold;
    static Int_t    px0, py0;
    static Int_t    linedrawn;

    Double_t        temp;
    Char_t          title[64];
    Double_t        x,y;

    TCanvas *c1 = fEmbeddedCanvas->GetCanvas();
    c1->AbsPixeltoXY(px,py,x,y); 

    switch(event)
    {
    case kMouseMotion:
	sprintf(title, "X: %8.4g Y: %8.1g", x, y);
	fStatusBar->SetText(title ,1);
	switch(PlotState)
	{
	case PLOT_STATE_ZOOM:
            if (linedrawn)
            {
                gVirtualX->DrawBox(px0, py0, pxold, pyold, TVirtualX::kHollow);
            }
            pxold = px; 
            pyold = py;
            linedrawn = 1;
	    gVirtualX->DrawBox(px0, py0, pxold, pyold, TVirtualX::kHollow);
	    break;
	}
	break;
    case kButton1Down:
	down_x = x;
	down_y = y;
	switch (PlotState)
	{
	case PLOT_STATE_ZOOM:
            gVirtualX->SetLineColor(-1);

            // Change line attributes only if necessary
	    //gPad->TAttLine::Modify(); 
            x0 = gPad->AbsPixeltoX(px);
            y0 = gPad->AbsPixeltoY(py);
            px0 = px; py0 = py;
            pxold = px; pyold = py;
            linedrawn = 0;
	    break;
	}
	break;
    case kButton1Up:
#if 0
	sprintf( title , "dt (ms): %8.2f dY: %8.1f", dx*1000.0, dy);
	fStatusBar->SetText(title ,2);
#endif
	switch (PlotState)
	{
	case PLOT_STATE_ZOOM:
            gPad->GetCanvas()->FeedbackMode(kFALSE);
            if (px == px0) return;
            if (py == py0) return;
            x1 = gPad->AbsPixeltoX(px);
            y1 = gPad->AbsPixeltoY(py);

            if (x1 < x0)
            {
                temp = x0; 
                x0   = x1;
                x1   = temp;
            }
            if (y1 < y0)
            {
                temp = y0;
                y0   = y1;
                y1   = temp;
            }
	    TH1 *h = fGraph->GetHistogram();
	    if (h)
	    {
		h->GetXaxis()->SetRangeUser(x0,x1);
		h->GetYaxis()->SetRangeUser(y0,y1);
	    }
            gPad->Modified();
            gPad->Update();
	    PlotState = PLOT_STATE_NORMAL;
	    break;
	}
	break;
    case kButton3Down:
    case kButton1Motion:
    case kMouseEnter:
    case kMouseLeave: 
	break;
    default:
	printf("Event %d\n", event);
	break;
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : UnZoom
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::UnZoom(void)
{
    SET_DEBUG_STACK;
    TH1 *h = fGraph->GetHistogram();
    if (h)
    {
	cout << "Got Histogram " << endl;
	h->GetXaxis()->UnZoom();
	h->GetYaxis()->UnZoom();
    }
    gPad->Update();
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Zoom
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::ZoomAxis(TAxis *a)
{
    SET_DEBUG_STACK;
    Double_t x,y, delta;
    x = a->GetXmin();
    y = a->GetXmax();
    delta = fabs(y-x);
    delta /= (2.0*fZoomLevel);
    if (x>0.0)
    {
	x += delta;
    }
    else
    {
	x -= delta;
    }
    if (y>0.0)
    {
	y -= delta;
    }
    else
    {
	y += delta;
    }
    cout << " Set range User " << x << " " << y << endl;
    a->SetRangeUser(x,y);
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Zoom
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::Zoom(void)
{
    SET_DEBUG_STACK;
    TH1 *h = fGraph->GetHistogram();
    TAxis *ax;
    if (h)
    {
	ax = h->GetXaxis();
	ZoomAxis(ax);
	// Now do y axis. 
	ax = h->GetYaxis();
	ZoomAxis(ax);
	fZoomLevel *= 2.0;
    }
    gPad->Update();
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Load
 *
 * Description : Load a previously generated file. 
 *
 * Inputs : filename to load
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IVCurve::Load(const char *file)
{
    SET_DEBUG_STACK;


    if (strstr(file, "root") != NULL)
    {
	CleanGraphObjects();
	// Open a file for save, use the root file protocol. 
	TFile myin(file, "READ");
	fGraph = (TGraph *)myin.Get("IVCurve");
	myin.Close();
    }
    else if(strstr(file, "csv") != NULL)
    {
	delete fGraph;
	fGraph = new TGraph(file, "%lg,%lg");
        // Named -- This appears to be used as the TKey. 
	fGraph->SetName("IVCurve");  
    }
    else if((strstr(file, "tsv") != NULL) ||
	    (strstr(file, "txt") != NULL))
    {
	// In this case delete and recreate the load. 
	delete fGraph;
	fGraph = new TGraph(file); 
	fGraph->SetName("IVCurve");
    }
    PlotMe(0);
    // Someday add in the ability to insert multiple files.
    SET_DEBUG_STACK;
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : Save
 *
 * Description : Save data to file
 *
 * Inputs : filename to save
 *
 * Returns : true on success. 
 *
 * Error Conditions : file open failed
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IVCurve::Save(const char *file)
{
    SET_DEBUG_STACK;

    // Look at the suffix and determine how we want to store. 
    if (strstr(file, "root") != NULL)
    {
	// Open a file for save, use the root file protocol. 
	TFile myout(file, "NEW", "IVCurve Data");
	//myout.cd();
	fGraph->Write();
	TNamed Named("Comment","NONE");

	if (fComment)
	{
	    // Name the string we want to write, this gives it a key. 
	    Named.SetTitle(fComment->Data());
	    Named.Write(); 
	}
	myout.Close();
    }
    else if((strstr(file, "csv") != NULL) ||
	    (strstr(file, "tsv") != NULL) ||
	    (strstr(file, "txt") != NULL))
    {
	// This format is really weird. 
	//fGraph->SaveAs(file);
	ofstream myout(file);
	if(myout.is_open())
	{
	    if (fComment)
		myout << "# " << *fComment << endl;
	    Int_t N = fGraph->GetN();
	    Double_t x, y;
	    for(Int_t i=0;i<N;i++)
	    {
		fGraph->GetPoint( i, x, y);
		myout << x << "," << y << endl;
	    }
	    myout.close();
	}
    }

    SET_DEBUG_STACK;
    return kTRUE;
}
/**
 ******************************************************************
 *
 * Function Name : CreateGraphObjects
 *
 * Description : Clean up any old graphics objects and create new ones
 *
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions : NONE to date
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IVCurve::CreateGraphObjects(void)
{
    SET_DEBUG_STACK;
//     ftmg         = new TMultiGraph();
//     fLegend      = new TLegend(0.80, 0.75, 0.95, 0.89);
    CleanGraphObjects();
    fGraph = new TGraph();
    // TNamed - This appears to create the key in the TFile structure. 
    fGraph->SetName("IVCurve");
    SET_DEBUG_STACK;
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : CleanGraphObjects
 *
 * Description : NOt sure this is necessary
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IVCurve::CleanGraphObjects(void)
{
    SET_DEBUG_STACK;
    if (gPad)
	gPad->Clear();
    delete fGraph;
    fGraph = NULL;

    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : DoSave
 *
 * Description : Handle the Save menu option by bringing up a file
 *               selection dialog. FIXME
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : 
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::DoSaveAs(void)
{
    SET_DEBUG_STACK;

    TGFileInfo fi;

    fi.fFileTypes = SPSaveTypes;
    fi.fIniDir    = StrDup(fLastDir->Data());

    new TGFileDialog( gClient->GetRoot(), this, kFDSave, &fi);
    if (fi.fFilename == NULL)
    {
	// No action to be taken!
	return;
    }

    if (strlen(fi.fFilename) > 0)
    {
	TCanvas *c1 = fEmbeddedCanvas->GetCanvas();
	c1->SaveAs(fi.fFilename);
    }
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : CheckInstrumentStatus
 *
 * Description : 
 *    Color is green if instrument is alive and talking otherwise red
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : 
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CheckInstrumentStatus(void)
{
    SET_DEBUG_STACK;

    if (fInstruments->Keithley196_OK())
    {
	fMenuInstrument->CheckEntry(M_INST_K196);
	fMultimeter->SetTextColor(fGreenColor);
    }
    else
    {
	fMenuInstrument->UnCheckEntry(M_INST_K196);
	fMultimeter->SetTextColor(fRedColor);
    }

    if (fInstruments->Keithley230_OK())
    {
	fMenuInstrument->CheckEntry(M_INST_K230);
	fVoltageSource->SetTextColor(fGreenColor);
    }
    else
    {
	fMenuInstrument->UnCheckEntry(M_INST_K230);
	fVoltageSource->SetTextColor(fRedColor);
    }

}
/**
 ******************************************************************
 *
 * Function Name : TimeoutProc
 *
 * Description : Update screen data periodically based on a timeout. 
 *               This is only relevant when running real time meaning
 *               there is a constantly updating position source.
 * 
 *               This should only be active when online. 
 *
 * Inputs :  NONE
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 23-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::TimeoutProc(void)
{
    SET_DEBUG_STACK;

    static Double_t x = 0;
    static Double_t y = 0;

    if(fTakeData)
    {
	switch(fMode)
	{
	case 0:
	    // Test code
	    x = x + 1.0;
	    y = pow(x,2.0);
	    fGraph->AddPoint(x,y);
	    break;
	default:
	    // advance the voltage, take the measurement and plot it. 
	    fInstruments->StepAndAcquire();
	    x = fInstruments->Voltage();
	    y = fInstruments->Result();
	    if (fMode == 2)
	    {
		x = x/fResistor;
	    }
	    fGraph->AddPoint(x,y);
	    fTakeData = !fInstruments->Done();
	    break;
	}

	PlotMe(0);
    }
    //cout << "Timeout" << endl;
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : FitData
 *
 * Description : Fit the diode equation to the current plot. 
 *
 * Inputs : NONE
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::FitData(void)
{
    SET_DEBUG_STACK;
    /*
     * https://en.wikipedia.org/wiki/Diode_modelling
     *
     * https://root.cern.ch/doc/master/classTGraph.html#a61269bcd47a57296f0f1d57ceff8feeb
     *
     * FIXME, put the upper and lower limits in. 
     * parameters dialog. 
     */
    PlotMe(0);
    TAxis *a     = fGraph->GetXaxis();
    Double_t min = a->GetXmin();
    Double_t max = a->GetXmax();
    fFitFunction->SetRange(min,max);
    fGraph->Fit(fFitFunction,"V","", min, max);
    fPlotNotes->DrawLatexNDC( 0.15, 0.85,
			      "I(V) = I_{s}(e^{#frac{V}{K_{b} T}}-1.0)");
    fFitFunction->Draw("SAME");
    gPad->Update();
}
/**
 ******************************************************************
 *
 * Function Name : CreateFitFunction
 *
 * Description :
 * https://root.cern.ch/doc/master/classTF1.html
 * 
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::CreateFitFunction(void)
{
    /*
     * Thermal voltage 
     */
    Double_t IdealityFactor = 1.0; 
    Double_t Temperature    = 293.15;
    Double_t ThermalVoltage =  IdealityFactor * TMath::K() * Temperature/TMath::Qe();
    fFitFunction = new TF1("Shockley", "[0]*(exp(x/[1])-1.0)",0.0, 1.0);
    fFitFunction->SetParameter(0, 1.0e-3);
    // Can we fix the thermal voltage? 
    fFitFunction->SetParameter(1, ThermalVoltage);
    fFitFunction->SetLineColor(2);

    //fPlotNotes = new TPaveLabel( 0.0, 140.0, 4.0, 180.0, 
    fPlotNotes = new TLatex( );
}
/**
 ******************************************************************
 *
 * Function Name : ReadConfiguration
 *
 * Description : Open read the configuration file. 
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IVCurve::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();

    fEnv = new TEnv(".IVCurve");
    log->Log("# IVCurve Loading prefs: %s \n", fEnv->GetRcName());

    int32_t v = fEnv->GetValue("IVCurve.Verbose", 0);
    log->SetVerbose(v);

    uint8_t Voltmeter     = fEnv->GetValue("Voltmeter.GPIB", 3);
    uint8_t VoltageSource = fEnv->GetValue("VoltageSource.GPIB", 14);
    double Start          = fEnv->GetValue("VoltageSource.Start",   -1.0);
    double Stop           = fEnv->GetValue("VoltageSource.Stop",     1.0);
    double Step           = fEnv->GetValue("VoltageSource.Step",     0.1);
    double Fine           = fEnv->GetValue("VoltageSource.FineStep", 0.01);
    double Window         = fEnv->GetValue("VoltageSource.Window",    0.1);
    fResistor             = fEnv->GetValue("IVCurve.Resistance",   1000.0);
    fMode                 = fEnv->GetValue("IVCurve.Mode",            0);
    double MaxCurrent     = fEnv->GetValue("VoltageSource.MaxI",   4.0e-3);
    
    switch (fMode)
    {
    case 0:
	log->Log("# IVCurve running in test mode.\n");
	break;
    case 1:
	log->Log("# IVCurve running Volts:Volts for calibration.\n");
	break;
    case 2:
	log->Log("# IVCurve running infered current using known resistor.\n");
	break;
    case 3:
	log->Log("# IVCurve running true I-V\n");
	break;
    }


    // Open the instruments - 
    fInstruments = new Instruments(Voltmeter, VoltageSource);
    fComment     = NULL;

    /**
     * Configure the various parameters in the instruments according to
     * the configuration file. 
     */
    fInstruments->Start(Start);
    fInstruments->Stop( Stop);
    fInstruments->Step( Step);
    fInstruments->Fine( Fine);
    fInstruments->Window(Window);
    fInstruments->SetCurrentLimit(MaxCurrent);

    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : WriteConfigurationFile
 *
 * Description : Write out final configuration
 *
 * Inputs : none
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on:  
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool IVCurve::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *log = CLogger::GetThis();

    log->Log("# IVCurve: Write Configuration.\n");
    fEnv->SetValue("IVCurve.Verbose"   , (Int_t) log->GetVerbose());
    fEnv->SetValue("Voltmeter.GPIB",     fInstruments->MultimeterAddress());
    fEnv->SetValue("VoltageSource.GPIB", fInstruments->VoltageSourceAddress());
    fEnv->SetValue("VoltageSource.Start",    fInstruments->Start());
    fEnv->SetValue("VoltageSource.Stop",     fInstruments->Stop());
    fEnv->SetValue("VoltageSource.Step",     fInstruments->Step());
    fEnv->SetValue("VoltageSource.FineStep", fInstruments->Fine());
    fEnv->SetValue("VoltageSource.Window",   fInstruments->Window());
    fEnv->SetValue("IVCurve.Resistance",     fResistor);
    fEnv->SetValue("IVCurve.Mode",           fMode);
    fEnv->SetValue("VoltageSource.MaxI",     fInstruments->CurrentLimit());

    fEnv->SaveLevel(kEnvUser);
    delete fEnv;
    fEnv = 0;

    SET_DEBUG_STACK;
    return true;
}
