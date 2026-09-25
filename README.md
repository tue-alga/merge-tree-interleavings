Merge Tree Interleavings
=======================================================

This repository contains code to compute the interleaving distance between two merge trees. The implementation includes two algorithms: the fixed-parameter tractable (FPT) algorithm by Touli and Wang [2], and a modification of their algorithm based on a sweepline through both trees [1].

The code is free software licensed under the GNU General Public License version 3.


## Compiling

First clone the repository: 

```sh
git clone https://github.com/tue-alga/merge-tree-interleavings.git
```

Create a build directory and build the code:
```sh
cmake -S . -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build
```


## Running

Call `build/interleavingcli --help` to get information on how to run the tool.


### Input

The tool needs merge tree files and (optionally) a restriction matrix as input. Merge trees are assumed to have exactly two children per merge (= internal vertex) of which the height is strictly lower than the merge.

The syntax of the merge tree files is:

* One line giving the number *n* of leaves.
* *n* lines, one for each leaf, giving (space-separated) the leaf's ID, height, x coordinate, y coordinate, and TTK ID. The IDs should start at 0 and increase sequentially. The x coordinate, y coordinate, and TTK ID are meant for debugging purposes and are not actually used for the interleaving distance computation; they can safely be set to `0`.
* One line giving the number *m* of merges. (This should be equal to *n* - 1.)
* *m* lines, one for each merge, giving (space-separated) the merge's ID, height, x coordinate, y coordinate, ID of first child, ID of second child, TTK ID. The IDs should start at *n* (so the IDs keep numbering onwards from the leaves' IDs) and increase sequentially.

The syntax of the restriction matrix is:

* *n*<sub>source</sub> lines, one for each leaf *l*₁ of the source tree, each containing *n*<sub>target</sub> numbers, one for each leaf *l*₂ of the target tree, representing the height of the lowest ancestor of *l*₂ that *l*₁ is allowed to map to. See §4 of [1] for details.

The restriction matrix can be omitted; in this case no restrictions are applied.

An example is provided in `data/example`. You can run the example using

```
build/interleavingcli data/example/source-tree.txt data/example/target-tree.txt data/example/restriction-matrix.txt sweepline/exponential
```


### Output

The tool reports progress information to stderr. When done it writes a line to stdout with the results, containing (tab-separated):

* the filename of the restriction matrix;
* the algorithm used;
* the computed interleaving distance;
* the time it took to do the full computation;
* for each call to the decision procedure: 
    * the δ for which the decision procedure was called;
    * the time it took to run the decision procedure.


### Experiments

The tool is intended to be run from the runner script `experiments/runner.py`. This then results in a complete CSV file.

To reproduce the experiments from [1], call `python3 experiments/runner.py data/processed/<name>/` for `<name>` being `heated-cylinder` or `redsea`.

The data in `data/processed` was generated from `data/raw`. To reproduce this process, call `pvpython experiments/data-extraction/generate_data.py`. You need ParaView and TTK for this; we used ParaView 6.0.1. The output appears in `data/generated`.


## References

[1] T. Beurskens, E.T. Gæde, T. Ophelders, W. Sonke, B. Speckmann, and K. Verbeek. A practical algorithm for (geometry-aware) interleavings between merge trees. *In Proc. 24th International Symposium on Experimental Algorithms (SEA)*, pages 6:1–6:18, 2026, [doi](https://doi.org/10.4230/LIPIcs.SEA.2026.6.).

[2] E.F. Touli and Y.Wang. FPT-algorithms for computing the Gromov-Hausdorff and interleaving distances between trees. *Journal of Computational Geometry*, 13:89–124, 2022, [doi](https://doi.org/10.20382/jocg.v13i1a4).
