// main.cpp
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
// ./gaston -i Chemical_340 -c Chemical_340_class.txt -o output -s stat
#include <iostream>
#include <fstream>
#include "database.h"
#include "path.h"
#include "misc.h"
#include "graphstate.h"
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "tcheck.h"

using namespace std;

Frequency minfreq = 1;
Database database, database_original;
Statistics statistics;
bool dooutput = false;
int phase = 3;
int maxsize = ( 1 << ( sizeof(NodeId)*8 ) ) - 1; // safe default for the largest allowed pattern
FILE *output;
ofstream OFS;

int N = 0, N_TOTAL = 0;
double ALPHA = 0.05, THRESHOLD, COUNT = 0, DELTA;
vector<int> CLASS_VEC;

bool RERUN = false;

void Statistics::print () {
  int total = 0, total2 = 0, total3 = 0;
  for ( int i = 0; i < frequenttreenumbers.size (); i++ ) {
    cout << "Frequent " << i + 2
         << " cyclic graphs: " << frequentgraphnumbers[i]
         << " real trees: " << frequenttreenumbers[i]
         << " paths: " << frequentpathnumbers[i]
         << " total: " << frequentgraphnumbers[i] + frequenttreenumbers[i] + frequentpathnumbers[i] << endl;
    total += frequentgraphnumbers[i];
    total2 += frequenttreenumbers[i];
    total3 += frequentpathnumbers[i];
  }
  cout << "TOTAL:" << endl
       << "Frequent cyclic graphs: " << total << " real trees: " << total2 << " paths: " << total3 << " total: " << total + total2 + total3 << endl;
}

void puti ( FILE *f, int i ) {
  char array[100];
  int k = 0;
  do {
    array[k] = ( i % 10 ) + '0';
    i /= 10;
    k++;
  }
  while ( i != 0 );
  do {
    k--;
    putc ( array[k], f );
  } while ( k );
}

void runGaston() {
  cerr << "Edgecount" << endl;
  database.edgecount ();
  cerr << "Reorder" << endl;
  database.reorder ();
  initLegStatics ();
  graphstate.init ();
  for ( int i = 0; i < database.nodelabels.size (); i++ ) {
    if ( database.nodelabels[i].frequency >= minfreq &&
         database.nodelabels[i].frequentedgelabels.size () ) {
      Path path ( i );
      path.expand ();
    }
  }
}

void clearGaston() {
  /*
  minfreq = 1;
  COUNT = 0;
  THRESHOLD = ALPHA / ((double)N / (double)N_TOTAL);
  */
  statistics.frequenttreenumbers.clear();
  statistics.frequentpathnumbers.clear();
  statistics.frequentgraphnumbers.clear();
  statistics.patternsize = 0;
}

main ( int argc, char *argv[] ) {
  clock_t t1 = clock ();
  cerr << "GASTON GrAph, Sequences and Tree ExtractiON algorithm" << endl;
  cerr << "Version 1.0 with Occurrence Lists" << endl;
  cerr << "Siegfried Nijssen, LIACS, 2004" << endl;

  // In "class_labels_file", 1 is minor class and 0 is major class
  char *input_file, *class_file, *output_file, *stat_file;
  bool flag_in = false, flag_out = false;

  char opt;
  while ( ( opt = getopt ( argc, argv, "m:tpa:i:c:o:s:" ) ) != -1 ) {
    switch ( opt ) {
    case 'm': maxsize = atoi ( optarg ) - 1; break;
    case 't': phase = 2; break;
    case 'p': phase = 1; break;
    case 'a': ALPHA = atof(optarg); break;
    case 'i': input_file = optarg; flag_in = true; break;
    case 'c': class_file = optarg; break;
    case 'o': output_file = optarg; flag_out = true; break;
    case 's': stat_file = optarg; break;
    }
  }

  if (!flag_in) {
    cerr << "Parameters:" << endl
	 << "-m [Maximum size of each subgraph]" << endl
	 << "-a [Significance level]" << endl
	 << "-i [input file for graphs]" << endl
	 << "-c [input file for class labels]" << endl
	 << "-o [output file for subgraphs]" << endl
	 << "-s [output file for statistics]" << endl
	 << endl;
    return 1;
  }

  cerr << "Read" << endl;
  FILE *input = fopen ( input_file, "r" );
  database.read ( input );
  rewind( input );
  database_original.read ( input );
  fclose ( input );

  readClass(class_file);
  cout << "N_TOTAL = " << N_TOTAL << endl;
  cout << "N = " << N << endl;

  if (flag_out) {
    // dooutput = true;
    output = fopen ( output_file, "w" );
    OFS.open(output_file);
  }

  // compute the first threshold for minfreq = 1
  THRESHOLD = ALPHA / ((double)N / (double)N_TOTAL);
  runGaston();

  clock_t t2 = clock ();

  // statistics.print ();
  cout << "Approximate total runtime: " << ( (float) t2 - t1 ) / CLOCKS_PER_SEC << "s" << endl;
  // if (flag_out)
  //   fclose ( output );
  DELTA = ALPHA / COUNT; // Corrected significance threshold for each test
  cout << "Corrected significance threshold: " << DELTA << endl;

  // re-run Gaston
  cout << "Rerun Gaston with minfreq: " << minfreq << endl;
  RERUN = true;
  clearGaston();
  runGaston();

  clock_t t3 = clock ();

  statistics.print ();
  if (flag_out) {
    fclose ( output );
    OFS.close();
  }
}
