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
#include "UserSignals.hh"

// Root specific
Bool_t rootint = kFALSE;
TApplication *theApp;

// My variables. 
const  double        Version   = 1.3;
static CLogger*      LogPtr;

extern char         *optarg;
static Int_t        verbose = 0;
static IVCurve      *plotWindow;


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
    cout << "*     -h Help                              *" << endl;
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
        option = getopt(argc, argv, "hHv:V:");
        switch(option)
        {
        case 'h':
        case 'H':
            Help();
            Terminate(0);
            break;
	case 'v':
        case 'V':
            verbose = atoi(optarg);
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

    SetSignals();

    LogPtr = new CLogger("IVCurve.log","IVCurve",Version);
    LogPtr->SetVerbose(verbose);

    // User initialization goes here.
    plotWindow = new IVCurve(gClient->GetRoot(), 640, 480);
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
