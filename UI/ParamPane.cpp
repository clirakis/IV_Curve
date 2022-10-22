/********************************************************************
 *
 * Module Name : ParamPane.cpp
 *
 * Author/Date : C.B. Lirakis / 23-Nov-18
 *
 * Description : Pane for editing parameters for IV curves
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 ********************************************************************/
// System includes.
#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <time.h>

// root includes
#include <TText.h>
#include <TView.h>
#include <TGButton.h>
#include <TMarker.h>
#include <TList.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGString.h>
#include <TGLabel.h>

// Local Includes.
#include "ParamPane.hh"
#include "debug.h"

/**
 ******************************************************************
 *
 * Function Name : ParamPane
 *
 * Description : A generic pane for displaying and editing navigation
 * points, these can be events or waypoints. 
 *
 * Inputs : p - the parent window. 
 *          l - the list to display. 
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
ParamPane::ParamPane(TGCompositeFrame* p) : TGVerticalFrame(p,20,20)
{
    SET_DEBUG_STACK;

    CreateFields();
    if (!fEditOnly)
    {
	CreateButtons();
    }
    Resize();
    TGLayoutHints *TL =  
	new TGLayoutHints(kLHintsTop | kLHintsExpandX,
			  2, 2, 2, 2);
    p->AddFrame(this, TL);
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Destructor for ParamPane
 *
 * Description : Delete all the items in the pane
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions :
 * 
 * Unit Tested on: 8-Nov-15
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
ParamPane::~ParamPane(void)
{
    Int_t i;
    delete fCount;
    for (i=0;i<3;i++)
    {
	delete fLat[i];
	delete fLon[i];
    }
    delete fTitle;
    delete fNext;
    delete fPrev;
}
/**
 ******************************************************************
 *
 * Function Name : CreateFields
 *
 * Description : Create all the fields on the point pane. It is
 * laid out in a group frame using matrix layout.
 *
 * Inputs :  
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
void ParamPane::CreateFields(void)
{
    const UInt_t W1 = 40;
    const UInt_t W2 = 80;
    const UInt_t H  = 20;

    TGLabel *label;
    TGLayoutHints *tl =  new TGLayoutHints(kLHintsTop | kLHintsExpandX,
					   2, 2, 2, 2);
    TGGroupFrame  *gf = new TGGroupFrame( this, "Events");
    // Rows, Columns. 
    gf->SetLayoutManager(new TGMatrixLayout(gf, 4, 4, 2, 2));
    AddFrame( gf, tl);

    // Column 0
    label = new TGLabel( gf, new TGHotString("Entry"));
    gf->AddFrame(label);
    fCount = new TGLabel( gf, new TGHotString("        "));
    gf->AddFrame(fCount);

    label = new TGLabel( gf, new TGHotString("Title"));
    gf->AddFrame(label);
    fTitle = new TGTextEntry( gf, "                   ");
    gf->AddFrame(fTitle);

    // ===========================================================
    label = new TGLabel( gf, new TGHotString("Latitude"));
    gf->AddFrame(label);
    fLat[0] = new TGNumberEntry( gf, 0.0, 2, 1, TGNumberFormat::kNESInteger, 
			      TGNumberFormat::kNEAAnyNumber, 
			      TGNumberFormat::kNELLimitMinMax, -90.0, 90.0);
    fLat[0]->Resize( W1, H);
    gf->AddFrame(fLat[0]);

    fLat[1] = new TGNumberEntry( gf, 0.0, 2, 1, TGNumberFormat::kNESInteger, 
			      TGNumberFormat::kNEANonNegative, 
			      TGNumberFormat::kNELLimitMinMax, 0.0, 60.0);
    fLat[1]->Resize( W1, H);
    gf->AddFrame(fLat[1]);

    fLat[2] = new TGNumberEntry( gf, 0.0, 5, 1, TGNumberFormat::kNESReal, 
			      TGNumberFormat::kNEANonNegative, 
			      TGNumberFormat::kNELLimitMinMax, 0.0, 60.0);
    fLat[2]->Resize( W2, H);
    gf->AddFrame(fLat[2]);

    // ===========================================================
    label = new TGLabel( gf, new TGHotString("Longitude"));
    gf->AddFrame(label);
    fLon[0] = new TGNumberEntry( gf, 0.0, 3, 1, TGNumberFormat::kNESInteger, 
			      TGNumberFormat::kNEAAnyNumber, 
			      TGNumberFormat::kNELLimitMinMax, -180.0, 180.0);
    fLon[0]->Resize( W1, H);
    gf->AddFrame(fLon[0]);

    fLon[1] = new TGNumberEntry( gf, 0.0, 2, 1, TGNumberFormat::kNESInteger, 
			      TGNumberFormat::kNEANonNegative, 
			      TGNumberFormat::kNELLimitMinMax, 0.0, 60.0);
    fLon[1]->Resize( W1, H);
    gf->AddFrame(fLon[1]);

    fLon[2] = new TGNumberEntry( gf, 0.0, 5, 1, TGNumberFormat::kNESReal, 
			      TGNumberFormat::kNEANonNegative, 
			      TGNumberFormat::kNELLimitMinMax, 0.0, 60.0);
    fLon[2]->Resize( W2, H);
    gf->AddFrame(fLon[2]);

    // ===========================================================
    label = new TGLabel( gf, new TGHotString("Z"));
    gf->AddFrame(label);
    
    fZ = new TGNumberEntry( gf, 0.0, 8, 3, TGNumberFormat::kNESReal, 
			    TGNumberFormat::kNEAAnyNumber, 
			    TGNumberFormat::kNELLimitMinMax, -1000.0, 1000.0);
    gf->Resize();
    tl =  new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,
			    2, 2, 2, 2);
    gf->AddFrame(fZ, tl);

    // Fill the fields
//    FillFields(fCurrent);
    TGTextButton *tb = new TGTextButton(this, new TGHotString("Apply Edits"));
    tb->Connect( "Clicked()", "ParamPane", this, "ApplyEdits()");

    this->AddFrame( tb, tl);
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
void ParamPane::CreateButtons(void)
{
    // Make a frame to group these in. 
    TGGroupFrame  *gf = new TGGroupFrame( this, "Select", kHorizontalFrame);
    TGLayoutHints *L1 = new TGLayoutHints(kLHintsTop|kLHintsRight
                                          |kLHintsExpandX,
					  2, 2, 2, 2);

    fNext = new TGPictureButton(gf, "arrow_right.xpm");
    fNext->Connect("Clicked()","ParamPane", this, "Next()");
    gf->AddFrame(fNext, L1);
    fNext->SetEnabled(kFALSE);

    fPrev = new TGPictureButton(gf, "arrow_left.xpm");
    fPrev->Connect("Clicked()","ParamPane", this, "Prev()");
    gf->AddFrame(fPrev, L1);
    fPrev->SetEnabled(kFALSE);

    AddFrame(gf, L1);

    // Setup any conditionals. 
    fNext->SetEnabled(kTRUE);
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description : 
 *
 * Inputs : none
 *
 * Returns : None
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
void ParamPane::Prev(void)
{
    SET_DEBUG_STACK;
    fIndex--;
#if 0
    if (fIndex <0)
    {
	fIndex = fPoints->GetSize()-1;
    }
    fCurrent = (NavPoint *) fPoints->At(fIndex);
    FillFields(fCurrent);
#endif
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description : 
 *
 * Inputs : none
 *
 * Returns : None
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
void ParamPane::Next(void)
{
    SET_DEBUG_STACK;
    fIndex++;
#if 0
    if (fIndex >= fPoints->GetSize())
    {
	fIndex = 0;
    }
    fCurrent = (NavPoint *) fPoints->At(fIndex);
    FillFields(fCurrent);
#endif
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description : 
 *
 * Inputs : none
 *
 * Returns : None
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
void ParamPane::FillFields(NavPoint *e)
{
    SET_DEBUG_STACK;
    Char_t   *s;
    Double_t *Z;
#if 0
    /* Check to see that the event is not NULL. */
    if (e)
    {
	/* Set the text field with the unique ID. */
	fCount->SetText(e->GetUniqueID());
	s = (Char_t *) e->GetName();
	if (s)
	{
	    fTitle->SetText(s);
	}
	else
	{
	    fTitle->SetText("                   ");
	}
	//cout.precision(8);
	//cout <<"Fill fields:" << e->GetY() << endl;

	/* Latitude */
	Z = DegMinSec(e->Y());
	fLat[0]->SetNumber(Z[0]);	
	fLat[1]->SetNumber(Z[1]);	
	fLat[2]->SetNumber(Z[2]);	

	Z = DegMinSec(e->X());
	fLon[0]->SetNumber(Z[0]);
	fLon[1]->SetNumber(Z[1]);
	fLon[2]->SetNumber(Z[2]);

	fZ->SetNumber(e->Z());
    }
#endif
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description : 
 *
 * Inputs : none
 *
 * Returns : None
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
void ParamPane::DoOK(void)
{
    ApplyEdits();
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
void ParamPane::ApplyEdits(void)
{
    SET_DEBUG_STACK;
    // Check all the fields. 
    SET_DEBUG_STACK;
}
