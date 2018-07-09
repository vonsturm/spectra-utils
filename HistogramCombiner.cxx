/*
 * Author      : K.v.Sturm
 * Date        : 06.07.2018
 * Note        : combined alpha spectra
 * Compilation : g++ -O3 -std=c++1y $(root-config --cflags) HistogramCombiner.cxx $(root-config --libs) -o HistogramCombiner
*/


// c/c++
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

// root cern
#include "TH1D.h"
#include "TFile.h"

using namespace std;

int main( int argc, char* argv[] )
{
    cout << "Combining histograms" << endl;

    // read in file names
    vector<string> files;
    for( int i = 1; i < argc-1; i++ ) files.push_back( argv[i] );

    // vector to hold summed histograms
    map<string,TH1D> hmap;

    // loop over files
    for( auto file : files )
    {
        cout << "\t" << file << endl;
        TFile rootfile( file.c_str() );

        // loop over histograms
        for( int dl = 0; dl <= 1000; dl += 100 )
        {
            string hname  = "hist_dl"; hname  += to_string(dl); hname  += "nm";
            string hcname = "hist_dl"; hcname += to_string(dl); hcname += "nm_copy";

            TH1D h = *(TH1D*)rootfile.Get( hname.c_str() );

            if( hmap.find(hname) == hmap.end() ) hmap[hname] = *(TH1D*)h.Clone( hcname.c_str() );
            else
            {
                // loop over bins
                int nbins = h.GetNbinsX();
                for( int b = 1; b <= nbins; b++ )
                {
                    //double center  = h.GetBinCenter(b);
                    double content = h.GetBinContent(b);
                    //int bin = hmap[hname].FindBin(center);
                    hmap[hname].Fill(b,content);
                }
            }
        }

        rootfile.Close();
    }

    // open output file and writing histograms
    string output = argv[argc-1];
    cout << "Output\n\t" << output << endl;

    TFile outfile( output.c_str(), "RECREATE" );
    for( int dl = 0; dl <= 1000; dl += 100 )
    {
        string hname  = "hist_dl"; hname  += to_string(dl); hname  += "nm";
        hmap[hname].SetName(hname.c_str());
        hmap[hname].SetTitle(hname.c_str());
        hmap[hname].Write();
    }
    outfile.Close();

    return 0;
}
