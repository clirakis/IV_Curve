/********************************************************************
 *
 * Module Name : Instruments.cpp
 *
 * Author/Date : C.B. Lirakis / 23-May-21
 *
 * Description : Generic Instruments
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

// GPIB control.
#include "Keithley2x0.hh"
#include "Keithley196.hh"


// Local Includes.
#include "debug.h"
#include "CLogger.hh"
#include "Instruments.hh"

// Starting values
const double kIncrement =  0.1;    // Volts
const double kMedium    =  0.05;   // Volts
const double kFine      =  0.01;   // Volts
const double kStart     = -1.0;    // Volts
const double kEnd       =  1.0;    // Volts

Instruments* Instruments::fInstruments;

/**
 ******************************************************************
 *
 * Function Name : Instruments constructor
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
Instruments::Instruments (void)
{
    SET_DEBUG_STACK;
    //CLogger *LogPtr = CLogger::GetThis();

    // Try to open the instruments. 
    OpenKeithley196();
    OpenKeithley230();
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : Instruments destructor
 *
 * Description : Clean up the instruments
 *
 * Inputs : NONE
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
Instruments::~Instruments (void)
{
    delete hgpib196;
    delete hgpib230;
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description : 
 *
 * Inputs : NONE
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
bool Instruments::OpenKeithley196(void)
{
    SET_DEBUG_STACK;
    CLogger *LogPtr = CLogger::GetThis();
    int verbose = LogPtr->GetVerbose();
    hgpib196 = new Keithley196( 3, verbose);
    if (hgpib196->CheckError())
    {
	LogPtr->Log("# Error opening device. perhaps wrong GPIB address.\n");
	delete hgpib196;
	hgpib196 = NULL;
	SET_DEBUG_STACK;    
	return false;
    }
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description : 
 *
 * Inputs : NONE
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
bool Instruments::OpenKeithley230(void)
{
    SET_DEBUG_STACK;
    CLogger *LogPtr = CLogger::GetThis();
    int verbose = LogPtr->GetVerbose();
    hgpib230 = new Keithley2x0( 13, 'V', verbose);
    if (hgpib230->CheckError())
    {
	LogPtr->Log("# Error opening 230. perhaps wrong GPIB address.%d \n", 1);
	delete hgpib230;
	hgpib230 = NULL;
	SET_DEBUG_STACK;    
	return false;
    }
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : StepAndAcquire
 *
 * Description : Step the voltage applied and measure the resulting current
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
void Instruments::StepAndAcquire(void)
{
    /* Sleep between set and read. */
    const struct timespec sleeptime = {0L, 500000000};
    CLogger *LogPtr = CLogger::GetThis();
    double x;

    cout <<"Setting up to run IV curve. " << endl;
    LogPtr->Log("# SETUP Keithley 196 DMM. \n");
    LogPtr->Log("#Read Keithley 196 DMM. Set to read DCV\n");
    //hgpib196->SetFunction(Keithley196::DCV);
    hgpib196->SetFunction(Keithley196::DCA);
    LogPtr->Log("# Initial read: %g status: %d Prefix: %s\n",
		 hgpib196->GetData(), hgpib196->ReadStatus(), 
		 hgpib196->Prefix());

    // Setup 230 voltage source. 
    hgpib230->SetUnitType(Keithley::VoltageSource);
    hgpib230->Operate();
    hgpib230->SetCurrent(8.0e-3);

    hgpib230->DisplaySource();
    x = kStart; // Output voltage. 

    cout << "Set Voltage: " << x << endl;
    hgpib230->SetVoltage(x);
    nanosleep(&sleeptime, NULL);
    // Read back value. 
    //LogPtr->Log("%g, %g,%s\n", x, hgpib196->GetData(), hgpib196->Prefix());
    LogPtr->Log("%g %g\n", x, hgpib196->GetData());
    // Step to next value. 
    if (fabs(x)>1.0)
	x += kIncrement;
    else if (fabs(x)>0.7)
	x += kMedium;
    else
	x += kFine;

}
