#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "legoccurrence.h"
#include "closeleg.h"
#include "database.h"
#include "graphstate.h"
#include "tcheck.h"

// read class labels
void readClass(char *class_file) {
  ifstream ifs(class_file);
  if (ifs.fail()) {
    cerr << "File do not exist." << endl;
    exit(0);
  }

  int c;
  string str;
  while (getline(ifs, str)) {
    sscanf(str.data(), "%d", &c);
    CLASS_VEC.push_back(c);
    N_TOTAL++;
    if (c == 1) N++;
    else if (c != 0) {
      cerr << "class labels should be 0 or 1" << endl;
      exit(0);
    }
  }

  if ((double)N > (double)N_TOTAL / 2.0) {
    cerr << "class 1 should be smalelr than 0" << endl;
    exit(0);
  }

  ifs.close();
}


// appear in "patterntree.cpp" (2 times), "path.cpp" (2 times), "patterngraph.cpp"
// data types are defined in "legoccurrence.h"
void checkTestable(vector<LegOccurrence>& elements, Frequency frequency) {
  checkCondition<LegOccurrence>(elements, frequency);
}

void checkTestableCl(vector<CloseLegOccurrence>& elements, Frequency frequency) {
  checkCondition<CloseLegOccurrence>(elements, frequency);
}

template<typename T> void checkCondition(vector<T>& elements, Frequency frequency) {
  Tid id_prev = NOTID;
  typename vector<T>::iterator itr, end;

  COUNT += 1.0;
  // THR = ALPHTA / minp(minfreq)
  if (COUNT > THRESHOLD) {
    minfreq++;
    cerr << "minfreq = " << minfreq << endl;
    // compute the minimum achievable P value, which is "binom(N, minfreq) / binom(N_TOTAL, minfreq)"
    double minp = 1;
    for (int i = 0; i < minfreq; i++) {
      minp *= (double)(N - i) / (double)(N_TOTAL - i);
    }
    // update threshold
    THRESHOLD = ALPHA / minp;
    cerr << "Threshold = " << THRESHOLD << endl;
  }
}
