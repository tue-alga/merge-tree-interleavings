#pragma once

#include <map>
#include <vector>

#include "mergetree.h"

/// An augmented merge tree, as defined by Touli and Wang. This is a form of a
/// merge tree which is augmented with a set of levels, each at some fixed
/// height, sorted from the leaves towards the root. Each node in the tree lies
/// on a level, and no edges “cross over” levels. Therefore to make an augmented
/// merge tree out of an ordinary merge tree, any edge in a merge tree that
/// crosses over intermediate levels has to be subdivided using helper nodes on
/// all these intermediate levels.
class AugmentedTree {

	using RestrictionMatrix = std::vector<std::vector<double>>;

	public:
		/// Given a set of level heights, computes the augmented tree from the
		/// given merge tree.
		AugmentedTree(const MergeTree& tree, const std::vector<double>& heights,
		              const RestrictionMatrix& restrictions);

		/// A node in the augmented merge tree. This is a simplified version of
		/// \ref MergeTree::Node.
		class Node {
			public:
				/// The level this node is in.
				int m_level;
				/// The index of this node, within level `m_level`.
				int m_index;
				/// Indices of the children of this node, within level `m_level - 1`.
				std::vector<int> m_children;
				/// Index of the parent of this node, if any, within level `m_level + 1`.
				std::optional<int> m_parent;

				/// The index of the \ref MergeTree::Node this node corresponds
				/// to. Nodes in the augmented merge tree that are not in the
				/// original merge tree (i.e., degree-1 nodes that were
				/// inserted) are annotated with the index of the first \ref
				/// MergeTree::Node encountered when walking towards the leaves
				/// (away from the root).
				int m_mergeTreeIndex;
				/// `true` if this node corresponds to a merge tree node;
				/// `false` if this node lies somewhere on a merge tree edge.
				bool m_isMergeTreeNode;

				/// The height of the subtree rooted at this node.
				double m_depth = 0.0;
		};

		/// Returns the `index`-th node on the `level`-th level.
		const Node& get(int level, int index) const;

		/// Returns the list of nodes on the `level`-th level.
		const std::vector<Node>& get(int level) const;

		/// Returns the height of the `level`-th level.
		double getLevelHeight(int level) const;

		/// Returns the number of levels in this augmented tree.
		int getLevelCount() const;

		/// Returns the indices of all children (on `level - 1`) of the given
		/// nodes (on `level`).
		std::vector<int> getChildren(int level, const std::vector<int>& indices);

		/// Finds maximal valid sets. A valid set is a set of nodes of the tree
		/// at level `level` that are connected within a distance of 2δ. This
		/// method returns only maximal valid sets (that is, those which cannot
		/// be extended with more nodes).
		///
		/// Returns a map from indices (on the last level at most 2δ closer to
		/// the root than the current level) to indices on `level`.
		std::map<int, std::vector<int>> collectComponents(int level, double delta) const;

		/// A valid pair is a set `S` of nodes in the source tree (on some
		/// level) and a node `w` in the target tree (on the corresponding
		/// level) such that (1) all nodes in `S` have the same ancestor at
		/// distance 2δ (i.e., `S` is a valid set; see \ref collectComponents),
		/// and (2) all nodes in `S` are mappable (by the \ref m_restriction) to
		/// `w`.
		class ValidPair {
			public:
				/// The set `S` of source nodes, as indices on the current
				/// level.
				std::vector<int> m_sourceNodes;
				/// The target node `w`, as an index on the current level.
				int m_targetNode;
				/// \todo
				std::map<int, int> m_map; // [ws] no idea, probably for computing the mapping or whatever

				/// Creates a new valid pair with an empty map.
				ValidPair(const std::vector<int>& sourceNodes, int targetNode);
				/// Creates a new valid pair.
				ValidPair(const std::vector<int>& sourceNodes, int targetNode,
				          std::map<int, int> map);

				friend bool operator<(const ValidPair& lhs, const ValidPair& rhs) {
					return lhs.m_targetNode < rhs.m_targetNode ||
					       (lhs.m_targetNode == rhs.m_targetNode &&
					        lhs.m_sourceNodes < rhs.m_sourceNodes);
				}
				friend bool operator==(const ValidPair& lhs, const ValidPair& rhs) {
					return lhs.m_targetNode == rhs.m_targetNode &&
					       lhs.m_sourceNodes == rhs.m_sourceNodes;
				}
		};

		/// Finds all valid pairs (see \ref ValidPair) for this source tree and
		/// the given target tree, for the given level.
		std::vector<ValidPair> collectValidPairs(const AugmentedTree& targetTree, int level,
		                                         double delta) const;

		/// Computes the level of the LCA of the two given nodes.
		int levelOfLCA(int level, int index1, int index2) const;

		double distanceOfLCA(int level, int index1, int index2) const;

		/// Returns all parents of the given nodes. We assume `indices` is
		/// sorted. The output will again be sorted, with no duplicates.
		std::vector<int> parentsOf(int level, const std::vector<int>& indices) const;

	private:
		/// Adds a new node to the given level `level`. The node is added as the
		/// last element in `level`, and the last element in `level + 1` is used
		/// as the node's parent. (The new node is also added to the parent's
		/// list of children.)
		void addNode(int level, int mergeTreeIndex);

		/// Computes the depth of the given node, and all nodes in its subtree.
		/// Returns the level index of the deepest node in the given node's
		/// subtree.
		int computeDepths(int level, int index);

		/// List of levels in this augmented merge tree. Each level is
		/// represented by a list of nodes.
		std::vector<std::vector<Node>> m_levels;

		/// List of level heights in this augmented merge tree. Always has the
		/// same number of elements as \ref m_levels.
		std::vector<double> m_levelHeights;

		/// The interleaving distance restriction.
		const RestrictionMatrix& m_restrictions;
};
