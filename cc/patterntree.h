// patterntree.h
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
#ifndef PATTERNTREE_H
#define PATTERNTREE_H
#include <iostream>
#include <vector>
#include "misc.h"
#include "database.h"
#include "legoccurrence.h"
#include "path.h"
#include "closeleg.h"

using namespace std;

struct Tuple {
  Depth depth;
  EdgeLabel label;
  NodeId connectingnode;
  
  Tuple ( Depth depth, EdgeLabel label, NodeId connectingnode ): 
    depth ( depth ), label ( label ), connectingnode ( connectingnode ) { }
  Tuple () { }

  friend bool operator< ( Tuple &a, Tuple &b ) { return a.depth > b.depth || ( a.depth == b.depth && a.label < b.label ); }
  friend bool operator== ( Tuple &a, Tuple &b ) { return a.depth == b.depth && a.label == b.label; }
  friend ostream &operator<< ( ostream &stream, Tuple &tuple );
};

struct Leg {
  Tuple tuple;
  LegOccurrences occurrences;
};

typedef Leg *LegPtr;

class PatternTree {
  public:
    PatternTree ( Path &path, unsigned int legindex );
    ~PatternTree ();
    void expand ();
  private:
    void checkIfIndeedNormal ();
    /* inline */ void addExtensionLegs ( Tuple &tuple, LegOccurrences &legoccurrences );
    /* inline */ void addLeg ( const NodeId connectingnode, const int depth, const EdgeLabel edgelabel, LegOccurrences &legoccurrences );
    /* inline */ void addLeftLegs ( Path &path, PathLeg &leg, int &i, Depth olddepth, EdgeLabel lowestlabel, int leftend, int edgesize2 );
    /* inline */ int addLeftLegs ( Path &path, PathLeg &leg, Tuple &tuple, unsigned int legindex, int leftend, int edgesize2 );
    /* inline */ void addRightLegs ( Path &path, PathLeg &leg, int &i, Depth olddepth, EdgeLabel lowestlabel, int rightstart, int nodesize2 );
    /* inline */ int addRightLegs ( Path &path, PathLeg &leg, Tuple &tuple, unsigned int legindex, int rightstart, int nodesize2 );
    PatternTree ( PatternTree &parenttree, unsigned int legindex );
    vector<Tuple> treetuples;
    vector<NodeId> rightmostindexes;
    vector<short> rootpathrelations;
    unsigned int nextprefixindex;
    unsigned int rootpathstart;
    unsigned int nextpathstart;
    unsigned int maxdepth;
    int symmetric; // 0 == not symmetric, 1 == symmetric, even length path, 2 == symmetric, odd length path
    int secondpathleg;
    vector<LegPtr> legs; // pointers used to avoid copy-constructor during a resize of the vector
    vector<CloseLegPtr> closelegs;
    friend ostream &operator<< ( ostream &stream, PatternTree &patterntree );
#ifdef GRAPH_OUTPUT
    friend void fillMatrix ( int **A, int &nextnode, int rootnode, NodeLabel rootlabel, 
                  int startpos, int endpos, PatternTree &patterntree );
    NodeLabel tree1rootlabel, tree2rootlabel;
    EdgeLabel rootpathlabel;
#endif
};

#define NONEXTPREFIX ((unsigned int) -1)

#endif
