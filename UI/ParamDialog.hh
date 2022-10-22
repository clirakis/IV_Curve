/**
 ******************************************************************
 *
 * Module Name : ParamDialog.hh
 *
 * Author/Date : C.B. Lirakis / 23-Nov-18
 *
 * Description : Dialog for viewing and editing parameters for IV
 * curve testing
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
#ifndef __PARAMDLG_hh_
#define __PARAMDLG_hh_

#  include <TGFrame.h>
#  include <RQ_OBJECT.h>
#  include "ParamPane.hh"

class TList;

class ParamDlg : public TGTransientFrame
{
    ClassDef( ParamDlg, 0);

public:
    /// Constructor
    ParamDlg (const TGWindow *parent);
    ~ParamDlg();

    /// Close the window
    void   CloseWindow();
    /// User pressed the OK button, end the dialog
    void   DoOK();
    /// User pressed the Cancel button, end the dialog
    void   DoCancel();
    /// Close the window
    void   DoClose();

private:

    /// Build the Ok and Cancel Buttons
    void BuildButtonBox();
    ParamPane *fParamPane;
};

#endif
