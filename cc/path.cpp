// path.cpp
// Siegfried Nijssen, snijssen@liacs.nl, jan 2004.
#include <algorithm>
#include "patterntree.h"
#include "patterngraph.h"
#include "path.h"
#include "graphstate.h"

Path::Path ( NodeLabel startnodelabel ) {
  graphstate.insertStartNode ( startnodelabel );

  nodelabels.push_back ( startnodelabel );
  frontsymmetry = backsymmetry = totalsymmetry = 0;

  cout << "Root " << database.nodelabels[startnodelabel].inputlabel << endl;

  DatabaseNodeLabel &databasenodelabel = database.nodelabels[startnodelabel];

  vector<EdgeLabel> edgelabelorder ( database.edgelabelsindexes.size () );
  EdgeLabel j = 0;

  vector<EdgeLabel> frequentedgelabels;
  for ( int i = 0; i < databasenodelabel.frequentedgelabels.size (); i++ )
    frequentedgelabels.push_back ( database.edgelabels[databasenodelabel.frequentedgelabels[i]].edgelabel );
/*  for ( int i = 0; i < databasenodelabel.edgelabelfrequencies.size (); i++ )
    if ( databasenodelabel.edgelabelfrequencies[i] >= minfreq )
      frequentedgelabels.push_back ( database.edgelabels[i].edgelabel );*/
  sort ( frequentedgelabels.begin (), frequentedgelabels.end () );
  Tid lastself[frequentedgelabels.size ()];

  for ( int i = 0; i < frequentedgelabels.size (); i++ ) {
    edgelabelorder[frequentedgelabels[i]] = j;
    j++;

    PathLegPtr leg = new PathLeg;
    legs.push_back ( leg );
    leg->tuple.depth = 0;
    leg->tuple.edgelabel = frequentedgelabels[i];
    leg->tuple.connectingnode = 0;
    leg->occurrences.parent = &databasenodelabel.occurrences;
    leg->occurrences.number = 2;
    leg->occurrences.maxdegree = 0;
    leg->occurrences.selfjoin = 0;
    lastself[i] = NOTID;
    DatabaseEdgeLabel &databaseedgelabel = database.edgelabels[database.edgelabelsindexes[frequentedgelabels[i]]];
    leg->occurrences.frequency = databaseedgelabel.frequency;
    if ( databaseedgelabel.fromnodelabel == startnodelabel )
      leg->tuple.nodelabel = databaseedgelabel.tonodelabel;
    else
      leg->tuple.nodelabel = databaseedgelabel.fromnodelabel;
  }

  for ( unsigned int i = 0; i < databasenodelabel.occurrences.elements.size (); i++ ) {
    DatabaseTree &tree = * (database.trees[databasenodelabel.occurrences.elements[i].tid]);
    DatabaseTreeNode &datanode = tree.nodes[databasenodelabel.occurrences.elements[i].tonodeid];
    for ( int j = 0; j < datanode.edges.size (); j++ ) {
      EdgeLabel edgelabel = edgelabelorder[datanode.edges[j].edgelabel];
      PathLeg &leg = * ( legs[edgelabel] );
      if ( !leg.occurrences.elements.empty () &&
           leg.occurrences.elements.back ().occurrenceid == i &&
           lastself[edgelabel] != tree.tid ) {
        leg.occurrences.selfjoin++;
        lastself[edgelabel] = tree.tid;
      }
      vector_push_back ( LegOccurrence, leg.occurrences.elements, legoccurrence );
      legoccurrence.tid = tree.tid;
      legoccurrence.occurrenceid = i;
      legoccurrence.tonodeid = datanode.edges[j].tonode;
      legoccurrence.fromnodeid = databasenodelabel.occurrences.elements[i].tonodeid;
    }
  }
  
}

Path::Path ( Path &parentpath, unsigned int legindex ) {
  PathLeg &leg = (*parentpath.legs[legindex]);
  int positionshift;
  
  // OUTPUT(parentpath.legs[legindex]->occurrences.frequency)
  checkTestable(parentpath.legs[legindex]->occurrences.elements, parentpath.legs[legindex]->occurrences.frequency);
 
  // fill in normalisation information, it seems a lot of code, but in fact it's just a lot
  // of code to efficiently perform one walk through the edge/nodelabels arrays.

  nodelabels.resize ( parentpath.nodelabels.size () + 1 );
  edgelabels.resize ( parentpath.edgelabels.size () + 1 );
  addCloseExtensions ( closelegs, parentpath.closelegs, leg.occurrences );

  if ( parentpath.nodelabels.size () == 1 ) {
    totalsymmetry = parentpath.nodelabels[0] - leg.tuple.nodelabel;
    frontsymmetry = backsymmetry = 0;
    nodelabels[1] = leg.tuple.nodelabel;
    edgelabels[0] = leg.tuple.edgelabel;
    nodelabels[0] = parentpath.nodelabels[0];
    positionshift = 0;
  }
  else if ( leg.tuple.depth == 0 ) {
    positionshift = 1;
    nodelabels[0] = leg.tuple.nodelabel;
    edgelabels[0] = leg.tuple.edgelabel;

    backsymmetry = parentpath.totalsymmetry;
    frontsymmetry = leg.tuple.nodelabel - parentpath.nodelabels[parentpath.nodelabels.size () - 2];
    totalsymmetry = leg.tuple.nodelabel - parentpath.nodelabels.back ();
    if ( !totalsymmetry )
      totalsymmetry = leg.tuple.edgelabel - parentpath.edgelabels.back ();

    int i = 0;
    // we can prepend only before strings of length 2
    if ( parentpath.nodelabels.size () > 2 ) {
      if ( !frontsymmetry )
        frontsymmetry = leg.tuple.edgelabel - parentpath.edgelabels[parentpath.nodelabels.size () - 3];

      while ( !frontsymmetry && i < parentpath.edgelabels.size () / 2 ) {
        nodelabels[i + 1] = parentpath.nodelabels[i];
        edgelabels[i + 1] = parentpath.edgelabels[i];

        frontsymmetry = parentpath.nodelabels[i] - parentpath.nodelabels[parentpath.nodelabels.size () - i - 3];
        if ( !frontsymmetry && parentpath.nodelabels.size () > 3 )
          frontsymmetry = parentpath.edgelabels[i] - parentpath.edgelabels[parentpath.nodelabels.size () - i - 4];

	if ( !totalsymmetry ) {
	  totalsymmetry = parentpath.nodelabels[i] - parentpath.nodelabels[parentpath.nodelabels.size () - i - 2];
	  if ( !totalsymmetry )
	    totalsymmetry = parentpath.edgelabels[i] - parentpath.edgelabels[parentpath.nodelabels.size () - i - 3];
	}

        i++;
      }
    }
    for ( ; !totalsymmetry && i < parentpath.edgelabels.size () / 2; i++ ) {
      nodelabels[i + 1] = parentpath.nodelabels[i];
      edgelabels[i + 1] = parentpath.edgelabels[i];

      totalsymmetry = parentpath.nodelabels[i] - parentpath.nodelabels[parentpath.nodelabels.size () - i - 2];
      if ( !totalsymmetry && parentpath.nodelabels.size () > 3 )
        totalsymmetry = parentpath.edgelabels[i] - parentpath.edgelabels[parentpath.nodelabels.size () - i - 3];
    }
    for ( ;i < parentpath.edgelabels.size (); i++ ) {
      nodelabels[i + 1] = parentpath.nodelabels[i];
      edgelabels[i + 1] = parentpath.edgelabels[i];
    }

    nodelabels[i + 1] = parentpath.nodelabels[i];

    // build OccurrenceLists
    extend ( leg.occurrences );
    for ( int i = 0; i < candidatelegsoccurrences.size (); i++ ) {
      if ( candidatelegsoccurrences[i].frequency >= minfreq ) {
        PathLegPtr leg2 = new PathLeg;
        legs.push_back ( leg2 );
        leg2->tuple.edgelabel = i;
	leg2->tuple.connectingnode = graphstate.lastNode ();
        DatabaseEdgeLabel &databaseedgelabel = database.edgelabels[database.edgelabelsindexes[i]];
        if ( databaseedgelabel.fromnodelabel == leg.tuple.nodelabel )
          leg2->tuple.nodelabel = databaseedgelabel.tonodelabel;
        else
          leg2->tuple.nodelabel = databaseedgelabel.fromnodelabel;
        leg2->tuple.depth = 0;
        store ( leg2->occurrences, candidatelegsoccurrences[i] ); // avoid copying
      }
    }
  }
  else  {
    positionshift = 0;

    frontsymmetry = parentpath.totalsymmetry;
    backsymmetry = parentpath.nodelabels[1] - leg.tuple.nodelabel;
    totalsymmetry = parentpath.nodelabels[0] - leg.tuple.nodelabel;
    if ( !totalsymmetry )
      totalsymmetry = parentpath.edgelabels[0] - leg.tuple.edgelabel;
    int i = 0;
    if ( parentpath.nodelabels.size () > 2 ) {
      if ( !backsymmetry )
        backsymmetry = parentpath.edgelabels[1] - leg.tuple.edgelabel;

      while ( !backsymmetry && i < parentpath.edgelabels.size () / 2 ) {
        nodelabels[i] = parentpath.nodelabels[i];
	edgelabels[i] = parentpath.edgelabels[i];

	backsymmetry = parentpath.nodelabels[i + 2] - parentpath.nodelabels[parentpath.nodelabels.size () - i - 1];
	if ( !backsymmetry && parentpath.nodelabels.size () > 3 )
	  backsymmetry = parentpath.edgelabels[i + 2] - parentpath.edgelabels[parentpath.nodelabels.size () - i - 2];

	if ( !totalsymmetry ) {
	  totalsymmetry = parentpath.nodelabels[i + 1] - parentpath.nodelabels[parentpath.nodelabels.size () - i - 1];
	  if ( !totalsymmetry && parentpath.nodelabels.size () > 3 )
	    totalsymmetry = parentpath.edgelabels[i + 1] - parentpath.edgelabels[parentpath.nodelabels.size () - i - 2];
	}
	i++;
      }
    }
    for ( ; !totalsymmetry && i < parentpath.edgelabels.size () / 2; i++ ) {
      nodelabels[i] = parentpath.nodelabels[i];
      edgelabels[i] = parentpath.edgelabels[i];
      totalsymmetry = parentpath.nodelabels[i + 1] - parentpath.nodelabels[parentpath.nodelabels.size () - i - 1];
      if ( !totalsymmetry && i < parentpath.edgelabels.size () - 1 )
        totalsymmetry = parentpath.edgelabels[i + 1] - parentpath.edgelabels[parentpath.nodelabels.size () - i - 2];
    }
    for ( ; i < parentpath.edgelabels.size (); i++ ) {
      nodelabels[i] = parentpath.nodelabels[i];
      edgelabels[i] = parentpath.edgelabels[i];
    }

    nodelabels[i] = parentpath.nodelabels[i];
    edgelabels[i] = leg.tuple.edgelabel;
    nodelabels[i+1] = leg.tuple.nodelabel;
  }

  int i = 0;
  LegOccurrencesPtr legoccurrencesptr;
  for ( ; i < legindex; i++ ) {
    PathLeg &leg2 = (*parentpath.legs[i]);

    if ( legoccurrencesptr = join ( leg.occurrences, leg2.tuple.connectingnode, leg2.occurrences ) ) {
      PathLegPtr leg3 = new PathLeg;
      legs.push_back ( leg3 );
      leg3->tuple.connectingnode = leg2.tuple.connectingnode;
      leg3->tuple.edgelabel = leg2.tuple.edgelabel;
      leg3->tuple.nodelabel = leg2.tuple.nodelabel;
      leg3->tuple.depth = leg2.tuple.depth + positionshift;
      store ( leg3->occurrences, *legoccurrencesptr );
    }
  }

  if ( legoccurrencesptr = join ( leg.occurrences ) ) {
    PathLegPtr leg3 = new PathLeg;
    legs.push_back ( leg3 );
    leg3->tuple.connectingnode = leg.tuple.connectingnode;
    leg3->tuple.edgelabel = leg.tuple.edgelabel;
    leg3->tuple.nodelabel = leg.tuple.nodelabel;
    leg3->tuple.depth = leg.tuple.depth + positionshift;
    store ( leg3->occurrences, *legoccurrencesptr );
  }

  for ( i++; i < parentpath.legs.size (); i++ ) {
    PathLeg &leg2 = (*parentpath.legs[i]);
    if ( legoccurrencesptr = join ( leg.occurrences, leg2.tuple.connectingnode, leg2.occurrences ) ) {
      PathLegPtr leg3 = new PathLeg;
      legs.push_back ( leg3 );
      leg3->tuple.connectingnode = leg2.tuple.connectingnode;
      leg3->tuple.edgelabel = leg2.tuple.edgelabel;
      leg3->tuple.nodelabel = leg2.tuple.nodelabel;
      leg3->tuple.depth = leg2.tuple.depth + positionshift;
      store ( leg3->occurrences, *legoccurrencesptr );
    }
  }

  if ( positionshift ) {
    addCloseExtensions ( closelegs, leg.occurrences.number ); // stored separately
    return;
  }

  extend ( leg.occurrences );
  for ( int i = 0; i < candidatelegsoccurrences.size (); i++ ) {
    if ( candidatelegsoccurrences[i].frequency >= minfreq ) {
      PathLegPtr leg2 = new PathLeg;
      legs.push_back ( leg2 );
      leg2->tuple.edgelabel = i;
      leg2->tuple.connectingnode = graphstate.lastNode ();
      DatabaseEdgeLabel &databaseedgelabel = database.edgelabels[database.edgelabelsindexes[i]];
      if ( databaseedgelabel.fromnodelabel == leg.tuple.nodelabel )
        leg2->tuple.nodelabel = databaseedgelabel.tonodelabel;
      else
        leg2->tuple.nodelabel = databaseedgelabel.fromnodelabel;
      leg2->tuple.depth = leg.tuple.depth + 1;
      store ( leg2->occurrences, candidatelegsoccurrences[i] ); // avoid copying
    }
  }

  addCloseExtensions ( closelegs, leg.occurrences.number );
}

Path::~Path () {
  for ( int i = 0; i < legs.size (); i++ )
    delete legs[i];
  for ( int i = 0; i < closelegs.size (); i++ )
    delete closelegs[i];
}

// ADDED
bool Path::isnormal ( EdgeLabel edgelabel ) {
  // symplistic quadratic algorithm
  int nodelabelssize = nodelabels.size (), step, add, start;
  
  edgelabels.push_back ( edgelabel );
    
  // if we would program it better, we would use the 'totalsymmetry' variable here;
  // however, to be quick & easy, we used a different coding here...
  int t = nodelabelssize - 1, r = 0;
  int symmetry;
  do {
    symmetry = nodelabels[t] - nodelabels[r];
    int nt = ( t + nodelabelssize - 1 ) % nodelabelssize;
    if ( !symmetry ) 
      symmetry = edgelabels[nt] - edgelabels[r];
    r = ( r + 1 ) % nodelabelssize;
    t = nt;
  }
  while ( symmetry == 0 && t != nodelabelssize - 1 );
  
  if ( symmetry < 0 ) {
    step = -1 + nodelabelssize ;
    add = -1 + nodelabelssize ;
    start = nodelabelssize - 1;
  }
  else {
    step = 1 + nodelabelssize;
    add = nodelabelssize ;
    start = 0;
  }
  for ( int i = 0; i < nodelabelssize; i++ ) {
    // starting positions for the new path
    int k = start, l = i, p;
    do {
      if ( nodelabels[l] < nodelabels[k] ) {
        edgelabels.pop_back ();
        return false;
      }
      if ( nodelabels[l] > nodelabels[k] )
        break;
      p = ( k + add ) % nodelabelssize;
      l = ( l + nodelabelssize - 1 ) % nodelabelssize;
      if ( edgelabels[l] < edgelabels[p] ) {
        edgelabels.pop_back ();
        return false;
      }
      if ( edgelabels[l] > edgelabels[p] ) 
        break;
      k = ( k + step ) % nodelabelssize;
    }
    while ( k != start );
    
    k = start, l = i;
    do {
      if ( nodelabels[l] < nodelabels[k] ) {
        edgelabels.pop_back ();
        return false;
      }
      if ( nodelabels[l] > nodelabels[k] ) 
        break;
      p = ( k + add ) % nodelabelssize;
      if ( edgelabels[l] < edgelabels[p] ) {
        edgelabels.pop_back ();
        return false;
      }
      if ( edgelabels[l] > edgelabels[p] ) 
        break;
      l = ( l + 1 ) % nodelabelssize;
      k = ( k + step ) % nodelabelssize;
    }
    while ( k != start );
    
  }
  edgelabels.pop_back ();
  return true;
}

void Path::expand2 () {
  // does not work for strings with only one node
  statistics.patternsize++;
  if ( statistics.patternsize > statistics.frequenttreenumbers.size () ) {
    statistics.frequenttreenumbers.push_back ( 0 );
    statistics.frequentpathnumbers.push_back ( 0 );
    statistics.frequentgraphnumbers.push_back ( 0 );
  }
  ++statistics.frequentpathnumbers[statistics.patternsize-1];
  
  if ( statistics.patternsize == maxsize ) {
    statistics.patternsize--;
    return;
  }
  
  if ( closelegs.size () && phase > 2 ) {
    
    NodeId from = graphstate.nodes.size ();
    NodeId to = 0;
    while ( graphstate.nodes[to].edges.size () == 2 )
      to++;
    to++;
      
    for ( int i = 0; i < closelegs.size (); i++ ) {
      if ( closelegs[i]->tuple.from == from &&
           closelegs[i]->tuple.to == to &&
           isnormal ( closelegs[i]->tuple.label ) ) {
        graphstate.insertEdge ( closelegs[i]->tuple.from, closelegs[i]->tuple.to, closelegs[i]->tuple.label );
        // OUTPUT(closelegs[i]->occurrences.frequency)
	checkTestableCl(closelegs[i]->occurrences.elements, closelegs[i]->occurrences.frequency);
        int addsize = statistics.patternsize + graphstate.edgessize - graphstate.nodes.size ();
        if ( addsize >= statistics.frequenttreenumbers.size () ) {
          statistics.frequenttreenumbers.resize ( addsize + 1, 0 );
          statistics.frequentpathnumbers.resize ( addsize + 1, 0 );
          statistics.frequentgraphnumbers.resize ( addsize + 1, 0 );
        }
        statistics.frequentgraphnumbers[addsize]++;
        graphstate.deleteEdge ( closelegs[i]->tuple.from, closelegs[i]->tuple.to );
        
        // DON'T RECURSE GRAPH GROWING!
        // only circle graphs can only grow from paths, all other graphs
        // can grow from spanning trees, which we prefer for now
      }
    }
  }

  for ( int i = 0; i < legs.size (); i++ ) {
    PathTuple &tuple = legs[i]->tuple;
    if ( tuple.depth == nodelabels.size () - 1 ) {
      if ( tuple.nodelabel > nodelabels[0] ||
           ( tuple.nodelabel == nodelabels[0] &&
             ( tuple.edgelabel > edgelabels[0] ||
               ( tuple.edgelabel == edgelabels[0] && backsymmetry <= 0 )
             )
           ) ) {
        graphstate.insertNode ( legs[i]->tuple.connectingnode, legs[i]->tuple.edgelabel, legs[i]->occurrences.maxdegree );
        Path path ( *this, i );
        path.expand2 ();
	graphstate.deleteNode ();
      }
    }
    else
      if ( legs[i]->tuple.depth == 0 ) {
        if ( totalsymmetry &&
             ( tuple.nodelabel > nodelabels.back () ||
             ( tuple.nodelabel == nodelabels.back () &&
               ( tuple.edgelabel > edgelabels.back () ||
                 ( tuple.edgelabel == edgelabels.back () && frontsymmetry >= 0 )
               )
             ) ) ) {
          graphstate.insertNode ( legs[i]->tuple.connectingnode, legs[i]->tuple.edgelabel, legs[i]->occurrences.maxdegree );
          Path path ( *this, i );
          path.expand2 ();
	  graphstate.deleteNode ();
        }
      }
      else {
        if ( ( totalsymmetry || legs[i]->tuple.depth <= edgelabels.size () / 2 ) &&
	     ( legs[i]->tuple.depth != 1 || legs[i]->tuple.edgelabel >= edgelabels[0] ) &&
	     ( legs[i]->tuple.depth != nodelabels.size () - 2 || legs[i]->tuple.edgelabel >= edgelabels.back () ) &&
	     phase > 1
	   ) {
          graphstate.insertNode ( legs[i]->tuple.connectingnode, legs[i]->tuple.edgelabel, legs[i]->occurrences.maxdegree );
	  PatternTree tree ( *this, i );
	  tree.expand ();
	  graphstate.deleteNode ();
	 // grow tree
        }
      }
  }

  statistics.patternsize--;
}

void Path::expand () {

  for ( int i = 0; i < legs.size (); i++ ) {
    PathTuple &tuple = legs[i]->tuple;
    if ( tuple.nodelabel >= nodelabels[0] ) {
      graphstate.insertNode ( tuple.connectingnode, tuple.edgelabel, legs[i]->occurrences.maxdegree );
      Path path ( *this, i );
      path.expand2 ();
      graphstate.deleteNode ();
    }
  }
  graphstate.deleteStartNode ();
}

ostream &operator<< ( ostream &stream, Path &path ) {
  stream << /* database.nodelabels[ */ (int) path.nodelabels[0] /* ].inputlabel; */ << " ";
  for ( int i = 0; i < path.edgelabels.size (); i++ ) {
    //stream << (char) ( path.edgelabels[i] + 'A' ) << path.nodelabels[i+1];
    stream << /*database.edgelabels[database.edgelabelsindexes[*/ (int) path.edgelabels[i] /*]].inputedgelabel */ << " " <<  /* database.nodelabels[ */ (int) path.nodelabels[i+1] /* ].inputlabel */ << " ";
  }
  return stream;
}
