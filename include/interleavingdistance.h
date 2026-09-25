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

#include <functional>
#include <iostream>
#include <optional>

#include "interleaving.h"
#include "mergetree.h"

using RestrictionMatrix = std::vector<std::vector<double>>;

enum class SearchAlgorithm {
	DeltaLinearSearch,
	DeltaExponentialSearch
};

enum class DeltaGoodMapAlgorithm {
	DP,
	Sweepline
};

/// Computes an optimal interleaving between the two given trees.
/// The distance is computed using the specified search and decision procedures.
/// Optionally, one can use a restriction matrix to compute a restricted interleaving.
Interleaving computeInterleavingDistance(const std::shared_ptr<MergeTree>& sourceTree,
                                         const std::shared_ptr<MergeTree>& targetTree,
                                         SearchAlgorithm searchAlgorithm,
                                         DeltaGoodMapAlgorithm deltaGoodMapAlgorithm,
                                         RestrictionMatrix restrictions = {},
                                         std::function<void(std::pair<double, double>)> onMovedSweepline = nullptr,
                                         std::function<void(double)> onStartedDelta = nullptr,
                                         std::function<void(double)> onEndedDelta = nullptr);
