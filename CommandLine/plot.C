{
    TCanvas *Hobbes = new TCanvas("Dist","A test",5,5,1200,600);
    Hobbes->cd();
    TPad    *Calvin = new TPad("Calvin","Silly",0.02,0.02,0.99,0.99,10);
    Calvin->Draw();
    Calvin->cd();

    Calvin->SetGrid();

    TGraph *IV = new TGraph("keithley.log");
    IV->SetMarkerStyle(22);
    IV->Draw("ALP");
    TH1F *hbs = IV->GetHistogram();
    hbs->SetXTitle("Voltage");
    hbs->SetYTitle("Current");
    hbs->SetLabelSize(0.03,"X");
    hbs->SetLabelSize(0.03,"Y");
}
