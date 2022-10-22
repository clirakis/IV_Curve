{
    TCanvas *Hobbes = new TCanvas("Dist","A test",5,5,1200,600);
    Hobbes->cd();
    TPad    *Calvin = new TPad("Calvin","Silly",0.02,0.02,0.99,0.99,10);
    Calvin->Draw();
    Calvin->cd();
    Calvin->SetGrid();

    const char *Filenames[] = {
	"./1N34/1.log",
	"./1N34/2.log",
	"./1N34/3.log",
	"./1N34/4.log",
	"./1N34/4a.log",
	"./1N34/5.log",
	"./1N34/6.log",
	"./1N34/7.log",
	"./1N34/8.log",
	"./1N34/9.log",
	NULL,
    };
    char Legends[32];

    TMultiGraph *tmg = new TMultiGraph();
    TLegend *legend  = new TLegend(0.1, 0.7, 0.3, 0.9);
    TGraph *IV;
    Int_t idx = 0;

    while (Filenames[idx] != NULL)
    {
	IV = new TGraph(Filenames[idx]);
	IV->SetMarkerStyle(22+idx);
	IV->SetMarkerColor(kBlack+idx);
	IV->SetLineColor(kBlack+idx);
	sprintf(Legends, "1N34 %d", idx);
	tmg->Add(IV);
	legend->AddEntry(IV, Legends);
	idx++;
    }

    tmg->SetTitle("Silicon and Germanium Diodes.");
    tmg->Draw("ALP");
    legend->Draw();

    TH1F *hbs = tmg->GetHistogram();
    hbs->SetXTitle("Voltage");
    hbs->SetYTitle("Current");
    hbs->SetLabelSize(0.03,"X");
    hbs->SetLabelSize(0.03,"Y");
}
