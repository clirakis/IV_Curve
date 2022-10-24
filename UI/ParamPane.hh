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
class TGNumberEntry;
class TGLabel;
class TGTextEntry;

class ParamPane : public TGVerticalFrame 
{
public:
    /*! 
     * Parameter setup for running an I-V curve. 
     */
    ParamPane(TGCompositeFrame*);
    ~ParamPane();

    void FillFields( Double_t Start, Double_t Stop, Double_t Step, Double_t Fine);

    Double_t GetStart(void);

    Double_t GetStop(void);

    Double_t GetStep(void);

    Double_t GetFine(void);

    // slots
    ClassDef(ParamPane, 0);

private:
    // private functions
    void CreateFields(void);

    TGNumberEntry   *fStart, *fStop, *fStep, *fFine;

};
#endif
