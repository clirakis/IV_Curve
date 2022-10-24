/**
 ******************************************************************
 *
 * Module Name : CommentDialog.hh
 *
 * Author/Date : C.B. Lirakis / 23-Oct-22
 *
 * Description : Comment dialog for the header of the IO
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
#ifndef __COMMENTDLG_hh_
#define __COMMENTDLG_hh_

#  include <TGFrame.h>
#  include <RQ_OBJECT.h>
class TGTextEntry;
class TString;

class CommentDlg : public TGTransientFrame
{
    ClassDef( CommentDlg, 0);

public:
    /// Constructor
    CommentDlg (const TGWindow *parent, TString *comment);
    ~CommentDlg();

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
    TGTextEntry *fComment;
    TString     *fReturn;
};

#endif
