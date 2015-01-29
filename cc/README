README
Siegfried Nijssen, snijssen@liacs.nl, november 2004.

This code implements GASTON, a frequent subGrAph, Sequences and Tree ExtractiON
algorithm for general graph databases.

USAGE
-----
The input is as follows:

(line of comment in the format: # (comment))
(transaction header in the format: t (comment))
(nodes and labels for each of the nodes in the format: v (node id) (label number))
(edges in the format: e (source node id) (target node id) (label number))

...

An example is:

# start
t # 1        (transactionid is 1)
v 0 1        (label of node 1 is 1)
v 1 2        (label of node 2 is 2)
e 0 1 3      (edge from node 1 to node 2, with label 3)

To run the algorithm, first type

make

and then

gaston (support) (input file)

or 

gaston (support) (input file) (output file)

if an output file is specified all frequent graphs will be written in the 
output file. The output file has the same format as the input file,
except that before each frequent graph a comment is added which 
gives the support of the graph in the original database, as an example:

# 4
t # 1        (transactionid is 1)
v 0 1        (label of node 1 is 1)
v 1 2        (label of node 2 is 2)
e 0 1 3      (edge from node 1 to node 2, with label 3)

This would mean that the corresponding graph occurs in 4 graphs in
the original database.

As an example file Chemical_340 is provided. To run Gaston on this file,
type:

gaston 7 Chemical_340

or

gaston 7 Chemical_340 Chemical_340.out


COMMAND LINE OPTIONS
--------------------
-m i: specify that the largest frequent pattern to be examined has i nodes
-t:   only output frequent paths and trees, no cyclic graphs
-p:   only output frequent paths, no trees and cyclic graphs

KNOWN ISSUES
------------
Gaston uses occurrence lists to determine the occurrences of a graph.
These occurrence lists require a lot of main memory. To some extent
the amount of main memory can be reduced by modifying the typedefs
for Tids, NodeIds and EdgeLabels in misc.h. Alternatively, download
the version of Gaston which does not use occurrence lists.

LICENSE
-------
All files in the main directory come under the LGPL license which can be found in the
LICENSE file.  If you use this code for scientific purposes, it would be appreciated
if you add proper references to your publication:
- Siegfried Nijssen and Joost Kok. A Quickstart in Frequent Structure Mining Can 
  Make a Difference. Proceedings of the SIGKDD, 2004.
- http://www.liacs.nl/home/snijssen/gaston/

