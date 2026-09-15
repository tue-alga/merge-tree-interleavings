#include "catch.hpp"

#include <limits>
#include <memory>

#include "interleavingdistance.h"

TEST_CASE("test the three basic cases of the interleaving distance") {
	auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();

	SECTION("leaf-leaf bottleneck") {
		t1->addLeaf(2.0);
		t2->addLeaf(3.0);
	}

	SECTION("vertex-vertex bottleneck") {
		int t1l1 = t1->addLeaf(2.0);
		int t1l2 = t1->addLeaf(2.0);
		t1->merge(t1l1, t1l2, 3.0);

		int t2l1 = t2->addLeaf(2.0);
		int t2l2 = t2->addLeaf(2.0);
		t2->merge(t2l1, t2l2, 4.0);
	}

	SECTION("zigzag bottleneck") {
		int t1l1 = t1->addLeaf(2.0);
		int t1l2 = t1->addLeaf(2.0);
		t1->merge(t1l1, t1l2, 4.0);

		t2->addLeaf(2.0);
	}

	Interleaving forwardInterleaving = computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(forwardInterleaving.getDelta() == Approx(1.0));
	Interleaving backwardInterleaving =
	    computeInterleavingDistance(t2, t1, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(backwardInterleaving.getDelta() == Approx(1.0));

	Interleaving forwardFastInterleaving =
	    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(forwardFastInterleaving.getDelta() == Approx(1.0));
	Interleaving backwardFastInterleaving =
	    computeInterleavingDistance(t2, t1, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(backwardFastInterleaving.getDelta() == Approx(1.0));
}

TEST_CASE("test the interleaving distance on larger trees") {
	auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();

	int t1l1 = t1->addLeaf(0.0);
	int t1l2 = t1->addLeaf(1.0);
	int t1l3 = t1->addLeaf(2.0);
	t1->merge(t1l1, t1l2, 2.0);
	t1->merge(t1l1, t1l3, 3.0);

	int t2l1 = t2->addLeaf(0.5);
	int t2l2 = t2->addLeaf(2.5);
	t2->merge(t2l1, t2l2, 3.5);

	Interleaving forwardInterleaving = computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(forwardInterleaving.getDelta() == Approx(0.5));
	Interleaving backwardInterleaving =
	    computeInterleavingDistance(t2, t1, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(backwardInterleaving.getDelta() == Approx(0.5));

	Interleaving forwardFastInterleaving =
	    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(forwardFastInterleaving.getDelta() == Approx(0.5));
	Interleaving backwardFastInterleaving =
	    computeInterleavingDistance(t2, t1, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(backwardFastInterleaving.getDelta() == Approx(0.5));
}

TEST_CASE("test the interleaving distance on two identical trees") {
	auto t1 = std::make_shared<MergeTree>();

	int t1l1 = t1->addLeaf(0.0);
	int t1l2 = t1->addLeaf(1.0);
	int t1l3 = t1->addLeaf(2.0);
	t1->merge(t1l1, t1l2, 2.0);
	t1->merge(t1l1, t1l3, 3.0);

	Interleaving forwardInterleaving = computeInterleavingDistance(t1, t1, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(forwardInterleaving.getDelta() == Approx(0.0));

	Interleaving forwardFastInterleaving =
	    computeInterleavingDistance(t1, t1, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(forwardFastInterleaving.getDelta() == Approx(0.0));
}

TEST_CASE("test the interleaving distance on a tree with a merge at height ∞") {
	// This tests if the interleaving distance deals properly with trees that
	// have a node at height ∞. This is a special case because ∞ - ∞ = NaN, and
	// trying to compute a δ-good set for δ = NaN does not make much sense.

	auto t = std::make_shared<MergeTree>();
	int l1 = t->addLeaf(0.0);
	int l2 = t->addLeaf(0.0);
	t->merge(l1, l2, std::numeric_limits<double>::infinity());

	Interleaving interleaving = computeInterleavingDistance(t, t, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(interleaving.getDelta() == Approx(0.0));

	Interleaving fastInterleaving = computeInterleavingDistance(t, t, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(fastInterleaving.getDelta() == Approx(0.0));
}

TEST_CASE("test the interleaving distance with input susceptible to floating-point errors") {
	// This tests if the interleaving distance deals properly with
	// floating-point rounding. We construct two trees with interleaving
	// distance 1. The leaves of tree `t1` have heights differing by just 1e-20.
	// These leaves should correspond to distinct levels. In the level set for
	// `t1` this is naturally true, as 0 and 1e-20 have distinct floating-point
	// representations. However, a naive implementation may compute the level
	// set for `t2` by adding the δ under test (e.g., 0.5 or 1) to each height
	// in `t1`, in which case we obtain δ and δ + 1e-20, which have identical
	// floating-point representations. In this case the naive implementation
	// could conclude that these are the same level, resulting in the number of
	// levels for the two trees being different (which should be impossible and
	// would likely result in a crash).

	auto t1 = std::make_shared<MergeTree>();
	int t1l1 = t1->addLeaf(0.0);
	int t1l2 = t1->addLeaf(1e-20);
	t1->merge(t1l1, t1l2, 2.0);

	auto t2 = std::make_shared<MergeTree>();
	int t2l1 = t2->addLeaf(0.0);
	int t2l2 = t2->addLeaf(1.0);
	t2->merge(t2l1, t2l2, 2.0);

	Interleaving interleaving = computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);
	CHECK(interleaving.getDelta() == Approx(1.0));

	Interleaving fastInterleaving =
	    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline);
	CHECK(fastInterleaving.getDelta() == Approx(1.0));
}

TEST_CASE("test the restricted interleaving distance on single-leaf trees") {
	auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();
	RestrictionMatrix restrictions;

	SECTION("source leaf < target leaf < target restriction") {
		t1->addLeaf(2.0);
		t2->addLeaf(3.0);
		restrictions = {{3.5}};
	}

	SECTION("target leaf < source leaf < target restriction (restriction is bottleneck)") {
		t1->addLeaf(3.0);
		t2->addLeaf(2.0);
		restrictions = {{4.5}};
	}

	SECTION("target leaf < source leaf < target restriction (back map is bottleneck)") {
		t1->addLeaf(3.5);
		t2->addLeaf(2.0);
		restrictions = {{4.5}};
	}

	SECTION("target leaf < target restriction < source leaf") {
		t1->addLeaf(3.5);
		t2->addLeaf(2.0);
		restrictions = {{3.0}};
	}

	Interleaving interleaving =
	    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP, restrictions);
	CHECK(interleaving.getDelta() == Approx(1.5));

	Interleaving fastInterleaving =
	    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline, restrictions);
	CHECK(fastInterleaving.getDelta() == Approx(1.5));
}

TEST_CASE("test the restricted interleaving distance on larger trees") {
	auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();
	RestrictionMatrix restrictions;
	double expected;

	SECTION("vertex-vertex bottleneck") {
		int t1l1 = t1->addLeaf(1.0);
		int t1l2 = t1->addLeaf(1.0);
		t1->merge(t1l1, t1l2, 3.0);

		int t2l1 = t2->addLeaf(1.0);
		int t2l2 = t2->addLeaf(1.0);
		t2->merge(t2l1, t2l2, 4.0);

		SECTION("restriction is not the bottleneck") {
			restrictions = {{2.0, 2.0}, {2.0, 2.0}};
			expected = 1.0;
		}
		SECTION("restriction causes a bottleneck") {
			restrictions = {{2.0, 4.0}, {2.0, 4.0}};
			expected = 1.5;
		}
	}

	{
		Interleaving interleaving =
		    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP, restrictions);
		CHECK(interleaving.getDelta() == Approx(expected));
	}

	{
		Interleaving interleaving =
		    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::Sweepline, restrictions);
		CHECK(interleaving.getDelta() == Approx(expected));
	}

	{
		Interleaving interleaving =
		    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaExponentialSearch, DeltaGoodMapAlgorithm::DP, restrictions);
		CHECK(interleaving.getDelta() == Approx(expected));
	}

	{
		Interleaving interleaving =
		    computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaExponentialSearch, DeltaGoodMapAlgorithm::Sweepline, restrictions);
		CHECK(interleaving.getDelta() == Approx(expected));
	}
}
