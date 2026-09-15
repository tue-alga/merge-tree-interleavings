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
cmake -S . -B build
cmake --build build
```

## References

[1] T. Beurskens, E.T. Gæde, T. Ophelders, W. Sonke, B. Speckmann, and K. Verbeek. A practical algorithm for (geometry-aware) interleavings between merge trees. *In Proc. 24th International Symposium on Experimental Algorithms (SEA)*, pages 6:1–6:18, 2026, [doi](https://doi.org/10.4230/LIPIcs.SEA.2026.6.).

[2] E.F. Touli and Y.Wang. FPT-algorithms for computing the Gromov-Hausdorff and interleaving distances between trees. *Journal of Computational Geometry*, 13:89–124, 2022, [doi](https://doi.org/10.20382/jocg.v13i1a4).
