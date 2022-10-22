/**
 ******************************************************************
 *
 * Module Name : Instruments.hh
 *
 * Author/Date : C.B. Lirakis / 22-Oct-22
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
#ifndef __INSTRUMENTS_hh_
#define __INSTRUMENTS_hh_

class    Keithley196;
class    Keithley2x0;

/// Instruments documentation here. 
class Instruments {
public:
    /// Default Constructor
    Instruments(void);
    /// Default destructor
    ~Instruments(void);
    /// Instruments function
    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    void StepAndAcquire(void);

    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    bool OpenKeithley196(void);

    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    bool OpenKeithley230(void);

    /* *****************************************************
     * Inline functions
     * *****************************************************/

    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    bool Keithley196_OK(void) const {return (hgpib196!=NULL);};
    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    bool Keithley230_OK(void) const {return (hgpib230!=NULL);};

private:
    Keithley196*  hgpib196;
    Keithley2x0*  hgpib230;

};
#endif
