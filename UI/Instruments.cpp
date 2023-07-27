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
const double kCurrentLimit = 1.0e3; // Current limit 1mA

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
    fCurrentStep  = fStep;
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
    fStepNumber  = 0;
    fSetVoltage  = fStartVoltage;
    fVoltage     = 0.0;
    fResult      = 0.0;
    fCurrentStep = fStep;
    fStepType    = 0;
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : OpenKeithley196
 *
 * Description : Open the multimeter in this case straight out of the box
 *               setup to measure voltage. 
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
    LogPtr->Log("# Keithley 196 open at address %d\n", address);
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : OpenKeithley230
 *
 * Description : open up the Keithley230 Voltage source
 *
 * Inputs : address - gpib address to use. 
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
    LogPtr->Log("# Keithley 230 open at address: %d\n", address);
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
 *   Current a boolean variable, if set to true setup the Keithley196
 *           to measure current. NOTE this requires a different physical
 *           lead setup
 *
 * Returns : true on success
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
bool Instruments::Setup(bool Current)
{
    SET_DEBUG_STACK;
    CLogger *LogPtr = CLogger::GetThis();

    if((hgpib196 == NULL) || (hgpib230 == NULL))
    {
	LogPtr->Log("# Setup: Units are not open.\n");
	return false;
    }

    LogPtr->Log("# Setting up to run IV curve.\n");
    LogPtr->Log("# SETUP Keithley 196 DMM. \n");
    if (Current)
    {
	LogPtr->Log("# Read Keithley 196 DMM. Set to read DCA\n");
	hgpib196->SetFunction(Keithley196::DCA);
    }
    else
    {
	LogPtr->Log("# Read Keithley 196 DMM. Set to read DCV\n");
	hgpib196->SetFunction(Keithley196::DCV);
    }
    LogPtr->Log("# 196DMM Initial read: %g status: %d Prefix: %s\n",
		 hgpib196->GetData(), hgpib196->ReadStatus(), 
		 hgpib196->Prefix());

    // Setup 230 voltage source. 
    LogPtr->Log("# SETUP Keithley 230 voltage source. \n");
    hgpib230->SetUnitType(Keithley::VoltageSource);
    hgpib230->Operate();
    // Set the current limit
    hgpib230->SetCurrent(kCurrentLimit);
    hgpib230->DisplaySource();

    LogPtr->Log("# Start: %f, Stop: %f, Step: %f, Fine: %f\n", 
		fStartVoltage, fStopVoltage, fStep, fFine);
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
 * Inputs : NONE
 *
 * Returns : true on success
 *
 * Error Conditions : fails if either of the GPIB units are not open
 * 
 * Unit Tested on: 25-Jul-23
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool Instruments::StepAndAcquire(void)
{
    SET_DEBUG_STACK;
    double StepSize = 0.0; 
    /* Sleep between set and read. */
    const struct timespec sleeptime = {0L, 250000000};
    const struct timespec slough    = {1L, 000000000};
    CLogger *LogPtr = CLogger::GetThis();
    //int verbose = LogPtr->GetVerbose();

    if((hgpib196 == NULL) || (hgpib230 == NULL))
    {
	LogPtr->Log("# Setup: Units are not open.\n");
	return false;
    }
    fStepNumber++;
    hgpib230->SetVoltage(fSetVoltage);
    fVoltage = fSetVoltage;
    // Settle time
    nanosleep(&sleeptime, NULL);
    // Read back value. 
    fResult = hgpib196->GetData();
    LogPtr->Log("%g, %g\n", fVoltage, fResult);

    /**
     * Step to next value. 
     * near zero or any other value for that matter step fine. 
     */
    switch(fStepType)
    {
    case 0: /* coarse */
	if (fabs(fSetVoltage+fCurrentStep)>=fWindow)
	{
	    StepSize = fStep;
	}
	else
	{
	    fStepType = 1;
	    StepSize = fFine;
	}
	break;
    case 1:
	if (fabs(fSetVoltage+fCurrentStep)>=fWindow)
	{
	    // Crossed the threshold
	    fStepType = 0;
	    StepSize = fStep;
	}
	else
	{
	    StepSize = fFine;
	}
	break;
    }
    // This is now set to the next requested voltage. 
    fSetVoltage += StepSize;
    // Do a little rounding
    if (fabs(fSetVoltage)<1.0e-6) fSetVoltage = 0.0;
    SET_DEBUG_STACK;
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
