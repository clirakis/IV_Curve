/**
 ******************************************************************
 *
 * Module Name : ParamPane.hh
 *
 * Author/Date : C.B. Lirakis / 23-Nov-18
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
#ifndef __PARAMPANE_hh_
#  define __PARAMPANE_hh_
#  include <RQ_OBJECT.h>
#  include <TGFrame.h>

class TList;
class NavPoint;
class TGNumberEntry;
class TGLabel;
class TGTextEntry;
class TGPictureButton;
class TListIter;

class ParamPane : public TGVerticalFrame {

public:
    /*! A pane for editing a NavPointList. 
     * If we added something, assume we added it at the end of the 
     * list prior to call. 
     * The frame is the parent window to hang off of. 
     * The TList is the list of points to operate on. 
     * EnterOnly, if true goes to the end of the list and edits 
     * that one point alone. 
     */
    ParamPane(TGCompositeFrame*);
    ~ParamPane();

    // slots
    void Next(void);
    void Prev(void);
    void DoOK(void);
    void ApplyEdits(void);
    ClassDef(ParamPane, 0);

private:
    // private functions
    void FillFields(NavPoint *);
    void CreateFields(void);
    void CreateButtons(void);
    Double_t* DegMinSec(Double_t in);

    TList*          fPoints;
    Int_t           fIndex;
    NavPoint*       fCurrent;
    TGLabel*        fCount;
    TGNumberEntry   *fLat[3], *fLon[3], *fZ;
    TGTextEntry     *fTitle;
    TGPictureButton *fNext,*fPrev;
    Bool_t          fEditOnly;
    Double_t        fZWork[3];  /* Workspace */
};
#endif
