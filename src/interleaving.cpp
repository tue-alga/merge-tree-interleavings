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

#include "interleaving.h"

Interleaving::Interleaving(const std::shared_ptr<MergeTree>& sourceTree,
                           const std::shared_ptr<MergeTree>& targetTree, double delta,
                           std::map<int, int> matching)
    : m_sourceTree(sourceTree), m_targetTree(targetTree), m_delta(delta), m_matching(matching) {}

double Interleaving::getDelta() const {
	return m_delta;
}

void Interleaving::setDelta(double delta) {
	m_delta = delta;
}

void Interleaving::print() const {
	for (auto [first, second] : m_matching) {
		std::cout << first << " -> " << second << std::endl;
	}
}
