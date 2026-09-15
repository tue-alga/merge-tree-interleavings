#pragma once

#include <vector>

#include "mergetree.h"

/// A merge tree divided into columns.
class ColumnTree {

	public:
		/// Constructs a column tree from the given merge tree.
		ColumnTree(const MergeTree& tree);

		struct Column {
			public:
				/// The index of this column.
				int m_index;

				/// Index of the leaf in the merge tree.
				int m_leafIndex;
				/// Height of the leaf in the merge tree.
				double m_leafHeight;
				/// Index of the merge node in the merge tree. This is
				/// `std::nullopt` for the column that includes the root (the
				/// one with index 0).
				std::optional<int> m_mergeIndex;
				/// Height of the merge node in the merge tree. This is
				/// `std::nullopt` for the column that includes the root (the
				/// one with index 0).
				std::optional<double> m_mergeHeight;
				/// The index of the column this column merges into. This is
				/// `std::nullopt` for the column that includes the root (the
				/// one with index 0).
				std::optional<int> m_parent;

				/// Creates a new column.
				Column(int index, int leafIndex, double leafHeight);
		};

		/// Returns the `i`-th column.
		const Column& getColumn(int i) const;

		/// Returns the column id of the ancestor of column `i` at the given height
		int ancestorAtHeight(int i, double height) const;

		/// Returns the number of columns in this column tree (which is equal to
		/// the number of leaves in the underlying merge tree).
		int columnCount() const;

		/// Returns the depth of the subtree rooted at the x, where x is the point
		/// in column `i` at height `height`. This assumes that persistent columns 
		/// have deepest leaves
		double getDepth(int i, double height) const;

		/// Returns a list of columns active at the given height.
		const std::vector<int> activeColumnsAt(double height) const;

	private:
		/// Checks if the given height lies between the leaf and the merge
		/// heights of the `i`-th column.
		bool isColumnActive(int i, double height) const;

		/// Adds columns for the subtree rooted at node `rootIndex` of `tree`.
		/// Returns the index of the leftmost column added.
		int buildColumnTree(const MergeTree& tree, int rootIndex);

		std::vector<Column> m_columns;
};
