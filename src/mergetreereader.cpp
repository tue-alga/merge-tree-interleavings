#include "mergetreereader.h"

#include <fstream>
#include <limits>
#include <sstream>
#include <string>

MergeTreeReader::MergeTrees MergeTreeReader::readMergeTrees(const std::string& fileName) {

	std::ifstream file(fileName);
	if (!file) {
		throw std::runtime_error("File could not be read");
	}

	std::vector<std::string> numbers;
	std::string token;
	while (file >> token) {
		numbers.push_back(token);
	}

	int index = 0;

	MergeTrees result;
	try {
		readMergeTree(result.m_sourceTree, numbers, index);
		readMergeTree(result.m_targetTree, numbers, index);
		readRestrictionMatrix(result.m_restrictions, result.m_targetTree.leafCount(),
		                      result.m_sourceTree.leafCount(), numbers, index);
		extendRestrictionMatrix(result.m_restrictions, result.m_targetTree);
	} catch (std::runtime_error& e) {
		std::cerr << e.what() << '\n';
		return {};
	}

	return result;
}

void MergeTreeReader::readMergeTree(MergeTree& tree, const std::vector<std::string>& numbers, int& index) {
	bool ok;

	// Read leaves.
	int leafCount;
	try {
		leafCount = std::stoi(numbers[index++]);
	} catch (...) {
		throw std::runtime_error("leaf count missing");
	}
	
	for (int i = 0; i < leafCount; i++) {
		int leafId;
		try {
			leafId = std::stoi(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid leaf ID");
		}
		if (leafId != i) {
			throw std::runtime_error("invalid leaf ID");
		}
				
		double leafHeight, x, y;
		try {
			leafHeight = std::stod(numbers[index++]);
			x = std::stod(numbers[index++]);
			y = std::stod(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid leaf data");
		}

		int ttkId;
		try {
			ttkId = std::stoi(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid merge node ID");
		}

		tree.addLeaf(leafHeight, {x, y}, ttkId);
	}

	// Read merge nodes.
	int mergeNodeCount;
	try {
		mergeNodeCount = std::stoi(numbers[index++]);
	} catch (...) {
		throw std::runtime_error("merge node count missing");
	}

	for (int i = 0; i < mergeNodeCount; i++) {
		int mergeId;
		try {
			mergeId = std::stoi(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid merge node ID");
		}
		if (mergeId != leafCount + i) {
			throw std::runtime_error("invalid merge node ID");
		}
		
		double mergeHeight, x, y;
		try {
			mergeHeight = std::stod(numbers[index++]);
			x = std::stod(numbers[index++]);
			y = std::stod(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid merge noed data");
		}
	
		int childId1, childId2;
		try {
			childId1 = std::stoi(numbers[index++]);
			childId2 = std::stoi(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid merge node child");
		}

		int ttkId;
		try {
			ttkId = std::stoi(numbers[index++]);
		} catch (...) {
			throw std::runtime_error("invalid merge node ID");
		}

		tree.merge(childId1, childId2, mergeHeight, {x, y}, ttkId);
	}
}

void MergeTreeReader::readRestrictionMatrix(RestrictionMatrix& matrix, int width, int height,
                                            const std::vector<std::string>& numbers, int& index) {
	bool ok;
	matrix.clear();
	for (int i = 0; i < height; i++) {
		std::vector<double> row;
		for (int j = 0; j < width; j++) {
			double value;
			try {
				value = std::stod(numbers[index++]);
			} catch (...) {
				throw std::runtime_error("invalid restriction matrix value");
			}
			row.push_back(value);
		}
		matrix.push_back(row);
	}
}

void MergeTreeReader::extendRestrictionMatrix(RestrictionMatrix& matrix,
                                              const MergeTree& targetTree) {
	int height = matrix.size();
	for (int i = 0; i < height; i++) {
		for (int j = targetTree.leafCount(); j < targetTree.size(); j++) {
			std::vector<int> children = targetTree.childrenOf(j);
			double nodeRestrictionHeight = std::numeric_limits<double>::infinity();
			for (int childIndex : children) {
				nodeRestrictionHeight = std::min(nodeRestrictionHeight, matrix[i][childIndex]);
			}
			matrix[i].push_back(nodeRestrictionHeight);
		}
	}
}
