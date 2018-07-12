/*
 * Author: K.v.Sturm
 * Date: 12.07.2018
 * Description: plot simulated spectra one over the other
 * Compilation: g++ -O3 -std=c++1y $(root-config --cflags) OplotBKGSpectra.cxx $(root-config --libs) -o OplotBKGSpectra
 */

// c/c++
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>

// cern root
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TLegend.h"

using namespace std;

void Usage();
void rootlogon( string style = "short" );

int main( int argc, char * argv[] )
{
    rootlogon();

    // get command line arguments
    vector<string> args;
    for ( int i = 0; i < argc; ++i ) args.push_back( argv[i] );

    // user requested help or made input error
    if ( argc < 2 ) { Usage(); return 1; }
    else if ( find(args.begin(), args.end(), "--help") != args.end() ||
              find(args.begin(), args.end(), "-h")     != args.end() ) { Usage(); return 1; }

    // check OPTIONS
    // get input filelist
    string filelistname;
    auto result = find( args.begin(), args.end(), "--input" );
    if ( result != args.end() ) filelistname = *(result+1);
    else { Usage(); return 1; }
    // common name of histogram to plot
    string hname;
    result = find( args.begin(), args.end(), "--histo" );
    if ( result != args.end() ) hname = *(result+1);
    else { Usage(); return 1; }

    // read list of files in a vector
    string directory, filename;
    vector<string> filelist;

    ifstream iflist( filelistname );
    iflist >> directory >> filename;
    cout << directory << endl;

    while( !iflist.eof() )
    {
        cout << "\t" << filename << endl;
        filelist.push_back(filename);
        iflist >> filename;
    }

    // define color seuqence
//    vector<int> color_sequence = {kMagenta, kViolet, kBlue, kAzure, kCyan, kTeal, kGreen, kSpring, kYellow, kOrange, kRed, kPink};
    vector<int> color_sequence = {kBlue, kMagenta, kTeal};
    vector<string> det = {
      "GD91A", "GD35B", "GD02B",  "GD00B", "GD61A", "GD89B", "-",
      "GD91C", "ANG5",  "RG1",    "ANG3",  "GD02A", "GD32B", "GD32A",
      "GD32C", "GD89C", "GD61C",  "GD76B", "GD00C", "GD35C", "GD76C",
      "GD89D", "GD00D", "GD79C",  "GD35A", "GD91B", "GD61B", "ANG2" ,
      "RG2",   "ANG4",  "GD00A",  "GD02C", "GD79B", "GD91D", "GD32D",
      "GD89A", "ANG1",  "GTF112", "GTF32", "GTF45"
    };

    // save histograms
    map<string,TH1D> histograms;

    // plot the histograms
    int ci = 0;
    for( auto filename : filelist )
    {
        // parse filename
        int ind1 = filename.rfind("-");
        int ind2 = filename.rfind(".root");
        int ind3 = filename.find("-");
        string isotope = filename.substr(ind1+1, ind2-ind1-1);
        string location = filename.substr(ind3+1, ind1-ind3-1);
        location.replace(location.find("-"),1,":");
        string label = isotope; label += ":"; label += location;

        string cname = hname; cname += "_clone"; cname += to_string(ci);

        // get hist
        TFile irfile( (directory+filename).c_str(), "READ");
        if(!irfile.IsOpen()) cout << "File not Found: " << directory << filename << endl;
        else                 cout << "File Found: " << directory << filename << endl;

        histograms[label] = *(TH1D*)irfile.Get(hname.c_str())->Clone(cname.c_str());

        // close file
        irfile.Close();
        ci++;
    }

    // create canvas
    TCanvas c("canvas","pdfs");
    c.SetMargin(0.08,0.01,0.15,0.01);
    TLegend l(0.1,0.7,0.5,0.97);
    l.SetMargin(0.2);

    ci = 0;
    for( auto & hist : histograms )
    {
        // set x-axis labels
        int nbins = hist.second.GetNbinsX();
        for (int b = 1; b <= nbins; ++b) {
            hist.second.GetXaxis()->SetBinLabel(b, det[b-1].c_str());
        }
        hist.second.GetXaxis()->LabelsOption("v");
        hist.second.GetYaxis()->SetTitle("a.u.");

        // normalize
        hist.second.Scale(1./hist.second.Integral());

        // set histogram attributes
        hist.second.SetLineColor( color_sequence.at(ci)+1 );
        hist.second.SetLineWidth(2);

        // add legend entry
        l.AddEntry(&hist.second,hist.first.c_str(),"l");

        if(ci == 0)
        {
            hist.second.GetYaxis()->SetRangeUser(0,0.138);
            hist.second.DrawClone("hist");
        }
        else        hist.second.DrawClone("histsame");

        ci++;
    }

    // draw legend
    l.Draw();
    c.Print("test.png");
    c.Print("test.pdf");

    TFile outfile( "test.root", "RECREATE" );
    c.Write();
    for( auto hist : histograms ) hist.second.Write();
    outfile.Close();

    return 0;
}

void Usage()
{
    cout << "USAGE   : ./OplotBKGSpectra \n\n";
    cout << "OPTIONS : \n\n";
    cout << "       required:   --input <filelist>  : txt file with directory and list of files\n";
    cout << "                   --histo <histoname> : name of histogram to plot\n\n";
    return;
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
    gerdaStyle->SetLineScalePS(2);

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

    return;
}
