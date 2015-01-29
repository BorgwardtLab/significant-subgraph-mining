// closeleg.h
// Siegfried Nijssen, snijssen@liacs.nl, feb 2004.
#ifndef CLOSELEG_H
#define CLOSELEG_H
#include <vector>
#include "misc.h"
#include "legoccurrence.h"

struct CloseTuple {
  EdgeLabel label;
  int from;
  int to;
  friend bool operator< ( CloseTuple &a, CloseTuple &b ) { return a.from < b.from || ( a.from == b.from && ( a.to < b.to || ( a.to == b.to && a.label < b.label ) ) ); }
  friend bool operator> ( CloseTuple &a, CloseTuple &b ) { return a.from > b.from || ( a.from == b.from && ( a.to > b.to || ( a.to == b.to && a.label > b.label ) ) ); }
  friend ostream &operator<< ( ostream &stream, CloseTuple &tuple ) { 
    stream << (int) tuple.from << " " << tuple.to << " " << (int) tuple.label << endl;
    return stream;
  }
};

struct CloseLegOccurrence {
  Tid tid;
  OccurrenceId occurrenceid;

  CloseLegOccurrence ( Tid tid, OccurrenceId occurrenceid ): tid ( tid ), occurrenceid ( occurrenceid ) { }
  CloseLegOccurrence () { }
};

struct CloseLegOccurrences {
  Frequency frequency;
  vector<CloseLegOccurrence> elements;
  CloseLegOccurrences () : frequency ( 0 ) { }
};

typedef CloseLegOccurrences *CloseLegOccurrencesPtr;

struct CloseLeg {
  bool copy;
  CloseTuple tuple;
  CloseLegOccurrences occurrences;
  CloseLeg (): copy ( true ) { }
};

typedef CloseLeg *CloseLegPtr;

extern bool closelegsoccsused;
extern vector<vector< CloseLegOccurrences> > candidatecloselegsoccs;
extern vector<bool> candidatecloselegsoccsused;

class Leg;
typedef Leg *LegPtr;

void addCloseExtensions ( vector<CloseLegPtr> &targetcloselegs, int number );
void addCloseExtensions ( vector<CloseLegPtr> &targetcloselegs, vector<CloseLegPtr> &sourcecloselegs, LegOccurrences &sourceoccs );
CloseLegOccurrencesPtr join ( LegOccurrences &legoccsdata, CloseLegOccurrences &closelegoccsdata );
CloseLegOccurrencesPtr join ( CloseLegOccurrences &closelegoccsdata1, CloseLegOccurrences &closelegoccsdata2 );

#endif
