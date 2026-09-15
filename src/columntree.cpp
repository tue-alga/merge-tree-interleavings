#include "columntree.h"
#include <algorithm>

ColumnTree::ColumnTree(const MergeTree& tree) {
	buildColumnTree(tree, tree.rootIndex());
}

ColumnTree::Column::Column(int index, int leafIndex, double leafHeight): m_index(index), m_leafIndex(leafIndex), m_leafHeight(leafHeight) {};

const ColumnTree::Column& ColumnTree::getColumn(int i) const {
	return m_columns[i];
}

int ColumnTree::ancestorAtHeight(int i, double height) const {
	// If the current column is the root column, or if its height
	// exceeds the given height, return the current column's index
	while (i > 0 && *m_columns[i].m_mergeHeight < height) {
		i = *m_columns[i].m_parent;
	}
	return i;
}


int ColumnTree::columnCount() const {
	return m_columns.size();
}


double ColumnTree::getDepth(int i, double height) const {
	return height - m_columns[i].m_leafHeight;
}

const std::vector<int> ColumnTree::activeColumnsAt(double height) const {
	std::vector<int> activeColumns;
	for (int i = 0; i < columnCount(); i++) {
		if (isColumnActive(i, height)) {
			activeColumns.push_back(i);
		}
	}
	return activeColumns;
}

bool ColumnTree::isColumnActive(int i, double height) const {
	const Column& column = m_columns[i];
	return height >= column.m_leafHeight && (i == 0 || height < *column.m_mergeHeight);
}

int ColumnTree::buildColumnTree(const MergeTree& tree, int rootIndex) {
	if (tree.isLeaf(rootIndex)) {
		int newColumnIndex = m_columns.size();
		m_columns.emplace_back(newColumnIndex, rootIndex, tree.heightOf(rootIndex));
		return newColumnIndex;
	}

	// Assumes child indices are sorted such that the deepest column persists.
	const std::vector<int>& childIndices = tree.childrenOf(rootIndex);
	int childColumn = buildColumnTree(tree, childIndices[0]);
	for (int i = 1; i < childIndices.size(); i++) {
		int newColumn = buildColumnTree(tree, childIndices[i]);
		m_columns[newColumn].m_parent = childColumn;
		m_columns[newColumn].m_mergeIndex = rootIndex;
		m_columns[newColumn].m_mergeHeight = tree.heightOf(rootIndex);
	}
	return childColumn;
}
