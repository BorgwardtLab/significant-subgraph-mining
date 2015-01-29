// path.h
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
#ifndef PATH_H
#define PATH_H
#include <iostream>
#include <vector>
#include "misc.h"
#include "database.h"
#include "legoccurrence.h"
#include "closeleg.h"
#include "tcheck.h"
//#include "patterntree.h"

using namespace std;

struct PathTuple {
  Depth depth;
  NodeId connectingnode;
  EdgeLabel edgelabel;
  NodeLabel nodelabel;
};

struct PathLeg {
  PathTuple tuple;
  LegOccurrences occurrences;
};

typedef PathLeg *PathLegPtr;

class Path {
  public:
    Path ( NodeLabel startnodelabel );
    ~Path ();
    void expand ();
  private:
    friend class PatternTree;
    bool isnormal ( EdgeLabel edgelabel ); // ADDED
    void expand2 ();
    Path ( Path &parentpath, unsigned int legindex );
    vector<PathLegPtr> legs; // pointers used to avoid copy-constructor during a resize of the vector
    vector<CloseLegPtr> closelegs;
    vector<NodeLabel> nodelabels;
    vector<EdgeLabel> edgelabels;
    int frontsymmetry; // which is lower, the front or front reverse?
    int backsymmetry; // which is lower, the back or back reverse?
    int totalsymmetry; // which is lower, from left to right, or the reverse?

    friend ostream &operator<< ( ostream &stream, Path &path );
};

#endif
