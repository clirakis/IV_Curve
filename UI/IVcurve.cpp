/**
 ******************************************************************
 *
 * Module Name : IVcurve.cpp
 *
 * Author/Date : C.B. Lirakis / 22-Nov-18
 *
 * Description : Run and plot the IV curves
 *
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
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
#include <TApplication.h>
#include <TVirtualX.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGFileDialog.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TEnv.h>
#include <TFile.h>
#include <TGTextEditDialogs.h> // TGPrintDialog
#include <TRootHelpDialog.h>
#include <TQObject.h>
#include <TGButtonGroup.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TGFileDialog.h>
#include <TPoint.h>
#include <TPad.h>
#include <TVirtualPad.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TSystem.h>
#include <TObjString.h>

// Local Includes
#include "debug.h"
#include "IVcurve.hh"
#include "ParamDialog.hh"

// Setup print queues. Way out of date!
const char *PrintPrg[]  = {"/usr/bin/lpr","/usr/bin/lp"};
const char *printName[] = {"zevonlaser","elvis"};

// File types supported for input. 
const char *filetypes[] = { 
    "Log files",   "*.log",
    "Data files",  "*.dat",
    "All files",     "*",
    "ROOT files",    "*.root",
    0,               0 };

// Save as file types
static const char *SPSaveTypes[] = {
    "PostScript",   "*.ps",
    "Encapsulated PostScript", "*.eps",
    "PDF",          "*.pdf",
    "PNG",          "*.png",
    0,              0 };

/*
 * Enumerations for menu and other buttons. 
 */
enum SVPCommandIdentifiers {
   M_FILE_EXIT=100,
   M_FILE_OPEN,
   M_FILE_PRINT,
   M_EDIT_PARAMETERS,
   M_HELP_ABOUT,
   M_ZOOM_PLUS,
   M_ZOOM_MINUS,
   M_ZOOM_SELECTED,
   M_RELOAD,
   M_FILE_SAVEAS
};

// Toolbar stuff
const char *dialog_xpm_names[] = {
    "bld_open.png",
    "stop_t.xpm",
    "profile_t.xpm",
    "tb_refresh.xpm",
    0
};


// ToolBarData fields.
// pixmap TipText stay_down ID TGButton
ToolBarData_t toolbar_data[] = {
  { "",     "Load file",    kFALSE,     M_FILE_OPEN, NULL },
  { "", "Zoom Selected",     kTRUE,     M_ZOOM_PLUS, NULL },
  { "",        "UnZoom",         0,    M_ZOOM_MINUS, NULL },
  { "", "Zoom Selected",         0, M_ZOOM_SELECTED, NULL },
  { "",        "Reload",     kTRUE,        M_RELOAD, NULL },
  { "",            NULL,         0,               0, NULL }
};


static const char *HelpText1 = 
    "This dialog allows plotting of the traces produced by gnucap\n";


/**
 ******************************************************************
 *
 * Function Name :
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
IVCurve::IVCurve(const TGWindow *p, UInt_t w, UInt_t h, int v) : 
    TGMainFrame( p, w, h,  kVerticalFrame)
{
    // Used to store GUI elements that need to be deleted in the destructor.

    Connect("CloseWindow()", "IVCurve" , this, "CloseWindow()");

    CreateMenuBar();
    CreateToolBar();
    AddEmbeddedCanvas(1400, 900);
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

    ftmg         = NULL;
    fLegend      = NULL;
    fZoomLevel   = 2;
    verbose      = v;
}
/**
 ******************************************************************
 *
 * Function Name : IVCurve Destructor
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
IVCurve::~IVCurve()
{
    if (fCurrentFile)
    {
	delete fCurrentFile;
    }
    delete fLastDir;
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
    TGPopupMenu *MenuFile, *MenuHelp, *MenuEdit;

    // Layout menu here. 
    // Create menubar and popup menus. The hint objects are used to place
    // and group the different menu widgets with respect to eachother.

    // First menu, named file, and only has exit associated with it. 
    // Standard File Menu
    // ---------------------------------------------------------
    MenuFile = new TGPopupMenu(gClient->GetRoot());

    MenuFile->AddEntry("O&pen"  , M_FILE_OPEN);
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

    MenuBar->AddPopup("&Help", MenuHelp, 
		      new TGLayoutHints(kLHintsTop | kLHintsRight) );


    // Connect the callbacks for the menu items. 
    MenuFile->Connect("Activated(Int_t)", "IVCurve", this,
		       "HandleMenu(Int_t)");

    MenuEdit->Connect("Activated(Int_t)", "IVCurve", this,
		       "HandleMenu(Int_t)");

    MenuHelp->Connect("Activated(Int_t)", "IVCurve", this,
		       "HandleMenu(Int_t)");

    AddFrame(MenuBar, 
	     new TGLayoutHints(kLHintsTop|kLHintsLeft|kLHintsExpandX,
			       0, 0, 1, 1));
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
 * Unit Tested on:
 *
 * Unit Tested by:
 *
 *
 *******************************************************************
 */
void IVCurve::HandleToolBar(Int_t id)
{
    TGButton *tb;
    TCanvas *c1;

    switch (id) 
    {
    case M_FILE_OPEN:
	DoLoad();
	tb = fToolBar->GetButton(M_FILE_OPEN);
        tb->SetState(kButtonUp);
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
    case M_RELOAD:
        tb = fToolBar->GetButton(M_RELOAD);
        tb->SetState(kButtonUp);
	OpenAndParseFile( NULL);
	break;
    }
}
/**
 ******************************************************************
 *
 * Function Name :
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
void IVCurve::CreateStatusBar()
{
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
}
/**
 ******************************************************************
 *
 * Function Name :
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
    TRootHelpDialog *trh;
    //TCanvas *c1;

    // Handle menu items.
    switch (id) 
    {

    case M_FILE_EXIT:
	SendCloseMessage();
	break;

    case M_FILE_OPEN:
	DoLoad();
	break;

    case M_FILE_SAVEAS:
	DoSaveAs();
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
    case M_HELP_ABOUT:
	trh = new TRootHelpDialog ( this,"IVCurve Help", 600, 400);
	trh->SetText(HelpText1);
	trh->Popup();
	break;

    default:
	printf("Menu item %d selected\n", id);
	break;
   }
}
/**
 ******************************************************************
 *
 * Function Name :
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
void IVCurve::SetCurrentFileName(const char *File)
{
    /* Delete current file name */
    if (fCurrentFile)
    {
	delete fCurrentFile;
    }
    fCurrentFile = new TString(File);
    if(verbose>0) 
    {
	cout << "Current " << fCurrentFile << endl;
    }
    fStatusBar->SetText( fCurrentFile->Data(), 0);
}

/**
 ******************************************************************
 *
 * Function Name : DoLoad
 *
 * Description : Handle the Load menu option by bringing up a file
 *               selection dialog. 
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : 
 * 
 * Unit Tested on: 25-Apr-08
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void IVCurve::DoLoad()
{
    TGFileInfo fi;

    fi.fFileTypes = filetypes;
    fi.fIniDir    = StrDup(fLastDir->Data());

    new TGFileDialog( gClient->GetRoot(), 0, kFDOpen, &fi);
    *fLastDir = fi.fIniDir;
    OpenAndParseFile(fi.fFilename);
}

/**
 ******************************************************************
 *
 * Function Name :
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
void IVCurve::PlotMe(Int_t Index)
{
    gPad->Clear();
    gPad->Update();
}

/**
 ******************************************************************
 *
 * Function Name :
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
void IVCurve::CloseWindow()
{
   // Got close message for this MainFrame. Terminates the application.
   gApplication->Terminate(0);
}

/**
 ******************************************************************
 *
 * Function Name :
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
	    TH1 *h = ftmg->GetHistogram();
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
void IVCurve::UnZoom()
{
    TH1 *h = ftmg->GetHistogram();
    if (h)
    {
	cout << "Got Histogram " << endl;
	h->GetXaxis()->UnZoom();
	h->GetYaxis()->UnZoom();
    }
    gPad->Update();
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
void IVCurve::Zoom()
{
    TH1 *h = ftmg->GetHistogram();
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
}
/**
 ******************************************************************
 *
 * Function Name :
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
bool IVCurve::OpenAndParseFile(const char *file)
{
    char       msg[512], line[1024], *p;
    ifstream   *file_in;
    TObjString *title;
    Int_t      i, NLines, LineNumber;
    TGraph     *tg;
    TList      *graphs;
    Double_t   x, y;
    TH1F       *hist;
    TList      *TraceTitles;
    TString    *col1Title;
    Int_t      NColumns;    // Number of columns of data. 


    if (file != NULL)
    {
	SetCurrentFileName(file);
    }
    else
    {
	cout <<"Reloading file. " << endl;
    }

    file_in = new ifstream( fCurrentFile->Data(), ifstream::in );
    if (file_in->fail())
    {
	sprintf (msg, "File not found: %s", fCurrentFile->Data());
	new TGMsgBox( gClient->GetRoot(), NULL, "Error", 
		      msg, kMBIconExclamation);
	fStatusBar->SetText( "NONE", 0);
    }
    else
    {
	if (verbose > 0) 
	{
	    cout << "Open and Parse File" << endl;
	}
	/* Clean up on reload or new load */
	CleanGraphObjects();
	CreateGraphObjects();
	TraceTitles = new TList();
	NColumns = 0;
	NLines   = 0;
	/*
	 * Find the # sign and parse out the trace data
	 * These should be used for the individual graphs and the 
	 * legend.
	 */
	memset( line, 0, sizeof(line));
	file_in->getline(line, sizeof(line));
	if (line[0] == '#')
	{
	    /*
	     * Get rid of the pound sign that says this is a comment line.
	     */
	    line[0] = ' ';
	    p = strtok(line, " ");
	    if (p!= NULL) 
	    {
		NColumns++;                 // Count up the number of columns. 
		// this should be time. 
		col1Title = new TString(p); // Usually the time axis.
		if (verbose>1)
		{
		    cout << " Column 1 title " << *col1Title << endl;
		}
	    }
	    // loop to find rest of the titles. 
	    while ( (p=strtok( NULL, " ")) != NULL)
	    {
		if(verbose>0) 
		{
		    cout << "Found " << p << endl;
		}
		TraceTitles->Add(new TObjString(p));
		NColumns++;
	    }
	    // Sometimes gnucap puts out only one title. 
	    // Let's fix that
	    if (NColumns == 1)
	    {
		TraceTitles->Add(new TObjString(*col1Title));
		*col1Title = "V";
		NColumns++;
	    }
	}
	else
	{
	    // This should be a failure.
	    cout << " Found no comment line" << endl;
	}
	if(verbose>0) 
	{
	    cout << "NColumns = " << NColumns << endl;
	}

	/* 
	 * Now loop and read the actual data. 
	 * Allocate the space for the data points. 
	 * and create the tgraphs. 
	 */
	NLines = 0;
	while (!file_in->eof())
	{
	    file_in->getline(line, sizeof(line));
	    if (strlen(line)>2)
	    {
		NLines++;
	    }
	}
	NLines--; // Remove title line.
	if(verbose>0) 
	{
	    cout << "NLines = " << NLines << endl;
	}
	/*
	 * Time to create the TGraphs. 
	 */
	i = 1;
	{
	    /*
	     * Loop over all the trace titles. There
	     * should be NColumn of these. Each representing
	     * a single trace variable. 
	     */
	    TListIter next(TraceTitles);
	    while((title = (TObjString *) next()))
	    {
		tg = new TGraph(NLines);
		tg->SetTitle( title->GetString().Data());
		tg->SetMarkerColor(i);
		tg->SetLineColor(i);
		tg->SetMarkerStyle(20);
		// The x axis most likely will only be time. 
		tg->GetXaxis()->SetTitle(col1Title->Data());
		// This could be any test point in the circuit.
		tg->GetYaxis()->SetTitle(title->GetString().Data()); 

		// Add it to the multigraph list. 
		ftmg->Add(tg);

		// Add to the legend box.
		fLegend->AddEntry (tg, title->GetString().Data(),"L");

		if(verbose>0) 
		{
		    cout << "Added tgraph " << i << endl;
		}
		i++;
	    }
	}
	/* 
	 * Cleanup from load
	 * the TGraphs now take care of the title.
	 */
	if (TraceTitles != NULL)
	{
	    TListIter next(TraceTitles);
	    while((title = (TObjString *) next()))
	    {
		delete title;
	    }
	    delete TraceTitles;
	}

	if (verbose > 0)
	{
	    cout <<"Rewind File " << endl;
	}
	// Seems like the best way to rewind given that it is a text file. 
	file_in->close();
	file_in->open( fCurrentFile->Data());
	// Skip first line
	file_in->getline(line, sizeof(line));

       	if(verbose>0) 
	{
	    cout << " Got first line " << line  
		 << " Number columns " << NColumns
		 << endl;
	}

	LineNumber = 0;
	/* Get a pointer to the list of graphs */
	graphs = ftmg->GetListOfGraphs();

	while (!file_in->eof())
	{
	    memset( line, 0, sizeof(line));
	    file_in->getline(line, sizeof(line));
	    if(verbose>1) 
	    {
		cout << LineNumber<< " Get Line:" << line << " | ";
	    }
	    p = strtok( line, " ");
	    if (p != NULL)
	    {
		/*
		 * For DC series this is not time but really the
		 * x axis
		 */
		x = atof(p);
	    
		if(verbose>1) 
		{
		    cout << " X = " << x;
		}
	    }
	    /* Parse all the data into correct format */
	    for (i=0; i<NColumns-1; i++)
	    {
		p = strtok( NULL, " ");
		if (p != NULL)
		{
		    //cout << "P = " << p << endl;
		    /* Get specific graph to fill */
		    tg = (TGraph *) graphs->At(i);
		    if (tg != NULL)
		    {
			y = atof(p);
			tg->SetPoint(LineNumber, x, y);
			if(verbose>1) 
			{
			    cout << " Y = " << y << endl;
			}
		    }
		}
	    }
	    LineNumber++;
	}

	file_in->close();
	delete file_in;

	if(verbose>1) 
	{
	    cout << endl << " Drawing all plots now " << endl;
	}

	ftmg->Draw("ALP");

	hist = ftmg->GetHistogram();
	if (hist)
	{
	    hist->SetTitle("Spice data plot");
	    hist->GetXaxis()->SetTitle(col1Title->Data());
	    hist->GetYaxis()->SetTitle("Volts"); 
	    //hist->GetYaxis()->SetTitle(col2Title->Data()); 
	}
	fLegend->Draw();
	fLegend->SetTextFont(2);
	fLegend->SetTextSize(0.02);

	if (col1Title != NULL)
	{
	    delete col1Title;
	}
	gPad->Update();
    }
    return true;
}
bool IVCurve::CreateGraphObjects()
{
    ftmg         = new TMultiGraph();
    fLegend      = new TLegend(0.80, 0.75, 0.95, 0.89);

    return true;
}
bool IVCurve::CleanGraphObjects()
{
    gPad->Clear();

    if (ftmg)
    {
	delete ftmg;
	ftmg = NULL;
    }
    if (fLegend)
    {
	delete fLegend;
	fLegend = NULL;
    }
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : DoSaveAs
 *
 * Description : Handle the Save menu option by bringing up a file
 *               selection dialog. 
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
void IVCurve::DoSaveAs()
{

    TGFileInfo fi;

    fi.fFileTypes = SPSaveTypes;
    fi.fIniDir    = StrDup(fLastDir->Data());

    new TGFileDialog( gClient->GetRoot(), this, kFDSave, &fi);
    if (fi.fFilename == NULL)
    {
	// No action to be taken!
	return;
    }
    cout << "Filename " << fi.fFilename << endl;
    if (strlen(fi.fFilename) > 0)
    {
	TCanvas *c1 = fEmbeddedCanvas->GetCanvas();
	c1->SaveAs(fi.fFilename);
    }
}
