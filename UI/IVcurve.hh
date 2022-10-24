/**
 ******************************************************************
 *
 * Module Name : IVcurve.hh
 *
 * Author/Date : C.B. Lirakis / 22-Nov-18
 *
 * Description : Run and manipulate IV curve testing. 
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
#ifndef __IVCURVE_hh_
#define __IVCURVE_hh_

#  include <TGFrame.h>
#  include <TPoint.h>
class TGWindow;
class TLegend;
class TMultiGraph;
class TObjString;
class TRootEmbeddedCanvas;
class TGStatusBar;
class TGToolBar;
class TPoint;
class TMarker;
class TAxis;
class TGLabel;
class TColor;
class Instruments;
class TGPopupMenu;
class TGraph;
class TTimer;
class TF1;
class TPaveLabel;
class TString;

enum PlotStateVals {PLOT_STATE_NORMAL, PLOT_STATE_ZOOM};

class IVCurve : public TGMainFrame {
    ClassDef(IVCurve, 0);

public:
    IVCurve(const TGWindow *p, UInt_t w = 800, UInt_t h = 400);
    ~IVCurve(void);

    void CloseWindow(void);
    void ProcessedEvent(Int_t event, Int_t x, Int_t y, TObject *selected);
    void HandleMenu(Int_t id);
    void HandleToolBar(Int_t id);
    void TimeoutProc(void);

private:
    TRootEmbeddedCanvas *fEmbeddedCanvas;
    TGStatusBar         *fStatusBar; 
    Double_t            down_x, down_y;
    TString             *fLastDir;
    TString             *fCurrentFile;
    TGToolBar           *fToolBar;

    /*
     * For zooming 
     */
    Int_t               PlotState;
    TPoint              screen;
    Double_t            X,Y;
    /*
     * For plotting
     */
    // Plotting
    TGraph*             fGraph;
    TF1*                fFitFunction;
    TPaveLabel*         fPlotNotes;

    Bool_t              fTakeData;

    // Things necessary for rubber band zoom.
    Bool_t              SelectOrZoom;   // True if in Zoom mode.
    Double_t            t1, y1, t2, y2;
    TMarker             *tm1, *tm2;

    // Regular zoom
    Double_t            fZoomLevel;

    // Labels for equipment status.
    TGLabel*            fMultimeter;
    TGLabel*            fVoltageSource;

    // Create colors for our status
    TColor*             fRedColor;
    TColor*             fGreenColor;

    // Instrument control
    Instruments*        fInstruments;
    TGPopupMenu*        fMenuInstrument; 

    TTimer*             fTimer;

    // Logging
    TString*            fComment;

    // Private functions.

    void AddEmbeddedCanvas(UInt_t w, UInt_t h);
    void AddStatusPane(void);
    void CreateStatusBar(void);
    void CreateMenuBar(void);
    void CreateToolBar(void);
    void FileDialog(bool LoadOrSave);
    void DoSaveAs(void);
    bool Load(const char *Filename);
    bool Save(const char *Filename);

    void PlotMe(Int_t);
    void UnZoom(void);
    void Zoom(void);
    void ZoomAxis(TAxis *a);

    void CheckInstrumentStatus(void);

    // For later
    void FitData(void);

    // Open and parse utilities
    bool CreateGraphObjects(void);
    bool CleanGraphObjects(void);
    void SetCurrentFileName(const char *File);
    void CreateFitFunction(void);

};
#endif
