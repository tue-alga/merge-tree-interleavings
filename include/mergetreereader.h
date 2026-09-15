#pragma once

#include <string>
#include <vector>

#include "mergetree.h"

class MergeTreeReader {
	public:
		using RestrictionMatrix = std::vector<std::vector<double>>;
		struct MergeTrees {
				MergeTree m_sourceTree;
				MergeTree m_targetTree;
				RestrictionMatrix m_restrictions;
		};

		static MergeTrees readMergeTrees(const std::string& sourceTreeFile, 
			const std::string& targetTreeFile, 
			const std::string& restrictionMatrixFile);

	private:
		static void readMergeTree(MergeTree& tree, const std::vector<std::string>& numbers);
		static void readRestrictionMatrix(RestrictionMatrix& matrix, int width, int height,
		                                  const std::vector<std::string>& numbers);
		static void extendRestrictionMatrix(RestrictionMatrix& matrix,
		                                    const MergeTree& targetTree);
};
