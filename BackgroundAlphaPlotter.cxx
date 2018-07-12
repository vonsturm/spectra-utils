/*
 * Author      : K.v.Sturm
 * Date        : 09.07.2018
 * Note        : draw gerda-bkg-model/alpha fits
 * Compilation : g++ -O3 -std=c++1y $(root-config --cflags) BackgroundAlphaPlotter.cxx $(root-config --libs) -o BackgroundAlphaPlotter
 */


// c/c++
#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>

// cern root
#include "TROOT.h"
#include "TFile.h"
#include "TH1D.h"
#include "TStyle.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TPad.h"
#include "TMath.h"
#include "Math/ProbFuncMathCore.h"

using namespace std;

void Usage();
string MakeLabel( TString title );
void rootlogon( string style = "short" );

int main( int argc, char* argv[] )
{
    //*******************************************************//
    // get command line arguments
    vector<string> args;
    for ( int i = 0; i < argc; ++i ) args.push_back( argv[i] );

    // user requested help or made input error
    if ( argc < 2 ) { Usage(); return 1; }
    else if ( find(args.begin(), args.end(), "--help") != args.end() ||
              find(args.begin(), args.end(), "-h")     != args.end() ) { Usage(); return 1; }

    // check OPTIONS
    // get input file name
    string input_filename;
    auto result = find( args.begin(), args.end(), "--input" );
    if ( result != args.end() ) input_filename = *(result+1);
    else { Usage(); return 1; }
    // output filename
    string output_filename;
    result = find( args.begin(), args.end(), "--output" );
    if ( result != args.end() ) output_filename = *(result+1);
    else { Usage(); return 1; }
    // binning
    int binning = 10;
    result = find( args.begin(), args.end(), "--binning" );
    if ( result != args.end() ) binning = stoi( *(result+1) );
    result = find( args.begin(), args.end(), "-b" );
    if ( result != args.end() ) binning = stoi( *(result+1) );
    // colorsequence
    int colors = 1;
    result = find( args.begin(), args.end(), "--color-sequence" );
    if ( result != args.end() ) colors = stoi( *(result+1) );
    result = find( args.begin(), args.end(), "-cs" );
    if ( result != args.end() ) colors = stoi( *(result+1) );
    // xmin xmax ymin ymax
    double xmin = 3500., xmax = 6000., ymin = 0.1, ymax = 1e3;
    result = find( args.begin(), args.end(), "-x" );
    if ( result != args.end() ) xmin = stod( *(result+1) );
    result = find( args.begin(), args.end(), "-X" );
    if ( result != args.end() ) xmax = stod( *(result+1) );
    result = find( args.begin(), args.end(), "-y" );
    if ( result != args.end() ) ymin = stod( *(result+1) );
    result = find( args.begin(), args.end(), "-Y" );
    if ( result != args.end() ) ymax = stod( *(result+1) );
    // canvas format
    string style = "long";
    result = find( args.begin(), args.end(), "--style" );
    if ( result != args.end() ) style = *(result+1);
    // draw residuals
    bool res_flag = false;
    result = find( args.begin(), args.end(), "-r" );
    if ( result != args.end() ) res_flag = true;
    //*******************************************************//

    // root style
    rootlogon(style);
    gStyle->SetLineScalePS(1.4);

    // open file
    TFile file( input_filename.c_str(), "READ" );

    // get histograms
    file.cd("results_canvas");
    TIter next(gDirectory->GetListOfKeys());
    TKey * key;
    TH1D hdata, hmc;

    while( key = (TKey*)next() )
    {
        TH1D h = *(TH1D*)key->ReadObj();
        string hname = h.GetName();

        // Find
        if(     hname.find("hSum_fine_") != string::npos) hdata = *(TH1D*)h.Clone("hdata");
        else if(hname.find("hMC_fine_")  != string::npos) hmc   = *(TH1D*)h.Clone("hmc");
   }

    // get components
    file.cd("components");
    next = gDirectory->GetListOfKeys();
    vector<TH1D> hcomp;
    int c = 0; // component counter

    while( key = (TKey*)next() )
    {
        TH1D h = *(TH1D*)key->ReadObj();
        string hname = h.GetName();

        // Find
        if( regex_match(hname, regex(".*_p[0-9]c[0-9]_fine_.*")) )
        {
            string compname = "hcomp_"; compname += to_string(c++);
            hcomp.push_back( *(TH1D*)h.Clone(compname.c_str()) );
        }
    }
    file.Close();

    // color sequence
    vector<int> sequence1 = { kViolet, kMagenta, kPink, kRed, kOrange, kYellow, kSpring+1, kGreen, kTeal, kCyan, kAzure, kBlue }; // rainbow
    vector<int> sequence2 = {                    kPink, kRed, kOrange,                             kTeal,        kAzure, kBlue }; // enrBEGe
    vector<int> sequence3 = {          kMagenta, kPink, kRed, kOrange, kYellow, kSpring+1,         kTeal,        kAzure, kBlue }; // enrCoax
    vector<int> sequence4 = { kViolet, kMagenta, kPink, kRed, kOrange,          kSpring+1,         kTeal, kCyan, kAzure, kBlue }; // natCoax
    vector<int> & sequence = sequence1;
    if(     colors == 1) sequence = sequence1;
    else if(colors == 2) sequence = sequence2;
    else if(colors == 3) sequence = sequence3;
    else if(colors == 4) sequence = sequence4;

    // plot
    TCanvas canvas("c","fit result alphas");
//    canvas.SetRightMargin(0.02);
    TPad mainpad( "mainpad", "fit result pad", 0, 0.3, 1, 1 );
    TPad respad( "respad", "residuals pad", 0, 0, 1, 0.3);
    if(res_flag)
    {
        canvas.Size( canvas.GetWindowWidth(), canvas.GetWindowHeight()*4./3. );
        mainpad.SetMargin(0.06,0.03,0,0.01); respad.SetMargin(0.06,0.03,0.3,0);
        mainpad.Draw(); respad.Draw();
        mainpad.cd();
    }

    double lx = 0.1, lX = 0.3, ly = 0.7, lY = 0.9;
    if( style == "short" )     { lx = 0.1; lX = 0.2; ly = 0.1; lY = 0.2; }
    else if( style == "long" ) { lx = 0.07; lX = 0.6; ly = 0.65; lY = 0.98; }
    TLegend l( lx,ly,lX,lY );
    l.SetNColumns(2);
    l.SetTextFont(42);
    l.SetTextSize(0.04);

    hdata.GetXaxis()->SetRangeUser(xmin,xmax);
    hdata.GetYaxis()->SetRangeUser(ymin,ymax);
    hdata.GetYaxis()->SetTitle(Form("cts / %ikeV",binning));
    hdata.SetMarkerStyle(21); hdata.SetMarkerSize(0.5);
    hdata.SetFillColor(kGray); hdata.SetLineColor(kGray);
    hdata.Rebin(binning); hdata.Draw("hist");
    hmc.SetLineColor(kBlack); hmc.SetLineWidth(2);
    hmc.Rebin(binning); hmc.Draw("histsame");
    l.AddEntry(&hdata,"data","f");
    l.AddEntry(&hmc,"fit","l");

    int i = 0; //color iterator
    for( auto & h : hcomp )
    {
        h.SetLineColor( sequence.at(i++)+1 );
        h.SetLineWidth(2);
        h.Rebin(binning);
        h.DrawClone("histsame");
        string label = MakeLabel(h.GetTitle());
        l.AddEntry(&h,label.c_str(),"l");
    }

    l.Draw();

    // logscale
    if(!res_flag) gPad->SetLogy();
    else          mainpad.SetLogy();

    // compute residuals
    auto res     = dynamic_cast<TH1D*>( hdata.Clone("h_res")       );
    auto res_b3u = dynamic_cast<TH1D*>( hdata.Clone("h_band3_up")  );
    auto res_b3l = dynamic_cast<TH1D*>( hdata.Clone("h_band3_low") );
    auto res_b2u = dynamic_cast<TH1D*>( hdata.Clone("h_band2_up")  );
    auto res_b2l = dynamic_cast<TH1D*>( hdata.Clone("h_band2_low") );
    auto res_b1u = dynamic_cast<TH1D*>( hdata.Clone("h_band1_up")  );
    auto res_b1l = dynamic_cast<TH1D*>( hdata.Clone("h_band1_low") );

    int minbin = res->GetXaxis()->FindBin(xmin);
    int maxbin = res->GetXaxis()->FindBin(xmax);

    for (int b = minbin; b <= maxbin; b++)
    {
        double d = hdata.GetBinContent(b);
        double m = hmc  .GetBinContent(b);
        double s = TMath::NormQuantile(ROOT::Math::poisson_cdf(d, m));

        res    ->SetBinContent(b, s);
        res_b3u->SetBinContent(b, +3); res_b3l->SetBinContent(b, -3);
        res_b2u->SetBinContent(b, +2); res_b2l->SetBinContent(b, -2);
        res_b1u->SetBinContent(b, +1); res_b1l->SetBinContent(b, -1);
    }

    // set residual colors
    int col3 = kOrange-9, col2 = kYellow-9, col1 = kSpring+1;
    res_b3u->SetFillColor(col3); res_b3l->SetFillColor(col3);
    res_b2u->SetFillColor(col2); res_b2l->SetFillColor(col2);
    res_b1u->SetFillColor(col1); res_b1l->SetFillColor(col1);
    res_b3u->SetLineColor(col3); res_b3l->SetLineColor(col3);
    res_b2u->SetLineColor(col2); res_b2l->SetLineColor(col2);
    res_b1u->SetLineColor(col1); res_b1l->SetLineColor(col1);

    // draw residuals
    if(res_flag)
    {
        respad.cd();
        double rl = -3.5, ru = 3.5;
        res->GetYaxis()->SetRangeUser(rl,ru);
        res_b3u->GetYaxis()->SetRangeUser(rl,ru); res_b3l->GetYaxis()->SetRangeUser(rl,ru);
        res_b2u->GetYaxis()->SetRangeUser(rl,ru); res_b2l->GetYaxis()->SetRangeUser(rl,ru);
        res_b1u->GetYaxis()->SetRangeUser(rl,ru); res_b1l->GetYaxis()->SetRangeUser(rl,ru);
        res_b3u->GetXaxis()->SetTitleOffset(3.0);
        res_b3u->GetYaxis()->SetNdivisions(305);
        res_b3u->Draw("hist");     res_b2u->Draw("histsame"); res_b1u->Draw("histsame");
        res_b3l->Draw("histsame"); res_b2l->Draw("histsame"); res_b1l->Draw("histsame");
        res->Draw("histpsame");
        respad.RedrawAxis("");
    }

    canvas.Update();

    // print pdf
    int index = output_filename.find_last_of(".");
    string pdf_filename = output_filename.substr(0,index);
    pdf_filename += ".pdf";
    canvas.Print(pdf_filename.c_str());

    // write to TFile
    TFile outfile( output_filename.c_str(), "RECREATE" );
    canvas.Write("plot");
    hdata.Write();
    hmc.Write();
    for( auto h : hcomp ) h.Write();
    res_b3u->Write(); res_b3l->Write();
    res_b2u->Write(); res_b2l->Write();
    res_b1u->Write(); res_b1l->Write();
    outfile.Close();

    return 0;
}

void Usage()
{
    cout << "Plot alpha model\n\n";
    cout << "USAGE   : ./BackgroundAlphaPlotter [OPTIONS]\n\n";
    cout << "OPTIONS :\n\n";
    cout << "    required :  --input <filename>         : input root file\n";
    cout << "                --output <filename>        : output root file\n\n";
    cout << "    optional :  --binning -b <int>         : binning of histograms\n";
    cout << "                                             need to match!\n";
    cout << "                --color-sequence -cs <int> : choose a color sequence number (1 rainbow, 2 enrBEGe, 3 enrCoax, 4 natCoax)\n";
    cout << "                -xXyY <double>             : x/y min/max values e.g. -x 0. -X 8000.\n";
    cout << "                --style <style>            : set canvas style (short,long)\n";
    cout << "                -r                         : draw residuals as normalized quantiles (brazilian plot)\n";
    return;
}

string MakeLabel( TString title )
{
    string label;

    if(title.Contains("Po210"))       label = "^{210}Po";
    else if(title.Contains("Ra226"))  label = "^{226}Ra";
    else if(title.Contains("Rn222"))  label = "^{222}Rn,^{218}Po,^{214}Po";
    else if(title.Contains("offset")) label = "f(E) = p_{0}";
    else if(title.Contains("slope"))  label = "g(E) = p_{1} E";

    if(title.Contains("LAr"))         label += " LAr ";
    else if(title.Contains("pPlus"))  label += " p^{+} ";

    int i = title.Index("00nm");
    if(i>0) label += (string)title(i-1,5);

    return label;
}

// this sets the GERDA default style for spectra plots
// ("short", 1 panel plots)
// ("long",  1 panel plots)
void rootlogon( string style )
{
    cout << "Loading GERDA ROOT-logon...";

    int font = 43;
    int fontsize = 22;

    // define and load gerda plot style
    TStyle *gerdaStyle  = new TStyle("gerda-style"," GERDA specific ROOT style");

    gerdaStyle->SetColorModelPS(1);
    gerdaStyle->SetLineScalePS(1);

    // use plain black on white colors
    gerdaStyle->SetFrameBorderMode(0);
    gerdaStyle->SetCanvasBorderMode(0);
    gerdaStyle->SetPadBorderMode(0);
    gerdaStyle->SetPadColor(0);
    gerdaStyle->SetCanvasColor(0);
    gerdaStyle->SetStatColor(0);
    gerdaStyle->SetPalette(kGreyScale);

    // set the paper & margin sizes
    gerdaStyle->SetPaperSize(20,26);
    if(      style == "short" )
    {
        gerdaStyle->SetPadLeftMargin(0.08);
        gerdaStyle->SetPadRightMargin(0.05);
    }
    else if( style == "long"  )
    {
        gerdaStyle->SetPadLeftMargin(0.053);
        gerdaStyle->SetPadRightMargin(0.02);
    }
    gerdaStyle->SetPadBottomMargin(0.1);
    gerdaStyle->SetPadTopMargin(0.011);

    // default canvas size
    if( style == "short" )
    {
        gerdaStyle->SetCanvasDefH(600);
        gerdaStyle->SetCanvasDefW(900);
    }
    else if( style == "long" )
    {
        gerdaStyle->SetCanvasDefH(550);
        gerdaStyle->SetCanvasDefW(1200);
    }

    // default font
    gerdaStyle->SetTextFont(font);
    gerdaStyle->SetTextSize(fontsize);

    // axis labels
    gerdaStyle->SetLabelFont(font, "XY");
    gerdaStyle->SetLabelSize(fontsize, "XY");
    gerdaStyle->SetTitleFont(43, "XY");
    gerdaStyle->SetTitleXSize(fontsize);
    gerdaStyle->SetTitleYSize(fontsize);
    gerdaStyle->SetTitleOffset(1, "X");
    if(      style == "short") gerdaStyle->SetTitleOffset(1, "Y");
    else if( style == "long" ) gerdaStyle->SetTitleOffset(0.67, "Y");

    // ticks
    gerdaStyle->SetTickLength(0.01, "Y");

    // grid
    gerdaStyle->SetGridStyle(1);
    gerdaStyle->SetGridColor(kGray);

    // legend
    gerdaStyle->SetLegendFont(font);
    gerdaStyle->SetLegendTextSize(fontsize);
    gerdaStyle->SetLegendBorderSize(0);

    // do not display any of the standard histogram decorations
    gerdaStyle->SetOptStat(false);
    gerdaStyle->SetOptTitle(false);
    gerdaStyle->SetOptFit(0);

    gROOT->ForceStyle();

    cout << " default style set to \"gerda-style\"\n";
    gROOT->SetStyle("gerda-style");

    /* Use this to draw the GERDA watermark
    auto gerdawtr = new TLatex(0.992, 0.76, "GERDA 18-06");
    gerdawtr->SetNDC();
    gerdawtr->SetTextFont(font);
    gerdawtr->SetTextSizePixels(15);
    gerdawtr->SetTextAngle(90);
    gerdawtr->Draw();
    */

    /* use this to draw the blinding box
    int low_edge = h->GetXaxis()->GetBinLowEdge(h->GetXaxis()->FindBin(2014));
    int up_edge = h->GetXaxis()->GetBinUpEdge(h->GetXaxis()->FindBin(2064));
    auto box = new TBox(low_edge, h->GetMinimum(), up_edge, h->GetMaximum());
    box->SetFillColorAlpha(kBlack, 0.25); // this not to cover the overlaying histograms
    box->SetFillStyle(1001);
    box->SetLineWidth(0);
    auto line = new TLine(2039, dataGe->GetMinimum(), 2039, dataGe->GetMaximum());
    line->SetLineStyle(2);
    box->Draw();
    */
}
