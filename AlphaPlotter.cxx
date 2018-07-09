/*
 * Author      : K.v.Sturm
 * Date        : 06.07.2018
 * Note        : draw spcetra made using alpha simulation macros in gerda-mage-sim
 * Compilation : g++ -O3 -std=c++1y $(root-config --cflags) AlphaPlotter.cxx $(root-config --libs) -o AlphaPlotter
*/


// c/c++
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

// root
#include "TROOT.h"
#include "TFile.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1D.h"
#include "TLine.h"
#include "TStyle.h"

using namespace std;

// usage
void Usage();

int main( int argc, char * argv[] )
{
    // root style
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    gStyle->SetLineScalePS(1);

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

    // inset flag (optional)
    string inset_pos = "none";
    result = find( args.begin(), args.end(), "--inset" );
    if ( result != args.end() ) inset_pos = *(result+1);
    // xmin main pad (optional)
    double xmin = 0.;
    result = find( args.begin(), args.end(), "--x-min" );
    if ( result != args.end() ) xmin = stod(*(result+1));
    result = find( args.begin(), args.end(), "-x" );
    if ( result != args.end() ) xmin = stod(*(result+1));
    // xmax main pad (optional)
    double xmax = 8000.;
    result = find( args.begin(), args.end(), "--x-max" );
    if ( result != args.end() ) xmax = stod(*(result+1));
    result = find( args.begin(), args.end(), "-X" );
    if ( result != args.end() ) xmax = stod(*(result+1));
    // ymin main pad (optional)
    double ymin = 1.e3;
    result = find( args.begin(), args.end(), "--y-min" );
    if ( result != args.end() ) ymin = stod(*(result+1));
    result = find( args.begin(), args.end(), "-y" );
    if ( result != args.end() ) ymin = stod(*(result+1));
    // ymax main pad (optional)
    double ymax = 5.e8;
    result = find( args.begin(), args.end(), "--y-max" );
    if ( result != args.end() ) ymax = stod(*(result+1));
    result = find( args.begin(), args.end(), "-Y" );
    if ( result != args.end() ) ymax = stod(*(result+1));
    // xmin inset pad (optional)
    double xmin_inset = 0.;
    result = find( args.begin(), args.end(), "--x-min-inset" );
    if ( result != args.end() ) xmin_inset = stod(*(result+1));
    result = find( args.begin(), args.end(), "-xi" );
    if ( result != args.end() ) xmin_inset = stod(*(result+1));
    // xmax inset pad (optional)
    double xmax_inset = 8000.;
    result = find( args.begin(), args.end(), "--x-max-inset" );
    if ( result != args.end() ) xmax_inset = stod(*(result+1));
    result = find( args.begin(), args.end(), "-Xi" );
    if ( result != args.end() ) xmax_inset = stod(*(result+1));


    string isotope = input_filename.substr(4,5);
    string loc = input_filename.substr(10,1);
    string location = "unknown";
    if( loc == 'p' ) location = "pPlus";
    else if( loc == 'L' ) location = "LAr";
    string output_filename = "plot-"; output_filename += isotope; output_filename += location; output_filename += ".root";
    string outpdf = "plot-"; outpdf += isotope; outpdf += location; outpdf += ".pdf";

    cout << "Plotting simulated alpha spectra for: " << isotope << " " << location << endl;
    cout << "Input: " << input_filename << endl;
    cout << "Output: " << output_filename << endl;

    // open input file
    TFile infile( input_filename.c_str() );

    // load histograms in vector
    vector<TH1D*> v_histos;

    for( int dl = 0; dl <= 1000; dl+=100 )
    {
        string histname = "hist_dl"; histname += to_string(dl); histname += "nm";
        v_histos.push_back( (TH1D*) infile.Get(histname.c_str()) );
    }

    // create canvas
    string ctitle = isotope; ctitle += " " + location + ": dl 0nm - 1000nm";
    TCanvas c( "hcan", ctitle.c_str(), 1000, 500 );
    TPad mainpad( "mainpad", "", 0.01, 0.01, 0.99, 0.99 );
    mainpad.SetMargin(0.06,0.03,0.1,0.05);
    mainpad.Draw();

    // draw inset for certain isotopes to see the effect of p+ thickness better
    double ipos = 0.;
    if(inset_pos == "top") ipos = 0.45;
    TPad inset("inset", "inset pad", 0.13, 0.15+ipos, 0.4, 0.42+ipos);
    inset.SetMargin(0.01,0.01,0.01,0.01);
    if(inset_pos != "none") inset.Draw();

    TLine linel(xmin_inset,ymin,xmin_inset,ymax);
    TLine liner(xmax_inset,ymin,xmax_inset,ymax);
    linel.SetLineColor(kGray); liner.SetLineColor(kGray);
    linel.SetLineStyle(2);     liner.SetLineStyle(2);
    linel.SetLineWidth(2);     liner.SetLineWidth(2);

    TLegend l( 0.42, 0.15+ipos, 0.69, 0.42+ipos );
    l.SetLineColor(kWhite);
    l.SetNColumns(2);

    // color index
    int i = 0;
    vector<int> sequence = { kRed, kOrange, kYellow, kSpring, kGreen, kTeal, kCyan, kAzure, kBlue, kViolet, kMagenta, kPink };

    // draw all histograms
    for( auto h : v_histos )
    {
        string dl = to_string(i*100); dl += "nm";
        string htitle = isotope; htitle += " " + location + ": dl " + dl;

        h->SetLineColor( sequence.at(i++)+1 );
        h->SetLineWidth( 2 );
        h->SetTitle( htitle.c_str() );
        h->GetXaxis()->SetTitle( "energy (keV)" );
        h->GetYaxis()->SetTitle( "cts/keV" );
        h->GetYaxis()->SetTitleOffset(0.8);
        h->GetYaxis()->SetRangeUser(ymin,ymax);
        l.AddEntry( h, dl.c_str(), "l" );

        if(inset_pos != "none")
        {
            inset.cd();
            h->GetXaxis()->SetRangeUser(xmin_inset,xmax_inset);
            h->GetXaxis()->SetNdivisions(0);
            h->GetYaxis()->SetNdivisions(0);
            if( i==0 ) h->DrawClone( "hist" );
            else       h->DrawClone( "histsame" );
        }

        mainpad.cd();
        h->GetXaxis()->SetRangeUser(xmin,xmax);
        h->GetXaxis()->SetNdivisions(510);
        h->GetYaxis()->SetNdivisions(510);
        if( i==0 ) h->DrawClone( "hist" );
        else       h->DrawClone( "histsame" );
    }

    mainpad.cd();

    // draw inset limits in main pad
    if(inset_pos != "none")
    {
        linel.Draw(); liner.Draw();
        l.AddEntry(&linel,"inset","l");
    }

    // draw legend
    l.Draw();

    // set logscale
    mainpad.SetLogy();
    inset.SetLogy();

    // write pdf to disc
    c.Print(outpdf.c_str());

    // open output file
    TFile outfile( output_filename.c_str(), "RECREATE" );
    c.Write();
    for( auto h : v_histos ) h->Write();

    // close root files
    outfile.Close();
    infile.Close();

    return 0;
}

// Prints usage information to shell
void Usage()
{
    cout << "Create plot of 0nm to 100nm p+ surface alpha simulation\n\n";
    cout << "USAGE   : ./AlphaPlotter [OPTIONS]\n\n";
    cout << "EXAMPLE : ./AlphaPlotter --inset top -x 0 -X 7000 -y 0.1 -Y 1e3 -xi 4800 -Xi 5600 --input default.root\n\n";
    cout << "OPTIONS :\n\n";
    cout << "    required :  --input <filename>    : input txt file containing a list of\n"
         << "                                        result files and binning \n";
    cout << "    optional :  --inset <position>    : draw inset with all histograms which can be\n" <<
                                                     "zoomed to a different region for evidencing features\n" <<
                                                     "(none (default), top, bottom)";
    cout << "                --x-min       -x      : x-min for main pad\n";
    cout << "                --x-max       -X      : x-max for main pad\n";
    cout << "                --y-min       -y      : y-min for main pad\n";
    cout << "                --y-max       -Y      : y-max for main pad\n";
    cout << "                --x-min-inset -xi     : x-min for inset pad\n";
    cout << "                --x-max-inset -Xi     : x-max for inset pad\n";
    return;
}

