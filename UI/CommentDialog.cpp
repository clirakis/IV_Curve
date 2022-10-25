/**
 ******************************************************************
 *
 * Module Name : CommentDialog.cpp
 *
 * Author/Date : C.B. Lirakis / 23-Oct-22
 *
 * Description : Create a comment for the I/O
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
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <RQ_OBJECT.h>
#include <TString.h>

/// Local Includes.
#include "debug.h"
#include "CommentDialog.hh"

/**
 ******************************************************************
 *
 * Function Name : CommentDlg  Constructor
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
CommentDlg::CommentDlg(const TGWindow *main, TString *in)
    : TGTransientFrame(gClient->GetRoot(), main, 60, 40)
{
    TGLayoutHints* fL2 = new 
	TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 5, 5);
    Connect("CloseWindow()", "CommentDlg", this, "CloseWindow()");
    SetWindowName("Plot Comment");
    fReturn = in;
    *fReturn = "NONE";

    fComment = new TGTextEntry(this, "NONE");
    fComment->Resize(500,30);
    AddFrame(fComment, fL2);

    BuildButtonBox();

    MapSubwindows();
    Resize();
    MapWindow();
    fClient->WaitFor(this);
}
/**
 ******************************************************************
 *
 * Function Name : CommentDlg  Constructor
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
CommentDlg::~CommentDlg()
{
    delete fComment;
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
void CommentDlg::BuildButtonBox()
{
    TGButton *tb;

    // Create a frame to hold the buttons.
    TGCompositeFrame *ButtonFrame = new 
    TGCompositeFrame(this, 600, 20, kHorizontalFrame);

    TGLayoutHints* fL2 = new 
	TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);

    tb = new TGTextButton( ButtonFrame, "  &Ok  ");
    tb->Connect("Clicked()", "CommentDlg", this, "DoOK()");
    ButtonFrame->AddFrame( tb, fL2);

    tb = new TGTextButton( ButtonFrame, "  &Cancel  ");
    tb->Connect("Clicked()", "CommentDlg", this, "DoCancel()");
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
void CommentDlg::CloseWindow()
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
void CommentDlg::DoOK(void)
{
    // Get the data an set the instruments up. 
    *fReturn = fComment->GetDisplayText();
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
void CommentDlg::DoCancel()
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
void CommentDlg::DoClose()
{
   // Handle close button.
    SendCloseMessage();
}
