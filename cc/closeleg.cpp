// closeleg.cpp
// Siegfried Nijssen, snijssen@liacs.nl, feb 2004.
#include <vector>
#include "misc.h"
#include "closeleg.h"

CloseLegOccurrences closelegoccurrences;

void addCloseExtensions ( vector<CloseLegPtr> &targetcloselegs, int number ) {
  if ( closelegsoccsused ) {
    for ( int i = 1; i < candidatecloselegsoccs.size (); i++ )
      if ( candidatecloselegsoccsused[i] ) {
        vector<CloseLegOccurrences> &edgelabeloccs = candidatecloselegsoccs[i];
        for ( EdgeLabel j = 0; j < edgelabeloccs.size (); j++ ) {
          if ( edgelabeloccs[j].frequency >= minfreq ) {
            CloseLegPtr closelegptr = new CloseLeg;
            closelegptr->tuple.label = j;
            closelegptr->tuple.to = i;
            closelegptr->tuple.from = number;
            swap ( closelegptr->occurrences, edgelabeloccs[j] );
            targetcloselegs.push_back ( closelegptr );
          }
        }
      }
  }
}

void addCloseExtensions ( vector<CloseLegPtr> &targetcloselegs, vector<CloseLegPtr> &sourcecloselegs, LegOccurrences &sourceoccs ) {
  for ( int i = 0; i < sourcecloselegs.size (); i++ ) {
    CloseLegOccurrencesPtr closelegoccurrencesptr = join ( sourceoccs, sourcecloselegs[i]->occurrences );
    if ( closelegoccurrencesptr ) {
      CloseLegPtr closelegptr = new CloseLeg;
      closelegptr->tuple = sourcecloselegs[i]->tuple;
      swap ( closelegptr->occurrences, *closelegoccurrencesptr );
      targetcloselegs.push_back ( closelegptr );
    }
  }
}

CloseLegOccurrencesPtr join ( LegOccurrences &legoccsdata, CloseLegOccurrences &closelegoccsdata ) {
  Frequency frequency = 0;
  Tid lasttid = NOTID;
  vector<CloseLegOccurrence> &closelegoccs = closelegoccsdata.elements;
  vector<LegOccurrence> &legoccs = legoccsdata.elements;

  closelegoccurrences.elements.resize ( 0 );

  unsigned int legoccssize = legoccs.size (), closelegoccssize = closelegoccs.size ();
  OccurrenceId j = 0, k = 0;
  int comp;

  while ( true ) {
    comp = legoccs[j].occurrenceid - closelegoccs[k].occurrenceid;
    if  ( comp < 0 ) {
      j++;
      if ( j == legoccssize )
        break;
    }
    else {
      if ( comp == 0 ) {
        closelegoccurrences.elements.push_back ( CloseLegOccurrence ( legoccs[j].tid, j ) );
        if ( legoccs[j].tid != lasttid ) {
          lasttid = legoccs[j].tid;
          frequency++;
        }
        j++;
        if ( j == legoccssize )
          break;
      }
      else {
        k++;
        if ( k == closelegoccssize )
          break;
      }
    }
  }

  if ( frequency >= minfreq ) {
    closelegoccurrences.frequency = frequency;
    return &closelegoccurrences;
  }
  else
    return NULL;
}

CloseLegOccurrencesPtr join ( CloseLegOccurrences &closelegoccsdata1, CloseLegOccurrences &closelegoccsdata2 ) {
  Frequency frequency = 0;
  Tid lasttid = NOTID;
  vector<CloseLegOccurrence> &closelegoccs1 = closelegoccsdata1.elements,
                             &closelegoccs2 = closelegoccsdata2.elements;

  unsigned int closelegoccs1size = closelegoccs1.size (), closelegoccs2size = closelegoccs2.size ();
  closelegoccurrences.elements.resize ( 0 );
  OccurrenceId j = 0, k = 0;
  int comp;

  while ( true ) {
    comp = closelegoccs1[j].occurrenceid - closelegoccs2[k].occurrenceid;
    if ( comp < 0 ) {
      j++;
      if ( j == closelegoccs1size )
        break;
    }
    else {
      if ( comp == 0 ) {
        closelegoccurrences.elements.push_back ( CloseLegOccurrence ( closelegoccs1[j].tid, closelegoccs1[j].occurrenceid )  );
        if ( closelegoccs1[j].tid != lasttid ) {
          lasttid = closelegoccs1[j].tid;
          frequency++;
        }
        j++;
        if ( j == closelegoccs1size )
          break;
      }
      k++;
      if ( k == closelegoccs2size )
        break;
    }
  }

  if ( frequency >= minfreq ) {
    closelegoccurrences.frequency = frequency;
    return &closelegoccurrences;
  }
  else
    return NULL;
}
