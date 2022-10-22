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

#  include <TROOT.h>
#  include <TGFrame.h>
#  include <TGWindow.h>
#  include <TList.h>
#  include <TGraph.h>
#  include <TMultiGraph.h>
#  include <TLegend.h>
#  include <TRootEmbeddedCanvas.h>
#  include <TGMenu.h>
#  include <TGButton.h>
#  include <TGStatusBar.h>
#  include <TGToolBar.h>
#  include <TPoint.h>
#  include <Gtypes.h>
#  include <Rtypes.h>
#  include <TMarker.h>

class TLegend;
class TMultiGraph;
class TObjString;

enum PlotStateVals {PLOT_STATE_NORMAL, PLOT_STATE_ZOOM};

class IVCurve : public TGMainFrame {
    ClassDef(IVCurve, 0);

public:
    IVCurve(const TGWindow *p, UInt_t w = 800, UInt_t h = 400, Int_t v=0);
    ~IVCurve();
    void CloseWindow();
    void ProcessedEvent(Int_t event, Int_t x, Int_t y, TObject *selected);
    void HandleMenu(Int_t id);
    void HandleToolBar(Int_t id);
    void SetVerbose(Int_t value) {verbose = value;};
    bool OpenAndParseFile(const char *file);

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
    TMultiGraph         *ftmg;
    TLegend             *fLegend;

    // Things necessary for rubber band zoom.
    Bool_t              SelectOrZoom;   // True if in Zoom mode.
    Double_t            t1, y1, t2, y2;
    TMarker             *tm1, *tm2;
    // Regular zoom
    Double_t            fZoomLevel;
    // Verbosity of printout.
    Int_t               verbose;

    void AddEmbeddedCanvas(UInt_t w, UInt_t h);
    void CreateStatusBar();
    void CreateMenuBar();
    void CreateToolBar();
    void DoLoad();
    void PlotMe(Int_t);
    void UnZoom();
    void Zoom();
    void ZoomAxis(TAxis *a);
    void DoSaveAs();

    // Open and parse utilities
    bool CreateGraphObjects();
    bool CleanGraphObjects();
    void SetCurrentFileName(const char *File);
};
#endif
