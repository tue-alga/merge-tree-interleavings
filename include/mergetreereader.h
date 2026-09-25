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
