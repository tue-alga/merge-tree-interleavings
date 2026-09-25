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

#include "interleavingdistance.h"
#include "mergetreereader.h"

int main() {
    auto t1 = std::make_shared<MergeTree>();
	auto t2 = std::make_shared<MergeTree>();

    t1->addLeaf(2.0);
    t2->addLeaf(3.0);

	Interleaving interleaving = computeInterleavingDistance(t1, t2, SearchAlgorithm::DeltaLinearSearch, DeltaGoodMapAlgorithm::DP);

    auto trees = MergeTreeReader::readMergeTrees("example/source-tree.txt", "example/target-tree.txt", "example/restriction-matrix.txt");
    interleaving = computeInterleavingDistance(std::make_shared<MergeTree>(trees.m_sourceTree), std::make_shared<MergeTree>(trees.m_targetTree), SearchAlgorithm::DeltaExponentialSearch, DeltaGoodMapAlgorithm::Sweepline, trees.m_restrictions);
    std::cout << interleaving.getDelta();

    return 0;
}
