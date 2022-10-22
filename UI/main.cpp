/**
 ******************************************************************
 *
 * Module Name : main.cpp
 *
 * Author/Date : C.B. Lirakis / 23-Apr-08
 *
 * Description : Standalone spiceplot
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 *******************************************************************
 */
// System includes.
#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <csignal>
#include <stdlib.h>

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
#include "IVcurve.hh"
// GPIB control.
#include "Keithley2x0.hh"
#include "Keithley196.hh"

// Root specific
Bool_t rootint = kFALSE;
static TApplication *theApp;

// My variables. 
const  double        Version   = 1.2;
static Keithley196*  hgpib196;
static Keithley2x0*  hgpib230;
static CLogger*      LogPtr;
static Bool_t        Offline = kFALSE;

extern char         *optarg;
static Int_t        verbose = 0;
static char         *FileName = NULL;
static IVCurve      *plotWindow;

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

    free(FileName);

    delete theApp;

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
    cout << "* Test file for making and displaying IV.  *" << endl;
    cout << "* curves.                                  *" << endl;
    cout << "* Built on "<< __DATE__ << " " << __TIME__ << "*" << endl;
    cout << "* Available options are :                  *" << endl;
    cout << "*     -f Filename                          *" << endl;
    cout << "*     -h Help                              *" << endl;
    cout << "*     -o Offline.                          *" << endl;
    cout << "*     -v verbose level (integer)           *" << endl;
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
static void ProcessCommandLineArgs(int argc, char **argv)
{
    int option;
    SET_DEBUG_STACK;
    do
    {
        option = getopt(argc, argv, "F:f:hHoOv:V:");
        switch(option)
        {
	case 'f':
	case 'F':
	    FileName = strdup(optarg);
	    Offline  = kTRUE;
	    break;
        case 'h':
        case 'H':
            Help();
            Terminate(0);
            break;
        case 'o':
        case 'O':
	    Offline = kTRUE;
	    break;
	case 'v':
        case 'V':
            verbose = atoi(optarg);
	    cout << "Verbose set to: " << verbose << endl;
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
    LastFile = (char *) __FILE__;
    LastLine = __LINE__;

    signal(SIGHUP , Terminate);   // Hangup.
    signal(SIGINT , Terminate);   // CTRL+C signal
    signal(SIGKILL, Terminate);   //
    signal(SIGQUIT, Terminate);   //
    signal(SIGILL , Terminate);   // Illegal instruction
    signal(SIGABRT, Terminate);   // Abnormal termination
    signal(SIGIOT , Terminate);   //
    signal(SIGBUS , Terminate);   //
    signal(SIGFPE , Terminate);   //
    signal(SIGSEGV, Terminate);   // Illegal storage access
    signal(SIGTERM, Terminate);   // Termination request
    signal(SIGSTKFLT, Terminate); //
    signal(SIGSTOP, Terminate);   //
    signal(SIGPWR, Terminate);    //
    signal(SIGSYS, Terminate);    //

    LogPtr = new CLogger("IVCurve.log","IVCurve",Version);

    // User initialization goes here.

    if (!Offline)
    {
	hgpib196 = new Keithley196( 3, verbose);
	if (hgpib196->CheckError())
	{
	    LogPtr->Log("# Error opening device. perhaps wrong GPIB address.\n");
	    delete hgpib196;
	    hgpib196 = NULL;
	    return false;
	}

	hgpib230 = new Keithley2x0( 13, 'V', verbose);
	if (hgpib230->CheckError())
	{
	    LogPtr->Log("# Error opening 230. perhaps wrong GPIB address.%d \n", 1);
	    delete hgpib230;
	    hgpib230 = NULL;
	    return false;
	}
    }

    // User initialization goes here.
    plotWindow = new IVCurve(gClient->GetRoot(), 800, 400, verbose);
    if (FileName)
    {
	plotWindow->OpenAndParseFile(FileName);
    }
    return true;
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
    // Start up root.
    if (rootint)
    {
       theApp = new TRint("App", &argc, argv);
    }
    else
    {
        theApp = new TApplication("App", &argc, argv);
    }
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(1111);
    gStyle->SetStatFont(42);
    if (Initialize())
    {
        theApp->Run();
    }
    Terminate(0);
}
