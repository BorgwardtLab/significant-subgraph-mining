// patterngraph.h
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
#ifndef PATTERNGRAPH_H
#define PATTERNGRAPH_H
#include <vector>
#include "closeleg.h"

using namespace std;

class PatternGraph {
  public:
    vector<CloseLegPtr> closelegs;
    vector<CloseTuple> closetuples;
    void init ( vector<CloseLegPtr> &closelegssource, int legindex );
    vector<CloseLegPtr> *closelegssource;
    int legindex;
    PatternGraph  ( vector<CloseLegPtr> &closelegssource, int legindex ) {
      init ( closelegssource, legindex );
    }
    PatternGraph  ( PatternGraph &parentpatterngraph, int legindex ):
      closetuples ( parentpatterngraph.closetuples ) {
      init ( parentpatterngraph.closelegs, legindex );
    }
    Frequency frequency; 
    ~PatternGraph ();
    void expand ();
};

#endif
