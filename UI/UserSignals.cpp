/********************************************************************
 *
 * Module Name : UserSignals.cpp
 *
 * Author/Date : C.B. Lirakis / 22-Feb-22
 *
 * Description : All signal handling here.
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
#include <cstring>
#include <unistd.h>
#include <csignal>

#include <TApplication.h>

// Local Includes.
#include "UserSignals.hh"
#include "debug.h"
#include "CLogger.hh"

extern TApplication *theApp;

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
void Terminate (int sig) 
{
    static int i=0;
    CLogger *logger = CLogger::GetThis();
    char msg[128], tmp[64];
    time_t now;
    time(&now);
 
    i++;
    if (i>1) 
    {
        _exit(-1);
    }

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
    case SIGSYS:               // 
        sprintf( msg, " SIGSYS ");
        break;
#ifndef MAC
     case SIGPWR:               // 
        sprintf( msg, " SIGPWR ");
        break;
    case SIGSTKFLT:               // Stack fault
        sprintf( msg, " SIGSTKFLT ");
        break;
#endif
   default:
        sprintf( msg, " Uknown signal type: %d", sig);
        break;
    }
    if (sig!=0)
    {
        sprintf ( tmp, " %s %d", LastFile, LastLine);
        strncat ( msg, tmp, sizeof(msg));
	logger->LogCommentTimestamp(msg);
	//logger->Log("# %s\n",msg);
    }

    // User termination here
    delete theApp;
    delete logger;

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
 * Function Name : UserSignal
 *
 * Description : Alternative way to communicate with a program. 
 *
 * Inputs : sig - signal issued.
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
void UserSignal(int sig)
{
    CLogger *logger = CLogger::GetThis();
    switch (sig)
    {
    case SIGUSR1:   // 10
    case SIGUSR2:   // 12
	logger->Log("# SIGUSR: %d\n", sig);
	// User code here. 
	break;
    }
}
/**
 ******************************************************************
 *
 * Function Name : SetSignals
 *
 * Description : Route termination signals through exit method. 
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : None
 * 
 * Unit Tested on: 23-Feb-08
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void SetSignals(void)
{
    /*
     * Setup a signal handler.      
     */
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
    signal (SIGSTOP, Terminate);   // 
    signal (SIGSYS, Terminate);    // 
#ifndef MAC
    signal (SIGSTKFLT, Terminate); // 
    signal (SIGPWR, Terminate);    // 
#endif
    // Setup user signals for further control
    signal (SIGUSR1, UserSignal);
    signal (SIGUSR2, UserSignal);  
}
