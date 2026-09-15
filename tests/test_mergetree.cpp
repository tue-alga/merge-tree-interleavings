#include "catch.hpp"

#include "mergetree.h"

TEST_CASE("creating a merge tree") {
	MergeTree t;
	int leaf1 = t.addLeaf(3.0);
	int leaf2 = t.addLeaf(4.0);
	int mergeNode = t.merge(leaf1, leaf2, 5.0);

	CHECK(t.size() == 3);
	CHECK(t.parentOf(leaf1) == mergeNode);
	CHECK(t.parentOf(leaf2) == mergeNode);
}

TEST_CASE("computing merge tree parents at a given height") {
	MergeTree t;
	int leaf1 = t.addLeaf(1.0);
	int leaf2 = t.addLeaf(2.0);
	int leaf3 = t.addLeaf(3.0);
	int merge1 = t.merge(leaf1, leaf2, 4.0);
	int merge2 = t.merge(leaf1, leaf3, 5.0);

	CHECK(t.parentAtHeight(leaf1, 1.0) == leaf1);
	CHECK(t.parentAtHeight(leaf1, 2.0) == leaf1);
	CHECK(t.parentAtHeight(leaf1, 3.0) == leaf1);
	CHECK(t.parentAtHeight(leaf1, 4.0) == merge1);
	CHECK(t.parentAtHeight(leaf1, 5.0) == merge2);
	CHECK(t.parentAtHeight(leaf1, 6.0) == merge2);

	CHECK(t.parentAtHeight(leaf2, 2.0) == leaf2);
	CHECK(t.parentAtHeight(leaf2, 3.0) == leaf2);
	CHECK(t.parentAtHeight(leaf2, 4.0) == merge1);
	CHECK(t.parentAtHeight(leaf2, 5.0) == merge2);
	CHECK(t.parentAtHeight(leaf2, 6.0) == merge2);

	CHECK(t.parentAtHeight(leaf3, 3.0) == leaf3);
	CHECK(t.parentAtHeight(leaf3, 4.0) == leaf3);
	CHECK(t.parentAtHeight(leaf3, 5.0) == merge2);
	CHECK(t.parentAtHeight(leaf3, 6.0) == merge2);
}

TEST_CASE("trying to add a merge node below its children adjusts the merge node's height") {
	MergeTree t;
	int leaf1 = t.addLeaf(1.0);
	int leaf2 = t.addLeaf(2.0);
	int leaf3 = t.addLeaf(3.0);
	int merge1 = t.merge(leaf1, leaf2, 5.0);
	int merge2 = t.merge(leaf1, leaf3, 4.0);

	CHECK(t.heightOf(merge1) == 5.0);
	CHECK(t.heightOf(merge2) == 5.0);
}
