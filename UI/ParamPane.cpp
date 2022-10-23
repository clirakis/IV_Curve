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
 * Change this pane to reflect the input parameters for an I-V curve
 * 23-Oct-22
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

enum EntryEnum {
    M_START,
    M_STOP,
    M_STEP,
    M_FINE,
};
/**
 ******************************************************************
 *
 * Function Name : ParamPane
 *
 * Description : A generic pane for displaying and editing navigation
 * points, these can be events or waypoints. 
 *
 * Inputs : p - the parent window. 
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
    Resize();
    TGLayoutHints *TL =  
	new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY,
			  2, 2, 2, 2);
    p->AddFrame(this, TL);
    p->Resize();
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
    const UInt_t W = 80;
    const UInt_t H = 20;

    TGLabel *label;
    TGLayoutHints *tl = new TGLayoutHints(kLHintsTop | kLHintsExpandX |
					  kLHintsExpandY, 2, 2, 2, 2);
    TGGroupFrame  *gf = new TGGroupFrame( this, "Setup");

    // Rows, Columns. 
    gf->SetLayoutManager(new TGMatrixLayout(gf, 4, 2, 2, 2));
    AddFrame( gf, tl);

    // Row 0 ==================================================
    label = new TGLabel( gf, new TGHotString("Start (V)"));
    gf->AddFrame(label);

    fStart = new TGNumberEntry( gf,-1.0, 3, M_START, 
				TGNumberFormat::kNESRealThree, 
				TGNumberFormat::kNEAAnyNumber, 
				TGNumberFormat::kNELLimitMinMax, -10.0, 10.0);
    fStart->Resize( W, H);
    gf->AddFrame(fStart);

    // Row 1=====================================================
    label = new TGLabel( gf, new TGHotString("Stop (V)"));
    gf->AddFrame(label);

    fStop = new TGNumberEntry( gf, 1.0, 3, M_STOP, 
			       TGNumberFormat::kNESRealThree, 
			       TGNumberFormat::kNEAAnyNumber, 
			       TGNumberFormat::kNELLimitMinMax, -10.0, 10.0);
    fStop->Resize( W, H);
    gf->AddFrame(fStop);


    // Row 2=====================================================
    label = new TGLabel( gf, new TGHotString("Regular Step(V)"));
    gf->AddFrame(label);
    fStep = new TGNumberEntry( gf, 0.1, 3, M_STEP, 
			       TGNumberFormat::kNESRealThree, 
			       TGNumberFormat::kNEANonNegative, 
			       TGNumberFormat::kNELLimitMinMax, 0.0, 1.0);
    fStep->Resize( W, H);
    gf->AddFrame(fStep);


    // Row 3=====================================================
    label = new TGLabel( gf, new TGHotString("Fine Step (V)"));
    gf->AddFrame(label);

    fFine = new TGNumberEntry( gf, 0.01, 3, M_FINE, 
			       TGNumberFormat::kNESRealThree, 
			       TGNumberFormat::kNEANonNegative, 
			       TGNumberFormat::kNELLimitMinMax, 0.0, 0.1);
    fFine->Resize( W, H);
    gf->AddFrame(fFine);

    this->AddFrame( gf, tl);
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
void ParamPane::FillFields(Double_t Start, Double_t Stop, Double_t Step, Double_t Fine)
{
    SET_DEBUG_STACK;

    fStart->SetNumber(Start);
    fStop->SetNumber(Stop);
    fStep->SetNumber(Step);
    fFine->SetNumber(Fine);

    SET_DEBUG_STACK;
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
Double_t ParamPane::GetStart(void)
{
    SET_DEBUG_STACK;
    return fStart->GetNumber();
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
Double_t ParamPane::GetStop(void)
{
    SET_DEBUG_STACK;
    return fStop->GetNumber();
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
Double_t ParamPane::GetStep(void)
{
    SET_DEBUG_STACK;
    return fStep->GetNumber();
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
Double_t ParamPane::GetFine(void)
{
    SET_DEBUG_STACK;
    return fFine->GetNumber();
}
