// legoccurrence.cpp
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
#include "legoccurrence.h"
#include "closeleg.h"
#include "database.h"
#include "graphstate.h"

vector<LegOccurrences> candidatelegsoccurrences; // for each frequent possible edge, the occurrences found, used by extend
LegOccurrences legoccurrences;

bool closelegsoccsused;
vector<vector< CloseLegOccurrences> > candidatecloselegsoccs;
vector<bool> candidatecloselegsoccsused;


void initLegStatics () {
  if (RERUN) {
    candidatecloselegsoccs.clear();
    candidatelegsoccurrences.clear();
  }
  candidatecloselegsoccs.reserve ( 200 ); // should be larger than the largest structure that contains a cycle
  candidatelegsoccurrences.resize ( database.frequentEdgeLabelSize () );
}


ostream &operator<< ( ostream &stream, LegOccurrence &occ ) {
  stream << "[" << occ.tid << "," << occ.occurrenceid << "," << occ.tonodeid << "," << occ.fromnodeid << "]";
  return stream;
}

ostream &operator<< ( ostream &stream, vector<LegOccurrence> &occs ) {
  Tid lasttid = NOTID;
  Frequency frequency = 0;
  for ( int i = 0; i < occs.size (); i++ ) {
    //stream << occs[i];
    if ( occs[i].tid != lasttid ) {
      stream << occs[i].tid << " ";
      lasttid = occs[i].tid;
      frequency++;
    }
  }
  stream << endl << " (" << frequency << ")" << endl;

  return stream;
}

// This function is on the critical path. Its efficiency is MOST important.
LegOccurrencesPtr join ( LegOccurrences &legoccsdata1, NodeId connectingnode, LegOccurrences &legoccsdata2 ) {
  if ( graphstate.getNodeDegree ( connectingnode ) == graphstate.getNodeMaxDegree ( connectingnode ) ) 
    return NULL;

  Frequency frequency = 0;
  Tid lasttid = NOTID;
  vector<LegOccurrence> &legoccs1 = legoccsdata1.elements, &legoccs2 = legoccsdata2.elements;
  legoccurrences.elements.resize ( 0 );
  legoccurrences.maxdegree = 0;
  legoccurrences.selfjoin = 0;
  //legoccurrences.elements.reserve ( legoccs1.size () * 2 ); // increased memory usage, and speed!
  OccurrenceId j = 0, k = 0, l, m;
  unsigned int legoccs1size = legoccs1.size (), legoccs2size = legoccs2.size (); // this increases speed CONSIDERABLY!
  Tid lastself = NOTID;

  do {
    while ( j < legoccs1size && legoccs1[j].occurrenceid < legoccs2[k].occurrenceid ) {
      j++;
    }
    if ( j < legoccs1size ) {
      LegOccurrence &jlegocc = legoccs1[j];
      while ( k < legoccs2size && legoccs2[k].occurrenceid < jlegocc.occurrenceid ) {
        k++;
      }
      if ( k < legoccs2size ) {
        if ( legoccs2[k].occurrenceid == jlegocc.occurrenceid ) {
          m = j;
          do {
            j++;
          }
          while ( j < legoccs1size && legoccs1[j].occurrenceid == jlegocc.occurrenceid );
          l = k;
          do {
            k++;
          }
          while ( k < legoccs2size && legoccs2[k].occurrenceid == jlegocc.occurrenceid );
	  bool add = false;
          for ( OccurrenceId m2 = m; m2 < j; m2++ ) {
            int d = 0;
            for ( OccurrenceId l2 = l; l2 < k; l2++ ) {
	      NodeId tonodeid = legoccs2[l2].tonodeid;
              if ( legoccs1[m2].tonodeid !=  tonodeid ) {
                legoccurrences.elements.push_back ( LegOccurrence ( jlegocc.tid, m2, tonodeid, legoccs2[l2].fromnodeid ) );
                setmax ( legoccurrences.maxdegree, database.trees[jlegocc.tid]->nodes[tonodeid].edges.size () );
		add = true;
		d++;
              }
            }
	    if ( d > 1 && jlegocc.tid != lastself ) {
	      lastself = jlegocc.tid;
	      legoccurrences.selfjoin++;
	    }
	  }
	  	  
	  if ( jlegocc.tid != lasttid && add ) {
            lasttid = jlegocc.tid;
	    frequency++;
	  }

          if ( k == legoccs2size )
            break;
        }
      }
      else
        break;
    }
    else
      break;
  }
  while ( true );

  if ( frequency >= minfreq ) {
    legoccurrences.parent = &legoccsdata1;
    legoccurrences.number = legoccsdata1.number + 1;
    legoccurrences.frequency = frequency;
    return &legoccurrences;
  }
  else
    return NULL;
}

LegOccurrencesPtr join ( LegOccurrences &legoccsdata ) {
  if ( legoccsdata.selfjoin < minfreq ) 
    return NULL;
  legoccurrences.elements.resize ( 0 );
  vector<LegOccurrence> &legoccs = legoccsdata.elements;
  legoccurrences.maxdegree = 0;
  legoccurrences.selfjoin = 0;
  Tid lastself = NOTID;

  OccurrenceId j = 0, k, l, m;
  do {
    k = j;
    LegOccurrence &legocc = legoccs[k];
    do {
      j++;
    }
    while ( j < legoccs.size () &&
            legoccs[j].occurrenceid == legocc.occurrenceid );
    for ( l = k; l < j; l++ )
      for ( m = k; m < j; m++ )
        if ( l != m ) {
          legoccurrences.elements.push_back ( LegOccurrence ( legocc.tid, l, legoccs[m].tonodeid, legoccs[m].fromnodeid ) );
          setmax ( legoccurrences.maxdegree, database.trees[legocc.tid]->nodes[legoccs[m].tonodeid].edges.size () );
        }
    if ( ( j - k > 2 ) && legocc.tid != lastself ) {
      lastself = legocc.tid;
      legoccurrences.selfjoin++;
    }
  }
  while ( j < legoccs.size () );

    // no need to check that we are frequent, we must be frequent
  legoccurrences.parent = &legoccsdata;
  legoccurrences.number = legoccsdata.number + 1;
  legoccurrences.frequency = legoccsdata.selfjoin; 
    // we compute the self-join frequency exactly while building the
    // previous list. It is therefore not necessary to recompute it.
  return &legoccurrences;
}

inline int nocycle ( DatabaseTreePtr tree, DatabaseTreeNode &node, NodeId tonode, OccurrenceId occurrenceid, LegOccurrencesPtr legoccurrencesdataptr ) {
  if ( !tree->nodes[tonode].incycle )
    return 0;
  if ( !node.incycle )
    return 0;
  while ( legoccurrencesdataptr ) {
    if ( legoccurrencesdataptr->elements[occurrenceid].tonodeid == tonode ) {
      return legoccurrencesdataptr->number;
    }
    occurrenceid = legoccurrencesdataptr->elements[occurrenceid].occurrenceid;
    legoccurrencesdataptr = legoccurrencesdataptr->parent;
  }
  return 0;
}

void candidateCloseLegsAllocate ( int number, int maxnumber ) {
  if ( !closelegsoccsused ) {
    int oldsize = candidatecloselegsoccs.size ();
    candidatecloselegsoccs.resize ( maxnumber );
    for ( int k = oldsize; k < candidatecloselegsoccs.size (); k++ ) {
      candidatecloselegsoccs[k].resize ( database.frequentEdgeLabelSize () );
    }
    candidatecloselegsoccsused.resize ( 0 );
    candidatecloselegsoccsused.resize ( maxnumber, false );
    closelegsoccsused = true;
  }
  if ( !candidatecloselegsoccsused[number] ) {
    candidatecloselegsoccsused[number] = true;
    vector<CloseLegOccurrences> &candidateedgelabeloccs = candidatecloselegsoccs[number];
    for ( int k = 0; k < candidateedgelabeloccs.size (); k++ ) {
      candidateedgelabeloccs[k].elements.resize ( 0 );
      candidateedgelabeloccs[k].frequency = 0;
    }
  }
}

void extend ( LegOccurrences &legoccurrencesdata ) {
  // we're trying hard to avoid repeated destructor/constructor calls for complex types like vectors.
  // better reuse previously allocated memory, if possible!
  vector<LegOccurrence> &legoccurrences = legoccurrencesdata.elements;
  Tid lastself[candidatelegsoccurrences.size ()];

  for ( int i = 0; i < candidatelegsoccurrences.size (); i++ ) {
    candidatelegsoccurrences[i].elements.resize ( 0 );
    //candidatelegsoccurrences[i].elements.reserve ( legoccurrences.size () ); // increases memory usage, but also speed!
    candidatelegsoccurrences[i].parent = &legoccurrencesdata;
    candidatelegsoccurrences[i].number = legoccurrencesdata.number + 1;
    candidatelegsoccurrences[i].maxdegree = 0;
    candidatelegsoccurrences[i].frequency = 0;
    candidatelegsoccurrences[i].selfjoin = 0;
    lastself[i] = NOTID;
  }

  closelegsoccsused = false; // we are lazy with the initialization of close leg arrays, as we may not need them at all in
                             // many cases

  for ( OccurrenceId i = 0; i < legoccurrences.size (); i++ ) {
    LegOccurrence &legocc = legoccurrences[i];
    DatabaseTreePtr tree = database.trees[legocc.tid];
    DatabaseTreeNode &node = tree->nodes[legocc.tonodeid];
    for ( int j = 0; j < node.edges.size (); j++ ) {
      if ( node.edges[j].tonode != legocc.fromnodeid ) {
  	EdgeLabel edgelabel = node.edges[j].edgelabel;
        int number = nocycle ( tree, node, node.edges[j].tonode, i, &legoccurrencesdata );
        if ( number == 0 ) {
          vector<LegOccurrence> &candidatelegsoccs = candidatelegsoccurrences[edgelabel].elements;
          if ( candidatelegsoccs.empty () )
  	    candidatelegsoccurrences[edgelabel].frequency++;
	  else {
	    if ( candidatelegsoccs.back ().tid != legocc.tid )
    	      candidatelegsoccurrences[edgelabel].frequency++;
	    if ( candidatelegsoccs.back ().occurrenceid == i &&
	         lastself[edgelabel] != legocc.tid ) {
              lastself[edgelabel] = legocc.tid;
	      candidatelegsoccurrences[edgelabel].selfjoin++;
	    }
          }
          candidatelegsoccs.push_back ( LegOccurrence ( legocc.tid, i, node.edges[j].tonode, legocc.tonodeid ) );
          setmax ( candidatelegsoccurrences[edgelabel].maxdegree, database.trees[legocc.tid]->nodes[node.edges[j].tonode].edges.size () );
        }
        else if ( number - 1 != graphstate.nodes.back().edges[0].tonode ) {
          candidateCloseLegsAllocate ( number, legoccurrencesdata.number + 1 );

          vector<CloseLegOccurrence> &candidatelegsoccs = candidatecloselegsoccs[number][edgelabel].elements;
          if ( !candidatelegsoccs.size () || candidatelegsoccs.back ().tid != legocc.tid )
	    candidatecloselegsoccs[number][edgelabel].frequency++;
          candidatelegsoccs.push_back ( CloseLegOccurrence ( legocc.tid, i ) );
            setmax ( candidatelegsoccurrences[edgelabel].maxdegree, database.trees[legocc.tid]->nodes[node.edges[j].tonode].edges.size () );
        }
      }
    }
  }
}

void extend ( LegOccurrences &legoccurrencesdata, EdgeLabel minlabel, EdgeLabel neglect ) {
  // we're trying hard to avoid repeated destructor/constructor calls for complex types like vectors.
  // better reuse previously allocated memory, if possible!
  vector<LegOccurrence> &legoccurrences = legoccurrencesdata.elements;
  int lastself[candidatelegsoccurrences.size ()];
  
  for ( int i = 0; i < candidatelegsoccurrences.size (); i++ ) {
    candidatelegsoccurrences[i].elements.resize ( 0 );
    candidatelegsoccurrences[i].parent = &legoccurrencesdata;
    candidatelegsoccurrences[i].number = legoccurrencesdata.number + 1;
    candidatelegsoccurrences[i].maxdegree = 0;
    candidatelegsoccurrences[i].selfjoin = 0;
    lastself[i] = NOTID;
    candidatelegsoccurrences[i].frequency = 0;
  }

  closelegsoccsused = false; // we are lazy with the initialization of close leg arrays, as we may not need them at all in
                             // many cases
  for ( OccurrenceId i = 0; i < legoccurrences.size (); i++ ) {
    LegOccurrence &legocc = legoccurrences[i];
    DatabaseTreePtr tree = database.trees[legocc.tid];
    DatabaseTreeNode &node = tree->nodes[legocc.tonodeid];
    for ( int j = 0; j < node.edges.size (); j++ ) {
      if ( node.edges[j].tonode != legocc.fromnodeid ) {
	EdgeLabel edgelabel = node.edges[j].edgelabel;
        int number = nocycle ( tree, node, node.edges[j].tonode, i, &legoccurrencesdata );
        if ( number == 0 ) {
	  if ( edgelabel >= minlabel && edgelabel != neglect ) {
            vector<LegOccurrence> &candidatelegsoccs = candidatelegsoccurrences[edgelabel].elements;
            if ( candidatelegsoccs.empty () )
  	      candidatelegsoccurrences[edgelabel].frequency++;
	    else {
	      if ( candidatelegsoccs.back ().tid != legocc.tid )
  	        candidatelegsoccurrences[edgelabel].frequency++;
	      if ( candidatelegsoccs.back ().occurrenceid == i &&
                lastself[edgelabel] != legocc.tid ) {
                lastself[edgelabel] = legocc.tid;
                candidatelegsoccurrences[edgelabel].selfjoin++;
              }
            }
            candidatelegsoccs.push_back ( LegOccurrence ( legocc.tid, i, node.edges[j].tonode, legocc.tonodeid ) );
	    setmax ( candidatelegsoccurrences[edgelabel].maxdegree, database.trees[legocc.tid]->nodes[node.edges[j].tonode].edges.size () );
	  }
        }
        else if ( number - 1 != graphstate.nodes.back().edges[0].tonode ) {
          candidateCloseLegsAllocate ( number, legoccurrencesdata.number + 1 );

          vector<CloseLegOccurrence> &candidatelegsoccs = candidatecloselegsoccs[number][edgelabel].elements;
          if ( !candidatelegsoccs.size () || candidatelegsoccs.back ().tid != legocc.tid )
	    candidatecloselegsoccs[number][edgelabel].frequency++;
          candidatelegsoccs.push_back ( CloseLegOccurrence ( legocc.tid, i ) );
          setmax ( candidatelegsoccurrences[edgelabel].maxdegree, database.trees[legocc.tid]->nodes[node.edges[j].tonode].edges.size () );
        }
      }
    }
  }
}

/*

class Counter {
  public:
  int number;
  Counter () { number = 0; }
  ~Counter () { cout << number << endl; }
} counter, counter2;

void sanityCheck ( LegOccurrencesPtr legoccurrencesptr ) {
  if ( !legoccurrencesptr )
    return;
  counter2.number++;
  for ( OccurrenceId i = 0; i < legoccurrencesptr->elements.size (); i++ ) {
    for ( OccurrenceId t = i + 1; t < legoccurrencesptr->elements.size () &&
                                  legoccurrencesptr->elements[t].occurrenceid == legoccurrencesptr->elements[i].occurrenceid; t++ )
      if ( legoccurrencesptr->elements[t].tonodeid == legoccurrencesptr->elements[i].tonodeid )
        cerr << "MULTIPLE OCCURRENCES FOR THE SAME OCCURRENCE ID OF THE SAME NODE: "
             << legoccurrencesptr->elements[t].tonodeid << " IN " << legoccurrencesptr->elements[t].tid << endl;
    LegOccurrence &legocc = legoccurrencesptr->elements[i];
    OccurrenceId j = i, k;
    LegOccurrencesPtr walk1 = legoccurrencesptr, walk2;
    int val = 0;
    while ( walk1 ) {
      k = walk1->elements[j].occurrenceid;
      walk2 = walk1->parent;
      while ( walk2 ) {
        if ( walk2->elements[k].tid != walk1->elements[j].tid ) {
          cerr << "INSANITY: TIDs DO NOT MATCH: " << walk2->elements[k].tid << " AND " << walk1->elements[j].tid << endl;
        }
        if ( walk2->elements[k].tonodeid == walk1->elements[j].tonodeid ) {
          cerr << "INSANITY: SAME NODE TWICE: " << walk2->elements[k].tid << " (TID) " << walk1->elements[j].tonodeid << " (NODEID)" << endl;
        }
        k = walk2->elements[k].occurrenceid;
        walk2 = walk2->parent;
      }
      j = walk1->elements[j].occurrenceid;
      walk1 = walk1->parent;
      val++;
    }
    if ( val > counter.number )
      counter.number = val;
  }
}

*/
