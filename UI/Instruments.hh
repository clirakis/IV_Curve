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
#include <stdint.h>

class    Keithley196;
class    Keithley2x0;

/// Instruments documentation here. 
class Instruments {
public:

    /*!
     * Description: 
     *    Open up both instruments and configure them as necessary.
     *
     * Arguments:
     *   Keithley196_Address - Keithley 196 Multimeter GPIB address
     *   Keithley230_Address - Keithley 230 Voltage Source GPIB address
     *
     * Returns:
     *   None
     *
     * Errors:
     *   If the device fails to open, check Error() method. 
     */
    Instruments(uint8_t Keithley196_Address, uint8_t Keithley230_Address);

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
    bool StepAndAcquire(void);
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
    inline bool Done(void) const{ return (fVoltage> fStopVoltage);};

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
    inline bool Keithley196_OK(void) const {return (hgpib196!=NULL);};

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
    inline bool Keithley230_OK(void) const {return (hgpib230!=NULL);};

    inline bool SystemOn(void) const {return ((hgpib196!=NULL) && 
					      (hgpib230!=NULL));};

    void Reset(void);
    bool Setup(void);

    inline double Voltage(void) const {return fVoltage;};
    inline double Current(void) const {return fCurrent;};

    inline void     Start(double Volts) {fStartVoltage = Volts;};
    inline double   Start(void) const {return fStartVoltage;};
    inline void     Stop (double Volts) {fStopVoltage = Volts;};
    inline double   Stop (void) const {return fStopVoltage;};
    inline void     Step (double Volts) {fStep = Volts;};
    inline double   Step (void) const {return fStep;};
    inline void     Fine (double Volts) {fFine = Volts;};
    inline double   Fine (void) const {return fFine;};

    uint8_t MultimeterAddress(void) const;
    uint8_t VoltageSourceAddress(void) const;

    /*! Access the This pointer. */
    static Instruments* GetThis(void) {return fInstruments;};

private:

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
    bool OpenKeithley196(uint8_t address);

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
    bool OpenKeithley230(uint8_t address);

    Keithley196*  hgpib196;
    Keithley2x0*  hgpib230;


    /* Maintain the current status of the operation */
    uint32_t fStepNumber;      /*! Count on step number.    */
    double   fStartVoltage;    /*! Starting Voltage         */
    double   fStopVoltage;     /*! Ending Voltage for sweep */
    double   fStep;            /*! Corse step               */
    double   fFine;            /*! Fine step                */
    double   fVoltage;         /*! Current voltage value    */

    double   fCurrent;         /*! Last current measured    */

    /*! The static 'this' pointer. */
    static Instruments *fInstruments;
};
#endif
