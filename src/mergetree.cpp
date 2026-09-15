#include "mergetree.h"

#include <algorithm>

int MergeTree::addLeaf(double height, Coordinate coordinate, int ttkId) {
	Node node(m_nodes.size(), {}, std::nullopt, 0.0, height, coordinate, ttkId);
	m_nodes.push_back(node);

	return node.m_index;
}

int MergeTree::merge(int i1, int i2, double height, Coordinate coordinate, int ttkId) {
	int root1 = findRootOf(i1);
	int root2 = findRootOf(i2);
	assert(root1 != root2);
	if (height < m_nodes[root1].m_height) {
		height = m_nodes[root1].m_height;
	}
	if (height < m_nodes[root2].m_height) {
		height = m_nodes[root2].m_height;
	}
	assert(height >= m_nodes[root1].m_height);
	assert(height >= m_nodes[root2].m_height);

	double depth1 = m_nodes[root1].m_depth + height - m_nodes[root1].m_height;
	double depth2 = m_nodes[root2].m_depth + height - m_nodes[root2].m_height;

	std::vector<int> children;
	double depth;
	if (depth1 > depth2) {
		children = {root1, root2};
		depth = depth1;
	} else {
		children = {root2, root1};
		depth = depth2;
	}
	Node node(m_nodes.size(), children, std::nullopt, depth, height, coordinate, ttkId);

	m_nodes.push_back(node);
	m_nodes[root1].m_parent = node.m_index;
	m_nodes[root2].m_parent = node.m_index;

	return node.m_index;
}

std::optional<int> MergeTree::parentAtHeight(int nodeId, double height) const {
	if (height < heightOf(nodeId)) {
		return std::nullopt;
	}
	while (parentOf(nodeId) &&
	       (heightOf(*parentOf(nodeId)) == height || heightOf(*parentOf(nodeId)) < height)) {
		nodeId = *parentOf(nodeId);
	}
	return nodeId;
}

int MergeTree::size() const {
	return m_nodes.size();
}

int MergeTree::leafCount() const {
	return (size() + 1) / 2;
}

int MergeTree::rootIndex() const {
	return m_nodes.size() - 1;
}

std::optional<int> MergeTree::parentOf(int index) const {
	assert(index >= 0 && index < m_nodes.size());
	return m_nodes[index].m_parent;
}

double MergeTree::heightOf(int index) const {
	assert(index >= 0 && index < m_nodes.size());
	return m_nodes[index].m_height;
}

double MergeTree::depthOf(int index) const {
	assert(index >= 0 && index < m_nodes.size());	
	return m_nodes[index].m_depth;
}

const std::vector<int>& MergeTree::childrenOf(int index) const {
	assert(index >= 0 && index < m_nodes.size());
	return m_nodes[index].m_children;
}

Coordinate MergeTree::coordinateOf(int index) const {
	assert(index >= 0 && index < m_nodes.size());
	return m_nodes[index].m_coordinate;
}

int MergeTree::ttkIdOf(int index) const {
	assert(index >= 0 && index < m_nodes.size());
	return m_nodes[index].m_ttkId;
}

bool MergeTree::isLeaf(int index) const {
	assert(index >= 0 && index < m_nodes.size());
	return childrenOf(index).size() == 0;
}

std::vector<double> MergeTree::getHeights(double shiftToRoot) const {
	std::vector<double> out;
	out.reserve(size());
	for (int i = 0; i < size(); i++) {
		out.push_back(heightOf(i) + shiftToRoot);
	}
	std::sort(out.begin(), out.end());
	return out;
}

std::vector<double> MergeTree::getLeafHeights(double shiftToRoot) const {
	std::vector<double> out;
	out.reserve(size());
	for (int i = 0; i < size(); i++) {
		if (isLeaf(i)) {
			out.push_back(heightOf(i) + shiftToRoot);
		}
	}
	std::sort(out.begin(), out.end());
	return out;
}

std::vector<double> MergeTree::getMergeHeights(double shiftToRoot) const {
	std::vector<double> out;
	out.reserve(size());
	for (int i = 0; i < size(); i++) {
		if (!isLeaf(i)) {
			out.push_back(heightOf(i) + shiftToRoot);
		}
	}
	std::sort(out.begin(), out.end());
	return out;
}

int MergeTree::findRootOf(int index) {
	while (m_nodes[index].m_parent) {
		assert(index >= 0 && index < m_nodes.size());
		index = *m_nodes[index].m_parent;
	}
	return index;
}

void MergeTree::output(std::ostream& out) {
	for (const Node& n : m_nodes) {
		if (!n.m_parent) {
			outputNode(out, n.m_index);
			out << "\n";
		}
	}
}

void MergeTree::outputNode(std::ostream& out, int i) {
	const Node& n = m_nodes[i];
	out << "(" << n.m_height;
	for (int i = 0; i < n.m_children.size(); i++) {
		out << ", ";
		outputNode(out, n.m_children[i]);
	}
	out << ")";
}
