Significant Subgraph Mining with Multiple Testing Correction
============================================================

Efficient detection of all significant subgraphs from graph databases while correcting for multiple testing.
GASTON [1] is used as a frequent subgraph mining algorithm.

[1] Nijssen, S. and Kok, J.: **A Quickstart in Frequent Structure Mining Can
  Make a Difference,** *ACM SIGKDD*, 647-652 (2004) [http://www.liacs.nl/~snijssen/gaston/iccs.html]


Usage
-----

Given a graph database and a predetermined significance level, this program computes the corrected significance level for each test and outputs all significant subgraphs.

The input is composed of two files: a graph database (a list of graphs) and a list of class labels.
In a graph database, each graph is described as follows:

```
# <comment>
t <transaction id>
v <node id> <label id>
e <source node id> <target node id> <label id>
```

For example:

```
# start  
t # 1        // transaction id is 1
v 0 1        // node 1 with label 1
v 1 2        // node 2 with label 2
e 0 1 3      // edge from node 1 to 2 with label 3
```

In the class file, each line contains the class label of the corresponding graph.
The class label should be either "0" or "1", and "1" should represent the minority class.


To compile the program, type

```
make
```

Then, to run the algorithm, type

```
./sgmine -a <alpha> -i <input_file> -c <input_class_file> -o <output_file>
```

* `<alpha>` is the overall significance level. It is set to be 0.05 if skipped.
* Resulting significant subgraphs are written to the file `<output_file>`. The output file has the same format as the input file except for two lines: the support (start from "\#") and the *p*-value (start from "p") of the graph. For example:

  ```
  # 29         // support of this subgraph
  p 1.5841e-07 // p-value of this subgraph
  t 1
  v 0 0
  v 1 9
  v 2 9
  e 0 1 0
  e 1 2 0
  ```

Example
-------

There is an example graph database "Chemical_340" and the corresponding class file "Chemical_340_class". To run the algorithm on this database, type:

```
./sgmine -i Chemical_340 -c Chemical_340_class -o output
```

or if you want to output resulting statistics to a file "stat", type:

```
./sgmine -i Chemical_340 -c Chemical_340_class -o output > stat
```


Contact
-------

* Author: Mahito Sugiyama
* Affiliation: ISIR, Osaka University, Japan
* Mail: mahito@ar.sanken.osaka-u.ac.jp
