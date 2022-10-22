/**
 ******************************************************************
 *
 * Module Name : ParamDialog.cpp
 *
 * Author/Date : C.B. Lirakis / 23-Nov-18
 *
 * Description : Edit the parameters for IV curve testing. 
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


/// Root Includes
#include <TROOT.h>
#include <TCanvas.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TMarker.h>
#include <TList.h>
#include <TGComboBox.h>
#include <RQ_OBJECT.h>


/// Local Includes.
#include "debug.h"
#include "ParamDialog.hh"
#include "ParamPane.hh"

/**
 ******************************************************************
 *
 * Function Name : PointDlg  Constructor
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
ParamDlg::ParamDlg(const TGWindow *main)
    : TGTransientFrame(gClient->GetRoot(), main, 60, 40)
{
    Connect("CloseWindow()", "ParamDlg", this, "CloseWindow()");

    fParamPane = new ParamPane(this);
    BuildButtonBox();

    MapSubwindows();
    Resize();
    MapWindow();
    fClient->WaitFor(this);
}
/**
 ******************************************************************
 *
 * Function Name : ParamDlg  Constructor
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
ParamDlg::~ParamDlg()
{
    delete fParamPane;
}
/**
 ******************************************************************
 *
 * Function Name : BuildButtonBox
 *
 * Description : Creates the GUI buttons Ok and Cancel
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on:
 *
 * Unit Tested by:
 *
 *
 *******************************************************************
 */
void ParamDlg::BuildButtonBox()
{
    TGButton *tb;

    // Create a frame to hold the buttons.
    TGCompositeFrame *ButtonFrame = new 
    TGCompositeFrame(this, 600, 20, kHorizontalFrame);

    TGLayoutHints* fL2 = new 
    TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);

    tb = new TGTextButton( ButtonFrame, "  &Ok  ");
    tb->Connect("Clicked()", "ParamDlg", this, "DoOK()");
    ButtonFrame->AddFrame( tb, fL2);

    tb = new TGTextButton( ButtonFrame, "  &Cancel  ");
    tb->Connect("Clicked()", "ParamDlg", this, "DoCancel()");
    ButtonFrame->AddFrame( tb, fL2);

    ButtonFrame->Resize();
    AddFrame(ButtonFrame, new TGLayoutHints( kLHintsExpandX|kLHintsLeft, 
					     2, 2, 2, 2));
}
/**
 ******************************************************************
 *
 * Function Name : CloseWindow
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on: 30-Jul-04
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void ParamDlg::CloseWindow()
{
    // Called when closed via window manager action.
    delete this;
}
/**
 ******************************************************************
 *
 * Function Name : DoOK
 *
 * Description : User pressed the OK button -
 *               Set the user-provided storage to the new minimum
 *               and maximum and close the window
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 * Unit Tested on:
 *
 * Unit Tested by:
 *
 *
 *******************************************************************
 */
void ParamDlg::DoOK()
{

    SendCloseMessage();
}
/**
 ******************************************************************
 *
 * Function Name : DoCancel
 *
 * Description : Close the window
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
void ParamDlg::DoCancel()
{
    SendCloseMessage();
}
/**
 ******************************************************************
 *
 * Function Name : DoClose
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
 * Unit Tested by:
 *
 *
 *******************************************************************
 */
void ParamDlg::DoClose()
{
   // Handle close button.
    SendCloseMessage();
}
