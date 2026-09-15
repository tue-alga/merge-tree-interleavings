#include "augmentedtree.h"

#include <stack>

#include "partitioner.h"

AugmentedTree::AugmentedTree(const MergeTree& tree, const std::vector<double>& heights,
                             const RestrictionMatrix& restrictions)
    : m_levels(heights.size(), std::vector<Node>()), m_levelHeights(heights),
      m_restrictions(restrictions) {

	// Do a depth-first traversal of the tree. For every level height we
	// encounter, add a node to that level. Maintain a stack of (level
	// ID, node ID) pairs. (The node ID refers to the ID of the node in
	// tree.)
	std::stack<std::pair<int, int>> stack;

	// Start at the tree's root.
	stack.push({heights.size() - 1, tree.size() - 1});

	while (!stack.empty()) {
		auto [level, nodeID] = stack.top();
		stack.pop();
		// Until we reach the level height that corresponds to the node,
		// add nodes to the current branch.
		while (level >= 0 && heights[level] >= tree.heightOf(nodeID)) {
			addNode(level, nodeID);
			level--;
		}
		// The last node we added corresponds to the actual merge tree
		// node.
		m_levels[level + 1].back().m_isMergeTreeNode = true;
		// Then visit the node's children.
		const std::vector<int>& children = tree.childrenOf(nodeID);
		for (auto it = children.rbegin(); it != children.rend(); it++) {
			stack.push({level, *it});
		}
	}

	// Set the depth values for each node.
	computeDepths(heights.size() - 1, 0);
}

const AugmentedTree::Node& AugmentedTree::get(int level, int index) const {
	assert(level >= 0 && level < m_levels.size());
	return m_levels[level][index];
}

const std::vector<AugmentedTree::Node>& AugmentedTree::get(int level) const {
	assert(level >= 0 && level < m_levels.size());
	return m_levels[level];
}

double AugmentedTree::getLevelHeight(int level) const {
	assert(level >= 0 && level < m_levels.size());
	return m_levelHeights[level];
}

int AugmentedTree::getLevelCount() const {
	return m_levels.size();
}

std::vector<int> AugmentedTree::getChildren(int level, const std::vector<int>& indices) {
	std::vector<int> children;
	for (int v : indices) {
		for (int child : m_levels[level][v].m_children) {
			children.push_back(child);
		}
	}
	return children;
}

std::map<int, std::vector<int>> AugmentedTree::collectComponents(int level, double delta) const {
	std::map<int, std::vector<int>> components;

	// Find which level we're returning nodes from: the last level that
	// is at most 2δ closer to the root than the current level.
	double limit = m_levelHeights[level] + 2 * delta;
	int targetLevel = level;
	while (targetLevel < m_levels.size() - 1 && limit >= m_levelHeights[targetLevel + 1]) {
		targetLevel++;
	}

	// Now for each node on the current level, walk up to the target
	// level and insert the resulting mapping into the result.
	for (int index = 0; index < m_levels[level].size(); index++) {
		int ancestorIndex = index;
		for (int ancestorLevel = level; ancestorLevel < targetLevel; ancestorLevel++) {
			ancestorIndex = *m_levels[ancestorLevel][ancestorIndex].m_parent;
		}
		components[ancestorIndex].push_back(index);
	}
	return components;
}

AugmentedTree::ValidPair::ValidPair(const std::vector<int>& sourceNodes, int targetNode)
    : m_sourceNodes(sourceNodes), m_targetNode(targetNode), m_map({}) {};

AugmentedTree::ValidPair::ValidPair(const std::vector<int>& sourceNodes, int targetNode,
                                    std::map<int, int> map)
    : m_sourceNodes(sourceNodes), m_targetNode(targetNode), m_map(std::move(map)) {};

std::vector<AugmentedTree::ValidPair> AugmentedTree::collectValidPairs(const AugmentedTree& targetTree, int level,
                                                        double delta) const {
	std::map<int, std::vector<int>> components = collectComponents(level, delta);
	std::vector<AugmentedTree::ValidPair> validPairs;
	for (int targetNode = 0; targetNode < targetTree.get(level).size(); targetNode++) {
		for (const auto& [_, component] : components) {
			std::vector<int> mappableNodesInComponent;
			for (int sourceNode : component) {
				bool sourceIsLeaf = get(level, sourceNode).m_children.empty();
				if (!sourceIsLeaf) {
					mappableNodesInComponent.push_back(sourceNode);
				} else {
					double minimumMappableHeight =
					    m_restrictions[get(level, sourceNode).m_mergeTreeIndex]
					                  [targetTree.get(level, targetNode).m_mergeTreeIndex];
					if (targetTree.getLevelHeight(level) >= minimumMappableHeight) {
						mappableNodesInComponent.push_back(sourceNode);
					}
				}
			}

			// We use Partition to iterate over all subsets
			// `partition.first` of the component.
			for (const auto& partition : Partition(mappableNodesInComponent)) {
				std::vector<int> sourceNodes = partition.first;
				std::map<int, int> map;
				if (!sourceNodes.empty()) {
					for (int sourceNode : sourceNodes) {
						if (m_levels[level][sourceNode].m_children.empty()) {
							map[m_levels[level][sourceNode].m_mergeTreeIndex] =
							    targetTree.m_levels[level][targetNode].m_mergeTreeIndex;
						}
					}
					validPairs.push_back(ValidPair{sourceNodes, targetNode, map});
				}
			}
		}
	}
	return validPairs;
}

int AugmentedTree::levelOfLCA(int level, int index1, int index2) const {
	// TODO replace by fancy RMQ stuff

	// Walk upwards from index1.
	std::vector<int> ancestorOnLevel(m_levels.size(), -1);
	int currentLevel = level;
	int currentIndex = index1;
	ancestorOnLevel[currentLevel] = currentIndex;
	while (m_levels[currentLevel][currentIndex].m_parent) {
		currentIndex = *m_levels[currentLevel][currentIndex].m_parent;
		currentLevel++;
		ancestorOnLevel[currentLevel] = currentIndex;
	}

	// Walk upwards from index2, and stop when we encounter the path from
	// index1.
	currentLevel = level;
	currentIndex = index2;
	while (currentIndex != ancestorOnLevel[currentLevel]) {
		currentIndex = *m_levels[currentLevel][currentIndex].m_parent;
		currentLevel++;
	}
	return currentLevel;
}

double AugmentedTree::distanceOfLCA(int level, int index1, int index2) const {
	int lca = levelOfLCA(level, index1, index2);
	return m_levelHeights[lca] - m_levelHeights[level];
}

std::vector<int> AugmentedTree::parentsOf(int level, const std::vector<int>& indices) const {
	std::vector<int> result;
	for (int index : indices) {
		std::optional<int> parent = m_levels[level][index].m_parent;
		if (parent && (result.empty() || result.back() != *parent)) {
			result.push_back(*parent);
		}
	}
	return result;
}

void AugmentedTree::addNode(int level, int mergeTreeIndex) {
	int nodeIndex = m_levels[level].size();
	if (level == m_levels.size() - 1) {
		m_levels.back().push_back({level, nodeIndex, {}, std::nullopt, mergeTreeIndex, false});
	} else {
		int parentIndex = m_levels[level + 1].size() - 1;
		m_levels[level + 1][parentIndex].m_children.push_back(nodeIndex);
		m_levels[level].push_back({level, nodeIndex, {}, parentIndex, mergeTreeIndex, false});
	}
}

int AugmentedTree::computeDepths(int level, int index) {
	int min = level;
	for (int child : m_levels[level][index].m_children) {
		min = std::min(min, computeDepths(level - 1, child));
	}
	m_levels[level][index].m_depth = std::abs(m_levelHeights[min] - m_levelHeights[level]);
	return min;
}
