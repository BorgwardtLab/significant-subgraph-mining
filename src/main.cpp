#include <iostream>
#include <fstream>
#include "database.h"
#include "path.h"
#include "misc.h"
#include "graphstate.h"
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "var_declare.h"
#include "tcheck.h"
#include "permutation.h"
#include "wy.h"

using namespace std;

Frequency minfreq = 1;
Database database, database_original;
Statistics statistics;
bool dooutput = false;
int phase = 3;
int maxsize = INT_MAX; // safe default for the largest allowed pattern
FILE *output;
ofstream OFS;

int N_SMALL = 0, N_TOTAL = 0;
double ALPHA = 0.05, THRESHOLD, COUNT = 0, DELTA;
vector<int> CLASS_VEC;
vector<Tid> OCC_VEC;
time_t SEED = 0;

bool RERUN = false;
bool WYLIGHT = false;

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
  // cerr << "Edgecount" << endl;
  database.edgecount ();
  // cerr << "Reorder" << endl;
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
  statistics.frequenttreenumbers.clear();
  statistics.frequentpathnumbers.clear();
  statistics.frequentgraphnumbers.clear();
  statistics.patternsize = 0;
}

int main ( int argc, char *argv[] ) {
  clock_t t1 = clock ();
  cerr << "Start Significant Subgraph Mining using GASTON" << endl
       << "Codes are wirtten by Mahito Sugiyama (testability part) and Felipe Llinares-Lopez (permutation part), 2014" << endl << endl;

  // In "class_labels_file", 1 is minor class and 0 is major class
  char *input_file, *class_file, *output_file;
  bool flag_in = false, flag_class = false, flag_out = false;
  int n_perm = 1000;

  char opt;
  while ( ( opt = getopt ( argc, argv, "m:a:j:f:r:tpa:i:c:o:w" ) ) != -1 ) {
    switch ( opt ) {
    case 'm': maxsize = atoi(optarg); break;
    case 'j': n_perm = atoi(optarg); break;
    case 'f': minfreq = atoi(optarg); break;
    case 'r': SEED = atoi(optarg); break;
    case 't': phase = 2; break;
    case 'p': phase = 1; break;
    case 'a': ALPHA = atof(optarg); break;
    case 'i': input_file = optarg; flag_in = true; break;
    case 'c': class_file = optarg; flag_class = true; break;
    case 'o': output_file = optarg; flag_out = true; break;
    case 'w': WYLIGHT = true; break;
    }
  }

  if (!flag_in || !flag_class) {
    if (!flag_in) cerr << "Input file is missing!" << endl;
    if (!flag_class) cerr << "Class file is missing!" << endl;
    
    cerr << "Parameters:" << endl
	 << "    -w optimal FWER control by Westfall-Young permutation" << endl
	 << "    -a [Target FWER] (default: 0.05)" << endl
      	 << "    -m [Maximum size of each subgraph] (default: unlimited)" << endl
      	 << "    -j [Number of permutations] (used only in '-w' mode, default: 1000)" << endl
	 << "    -r [seed for permutations] (used only in '-w' mode, default: 0)" << endl
	 << "    -i [input file for graphs]" << endl
	 << "    -c [input file for class labels]" << endl
	 << "    -o [output file for subgraphs]" << endl
	 << endl;
    return 1;
  }

  cerr << "Database file read ...";
  FILE *input = fopen ( input_file, "r" );
  database.read ( input );
  rewind( input );
  database_original.read ( input );
  fclose ( input );
  cerr << " end" << endl;

  cerr << "Class file read ...";
  readClass(class_file);
  cerr << " end" << endl;

  if (WYLIGHT) {
    cerr << "Initialize random permutations ...";
    permutation_init(n_perm, class_file);
    cerr << " end" << endl << endl;
  } else {
    cerr << endl;
  }
   
  cout << "Target FWER:                     \t" << ALPHA << endl;
  cout << "Number of graphs in total:       \t" << N_TOTAL << endl;
  cout << "Number of graphs in minor class: \t" << N_SMALL << endl;
  cout << "Maximum size of each subgraph:   \t";
  if (maxsize < INT_MAX) {
    cout << maxsize << endl;
  } else {
    cout << "unlimited" << endl;
  }
  if (WYLIGHT) {
    cout << "Number of permutations:         \t" << n_perm << endl;
    cout << "Seed of permutations:           \t" << SEED << endl;      
    OCC_VEC.resize(N_TOTAL); // Initialise occurrence vector    
    wy_init(ALPHA); // Initialize Westfall-Young permutation code
  } else {
    THRESHOLD = ALPHA / ((double)N_SMALL / (double)N_TOTAL); // compute the first threshold for minfreq = 1
  }

  if (flag_out) {
    OFS.open(output_file);
  }
  
  cerr << endl << "Start GASTON to compute the corrected significance threshold" << endl;
  runGaston();
  if (WYLIGHT) {
    wy_end(); // get corrected significance threshold for each test
    permutation_end();
  } else {
    DELTA = ALPHA / COUNT; // Corrected significance threshold for each test
  }
  cerr << "End GASTON" << endl << endl;

  clock_t t2 = clock();

  cout << "Corrected significance threshold:\t" << DELTA << endl;

  // re-run Gaston to enumerate eignificant subgraphs
  // Fishers's exact test is performed for each subgraph
  cerr << endl << "Start GASTON to enumerate significant subgraphs ... ";
  RERUN = true;
  COUNT = 0.0;
  clearGaston();
  runGaston();
  cerr << "end" << endl << endl;

  clock_t t3 = clock();

  cout << "Number of significant subgraphs: \t" << COUNT << endl;
  cout << "Runtime for correction (s):      \t" << ( (float) t2 - t1 ) / CLOCKS_PER_SEC << endl;
  cout << "Runtime for sig.subgraphs (s):   \t" << ( (float) t3 - t2 ) / CLOCKS_PER_SEC << endl;
  cout << "Total runtime (s):               \t" << ( (float) t3 - t1 ) / CLOCKS_PER_SEC << endl;
  // statistics.print ();

  if (flag_out) {
    OFS.close();
  }
}
