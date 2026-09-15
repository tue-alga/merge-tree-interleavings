#pragma once

#include <functional>
#include <iostream>
#include <optional>

#include "interleaving.h"
#include "mergetree.h"

using RestrictionMatrix = std::vector<std::vector<double>>;

// TODO debug
template <typename T> std::ostream& operator<<(std::ostream& out, const std::vector<T>& v) {
	out << "(";
	bool first = true;
	if (!v.empty()) {
		for (const T& t : v) {
			if (!first) {
				out << ", ";
			}
			first = false;
			out << t;
		}
	}
	out << ")";
	return out;
}

/// Computes a finite set of candidates for the interleaving distance between
/// the two given trees. See [Touli and Wang, 2022], Lemma 8.
///
/// The candidate set is returned as a list in ascending order with no
/// duplicates. 0.0 is always a candidate and hence is always the first element
/// of the list. Although the (restricted) interleaving distance can be infinite
/// (if the restrictions forbid all interleavings, for example), ∞ is not
/// returned in the list; it is instead handled separately in \ref
/// computeInterleavingDistance.
std::vector<double> interleavingDistanceCandidates(const MergeTree& T1, const MergeTree& T2,
                                                   const RestrictionMatrix& restrictions);

RestrictionMatrix createEmptyRestrictionMatrix(const MergeTree& sourceTree,
                                               const MergeTree& targetTree);

/// Computes the levels for each augmented tree. The levels are all the heights
/// of the vertices of the tree itself, plus the shifted heights of the vertices
/// of the other tree. Returns a pair consisting of the `sourceLevels` (the
/// levels for the augmented source tree) and the `targetLevels` (the levels for
/// the augmented target tree).
///
/// \note We're computing the `sourceLevels` and `targetLevels` independently,
/// even though the `targetLevels` are simply the `sourceLevels` shifted by δ.
/// However, we want to make sure that the vertex heights of the target tree are
/// included exactly, which (due to floating-point rounding errors) cannot
/// necessarily be guaranteed if we compute `targetLevels` by adding δ to each
/// of the `sourceLevels`.
std::pair<std::vector<double>, std::vector<double>>
computeLevels(const MergeTree& sourceTree, const MergeTree& targetTree, double delta);

// Utility function to merge maps
inline void insertInto(std::map<int, int>& lhs, const std::map<int, int>& rhs) {
	lhs.insert(rhs.begin(), rhs.end());
}

/// Determines if a δ-good map from `sourceTree` to `targetTree` exists, i.e.,
/// if the interleaving distance between `sourceTree` and `targetTree` is
/// upper-bounded by δ. This implements the dynamic program proposed by [Touli
/// and Wang, 2022], §4.1. If so, this returns the interleaving.
std::optional<Interleaving> computeDeltaGoodMapSlow(const std::shared_ptr<MergeTree>& sourceTree,
                                                    const std::shared_ptr<MergeTree>& targetTree,
                                                    double delta,
                                                    const RestrictionMatrix& restrictions,
                                                    std::function<void(std::pair<double, double>)> onMovedSweepline);

std::optional<Interleaving> computeDeltaGoodMapFast(const std::shared_ptr<MergeTree>& sourceTree,
                                                    const std::shared_ptr<MergeTree>& targetTree,
                                                    double delta,
                                                    const RestrictionMatrix& restrictions,
                                                    std::function<void(std::pair<double, double>)> onMovedSweepline);

std::optional<Interleaving> computeDeltaGoodMapFaster(const std::shared_ptr<MergeTree>& sourceTree,
                                                      const std::shared_ptr<MergeTree>& targetTree,
                                                      double delta,
                                                      const RestrictionMatrix& restrictions,
                                                      std::function<void(std::pair<double, double>)> onMovedSweepline);

enum class SearchAlgorithm {
	DeltaLinearSearch,
	DeltaExponentialSearch
};

enum class DeltaGoodMapAlgorithm {
	DP,
	FeasiblePairsFromPrevious,
	Sweepline
};

/// Computes the interleaving distance between the two given trees.
///
/// This is an implementation of Touli and Wang's FPT algorithm for exact
/// interleaving distance (see https://doi.org/10.20382/jocg.v13i1a4). This
/// function returns a distance of infinity if the restriction does not admit
/// any interleaving.
Interleaving computeInterleavingDistance(const std::shared_ptr<MergeTree>& sourceTree,
                                         const std::shared_ptr<MergeTree>& targetTree,
                                         SearchAlgorithm searchAlgorithm,
                                         DeltaGoodMapAlgorithm deltaGoodMapAlgorithm,
                                         RestrictionMatrix restrictions = {},
                                         std::function<void(std::pair<double, double>)> onMovedSweepline = nullptr,
                                         std::function<void(double)> onStartedDelta = nullptr,
                                         std::function<void(double)> onEndedDelta = nullptr);
