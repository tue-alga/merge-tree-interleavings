#include "interleavingdistance.h"

#include "augmentedtree.h"
#include "columntree.h"
#include "partitioner.h"

#include <algorithm>
#include <bitset>
#include <cmath>
#include <iostream>
#include <iterator>
#include <limits>
#include <queue>
#include <set>

#define DEBUG_INTERLEAVING false
#define BITSET_SIZE 128

// Utility function to merge maps
inline void insertInto(std::map<int, int>& lhs, const std::map<int, int>& rhs) {
	lhs.insert(rhs.begin(), rhs.end());
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
                                                   const RestrictionMatrix& restrictions) {
	std::vector<double> candidates = {0.0};
	std::vector<double> leafHeightsT1 = T1.getLeafHeights();
	std::vector<double> leafHeightsT2 = T2.getLeafHeights();
	std::vector<double> mergeHeightsT1 = T1.getMergeHeights();
	std::vector<double> mergeHeightsT2 = T2.getMergeHeights();

	// [Touli and Wang, 2022] describe the set of candidates as the union of the
	// following:
	//   * Π_1: all height differences between nodes in T1 and T2.
	//   * Π_2: all candidate heights belonging to zigzags from T1 to T2 to T1
	//     (= 1/2 * all height differences between nodes in T1).
	//   * Π_3: all candidate heights belonging to zigzags from T2 to T1 to T2
	//     (= 1/2 * all height differences between nodes in T2).
	//
	// In this implementation we use a smaller candidate set, which is the union
	// of the following four sets:

	//   * Π_1: all height differences between leaves in T1 and T2.
	for (int i = 0; i < leafHeightsT1.size(); i++) {
		for (int j = 0; j < leafHeightsT2.size(); j++) {
			if (leafHeightsT1[i] < restrictions[i][j]) {
				candidates.push_back(restrictions[i][j] - leafHeightsT1[i]);
			}
			if (leafHeightsT2[j] < leafHeightsT1[i]) {
				candidates.push_back(leafHeightsT1[i] - leafHeightsT2[j]);
			}
		}
	}

	//   * Π_2: all height differences between merge nodes in T1 and T2.
	for (int i = 0; i < mergeHeightsT1.size(); i++) {
		for (int j = 0; j < mergeHeightsT2.size(); j++) {
			candidates.push_back(std::abs(mergeHeightsT1[i] - mergeHeightsT2[j]));
		}
	}

	//   * Π_3: 1/2 * all height differences between each leaf in T1 and its
	//     ancestors.
	for (int i = 0; i < T1.size(); i++) {
		if (T1.isLeaf(i)) {
			int nodeIndex = i;
			while (T1.parentOf(nodeIndex)) {
				nodeIndex = *T1.parentOf(nodeIndex);
				candidates.push_back(std::abs(T1.heightOf(i) - T1.heightOf(nodeIndex)) / 2.0);
			}
		}
	}

	//   * Π_4: 1/2 * all height differences between each leaf in T2 and its
	//     ancestors.
	for (int i = 0; i < T2.size(); i++) {
		if (T2.isLeaf(i)) {
			int nodeIndex = i;
			while (T2.parentOf(nodeIndex)) {
				nodeIndex = *T2.parentOf(nodeIndex);
				candidates.push_back(std::abs(T2.heightOf(i) - T2.heightOf(nodeIndex)) / 2.0);
			}
		}
	}

	// Remove ∞ values (as ∞ doesn't need to be checked specifically: we report
	// the interleaving distance as ∞ if there is a δ-good map for none of the
	// candidates δ) and NaN values (which can arise if there are merge nodes at
	// ∞ in both trees, due to computing ∞ - ∞).
	candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
	                                [](double d) -> bool {
		                                return d == std::numeric_limits<double>::infinity() ||
		                                       std::isnan(d);
	                                }),
	                 candidates.end());

	// Sort the result and erase duplicates.
	std::sort(candidates.begin(), candidates.end());
	candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
	return candidates;
}

RestrictionMatrix createEmptyRestrictionMatrix(const MergeTree& sourceTree,
                                               const MergeTree& targetTree) {
	RestrictionMatrix result;
	for (int i = 0; i < sourceTree.leafCount(); i++) {
		result.push_back({});
		for (int j = 0; j < targetTree.size(); j++) {
			result.back().push_back(targetTree.heightOf(j));
		}
	}
	return result;
}

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
computeLevels(const MergeTree& sourceTree, const MergeTree& targetTree, double delta) {

	// Find the vertex heights and shifted vertex heights of both trees.
	std::vector<double> sourceHeights = sourceTree.getHeights();
	std::vector<double> targetHeights = targetTree.getHeights();
	std::vector<double> shiftedSourceHeights = sourceTree.getHeights(delta);
	std::vector<double> shiftedTargetHeights = targetTree.getHeights(-delta);

	// Merge the vertex heights of each tree with the shifted vertex heights of
	// the other tree to obtain the two lists of levels.
	std::vector<double> sourceLevels, targetLevels;
	std::merge(sourceHeights.begin(), sourceHeights.end(), shiftedTargetHeights.begin(),
	           shiftedTargetHeights.end(), std::back_inserter(sourceLevels), std::less{});
	std::merge(targetHeights.begin(), targetHeights.end(), shiftedSourceHeights.begin(),
	           shiftedSourceHeights.end(), std::back_inserter(targetLevels), std::less{});

	// Remove levels that are identical in both the sourceLevels and
	// targetLevels lists.
	std::vector<double> deduplicatedSourceLevels, deduplicatedTargetLevels;
	assert(sourceLevels.size() == targetLevels.size());
	for (int i = 0; i < sourceLevels.size(); i++) {
		if (i == 0 || sourceLevels[i] != sourceLevels[i - 1] ||
		    targetLevels[i] != targetLevels[i - 1]) {
			deduplicatedSourceLevels.push_back(sourceLevels[i]);
			deduplicatedTargetLevels.push_back(targetLevels[i]);
		}
	}

	return {deduplicatedSourceLevels, deduplicatedTargetLevels};
}

/// Determines if a δ-good map from `sourceTree` to `targetTree` exists, i.e.,
/// if the interleaving distance between `sourceTree` and `targetTree` is
/// upper-bounded by δ. This implements the dynamic program proposed by [Touli
/// and Wang, 2022], §4.1. If so, this returns the interleaving.
std::optional<Interleaving> computeDecisionDP(const std::shared_ptr<MergeTree>& sourceTree,
                                                    const std::shared_ptr<MergeTree>& targetTree,
                                                    double delta,
                                                    const RestrictionMatrix& restrictions,
                                                    std::function<void(std::pair<double, double>)> onMovedSweepline) {
	std::cerr << "    trying \033[1;1mδ = " << delta << "\033[1;0m..." << std::flush;

	// Construct the augmented trees.
	auto [sourceLevels, targetLevels] = computeLevels(*sourceTree, *targetTree, delta);
	AugmentedTree sourceAugmented(*sourceTree, sourceLevels, restrictions);
	AugmentedTree targetAugmented(*targetTree, targetLevels, restrictions);

	// We use a DP to compute feasible pairs for each level of the augmented
	// trees, starting at the bottommost level and going upwards towards the
	// trees' roots. To conserve memory, we store just the set of feasible pairs
	// we're currently computing for the current level, and the set of feasible
	// pairs we just computed for the previous level.
	std::set<AugmentedTree::ValidPair> feasiblePairs, childrenFeasiblePairs;

	for (int level = 0; level < sourceLevels.size(); level++) {
		std::cerr << "\033[1K\r"
		          << "    trying \033[1;1mδ = " << delta << "\033[1;0m... level " << level << "/"
		          << sourceLevels.size() << ", height " << sourceLevels[level] << std::flush;

		childrenFeasiblePairs = feasiblePairs;
		feasiblePairs.clear();

		// A pair (∅, w) is feasible if the subtree of w has height at most 2δ
		// minus the height difference between the two levels. (This replaces
		// case (F-2): instead of special-casing this, we simply put all
		// corresponding “S = ∅” pairs into the feasible pairs.)
		if (level < targetAugmented.getLevelCount() - 1) {
			for (int targetNode = 0; targetNode < targetAugmented.get(level).size(); targetNode++) {
				if (targetAugmented.get(level, targetNode).m_depth <=
				    2 * delta - std::abs(targetAugmented.getLevelHeight(level + 1) -
				                         targetAugmented.getLevelHeight(level))) {
					feasiblePairs.insert({{}, targetNode});
				}
			}
		}

		// For each valid pair (S, w), let {w_1, ..., w_k} be the children of w.
		// Add (S, w) as a feasible pair if the children of S can be partitioned
		// into k possibly empty pieces {S_1, ..., S_k} in such a way that (S_1,
		// w_1), ..., (S_k, w_k) are all feasible pairs on the previous level.
		for (AugmentedTree::ValidPair& validPair :
		     sourceAugmented.collectValidPairs(targetAugmented, level, delta)) {
			std::vector<int> sourceChildren =
			    sourceAugmented.getChildren(level, validPair.m_sourceNodes);
			std::vector<int> targetChildren =
			    targetAugmented.getChildren(level, {validPair.m_targetNode});

			if (targetChildren.empty()) {
				if (sourceChildren.empty()) {
					feasiblePairs.insert(validPair);
				}
			} else {
				// We partition the sourceChildren in as many sets as there are
				// targetChildren. For now we assume there are at most 2
				// targetChildren. Therefore we get two cases: 1 or 2 children.
				if (targetChildren.size() == 1) {
					// Singleton case
					if (childrenFeasiblePairs.contains({sourceChildren, targetChildren[0]})) {
						// This really should be done better. (Emil's words, not
						// mine.)
						if (!sourceChildren.empty()) {
							insertInto(
							    validPair.m_map,
							    childrenFeasiblePairs.find({sourceChildren, targetChildren[0]})->m_map);
						}
						feasiblePairs.insert(validPair);
					}

				} else {
					assert(targetChildren.size() == 2);
					for (const auto& s : Partition(sourceChildren)) {
						if (childrenFeasiblePairs.contains({s.first, targetChildren[0]}) &&
						    childrenFeasiblePairs.contains({s.second, targetChildren[1]})) {
							if (!s.first.empty()) {
								insertInto(
								    validPair.m_map,
								    childrenFeasiblePairs.find({s.first, targetChildren[0]})->m_map);
							}
							if (!s.second.empty()) {
								insertInto(
								    validPair.m_map,
								    childrenFeasiblePairs.find({s.second, targetChildren[1]})->m_map);
							}
							feasiblePairs.insert(validPair);
							break;
						}
					}
				}
			}
		}
	}

	std::cerr << "\033[1K\r"
	          << "    trying \033[1;1mδ = " << delta << "\033[1;0m";

	// A δ-good map exists if in the root-most layer, the one node in T1 forms a
	// feasible valid pair with the one node in T2.
	auto result = feasiblePairs.find(AugmentedTree::ValidPair({0}, 0));
	if (result != feasiblePairs.end()) {
		std::cerr << " → \033[1;32mtrue\033[1;0m" << std::endl;
		return Interleaving{sourceTree, targetTree, delta, result->m_map};
	} else {
		std::cerr << " → \033[1;31mfalse\033[1;0m" << std::endl;
		return std::nullopt;
	}
}

bool isDisjoint(const std::vector<int>& a, const std::vector<int>& b) {
	auto i = a.begin();
	auto j = b.begin();
	while (i != a.end() && j != b.end()) {
		if (*i == *j) {
			return false;
		} else if (*i < *j) {
			i++;
		} else {
			j++;
		}
	}
	return true;
}

// std::optional<Interleaving> computeDeltaGoodMapFast(const std::shared_ptr<MergeTree>& sourceTree,
//                                                     const std::shared_ptr<MergeTree>& targetTree,
//                                                     double delta,
//                                                     const RestrictionMatrix& restrictions,
//                                                     std::function<void(std::pair<double, double>)> onMovedSweepline) {
// 	std::cerr << "    trying \033[1;1mδ = " << delta << "\033[1;0m..." << std::flush;

// 	// Construct the augmented trees.
// 	auto [sourceLevels, targetLevels] = computeLevels(*sourceTree, *targetTree, delta);
// 	AugmentedTree sourceAugmented(*sourceTree, sourceLevels, restrictions);
// 	AugmentedTree targetAugmented(*targetTree, targetLevels, restrictions);

// 	using SourceNodeSet = std::vector<int>;

// 	// We use a DP to compute feasible pairs for each level of the augmented
// 	// trees, starting at the bottommost level and going upwards towards the
// 	// trees' roots. To conserve memory, we store just the set of feasible pairs
// 	// we're currently computing for the current level, and the set of feasible
// 	// pairs we just computed for the previous level.
// 	//
// 	// feasiblePairs[w] = for target node w, all sets S for which (S, w) is a
// 	//                    feasible pair
// 	//
// 	// Each set S is represented as an ordered vector of indices.
// 	std::vector<std::set<SourceNodeSet>> feasiblePairs, childrenFeasiblePairs;

// 	for (int level = 0; level < sourceLevels.size(); level++) {
// 		std::cerr << "\033[1K\r"
// 		          << "    trying \033[1;1mδ = " << delta << "\033[1;0m... level " << level << "/"
// 		          << sourceLevels.size() << ", height " << sourceLevels[level] << std::flush;

// 		childrenFeasiblePairs = feasiblePairs;
// 		feasiblePairs = std::vector<std::set<SourceNodeSet>>(targetAugmented.get(level).size(),
// 		                                                     std::set<SourceNodeSet>{});

// 		for (int targetIndex = 0; targetIndex < targetAugmented.get(level).size(); targetIndex++) {
// 			int targetChildCount = targetAugmented.get(level, targetIndex).m_children.size();

// 			std::vector<SourceNodeSet> feasiblePairsToAdd;

// 			if (targetChildCount == 0) {
// 				// Skip: we don't need to add any feasible pairs.
// 				feasiblePairsToAdd.push_back({}); // TODO ???????????????

// 			} else if (targetChildCount == 1) {
// 				// Target node has one child.
// 				int targetChildIndex = targetAugmented.get(level, targetIndex).m_children[0];
// 				for (const SourceNodeSet& sourceNodeSet : childrenFeasiblePairs[targetChildIndex]) {
// 					if (sourceNodeSet.empty()) {
// 						// Source node set is empty. Need to check if the
// 						// unmapped subtree in the target tree is not too high.
// 						if (targetAugmented.get(level, targetIndex).m_depth > 2 * delta) {
// 							continue;
// 						}
// 					}

// 					SourceNodeSet parents = sourceAugmented.parentsOf(level - 1, sourceNodeSet);

// 					// Check if the children of the parentUnion are actually a
// 					// subset of sourceNodeSet. If not, there is some extra
// 					// subtree hanging off that we aren't mapping.
// 					const std::vector<int> parentChildren =
// 					    sourceAugmented.getChildren(level, parents);
// 					if (parentChildren.size() != sourceNodeSet.size()) {
// 						// It suffices to just check the sizes, as we
// 						// already know that the parentChildren can only be
// 						// a superset of source1NodeSet \union
// 						// source2NodeSet.
// 						continue;
// 					}

// 					feasiblePairsToAdd.push_back(parents);
// 				}

// 			} else {
// 				// Target node is a split node.
// 				assert(targetChildCount == 2);
// 				int targetChild1Index = targetAugmented.get(level, targetIndex).m_children[0];
// 				int targetChild2Index = targetAugmented.get(level, targetIndex).m_children[1];

// 				for (const SourceNodeSet& source1NodeSet : childrenFeasiblePairs[targetChild1Index]) {
// 					if (source1NodeSet.empty()) {
// 						if (targetAugmented.get(level - 1, targetChild1Index).m_depth >
// 						    2 * delta - std::abs(targetAugmented.getLevelHeight(level) -
// 						                         targetAugmented.getLevelHeight(level - 1))) {
// 							continue;
// 						}
// 					}

// 					// TODO It would be more efficient to filter the lists of
// 					// childrenFeasiblePairs beforehand, so that empty feasible
// 					// pairs that fail the unmapped-height-criterion are already
// 					// excluded. We could even avoid putting them into the
// 					// feasiblePairs lists altogether, so we never see them in
// 					// the first place! This is a bit confusing though, so for
// 					// now we simply do it like this.
// 					for (const SourceNodeSet& source2NodeSet :
// 					     childrenFeasiblePairs[targetChild2Index]) {
// 						if (source2NodeSet.empty()) {
// 							if (targetAugmented.get(level - 1, targetChild2Index).m_depth >
// 							    2 * delta - std::abs(targetAugmented.getLevelHeight(level) -
// 							                         targetAugmented.getLevelHeight(level - 1))) {
// 								continue;
// 							}
// 						}

// 						if (!isDisjoint(source1NodeSet, source2NodeSet)) {
// 							continue;
// 						}

// 						SourceNodeSet parents1 = sourceAugmented.parentsOf(level - 1, source1NodeSet);
// 						SourceNodeSet parents2 = sourceAugmented.parentsOf(level - 1, source2NodeSet);
// 						SourceNodeSet parentUnion;
// 						std::set_union(parents1.begin(), parents1.end(), parents2.begin(),
// 						               parents2.end(), std::back_inserter(parentUnion));

// 						// Check if the children of the parentUnion are actually
// 						// a subset of source1NodeSet \union source2Node set. If
// 						// not, there is some extra subtree hanging off that we
// 						// aren't mapping.
// 						const std::vector<int> parentChildren =
// 						    sourceAugmented.getChildren(level, parentUnion);
// 						if (parentChildren.size() != source1NodeSet.size() + source2NodeSet.size()) {
// 							// It suffices to just check the sizes, as we
// 							// already know that the parentChildren can only be
// 							// a superset of source1NodeSet \union
// 							// source2NodeSet.
// 							continue;
// 						}

// 						if (!parentUnion.empty() &&
// 						    sourceAugmented.distanceOfLCA(level, parentUnion.front(),
// 						                                  parentUnion.back()) > 2 * delta) {
// 							continue;
// 						}

// 						feasiblePairsToAdd.push_back(parentUnion);
// 					}
// 				}
// 			}

// 			for (int i = 0; i < feasiblePairsToAdd.size(); i++) {
// 				const SourceNodeSet& sourceNodeSet = feasiblePairsToAdd[i];
// 				feasiblePairs[targetIndex].insert(sourceNodeSet);
// 			}

// 			// TODO leaves
// 			std::vector<int> sourceLeaves;
// 			for (int sourceIndex = 0; sourceIndex < sourceAugmented.get(level).size(); sourceIndex++) {
// 				if (sourceAugmented.get(level, sourceIndex).m_children.empty()) {
// 					sourceLeaves.push_back(sourceIndex);
// 				}
// 			}
// 			assert(sourceLeaves.size() <= 1); // TODO this is probably fine in practice, but ...
// 			if (!sourceLeaves.empty()) {
// 				int sourceLeafIndex = sourceLeaves[0];
// 				// During the for loop we add elements to feasiblePairs, but we
// 				// want to have i run only up until the original range of
// 				// feasiblePairs.
// 				for (int i = 0; i < feasiblePairsToAdd.size(); i++) {
// 					const SourceNodeSet& sourceNodeSet = feasiblePairsToAdd[i];

// 					int minIndex = sourceNodeSet.empty()
// 					                   ? sourceLeafIndex
// 					                   : std::min(sourceNodeSet.front(), sourceLeafIndex);
// 					int maxIndex = sourceNodeSet.empty()
// 					                   ? sourceLeafIndex
// 					                   : std::max(sourceNodeSet.back(), sourceLeafIndex);

// 					if (sourceAugmented.distanceOfLCA(level, minIndex, maxIndex) > 2 * delta) {
// 						continue;
// 					}

// 					std::vector<int> extendedSourceNodeSet = sourceNodeSet;
// 					extendedSourceNodeSet.push_back(sourceLeafIndex);
// 					std::sort(extendedSourceNodeSet.begin(),
// 					          extendedSourceNodeSet.end()); // TODO don't sort the entire thing, just insert in the right place
// 					feasiblePairs[targetIndex].insert(extendedSourceNodeSet);
// 				}
// 			}
// 		}

// 		for (int i = 0; i < feasiblePairs.size(); i++) {
// 			const std::set<SourceNodeSet>& set = feasiblePairs[i];
// 			if (set.size() == 0) {
// 				std::cerr << "\033[1K\r"
// 				          << "    trying \033[1;1mδ = " << delta << "\033[1;0m";
// 				std::cerr << " → \033[1;31mfalse\033[1;0m" << std::endl;
// 				return std::nullopt;
// 			}
// 		}
// 	}

// 	std::cerr << "\033[1K\r"
// 	          << "    trying \033[1;1mδ = " << delta << "\033[1;0m";

// 	// A δ-good map exists if in the root-most layer, the one node in T1 forms a
// 	// feasible valid pair with the one node in T2.
// 	if (!feasiblePairs[0].empty()) {
// 		std::cerr << " → \033[1;32mtrue\033[1;0m" << std::endl;
// 		return Interleaving{sourceTree, targetTree, delta, {}};
// 	} else {
// 		std::cerr << " → \033[1;31mfalse\033[1;0m" << std::endl;
// 		return std::nullopt;
// 	}
// }

/// A sweep event in the δ-good map sweepline algorithm.
struct SweepEvent {
		/// Possible sweep event types.
		enum class Type {
			/// The sweepline hits a leaf in the target tree, or equivalently,
			/// it hits the leaf height of a column.
			TargetLeaf,
			/// The sweepline hits an internal vertex in the target tree, or
			/// equivalently, it hits the merge height of a column.
			TargetInternal,
			/// The sweepline hits a leaf in the source tree, or equivalently,
			/// it hits the leaf height of a column.
			SourceLeaf,
			/// The sweepline hits an internal vertex in the source tree, or
			/// equivalently, it hits the merge height of a column.
			SourceInternal
		};

		/// The height of the sweepline in the source tree at the time of this
		/// event.
		double m_sourceHeight;
		/// The height of the sweepline in the target tree at the time of this
		/// event.
		double m_targetHeight;

		/// The type of this event.
		Type m_type;
		/// The index of the column in the (source or target) column tree in
		/// which a leaf or internal vertex is being hit.
		int m_columnIndex;

		/// The index of the hit vertex in the merge tree.
		int m_mergeTreeIndex;

		SweepEvent(double sourceHeight, double targetHeight, Type type, int columnIndex,
		           int mergeTreeIndex)
		    : m_sourceHeight(sourceHeight), m_targetHeight(targetHeight), m_type(type),
		      m_columnIndex(columnIndex), m_mergeTreeIndex(mergeTreeIndex) {}

		bool operator<(const SweepEvent& other) const {
			if (m_sourceHeight != other.m_sourceHeight) {
				return m_sourceHeight > other.m_sourceHeight;
			}
			if (m_targetHeight != other.m_targetHeight) {
				return m_targetHeight > other.m_targetHeight;
			}
			if (m_type != other.m_type) {
				return m_type > other.m_type;
			}
			return m_mergeTreeIndex > other.m_mergeTreeIndex;
		}
};

using FeasibleSet = std::bitset<BITSET_SIZE>;

struct FeasibleSetCompare {
	bool operator()(const FeasibleSet& lhs, const FeasibleSet& rhs) const {
		// [ws] TODO This is absurdly slow because we're checking per bit instead of
		// per word. Why doesn't std::bitset define operator< instead? Who knows.
		for (int i = 0; i < BITSET_SIZE - 1; i++) {
			if (lhs[i] && !rhs[i]) {
				return false;
			}
			if (!lhs[i] && rhs[i]) {
				return true;
			}
		}
		return false;
	}
};

int indexOfFirstOne(const FeasibleSet& set) {
	for (int i = 0; i < BITSET_SIZE - 1; i++) {
		if (set[i]) {
			return i;
		}
	}
	return -1;
}

using FeasibleSetSet = std::set<FeasibleSet, FeasibleSetCompare>;

void printFeasibleSets(const std::vector<std::optional<FeasibleSetSet>>& feasibleSets) {
	for (int i = 0; i < feasibleSets.size(); i++) {
		if (!feasibleSets[i]) {
			std::cerr << "F[" << i << "] = nil" << std::endl;
		} else {
			std::cerr << "F[" << i << "] = {";
			bool first = true;
			for (const FeasibleSet& set : *feasibleSets[i]) {
				if (!first) {
					std::cerr << ", ";
				}
				first = false;
				std::cerr << "(";
				bool first2 = true;
				for (int i = 0; i < BITSET_SIZE; i++) {
					if (set[i]) {
						if (!first2) {
							std::cerr << ", ";
						}
						first2 = false;
						std::cerr << i;
					}
				}
				std::cerr << ")";
			}
			std::cerr << "}" << std::endl;
		}
	}
}

std::optional<Interleaving> computeDecisionSweepline(const std::shared_ptr<MergeTree>& sourceTree,
                                                      const std::shared_ptr<MergeTree>& targetTree,
                                                      double delta,
                                                      const RestrictionMatrix& restrictions,
                                                      std::function<void(std::pair<double, double>)> onMovedSweepline) {
	std::cerr << "    trying \033[1;1mδ = " << delta << "\033[1;0m..." << std::flush;

	// Build column trees of the input merge trees.
	ColumnTree sourceColumns(*sourceTree);
	ColumnTree targetColumns(*targetTree);

	// Set up a queue with all events.
	std::priority_queue<SweepEvent> queue;
	for (int i = 0; i < sourceColumns.columnCount(); i++) {
		const ColumnTree::Column& c = sourceColumns.getColumn(i);
		queue.push({c.m_leafHeight, c.m_leafHeight + delta, SweepEvent::Type::SourceLeaf, i,
		            c.m_leafIndex});
		if (c.m_mergeHeight) {
			queue.push({*c.m_mergeHeight, *c.m_mergeHeight + delta,
			            SweepEvent::Type::SourceInternal, i, *c.m_mergeIndex});
		}
	}
	for (int j = 0; j < targetColumns.columnCount(); j++) {
		const ColumnTree::Column& c = targetColumns.getColumn(j);
		queue.push({c.m_leafHeight - delta, c.m_leafHeight, SweepEvent::Type::TargetLeaf, j,
		            c.m_leafIndex});
		if (c.m_mergeHeight) {
			queue.push({*c.m_mergeHeight - delta, *c.m_mergeHeight,
			            SweepEvent::Type::TargetInternal, j, *c.m_mergeIndex});
		}
	}

	// During the sweep, we maintain for each active target column `j`, the set
	// of feasible sets of `j`, that is, the source column sets `I` such that
	// `(I, j)` is a feasible pair. `feasibleSets[j]` is this set of source
	// column sets for column `j`. We store each feasible set as a sorted vector
	// of column indices.
	std::vector<std::optional<FeasibleSetSet>> feasibleSets(targetTree->leafCount(),
		std::nullopt);

	int eventCount = 0;
	int totalEventCount = queue.size();

	// Perform the sweep, handling events one by one.
	while (!queue.empty()) {
		SweepEvent event = queue.top();
		queue.pop();

		if (DEBUG_INTERLEAVING) {
			std::cerr << std::endl;
		}

		if (onMovedSweepline) {
			onMovedSweepline({event.m_sourceHeight, event.m_targetHeight});
		}
		std::cerr << "\033[1K\r"
		          << "    trying \033[1;1mδ = " << delta << "\033[1;0m... event " << ++eventCount
		          << "/" << totalEventCount << ", height " << event.m_sourceHeight << ": "
		          << std::flush;

		switch (event.m_type) {

		case SweepEvent::Type::TargetLeaf:
			std::cerr << "(a) target leaf event" << std::flush;
			assert(!feasibleSets[event.m_columnIndex]);
			feasibleSets[event.m_columnIndex] = FeasibleSetSet();
			break;

		case SweepEvent::Type::TargetInternal: {
			std::cerr << "(b) target internal vertex event" << std::flush;
			const ColumnTree::Column& c = targetColumns.getColumn(event.m_columnIndex);
			assert(c.m_parent);
			assert(feasibleSets[*c.m_parent]);
			assert(feasibleSets[c.m_index]);

			// We will build a new set of feasible sets by combining the
			// feasible sets of the parent column and the feasible sets of the
			// child column.
			FeasibleSetSet mergedFeasibleSets;

			// Look at each 2δ-ancestor of the sweepline in the source tree.
			std::vector<int> ancestors =
			    sourceColumns.activeColumnsAt(event.m_sourceHeight + 2 * delta);
			ancestors.push_back(sourceColumns.columnCount());

			for (int i = 0; i < ancestors.size() - 1; i++) {
				// For an 2δ-ancestor `a`, within the set of feasible sets of
				// the parent column, we aim to find all feasible sets with `a`
				// as their 2δ-ancestor. Because the feasible sets are stored as
				// sorted bitsets in an `std::set`, they are sorted (in reverse
				// order) by the lowest column they contain. Hence, the feasible
				// sets with `a` as their 2δ-ancestor form an interval within
				// the `std::set`. We find the begin and end iterators of this
				// interval.
				FeasibleSetSet::iterator parentFeasibleSetBegin =
				    (*feasibleSets[*c.m_parent]).lower_bound(FeasibleSet().set(ancestors[i + 1] - 1));
				FeasibleSetSet::iterator parentFeasibleSetEnd =
				    ancestors[i] == 0
				        ? (*feasibleSets[*c.m_parent]).end()
				        : (*feasibleSets[*c.m_parent]).lower_bound(FeasibleSet().set(ancestors[i] - 1));

				// We do the same for the set of feasible sets of the child
				// column.
				FeasibleSetSet::iterator childFeasibleSetBegin =
				    (*feasibleSets[c.m_index]).lower_bound(FeasibleSet().set(ancestors[i + 1] - 1));
				FeasibleSetSet::iterator childFeasibleSetEnd =
				    ancestors[i] == 0
				        ? (*feasibleSets[c.m_index]).end()
				        : (*feasibleSets[c.m_index]).lower_bound(FeasibleSet().set(ancestors[i] - 1));

				// Now we generate all (non-empty) unions of feasible sets from
				// the parent set and from the child set.
				for (auto parentFeasibleSet = parentFeasibleSetBegin;
				     parentFeasibleSet != parentFeasibleSetEnd; parentFeasibleSet++) {
					for (auto childFeasibleSet = childFeasibleSetBegin;
					     childFeasibleSet != childFeasibleSetEnd; childFeasibleSet++) {
						if ((*parentFeasibleSet & *childFeasibleSet).none()) {
							mergedFeasibleSets.insert(*parentFeasibleSet | *childFeasibleSet);
						}
					}
				}
			}

			// If the depth of the parent subtree is at most 2δ, we additionally insert all
			// feasible sets in the child subtree
			if (targetColumns.getDepth(*c.m_parent, event.m_targetHeight) <= 2 * delta) {
				for (const FeasibleSet feasibleSet : *feasibleSets[c.m_index]) {
					mergedFeasibleSets.insert(feasibleSet);
				}
			}

			// Similarly, if the depth of the child subtree is at most 2δ, we additionally insert all
			// feasible sets in the parent subtree
			if (targetColumns.getDepth(c.m_index, event.m_targetHeight) <= 2 * delta) {
				for (const FeasibleSet feasibleSet : *feasibleSets[*c.m_parent]) {
					mergedFeasibleSets.insert(feasibleSet);
				}
			}

			// Finally, overwrite the old sets of feasible sets by the newly
			// constructed one.
			feasibleSets[*c.m_parent] = mergedFeasibleSets;
			feasibleSets[c.m_index] = std::nullopt;
			break;
		}

		case SweepEvent::Type::SourceLeaf:
			std::cerr << "(c) source leaf event" << std::flush;

			// Iterate over all active target columns j.
			for (int j = 0; j < targetColumns.columnCount(); j++) {
				if (!feasibleSets[j]) {
					continue;
				}

				bool allowedByRestriction =
				    restrictions[event.m_mergeTreeIndex][targetColumns.getColumn(j).m_leafIndex] <=
				    event.m_targetHeight;
				if (!allowedByRestriction) {
					continue;
				}

				// For all existing feasible sets that have the same 2δ-ancestor
				// as the new leaf i, we insert a copy of the feasible set
				// augmented with i.
				std::vector<FeasibleSet> setsToAdd;
				for (const FeasibleSet& feasibleSet : *feasibleSets[j]) {
					if (sourceColumns.ancestorAtHeight(indexOfFirstOne(feasibleSet),
														event.m_sourceHeight + 2 * delta) ==
						sourceColumns.ancestorAtHeight(event.m_columnIndex,
														event.m_sourceHeight + 2 * delta)) {
						setsToAdd.push_back(feasibleSet | FeasibleSet().set(event.m_columnIndex));
					}
				}

				// If the depth of j is at most 2δ, we additionally insert the
				// feasible set {i}.
				if (targetColumns.getDepth(j, event.m_targetHeight) <= 2 * delta) {
					setsToAdd.push_back(FeasibleSet().set(event.m_columnIndex));
				}

				for (const FeasibleSet& setToAdd : setsToAdd) {
					(*feasibleSets[j]).insert(setToAdd);
				}
			}

			break;

		case SweepEvent::Type::SourceInternal:
			std::cerr << "(d) source internal vertex event" << std::flush;
			const ColumnTree::Column& c = sourceColumns.getColumn(event.m_columnIndex);
			assert(c.m_parent);

			int column1 = *c.m_parent;
			int column2 = event.m_columnIndex;

			// Iterate over all active target columns j.
			for (int j = 0; j < targetColumns.columnCount(); j++) {
				if (!feasibleSets[j]) {
					continue;
				}

				FeasibleSetSet updatedFeasibleSets;
				for (const FeasibleSet& feasibleSet : *feasibleSets[j]) {
					bool column1Present = feasibleSet[column1];
					bool column2Present = feasibleSet[column2];

					if (!column1Present && !column2Present) {
						updatedFeasibleSets.insert(feasibleSet);
					} else if (column1Present && column2Present) {
						FeasibleSet augmentedSet = feasibleSet;
						augmentedSet.reset(column2);
						updatedFeasibleSets.insert(augmentedSet);
					}
				}
				feasibleSets[j] = updatedFeasibleSets;
			}

			break;
		}

		if (DEBUG_INTERLEAVING) {
			std::cerr << "\n" << std::endl;
			printFeasibleSets(feasibleSets);
		}
	}

	if (DEBUG_INTERLEAVING) {
		std::cerr << std::endl;
	}

	std::cerr << "\033[1K\r"
	          << "    trying \033[1;1mδ = " << delta << "\033[1;0m";

	// A δ-good map exists if in the root-most layer, the one node in T1 forms a
	// feasible valid pair with the one node in T2.
	if (!feasibleSets[0]->empty()) {
		std::cerr << " → \033[1;32mtrue\033[1;0m" << std::endl;
		return Interleaving{sourceTree, targetTree, delta, {}};
	} else {
		std::cerr << " → \033[1;31mfalse\033[1;0m" << std::endl;
		return std::nullopt;
	}
}

Interleaving computeInterleavingDistance(const std::shared_ptr<MergeTree>& sourceTree,
                                         const std::shared_ptr<MergeTree>& targetTree,
                                         SearchAlgorithm searchAlgorithm,
                                         DeltaGoodMapAlgorithm deltaGoodMapAlgorithm,
                                         RestrictionMatrix restrictions,
                                         std::function<void(std::pair<double, double>)> onMovedSweepline,
                                         std::function<void(double)> onStartedDelta,
                                         std::function<void(double)> onEndedDelta) {
	switch (deltaGoodMapAlgorithm) {
	case DeltaGoodMapAlgorithm::DP:
		std::cerr << "Computing interleaving distance with the Touli and Wang method..." << std::endl;
		break;
	case DeltaGoodMapAlgorithm::Sweepline:
		std::cerr << "Computing interleaving distance with the sweepline method, updating only "
		             "changed feasible pairs from the previous ones..."
		          << std::endl;
		break;
	}

	// Yuck. TODO
	if (restrictions.empty()) {
		restrictions = createEmptyRestrictionMatrix(*sourceTree, *targetTree);
	}

	// Find a set of possible candidates for the interleaving distance.
	std::vector<double> candidates =
	    interleavingDistanceCandidates(*sourceTree, *targetTree, restrictions);

	// Find the midpoints right between these candidates. (Instead of querying
	// at the candidates themselves, we'll query at these midpoints, to avoid
	// floating-point rounding errors.)
	std::vector<double> midpoints(candidates.size());
	for (int i = 0; i < candidates.size() - 1; i++) {
		midpoints[i] = (candidates[i] + candidates[i + 1]) / 2;
	}
	midpoints.back() = candidates.back() * 1.1;

	std::optional<Interleaving> interleaving;
	int foundIndex = -1;

	auto computeDeltaGoodMap = computeDecisionDP;
	switch (deltaGoodMapAlgorithm) {
	case DeltaGoodMapAlgorithm::Sweepline:
		computeDeltaGoodMap = computeDecisionSweepline;
	}

	if (searchAlgorithm == SearchAlgorithm::DeltaLinearSearch) {
		// Do linear search.
		for (int i = 0; i < candidates.size(); i++) {
			if (onStartedDelta) {
				onStartedDelta(midpoints[i]);
			}
			std::optional<Interleaving> result =
			    computeDeltaGoodMap(sourceTree, targetTree, midpoints[i], restrictions, onMovedSweepline);
			if (onEndedDelta) {
				onEndedDelta(midpoints[i]);
			}

			if (result.has_value()) {
				interleaving = result.value();
				foundIndex = i;
				break;
			}
		}

		// Didn't find anything? Then the interleaving distance is infinity.
		if (!interleaving) {
			std::cerr << "Result: \033[1;1mδ = ∞\033[1;0m" << std::endl;
			return Interleaving{sourceTree, targetTree, std::numeric_limits<double>::infinity(), {}};
		}

	} else {
		// First do an exponential search to reach an upper bound.
		int lower = 0;
		std::size_t upper = std::min<std::size_t>(2, candidates.size() - 1);
		while (true) {
			if (lower > candidates.size() - 1) {
				std::cerr << "Result: \033[1;1mδ = ∞\033[1;0m" << std::endl;
				return Interleaving{sourceTree, targetTree, std::numeric_limits<double>::infinity(), {}};
			}
			if (upper > candidates.size() - 1) {
				upper = candidates.size() - 1;
			}
			if (onStartedDelta) {
				onStartedDelta(midpoints[upper]);
			}
			std::optional<Interleaving> result =
			    computeDeltaGoodMap(sourceTree, targetTree, midpoints[upper], restrictions, onMovedSweepline);
			if (onEndedDelta) {
				onEndedDelta(midpoints[upper]);
			}
			if (result.has_value()) {
				interleaving = result.value();
				break;
			} else {
				lower = upper + 1;
				// Take as the new upper bound the last candidate that is within a
				// factor of 1.5 of the lower bound.
				upper = lower;
				do {
					upper++;
				} while (upper + 1 < candidates.size() && midpoints[upper + 1] < midpoints[lower] * 1.5);
			}
		}

		// Then find the exact value of the interleaving distance using binary
		// search.
		while (upper - lower >= 1) {
			int mid = (upper + lower) / 2;
			if (onStartedDelta) {
				onStartedDelta(midpoints[mid]);
			}
			std::optional<Interleaving> result =
			    computeDeltaGoodMap(sourceTree, targetTree, midpoints[mid], restrictions, onMovedSweepline);
			if (onEndedDelta) {
				onEndedDelta(midpoints[mid]);
			}
			if (result.has_value()) {
				upper = mid;
				interleaving = result.value();
			} else {
				lower = mid + 1;
			}
		}
		foundIndex = upper;
	}

	std::cerr << "Result: \033[1;1mδ = " << candidates[foundIndex] << "\033[1;0m" << std::endl;
	assert(interleaving.has_value());

	// Because we're checking midpoints rather than candidates, the computed
	// interleaving will still have its delta set to the midpoint. Override that
	// with the actual candidate.
	interleaving->setDelta(candidates[foundIndex]);

	return *interleaving;
}
