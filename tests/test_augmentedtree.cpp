#include "catch.hpp"

#include "augmentedtree.h"

TEST_CASE("create a simple augmented tree") {
	MergeTree tree;

	int l1 = tree.addLeaf(0, {0, 0});
	int l2 = tree.addLeaf(1, {0, 1});
	tree.merge(l1, l2, 2, {1, 0});

	AugmentedTree at(tree, {-1, 0, 1, 2}, {});

	REQUIRE(at.get(0).size() == 0);

	REQUIRE(at.get(1).size() == 1);
	CHECK(at.getChildren(1, {0}).size() == 0);

	REQUIRE(at.get(2).size() == 2);
	CHECK(at.getChildren(2, {0}).size() == 1);
	CHECK(at.getChildren(2, {1}).size() == 0);

	REQUIRE(at.get(3).size() == 1);
	CHECK(at.getChildren(3, {0}).size() == 2);
}

TEST_CASE("create a slightly more complex augmented tree") {
	MergeTree t2;
	int t2l1 = t2.addLeaf(0, {0, 0});
	int t2l2 = t2.addLeaf(1, {0, 1});
	int t2l3 = t2.addLeaf(1, {0, 2});
	t2.merge(t2l1, t2l2, 2, {1, 0});
	t2.merge(t2l1, t2l3, 3, {1, 0});

	AugmentedTree at(t2, {0, 1, 2, 3}, {});

	REQUIRE(at.get(2).size() == 2);
	CHECK(at.getChildren(2, {0}).size() == 2);
	CHECK(at.getChildren(2, {1}).size() == 1);
}

TEST_CASE("compute an augmented tree with duplicate levels") {
	MergeTree tree;

	int l1 = tree.addLeaf(0, {0, 0});
	int l2 = tree.addLeaf(1, {0, 1});
	tree.merge(l1, l2, 2, {1, 0});

	AugmentedTree at(tree, {0, 1, 1, 1.5, 2, 2}, {});

	REQUIRE(at.get(0).size() == 1);
	CHECK(at.get(0, 0).m_isMergeTreeNode == true);
	CHECK(at.getChildren(0, {0}).size() == 0);

	REQUIRE(at.get(1).size() == 2);
	CHECK(at.get(1, 0).m_isMergeTreeNode == false);
	CHECK(at.getChildren(1, {0}).size() == 1);
	CHECK(at.get(1, 1).m_isMergeTreeNode == true);
	CHECK(at.getChildren(1, {1}).size() == 0);

	REQUIRE(at.get(2).size() == 2);
	CHECK(at.get(2, 0).m_isMergeTreeNode == false);
	CHECK(at.getChildren(2, {0}).size() == 1);
	CHECK(at.get(2, 1).m_isMergeTreeNode == false);
	CHECK(at.getChildren(2, {1}).size() == 1);

	REQUIRE(at.get(3).size() == 2);
	CHECK(at.get(3, 0).m_isMergeTreeNode == false);
	CHECK(at.getChildren(3, {0}).size() == 1);
	CHECK(at.get(3, 1).m_isMergeTreeNode == false);
	CHECK(at.getChildren(3, {1}).size() == 1);

	REQUIRE(at.get(4).size() == 1);
	CHECK(at.get(4, 0).m_isMergeTreeNode == true);
	CHECK(at.getChildren(4, {0}).size() == 2);

	REQUIRE(at.get(5).size() == 1);
	CHECK(at.get(5, 0).m_isMergeTreeNode == false);
	CHECK(at.getChildren(5, {0}).size() == 1);
}

/*TEST_CASE("compute LCAs") {
	MergeTree tree;

	int l1 = tree.addLeaf(0);
	int l2 = tree.addLeaf(0);
	int l3 = tree.addLeaf(0);
	int l4 = tree.addLeaf(1);
	int l5 = tree.addLeaf(1);
	tree.merge(l1, l2, 2);
	tree.merge(l2, l3, 3);
	tree.merge(l4, l5, 3);
	tree.merge(l1, l4, 4);

	AugmentedTree at(tree, {0, 1, 2, 3, 4, 5}, {});

	CHECK(at.levelOfLCA(1, 0, 1) == 2);
	CHECK(at.levelOfLCA(1, 0, 2) == 3);
	CHECK(at.levelOfLCA(1, 0, 3) == 4);
	CHECK(at.levelOfLCA(1, 0, 4) == 4);
	CHECK(at.levelOfLCA(1, 1, 2) == 3);
	CHECK(at.levelOfLCA(1, 1, 3) == 4);
	CHECK(at.levelOfLCA(1, 1, 4) == 4);
	CHECK(at.levelOfLCA(1, 2, 3) == 4);
	CHECK(at.levelOfLCA(1, 2, 4) == 4);
	CHECK(at.levelOfLCA(1, 3, 4) == 3);
}*/
