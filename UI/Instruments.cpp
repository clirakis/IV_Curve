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

// Default Starting values
const double kIncrement =  0.1;    // Volts
const double kMedium    =  0.05;   // Volts
const double kFine      =  0.01;   // Volts
const double kStart     = -1.0;    // Volts
const double kStop      =  1.0;    // Volts

Instruments* Instruments::fInstruments;

/**
 ******************************************************************
 *
 * Function Name : Instruments constructor
 *
 * Description :
 *     Open up both instruments and configure them as necessary.
 *
 * Inputs :
 *   Keithley196_Address - Keithley 196 Multimeter GPIB address
 *   Keithley230_Address - Keithley 230 Voltage Source GPIB address
 *
 * Returns : NONE
 *
 * Error Conditions : if GPIB open fails
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
Instruments::Instruments (uint8_t Keithley196_Address, 
			  uint8_t Keithley230_Address)
{
    SET_DEBUG_STACK;
    //CLogger *LogPtr = CLogger::GetThis();
    fInstruments = this;
    // Try to open the instruments. 
    OpenKeithley196(Keithley196_Address);
    OpenKeithley230(Keithley230_Address);

    Reset();
    fStartVoltage = kStart;
    fStopVoltage  = kStop;
    fStep         = kIncrement;
    fFine         = kFine;

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
 * Function Name : Reset
 *
 * Description : Reset all state machine pointers etc. 
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
void Instruments::Reset(void)
{
    SET_DEBUG_STACK;
    fStepNumber = 0;
    fVoltage    = fStartVoltage;
    fCurrent    = 0.0;
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
bool Instruments::OpenKeithley196(uint8_t address)
{
    SET_DEBUG_STACK;
    CLogger *LogPtr = CLogger::GetThis();
    int verbose = LogPtr->GetVerbose();
    hgpib196 = new Keithley196( address, verbose);
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
bool Instruments::OpenKeithley230(uint8_t address)
{
    SET_DEBUG_STACK;
    CLogger *LogPtr = CLogger::GetThis();
    int verbose = LogPtr->GetVerbose();
    hgpib230 = new Keithley2x0( address, 'V', verbose);
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
 * Function Name : Setup
 *
 * Description : Setup the instruments for acquisition. 
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
bool Instruments::Setup(void)
{
    CLogger *LogPtr = CLogger::GetThis();

    if((hgpib196 == NULL) || (hgpib230 == NULL))
    {
	LogPtr->Log("# Setup: Units are not open.\n");
	return false;
    }

    LogPtr->Log("# Setting up to run IV curve.\n");
    LogPtr->Log("# SETUP Keithley 196 DMM. \n");
#if 0
    LogPtr->Log("# Read Keithley 196 DMM. Set to read DCV\n");
    hgpib196->SetFunction(Keithley196::DCV);
#else
    LogPtr->Log("# Read Keithley 196 DMM. Set to read DCA\n");
    hgpib196->SetFunction(Keithley196::DCA);
#endif
    LogPtr->Log("# 196DMM Initial read: %g status: %d Prefix: %s\n",
		 hgpib196->GetData(), hgpib196->ReadStatus(), 
		 hgpib196->Prefix());

    // Setup 230 voltage source. 
    LogPtr->Log("# SETUP Keithley 230 voltage source. \n");
    hgpib230->SetUnitType(Keithley::VoltageSource);
    hgpib230->Operate();
    // Limit the current to 8mA
    hgpib230->SetCurrent(8.0e-3);
    hgpib230->DisplaySource();

    LogPtr->Log("# Start: %f, Stop: %f, Step: %f, Fine: %f\n", 
		fStartVoltage, fStopVoltage, fStep, fFine);
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
bool Instruments::StepAndAcquire(void)
{
    /* Sleep between set and read. */
    const struct timespec sleeptime = {0L, 250000000};
    CLogger *LogPtr = CLogger::GetThis();

    if((hgpib196 == NULL) || (hgpib230 == NULL))
    {
	LogPtr->Log("# Setup: Units are not open.\n");
	return false;
    }
    fStepNumber++;
#if 0
    cout << "DEBUG Set Voltage: " << fVoltage << " " 
	 << fStepNumber << endl;
#endif

#if 0
    fStepNumber++;
    if (fabs(x)>1.0)
	x += kIncrement;
    else if (fabs(x)>0.7)
	x += kMedium;
    else
	x += kFine;
#else
    hgpib230->SetVoltage(fVoltage);
    // Settle time
    nanosleep(&sleeptime, NULL);
    // Read back value. 
    fCurrent = hgpib196->GetData();
    //LogPtr->Log("%g, %g,%s\n", x, hgpib196->GetData(), hgpib196->Prefix());
    LogPtr->Log("%g %g\n", fVoltage, fCurrent);

    // Look at result and determine when we should go fine. 
    // Step to next value. FIXME
    fVoltage += (fStep+fFine);
#endif
    return true;
}
uint8_t Instruments::MultimeterAddress(void) const 
{
    SET_DEBUG_STACK;
    return hgpib196->Address();
}
uint8_t Instruments::VoltageSourceAddress(void) const 
{
    SET_DEBUG_STACK;
    return hgpib230->Address();
}
