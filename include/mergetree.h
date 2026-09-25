// Copyright (C) 2025-2026 TU Eindhoven
//
// This file is part of merge-tree-interleavings.
// 
// merge-tree-interleavings is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
// 
// merge-tree-interleavings is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// merge-tree-interleavings. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

struct Coordinate {
	double m_x;
	double m_y;
	Coordinate() : m_x(0.0), m_y(0.0) {}
	Coordinate(double x, double y) : m_x(x), m_y(y) {}
};

/// Representation of a merge tree. A merge tree is a binary tree of which each
/// node has a real value called its _height_, such that each simple path from
/// the root to a leaf encounters nodes with monotonically decreasing or
/// increasing height values.
///
/// This class allows building the tree incrementally by maintaining a “merge
/// forest”: one can add leaves (\ref addLeaf()) and merge trees at their roots
/// (\ref merge()). The expectation is that when the tree is fully built,
/// everything has been merged so we are left with a single merge tree (but the
/// class doesn't enforce this.)
///
/// We usually think of a merge tree of having its root at +∞, but this “virtual
/// root” is not represented in the data structure. Instead we say that the root
/// is the first node encountered when walking down/up from the virtual root.
class MergeTree {

	public:
		/// Creates an empty merge tree.
		MergeTree() {}

		/// Adds a new leaf to this merge tree.
		/// \return The index of the new leaf.
		int addLeaf(double height, Coordinate coordinate = {}, int ttkId = -1);

		/// Merges the two given nodes `i1` and `i2` by adding a new merge node
		/// that connects the roots of `i1` and `i2`. If `i1` and `i2` are
		/// already in the same tree, behavior is undefined. If inserting the
		/// merge node at the given `height` would break the merge tree
		/// invariant (that is, if the merge node would be further from the root
		/// than the roots of `i1` and `i2`) the inserted merge node is instead
		/// inserted at the height of whichever of `i1` and `i2` is closer to
		/// the root.
		///
		/// \return The index of the new merge node.
		int merge(int i1, int i2, double height, Coordinate coordinate = {}, int ttkId = -1);

		/// On the path from the given node to the root, returns the last node which
		/// is not closer to the root than the given height.
		std::optional<int> parentAtHeight(int nodeId, double height) const;

		/// Returns the number of nodes in this merge tree.
		int size() const;

		int leafCount() const;

		/// Returns the index of the root in this merge tree
		int rootIndex() const;

		/// Returns the index of the parent of the given node.
		std::optional<int> parentOf(int index) const;

		/// Returns the height of the given node.
		double heightOf(int index) const;

		/// Returns the depth of the given ndoe.
		double depthOf(int index) const;

		/// Returns a list of the indices of the children of the given node.
		const std::vector<int>& childrenOf(int index) const;

		Coordinate coordinateOf(int index) const;
		int ttkIdOf(int index) const;

		/// Checks if the given node is a leaf (that is, if it has no children).
		bool isLeaf(int index) const;

		/// Returns a list of all node heights in this merge tree. The heights
		/// are sorted from the leaves towards the root.
		std::vector<double> getHeights(double shiftToRoot = 0.0) const;

		/// Returns a list of all leaf heights in this merge tree. The heights
		/// are sorted from the leaves towards the root.
		std::vector<double> getLeafHeights(double shiftToRoot = 0.0) const;

		/// Returns a list of all merge node heights in this merge tree. The
		/// heights are sorted from the leaves towards the root.
		std::vector<double> getMergeHeights(double shiftToRoot = 0.0) const;

		/// Finds the highest ancestor of the given node.
		int findRootOf(int index);

		// Outputs this merge tree.
		void output(std::ostream& out);

	private:
		/// A node (leaf or internal node) in the merge tree.
		class Node {
			public:
				/// Index of this node.
				int m_index;
				/// Indices of the children of this node.
				std::vector<int> m_children;
				/// Index of the parent of this node, if any.
				std::optional<int> m_parent;
				/// The depth of this node.
				double m_depth;
				/// The height of this node.
				double m_height;

				Coordinate m_coordinate;
				int m_ttkId;
		};

		/// Outputs one node and its children. Used by \ref output().
		void outputNode(std::ostream& out, int i);
		
		std::vector<Node> m_nodes;
};
