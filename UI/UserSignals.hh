/**
 ******************************************************************
 *
 * Module Name : UserSignals.hh
 *
 * Author/Date : C.B. Lirakis / 20-Feb-22
 *
 * Description : Access the terminate function from anywhere in
 * the module. 
 *
 * Restrictions/Limitations : none
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
#ifndef __USERSIGNALS_hh_
#define __USERSIGNALS_hh_
/**
 * Terminate - this function is used by the module and is linked to most of
 * the signals associated with the overall module. 
 */
void Terminate (int sig);
/**
 * Catch and deal with user signals here. 
 */
void UserSignal(int sig);
/**
 * Call to setup all signals. 
 */
void SetSignals(void);

#endif
