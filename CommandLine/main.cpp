/**
 ******************************************************************
 *
 * Module Name : IV test program
 *
 * Author/Date : C.B. Lirakis / 21-Nov-18
 *
 * Description :
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
#include <cstring>
#include <cmath>
#include <csignal>
#include <unistd.h>
#include <time.h>
#include <fstream>
#include <ostream>
#include <sys/time.h>
#include <sys/resource.h>

/// Root includes http://root.cern.ch
#include <TROOT.h>
#include <TRint.h>
#include <TStyle.h>
#include <TEnv.h>  
#include <KeySymbols.h>
#include <pthread.h>


/// Local Includes.
#include "debug.h"
#include "CLogger.hh"

#include "Keithley2x0.hh"
#include "Keithley196.hh"

static bool          verbose;
static double        Version   = 1.0;

static Keithley196*  hgpib196;
static Keithley2x0*  hgpib230;
static CLogger*      LogPtr;

/**
******************************************************************
*
* Function Name : Terminate
*
* Description : Deal with errors in a clean way!
*               ALL, and I mean ALL exits are brought 
*               through here!
*
* Inputs : Signal causing termination. 
*
* Returns : none
*
* Error Conditions : Well, we got an error to get here. 
*
*******************************************************************
*/ 
static void Terminate (int sig) 
{
    static int i=0;
    char msg[128], tmp[64];
    time_t now;
    time(&now);
 
    i++;
    if (i>1) 
    {
        _exit(-1);
    }

    LogPtr->Log("# Program Ends: %s",ctime(&now));

    switch (sig)
    {
    case -1: 
      sprintf( msg, "User abnormal termination");
      break;
    case 0:                    // Normal termination
        sprintf( msg, "Normal program termination.");
        break;
    case SIGHUP:
        sprintf( msg, " Hangup");
        break;
    case SIGINT:               // CTRL+C signal 
        sprintf( msg, " SIGINT ");
        break;
    case SIGQUIT:               //QUIT 
        sprintf( msg, " SIGQUIT ");
        break;
    case SIGILL:               // Illegal instruction 
        sprintf( msg, " SIGILL ");
        break;
    case SIGABRT:              // Abnormal termination 
        sprintf( msg, " SIGABRT ");
        break;
    case SIGBUS:               //Bus Error! 
        sprintf( msg, " SIGBUS ");
        break;
    case SIGFPE:               // Floating-point error 
        sprintf( msg, " SIGFPE ");
        break;
    case SIGKILL:               // Kill!!!! 
        sprintf( msg, " SIGKILL");
        break;
    case SIGSEGV:              // Illegal storage access 
        sprintf( msg, " SIGSEGV ");
        break;
    case SIGTERM:              // Termination request 
        sprintf( msg, " SIGTERM ");
        break;
    case SIGSTKFLT:               // Stack fault
        sprintf( msg, " SIGSTKFLT ");
        break;
    case SIGTSTP:               // 
        sprintf( msg, " SIGTSTP");
        break;
    case SIGXCPU:               // 
        sprintf( msg, " SIGXCPU");
        break;
    case SIGXFSZ:               // 
        sprintf( msg, " SIGXFSZ");
        break;
    case SIGSTOP:               // 
        sprintf( msg, " SIGSTOP ");
        break;
    case SIGPWR:               // 
        sprintf( msg, " SIGPWR ");
        break;
    case SIGSYS:               // 
        sprintf( msg, " SIGSYS ");
        break;
    default:
        sprintf( msg, " Uknown signal type: %d", sig);
        break;
    }
    sprintf ( tmp, " %s %d", LastFile, LastLine);
    strncat ( msg, tmp, sizeof(msg));

    LogPtr->Log("# %s\n", msg);
    
    // User termination here
    delete hgpib196;
    delete hgpib230;

    delete LogPtr;

    if (sig == 0)
    {
        _exit (0);
    }
    else
    {
        _exit (-1);
    }
}

/**
 ******************************************************************
 *
 * Function Name : Help
 *
 * Description : provides user with help if needed.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 *******************************************************************
 */
static void Help(void)
{
    SET_DEBUG_STACK;
    cout << "********************************************" << endl;
    cout << "* Keithly test                             *" << endl;
    cout << "* Built on "<< __DATE__ << " " << __TIME__ << "*" << endl;
    cout << "* Version: " << Version << "*" << endl;
    cout << "* Available options are :                  *" << endl;
    cout << "*     -v verbose                           *" << endl;
    cout << "*                                          *" << endl;
    cout << "********************************************" << endl;
}
/**
 ******************************************************************
 *
 * Function Name :  ProcessCommandLineArgs
 *
 * Description : Loop over all command line arguments
 *               and parse them into useful data.
 *
 * Inputs : command line arguments. 
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static void
ProcessCommandLineArgs(int argc, char **argv)
{
    int option;
    SET_DEBUG_STACK;
    do
    {
        option = getopt( argc, argv, "hHnv");
        switch(option)
        {
        case 'h':
        case 'H':
            Help();
        Terminate(0);
        break;
        case 'v':
            verbose = true;
            break;
        }
    } while(option != -1);
}
/**
 ******************************************************************
 *
 * Function Name : Initialize
 *
 * Description : Initialze the process
 *               - Setup traceback utility
 *               - Connect all signals to route through the terminate 
 *                 method
 *               - Perform any user initialization
 *
 * Inputs : none
 *
 * Returns : true on success. 
 *
 * Error Conditions : depends mostly on user code
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static bool Initialize(void)
{
    SET_DEBUG_STACK;
    time_t now;
    char   msg[128];
    time(&now);
    struct tm *tmnow = gmtime(&now);
    strftime (msg, sizeof(msg), "%F %T", tmnow);

    signal (SIGHUP , Terminate);   // Hangup.
    signal (SIGINT , Terminate);   // CTRL+C signal 
    signal (SIGKILL, Terminate);   // 
    signal (SIGQUIT, Terminate);   // 
    signal (SIGILL , Terminate);   // Illegal instruction 
    signal (SIGABRT, Terminate);   // Abnormal termination 
    signal (SIGIOT , Terminate);   // 
    signal (SIGBUS , Terminate);   // 
    signal (SIGFPE , Terminate);   // 
    signal (SIGSEGV, Terminate);   // Illegal storage access 
    signal (SIGTERM, Terminate);   // Termination request 
    signal (SIGSTKFLT, Terminate); // 
    signal (SIGSTOP, Terminate);   // 
    signal (SIGPWR, Terminate);    // 
    signal (SIGSYS, Terminate);    // 

    // User initialization goes here. 
    // ========================================================
    LogPtr = new CLogger("keithley.log","Keithley",1.7);

    // User initialization goes here.

    hgpib196 = new Keithley196( 3, verbose);
    if (hgpib196->CheckError())
    {
	LogPtr->Log("# Error opening device. perhaps wrong GPIB address.\n");
	return false;
    }

    hgpib230 = new Keithley2x0( 13, 'V', verbose);
    if (hgpib230->CheckError())
    {
	LogPtr->Log("# Error opening 230. perhaps wrong GPIB address.%d \n", 1);
	return false;
    }
    return true;
}
#ifdef KEITHLEY196
static void TestKeithley196(void)
{
    double x, buffer[16];
    // 196 testing

    cout << "Testing Keithley 196 DMM. " << endl;
#if 0
    cout << "Read Keithley 196 DMM. Set to read DCA " << endl;
    hgpib->SetFunction(Keithley196::DCA);
#else
    cout << "Read Keithley 196 DMM. Set to read DCV " << endl;
    hgpib->SetFunction(Keithley196::DCV);
#endif
    cout << hgpib->GetData() << " " 
	 << hgpib->ReadStatus() << " " 
	 << hgpib->Prefix() << endl;
    //hgpib->DisplayCharacter((char*)"CHRIS");
    //sleep(3);
    //hgpib->DisplayCharacter((char*)"");
    //hgpib->SetFilter(20);
    //hgpib->SetAutoCalMult(true);
    //hgpib->Read_AD(false);
    //hgpib->SetFunction(Keithley196::ACV);
    //hgpib->SetFormat(Keithley196::ReadingWithoutPrefix);
    //hgpib->SetSRQ(Keithley196::ReadingOverflow);
    //hgpib->ExpFilter(true);
    //hgpib->SetDataStore(100,5);
    //hgpib->SetRate(Keithley196::Digit3_5);
    //hgpib->SetTrigger(Keithley196::OneShotOnGET);
    //hgpib->SetDelay(10);
//	hgpib->Zero(true);
//	hgpib->GetMachineStatus();
//	hgpib->DumpMachineStatus();
//	sleep(1);
    //hgpib->SetAutoCalMult(false);
    //hgpib->SetFilter(0);
    //hgpib->Read_AD(true);
    //hgpib->SetFunction(Keithley196::DCV);
    //hgpib->SetFormat(Keithley196::ReadingWithPrefix);
    //hgpib->SetSRQ(Keithley196::Disable);
    //hgpib->ExpFilter(false);
    //hgpib->SetDataStore(200,6);
    //hgpib->SetRate(Keithley196::Digit5_5);
    //hgpib->SetTrigger(Keithley196::OneShotOnX);
    //hgpib->SetDelay(0);
//	hgpib->Zero(false);
//	hgpib->GetMachineStatus();
//	hgpib->DumpMachineStatus();


#if 0
    hgpib->GetMachineStatus();
    hgpib->DumpMachineStatus();
#endif
#if 0
    // 196 get a buffer of data. 
    hgpib->SetFunction(Keithley196::DCV);
    cout << hgpib->GetData() << " " 
	 << hgpib->ReadStatus() << " " 
	 << hgpib->Prefix() << endl;
    // buffer, buffer size, interval in ms
    hgpib->GetBufferOfData( buffer, 5, 100);
    for (i=0;i<5;i++)
    {
	cout << i << " " << buffer[i] << endl;
    }
	
#endif
    //hgpib->GetStatus();
    //hgpib->DumpStatus();
}
#endif
#ifdef KEITHLEY230
static void TestKeithley230(void)
{
    double x, y;
    int    i;

    hgpib->SetUnitType(Keithley::VoltageSource);
    hgpib->Operate();
#if 0
    hgpib->DisplaySource();
    sleep(2);
    hgpib->DisplayLimit();
    sleep(2);
    hgpib->DisplayDwell();
    sleep(2);
    hgpib->DisplayMemory();
    sleep(2);
#endif

#if 1
    hgpib->DisplaySource();
    x = 0.1;
    for(i=0;i<10;i++)
    {
	cout << "Set Voltage: " << x << endl;
	hgpib->SetVoltage(x);
	x += 0.1;
	y=hgpib->Get();
	cout <<"Read back: " << y << endl;
	sleep(1);
    }
#endif
#if 0
    hgpib->DisplayMemory();
    for (i=0;i<10;i++)
    {
	hgpib->SetMemory(i);
	hgpib->Get();
	sleep(1);
    }
#endif
#if 0
    hgpib->DisplaySource();
    for (i=0;i<10;i++)
    {
	hgpib->SetBuffer(i);
	hgpib->Get();
	sleep(1);
    }
#endif
#if 0
    hgpib->DisplayLimit();
    x = 1.0e-3;
    for (i=0;i<10;i++)
    {
	hgpib->SetCurrent(x);
	hgpib->Get();
	sleep(1);
	x += 1.0e-3;
    }
#endif
#if 0
    x = 1.0e-2;
    for (i=0;i<10;i++)
    {
	hgpib->SetDwell(x);
	hgpib->Get();
	sleep(1);
	x += 1.0e-2;
    }
#endif
#if 0
    cout << "Testing set command." << endl;
    // Full set command
    hgpib->SetBuffer(0);
    for (i=0;i<15;i++)
    {
	x = 0.06*i;
	hgpib->Set( x, 1.0, 1.0, i, i);
    }
    //hgpib->Get();
    //hgpib->DumpBuffer();
    hgpib->SetBuffer(0);
    hgpib->ProgramSingle();
    hgpib->SetTrigger(Keithley2x0::StartOnX);
    hgpib->Execute();
#endif
}
#endif
static void IVCurve(void)
{
    const struct timespec sleeptime = {0L, 900000000};

    const double kIncrement = 0.1;    // Volts
    const double kMedium    = 0.05;   // Volts
    const double kFine      = 0.01;   // Volts
    const double kStart     = -1.0;   // Volts
    const double kEnd       = 1.0;    // Volts
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

    do {
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

	nanosleep(&sleeptime, NULL);
    } while (x<kEnd);
}
/**
 ******************************************************************
 *
 * Function Name : main
 *
 * Description : It all starts here:
 *               - Process any command line arguments
 *               - Do any necessary initialization as a result of that
 *               - Do the operations
 *               - Terminate and cleanup
 *
 * Inputs : command line arguments
 *
 * Returns : exit code
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
int main(int argc, char **argv)
{
    ProcessCommandLineArgs(argc, argv);
    //struct timespec sleeptime = {1L,000000000};
    if (Initialize())
    {
	IVCurve();
    }
    Terminate(0);
}
