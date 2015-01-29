// misc.h
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
#ifndef MISC_H
#define MISC_H
#include <vector>
#include <stdio.h>
using namespace std;

typedef unsigned char EdgeLabel; // combined node-edge label of the input file.
typedef unsigned char NodeLabel;
typedef unsigned short NodeId;
typedef unsigned int Depth; // unsigned int is more efficient than short, but requires more memory...
typedef unsigned int Tid;
typedef unsigned int Frequency;

extern Frequency minfreq;

#define NOTID ((Tid) -1)
#define NOEDGELABEL ((EdgeLabel) -1)
#define MAXEDGELABEL NOEDGELABEL
#define NONODELABEL ((NodeLabel) -1)
#define NODEPTH ((Depth) -1)
#define NOLEG (-1)
#define NONODE ((NodeId) -1)

// this macro can be used when push_back-ing large structures. In stead of first allocating a local
// variable and pushing this, one first pushes and defines a reference to the space in the vector.
// This avoids re-allocation.
#define vector_push_back(_type,_vector,_var) (_vector).push_back ( _type () ); _type &_var = (_vector).back ();

// can be used to obtain a type when inserting into a map
#define map_insert_pair(_type) typedef typeof(_type) T##_type; pair<T##_type::iterator,bool>

#define store(a,b) { if ( (b).elements.capacity () - (b).elements.size () > (b).elements.size () / 2 ) (a) = (b); else swap ( (a), (b) ); }

extern bool dooutput;
extern int phase; // 3 : all, 2 : paths and trees, 1 : paths
extern int maxsize; // maximal size

void puti ( FILE *f, int i );
extern FILE *output;

#define OUTPUT(frequency) if ( dooutput  ) { putc ( '#', output ); putc ( ' ', output ); puti ( output, frequency ); putc ( '\n', output ); graphstate.print ( output ); }

inline void setmax ( short unsigned int &a, short unsigned int b ) { if ( b > a ) a = b; }

class Statistics {
  public:
    vector<unsigned int> frequenttreenumbers;
    vector<unsigned int> frequentpathnumbers;
    vector<unsigned int> frequentgraphnumbers;
    int patternsize;
    void print ();
};

extern Statistics statistics;

#endif
