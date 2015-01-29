#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "legoccurrence.h"
#include "closeleg.h"
#include "database.h"
#include "graphstate.h"
#include "tcheck.h"
#include <boost/math/distributions/hypergeometric.hpp>

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
  if (!RERUN) checkCondition();
  else computePvalue<LegOccurrence>(elements, frequency);
}

void checkTestableCl(vector<CloseLegOccurrence>& elements, Frequency frequency) {
  if (!RERUN) checkCondition();
  else computePvalue<CloseLegOccurrence>(elements, frequency);
}

void checkCondition() {
  Tid id_prev = NOTID;

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

template<typename T> void computePvalue(vector<T>& elements, Frequency frequency) {
  typename vector<T>::iterator itr, end;

  Tid id_prev = NOTID;
  int count = 0;
  for (itr = elements.begin(), end = elements.end(); itr != end; ++itr) {
    if (id_prev != itr->tid) {
      // retrieving occurrences in graphs
      if (CLASS_VEC[itr->tid] == 1) count++;
      id_prev = itr->tid;
    }
  }

  // compute p-value from "count", "frequency", "N", and "N_TOTAL"
  boost::math::hypergeometric_distribution<> phyper(frequency, N, N_TOTAL);
  double p_value_L = 0, p_value_R = 0;
  if (count == 0) count = 1;
  else if (count == N) count = N - 1;
  if (frequency - count == 0) count = frequency - 1;
  else if (frequency - count == N_TOTAL - N) count = frequency - N_TOTAL + N + 1;
  // if (0 < count && count < N && 0 < frequency - count && frequency - count < (N_TOTAL - N)) {
  p_value_L = cdf(phyper, count);
  p_value_R = cdf(complement(phyper, count - 1));
  // }
  double p_value_D = p_value_L < p_value_R ? 2 * p_value_L : 2 * p_value_R;

  if (p_value_D < DELTA) {
    cout << "p-value: " << p_value_D << endl;
    // significant_subgraphs.push_back(i);
    // p_value_vec.push_back(p_value);
  }
}
