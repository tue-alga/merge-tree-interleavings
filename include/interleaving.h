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

#include <map>
#include <memory>

#include "mergetree.h"

/// An interleaving between two merge trees.
class Interleaving {

	public:
		/// Creates an interleaving for the given trees, with the given delta
		/// value and matching.
		Interleaving(const std::shared_ptr<MergeTree>& sourceTree,
		             const std::shared_ptr<MergeTree>& targetTree, double delta,
		             std::map<int, int> matching);

		/// Returns the shift of this interleaving.
		double getDelta() const;
		/// Sets the shift of this interleaving.
		void setDelta(double delta);

		void print() const;

	private:
		/// The source tree.
		std::shared_ptr<MergeTree> m_sourceTree;
		/// The target tree.
		std::shared_ptr<MergeTree> m_targetTree;
		/// The shift of this interleaving.
		double m_delta;
		/// The matching from source tree nodes to target tree nodes.
		std::map<int, int> m_matching;
};
